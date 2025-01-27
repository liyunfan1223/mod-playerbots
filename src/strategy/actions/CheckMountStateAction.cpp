/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CheckMountStateAction.h"
#include "BattlegroundWS.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SpellAuraEffects.h"

bool CheckMountStateAction::Execute(Event event)
{
    bool noAttackers = !AI_VALUE2(bool, "combat", "self target") || !AI_VALUE(uint8, "attacker count");
    bool shouldDismount = false;
    bool shouldMount = false;

    if (Unit* currentTarget = AI_VALUE(Unit*, "current target"))
    {
        float dismountDistance = CalculateDismountDistance();
        float mountDistance = CalculateMountDistance();

        float combatReach = bot->GetCombatReach() + currentTarget->GetCombatReach();
        float distanceToTarget = bot->GetExactDist(currentTarget);
        shouldDismount = distanceToTarget <= dismountDistance + combatReach;
        shouldMount = distanceToTarget > mountDistance + combatReach;
    }
    else
    {
        shouldMount = true;
    }

    if (bot->IsMounted() && shouldDismount)
    {
        Dismount();
        return true;
    }

    Player* master = GetMaster();
    if (master && !bot->InBattleground())
    {
        if (!bot->GetGroup() || bot->GetGroup()->GetLeaderGUID() != master->GetGUID())
            return false;

        if (ShouldFollowMasterMountState(master, noAttackers, shouldMount))
            return Mount();

        if (ShouldDismountForMaster(master))
        {
            Dismount();
            return true;
        }

        return false;
    }

    if (!bot->InBattleground() && !master && !bot->IsMounted() && noAttackers && shouldMount && !bot->IsInCombat())
        return Mount();

    if (ShouldMountInBattleground())
        return Mount();

    if (!bot->IsFlying() && shouldDismount && bot->IsMounted() && (AI_VALUE(Unit*, "enemy player target") || AI_VALUE(Unit*, "dps target") || (!noAttackers && bot->IsInCombat())))
    {
        Dismount();
        return true;
    }

    return false;
}

bool CheckMountStateAction::isUseful()
{
    if (botAI->IsInVehicle() || bot->isDead() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) || !bot->IsOutdoors() || bot->InArena())
        return false;

    if (!bot->IsMounted() && bot->GetPositionZ() < bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
        return false;

    if (!GET_PLAYERBOT_AI(bot)->HasStrategy("mount", BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    if (bot->GetLevel() < 20)
        return false;

    if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
        return false;

    if (bot->InBattleground())
    {
        if (Battleground* bg = bot->GetBattleground())
            if (bg->GetStatus() == STATUS_WAIT_JOIN && bg->GetStartDelayTime() > BG_START_DELAY_30S)
                return false;
    }

    return true;
}

bool CheckMountStateAction::Mount()
{
    if (bot->isMoving())
        bot->StopMoving();

    Player* master = GetMaster();
    botAI->RemoveShapeshift();
    botAI->RemoveAura("tree of life");

    int32 masterSpeed = CalculateMasterMountSpeed(master);
    bool hasSwiftMount = CheckForSwiftMount();

    auto allSpells = GetAllMountSpells();
    int32 masterMountType = GetMountType(master);

    if (TryPreferredMount(master))
        return true;

    auto& spells = allSpells[masterMountType];
    if (hasSwiftMount)
        FilterMountsBySpeed(spells, masterSpeed);

    if (TryRandomMount(spells))
        return true;

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "mount");
    if (!items.empty())
        return UseItemAuto(*items.begin());

    return false;
}

float CheckMountStateAction::CalculateDismountDistance() const
{
    float dismountDistance = PlayerbotAI::IsMelee(bot) ? sPlayerbotAIConfig->meleeDistance + 2.0f : sPlayerbotAIConfig->spellDistance + 2.0f;
    return bot->getClass() == CLASS_WARRIOR ? std::max(18.0f, dismountDistance) : dismountDistance;
}

float CheckMountStateAction::CalculateMountDistance() const
{
    return std::max(21.0f, PlayerbotAI::IsMelee(bot) ? sPlayerbotAIConfig->meleeDistance + 10.0f : sPlayerbotAIConfig->spellDistance + 10.0f);
}

void CheckMountStateAction::Dismount()
{
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
}

bool CheckMountStateAction::ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const
{
    //return master->IsMounted() && !bot->IsMounted() && noAttackers && shouldMount && !bot->IsInCombat() && botAI->GetState() != BOT_STATE_COMBAT;
    auto masterInShapeshiftForm = master->GetShapeshiftForm();
    return ((master->IsMounted() || masterInShapeshiftForm == FORM_FLIGHT || masterInShapeshiftForm == FORM_FLIGHT_EPIC || masterInShapeshiftForm == FORM_TRAVEL)
        && !bot->IsMounted() && noAttackers && shouldMount && !bot->IsInCombat() && botAI->GetState() != BOT_STATE_COMBAT);
}

bool CheckMountStateAction::ShouldDismountForMaster(Player* master) const
{
    auto masterInShapeshiftForm = master->GetShapeshiftForm();
    return ((!master->IsMounted() && masterInShapeshiftForm != FORM_FLIGHT && masterInShapeshiftForm != FORM_FLIGHT_EPIC && masterInShapeshiftForm != FORM_TRAVEL)
        && bot->IsMounted());
}

bool CheckMountStateAction::ShouldMountInBattleground() const
{
    if (!bot->InBattleground() || bot->IsMounted() || bot->IsInCombat())
        return false;

    if (bot->GetBattlegroundTypeId() == BATTLEGROUND_WS)
    {
        BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();
        if (bot->HasAura(23333) || bot->HasAura(23335))
            return false;
    }

    return true;
}

int32 CheckMountStateAction::CalculateMasterMountSpeed(Player* master) const
{
    if (!master || bot->InBattleground())
        return 59;

    auto masterInShapeshiftForm = master->GetShapeshiftForm();

    auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
    if (!auraEffects.empty())
    {
        SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
        return std::max(masterSpell->Effects[1].BasePoints, masterSpell->Effects[2].BasePoints);
    }
    else if (masterInShapeshiftForm == FORM_FLIGHT_EPIC)
        return 279;
    else if (masterInShapeshiftForm == FORM_FLIGHT)
        return 149;

    // By default
    return 59;
}

bool CheckMountStateAction::CheckForSwiftMount() const
{
    for (auto& itr : bot->GetSpellMap())
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr.first);
        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        if (itr.second->State == PLAYERSPELL_REMOVED || !itr.second->Active || spellInfo->IsPassive())
            continue;

        int32 effect = std::max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints);
        if ((effect > 59 && spellInfo->Effects[1].ApplyAuraName != SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ||
            (effect > 149 && spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
            return true;
    }
    return false;
}

std::map<uint32, std::map<int32, std::vector<uint32>>> CheckMountStateAction::GetAllMountSpells() const
{
    std::map<uint32, std::map<int32, std::vector<uint32>>> allSpells;
    for (auto& itr : bot->GetSpellMap())
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr.first);
        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        if (itr.second->State == PLAYERSPELL_REMOVED || !itr.second->Active || spellInfo->IsPassive())
            continue;

        int32 effect = std::max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints);
        uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        spellInfo->Id == 54729) ? 1 : 0;

        allSpells[index][effect].push_back(itr.first);
    }
    return allSpells;
}

bool CheckMountStateAction::TryPreferredMount(Player* master) const
{
    QueryResult checkTable = PlayerbotsDatabase.Query(
        "SELECT EXISTS(SELECT * FROM information_schema.tables WHERE table_schema = 'acore_playerbots' AND table_name = 'playerbots_preferred_mounts')");

    if (checkTable && checkTable->Fetch()[0].Get<uint32>() == 1)
    {
        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT spellid FROM playerbots_preferred_mounts WHERE guid = {} AND type = {}",
            bot->GetGUID().GetCounter(), GetMountType(master));

        if (result)
        {
            std::vector<uint32> mounts;
            do
            {
                mounts.push_back(result->Fetch()[0].Get<uint32>());
            } while (result->NextRow());

            uint32 index = urand(0, mounts.size() - 1);
            if (index < mounts.size() && sSpellMgr->GetSpellInfo(mounts[index]))
                return botAI->CastSpell(mounts[index], bot);
        }
    }
    return false;
}

uint32 CheckMountStateAction::GetMountType(Player* master) const
{
    // MountType 0: Ground
    // MountType 1: Flying
    if (!master)
        return 0;

    auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
    auto masterInShapeshiftForm = master->GetShapeshiftForm();

    if (!auraEffects.empty())
    {
        SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
        return (masterSpell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                masterSpell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;
    }
    else if (masterInShapeshiftForm == FORM_FLIGHT || masterInShapeshiftForm == FORM_FLIGHT_EPIC)
        return 1;

    // By default: Ground
    return 0;
}

void CheckMountStateAction::FilterMountsBySpeed(std::map<int32, std::vector<uint32>>& spells, int32 masterSpeed) const
{
    for (auto& i : spells)
    {
        for (auto& id : i.second)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
            if (!spellInfo)
                continue;

            if (masterSpeed > 59 && std::max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints) < 99)
                spells[59].clear();

            if (masterSpeed > 149 && std::max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints) < 279)
                spells[149].clear();
        }
    }
}

bool CheckMountStateAction::TryRandomMount(const std::map<int32, std::vector<uint32>>& spells) const
{
    for (auto& i : spells)
    {
        const std::vector<uint32>& ids = i.second;
        if (ids.empty())
            continue;

        uint32 index = urand(0, ids.size() - 1);
        if (index < ids.size())
            return botAI->CastSpell(ids[index], bot);
    }
    return false;
}
