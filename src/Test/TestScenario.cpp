/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "TestRunner.h"
#include "TestScenario.h"

#include "Bag.h"
#include "CharacterCache.h"
#include "Creature.h"
#include "GroupMgr.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "SpellDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "StringFormat.h"

#include <memory>
#include <set>

void TestContext::Step(std::string name, uint32 timeoutMs, std::function<StepStatus(TestContext&)> tick)
{
    steps.push_back(TestStep{ std::move(name), timeoutMs, std::move(tick) });
}

void TestContext::Do(std::string name, std::function<bool(TestContext&)> action)
{
    Step(std::move(name), 0, [action = std::move(action)](TestContext& ctx)
    {
        return action(ctx) ? StepStatus::Done : StepStatus::Failed;
    });
}

void TestContext::WaitUntil(std::string name, uint32 timeoutMs, std::function<bool(TestContext&)> pred)
{
    Step(std::move(name), timeoutMs, [pred = std::move(pred)](TestContext& ctx)
    {
        return pred(ctx) ? StepStatus::Done : StepStatus::InProgress;
    });
}

void TestContext::Assert(std::string name, std::function<bool(TestContext&)> pred)
{
    Do(std::move(name), std::move(pred));
}

Player* TestContext::GetBot(size_t index) const
{
    if (index >= botGuids.size())
        return nullptr;

    Player* bot = ObjectAccessor::FindConnectedPlayer(botGuids[index]);
    return bot && bot->IsInWorld() ? bot : nullptr;
}

Player* TestContext::FirstAliveBot() const
{
    for (size_t i = 0; i < botGuids.size(); ++i)
        if (Player* bot = GetBot(i))
            if (bot->IsAlive())
                return bot;

    return nullptr;
}

uint32 TestContext::AliveBotCount() const
{
    uint32 count = 0;
    for (size_t i = 0; i < botGuids.size(); ++i)
        if (Player* bot = GetBot(i))
            if (bot->IsAlive())
                ++count;

    return count;
}

void TestContext::ForEachBotAI(std::function<void(Player*, PlayerbotAI*)> const& fn) const
{
    for (size_t i = 0; i < botGuids.size(); ++i)
        if (Player* bot = GetBot(i))
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                fn(bot, botAI);
}

void TestContext::MarkSkull(ObjectGuid target) const
{
    Player* leader = GetBot(0);
    if (!leader)
        return;

    if (Group* group = leader->GetGroup())
        group->SetTargetIcon(7, leader->GetGUID(), target);
}

void TestContext::OrderAttack(size_t index) const
{
    if (Player* bot = GetBot(index))
        if (bot->IsAlive())
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                botAI->DoSpecificAction("attack rti target");
}

void TestContext::OrderAttackAll(size_t skipOffTanksBelow) const
{
    for (size_t i = 0; i < botGuids.size(); ++i)
    {
        if (i > 0 && i < skipOffTanksBelow)
            continue;

        OrderAttack(i);
    }
}

void TestContext::PrepareRaid(uint32 level, std::vector<BotDef> const& comp, GearDef gear, uint8 progression,
                              uint32 mapId, Position const& pos, std::vector<uint32> const& spare)
{
    SpawnBots(level, comp, gear);
    SetProgression(progression);
    FormGroup(true);
    TeleportTo(mapId, pos);
    ClearMap(spare);
    ReadyCheck();
}

void TestContext::SpawnBots(uint32 level, std::vector<BotDef> const& bots, GearDef gear, bool alliance)
{
    struct SpawnState
    {
        bool requested = false;
        uint32 lastPrepareMs = 0;
        std::set<ObjectGuid> factoryDone;
    };
    auto state = std::make_shared<SpawnState>();

    // Generous: a fresh test DB creates the whole account pool on first boot.
    Step("spawn bots", 15 * MINUTE * IN_MILLISECONDS, [state, level, bots, gear, alliance](TestContext& ctx) -> StepStatus
    {
        if (!state->requested)
        {
            // Try to pick one offline character per requested class (alliance pool).
            std::set<ObjectGuid> used;
            std::vector<ObjectGuid> picked;
            for (BotDef const& def : bots)
            {
                auto const& cache = sRandomPlayerbotMgr.addclassCache[RandomPlayerbotMgr::GetTeamClassIdx(alliance, def.cls)];
                ObjectGuid chosen;
                for (ObjectGuid const& guid : cache)
                {
                    // Skip bots connected, already picked, or owned by another run.
                    if (used.count(guid) || ObjectAccessor::FindConnectedPlayer(guid) ||
                        IntegrationTestMgr::instance().IsParticipant(guid))
                        continue;

                    // Race-locked pick for race-gated quests (SatisfyQuestRace).
                    if (def.race != 0)
                    {
                        CharacterCacheEntry const* entry = sCharacterCache->GetCharacterCacheByGuid(guid);
                        if (!entry || entry->Race != def.race)
                            continue;
                    }

                    chosen = guid;
                    break;
                }

                if (chosen.IsEmpty())
                    break;

                used.insert(chosen);
                picked.push_back(chosen);
            }

            if (picked.size() < bots.size())
            {
                // Pool not ready (first boot) — refresh it periodically and retry.
                uint32 nowMs = ctx.StepElapsedMs();
                if (nowMs - state->lastPrepareMs >= 30000)
                {
                    state->lastPrepareMs = nowMs;
                    sRandomPlayerbotMgr.PrepareAddclassCache();
                }
                return StepStatus::InProgress;
            }

            for (ObjectGuid const& guid : picked)
                sRandomPlayerbotMgr.AddPlayerBot(guid, 0);  // masterless

            // Class-qualified spec labels: premade names collide across classes.
            static char const* CLASS_NAMES[MAX_CLASSES] = { "", "warrior", "paladin", "hunter", "rogue", "priest",
                                                            "dk", "shaman", "mage", "warlock", "", "druid" };
            ctx.botGuids = picked;
            ctx.botSpecs.clear();
            for (size_t i = 0; i < picked.size(); ++i)
                ctx.botSpecs.push_back(Acore::StringFormat("{} {}",
                    bots[i].cls < MAX_CLASSES ? CLASS_NAMES[bots[i].cls] : "?",
                    bots[i].spec.empty() ? "random" : bots[i].spec));
            state->requested = true;
            return StepStatus::InProgress;
        }

        // Wait until every bot is fully logged in with AI attached.
        for (size_t i = 0; i < ctx.botGuids.size(); ++i)
        {
            Player* bot = ctx.GetBot(i);
            if (!bot || !GET_PLAYERBOT_AI(bot))
                return StepStatus::InProgress;
        }

        // Level + gear each bot once; apply the requested spec after Randomize.
        for (size_t i = 0; i < ctx.botGuids.size(); ++i)
        {
            if (state->factoryDone.count(ctx.botGuids[i]))
                continue;

            Player* bot = ctx.GetBot(i);

            // Pooled chars saved with PLAYER_EXTRA_GM_ON load as GM, which creatures
            // can't attack (they evade) — clear it so combat objectives work.
            bot->SetGameMaster(false);

            uint32 const gearScore = gear.ilvl ? PlayerbotFactory::CalcMixedGearScore(gear.ilvl, gear.quality) : 0;
            PlayerbotFactory factory(bot, level, gear.quality, gearScore);
            factory.Randomize(false);

            std::string const& spec = bots[i].spec;
            if (!spec.empty())
            {
                int specNo = -1;
                for (int no = 0; no < MAX_SPECNO; ++no)
                {
                    if (sPlayerbotAIConfig.PremadeSpecName[bots[i].cls][no] == spec)
                    {
                        specNo = no;
                        break;
                    }
                }

                if (specNo < 0)
                {
                    ctx.AddNote(Acore::StringFormat("SpawnBots: unknown premade spec '{}' for class {}", spec, bots[i].cls));
                    return StepStatus::Failed;
                }

                PlayerbotFactory::InitTalentsBySpecNo(bot, specNo, true);
                PlayerbotFactory regear(bot, level, gear.quality, gearScore);
                regear.InitEquipment(false, sPlayerbotAIConfig.TwoRoundsGearInit);
                regear.InitGlyphs(false);
                // Randomize enchanted the random-spec gear this regear just replaced.
                if (level >= uint32(sPlayerbotAIConfig.MinEnchantingBotLevel))
                    regear.ApplyEnchantAndGemsNew();
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                    botAI->ResetStrategies(false);
            }

            // BiS gearing runs after talents (BiS list resolves by active spec tab).
            if (gear.bis)
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                    if (!botAI->DoSpecificAction("autogear bis",
                            gear.ilvl ? Event("autogear bis", std::to_string(gear.ilvl)) : Event("autogear bis")))
                        ctx.AddNote(Acore::StringFormat("SpawnBots: bis gearing refused for {} (check AutoGearBis* config)",
                            bot->GetName()));

            // Restock consumables: reused pool bots deplete potions/reagents.
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                botAI->DoSpecificAction("maintenance");

            // Masterless bots skip the factory's avoid-aoe strategy; add it back.
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                botAI->ChangeStrategy("+avoid aoe", BOT_STATE_COMBAT);

            // Scenarios start topped off.
            bot->SetFullHealth();
            bot->SetPower(bot->getPowerType(), bot->GetMaxPower(bot->getPowerType()));

            state->factoryDone.insert(ctx.botGuids[i]);
        }

        return StepStatus::Done;
    });
}

void TestContext::FormGroup(bool raid)
{
    Do(raid ? "form raid" : "form group", [raid](TestContext& ctx)
    {
        Player* leader = ctx.GetBot(0);
        if (!leader)
            return false;

        if (leader->GetGroup())
        {
            ctx.AddNote(Acore::StringFormat("FormGroup: bot '{}' is already in a group", leader->GetName()));
            return false;
        }

        Group* group = new Group();
        if (!group->Create(leader))
        {
            delete group;
            return false;
        }
        sGroupMgr->AddGroup(group);

        for (size_t i = 1; i < ctx.botGuids.size(); ++i)
        {
            Player* bot = ctx.GetBot(i);
            if (!bot)
            {
                group->Disband();
                return false;
            }

            if (bot->GetGroup())
            {
                ctx.AddNote(Acore::StringFormat("FormGroup: bot '{}' is already in a group", bot->GetName()));
                group->Disband();
                return false;
            }

            if (raid && !group->isRaidGroup() && group->GetMembersCount() >= 5)
                group->ConvertToRaid();

            if (!group->AddMember(bot))
            {
                group->Disband();
                return false;
            }
        }

        // Set difficulty by raid size unconditionally: a reused leader can carry a
        // stale 25-man difficulty, so a 10-man comp would otherwise fight the 25-man
        // boss (single-mode maps downscale anyway).
        if (raid)
            group->SetRaidDifficulty(ctx.botGuids.size() > 10 ? RAID_DIFFICULTY_25MAN_NORMAL
                                                              : RAID_DIFFICULTY_10MAN_NORMAL);

        ctx.groupLeader = leader->GetGUID();
        return true;
    });
}

void TestContext::ReadyCheck(uint32 minMs, uint32 softCapMs)
{
    // Mirrors the client ready check: 35s rounds, re-issued while anyone is pending,
    // bounded by the soft cap.
    constexpr uint32 ROUND_MS = 35 * IN_MILLISECONDS;
    struct CheckState
    {
        bool started = false;
        uint32 round = 1;
        uint32 roundStartMs = 0;
    };
    auto state = std::make_shared<CheckState>();
    Step("ready check", (softCapMs + 30000), [state, minMs, softCapMs](TestContext& ctx) -> StepStatus
    {
        bool anyInCombat = false;
        uint32 pending = 0;
        ctx.ForEachBotAI([&](Player* bot, PlayerbotAI* botAI)
        {
            if (!bot->IsAlive())
                return;

            if (!state->started)
                botAI->forceRebuff.Begin(false);
            else if (botAI->forceRebuff.IsPending())
                ++pending;

            if (bot->IsInCombat())
                anyInCombat = true;
        });

        if (!state->started)
        {
            state->started = true;
            return StepStatus::InProgress;
        }

        uint32 const elapsed = ctx.StepElapsedMs();

        // Pending flags are only meaningful after a few buff cycles.
        if (!anyInCombat && elapsed < minMs)
            return StepStatus::InProgress;

        if (!anyInCombat && pending && elapsed < softCapMs)
        {
            // Round expired with stragglers: check again, carrying rebuff state over.
            if (elapsed - state->roundStartMs >= ROUND_MS)
            {
                ctx.AddNote(Acore::StringFormat("ready check round {} ended: {} still pending", state->round, pending));
                ++state->round;
                state->roundStartMs = elapsed;
            }

            return StepStatus::InProgress;
        }

        if (pending)
        {
            ctx.AddNote(anyInCombat ? "ready check cut short: combat started"
                                    : "ready check timeout: proceeding with unready bots");
            ctx.ForEachBotAI([](Player*, PlayerbotAI* botAI) { botAI->forceRebuff.End(); });
        }

        return StepStatus::Done;
    });
}

TestContext::ReadinessSummary TestContext::NoteReadiness(std::string const& tag)
{
    ReadinessSummary summary;
    for (size_t i = 0; i < botGuids.size(); ++i)
    {
        Player* bot = GetBot(i);
        if (!bot)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

        uint32 manaPotions = 0;
        uint32 freeBagSlots = 0;
        auto scanItem = [&manaPotions](Item* item)
        {
            if (!item)
                return;
            ItemTemplate const* proto = item->GetTemplate();
            if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                (proto->SubClass != ITEM_SUBCLASS_POTION && proto->SubClass != ITEM_SUBCLASS_FLASK))
                return;
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[0].SpellId);
            if (spellInfo && spellInfo->Effects[EFFECT_0].Effect == SPELL_EFFECT_ENERGIZE)
                manaPotions += item->GetCount();
        };
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        {
            Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!item)
                ++freeBagSlots;
            scanItem(item);
        }
        for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
            if (Bag* bag = bot->GetBagByPos(bagSlot))
                for (uint32 slot = 0; slot < bag->GetBagSize(); ++slot)
                {
                    Item* item = bag->GetItemByPos(slot);
                    if (!item)
                        ++freeBagSlots;
                    scanItem(item);
                }

        // Capital = greater (class-wide) variant, lower = single-target.
        std::string blessings;
        if (botAI)
        {
            if (botAI->HasAura("greater blessing of might", bot)) blessings += 'M';
            else if (botAI->HasAura("blessing of might", bot)) blessings += 'm';
            if (botAI->HasAura("greater blessing of wisdom", bot)) blessings += 'W';
            else if (botAI->HasAura("blessing of wisdom", bot)) blessings += 'w';
            if (botAI->HasAura("greater blessing of kings", bot)) blessings += 'K';
            else if (botAI->HasAura("blessing of kings", bot)) blessings += 'k';
            if (botAI->HasAura("greater blessing of sanctuary", bot)) blessings += 'S';
            else if (botAI->HasAura("blessing of sanctuary", bot)) blessings += 's';
        }
        if (blessings.empty())
            blessings = "-";

        uint32 manaPct = bot->GetMaxPower(POWER_MANA)
            ? bot->GetPower(POWER_MANA) * 100 / bot->GetMaxPower(POWER_MANA) : 0;

        if (blessings == "-")
            ++summary.unblessed;
        if (bot->GetMaxPower(POWER_MANA) && !manaPotions)
            ++summary.unstockedManaUsers;

        // Rogues/shamans imbue weapons (temp enchant slot); "-" = missing.
        std::string imbues = "n/a";
        if (bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_SHAMAN)
        {
            imbues.clear();
            for (uint8 slot : { uint8(EQUIPMENT_SLOT_MAINHAND), uint8(EQUIPMENT_SLOT_OFFHAND) })
            {
                Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (!weapon || weapon->GetTemplate()->Class != ITEM_CLASS_WEAPON)
                    continue;
                bool const imbued = weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) != 0;
                imbues += imbued ? (slot == EQUIPMENT_SLOT_MAINHAND ? "MH" : "OH") : "-";
                if (!imbued)
                    ++summary.unimbuedWeapons;
            }
            if (imbues.empty())
                imbues = "none";
        }

        // A warlock's soulstone ends up as an aura on some raid member.
        if (bot->getClass() == CLASS_WARLOCK && botAI)
        {
            bool stoned = false;
            for (size_t j = 0; j < botGuids.size() && !stoned; ++j)
                if (Player* member = GetBot(j))
                    stoned = botAI->HasAura("soulstone resurrection", member);
            if (!stoned)
                ++summary.missingSoulstones;
        }

        Player* leader = GetBot(0);
        float distToLeader = leader && leader != bot && leader->GetMapId() == bot->GetMapId()
            ? bot->GetDistance(leader) : 0.0f;

        AddNote(Acore::StringFormat("readiness@{}: {} class {} spec '{}' lvl {} hp {} mana {}% fr {} grp {} manapots {} bless {} imbue {} freebag {} pvp {} ffa {} map {} dist {:.0f}",
            tag, bot->GetName(), bot->getClass(),
            i < botSpecs.size() ? botSpecs[i] : "?",
            bot->GetLevel(), bot->GetMaxHealth(), manaPct,
            bot->GetResistance(SPELL_SCHOOL_FIRE),
            bot->GetGroup() ? bot->GetGroup()->GetMemberGroup(bot->GetGUID()) : 99,
            manaPotions, blessings, imbues, freeBagSlots,
            bot->IsPvP(), bot->IsFFAPvP(), bot->GetMapId(), distToLeader));
    }

    return summary;
}

void TestContext::ClearMap(std::vector<uint32> const& spareEntries)
{
    Do("clear map", [spareEntries](TestContext& ctx)
    {
        Player* ref = ctx.GetBot(0);
        if (!ref)
            return false;

        Map* map = ref->GetMap();
        map->LoadAllGrids();

        std::vector<Creature*> doomed;
        for (auto const& [spawnId, creature] : map->GetCreatureBySpawnIdStore())
        {
            if (!creature->IsAlive() || creature->IsPet() || creature->IsSummon() || creature->IsInCombat())
                continue;

            if (!creature->IsHostileTo(ref))
                continue;

            if (std::find(spareEntries.begin(), spareEntries.end(), creature->GetEntry()) != spareEntries.end())
                continue;

            doomed.push_back(creature);
        }

        for (Creature* creature : doomed)
            creature->DespawnOrUnsummon();

        ctx.AddNote(Acore::StringFormat("cleared {} hostiles map-wide", doomed.size()));
        return true;
    });
}

void TestContext::SetProgression(uint8 state)
{
    Do("set progression", [state](TestContext& ctx)
    {
        for (size_t i = 0; i < ctx.botGuids.size(); ++i)
        {
            Player* bot = ctx.GetBot(i);
            if (!bot)
                return false;

            for (uint8 tier = 1; tier <= 18; ++tier)
            {
                uint32 questId = 66000 + tier;
                Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
                if (!quest)
                    continue;

                if (tier <= state)
                {
                    if (bot->GetQuestStatus(questId) != QUEST_STATUS_REWARDED)
                    {
                        bot->AddQuest(quest, nullptr);
                        bot->CompleteQuest(questId);
                        bot->RewardQuest(quest, 0, bot, false, false);
                    }
                }
                else if (bot->GetQuestStatus(questId) == QUEST_STATUS_REWARDED)
                    bot->RemoveRewardedQuest(questId);
            }
        }
        return true;
    });
}

namespace
{
    bool TeleportOneBot(Player* bot, uint32 mapId, float x, float y, float z, float o)
    {
        bot->GetMotionMaster()->Clear();
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
            botAI->Reset(true);
        bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP);
        bool result = bot->TeleportTo(mapId, x, y, z, o);
        bot->SendMovementFlagUpdate();
        return result;
    }

    bool BotArrived(ObjectGuid guid, uint32 mapId)
    {
        Player* bot = ObjectAccessor::FindConnectedPlayer(guid);
        return bot && !bot->IsBeingTeleported() && bot->IsInWorld() && bot->GetMapId() == mapId;
    }
}

void TestContext::TeleportTo(uint32 mapId, float x, float y, float z, float o)
{
    // Leader zones in first to create/bind the instance; the rest follow next tick.
    enum class Phase { Leader, Members, Wait };
    auto phase = std::make_shared<Phase>(Phase::Leader);
    Step("teleport", 120 * IN_MILLISECONDS, [phase, mapId, x, y, z, o](TestContext& ctx) -> StepStatus
    {
        switch (*phase)
        {
            case Phase::Leader:
            {
                Player* leader = ctx.GetBot(0);
                if (!leader || !TeleportOneBot(leader, mapId, x, y, z, o))
                    return StepStatus::Failed;

                *phase = Phase::Members;
                return StepStatus::InProgress;
            }
            case Phase::Members:
            {
                if (!BotArrived(ctx.botGuids[0], mapId))
                    return StepStatus::InProgress;

                for (size_t i = 1; i < ctx.botGuids.size(); ++i)
                {
                    Player* bot = ctx.GetBot(i);
                    if (!bot)
                        return StepStatus::Failed;

                    // Stack everyone on one spot: spread bots fall out of buff LOS.
                    if (!TeleportOneBot(bot, mapId, x, y, z, o))
                        return StepStatus::Failed;
                }
                *phase = Phase::Wait;
                return StepStatus::InProgress;
            }
            case Phase::Wait:
            {
                for (ObjectGuid const& guid : ctx.botGuids)
                    if (!BotArrived(guid, mapId))
                        return StepStatus::InProgress;

                return StepStatus::Done;
            }
        }
        return StepStatus::InProgress;
    });
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
