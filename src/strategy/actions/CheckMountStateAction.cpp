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

MountData CollectMountData(const Player* bot)
{
    MountData data;
    for (auto& entry : bot->GetSpellMap())
    {
        uint32 spellId = entry.first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        if (entry.second->State == PLAYERSPELL_REMOVED || !entry.second->Active || spellInfo->IsPassive())
            continue;

        int32 effect1 = spellInfo->Effects[1].BasePoints;
        int32 effect2 = spellInfo->Effects[2].BasePoints;

        int32 speed = std::max(effect1, effect2);

        // Update max speed if appropriate.
        if (speed > data.maxSpeed)
            data.maxSpeed = speed;  // In BG, clamp max speed to 99 later; here we just store the maximum found.

        // Determine index: flight if either effect has flight aura or specific mount ID.
        uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        // Winged Steed of the Ebon Blade
                        // This mount is meant to autoscale from a 150% flyer
                        // up to a 280% as you train your flying skill up.
                        // This incorrectly gets categorised as a ground mount, force this to flyer only.
                        // TODO: Add other scaling mounts here if they have the same issue, or adjust above
                        // checks so that they are all correctly detected.
                        spellInfo->Id == 54729) ? 1 : 0;
        data.allSpells[index][speed].push_back(spellId);
    }
    return data;
}

bool CheckMountStateAction::Execute(Event event)
{
    // Determine if there are no attackers
    bool noAttackers = !AI_VALUE2(bool, "combat", "self target") || !AI_VALUE(uint8, "attacker count");
    bool enemy = AI_VALUE(Unit*, "enemy player target");
    bool dps = AI_VALUE(Unit*, "dps target");
    bool shouldDismount = false;
    bool shouldMount = false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget)
    {
        float dismountDistance = CalculateDismountDistance();
        float mountDistance = CalculateMountDistance();

        // Cache combat reach and distance calculations
        float combatReach = bot->GetCombatReach() + currentTarget->GetCombatReach();
        float distanceToTarget = bot->GetExactDist(currentTarget);
        shouldDismount = (distanceToTarget <= dismountDistance + combatReach);
        shouldMount = (distanceToTarget > mountDistance + combatReach);
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

    // If there is a master and bot not in BG
    Player* master = GetMaster();
    bool inBattleground = bot->InBattleground();
    if (master && !inBattleground)
    {
        Group* group = bot->GetGroup();
        if (!group || group->GetLeaderGUID() != master->GetGUID())
            return false;

        masterInShapeshiftForm = master->GetShapeshiftForm();

        if (ShouldFollowMasterMountState(master, noAttackers, shouldMount))
            return Mount();

        if (ShouldDismountForMaster(master))
        {
            Dismount();
            return true;
        }

        return false;
    }

    // If there is no master or bot in BG
    if ((!master || inBattleground) && !bot->IsMounted() &&
        noAttackers && shouldMount && !bot->IsInCombat())
        return Mount();

    if (!bot->IsFlying() && shouldDismount && bot->IsMounted() &&
        (enemy || dps || (!noAttackers && bot->IsInCombat())))
    {
        Dismount();
        return true;
    }

    return false;
}

bool CheckMountStateAction::isUseful()
{
    if (botAI->IsInVehicle() || bot->isDead() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
        !bot->IsOutdoors() || bot->InArena())
        return false;

    // In addition to checking IsOutdoors, also check whether bot is clipping below floor slightly because that will
    // cause bot to falsly indicate they are outdoors. This fixes bug where bot tries to mount indoors (which seems
    // to mostly be an issue in tunnels of WSG and AV)
    float posZ = bot->GetPositionZ();
    float groundLevel = bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), posZ);
    if (!bot->IsMounted() && posZ < groundLevel)
        return false;

    if (!GET_PLAYERBOT_AI(bot)->HasStrategy("mount", BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    // Do not mount when level lower than minimum required
    if (bot->GetLevel() < sPlayerbotAIConfig->useGroundMountAtMinLevel)
        return false;

    // Allow mounting while transformed only if the form allows it
    if (bot->HasAuraType(SPELL_AURA_TRANSFORM) && bot->IsInDisallowedMountForm())
        return false;

    // BG Logic
    if (bot->InBattleground())
    {
        // Do not use when carrying BG Flags
        if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
            return false;

        // Only mount if BG starts in less than 30 sec
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

    MountData mountData = CollectMountData(bot);
    int32 masterSpeed = CalculateMasterMountSpeed(master, mountData);

    if (TryPreferredMount(master))
        return true;

    int32 masterMountType = GetMountType(master);
    auto spellsIt = mountData.allSpells.find(masterMountType);
    if (spellsIt != mountData.allSpells.end())
    {
        auto& spells = spellsIt->second;
        if (TryRandomMountFiltered(spells, masterSpeed))
            return true;
    }

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "mount");
    if (!items.empty())
        return UseItemAuto(*items.begin());

    return false;
}

float CheckMountStateAction::CalculateDismountDistance() const
{
    // Warrior bots should dismount far enough to charge (because it's important for generating some initial rage),
    // a real player would be riding toward enemy mashing the charge key but the bots won't cast charge while mounted.
    bool isMelee = PlayerbotAI::IsMelee(bot);
    float dismountDistance = isMelee ? sPlayerbotAIConfig->meleeDistance + 2.0f : sPlayerbotAIConfig->spellDistance + 2.0f;
    return bot->getClass() == CLASS_WARRIOR ? std::max(18.0f, dismountDistance) : dismountDistance;
}

float CheckMountStateAction::CalculateMountDistance() const
{
    // Mount distance should be >= 21 regardless of class, because when travelling a distance < 21 it takes longer
    // to cast mount-spell than the time saved from the speed increase. At a distance of 21 both approaches take 3
    // seconds:
    // 21 / 7  =  21 / 14 + 1.5  =  3   (7 = dismounted speed  14 = epic-mount speed  1.5 = mount-spell cast time)
    bool isMelee = PlayerbotAI::IsMelee(bot);
    float baseDistance = isMelee ? sPlayerbotAIConfig->meleeDistance + 10.0f : sPlayerbotAIConfig->spellDistance + 10.0f;
    return std::max(21.0f, baseDistance);
}

void CheckMountStateAction::Dismount()
{
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
}

bool CheckMountStateAction::ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const
{
    bool isMasterMounted = master->IsMounted() || (masterInShapeshiftForm == FORM_FLIGHT ||
                                                    masterInShapeshiftForm == FORM_FLIGHT_EPIC ||
                                                    masterInShapeshiftForm == FORM_TRAVEL);
    return isMasterMounted && !bot->IsMounted() && noAttackers &&
           shouldMount && !bot->IsInCombat() && botAI->GetState() != BOT_STATE_COMBAT;
}

bool CheckMountStateAction::ShouldDismountForMaster(Player* master) const
{
    bool isMasterMounted = master->IsMounted() || (masterInShapeshiftForm == FORM_FLIGHT ||
                                                    masterInShapeshiftForm == FORM_FLIGHT_EPIC ||
                                                    masterInShapeshiftForm == FORM_TRAVEL);
    return !isMasterMounted && bot->IsMounted();
}

int32 CheckMountStateAction::CalculateMasterMountSpeed(Player* master, const MountData& mountData) const
{
    // Check riding skill and level requirements
    int32 ridingSkill = bot->GetPureSkillValue(SKILL_RIDING);
    int32 botLevel = bot->GetLevel();

    if (ridingSkill <= 75 && botLevel < static_cast<int32>(sPlayerbotAIConfig->useFastGroundMountAtMinLevel))
        return 59;

    // If there is a master and bot not in BG, use master's aura effects.
    if (master && !bot->InBattleground())
    {
        auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
        if (!auraEffects.empty())
        {
            SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
            int32 effect1 = masterSpell->Effects[1].BasePoints;
            int32 effect2 = masterSpell->Effects[2].BasePoints;
            return std::max(effect1, effect2);
        }
        else if (masterInShapeshiftForm == FORM_FLIGHT_EPIC)
            return 279;
        else if (masterInShapeshiftForm == FORM_FLIGHT)
            return 149;
    }
    else
    {
        // Bots on their own.
        int32 speed = mountData.maxSpeed;
        if (bot->InBattleground() && speed > 99)
            return 99;

        return speed;
    }

    return 59;
}

uint32 CheckMountStateAction::GetMountType(Player* master) const
{
    if (!master)
        return 0;

    auto auraEffects = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED);

    if (!auraEffects.empty())
    {
        SpellInfo const* masterSpell = auraEffects.front()->GetSpellInfo();
        return (masterSpell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                masterSpell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;
    }
    else if (masterInShapeshiftForm == FORM_FLIGHT || masterInShapeshiftForm == FORM_FLIGHT_EPIC)
        return 1;

    return 0;
}

bool CheckMountStateAction::TryPreferredMount(Player* master) const
{
    static bool tableExists = false;
    static bool tableChecked = false;

    if (!tableChecked)
    {
        // Check for preferred mounts table in db
        QueryResult checkTable = PlayerbotsDatabase.Query(
            "SELECT EXISTS(SELECT * FROM information_schema.tables WHERE table_schema = 'acore_playerbots' AND table_name = 'playerbots_preferred_mounts')");
        tableExists = checkTable && checkTable->Fetch()[0].Get<uint32>() == 1;
        tableChecked = true;
    }

    if (tableExists)
    {
        // Check for preferred mount entry
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

            // Validate spell ID
            // TODO: May want to do checks for 'bot riding skill > skill required to ride the mount'
            if (!mounts.empty())
            {
                uint32 index = urand(0, mounts.size() - 1);
                if (index < mounts.size() && sSpellMgr->GetSpellInfo(mounts[index]))
                    return botAI->CastSpell(mounts[index], bot);
            }
        }
    }
    return false;
}

bool CheckMountStateAction::TryRandomMountFiltered(const std::map<int32, std::vector<uint32>>& spells, int32 masterSpeed) const
{
    // Iterate over each speed group once.
    for (const auto& pair : spells)
    {
        int32 currentSpeed = pair.first;
        if ((masterSpeed > 59 && currentSpeed < 99) || (masterSpeed > 149 && currentSpeed < 279))
            continue;

        const auto& ids = pair.second;
        if (!ids.empty())
        {
            // Pick a random mount from the candidate group.
            uint32 index = urand(0, ids.size() - 1);
            return botAI->CastSpell(ids[index], bot);
        }
    }
    return false;
}
