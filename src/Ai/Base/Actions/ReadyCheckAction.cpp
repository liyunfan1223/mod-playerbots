/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ReadyCheckAction.h"
#include "Event.h"
#include "Playerbots.h"
#include <memory>
#include <mutex>
#include <vector>

static void SendReadyConfirm(Player* bot)
{
    WorldPacket packet(MSG_RAID_READY_CHECK);
    packet << bot->GetGUID();
    packet << uint8(1);
    bot->GetSession()->HandleRaidReadyCheckOpcode(packet);
}

std::string const formatPercent(std::string const name, uint8 value, float percent)
{
    std::ostringstream out;

    std::string color;
    if (percent > 75)
        color = "|cff00ff00";
    else if (percent > 50)
        color = "|cffffff00";
    else
        color = "|cffff0000";

    out << "|cffffffff[" << name << "]" << color << "x" << (int)value;
    return out.str();
}

class ReadyChecker
{
public:
    virtual ~ReadyChecker() = default;
    virtual bool Check(PlayerbotAI* botAI, AiObjectContext* context) = 0;
    virtual std::string const getName() = 0;
    virtual bool PrintAlways() { return true; }

    static std::vector<std::unique_ptr<ReadyChecker>> checkers;
    static std::once_flag initFlag;
};

std::vector<std::unique_ptr<ReadyChecker>> ReadyChecker::checkers;
std::once_flag ReadyChecker::initFlag;

class HealthChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* /*botAI*/, AiObjectContext* context) override
    {
        return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig.AlmostFullHealth;
    }

    std::string const getName() override { return "HP"; }
};

class ManaChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* /*botAI*/, AiObjectContext* context) override
    {
        return !AI_VALUE2(bool, "has mana", "self target") ||
               AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.MediumHealth;
    }

    std::string const getName() override { return "MP"; }
};

class DistanceChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* botAI, AiObjectContext* /*context*/) override
    {
        Player* bot = botAI->GetBot();
        if (Player* master = botAI->GetMaster())
        {
            bool distance = bot->GetDistance(master) <= sPlayerbotAIConfig.SightDistance;
            if (!distance)
            {
                return false;
            }
        }

        return true;
    }

    bool PrintAlways() override { return false; }
    std::string const getName() override { return "Far away"; }
};

class HunterChecker : public ReadyChecker
{
public:
    bool Check(PlayerbotAI* botAI, AiObjectContext* /*context*/) override
    {
        Player* bot = botAI->GetBot();
        if (bot->getClass() == CLASS_HUNTER)
        {
            if (!bot->GetUInt32Value(PLAYER_AMMO_ID))
            {
                botAI->TellError("Out of ammo!");
                return false;
            }

            if (!bot->GetPet())
            {
                botAI->TellError("No pet!");
                return false;
            }

            if (bot->GetPet()->GetHappinessState() == UNHAPPY)
            {
                botAI->TellError("Pet is unhappy!");
                return false;
            }
        }

        return true;
    }

    bool PrintAlways() override { return false; }
    std::string const getName() override { return "Far away"; }
};

class ItemCountChecker : public ReadyChecker
{
public:
    ItemCountChecker(std::string const item, std::string const name) : item(item), name(name) {}

    bool Check(PlayerbotAI* /*botAI*/, AiObjectContext* context) override
    {
        return AI_VALUE2(uint32, "item count", item) > 0;
    }

    std::string const getName() override { return name; }

private:
    std::string const item;
    std::string const name;
};

class ManaPotionChecker : public ItemCountChecker
{
public:
    ManaPotionChecker(std::string const item, std::string const name) : ItemCountChecker(item, name) {}

    bool Check(PlayerbotAI* botAI, AiObjectContext* context) override
    {
        return !AI_VALUE2(bool, "has mana", "self target") || ItemCountChecker::Check(botAI, context);
    }
};

bool ReadyCheckAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();
    ObjectGuid player;
    p.rpos(0);
    if (!p.empty())
    {
        p >> player;
        if (player == bot->GetGUID())
            return false;

        // Defer the ready reply until buffs are topped off.
        if (sPlayerbotAIConfig.ForceRebuffOnReadyCheck && !bot->IsInCombat() &&
            botAI->HasStrategy("force rebuff", BOT_STATE_NON_COMBAT))
        {
            ReportReadinessToMaster();
            botAI->forceRebuff.Begin(true);
            return true;
        }
    }

    return ReadyCheck();
}

void ReadyCheckAction::ReportReadinessToMaster()
{
    std::call_once(
        ReadyChecker::initFlag,
        []()
        {
            ReadyChecker::checkers.reserve(8);

            ReadyChecker::checkers.emplace_back(std::make_unique<HealthChecker>());
            ReadyChecker::checkers.emplace_back(std::make_unique<ManaChecker>());
            ReadyChecker::checkers.emplace_back(std::make_unique<DistanceChecker>());
            ReadyChecker::checkers.emplace_back(std::make_unique<HunterChecker>());

            ReadyChecker::checkers.emplace_back(std::make_unique<ItemCountChecker>("food", "Food"));
            ReadyChecker::checkers.emplace_back(std::make_unique<ManaPotionChecker>("drink", "Water"));
            ReadyChecker::checkers.emplace_back(std::make_unique<ItemCountChecker>("healing potion", "Hpot"));
            ReadyChecker::checkers.emplace_back(std::make_unique<ManaPotionChecker>("mana potion", "Mpot"));
        });

    bool result = true;
    for (auto const& checkerPtr : ReadyChecker::checkers)
    {
        if (!checkerPtr)
            continue;

        bool ok = checkerPtr->Check(botAI, context);
        result = result && ok;
    }

    std::ostringstream out;

    uint32 hp = AI_VALUE2(uint32, "item count", "healing potion");
    out << formatPercent("Hp", hp, 100.0 * hp / 5);

    out << ", ";
    uint32 food = AI_VALUE2(uint32, "item count", "food");
    out << formatPercent("Food", food, 100.0 * food / 20);

    if (AI_VALUE2(bool, "has mana", "self target"))
    {
        out << ", ";
        uint32 mp = AI_VALUE2(uint32, "item count", "mana potion");
        out << formatPercent("Mp", mp, 100.0 * mp / 5);

        out << ", ";
        uint32 water = AI_VALUE2(uint32, "item count", "water");
        out << formatPercent("Water", water, 100.0 * water / 20);
    }

    botAI->TellMaster(out);
}

bool ReadyCheckAction::ReadyCheck()
{
    ReportReadinessToMaster();

    SendReadyConfirm(bot);

    botAI->forceRebuff.End();

    botAI->ChangeStrategy("-ready check", BOT_STATE_NON_COMBAT);

    return true;
}

bool FinishReadyCheckAction::Execute(Event /*event*/) { return ReadyCheck(); }

// Manual "rebuff" command: opens a rebuff window with no ready check to answer.
bool ForceRebuffAction::Execute(Event /*event*/)
{
    if (bot->IsInCombat() || !botAI->HasStrategy("force rebuff", BOT_STATE_NON_COMBAT))
        return false;

    botAI->forceRebuff.Begin(false);
    return true;
}

bool ReadyReplyAction::isUseful()
{
    ForceRebuffState const& rebuff = botAI->forceRebuff;
    return rebuff.IsPending() && !rebuff.IsOnGlobalCooldown() &&
           !bot->GetCurrentSpell(CURRENT_GENERIC_SPELL) && !bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) &&
           !rebuff.IsBuffPendingThisCycle();
}

bool ReadyReplyAction::Execute(Event /*event*/)
{
    if (botAI->forceRebuff.ShouldReplyToReadyCheck())
        SendReadyConfirm(bot);

    botAI->forceRebuff.End();
    return true;
}
