/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RpgTriggers.h"

#include "BudgetValues.h"
#include "GuildCreateActions.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SocialMgr.h"

bool NoRpgTargetTrigger::IsActive() { return !AI_VALUE(GuidPosition, "rpg target"); }

bool HasRpgTargetTrigger::IsActive() { return !NoRpgTargetTrigger::IsActive(); }

bool FarFromRpgTargetTrigger::IsActive()
{
    return !NoRpgTargetTrigger::IsActive() && AI_VALUE2(float, "distance", "rpg target") > INTERACTION_DISTANCE;
}

bool NearRpgTargetTrigger::IsActive()
{
    return !NoRpgTargetTrigger::IsActive() && !FarFromRpgTargetTrigger::IsActive();
}

GuidPosition RpgTrigger::getGuidP() { return AI_VALUE(GuidPosition, "rpg target"); }

bool RpgTrigger::IsActive() { return true; }

Event RpgTrigger::Check()
{
    if (!NoRpgTargetTrigger::IsActive() && (AI_VALUE(std::string, "next rpg action") == "choose rpg target") ||
        !FarFromRpgTargetTrigger::IsActive())
        return Trigger::Check();

    return Event();
}

bool RpgTaxiTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_FLIGHTMASTER))
        return false;

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.getX(), guidP.getY(), guidP.getZ(), guidP.getMapId(), bot->GetTeamId());

    if (!node)
        return false;

    if (!bot->m_taxi.IsTaximaskNodeKnown(node))
        return false;

    return true;
}

bool RpgDiscoverTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_FLIGHTMASTER))
        return false;

    if (bot->isTaxiCheater())
        return false;

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.getX(), guidP.getY(), guidP.getZ(), guidP.getMapId(), bot->GetTeamId());

    if (bot->m_taxi.IsTaximaskNodeKnown(node))
        return false;

    return true;
}

bool RpgStartQuestTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.IsCreature() && !guidP.IsGameObject())
        return false;

    if (AI_VALUE(bool, "can fight equal"))
    {
        if (AI_VALUE2(bool, "can accept quest npc", guidP.GetEntry()))
            return true;
    }
    else
    {
        if (AI_VALUE2(bool, "can accept quest low level npc", guidP.GetEntry()))
            return true;
    }

    return false;
}

bool RpgEndQuestTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.IsCreature() && !guidP.IsGameObject())
        return false;

    if (AI_VALUE2(bool, "can turn in quest npc", guidP.GetEntry()))
        return true;

    if (!AI_VALUE2(bool, "can accept quest low level npc", guidP.GetEntry()))
        return false;

    if (guidP.GetEntry() == AI_VALUE(TravelTarget*, "travel target")->getEntry())
        return true;

    return false;
}

bool RpgBuyTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_VENDOR))
        return false;

    if (AI_VALUE(uint8, "durability") > 50)
        return false;

    if (!AI_VALUE(bool, "can sell"))  // Need better condition.
        return false;

    return true;
}

bool RpgSellTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_VENDOR))
        return false;

    if (!AI_VALUE(bool, "can sell"))
        return false;

    return true;
}

bool RpgRepairTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_REPAIR))
        return false;

    if (AI_VALUE2_LAZY(bool, "group or", "should sell,can sell,following party,near leader"))
        return true;

    if (AI_VALUE2_LAZY(bool, "group or", "should repair,can repair,following party,near leader"))
        return true;

    return false;
}

bool RpgTrainTrigger::IsTrainerOf(CreatureTemplate const* cInfo, Player* pPlayer)
{
    switch (cInfo->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if (pPlayer->getClass() != cInfo->trainer_class)
            {
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if (pPlayer->getClass() != CLASS_HUNTER)
            {
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if (cInfo->trainer_race && pPlayer->getRace() != cInfo->trainer_race)
            {
                // Allowed to train if exalted
                if (FactionTemplateEntry const* faction_template = sFactionTemplateStore.LookupEntry(cInfo->faction))
                {
                    if (pPlayer->GetReputationRank(faction_template->faction) == REP_EXALTED)
                        return true;
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if (cInfo->trainer_spell && !pPlayer->HasSpell(cInfo->trainer_spell))
            {
                return false;
            }
            break;
        default:
            return false;  // checked and error output at creature_template loading
    }

    return true;
}

bool RpgTrainTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_TRAINER))
        return false;

    CreatureTemplate const* cInfo = guidP.GetCreatureTemplate();

    if (!IsTrainerOf(cInfo, bot))
        return false;

    // check present spell in trainer spell list
    TrainerSpellData const* cSpells = sObjectMgr->GetNpcTrainerSpells(guidP.GetEntry());
    if (!cSpells)
    {
        return false;
    }

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction);
    float fDiscountMod = bot->GetReputationPriceDiscount(factionTemplate);

    TrainerSpellMap trainer_spells;
    if (cSpells)
        trainer_spells.insert(cSpells->spellList.begin(), cSpells->spellList.end());

    for (TrainerSpellMap::const_iterator itr = trainer_spells.begin(); itr != trainer_spells.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        if (!tSpell)
            continue;

        TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
        if (state != TRAINER_SPELL_GREEN)
            continue;

        uint32 spellId = tSpell->spell;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        uint32 cost = uint32(floor(tSpell->spellCost * fDiscountMod));
        if (cost > AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells))
            continue;

        return true;
    }

    return false;
}

bool RpgHealTrigger::IsActive()
{
    if (!botAI->HasStrategy("heal", BOT_STATE_COMBAT))
        return false;

    GuidPosition guidP(getGuidP());

    Unit* unit = guidP.GetUnit();

    if (!unit)
        return false;

    if (!unit->IsFriendlyTo(bot))
        return false;

    if (unit->isDead() || unit->GetHealthPct() >= 100)
        return false;

    return true;
}

bool RpgHomeBindTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_INNKEEPER))
        return false;

    if (AI_VALUE(WorldPosition, "home bind").distance(bot) < 500.0f)
        return false;

    return true;
}

bool RpgQueueBGTrigger::IsActive()
{
    // skip bots not in continents
    if (!WorldPosition(bot).isOverworld())  // bg, raid, dungeon
        return false;

    GuidPosition guidP(getGuidP());

    if (!guidP.IsCreature())
        return false;
    
    // if bot is not leader disallow tag bg
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    if (AI_VALUE(BattlegroundTypeId, "rpg bg type") == BATTLEGROUND_TYPE_NONE)
        return false;

    return true;
}

bool RpgBuyPetitionTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.HasNpcFlag(UNIT_NPC_FLAG_PETITIONER))
        return false;

    if (!BuyPetitionAction::canBuyPetition(bot))
        return false;

    return true;
}

bool RpgUseTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.IsGameObject())
        return false;

    GameObjectTemplate const* goInfo = guidP.GetGameObjectTemplate();

    switch (goInfo->type)
    {
        case GAMEOBJECT_TYPE_BINDER:
        case GAMEOBJECT_TYPE_GENERIC:
        case GAMEOBJECT_TYPE_TEXT:
        case GAMEOBJECT_TYPE_GOOBER:
        case GAMEOBJECT_TYPE_TRANSPORT:
        case GAMEOBJECT_TYPE_AREADAMAGE:
        case GAMEOBJECT_TYPE_CAMERA:
        case GAMEOBJECT_TYPE_MAP_OBJECT:
        case GAMEOBJECT_TYPE_MO_TRANSPORT:
        case GAMEOBJECT_TYPE_DUEL_ARBITER:
        case GAMEOBJECT_TYPE_FISHINGNODE:
        case GAMEOBJECT_TYPE_GUARDPOST:
        case GAMEOBJECT_TYPE_SPELLCASTER:
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_AURA_GENERATOR:
            return false;
        default:
            break;
    }

    return true;
}

bool RpgSpellTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (guidP.IsPlayer())
        return false;

    if (!guidP.GetWorldObject())
        return false;

    return true;
}

bool RpgCraftTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (guidP.IsPlayer())
        return false;

    if (!guidP.GetWorldObject())
        return false;

    return true;
}

bool RpgTradeUsefulTrigger::IsActive()
{
    GuidPosition guidP(getGuidP());

    if (!guidP.IsPlayer())
        return false;

    Player* player = guidP.GetPlayer();
    if (!player)
        return false;

    // More code/botAI-value here to see if bot is friendly enough.
    bool isFriend = false;
    if (player->GetGuildId() != bot->GetGuildId())
        isFriend = true;

    if (bot->GetGroup() == player->GetGroup() && !urand(0, 5))
        isFriend = true;

    if (!urand(0, 20))
        isFriend = true;

    if (!isFriend)
        return false;

    if (!player->IsWithinLOSInMap(bot))
        return false;

    if (player->GetTrader() && player->GetTrader() != bot)
        return false;

    if (bot->GetTrader() && bot->GetTrader() != player)
        return false;

    if (AI_VALUE(std::vector<Item*>, "items useful to give").empty())
        return false;

    return true;
}

bool RpgDuelTrigger::IsActive()
{
    if (!botAI->HasStrategy("start duel", BOT_STATE_NON_COMBAT))
        return false;

    // Less spammy duels
    if (bot->GetLevel() < 3)
        return false;

    if (botAI->HasRealPlayerMaster())
    {
        // do not auto duel if master is not afk
        if (botAI->GetMaster() && !botAI->GetMaster()->isAFK())
            return false;
    }

    // do not auto duel with low hp
    if (AI_VALUE2(uint8, "health", "self target") < 90)
        return false;

    GuidPosition guidP(getGuidP());

    if (!guidP.IsPlayer())
        return false;

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    if (player->GetLevel() > bot->GetLevel() + 3)
        return false;

    if (bot->GetLevel() > player->GetLevel() + 10)
        return false;

    // caster or target already have requested duel
    if (bot->duel || player->duel || !player->GetSocial() || player->GetSocial()->HasIgnore(bot->GetGUID()))
        return false;

    AreaTableEntry const* targetAreaEntry = sAreaTableStore.LookupEntry(player->GetAreaId());
    if (targetAreaEntry && !(targetAreaEntry->flags & AREA_FLAG_ALLOW_DUELS))
    {
        // Dueling isn't allowed here
        return false;
    }

    if (!AI_VALUE(GuidVector, "all targets").empty())
        return false;

    return true;
}
