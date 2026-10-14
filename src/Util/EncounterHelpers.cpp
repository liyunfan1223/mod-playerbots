/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EncounterHelpers.h"
#include "CellImpl.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "HunterActions.h"
#include "InstanceScript.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "RogueActions.h"
#include "RtiTargetValue.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include <algorithm>
#include <cmath>
#include <list>

namespace EncounterHelpers
{

// Calling InstanceScript::IsEncounterInProgress is a very cheap check to use as an initial gate
// for triggers and multipliers that should run only during a boss fight. This will not work for
// every single encounter, as some bosses are not scripted to report IN_PROGRESS (but at least in
// TBC raids, that is rare: only Terestian Illhoof and Illidari Council do not). It's also possible
// for a boss script to set IN_PROGRESS upon an event other than the pull; that's at least the case
// with Kil'jaeden, who is set to IN_PROGRESS only after 1 of the 3 Hands of the Deceiver is killed
// in phase 1. To avoid spamming this check across each trigger and multiplier, you can create a
// derived class of Trigger or Multiplier to call this helper and then derive your triggers and
// multipliers from the intermediate class.
bool IsEncounterInProgress(Player* bot, uint32 mapId)
{
    if (bot->GetMapId() != mapId)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && instance->IsEncounterInProgress();
}

// For validating ground and collision in connection with issuing incremental movement. The caller
// gives a destination and how far to travel towards it per tick. The helper projects that step,
// checks whether the bot can actually take it, and returns where it lands. The returned stepZ is
// snapped to the ground, so a MoveTo() using this helper should pass stepZ rather than the bot's Z.
bool CanTakeStepTowards(
    Player* bot, float destinationX, float destinationY, float moveDist,
    float& stepX, float& stepY, float& stepZ)
{
    constexpr float minMoveDistance = 0.5f;

    float const distance = bot->GetExactDist2d(destinationX, destinationY);
    if (distance < minMoveDistance)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botZ = bot->GetPositionZ();

    float const ratio = std::min(moveDist, distance) / distance;
    float candidateX = botX + (destinationX - botX) * ratio;
    float candidateY = botY + (destinationY - botY) * ratio;
    float candidateZ = bot->GetMapWaterOrGroundLevel(candidateX, candidateY, botZ);

    if (candidateZ <= INVALID_HEIGHT)
        candidateZ = botZ;

    // The 9th parameter of CanReachPositionAndGetValidCoords(), failOnSlopes, returns false for a
    // non-walkable slope, but in my experience, walking downhill is always possible, and thus the
    // check needlessly rejects descents. This variable gets around that problem.
    bool const failOnSlopes = candidateZ > botZ;

    // This helper will return false on collision rather than clamping to the contact point so that
    // the caller can try a different path. Clamping is useless for avoidance since the bot will die
    // just the same if it is in the middle of a hazard vs. halfway out and returning true.
    float const requestedX = candidateX;
    float const requestedY = candidateY;

    if (!bot->GetMap()->CanReachPositionAndGetValidCoords(
            bot, botX, botY, botZ, candidateX, candidateY, candidateZ, true, failOnSlopes))
    {
        return false;
    }

    constexpr float truncationTolerance = 1.0f;
    if (std::hypot(candidateX - requestedX, candidateY - requestedY) > truncationTolerance)
        return false;

    stepX = candidateX;
    stepY = candidateY;
    stepZ = candidateZ;
    return true;
}

// Calculate incremental movement to a position. No ground or collision is validated, unlike
// CanTakeStepTowards(). The Z position passed for the MoveTo() action using this helper should
// use the bot's Z, not the position's. Returns false once the bot is within arrivalDist.
bool GetStepToPosition(
    Player* bot, Position const& position, float arrivalDist, Unit* facing, float& stepX,
    float& stepY, bool& backwards)
{
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= arrivalDist)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = position.GetPositionX() - botX;
    float const toPosY = position.GetPositionY() - botY;

    // 'facing' is optional and is for tanks. Pass the mob being tanked to allow the step to be
    // walked backwards when (1) the bot has aggro on the mob it is tanking, (2) the bot is in
    // melee range of the mob, and (3) the destination is on the opposite side of the bot from the
    // mob. Generally, the entire movement would be gated on (1) and (2) anyway, but there are some
    // exceptions and thus the checks are made again here. Pass nullptr for a plain forward step.
    backwards = false;
    if (facing && facing->GetVictim() == bot && bot->IsWithinMeleeRange(facing))
    {
        float const toFacingX = facing->GetPositionX() - botX;
        float const toFacingY = facing->GetPositionY() - botY;
        backwards = (toPosX * toFacingX + toPosY * toFacingY) < 0.0f;
    }

    // Default time between AI ticks is 100ms, and base movement speed for players is 7y/s forwards
    // and 4.5y/s backwards (i.e., 0.7y/0.45y per tick). There is not really benefit to having the
    // step be farther than the distance that can be covered in a single tick. But this helper
    // uses 5x tick distance to account for possible speed boosts, latency, and longer configured
    // AI ticks. In my experience, this is plenty short enough to navigate poor terrain, but if you
    // are moving steeply uphill and find that movement is failing, it may be possible that the step
    // distances would need to be even shorter (in which case you couldn't use this helper).
    constexpr float backwardDistancePerStep = 2.25f;
    constexpr float forwardDistancePerStep = 3.5f;
    float const maxMoveDist = backwards ? backwardDistancePerStep : forwardDistancePerStep;
    float const ratio = std::min(maxMoveDist, distToPosition) / distToPosition;

    stepX = botX + toPosX * ratio;
    stepY = botY + toPosY * ratio;

    return true;
}

// Functions to mark targets with raid target icons.
// Note that these functions do not allow the player to change the icon during the encounter.
bool MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
{
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid currentGuid = group->GetTargetIcon(iconId);
    if (currentGuid != target->GetGUID())
    {
        group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
        return true;
    }

    return false;
}

bool MarkTargetWithSkull(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
}

bool MarkTargetWithSquare(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
}

bool MarkTargetWithStar(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
}

bool MarkTargetWithCircle(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
}

bool MarkTargetWithDiamond(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
}

bool MarkTargetWithTriangle(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
}

bool MarkTargetWithCross(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
}

bool MarkTargetWithMoon(Player* bot, Unit* target)
{
    return MarkTargetWithIcon(bot, target, RtiTargetValue::moonIndex);
}

// For clearing marks outside of combat so bots don't Leeroy on sight. This is best used when gated
// behind an out-of-combat check (such as with IsInCombatValue).
bool ClearTargetIcon(Player* bot, uint8 iconId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid currentGuid = group->GetTargetIcon(iconId);
    if (currentGuid != ObjectGuid::Empty)
    {
        group->SetTargetIcon(iconId, bot->GetGUID(), ObjectGuid::Empty);
        return true;
    }

    return false;
}

// For bots to set their raid target icon to the specified icon
void SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName)
{
    Value<std::string>* rtiValue =
        botAI->GetAiObjectContext()->GetValue<std::string>("rti");

    if (rtiValue->Get() != rtiName)
        rtiValue->Set(rtiName);
}

// Return the first alive bot in the specified instance map for purposes of assigning
// a single bot to manage associative containers, mark targets, etc.
bool IsMechanicTrackerBot(Player* bot, uint32 mapId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != mapId ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        return member == bot;
    }

    return false;
}

// Requires the main tank to be alive
Player* GetGroupMainTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    ObjectGuid const mainTankGuid = PlayerbotAI::GetMainTankGuid(group);
    if (mainTankGuid.IsEmpty())
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetGUID() == mainTankGuid)
            return member;
    }

    return nullptr;
}

// Returns the alive assist tank of the specified index (0 = first, 1 = second, etc.)
Player* GetGroupAssistTank(Player* bot, uint8 index)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    ObjectGuid const mainTankGuid = PlayerbotAI::GetMainTankGuid(group);
    if (mainTankGuid.IsEmpty())
        return nullptr;

    uint8 assistantCount = 0;
    std::vector<Player*> nonAssistantTanks;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member) ||
            member->GetGUID() == mainTankGuid)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
        {
            if (assistantCount == index)
                return member;

            assistantCount++;
        }
        else
        {
            nonAssistantTanks.push_back(member);
        }
    }

    uint8 nonAssistantIndex = index - assistantCount;
    if (nonAssistantIndex < nonAssistantTanks.size())
        return nonAssistantTanks[nonAssistantIndex];

    return nullptr;
}

// DO NOT USE. TO BE REMOVED HERE ONCE ALL CALL SITES ARE MODIFIED.
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry)
{
    auto const& units =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    for (auto const& unitGuid : units)
    {
        Unit* unit = botAI->GetUnit(unitGuid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

// Return the nearest alive player (human or bot) within the specified radius. Distance is
// measured by GetExactDist2d(), which does not take into account either player's CombatReach
// (i.e., their hitboxes), which are 1.5y for all races (or 1.95y with Bloodlust/Heroism active).
Player* GetNearestPlayerInRadius(Player* bot, float radius)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearestPlayer = nullptr;
    float nearestDistance = radius;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        float distance = bot->GetExactDist2d(member);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestPlayer = member;
        }
    }

    return nearestPlayer;
}

// Grid search for dynamic objects for methods to avoid dynobj-based AoE hazards.
std::vector<Position> GetDynamicObjectPositions(Player* bot, float searchRadius, uint32 spellId)
{
    std::list<WorldObject*> objs;
    Acore::AllWorldObjectsInRange check(bot, searchRadius);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> searcher(
        bot, objs, check, GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(bot, searcher, searchRadius);

    std::vector<Position> dynObjs;
    for (WorldObject* obj : objs)
    {
        if (obj->GetTypeId() != TYPEID_DYNAMICOBJECT)
            continue;

        DynamicObject* dynObj = static_cast<DynamicObject*>(obj);
        if (dynObj->GetSpellId() == spellId)
        {
            dynObjs.emplace_back(
                dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetPositionZ());
        }
    }

    return dynObjs;
}

// Ice Block, Divine Shield, and/or Cloak of Shadows can be used to nullify or ignore several
// dangerous boss mechanics.
uint32 GetSelfImmunitySpell(Player* bot)
{
    constexpr uint32 iceBlock = 45438;
    constexpr uint32 divineShield = 642;
    constexpr uint32 cloakOfShadows = 31224;

    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return iceBlock;

        case CLASS_PALADIN:
            return divineShield;

        case CLASS_ROGUE:
            return cloakOfShadows;

        default:
            return 0;
    }
}

// This function is primarily for use in multipliers during encounters where it is desirable
// for bots to save cooldowns for particular phases (or for a bit after the pull).
bool IsDpsCooldownAction(Player* bot, Action* action)
{
    if (bot->getClass() == CLASS_SHAMAN && // Before dps gate to capture Resto
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action)))
    {
        return true;
    }

    if (!PlayerbotAI::IsDps(bot))
        return false;

    if (dynamic_cast<UseTrinketAction*>(action))
        return true;

    bool isClassCooldown = false;
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            isClassCooldown = dynamic_cast<CastSummonGargoyleAction*>(action) ||
                dynamic_cast<CastDeathchillAction*>(action) ||
                dynamic_cast<CastEmpowerRuneWeaponAction*>(action) ||
                dynamic_cast<CastArmyOfTheDeadAction*>(action);
            break;

        case CLASS_DRUID:
            isClassCooldown = dynamic_cast<CastStarfallAction*>(action) ||
                dynamic_cast<CastForceOfNatureAction*>(action) ||
                dynamic_cast<CastBerserkAction*>(action);
            break;

        case CLASS_HUNTER:
            isClassCooldown = dynamic_cast<CastKillCommandAction*>(action) ||
                dynamic_cast<CastRapidFireAction*>(action) ||
                dynamic_cast<CastReadinessAction*>(action) ||
                dynamic_cast<CastBestialWrathAction*>(action);
            break;

        case CLASS_MAGE:
            isClassCooldown = dynamic_cast<CastArcanePowerAction*>(action) ||
                dynamic_cast<CastCombustionAction*>(action) ||
                dynamic_cast<CastIcyVeinsAction*>(action) ||
                dynamic_cast<CastMirrorImageAction*>(action) ||
                dynamic_cast<CastColdSnapAction*>(action) ||
                dynamic_cast<CastPresenceOfMindAction*>(action);
            break;

        case CLASS_SHAMAN:
            isClassCooldown = dynamic_cast<CastElementalMasteryAction*>(action) ||
                dynamic_cast<CastFeralSpiritAction*>(action) ||
                dynamic_cast<CastFireElementalTotemAction*>(action) ||
                dynamic_cast<CastFireElementalTotemMeleeAction*>(action);
            break;

        case CLASS_PALADIN:
            isClassCooldown = dynamic_cast<CastAvengingWrathAction*>(action);
            break;

        case CLASS_ROGUE:
            isClassCooldown = dynamic_cast<CastKillingSpreeAction*>(action) ||
                dynamic_cast<CastBladeFlurryAction*>(action) ||
                dynamic_cast<CastAdrenalineRushAction*>(action) ||
                dynamic_cast<CastColdBloodAction*>(action);
            break;

        case CLASS_WARLOCK:
            isClassCooldown = dynamic_cast<CastMetamorphosisAction*>(action);
            break;

        case CLASS_WARRIOR:
            isClassCooldown = dynamic_cast<CastDeathWishAction*>(action) ||
                dynamic_cast<CastBladestormAction*>(action) ||
                dynamic_cast<CastRecklessnessAction*>(action);
            break;

        default:
            break; // Priest =(
    }

    if (isClassCooldown)
        return true;

    switch (bot->getRace())
    {
        case RACE_BLOODELF:
            return dynamic_cast<CastArcaneTorrentAction*>(action);

        case RACE_ORC:
            return dynamic_cast<CastBloodFuryAction*>(action);

        case RACE_TROLL:
            return dynamic_cast<CastBerserkingAction*>(action);

        default:
            return false;
    }
}

bool IsTauntAction(Player* bot, Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastDeathGripAction*>(action);

        case CLASS_DRUID:
            return dynamic_cast<CastGrowlAction*>(action) ||
                dynamic_cast<CastChallengingRoarAction*>(action);

        case CLASS_PALADIN:
            return dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastRighteousDefenseAction*>(action);

        case CLASS_WARRIOR:
            return dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastChallengingShoutAction*>(action);

        default:
            return false;
    }
}

// These abilities can be particularly problematic on the pull for a council-type boss.
bool IsAoeThreatAction(Player* bot, Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return dynamic_cast<CastDeathAndDecayAction*>(action) ||
                dynamic_cast<CastPestilenceAction*>(action) ||
                dynamic_cast<CastBloodBoilAction*>(action);

        case CLASS_DRUID:
            return dynamic_cast<CastSwipeBearAction*>(action);

        case CLASS_PALADIN:
            return dynamic_cast<CastAvengersShieldAction*>(action) ||
                dynamic_cast<CastConsecrationAction*>(action);

        case CLASS_WARRIOR:
            return dynamic_cast<CastThunderClapAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastCleaveAction*>(action);

        default:
            return false;
    }
}

}
