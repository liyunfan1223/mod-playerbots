/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
    if ((!NoRpgTargetTrigger::IsActive() && (AI_VALUE(std::string, "next rpg action") == "choose rpg target")) ||
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
        sObjectMgr->GetNearestTaxiNode(guidP.GetPositionX(), guidP.GetPositionY(), guidP.GetPositionZ(),
                                       guidP.GetMapId(), bot->GetTeamId());

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
        sObjectMgr->GetNearestTaxiNode(guidP.GetPositionX(), guidP.GetPositionY(), guidP.GetPositionZ(),
                                       guidP.GetMapId(), bot->GetTeamId());

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

bool RpgTrainTrigger::IsActive()
{
    GuidPosition gp = getGuidP();
    if (!gp)
        return false;

    if (!gp.HasNpcFlag(UNIT_NPC_FLAG_TRAINER))
        return false;

    if (!AI_VALUE(bool, "can train"))
        return false;

    return true;
}

bool RpgHealTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot))
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

    if (botAI->HasGameClientMaster())
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
