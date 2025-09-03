/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CastCustomSpellAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "Playerbots.h"
#include "ServerFacade.h"

size_t FindLastSeparator(std::string const text, std::string const sep)
{
    size_t pos = text.rfind(sep);
    if (pos == std::string::npos)
        return pos;

    size_t lastLinkBegin = text.rfind("|H");
    size_t lastLinkEnd = text.find("|h|r", lastLinkBegin + 1);
    if (pos >= lastLinkBegin && pos <= lastLinkEnd)
        pos = text.find_last_of(sep, lastLinkBegin);

    return pos;
}

static inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

bool CastCustomSpellAction::Execute(Event event)
{
    // only allow proper vehicle seats
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    Player* master = GetMaster();
    Unit* target = nullptr;
    std::string text = event.getParam();

    GuidVector gos = chat->parseGameobjects(text);
    if (!gos.empty())
    {
        for (auto go : gos)
        {
            if (!target)
                target = botAI->GetUnit(go);

            if (!botAI->GetUnit(go) || !botAI->GetUnit(go)->IsInWorld())
                continue;

            chat->eraseAllSubStr(text, chat->FormatWorldobject(botAI->GetUnit(go)));
        }

        ltrim(text);
    }

    uint32 spell = 0;
    if (!target)
    {
        size_t onPos = FindLastSeparator(text, " on ");
        if (onPos != std::string::npos)
        {
            std::string targetName = text.substr(onPos + 4);
            ltrim(targetName);
            if (!targetName.empty())
            {
                // check if spell still exists after we remove " on PlayerName" part
                std::string truncatedText = text.substr(0, onPos);
                ltrim(truncatedText);
                spell = AI_VALUE2(uint32, "spell id", truncatedText);

                if (spell)
                {
                    if (Player* targetPlayer = ObjectAccessor::FindPlayerByName(targetName))
                    {
                        target = targetPlayer;
                        text = truncatedText;
                    }
                }
            }
        }
    }

    if (!target)
        if (master && master->GetTarget())
            target = botAI->GetUnit(master->GetTarget());

    if (!target)
        target = bot;

    if (!master)  // Use self as master for permissions.
        master = bot;

    Item* itemTarget = nullptr;

    size_t pos = FindLastSeparator(text, " ");
    uint32 castCount = 1;
    if (pos != std::string::npos)
    {
        std::string const param = text.substr(pos + 1);
        std::vector<Item*> items = InventoryAction::parseItems(param, ITERATE_ITEMS_IN_BAGS);
        if (!items.empty())
            itemTarget = *items.begin();
        else
        {
            if (atoi(param.c_str()) > 0)
                text = text.substr(0, pos);
        }
    }

    if (!spell)
        spell = AI_VALUE2(uint32, "spell id", text);

    std::ostringstream msg;
    if (!spell)
    {
        msg << "Unknown spell " << text;
        botAI->TellError(msg.str());
        return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
    if (!spellInfo)
    {
        msg << "Unknown spell " << text;
        botAI->TellError(msg.str());
        return false;
    }

    if (target != bot && !bot->HasInArc(CAST_ANGLE_IN_FRONT, target, sPlayerbotAIConfig->sightDistance))
    {
        sServerFacade->SetFacingTo(bot, target);
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);

        msg << "cast " << text;
        botAI->HandleCommand(CHAT_MSG_WHISPER, msg.str(), master);
        return true;
    }

    std::ostringstream spellName;
    spellName << ChatHelper::FormatSpell(spellInfo) << " on ";

    if (bot->GetTrader())
        spellName << "trade item";
    else if (itemTarget)
        spellName << chat->FormatItem(itemTarget->GetTemplate());
    else if (target == bot)
        spellName << "self";
    else
        spellName << target->GetName();

    if (!bot->GetTrader() && !botAI->CanCastSpell(spell, target, true, itemTarget))
    {
        msg << "Cannot cast " << spellName.str();
        botAI->TellError(msg.str());
        return false;
    }

    bool result = spell ? botAI->CastSpell(spell, target, itemTarget) : botAI->CastSpell(text, target, itemTarget);
    if (result)
    {
        msg << "Casting " << spellName.str();

        if (castCount > 1)
        {
            std::ostringstream cmd;
            cmd << castString(target) << " " << text << " " << (castCount - 1);
            botAI->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), master);
            msg << "|cffffff00(x" << (castCount - 1) << " left)|r";
        }

        botAI->TellMasterNoFacing(msg.str());
    }
    else
    {
        msg << "Cast " << spellName.str() << " is failed";
        botAI->TellError(msg.str());
    }

    return result;
}

bool CastCustomNcSpellAction::isUseful() { return !bot->IsInCombat(); }

std::string const CastCustomNcSpellAction::castString(WorldObject* target)
{
    return "castnc " + chat->FormatWorldobject(target);
}

bool CastRandomSpellAction::AcceptSpell(SpellInfo const* spellInfo)
{
    bool isTradeSkill = spellInfo->Effects[EFFECT_0].Effect == SPELL_EFFECT_CREATE_ITEM &&
                        spellInfo->ReagentCount[EFFECT_0] > 0 && spellInfo->SchoolMask == 1;
    return !isTradeSkill && spellInfo->GetRecoveryTime() < MINUTE * IN_MILLISECONDS;
}

bool CastRandomSpellAction::Execute(Event event)
{
    std::vector<std::pair<uint32, std::string>> spellMap = GetSpellList();
    Player* master = GetMaster();

    Unit* target = nullptr;
    GameObject* got = nullptr;

    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    GuidVector wos = chat->parseGameobjects(name);
    for (auto wo : wos)
    {
        target = botAI->GetUnit(wo);
        got = botAI->GetGameObject(wo);

        if (got || target)
            break;
    }

    if (!got && !target && bot->GetTarget())
    {
        target = botAI->GetUnit(bot->GetTarget());
        got = botAI->GetGameObject(bot->GetTarget());
    }

    if (!got && !target)
        if (master && master->GetTarget())
            target = botAI->GetUnit(master->GetTarget());

    if (!got && !target)
        target = bot;

    std::vector<std::pair<uint32, std::pair<uint32, WorldObject*>>> spellList;

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (!AcceptSpell(spellInfo))
            continue;

        if (bot->HasSpell(spellId))
        {
            uint32 spellPriority = GetSpellPriority(spellInfo);

            if (target && botAI->CanCastSpell(spellId, target, true))
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, target)));
            if (got &&
                botAI->CanCastSpell(spellId, got->GetPositionX(), got->GetPositionY(), got->GetPositionZ(), true))
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, got)));
            if (botAI->CanCastSpell(spellId, bot, true))
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, bot)));
        }
    }

    if (spellList.empty())
        return false;

    // bool isCast = false; //not used, line marked for removal.

    std::sort(spellList.begin(), spellList.end(),
              [](std::pair<uint32, std::pair<uint32, WorldObject*>> i,
                 std::pair<uint32, std::pair<uint32, WorldObject*>> j) { return i.first > j.first; });

    uint32 rndBound = spellList.size() / 4;

    rndBound = std::min(rndBound, (uint32)10);
    rndBound = std::max(rndBound, (uint32)0);

    for (uint32 i = 0; i < 5; i++)
    {
        uint32 rnd = urand(0, rndBound);

        auto spell = spellList[rnd];

        uint32 spellId = spell.first;
        WorldObject* wo = spell.second.second;

        bool isCast = castSpell(spellId, wo);

        if (isCast)
        {
            if (MultiCast && ((wo && bot->HasInArc(CAST_ANGLE_IN_FRONT, wo, sPlayerbotAIConfig->sightDistance))))
            {
                std::ostringstream cmd;
                cmd << "castnc " << chat->FormatWorldobject(wo) + " " << spellId << " " << 19;
                botAI->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), bot);
            }
            return true;
        }
    }

    return false;
}

bool CraftRandomItemAction::AcceptSpell(SpellInfo const* spellInfo)
{
    return spellInfo->Effects[EFFECT_0].Effect == SPELL_EFFECT_CREATE_ITEM && spellInfo->ReagentCount[EFFECT_0] > 0 &&
           spellInfo->SchoolMask == 0;
}

uint32 CraftRandomItemAction::GetSpellPriority(SpellInfo const* spellInfo)
{
    if (spellInfo->Effects[EFFECT_0].Effect != SPELL_EFFECT_CREATE_ITEM)
    {
        uint32 newItemId = spellInfo->Effects[EFFECT_0].ItemType;
        if (newItemId)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", newItemId);

            if (usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_AMMO ||
                usage == ITEM_USAGE_QUEST || usage == ITEM_USAGE_SKILL || usage == ITEM_USAGE_USE)
                return 10;
        }

        if (ItemUsageValue::SpellGivesSkillUp(spellInfo->Id, bot))
            return 8;
    }

    return 1;
}

bool CastRandomSpellAction::castSpell(uint32 spellId, WorldObject* wo)
{
    if (wo->GetGUID().IsUnit())
        return botAI->CastSpell(spellId, (Unit*)(wo));
    else
        return botAI->CastSpell(spellId, wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ());
}

bool DisEnchantRandomItemAction::Execute(Event event)
{
    std::vector<Item*> items =
        AI_VALUE2(std::vector<Item*>, "inventory items", "usage " + std::to_string(ITEM_USAGE_DISENCHANT));
    std::reverse(items.begin(), items.end());

    for (auto& item : items)
    {
        // don't touch rare+ items if with real player/guild
        if ((botAI->HasRealPlayerMaster() || botAI->IsInRealGuild()) &&
            item->GetTemplate()->Quality > ITEM_QUALITY_UNCOMMON)
            return false;

        if (CastCustomSpellAction::Execute(
                Event("disenchant random item", "13262 " + chat->FormatQItem(item->GetEntry()))))
            return true;
    }

    return false;
}

bool DisEnchantRandomItemAction::isUseful()
{
    return botAI->HasSkill(SKILL_ENCHANTING) && !bot->IsInCombat() &&
           AI_VALUE2(uint32, "item count", "usage " + std::to_string(ITEM_USAGE_DISENCHANT)) > 0;
}

bool EnchantRandomItemAction::isUseful() { return botAI->HasSkill(SKILL_ENCHANTING) && !bot->IsInCombat(); }

bool EnchantRandomItemAction::AcceptSpell(SpellInfo const* spellInfo)
{
    return spellInfo->Effects[EFFECT_0].Effect == SPELL_EFFECT_ENCHANT_ITEM && spellInfo->ReagentCount[EFFECT_0] > 0;
}

uint32 EnchantRandomItemAction::GetSpellPriority(SpellInfo const* spellInfo)
{
    if (spellInfo->Effects[EFFECT_0].Effect == SPELL_EFFECT_ENCHANT_ITEM)
    {
        if (AI_VALUE2(Item*, "item for spell", spellInfo->Id) && ItemUsageValue::SpellGivesSkillUp(spellInfo->Id, bot))
            return 10;
    }

    return 1;
}
