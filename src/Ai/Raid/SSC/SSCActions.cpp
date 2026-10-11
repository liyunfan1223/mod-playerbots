/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCActions.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "EncounterHelpers.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "SSCHelpers.h"

using namespace SerpentShrineCavernHelpers;
using namespace EncounterHelpers;

// General

bool SerpentShrineCavernEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const uint32 instanceId = bot->GetMap()->GetInstanceId();
    const ObjectGuid guid = bot->GetGUID();

    bool erased = false;

    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable") &&
        (hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0 ||
         hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0 ||
         hydrossNatureDpsWaitTimer.erase(instanceId) > 0 ||
         hydrossFrostDpsWaitTimer.erase(instanceId) > 0))
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "the lurker below") &&
        (lurkerRangedPositions.erase(guid) > 0 ||
         lurkerSpoutTimer.erase(instanceId) > 0))
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress") &&
        karathressDpsWaitTimer.erase(instanceId) > 0)
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "morogrim tidewalker") &&
        (tidewalkerTankStep.erase(guid) > 0 ||
         tidewalkerRangedStep.erase(guid) > 0))
    {
        erased = true;
    }

    if (!AI_VALUE2(Unit*, "find target", "lady vashj") &&
         hasReachedVashjRangedPosition.erase(guid) > 0)
    {
        erased = true;
    }

    return erased;
}

// Trash Mobs

// Move out of toxic pool left behind by some colossi upon death
bool UnderbogColossusEscapeToxicPoolAction::Execute(Event /*event*/)
{
    Aura* aura = bot->GetAura(SPELL_TOXIC_POOL);
    if (!aura)
        return false;

    DynamicObject* dynObj = aura->GetDynobjOwner();
    if (!dynObj)
        return false;

    float radius = dynObj->GetRadius();
    SpellInfo const* sInfo = sSpellMgr->GetSpellInfo(dynObj->GetSpellId());
    if (radius <= 0.0f && sInfo)
    {
        for (int e = 0; e < MAX_SPELL_EFFECTS; ++e)
        {
            auto const& eff = sInfo->Effects[e];
            if (eff.Effect == SPELL_EFFECT_SCHOOL_DAMAGE ||
                (eff.Effect == SPELL_EFFECT_APPLY_AURA &&
                 eff.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE))
            {
                radius = eff.CalcRadius();
                break;
            }
        }
    }

    if (radius <= 0.0f)
        return false;

    constexpr float bufferDist = 3.0f;
    constexpr float centerThreshold = 1.0f;

    float dx = bot->GetPositionX() - dynObj->GetPositionX();
    float dy = bot->GetPositionY() - dynObj->GetPositionY();

    float distToObj = bot->GetExactDist2d(dynObj->GetPositionX(), dynObj->GetPositionY());
    const float insideThresh = radius + centerThreshold;

    if (distToObj > insideThresh)
        return false;

    float safeDist = radius + bufferDist;
    float moveX, moveY;

    if (distToObj == 0.0f)
    {
        float angle = frand(0.0f, static_cast<float>(M_PI * 2.0));
        moveX = dynObj->GetPositionX() + std::cos(angle) * safeDist;
        moveY = dynObj->GetPositionY() + std::sin(angle) * safeDist;
    }
    else
    {
        float invDist = 1.0f / distToObj;
        moveX = dynObj->GetPositionX() + (dx * invDist) * safeDist;
        moveY = dynObj->GetPositionY() + (dy * invDist) * safeDist;
    }

    botAI->Reset();
    return MoveTo(SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event /*event*/)
{
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM);
    return totem && MarkTargetWithSkull(bot, totem);
}

// Hydross the Unstable <Duke of Currents>

// (1) When tanking, move to designated tanking spot on frost side
// (2) 1 second after 100% Mark of Hydross, move to nature tank's spot to hand off boss
// (3) When Hydross is in nature form, move back to frost tank spot and wait for transition
bool HydrossTheUnstablePositionFrostTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfHydrossAt100Percent(bot))
    {
        if (MarkTargetWithSquare(bot, hydross))
            return true;

        SetRtiTarget(botAI, "square");

        if (AI_VALUE(Unit*, "current target") != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
        {
            Position const& position = HYDROSS_FROST_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfHydrossAt100Percent(bot) &&
        hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
    {
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToNaturePhaseTimer.find(hydross->GetMap()->GetInstanceId());

        if (it != hydrossChangeToNaturePhaseTimer.end() && (now - it->second) >= 1)
        {
            Position const& position = HYDROSS_NATURE_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                botAI->Reset();
                return true;
            }
        }
    }

    Position const& position = HYDROSS_FROST_TANK_POSITION;
    if (hydross->HasAura(SPELL_CORRUPTION) &&
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                        position.GetPositionZ(), false, false, false, true,
                        MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// (1) When tanking, move to designated tanking spot on nature side
// (2) 1 second after 100% Mark of Corruption, move to frost tank's spot to hand off boss
// (3) When Hydross is in frost form, move back to nature tank spot and wait for transition
bool HydrossTheUnstablePositionNatureTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfCorruptionAt100Percent(bot))
    {
        if (MarkTargetWithTriangle(bot, hydross))
            return true;

        SetRtiTarget(botAI, "triangle");

        if (AI_VALUE(Unit*, "current target") != hydross)
            return Attack(hydross);

        if (hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
        {
            Position const& position = HYDROSS_NATURE_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfCorruptionAt100Percent(bot) &&
        hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
    {
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToFrostPhaseTimer.find(hydross->GetMap()->GetInstanceId());

        if (it != hydrossChangeToFrostPhaseTimer.end() && (now - it->second) >= 1)
        {
            Position const& position = HYDROSS_FROST_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                botAI->Reset();
                return true;
            }
        }
    }

    Position const& position = HYDROSS_NATURE_TANK_POSITION;
    if (!hydross->HasAura(SPELL_CORRUPTION) &&
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                        position.GetPositionZ(), false, false, false, true,
                        MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool HydrossTheUnstablePrioritizeElementalAddsAction::Execute(Event /*event*/)
{
    if (Unit* waterElemental = GetFirstAliveUnitByEntry(botAI, NPC_PURE_SPAWN_OF_HYDROSS))
    {
        if (MarkTargetWithSkull(bot, waterElemental))
            return true;

        SetRtiTarget(botAI, "skull");

        if (AI_VALUE(Unit*, "current target") != waterElemental)
            return Attack(waterElemental);
    }
    else if (Unit* natureElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_SPAWN_OF_HYDROSS))
    {
        if (MarkTargetWithSkull(bot, natureElemental))
            return true;

        SetRtiTarget(botAI, "skull");

        if (AI_VALUE(Unit*, "current target") != natureElemental)
            return Attack(natureElemental);
    }

    return false;
}

// To mitigate the effect of Water Tomb
bool HydrossTheUnstableFrostPhaseSpreadOutAction::Execute(Event /*event*/)
{
    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return false;

    constexpr float safeDistance = 6.0f;
    constexpr uint32 minInterval = 1000;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    return TryMisdirectToFrostTank(hydross) || TryMisdirectToNatureTank(hydross);
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToFrostTank(
    Unit* hydross)
{
    Player* frostTank = GetGroupMainTank(bot);
    if (!frostTank)
        return false;

    if (HasNoMarkOfHydross(bot) && !hydross->HasAura(SPELL_CORRUPTION))
    {
        if (botAI->CanCastSpell("misdirection", frostTank))
            return botAI->CastSpell("misdirection", frostTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToNatureTank(
    Unit* hydross)
{
    Player* natureTank = GetGroupAssistTank(bot, 0);
    if (!natureTank)
        return false;

    if (HasNoMarkOfCorruption(bot) && hydross->HasAura(SPELL_CORRUPTION))
    {
        if (botAI->CanCastSpell("misdirection", natureTank))
            return botAI->CastSpell("misdirection", natureTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableStopDpsUponPhaseChangeAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const uint32 instanceId = hydross->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    constexpr uint8 phaseStartStopSeconds = 5;
    constexpr uint8 phaseEndStopSeconds = 1;

    bool shouldStopDps = false;

    // 1 second after 100% Mark of Hydross, stop DPS
    auto itNature = hydrossChangeToNaturePhaseTimer.find(instanceId);
    if (itNature != hydrossChangeToNaturePhaseTimer.end() &&
        (now - itNature->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into nature phase
    auto itNatureDps = hydrossNatureDpsWaitTimer.find(instanceId);
    if (itNatureDps != hydrossNatureDpsWaitTimer.end() &&
        (now - itNatureDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    // 1 second after 100% Mark of Corruption, stop DPS
    auto itFrost = hydrossChangeToFrostPhaseTimer.find(instanceId);
    if (itFrost != hydrossChangeToFrostPhaseTimer.end() &&
        (now - itFrost->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into frost phase
    auto itFrostDps = hydrossFrostDpsWaitTimer.find(instanceId);
    if (itFrostDps != hydrossFrostDpsWaitTimer.end() &&
        (now - itFrostDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    if (shouldStopDps)
    {
        botAI->Reset();
        return true;
    }

    return false;
}

bool HydrossTheUnstableManageTimersAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const uint32 instanceId = hydross->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    bool changed = false;

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossFrostDpsWaitTimer.try_emplace(instanceId, now).second ||
            hydrossNatureDpsWaitTimer.erase(instanceId) > 0 ||
            hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0)
            changed = true;

        if (HasMarkOfHydrossAt100Percent(bot) &&
            hydrossChangeToNaturePhaseTimer.try_emplace(instanceId, now).second)
            changed = true;
    }
    else
    {
        if (hydrossNatureDpsWaitTimer.try_emplace(instanceId, now).second ||
            hydrossFrostDpsWaitTimer.erase(instanceId) > 0 ||
            hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0)
            changed = true;

        if (HasMarkOfCorruptionAt100Percent(bot) &&
            hydrossChangeToFrostPhaseTimer.try_emplace(instanceId, now).second)
            changed = true;
    }

    return changed;
}

// The Lurker Below

// Run around behind Lurker during Spout
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    float radius = frand(19.0f, 20.0f);
    float botAngle = std::atan2(
        bot->GetPositionY() - lurker->GetPositionY(), bot->GetPositionX() - lurker->GetPositionX());
    float relativeAngle = Position::NormalizeOrientation(botAngle - lurker->GetOrientation());
    constexpr float safeArc = M_PI / 2.0f;

    if (!PlayerbotAI::IsMainTank(bot) &&
        std::fabs(Position::NormalizeOrientation(relativeAngle - M_PI)) > safeArc / 2.0f)
    {
        float tangentAngle = botAngle + (relativeAngle > M_PI ? -0.1f : 0.1f);
        float moveX = lurker->GetPositionX() + radius * std::cos(tangentAngle);
        float moveY = lurker->GetPositionY() + radius * std::sin(tangentAngle);

        botAI->Reset();
        return MoveTo(SSC_MAP_ID, moveX, moveY, lurker->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        float behindAngle = lurker->GetOrientation() + M_PI + frand(-0.5f, 0.5f) * safeArc;
        float targetX = lurker->GetPositionX() + radius * std::cos(behindAngle);
        float targetY = lurker->GetPositionY() + radius * std::sin(behindAngle);

        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            botAI->Reset();
            return MoveTo(SSC_MAP_ID, targetX, targetY, lurker->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool TheLurkerBelowPositionMainTankAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (AI_VALUE(Unit*, "current target") != lurker)
        return Attack(lurker);

    Position const& position = LURKER_MAIN_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Assign ranged positions within a 120-degree arc behind Lurker
bool TheLurkerBelowSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    std::vector<Player*> rangedMembers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !PlayerbotAI::IsRanged(member))
                continue;

            rangedMembers.push_back(member);
        }
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = lurkerRangedPositions.find(guid);
    if (it == lurkerRangedPositions.end())
    {
        size_t count = rangedMembers.size();
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ?
            std::distance(rangedMembers.begin(), findIt) : 0;

        constexpr float arcSpan = 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 2.262f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        float angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        constexpr float radius = 27.0f;

        float targetX = lurker->GetPositionX() + radius * std::sin(angle);
        float targetY = lurker->GetPositionY() + radius * std::cos(angle);

        lurkerRangedPositions.try_emplace(guid, Position(targetX, targetY, lurker->GetPositionZ()));
        it = lurkerRangedPositions.find(guid);
    }

    if (it == lurkerRangedPositions.end())
        return false;

    Position const& position = it->second;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// During the submerge phase, if there are >= 3 tanks in the raid,
// the first 3 will each pick up 1 Guardian
bool TheLurkerBelowTanksPickUpAddsAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(bot, 1);
    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    std::vector<Unit*> guardians;
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() &&
            unit->GetEntry() == NPC_COILFANG_GUARDIAN)
        {
            guardians.push_back(unit);
        }
    }

    if (guardians.size() < 3)
        return false;

    std::vector<Player*> tanks = { mainTank, firstAssistTank, secondAssistTank };
    std::vector<uint8> rtiIndices =
    {
        RtiTargetValue::starIndex,
        RtiTargetValue::circleIndex,
        RtiTargetValue::diamondIndex
    };
    std::vector<std::string> rtiNames = { "star", "circle", "diamond" };

    for (size_t i = 0; i < 3; ++i)
    {
        Player* tank = tanks[i];
        Unit* guardian = guardians[i];
        if (bot == tank)
        {
            if (MarkTargetWithIcon(bot, guardian, rtiIndices[i]))
                return true;

            SetRtiTarget(botAI, rtiNames[i]);

            if (AI_VALUE(Unit*, "current target") != guardian)
                return Attack(guardian);
        }
    }

    return false;
}

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 instanceId = lurker->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    bool changed = false;

    auto it = lurkerSpoutTimer.find(instanceId);
    if (it != lurkerSpoutTimer.end() && it->second <= now)
    {
        lurkerSpoutTimer.erase(it);
        changed = true;
        it = lurkerSpoutTimer.end();
    }

    const time_t spoutCastTime = 20;
    if (IsLurkerCastingSpout(lurker) && it == lurkerSpoutTimer.end())
    {
        lurkerSpoutTimer.try_emplace(instanceId, now + spoutCastTime);
        changed = true;
    }

    return changed;
}

// Leotheras the Blind

bool LeotherasTheBlindTargetSpellbindersAction::Execute(Event /*event*/)
{
    Unit* spellbinder = GetFirstAliveUnitByEntry(botAI, NPC_GREYHEART_SPELLBINDER);
    if (spellbinder && MarkTargetWithSkull(bot, spellbinder))
        return true;

    return false;
}

// Warlock tank action--see GetLeotherasDemonFormTank in RaidSSCHelpers.cpp
// Use tank strategy for Demon Form and DPS strategy for Human Form
bool LeotherasTheBlindDemonFormTankAttackBossAction::Execute(Event /*event*/)
{
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    Unit* innerDemon = nullptr;
    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        if (creature && creature->GetEntry() == NPC_INNER_DEMON &&
            creature->GetSummonerGUID() == bot->GetGUID())
        {
            innerDemon = creature;
            break;
        }
    }

    if (innerDemon)
        return false;

    if (Unit* leotherasDemon = GetActiveLeotherasDemon(bot))
    {
        if (MarkTargetWithSquare(bot, leotherasDemon))
            return true;

        SetRtiTarget(botAI, "square");

        if (botAI->CanCastSpell("searing pain", leotherasDemon))
            return botAI->CastSpell("searing pain", leotherasDemon);
    }

    return false;
}

// Stop melee tanks from attacking upon transformation so they don't take aggro
// Applies only if there is a Warlock tank present
bool LeotherasTheBlindMeleeTanksDontAttackDemonFormAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    botAI->Reset();
    return true;
}

// Intent is to keep enough distance from Leotheras and spread to prepare for Whirlwind
// And stay away from the Warlock tank to avoid Chaos Blasts
bool LeotherasTheBlindPositionRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistFromBoss = 15.0f;
    Unit* leotherasHuman = GetLeotherasHuman(bot);
    if (leotherasHuman && bot->GetExactDist2d(leotherasHuman) < safeDistFromBoss &&
        leotherasHuman->GetVictim() != bot)
    {
        constexpr uint32 minInterval = 500;
        return FleePosition(leotherasHuman->GetPosition(), safeDistFromBoss, minInterval);
    }

    if (!GetActiveLeotherasDemon(bot))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            constexpr uint32 minInterval = 0;
            if (GetLeotherasDemonFormTank(bot) == member)
            {
                constexpr float safeDistFromMember = 10.0f;
                if (bot->GetExactDist2d(member) < safeDistFromMember)
                    return FleePosition(member->GetPosition(), safeDistFromMember, minInterval);
            }
            else
            {
                constexpr float safeDistFromMember = 6.0f;
                if (bot->GetExactDist2d(member) < safeDistFromMember)
                    return FleePosition(member->GetPosition(), safeDistFromMember, minInterval);
            }
        }
    }

    return false;
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    if (Unit* leotherasHuman = GetLeotherasHuman(bot))
    {
        float currentDistance = bot->GetExactDist2d(leotherasHuman);
        constexpr float safeDistance = 25.0f;
        if (currentDistance < safeDistance)
        {
            botAI->Reset();
            return MoveAway(leotherasHuman, safeDistance - currentDistance);
        }
    }

    return false;
}

// This method is likely unnecessary unless the player does not use a Warlock tank
// If a melee tank is used, other melee needs to run away after too many Chaos Blast stacks
bool LeotherasTheBlindMeleeDpsRunAwayFromBossAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("cloak of shadows", bot))
        return botAI->CastSpell("cloak of shadows", bot);

    Unit* leotheras = GetPhase2LeotherasDemon(bot);
    if (!leotheras)
        return false;

    Unit* demonVictim = leotheras->GetVictim();
    if (!demonVictim)
        return false;

    float currentDistance = bot->GetExactDist2d(demonVictim);
    constexpr float safeDistance = 10.0f;
    if (currentDistance < safeDistance && demonVictim != bot)
        return MoveAway(demonVictim, safeDistance - currentDistance);

    return false;
}

// Hardcoded actions for healers and bear tanks to kill Inner Demons
bool LeotherasTheBlindDestroyInnerDemonAction::Execute(Event /*event*/)
{
    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    Unit* innerDemon = nullptr;
    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        if (creature && creature->GetEntry() == NPC_INNER_DEMON &&
            creature->GetSummonerGUID() == bot->GetGUID())
        {
            innerDemon = creature;
            break;
        }
    }

    if (innerDemon)
    {
        if (PlayerbotAI::IsTank(bot) && bot->getClass() == CLASS_DRUID)
            return HandleFeralTankStrategy(innerDemon);

        if (PlayerbotAI::IsHeal(bot))
            return HandleHealerStrategy(innerDemon);

        // Roles without a strategy need to affirmatively attack their Inner Demons
        // Because DPS assist is disabled via multipliers
        if (AI_VALUE(Unit*, "current target") != innerDemon)
            return Attack(innerDemon);
    }

    return false;
}

// At 50% nerfed damage, bears have trouble killing their Inner Demons without a specific strategy
// Warrior and Paladin tanks have no trouble in my experience (Prot Warriors have high DPS, and
// Prot Paladins have an advantage in that Inner Demons are weak to Holy)
bool LeotherasTheBlindDestroyInnerDemonAction::HandleFeralTankStrategy(Unit* innerDemon)
{
    if (bot->HasAura(SPELL_DIRE_BEAR_FORM))
        bot->RemoveAura(SPELL_DIRE_BEAR_FORM);

    if (bot->HasAura(SPELL_BEAR_FORM))
        bot->RemoveAura(SPELL_BEAR_FORM);

    bool casted = false;

    if (!bot->HasAura(SPELL_CAT_FORM) &&
        botAI->CanCastSpell("cat form", bot) &&
        botAI->CastSpell("cat form", bot))
        casted = true;

    if (botAI->CanCastSpell("berserk", bot) &&
        botAI->CastSpell("berserk", bot))
        casted = true;

    if (bot->GetPower(POWER_ENERGY) < 30 &&
        botAI->CanCastSpell("tiger's fury", bot) &&
        botAI->CastSpell("tiger's fury", bot))
        casted = true;

    if (bot->GetComboPoints() >= 4 &&
        botAI->CanCastSpell("ferocious bite", innerDemon) &&
        botAI->CastSpell("ferocious bite", innerDemon))
        casted = true;

    if (bot->GetComboPoints() == 0 && innerDemon->GetHealthPct() > 25.0f &&
        botAI->CanCastSpell("rake", innerDemon) &&
        botAI->CastSpell("rake", innerDemon))
        casted = true;

    if (botAI->CanCastSpell("mangle (cat)", innerDemon) &&
        botAI->CastSpell("mangle (cat)", innerDemon))
        casted = true;

    return casted;
}

bool LeotherasTheBlindDestroyInnerDemonAction::HandleHealerStrategy(Unit* innerDemon)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        if (bot->HasAura(SPELL_TREE_OF_LIFE))
            bot->RemoveAura(SPELL_TREE_OF_LIFE);

        bool casted = false;

        if (botAI->CanCastSpell("barkskin", bot) &&
            botAI->CastSpell("barkskin", bot))
            casted = true;

        if (botAI->CanCastSpell("wrath", innerDemon) &&
            botAI->CastSpell("wrath", innerDemon))
            casted = true;

        return casted;
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        bool casted = false;

        if (botAI->CanCastSpell("avenging wrath", bot) &&
            botAI->CastSpell("avenging wrath", bot))
            casted = true;

        if (botAI->CanCastSpell("consecration", bot) &&
            botAI->CastSpell("consecration", bot))
            casted = true;

        if (botAI->CanCastSpell("exorcism", innerDemon) &&
            botAI->CastSpell("exorcism", innerDemon))
            casted = true;

        if (botAI->CanCastSpell("hammer of wrath", innerDemon) &&
            botAI->CastSpell("hammer of wrath", innerDemon))
            casted = true;

        if (botAI->CanCastSpell("holy shock", innerDemon) &&
            botAI->CastSpell("holy shock", innerDemon))
            casted = true;

        if (botAI->CanCastSpell("judgement of light", innerDemon) &&
            botAI->CastSpell("judgement of light", innerDemon))
            casted = true;

        return casted;
    }
    else if (bot->getClass() == CLASS_PRIEST)
    {
        if (botAI->CanCastSpell("smite", innerDemon))
            return botAI->CastSpell("smite", innerDemon);
    }
    else if (bot->getClass() == CLASS_SHAMAN)
    {
        bool casted = false;

        if (botAI->CanCastSpell("earth shock", innerDemon) &&
            botAI->CastSpell("earth shock", innerDemon))
            casted = true;

        if (botAI->CanCastSpell("chain lightning", innerDemon) &&
            botAI->CastSpell("chain lightning", innerDemon))
            casted = true;

        if (botAI->CanCastSpell("lightning bolt", innerDemon) &&
            botAI->CastSpell("lightning bolt", innerDemon))
            casted = true;

        return casted;
    }

    return false;
}

// Everybody except the Warlock tank should focus on Leotheras in Phase 3
bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* leotherasHuman = GetLeotherasHuman(bot);
    if (!leotherasHuman)
        return false;

    if (MarkTargetWithStar(bot, leotherasHuman))
        return true;

    SetRtiTarget(botAI, "star");

    if (AI_VALUE(Unit*, "current target") != leotherasHuman)
        return Attack(leotherasHuman);

    Unit* leotherasDemon = GetPhase3LeotherasDemon(bot);
    if (!leotherasDemon)
        return false;

    if (leotherasHuman->GetVictim() != bot)
        return false;

    Unit* demonTarget = leotherasDemon->GetVictim();
    if (!demonTarget)
        return false;

    constexpr float safeDistanceFromDemon = 20.0f;
    if (leotherasHuman->GetExactDist2d(demonTarget) < safeDistanceFromDemon)
    {
        float angle = atan2(bot->GetPositionY() - demonTarget->GetPositionY(),
                            bot->GetPositionX() - demonTarget->GetPositionX());
        float targetX = bot->GetPositionX() + safeDistanceFromDemon * std::cos(angle);
        float targetY = bot->GetPositionY() + safeDistanceFromDemon * std::sin(angle);

        return MoveTo(SSC_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Misdirect to Warlock tank or to main tank if there is no Warlock tank
bool LeotherasTheBlindMisdirectBossToDemonFormTankAction::Execute(Event /*event*/)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(bot);
    if (!leotherasDemon)
        return false;

    Player* targetTank = GetLeotherasDemonFormTank(bot);
    if (!targetTank)
        targetTank = GetGroupMainTank(bot);

    if (!targetTank)
        return false;

    if (botAI->CanCastSpell("misdirection", targetTank))
        return botAI->CastSpell("misdirection", targetTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", leotherasDemon))
        return botAI->CastSpell("steady shot", leotherasDemon);

    return false;
}

// This does not pause DPS after a Whirlwind, which is also an aggro wipe
bool LeotherasTheBlindManageDpsWaitTimersAction::Execute(Event /*event*/)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    const uint32 instanceId = leotheras->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    bool changed = false;
    // Encounter start/reset: clear all timers
    if (leotheras->HasAura(SPELL_LEOTHERAS_BANISHED) &&
        (leotherasHumanFormDpsWaitTimer.erase(instanceId) > 0 ||
         leotherasDemonFormDpsWaitTimer.erase(instanceId) > 0 ||
         leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0))
    {
        changed = true;
    }

    // Human Phase
    Unit* leotherasHuman = GetLeotherasHuman(bot);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(bot);
    if (leotherasHuman && !leotherasPhase3Demon &&
        (leotherasHumanFormDpsWaitTimer.try_emplace(instanceId, now).second ||
         leotherasDemonFormDpsWaitTimer.erase(instanceId) > 0))
    {
        changed = true;
    }
    // Demon Phase
    else if (GetPhase2LeotherasDemon(bot) &&
             (leotherasDemonFormDpsWaitTimer.try_emplace(instanceId, now).second ||
              leotherasHumanFormDpsWaitTimer.erase(instanceId) > 0))
    {
        changed = true;
    }
    // Final Phase (<15% HP)
    else if (leotherasHuman && leotherasPhase3Demon &&
             (leotherasFinalPhaseDpsWaitTimer.try_emplace(instanceId, now).second ||
              leotherasHumanFormDpsWaitTimer.erase(instanceId) > 0 ||
              leotherasDemonFormDpsWaitTimer.erase(instanceId) > 0))
    {
        changed = true;
    }

    return changed;
}

// Fathom-Lord Karathress
// Note: 4 tanks are required for the full strategy, and having at least 2
// is crucial to separate Caribdis from the others

// Karathress is tanked near his starting position
bool FathomLordKarathressMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    if (MarkTargetWithTriangle(bot, karathress))
        return true;

    SetRtiTarget(botAI, "triangle");

    if (AI_VALUE(Unit*, "current target") != karathress)
        return Attack(karathress);

    if (karathress->GetVictim() == bot && bot->IsWithinMeleeRange(karathress))
    {
        Position const& position = KARATHRESS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Caribdis is pulled far to the West in the corner
// Best to use a Warrior or Druid tank for interrupts
bool FathomLordKarathressFirstAssistTankPositionCaribdisAction::Execute(Event /*event*/)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    if (MarkTargetWithDiamond(bot, caribdis))
        return true;

    SetRtiTarget(botAI, "diamond");

    if (AI_VALUE(Unit*, "current target") != caribdis)
        return Attack(caribdis);

    if (caribdis->GetVictim() == bot)
    {
        Position const& position = CARIBDIS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Sharkkis is pulled North to the other side of the ramp
bool FathomLordKarathressSecondAssistTankPositionSharkkisAction::Execute(Event /*event*/)
{
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (!sharkkis)
        return false;

    if (MarkTargetWithStar(bot, sharkkis))
        return true;

    SetRtiTarget(botAI, "star");

    if (AI_VALUE(Unit*, "current target") != sharkkis)
        return Attack(sharkkis);

    if (sharkkis->GetVictim() == bot && bot->IsWithinMeleeRange(sharkkis))
    {
        Position const& position = SHARKKIS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Tidalvess is pulled Northwest near the pillar
bool FathomLordKarathressThirdAssistTankPositionTidalvessAction::Execute(Event /*event*/)
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (!tidalvess)
        return false;

    if (MarkTargetWithCircle(bot, tidalvess))
        return true;

    SetRtiTarget(botAI, "circle");

    if (AI_VALUE(Unit*, "current target") != tidalvess)
        return Attack(tidalvess);

    if (tidalvess->GetVictim() == bot && bot->IsWithinMeleeRange(tidalvess))
    {
        Position const& position = TIDALVESS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Caribdis's tank spot is far away so a dedicated healer is needed
// Use the assistant flag to select the healer
bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event /*event*/)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    Position const& position = CARIBDIS_HEALER_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 3.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank
bool FathomLordKarathressMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
            hunters.push_back(member);

        if (hunters.size() >= 3)
            break;
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        tankTarget = GetGroupAssistTank(bot, 0);
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        tankTarget = GetGroupAssistTank(bot, 2);
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
        tankTarget = GetGroupAssistTank(bot, 1);
    }

    if (!bossTarget || !tankTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

// Kill order is non-standard because bots handle Cyclones poorly and need more time
// to get her down than real players (standard is ranged DPS help with Sharkkis first)
bool FathomLordKarathressAssignDpsPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Spitfire Totems for melee dps
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
    if (totem && PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
    {
        if (MarkTargetWithSkull(bot, totem))
            return true;

        SetRtiTarget(botAI, "skull");

        if (AI_VALUE(Unit*, "current target") != totem)
            return Attack(totem);

        // Direct movement order due to path between Sharkkis and totem sometimes being screwy
        if (!bot->IsWithinMeleeRange(totem))
        {
            return MoveTo(SSC_MAP_ID, totem->GetPositionX(), totem->GetPositionY(),
                          totem->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        return false;
    }

    // Target priority 2: Tidalvess for all dps
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess)
    {
        if (MarkTargetWithCircle(bot, tidalvess))
            return true;

        SetRtiTarget(botAI, "circle");

        if (AI_VALUE(Unit*, "current target") != tidalvess)
            return Attack(tidalvess);

        return false;
    }

    // Target priority 3: Caribdis for ranged dps
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (PlayerbotAI::IsRangedDps(bot) && caribdis)
    {
        if (MarkTargetWithDiamond(bot, caribdis))
            return true;

        SetRtiTarget(botAI, "diamond");

        Position const& position = CARIBDIS_RANGED_DPS_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveInside(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                              position.GetPositionZ(), 8.0f, MovementPriority::MOVEMENT_COMBAT);
        }

        if (AI_VALUE(Unit*, "current target") != caribdis)
            return Attack(caribdis);

        return false;
    }

    // Target priority 4: Sharkkis for melee dps and, after Caribdis is down, ranged dps also
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis)
    {
        if (MarkTargetWithStar(bot, sharkkis))
            return true;

        SetRtiTarget(botAI, "star");

        if (AI_VALUE(Unit*, "current target") != sharkkis)
            return Attack(sharkkis);

        return false;
    }

    // Target priority 5: Sharkkis pets for all dps
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (fathomSporebat && PlayerbotAI::IsMelee(bot))
    {
        if (MarkTargetWithCross(bot, fathomSporebat))
            return true;

        SetRtiTarget(botAI, "cross");

        if (AI_VALUE(Unit*, "current target") != fathomSporebat)
            return Attack(fathomSporebat);

        return false;
    }

    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomLurker && PlayerbotAI::IsMelee(bot))
    {
        if (MarkTargetWithSquare(bot, fathomLurker))
            return true;

        SetRtiTarget(botAI, "square");

        if (AI_VALUE(Unit*, "current target") != fathomLurker)
            return Attack(fathomLurker);

        return false;
    }

    // Target priority 6: Karathress for all dps
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress)
    {
        if (MarkTargetWithTriangle(bot, karathress))
            return true;

        SetRtiTarget(botAI, "triangle");

        if (AI_VALUE(Unit*, "current target") != karathress)
            return Attack(karathress);
    }

    return false;
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress && karathressDpsWaitTimer.try_emplace(
        karathress->GetMap()->GetInstanceId(), std::time(nullptr)).second)
        return true;

    return false;
}

// Morogrim Tidewalker

bool MorogrimTidewalkerMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", tidewalker))
        return botAI->CastSpell("steady shot", tidewalker);

    return false;
}

// Separate tanking positions are used for phase 1 and phase 2 to address the
// Water Globule mechanic in phase 2
bool MorogrimTidewalkerMoveBossToTankPositionAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    if (AI_VALUE(Unit*, "current target") != tidewalker)
        return Attack(tidewalker);

    if (tidewalker->GetVictim() == bot && bot->IsWithinMeleeRange(tidewalker))
    {
        if (tidewalker->GetHealthPct() > 26.0f)
            return MoveToPhase1TankPosition();
        else
            return MoveToPhase2TankPosition();
    }

    return false;
}

// Phase 1: tank position is up against the Northeast pillar
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase1TankPosition()
{
    Position const& phase1 = TIDEWALKER_PHASE_1_TANK_POSITION;
    float distToPhase1 = bot->GetExactDist2d(phase1.GetPositionX(), phase1.GetPositionY());
    if (distToPhase1 > 1.0f)
    {
        float dX = phase1.GetPositionX() - bot->GetPositionX();
        float dY = phase1.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToPhase1);
        float moveX = bot->GetPositionX() + (dX / distToPhase1) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPhase1) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, phase1.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

// Phase 2: move in two steps to get around the pillar and back up into the Northeast corner
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase2TankPosition()
{
    Position const& phase2 = TIDEWALKER_PHASE_2_TANK_POSITION;
    Position const& transition = TIDEWALKER_PHASE_TRANSITION_WAYPOINT;

    auto itStep = tidewalkerTankStep.find(bot->GetGUID());
    uint8 step = (itStep != tidewalkerTankStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        float distToTransition =
            bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY());

        if (distToTransition > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToTransition);
            float moveX = bot->GetPositionX() + (dX / distToTransition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToTransition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else
            tidewalkerTankStep.try_emplace(bot->GetGUID(), 1);
    }

    if (step == 1)
    {
        float distToPhase2 =
            bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY());

        if (distToPhase2 > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPhase2);
            float moveX = bot->GetPositionX() + (dX / distToPhase2) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPhase2) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Ranged stack behind the boss in the Northeast corner in phase 2
// No corresponding method for melee since they will do so anyway
bool MorogrimTidewalkerPhase2RepositionRangedAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    Position const& phase2 = TIDEWALKER_PHASE_2_RANGED_POSITION;
    Position const& transition = TIDEWALKER_PHASE_TRANSITION_WAYPOINT;

    auto itStep = tidewalkerRangedStep.find(bot->GetGUID());
    uint8 step = (itStep != tidewalkerRangedStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        float distToTransition =
            bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY());

        if (distToTransition > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToTransition);
            float moveX = bot->GetPositionX() + (dX / distToTransition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToTransition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            tidewalkerRangedStep.try_emplace(bot->GetGUID(), 1);
            step = 1;
        }
    }

    if (step == 1)
    {
        float distToPhase2 =
            bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY());

        if (distToPhase2 > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPhase2);
            float moveX = bot->GetPositionX() + (dX / distToPhase2) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPhase2) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (AI_VALUE(Unit*, "current target") != vashj)
        return Attack(vashj);

    if (vashj->GetVictim() == bot && bot->IsWithinMeleeRange(vashj))
    {
        // Phase 1: Position Vashj in the center of the platform
        if (IsLadyVashjInPhase1(botAI))
        {
            Position const& position = VASHJ_PLATFORM_CENTER_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
        // Phase 3: No fixed position, but move Vashj away from Enchanted Elementals
        else if (IsLadyVashjInPhase3(botAI))
        {
            if (Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental"))
            {
                float currentDistance = bot->GetExactDist2d(enchanted);
                constexpr float safeDistance = 10.0f;
                if (currentDistance < safeDistance)
                    return MoveAway(enchanted, safeDistance - currentDistance);
            }
        }
    }

    return false;
}

// Semicircle around center of the room (to allow escape paths by Static Charged bots)
bool LadyVashjPhase1SpreadRangedInArcAction::Execute(Event /*event*/)
{
    std::vector<Player*> spreadMembers;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && GET_PLAYERBOT_AI(member) && PlayerbotAI::IsRanged(member))
            spreadMembers.push_back(member);
    }

    const ObjectGuid guid = bot->GetGUID();
    auto itReached = hasReachedVashjRangedPosition.find(guid);

    auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
    size_t botIndex = (it != spreadMembers.end()) ?
        std::distance(spreadMembers.begin(), it) : 0;
    size_t count = spreadMembers.size();
    if (count == 0)
        return false;

    constexpr float arcCenter = M_PI / 2.0f; // North
    constexpr float arcSpan = M_PI; // 180°
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    float angle;
    if (count == 1)
        angle = arcCenter;
    else
        angle = arcStart + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

    Position const& center = VASHJ_PLATFORM_CENTER_POSITION;
    float radius = 25.0f;
    float targetX = center.GetPositionX() + radius * std::cos(angle);
    float targetY = center.GetPositionY() + radius * std::sin(angle);
    float targetZ = center.GetPositionZ();

    if (itReached == hasReachedVashjRangedPosition.end() || !(itReached->second))
    {
        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            hasReachedVashjRangedPosition.try_emplace(guid, false);
            return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        hasReachedVashjRangedPosition[guid] = true;
    }

    return false;
}

// For absorbing Shock Burst
bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event /*event*/)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (mainTank->HasAura(SPELL_GROUNDING_TOTEM_EFFECT))
        return false;

    constexpr float distFromTank = 25.0f;
    if (bot->GetDistance(mainTank) > distFromTank)
        return MoveTo(mainTank, distFromTank, MovementPriority::MOVEMENT_COMBAT);

    return botAI->CanCastSpell("grounding totem", bot) &&
           botAI->CastSpell("grounding totem", bot);
}

bool LadyVashjMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", vashj))
        return botAI->CastSpell("steady shot", vashj);

    return false;
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // If the main tank has Static Charge, other group members should move away
    Player* mainTank = GetGroupMainTank(bot);
    if (mainTank && bot != mainTank && mainTank->HasAura(SPELL_STATIC_CHARGE))
    {
        float currentDistance = bot->GetExactDist2d(mainTank);
        constexpr float safeDistance = 11.0f;
        if (currentDistance < safeDistance)
            return MoveAway(mainTank, safeDistance - currentDistance);
    }

    // If any other bot has Static Charge, it should move away from other group members
    if (!PlayerbotAI::IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            float currentDistance = bot->GetExactDist2d(member);
            constexpr float safeDistance = 11.0f;
            if (currentDistance < safeDistance)
                return MoveFromGroup(safeDistance);
        }
    }

    return false;
}

bool LadyVashjAssignPhase2AndPhase3DpsPriorityAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Position const& center = VASHJ_PLATFORM_CENTER_POSITION;
    float platformZ = center.GetPositionZ();
    if (bot->GetPositionZ() - platformZ > 2.0f)
    {
        // This block is needed to prevent bots from floating into the air to attack sporebats
        bot->AttackStop();
        bot->CastStop();
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->NearTeleportTo(bot->GetPositionX(), bot->GetPositionY(),
                            platformZ, bot->GetOrientation());
        return true;
    }

    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    Unit* target = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    // Search and attack radius are intended to keep bots from going down the stairs
    const float maxSearchRange =
        PlayerbotAI::IsRanged(bot) ? 60.0f : 55.0f;
    const float maxPursueRange = maxSearchRange - 5.0f;

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsValidLadyVashjCombatNpc(unit, botAI))
            continue;

        float distFromCenter = unit->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
        if (IsLadyVashjInPhase2(botAI) && distFromCenter > maxSearchRange)
            continue;

        switch (unit->GetEntry())
        {
            case NPC_ENCHANTED_ELEMENTAL:
                if (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted))
                    enchanted = unit;
                break;

            case NPC_COILFANG_ELITE:
                if (!elite || unit->GetHealthPct() < elite->GetHealthPct())
                    elite = unit;
                break;

            case NPC_COILFANG_STRIDER:
                if (!strider || unit->GetHealthPct() < strider->GetHealthPct())
                    strider = unit;
                break;

            case NPC_TOXIC_SPOREBAT:
                if (!sporebat || bot->GetDistance(unit) < bot->GetDistance(sporebat))
                    sporebat = unit;
                break;

            case NPC_LADY_VASHJ:
                vashj = unit;
                break;

            default:
                break;
        }
    }

    std::vector<Unit*> targets;
    if (IsLadyVashjInPhase2(botAI))
    {
        if (PlayerbotAI::IsRanged(bot))
        {
            // Hunters and Mages prioritize Enchanted Elementals,
            // while other ranged DPS prioritize Striders
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { enchanted, strider, elite };
            else
                targets = { strider, elite, enchanted };
        }
        else if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            targets = { enchanted, elite };
        else if (PlayerbotAI::IsTank(bot))
        {
            if (botAI->HasCheat(BotCheatMask::raid) && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
                targets = { strider, elite, enchanted };
            else
                targets = { elite, strider, enchanted };
        }
        else
            targets = { enchanted, elite, strider };
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (PlayerbotAI::IsTank(bot))
        {
            if (PlayerbotAI::IsMainTank(bot))
            {
                if (MarkTargetWithDiamond(bot, vashj))
                    return true;

                SetRtiTarget(botAI, "diamond");
                targets = { vashj };
            }
            else if (botAI->HasCheat(BotCheatMask::raid) &&
                     PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
            {
                targets = { strider, elite, enchanted, vashj };
            }
            else
                targets = { elite, strider, enchanted, vashj };
        }
        else if (PlayerbotAI::IsRanged(bot))
        {
            // Hunters are assigned to kill Sporebats in Phase 3
            if (bot->getClass() == CLASS_HUNTER)
                targets = { sporebat, enchanted, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        else if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    for (Unit* candidate : targets)
    {
        if (candidate && bot->GetExactDist2d(candidate) <= maxPursueRange)
        {
            target = candidate;
            break;
        }
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();

    if (currentTarget && !IsValidLadyVashjCombatNpc(currentTarget, botAI))
    {
        bot->AttackStop();
        bot->CastStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
        currentTarget = nullptr;
    }

    if (target && currentTarget != target &&
        AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    // If bots have wandered too far from the center, move them back
    if (bot->GetExactDist2d(vashj) > maxPursueRange)
        return MoveTo(vashj, maxPursueRange - 10.0f, MovementPriority::MOVEMENT_FORCED);

    return false;
}

bool LadyVashjMisdirectStriderToFirstAssistTankAction::Execute(Event /*event*/)
{
    // Striders are not tankable without a cheat to block Fear so there is
    // no point in misdirecting if raid cheats are not enabled
    if (!botAI->HasCheat(BotCheatMask::raid))
        return false;

    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    if (!strider)
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    if (!firstAssistTank || strider->GetVictim() == firstAssistTank)
        return false;

    if (botAI->CanCastSpell("misdirection", firstAssistTank))
        return botAI->CastSpell("misdirection", firstAssistTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", strider))
        return botAI->CastSpell("steady shot", strider);

    return false;
}

bool LadyVashjTankAttackAndMoveAwayStriderAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Unit* strider = AI_VALUE2(Unit*, "find target", "coilfang strider");
    if (!strider)
        return false;

    // Raid cheat automatically applies Fear Ward to tanks to make Strider tankable
    // This simulates the real-life strategy where the Strider can be meleed by
    // players wearing an Ogre Suit (due to the extended combat reach)
    if (botAI->HasCheat(BotCheatMask::raid) && PlayerbotAI::IsTank(bot))
    {
        if (!bot->HasAura(SPELL_FEAR_WARD))
            bot->AddAura(SPELL_FEAR_WARD, bot);

        if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
            AI_VALUE(Unit*, "current target") != strider)
            return Attack(strider);

        float currentDistance = bot->GetExactDist2d(vashj);
        constexpr float safeDistance = 28.0f;
        if (strider->GetVictim() == bot && currentDistance < safeDistance)
            return MoveAway(vashj, safeDistance - currentDistance);

        return false;
    }

    // Don't move away if raid cheats are enabled, or in any case if the bot is a tank
    if (!botAI->HasCheat(BotCheatMask::raid))
    {
        float currentDistance = bot->GetExactDist2d(strider);
        constexpr float safeDistance = 20.0f;
        if (!PlayerbotAI::IsTank(bot) && currentDistance < safeDistance)
            return MoveAway(strider, safeDistance - currentDistance);

        // Try to root/slow the Strider if it is not tankable (poor man's kiting strategy)
        if (!botAI->HasAura("frost shock", strider) && bot->getClass() == CLASS_SHAMAN &&
            botAI->CanCastSpell("frost shock", strider))
        {
            return botAI->CastSpell("frost shock", strider);
        }
        else if (!strider->HasAura(SPELL_CURSE_OF_EXHAUSTION) && bot->getClass() == CLASS_WARLOCK &&
                 botAI->CanCastSpell("curse of exhaustion", strider))
        {
            return botAI->CastSpell("curse of exhaustion", strider);
        }
        else if (!strider->HasAura(SPELL_SLOW) && bot->getClass() == CLASS_MAGE &&
                 botAI->CanCastSpell("slow", strider))
        {
            return botAI->CastSpell("slow", strider);
        }
    }

    return false;
}

// If cheats are enabled, the first returned melee DPS bot will teleport to Tainted Elementals
// Such bot will recover HP and remove the Poison Bolt debuff while attacking the elemental
bool LadyVashjTeleportToTaintedElementalAction::Execute(Event /*event*/)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    if (bot->GetExactDist2d(tainted) > 10.0f)
    {
        bot->AttackStop();
        bot->CastStop();
        bot->NearTeleportTo(tainted->GetPositionX(), tainted->GetPositionY(),
                            tainted->GetPositionZ(), tainted->GetOrientation());
    }

    if (AI_VALUE(Unit*, "current target") != tainted)
    {
        if (MarkTargetWithStar(bot, tainted))
            return true;

        SetRtiTarget(botAI, "star");
        return Attack(tainted);
    }

    if (bot->GetExactDist2d(tainted) < 5.0f)
    {
        bot->SetFullHealth();
        bot->RemoveAura(SPELL_POISON_BOLT);
    }

    return false;
}

bool LadyVashjLootTaintedCoreAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
            return false;
    }

    constexpr float searchRadius = 150.0f;
    Creature* elemental = bot->FindNearestCreature(NPC_TAINTED_ELEMENTAL, searchRadius, false);

    if (!elemental || elemental->IsAlive())
        return false;

    LootObject loot(bot, elemental->GetGUID());
    if (!loot.IsLootPossible(bot))
        return false;

    context->GetValue<LootObject>("loot target")->Set(loot);

    const float maxLootRange = sPlayerbotAIConfig.LootDistance;
    constexpr float distFromObject = 2.0f;

    if (bot->GetDistance(elemental) > maxLootRange)
        return MoveTo(elemental, distFromObject, MovementPriority::MOVEMENT_FORCED);

    OpenLootAction open(botAI);
    if (!open.Execute(Event()))
        return false;

    bot->SetLootGUID(elemental->GetGUID());
    constexpr uint8 coreIndex = 0;
    WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
    *packet << coreIndex;
    bot->GetSession()->QueuePacket(packet);

    const time_t now = std::time(nullptr);
    lastCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);

    return true;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(botAI, bot);
    Player* firstCorePasser = GetFirstTaintedCorePasser(botAI, bot);
    Player* secondCorePasser = GetSecondTaintedCorePasser(botAI, bot);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(botAI, bot);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(botAI, bot);

    const uint32 instanceId = vashj->GetMap()->GetInstanceId();

    Unit* closestTrigger = nullptr;
    if (Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
        (closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(tainted)))
    {
        nearestTriggerGuid.try_emplace(instanceId, closestTrigger->GetGUID());
    }

    auto itSnap = nearestTriggerGuid.find(instanceId);
    if (itSnap != nearestTriggerGuid.end() && !itSnap->second.IsEmpty())
    {
        if (Unit* snapUnit = botAI->GetUnit(itSnap->second))
            closestTrigger = snapUnit;
        else
            nearestTriggerGuid.erase(instanceId);
    }

    if (!closestTrigger)
        return false;

    // Not gated behind CheatMask because the auto application of Fear Ward is necessary
    // to address an issue with bot movement, which is that bots cannot be rooted and
    // therefore will move when feared while holding the Tainted Core
    if (!bot->HasAura(SPELL_FEAR_WARD))
        bot->AddAura(SPELL_FEAR_WARD, bot);

    Item* item = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (!item || !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // Passer order: HealAssistantOfIndex 0, 1, 2, then RangedDpsAssistantOfIndex 0
        if (bot == firstCorePasser &&
            LineUpFirstCorePasser(designatedLooter))
        {
            return true;
        }
        else if (bot == secondCorePasser &&
                 LineUpSecondCorePasser(firstCorePasser, closestTrigger))
        {
            return true;
        }
        else if (bot == thirdCorePasser && LineUpThirdCorePasser(
                 designatedLooter, firstCorePasser, secondCorePasser, closestTrigger))
        {
            return true;
        }
        else if (bot == fourthCorePasser && LineUpFourthCorePasser(
                 firstCorePasser, secondCorePasser, thirdCorePasser, closestTrigger))
        {
            return true;
        }
    }
    else if (item && botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // Designated core looter logic
        // Applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter &&
            IsFirstCorePasserInPosition(firstCorePasser))
        {
            const time_t now = std::time(nullptr);
            auto it = lastImbueAttempt.find(instanceId);
            if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
            {
                lastImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, firstCorePasser);
                lastCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                ScheduleTransferCoreAfterImbue(botAI, bot, firstCorePasser);
                return true;
            }
        }
        // First core passer: receive core from looter at the top of the stairs,
        // pass to second core passer
        else if (bot == firstCorePasser &&
                 IsSecondCorePasserInPosition(secondCorePasser))
        {
            const time_t now = std::time(nullptr);
            auto it = lastImbueAttempt.find(instanceId);
            if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
            {
                lastImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, secondCorePasser);
                lastCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                ScheduleTransferCoreAfterImbue(botAI, bot, secondCorePasser);
                return true;
            }
        }
        // Second core passer: if closest usable generator is within passing distance
        // of the first passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == secondCorePasser && !UseCoreOnNearestGenerator(instanceId) &&
                 IsThirdCorePasserInPosition(thirdCorePasser))
        {
            const time_t now = std::time(nullptr);
            auto it = lastImbueAttempt.find(instanceId);
            if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
            {
                lastImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, thirdCorePasser);
                lastCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                ScheduleTransferCoreAfterImbue(botAI, bot, thirdCorePasser);
                return true;
            }
        }
        // Third core passer: if closest usable generator is within passing distance
        // of the second passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == thirdCorePasser && !UseCoreOnNearestGenerator(instanceId) &&
                 IsFourthCorePasserInPosition(fourthCorePasser))
        {
            const time_t now = std::time(nullptr);
            auto it = lastImbueAttempt.find(instanceId);
            if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
            {
                lastImbueAttempt.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, fourthCorePasser);
                lastCoreInInventoryTime.insert_or_assign(bot->GetGUID(), now);
                ScheduleTransferCoreAfterImbue(botAI, bot, fourthCorePasser);
                return true;
            }
        }
        // Fourth core passer: the fourth passer is rarely needed and no more than
        // four ever should be, so it should use the Core on the nearest generator
        else if (bot == fourthCorePasser && UseCoreOnNearestGenerator(instanceId))
            return true;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(
    Player* designatedLooter)
{
    const float centerX = VASHJ_PLATFORM_CENTER_POSITION.GetPositionX();
    const float centerY = VASHJ_PLATFORM_CENTER_POSITION.GetPositionY();
    constexpr float radius = 57.5f;

    auto it = intendedLineup.find(bot->GetGUID());
    if (it == intendedLineup.end())
    {
        float mx = designatedLooter->GetPositionX();
        float my = designatedLooter->GetPositionY();
        float angle = atan2(my - centerY, mx - centerX);

        float targetX = centerX + radius * std::cos(angle);
        float targetY = centerY + radius * std::sin(angle);
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;

        intendedLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        it = intendedLineup.find(bot->GetGUID());
    }

    Position const& pos = it->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->AttackStop();
    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(
    Player* firstCorePasser, Unit* closestTrigger)
{
    auto itFirst = intendedLineup.find(firstCorePasser->GetGUID());
    if (itFirst == intendedLineup.end())
        return false;

    auto itSecond = intendedLineup.find(bot->GetGUID());
    if (itSecond == intendedLineup.end())
    {
        float fx = itFirst->second.GetPositionX();
        float fy = itFirst->second.GetPositionY();

        float dx = closestTrigger->GetPositionX() - fx;
        float dy = closestTrigger->GetPositionY() - fy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        float targetX, targetY;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;
        constexpr float thresholdDist = 40.0f;
        constexpr float nearTriggerDist = 1.5f;
        constexpr float farDistance = 38.0f;

        if (distToTrigger <= thresholdDist)
        {
            float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
            targetX = fx + dx * moveDist;
            targetY = fy + dy * moveDist;
        }
        else
        {
            targetX = fx + dx * farDistance;
            targetY = fy + dy * farDistance;
        }

        intendedLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        itSecond = intendedLineup.find(bot->GetGUID());
    }

    Position const& pos = itSecond->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->AttackStop();
    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(
    Player*, Player* firstCorePasser,
    Player* secondCorePasser, Unit* closestTrigger)
{
    bool needThirdPasser =
        (IsFirstCorePasserInPosition(firstCorePasser) &&
         firstCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsSecondCorePasserInPosition(secondCorePasser) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needThirdPasser)
        return false;

    auto itSecond = intendedLineup.find(secondCorePasser->GetGUID());
    if (itSecond == intendedLineup.end())
        return false;

    auto itThird = intendedLineup.find(bot->GetGUID());
    if (itThird == intendedLineup.end())
    {
        float sx = itSecond->second.GetPositionX();
        float sy = itSecond->second.GetPositionY();

        float dx = closestTrigger->GetPositionX() - sx;
        float dy = closestTrigger->GetPositionY() - sy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        float targetX, targetY;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;
        constexpr float thresholdDist = 40.0f;
        constexpr float nearTriggerDist = 1.5f;
        constexpr float farDistance = 38.0f;

        if (distToTrigger <= thresholdDist)
        {
            float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
            targetX = sx + dx * moveDist;
            targetY = sy + dy * moveDist;
        }
        else
        {
            targetX = sx + dx * farDistance;
            targetY = sy + dy * farDistance;
        }

        intendedLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        itThird = intendedLineup.find(bot->GetGUID());
    }

    Position const& pos = itThird->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->AttackStop();
    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFourthCorePasser(
    Player*, Player* secondCorePasser,
    Player* thirdCorePasser, Unit* closestTrigger)
{
    bool needFourthPasser =
        (IsSecondCorePasserInPosition(secondCorePasser) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsThirdCorePasserInPosition(thirdCorePasser) &&
         thirdCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needFourthPasser)
        return false;

    auto itThird = intendedLineup.find(thirdCorePasser->GetGUID());
    if (itThird == intendedLineup.end())
        return false;

    auto itFourth = intendedLineup.find(bot->GetGUID());
    if (itFourth == intendedLineup.end())
    {
        float sx = itThird->second.GetPositionX();
        float sy = itThird->second.GetPositionY();

        float tx = closestTrigger->GetPositionX();
        float ty = closestTrigger->GetPositionY();

        float dx = tx - sx;
        float dy = ty - sy;
        float distToTrigger = std::sqrt(dx * dx + dy * dy);

        if (distToTrigger == 0.0f)
            return false;

        dx /= distToTrigger; dy /= distToTrigger;

        constexpr float nearTriggerDist = 1.5f;
        float targetX = tx - dx * nearTriggerDist;
        float targetY = ty - dy * nearTriggerDist;
        constexpr float targetZ = VASHJ_PLATFORM_CENTER_Z;

        intendedLineup.try_emplace(bot->GetGUID(), Position(targetX, targetY, targetZ));
        itFourth = intendedLineup.find(bot->GetGUID());
    }

    Position const& pos = itFourth->second;
    float targetX = pos.GetPositionX();
    float targetY = pos.GetPositionY();
    float targetZ = pos.GetPositionZ();

    bot->AttackStop();
    bot->CastStop();
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

// The next four functions check if the respective passer is <= 2 yards of their intended
// position and are used to determine when the prior bot in the chain can pass the core
bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInPosition(Player* firstCorePasser)
{
    auto itSnap = intendedLineup.find(firstCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = firstCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                       itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInPosition(Player* secondCorePasser)
{
    auto itSnap = intendedLineup.find(secondCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = secondCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                        itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsThirdCorePasserInPosition(Player* thirdCorePasser)
{
    auto itSnap = intendedLineup.find(thirdCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = thirdCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                       itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsFourthCorePasserInPosition(Player* fourthCorePasser)
{
    auto itSnap = intendedLineup.find(fourthCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = fourthCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                        itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

// ImbueItem() is inconsistent in causing the receiver bot to receive the core and the giver
// bot to remove the core, so ScheduleTransferCoreAfterImbue() creates the core on the receiver
// and removes it from the giver, with ImbueItem() called primarily for the throwing animation
void LadyVashjPassTheTaintedCoreAction::ScheduleTransferCoreAfterImbue(
    PlayerbotAI* botAI, Player* giver, Player* receiver)
{
    if (!receiver || !giver)
        return;

    constexpr uint32 delayMs = 1500;
    const ObjectGuid receiverGuid = receiver->GetGUID();
    const ObjectGuid giverGuid = giver->GetGUID();

    botAI->AddTimedEvent([receiverGuid, giverGuid]()
    {
        Player* receiverPlayer =
            receiverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(receiverGuid);
        Player* giverPlayer =
            giverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(giverGuid);

        if (!receiverPlayer)
            return;

        if (!receiverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        {
            ItemPosCountVec dest;
            uint32 count = 1;
            int canStore =
                receiverPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TAINTED_CORE, count);

            if (canStore == EQUIP_ERR_OK)
            {
                receiverPlayer->StoreNewItem(dest, ITEM_TAINTED_CORE, true,
                    Item::GenerateItemRandomPropertyId(ITEM_TAINTED_CORE));
            }
        }

        if (giverPlayer)
        {
            Item* item = giverPlayer->GetItemByEntry(ITEM_TAINTED_CORE);
            if (item && giverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                giverPlayer->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        }
    }, delayMs);
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator(const uint32 instanceId)
{
    auto const& generators =
        GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    GeneratorInfo const* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return false;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator || bot->GetExactDist2d(generator) > 4.5f)
        return false;

    Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (!core || bot->CanUseItem(core) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    const uint8 bagIndex = core->GetBagSlot();
    const uint8 slot = core->GetSlot();
    constexpr uint8 cast_count = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (core->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = core->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    const ObjectGuid item_guid = core->GetGUID();
    constexpr uint32 glyphIndex = 0;
    constexpr uint8 castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex;
    packet << slot;
    packet << cast_count;
    packet << spellId;
    packet << item_guid;
    packet << glyphIndex;
    packet << castFlags;
    packet << (uint32)TARGET_FLAG_GAMEOBJECT;
    packet << generator->GetGUID().WriteAsPacked();

    bot->GetSession()->HandleUseItemOpcode(packet);

    lastImbueAttempt.erase(instanceId);
    auto coreHandlers = GetCoreHandlers(botAI, bot);
    for (Player* handler : coreHandlers)
    {
        if (handler)
        {
            intendedLineup.erase(handler->GetGUID());
            lastCoreInInventoryTime.erase(handler->GetGUID());
        }
    }

    return true;
}

// Fallback for residual cores to be destroyed in Phase 3 in case
// ScheduleTransferCoreAfterImbue() fails to remove the core from the giver
bool LadyVashjDestroyTaintedCoreAction::Execute(Event /*event*/)
{
    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        bot->DestroyItem(core->GetBagSlot(), core->GetSlot(), true);
        return true;
    }

    return false;
}

// The standard "avoid aoe" strategy does work for Toxic Spores, but this method
// provides more buffer distance and limits the area in which bots can move
// so that they do not go down the stairs
bool LadyVashjAvoidToxicSporesAction::Execute(Event /*event*/)
{
    auto const& spores = GetAllSporeDropTriggers(bot);
    if (spores.empty())
        return false;

    constexpr float hazardRadius = 7.0f;
    bool inDanger = false;
    for (Unit* spore : spores)
    {
        if (bot->GetExactDist2d(spore) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Position const& vashjCenter = VASHJ_PLATFORM_CENTER_POSITION;
    constexpr float maxRadius = 60.0f;

    Position safestPos = FindSafestNearbyPosition(spores, vashjCenter, maxRadius, hazardRadius);
    bool backwards = vashj->GetVictim() == bot;
    MovementPriority priority = backwards ?
        MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT;

    return MoveTo(SSC_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  priority, true, backwards);
}

Position LadyVashjAvoidToxicSporesAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& spores, Position const& vashjCenter,
    float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 40.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = minDistance;
         distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);
            float z = bot->GetPositionZ();

            if (vashjCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            bool isSafe = true;
            for (Unit* spore : spores)
            {
                if (spore->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, z);

            bool pathSafe =
                IsPathSafeFromSpores(bot->GetPosition(), testPos, spores, hazardRadius);
            if (pathSafe || !foundSafe)
            {
                float moveDistance = bot->GetExactDist2d(x, y);

                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool LadyVashjAvoidToxicSporesAction::IsPathSafeFromSpores(Position const& start,
    Position const& end, std::vector<Unit*> const& spores, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* spore : spores)
        {
            float distToSpore = spore->GetExactDist2d(checkX, checkY);
            if (distToSpore < hazardRadius)
                return false;
        }
    }

    return true;
}

// When Toxic Sporebats spit poison, they summon "Spore Drop Trigger" NPCs
// that create the toxic pools
std::vector<Unit*> LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(Player* bot)
{
    std::vector<Unit*> sporeDropTriggers;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 50.0f;

    bot->GetCreatureListWithEntryInGrid(creatureList, NPC_SPORE_DROP_TRIGGER, searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            sporeDropTriggers.push_back(creature);
    }

    return sporeDropTriggers;
}

bool LadyVashjUseFreeActionAbilitiesAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto const& spores =
        LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(bot);
    constexpr float toxicSporeRadius = 6.0f;

    // If Rogues are Entangled and either have Static Charge or
    // are near a spore, use Cloak of Shadows
    if (bot->getClass() == CLASS_ROGUE && bot->HasAura(SPELL_ENTANGLE))
    {
        bool nearSpore = false;
        for (Unit* spore : spores)
        {
            if (bot->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearSpore = true;
                break;
            }
        }
        if (bot->HasAura(SPELL_STATIC_CHARGE) || nearSpore)
        {
            if (botAI->CanCastSpell("cloak of shadows", bot))
                return botAI->CastSpell("cloak of shadows", bot);
        }
    }

    // The remainder of the logic is for Paladins to use Hand of Freedom
    Player* mainTankToxic = nullptr;
    Player* anyToxic = nullptr;
    Player* mainTankStatic = nullptr;
    Player* anyStatic = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->HasAura(SPELL_ENTANGLE) ||
            !PlayerbotAI::IsMelee(member))
            continue;

        bool nearToxicSpore = false;
        for (Unit* spore : spores)
        {
            if (member->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearToxicSpore = true;
                break;
            }
        }

        if (nearToxicSpore)
        {
            if (PlayerbotAI::IsMainTank(member))
                mainTankToxic = member;

            if (!anyToxic)
                anyToxic = member;
        }

        if (member->HasAura(SPELL_STATIC_CHARGE))
        {
            if (PlayerbotAI::IsMainTank(member))
                mainTankStatic = member;

            if (!anyStatic)
                anyStatic = member;
        }
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        // Priority 1: Entangled in Toxic Spores (prefer main tank)
        Player* toxicTarget = mainTankToxic ? mainTankToxic : anyToxic;
        if (toxicTarget && botAI->CanCastSpell("hand of freedom", toxicTarget))
            return botAI->CastSpell("hand of freedom", toxicTarget);

        // Priority 2: Entangled with Static Charge (prefer main tank)
        Player* staticTarget = mainTankStatic ? mainTankStatic : anyStatic;
        if (staticTarget && botAI->CanCastSpell("hand of freedom", staticTarget))
            return botAI->CastSpell("hand of freedom", staticTarget);
    }

    return false;
}
