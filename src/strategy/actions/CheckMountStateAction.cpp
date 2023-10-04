/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CheckMountStateAction.h"
#include "BattlegroundWS.h"
#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SpellAuraEffects.h"

bool CheckMountStateAction::Execute(Event event)
{
    bool noattackers = AI_VALUE2(bool, "combat", "self target") ? (AI_VALUE(uint8, "attacker count") > 0 ? false : true) : true;
    bool enemy = AI_VALUE(Unit*, "enemy player target");
    bool dps = (AI_VALUE(Unit*, "dps target") || AI_VALUE(Unit*, "grind target"));
    bool fartarget = (enemy && sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "enemy player target"), 40.0f)) ||
        (dps && sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "dps target"), 50.0f));
    bool attackdistance = false;
    bool chasedistance = false;
    float attack_distance = 35.0f;

    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
            attack_distance = 10.0f;
            break;
        case CLASS_ROGUE:
            attack_distance = 40.0f;
            break;
    }

    if (enemy)
        attack_distance /= 2;

    if (dps || enemy)
    {
        attackdistance = (enemy || dps) && sServerFacade->IsDistanceLessThan(AI_VALUE2(float, "distance", "current target"), attack_distance);
        chasedistance = enemy && sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "enemy player target"), 45.0f) && AI_VALUE2(bool, "moving", "enemy player target");
    }

    Player* master = GetMaster();
    if (master != nullptr && !bot->InBattleground())
    {
        if (!bot->GetGroup() || bot->GetGroup()->GetLeaderGUID() != master->GetGUID())
            return false;

        // bool farFromMaster = sServerFacade->GetDistance2d(bot, master) > sPlayerbotAIConfig->sightDistance;
        if (master->IsMounted() && !bot->IsMounted() && noattackers)
        {
            return Mount();
        }

        if (!master->IsMounted() && bot->IsMounted())
        {
            WorldPacket emptyPacket;
            bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
            return true;
        }
        // if (!bot->IsMounted() && (chasedistance || (farFromMaster && botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))) && !bot->IsInCombat() && !dps)
        //     return Mount();

        // if (!bot->IsFlying() && ((!farFromMaster && !master->IsMounted()) || attackdistance) && bot->IsMounted())
        // {
        //     WorldPacket emptyPacket;
        //     bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        //     return true;
        // }

        return false;
    }

    if (bot->InBattleground() && !attackdistance && (noattackers || fartarget) && !bot->IsInCombat() && !bot->IsMounted())
    {
        if (bot->GetBattlegroundTypeId() == BATTLEGROUND_WS)
        {
            BattlegroundWS *bg = (BattlegroundWS*)botAI->GetBot()->GetBattleground();
            if (bot->HasAura(23333) || bot->HasAura(23335))
            {
                return false;
            }
        }

        return Mount();
    }

    // if (!bot->InBattleground())
    // {
    //     if (AI_VALUE(GuidPosition, "rpg target"))
    //     {
    //         if (sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "rpg target"), sPlayerbotAIConfig->farDistance) && noattackers && !dps && !enemy)
    //             return Mount();
    //     }

    //     if (((!AI_VALUE(GuidVector, "possible rpg targets").empty()) && noattackers && !dps && !enemy) && urand(0, 100) > 50)
    //         return Mount();
    // }

    // if (!bot->IsMounted() && !attackdistance && (fartarget || chasedistance))
    //     return Mount();

    if (!bot->IsFlying() && attackdistance && bot->IsMounted() && (enemy || dps || (!noattackers && bot->IsInCombat())))
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

    bool isOutdoor = bot->IsOutdoors();
    if (!isOutdoor)
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    if (bot->InArena())
        return false;

    if (!GET_PLAYERBOT_AI(bot)->HasStrategy("mount", BOT_STATE_NON_COMBAT) && !bot->IsMounted())
        return false;

    bool firstmount = bot->getLevel() >= 20;
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

    if (bot->GetPureSkillValue(SKILL_RIDING) <= 75 && bot->getLevel() < secondmount)
        masterSpeed = 59;

    if (bot->InBattleground() && masterSpeed > 99)
        masterSpeed = 99;

    bool hasSwiftMount = false;

    //std::map<int32, std::vector<uint32> > spells;
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
        //if (effect < masterSpeed)
            //continue;

        uint32 index = (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
            spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;

        if (index == 0 && std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) > 59)
            hasSwiftMount = true;

        if (index == 1 && std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) > 149)
            hasSwiftMount = true;

        allSpells[index][effect].push_back(spellId);
    }

    int32 masterMountType = 0;
    if (masterSpell)
    {
        masterMountType = (masterSpell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
            masterSpell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) ? 1 : 0;
    }

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

                if (masterMountType == 0 && masterSpeed > 59 && std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) < 99)
                    spells[59].clear();

                if (masterMountType == 1 && masterSpeed > 149 && std::max(spellInfo->Effects[EFFECT_1].BasePoints, spellInfo->Effects[EFFECT_2].BasePoints) < 279)
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

        botAI->CastSpell(ids[index], bot);
        return true;
    }

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", "mount");
    if (!items.empty())
        return UseItemAuto(*items.begin());

    return false;
}
