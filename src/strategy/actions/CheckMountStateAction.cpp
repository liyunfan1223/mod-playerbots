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
    bool noattackers = !AI_VALUE2(bool, "combat", "self target") || !AI_VALUE(uint8, "attacker count");
    bool enemy = AI_VALUE(Unit*, "enemy player target");
    bool dps = AI_VALUE(Unit*, "dps target");
    bool shouldDismount = false;
    bool shouldMount = false;

    if (Unit* currentTarget = AI_VALUE(Unit*, "current target"))
    {
        float dismount_distance;
        float mount_distance;
        if (PlayerbotAI::IsMelee(bot))
        {
            dismount_distance = sPlayerbotAIConfig->meleeDistance + 2.0f;
            mount_distance = sPlayerbotAIConfig->meleeDistance + 10.0f;
        }
        else
        {
            dismount_distance = sPlayerbotAIConfig->spellDistance + 2.0f;
            mount_distance = sPlayerbotAIConfig->spellDistance + 10.0f;
        }

        // warrior bots should dismount far enough to charge (because its important for generating some initial rage),
        // a real player would be riding toward enemy mashing the charge key but the bots wont cast charge while mounted
        if (CLASS_WARRIOR == bot->getClass())
            dismount_distance = std::max(18.0f, dismount_distance);

        // mount_distance should be >= 21 regardless of class, because when travelling a distance < 21 it takes longer
        // to cast mount-spell than the time saved from the speed increase. At a distance of 21 both approaches take 3
        // seconds:
        // 21 / 7  =  21 / 14 + 1.5  =  3   (7 = dismounted speed  14 = epic-mount speed  1.5 = mount-spell cast time)
        mount_distance = std::max(21.0f, mount_distance);

        float combatReach = bot->GetCombatReach() + currentTarget->GetCombatReach();
        float disToTarget = bot->GetExactDist(currentTarget);
        shouldDismount = disToTarget <= dismount_distance + combatReach;
        shouldMount = disToTarget > mount_distance + combatReach;
    }
    else
    {
        shouldDismount = false;
        shouldMount = true;
    }

    if (bot->IsMounted() && shouldDismount)
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        return true;
    }

    Player* master = GetMaster();
    if (master != nullptr && !bot->InBattleground())
    {
        if (!bot->GetGroup() || bot->GetGroup()->GetLeaderGUID() != master->GetGUID())
            return false;

        // bool farFromMaster = sServerFacade->GetDistance2d(bot, master) > sPlayerbotAIConfig->sightDistance;
        if (master->IsMounted() && !bot->IsMounted() && noattackers && shouldMount && !bot->IsInCombat() &&
            botAI->GetState() != BOT_STATE_COMBAT)
        {
            return Mount();
        }

        if (!master->IsMounted() && bot->IsMounted())
        {
            WorldPacket emptyPacket;
            bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
            return true;
        }

        return false;
    }

    // For random bots
    if (!bot->InBattleground() && !master)
    {
        if (!bot->IsMounted() && noattackers && shouldMount && !bot->IsInCombat())
        {
            return Mount();
        }
    }

    if (bot->InBattleground() && shouldMount && noattackers && !bot->IsInCombat() && !bot->IsMounted())
    {
        if (bot->GetBattlegroundTypeId() == BATTLEGROUND_WS)
        {
            BattlegroundWS* bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();
            if (bot->HasAura(23333) || bot->HasAura(23335))
            {
                return false;
            }
        }

        return Mount();
    }

    if (!bot->IsFlying() && shouldDismount && bot->IsMounted() && (enemy || dps || (!noattackers && bot->IsInCombat())))
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        return true;
    }

    return false;
}

bool CheckMountStateAction::isUseful()
{
    // do not use on vehicle
    if (botAI->IsInVehicle())
        return false;

    if (bot->isDead())
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    if (!bot->IsOutdoors())
        return false;

    // in addition to checking IsOutdoors, also check whether bot is clipping below floor slightly because that will
    // cause bot to falsly indicate they are outdoors. This fixes bug where bot tries to mount indoors (which seems
    // to mostly be an issue in tunnels of WSG and AV)
    if (!bot->IsMounted() && bot->GetPositionZ() < bot->GetMapWaterOrGroundLevel(
                                                       bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
        return false;

    if (bot->InArena())
        return false;

    if (!GET_PLAYERBOT_AI(bot)->HasStrategy("mount", BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    bool firstmount = bot->GetLevel() >= 20;
    if (!firstmount)
        return false;

    // Do not use with BG Flags
    if (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))
    {
        return false;
    }

    // Only mount if BG starts in less than 30 sec
    if (bot->InBattleground())
    {
        if (Battleground* bg = bot->GetBattleground())
            if (bg->GetStatus() == STATUS_WAIT_JOIN)
            {
                if (bg->GetStartDelayTime() > BG_START_DELAY_30S)
                    return false;
            }
    }

    return true;
}

bool CheckMountStateAction::Mount()
{
    uint32 secondmount = 40;

    if (bot->isMoving())
    {
        bot->StopMoving();
        // bot->GetMotionMaster()->Clear();
        // bot->GetMotionMaster()->MoveIdle();
    }

    Player* master = GetMaster();
    botAI->RemoveShapeshift();
    botAI->RemoveAura("tree of life");
    int32 masterSpeed = 59;
    SpellInfo const* masterSpell = nullptr;

    if (master && !master->GetAuraEffectsByType(SPELL_AURA_MOUNTED).empty() && !bot->InBattleground())
    {
        masterSpell = master->GetAuraEffectsByType(SPELL_AURA_MOUNTED).front()->GetSpellInfo();
        masterSpeed = std::max(masterSpell->Effects[1].BasePoints, masterSpell->Effects[2].BasePoints);
    }
    else
    {
        masterSpeed = 59;
        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            uint32 spellId = itr->first;
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
                continue;

            if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active || spellInfo->IsPassive())
                continue;

            int32 effect = std::max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints);
            if (effect > masterSpeed)
                masterSpeed = effect;
        }
    }

    if (bot->GetPureSkillValue(SKILL_RIDING) <= 75 && bot->GetLevel() < secondmount)
        masterSpeed = 59;

    if (bot->InBattleground() && masterSpeed > 99)
        masterSpeed = 99;

    bool hasSwiftMount = false;

    // std::map<int32, std::vector<uint32> > spells;
    std::map<uint32, std::map<int32, std::vector<uint32>>> allSpells;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED)
            continue;

        if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active || spellInfo->IsPassive())
            continue;

        int32 effect = std::max(spellInfo->Effects[1].BasePoints, spellInfo->Effects[2].BasePoints);
        // if (effect < masterSpeed)
        // continue;

        uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                        // Winged Steed of the Ebon Blade
                        // This mount is meant to autoscale from a 150% flyer
                        // up to a 280% as you train your flying skill up.
                        // This incorrectly gets categorised as a ground mount, force this to flyer only.
                        // TODO: Add other scaling mounts here if they have the same issue, or adjust above
                        // checks so that they are all correctly detected.
                        spellInfo->Id == 54729)
                           ? 1      // Flying Mount
                           : 0;     // Ground Mount

        if (index == 0 &&
            std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) > 59)
            hasSwiftMount = true;

        if (index == 1 &&
            std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) > 149)
            hasSwiftMount = true;

        allSpells[index][effect].push_back(spellId);
    }

    int32 masterMountType = 0;
    if (masterSpell)
    {
        masterMountType = (masterSpell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
                           masterSpell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
                              ? 1
                              : 0;
    }

    // Check for preferred mounts table in db
    QueryResult checkTable = PlayerbotsDatabase.Query(
        "SELECT EXISTS(SELECT * FROM information_schema.tables WHERE table_schema = 'acore_playerbots' AND table_name = 'playerbots_preferred_mounts')");
    
    if (checkTable)
    {
        uint32 tableExists = checkTable->Fetch()[0].Get<uint32>();
        if (tableExists == 1)
        {
            // Check for preferred mount entry
            QueryResult result = PlayerbotsDatabase.Query(
            "SELECT spellid FROM playerbots_preferred_mounts WHERE guid = {} AND type = {}",
            bot->GetGUID().GetCounter(), masterMountType);

            if (result)
            {
                std::vector<uint32> mounts;
                    do
                    {
                        Field* fields = result->Fetch();
                        uint32 spellId = fields[0].Get<uint32>();
                        mounts.push_back(spellId);
                    } while (result->NextRow());
                
                uint32 index = urand(0, mounts.size() - 1);
                // Validate spell ID
                if (index < mounts.size() && sSpellMgr->GetSpellInfo(mounts[index]))
                {
                    // TODO: May want to do checks for 'bot riding skill > skill required to ride the mount'
                    return botAI->CastSpell(mounts[index], bot);
                }
            }
        }
    }

    // No preferred mount found (or invalid), continue with random mount selection
    std::map<int32, std::vector<uint32>>& spells = allSpells[masterMountType];
    if (hasSwiftMount)
    {
        for (auto i : spells)
        {
            std::vector<uint32> ids = i.second;
            for (auto itr : ids)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr);
                if (!spellInfo)
                    continue;

                if (masterMountType == 0 && masterSpeed > 59 &&
                    std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) < 99)
                    spells[59].clear();

                if (masterMountType == 1 && masterSpeed > 149 &&
                    std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) < 279)
                    spells[149].clear();
            }
        }
    }

    for (std::map<int32, std::vector<uint32>>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        std::vector<uint32>& ids = i->second;
        uint32 index = urand(0, ids.size() - 1);
        if (index >= ids.size())
            continue;

        return botAI->CastSpell(ids[index], bot);
        ;
    }

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "mount");
    if (!items.empty())
        return UseItemAuto(*items.begin());

    return false;
}
