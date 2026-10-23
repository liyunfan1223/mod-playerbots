/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCHelpers.h"
#include "EncounterHelpers.h"
#include "Map.h"
#include "PathGenerator.h"
#include "Random.h"
#include "RtiTargetValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

using namespace EncounterHelpers;

namespace SscHelpers
{

namespace
{

std::mutex sscStateMutex;
std::unordered_map<uint32, SscInstanceState> sscStates;

Creature* GetCachedCreature(PlayerbotAI* botAI, char const* value)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    Creature* creature = botAI->GetCreature(AI_VALUE(ObjectGuid, value));
    return creature && creature->IsAlive() ? creature : nullptr;
}

std::vector<Position> const& GetCachedHazardPositions(PlayerbotAI* botAI, char const* value)
{
    return botAI->GetAiObjectContext()->GetValue<std::vector<Position>>(value)->RefGet();
}

} // end anonymous namespace

// Shared

bool MisdirectTargetToTank(PlayerbotAI* botAI, Unit* target, Player* tank)
{
    if (!target || !tank)
        return false;

    if (botAI->CanCastSpell(Id(SscSpells::SPELL_MISDIRECTION), tank))
        return botAI->CastSpell(Id(SscSpells::SPELL_MISDIRECTION), tank);

    if (!botAI->GetBot()->HasAura(Id(SscSpells::SPELL_MISDIRECTION_AURA)))
        return false;

    return botAI->CanCastSpell("steady shot", target) && botAI->CastSpell("steady shot", target);
}

bool CastTankTaunt(PlayerbotAI* botAI, Unit* target)
{
    if (!target)
        return false;

    char const* taunt = nullptr;
    switch (botAI->GetBot()->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            taunt = "dark command";
            break;
        case CLASS_DRUID:
            taunt = "growl";
            break;
        case CLASS_PALADIN:
            taunt = "hand of reckoning";
            break;
        case CLASS_WARRIOR:
            taunt = "taunt";
            break;
        default:
            return false;
    }

    return botAI->CanCastSpell(taunt, target) && botAI->CastSpell(taunt, target);
}

bool FindHazardEscapeStep(
    Player* bot, Position const& hazard, float moveDist, float& stepX, float& stepY, float& stepZ)
{
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const botDistance = bot->GetExactDist2d(hazard);

    float escapeAngle = std::atan2(botY - hazard.GetPositionY(), botX - hazard.GetPositionX());
    if (botDistance <= 0.1f)
        escapeAngle = bot->GetOrientation();

    constexpr uint8 fanSteps = 16;
    constexpr float fanStep = static_cast<float>(M_PI) / fanSteps;

    for (uint8 step = 0; step <= fanSteps; ++step)
    {
        float const delta = fanStep * step;
        uint8 const candidates = (step == 0) ? 1 : 2;
        for (uint8 i = 0; i < candidates; ++i)
        {
            float const angle = escapeAngle + (i == 0 ? delta : -delta);
            float const candidateX = botX + std::cos(angle) * moveDist;
            float const candidateY = botY + std::sin(angle) * moveDist;

            if (hazard.GetExactDist2d(candidateX, candidateY) <= botDistance)
                continue;

            if (!IsDryGround(bot, candidateX, candidateY))
                continue;

            if (CanTakeStepTowards(bot, candidateX, candidateY, moveDist, stepX, stepY, stepZ))
                return true;
        }
    }

    return false;
}

bool IsDryGround(Player* bot, float x, float y)
{
    float const ground = bot->GetMapHeight(x, y, bot->GetPositionZ());
    if (ground <= INVALID_HEIGHT)
        return false;

    LiquidData const liquid = bot->GetMap()->GetLiquidData(
        bot->GetPhaseMask(), x, y, bot->GetPositionZ(), bot->GetCollisionHeight(), {});

    constexpr float clearance = 0.5f;
    return liquid.Level <= INVALID_HEIGHT || ground > liquid.Level - clearance;
}

bool GetPathStepTowardUnit(
    Player* bot, Unit* target, float stopDistance, float& stepX, float& stepY)
{
    if (!target)
        return false;

    return GetPathStepTowardPoint(
        bot, target->GetPosition(), stopDistance, PATH_STEP_DISTANCE, stepX, stepY);
}

bool GetPathStepTowardPoint(
    Player* bot, Position const& destination, float stopDistance, float stepDistance,
    float& stepX, float& stepY)
{
    if (bot->GetExactDist(destination) < stopDistance)
        return false;

    PathGenerator path(bot);
    path.CalculatePath(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());
    if (!(path.GetPathType() & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT)))
        return false;

    Movement::PointsArray const& points = path.GetPath();
    if (points.size() < 2)
        return false;

    G3D::Vector3 const targetPos(
        destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());

    float remaining = stepDistance;
    for (std::size_t i = 1; i < points.size(); ++i)
    {
        G3D::Vector3 const& from = points[i - 1];
        G3D::Vector3 const& to = points[i];

        float const segment = (to - from).length();
        if (segment <= 0.0f)
            continue;

        float const toDist = (to - targetPos).length();
        float ratio = 1.0f;

        if (toDist < stopDistance)
        {
            float const fromDist = (from - targetPos).length();
            if (fromDist <= stopDistance)
                break;

            ratio = (fromDist - stopDistance) / (fromDist - toDist);
        }

        if (segment * ratio >= remaining)
            ratio = remaining / segment;

        remaining -= segment * ratio;

        G3D::Vector3 const step = from + (to - from) * ratio;
        stepX = step.x;
        stepY = step.y;

        if (remaining <= 0.0f || ratio < 1.0f)
            return true;
    }

    return remaining < stepDistance;
}

bool GetRangedArcAngle(Player* bot, float arcCenter, float arcSpan, float& angle)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    size_t count = 0;
    size_t botIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != SSC_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (member == bot)
            botIndex = count;

        ++count;
    }

    if (count == 0)
        return false;

    angle = count == 1 ? arcCenter : arcCenter - arcSpan / 2.0f +
        arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1);
    return true;
}

std::vector<Unit*> GetOtherLivingGroupMembers(Player* bot)
{
    std::vector<Unit*> members;
    Group* group = bot->GetGroup();
    if (!group)
        return members;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->GetMapId() == SSC_MAP_ID)
            members.push_back(member);
    }

    return members;
}

// Trash

bool GetToxicPoolPosition(PlayerbotAI* botAI, Position& toxicPool)
{
    std::vector<Position> const& positions =
        GetCachedHazardPositions(botAI, "ssc toxic pool");
    if (positions.empty())
        return false;

    Player* bot = botAI->GetBot();
    toxicPool = *std::min_element(positions.begin(), positions.end(),
        [bot](Position const& a, Position const& b)
        {
            return bot->GetExactDist2d(a) < bot->GetExactDist2d(b);
        });
    return true;
}

bool IsNearToxicPool(PlayerbotAI* botAI, float radius)
{
    Position toxicPool;
    return GetToxicPoolPosition(botAI, toxicPool) &&
        botAI->GetBot()->GetExactDist2d(toxicPool) < radius;
}

ObjectGuid FindWaterElementalTotemGuid(Player* bot)
{
    Creature* totem = bot->FindNearestCreature(
        Id(SscNpcs::NPC_WATER_ELEMENTAL_TOTEM), WATER_ELEMENTAL_TOTEM_SEARCH_DISTANCE);
    return totem ? totem->GetGUID() : ObjectGuid::Empty;
}

Creature* GetWaterElementalTotem(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc water elemental totem");
}

bool IsSkullOnWaterElementalTotem(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return false;

    ObjectGuid const skull = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (skull.GetEntry() != Id(SscNpcs::NPC_WATER_ELEMENTAL_TOTEM))
        return false;

    Unit* totem = botAI->GetUnit(skull);
    return totem && totem->IsAlive();
}

// Hydross the Unstable <Duke of Currents>

bool IsHydrossFrostTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
}

bool IsHydrossNatureTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool IsHydrossPhaseTank(Player* bot)
{
    return IsHydrossFrostTank(bot) || IsHydrossNatureTank(bot);
}

bool IsHydrossAddTank(Player* bot)
{
    return PlayerbotAI::IsTank(bot) && !IsHydrossPhaseTank(bot);
}

bool IsHydrossInFrostPhase(Unit* hydross)
{
    return hydross && !hydross->HasAura(Id(SscSpells::SPELL_HYDROSS_CORRUPTION));
}

bool IsHydrossInNaturePhase(Unit* hydross)
{
    return hydross && hydross->HasAura(Id(SscSpells::SPELL_HYDROSS_CORRUPTION));
}

HydrossDpsHoldWindow GetHydrossDpsHoldWindow(Unit* hydross)
{
    if (!hydross)
        return HydrossDpsHoldWindow::None;

    bool const frostPhase = IsHydrossInFrostPhase(hydross);
    SscInstanceState const& state = SscState(hydross->GetInstanceId());
    std::optional<uint32> const& phaseStartTime =
        frostPhase ? state.hydrossFrostPhaseStartTime : state.hydrossNaturePhaseStartTime;
    std::optional<uint32> const& markMaxedTime =
        frostPhase ? state.hydrossFrostMarkMaxedTime : state.hydrossNatureMarkMaxedTime;

    uint32 const now = getMSTime();
    constexpr uint32 handOverWaitMs = 1 * IN_MILLISECONDS;
    constexpr uint32 phaseStartWaitMs = 5 * IN_MILLISECONDS;

    if (markMaxedTime && getMSTimeDiff(*markMaxedTime, now) >= handOverWaitMs)
        return HydrossDpsHoldWindow::BeforePhaseChange;

    if (!phaseStartTime || getMSTimeDiff(*phaseStartTime, now) < phaseStartWaitMs)
    {
        return HydrossDpsHoldWindow::AfterPhaseChange;
    }

    return HydrossDpsHoldWindow::None;
}

Position GetHydrossHandoffPosition(bool frostTank)
{
    Position const& own = frostTank ? HYDROSS_FROST_TANK_POSITION : HYDROSS_NATURE_TANK_POSITION;
    Position const& other =
        frostTank ? HYDROSS_NATURE_TANK_POSITION : HYDROSS_FROST_TANK_POSITION;

    float const dx = other.GetPositionX() - own.GetPositionX();
    float const dy = other.GetPositionY() - own.GetPositionY();
    float const length = std::sqrt(dx * dx + dy * dy);
    if (length <= 0.0f)
        return own;

    float const dirX = dx / length;
    float const dirY = dy / length;
    float const offX = own.GetPositionX() - HYDROSS_CLEANSING_FIELD_CENTER.GetPositionX();
    float const offY = own.GetPositionY() - HYDROSS_CLEANSING_FIELD_CENTER.GetPositionY();
    float const b = offX * dirX + offY * dirY;
    float const c = offX * offX + offY * offY -
        HYDROSS_CLEANSING_FIELD_RADIUS * HYDROSS_CLEANSING_FIELD_RADIUS;
    float const discriminant = b * b - c;
    if (discriminant < 0.0f)
        return own;

    float const root = std::sqrt(discriminant);
    float const toEdge = c < 0.0f ? -b + root : -b - root;
    float const distance = toEdge - HYDROSS_HANDOFF_SHORT_DISTANCE;
    if (distance <= 0.0f || toEdge > length)
        return own;

    return Position(own.GetPositionX() + dirX * distance, own.GetPositionY() + dirY * distance,
        own.GetPositionZ());
}

bool CanHydrossMarkBeMaxed(Unit* hydross)
{
    if (!hydross)
        return false;

    SscInstanceState const& state = SscState(hydross->GetInstanceId());
    std::optional<uint32> const& phaseStartTime = IsHydrossInFrostPhase(hydross) ?
        state.hydrossFrostPhaseStartTime : state.hydrossNaturePhaseStartTime;

    // Don't count residual Marks from a bad transition for purposes of the dps hold window.
    constexpr uint32 staleMarkWindowMs = 45 * IN_MILLISECONDS;
    return phaseStartTime && getMSTimeDiff(*phaseStartTime, getMSTime()) >= staleMarkWindowMs;
}

bool HasMarkOfHydrossAt100Percent(Player* player)
{
    if (!player)
        return false;

    return player->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_100)) ||
        player->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_250)) ||
        player->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_500));
}

bool HasNoMarkOfHydross(Player* bot)
{
    return !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_10)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_25)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_50)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_100)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_250)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_HYDROSS_500));
}

bool HasMarkOfCorruptionAt100Percent(Player* player)
{
    if (!player)
        return false;

    return player->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_100)) ||
        player->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_250)) ||
        player->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_500));
}

bool HasNoMarkOfCorruption(Player* bot)
{
    return !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_10)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_25)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_50)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_100)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_250)) &&
        !bot->HasAura(Id(SscSpells::SPELL_MARK_OF_CORRUPTION_500));
}

// The Lurker Below

bool IsLurkerSpouting(Unit* lurker)
{
    Creature* creature = lurker ? lurker->ToCreature() : nullptr;
    return creature && creature->IsInCombat() && creature->GetReactState() == REACT_PASSIVE;
}

bool IsLurkerSurfacedAndCalm(Unit* lurker)
{
    return lurker && lurker->getStandState() != UNIT_STAND_STATE_SUBMERGED &&
        !IsLurkerSpouting(lurker);
}

bool DoesPathRoundLurker(Player* bot, Unit* lurker, float x, float y, float z, int8 direction)
{
    if (!lurker)
        return false;

    PathGenerator path(bot);
    if (!path.CalculatePath(x, y, z) || (path.GetPathType() & PATHFIND_NOPATH))
        return false;

    Movement::PointsArray const& points = path.GetPath();
    if (points.size() < 2)
        return false;

    float const startAngle = std::atan2(
        points[0].y - lurker->GetPositionY(), points[0].x - lurker->GetPositionX());
    float const cornerAngle = std::atan2(
        points[1].y - lurker->GetPositionY(), points[1].x - lurker->GetPositionX());
    float delta = Position::NormalizeOrientation(cornerAngle - startAngle);
    if (delta > M_PI)
        delta -= 2.0f * static_cast<float>(M_PI);

    return delta * direction > 0.0f;
}

float GetArrivingPathLength(Player* bot, float x, float y, float z, float tolerance)
{
    PathGenerator path(bot);
    if (!path.CalculatePath(x, y, z) || (path.GetPathType() & PATHFIND_NOPATH))
        return -1.0f;

    G3D::Vector3 const& end = path.GetActualEndPosition();
    if (std::hypot(end.x - x, end.y - y) > tolerance)
        return -1.0f;

    Movement::PointsArray const& points = path.GetPath();
    float length = 0.0f;
    for (size_t i = 1; i < points.size(); ++i)
        length += std::hypot(points[i].x - points[i - 1].x, points[i].y - points[i - 1].y);

    return length;
}

int8 GetLurkerSpoutSpin(Unit* lurker)
{
    if (!lurker)
        return 0;

    if (lurker->HasAura(Id(SscSpells::SPELL_SPOUT_COUNTERCLOCKWISE)))
        return 1;

    if (lurker->HasAura(Id(SscSpells::SPELL_SPOUT_CLOCKWISE)))
        return -1;

    return 0;
}

GuidVector FindLurkerGuardianGuids(Player* bot)
{
    GuidVector guids;

    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(
        creatures, Id(SscNpcs::NPC_COILFANG_GUARDIAN), LURKER_GUARDIAN_SEARCH_RADIUS);

    for (Creature* creature : creatures)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end());

    return guids;
}

std::vector<Unit*> GetLurkerGuardians(PlayerbotAI* botAI)
{
    std::vector<Unit*> guardians;

    for (auto const& guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("ssc lurker guardians")->RefGet())
    {
        Unit* guardian = botAI->GetUnit(guid);
        if (guardian && guardian->IsAlive())
            guardians.push_back(guardian);
    }

    return guardians;
}

GuidVector FindLurkerGuardianTankGuids(Player* bot)
{
    std::array const tanks = {
        GetGroupMainTank(bot), GetGroupAssistTank(bot, 0), GetGroupAssistTank(bot, 1) };
    static_assert(std::tuple_size_v<decltype(tanks)> == LURKER_GUARDIAN_TANK_COUNT);

    GuidVector guids;
    for (Player* tank : tanks)
    {
        if (!tank)
            return {};

        guids.push_back(tank->GetGUID());
    }

    return guids;
}

int8 GetLurkerGuardianTankIndex(PlayerbotAI* botAI)
{
    auto const& tanks = botAI->GetAiObjectContext()
        ->GetValue<GuidVector>("ssc lurker guardian tanks")->RefGet();
    ObjectGuid const guid = botAI->GetBot()->GetGUID();
    for (size_t i = 0; i < tanks.size(); ++i)
    {
        if (tanks[i] == guid)
            return static_cast<int8>(i);
    }

    return -1;
}

bool ShouldGoToLurkerWalkway(Player* bot, Unit* lurker, Unit* target)
{
    if (!lurker || !target)
        return false;

    if (target == lurker)
        return true;

    return target->GetEntry() == Id(SscNpcs::NPC_COILFANG_GUARDIAN) &&
        bot->GetExactDist2d(lurker) > LURKER_ISLET_DISTANCE;
}

// Leotheras the Blind

ObjectGuid FindLeotherasGuid(Player* bot)
{
    Creature* leotheras =
        bot->FindNearestCreature(Id(SscNpcs::NPC_LEOTHERAS_THE_BLIND), LEOTHERAS_SEARCH_DISTANCE);
    return leotheras ? leotheras->GetGUID() : ObjectGuid::Empty;
}

ObjectGuid FindShadowOfLeotherasGuid(Player* bot)
{
    Creature* shadow =
        bot->FindNearestCreature(Id(SscNpcs::NPC_SHADOW_OF_LEOTHERAS), LEOTHERAS_SEARCH_DISTANCE);
    return shadow ? shadow->GetGUID() : ObjectGuid::Empty;
}

Creature* GetLeotheras(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc leotheras");
}

bool IsSpellbinderPhase(Unit* leotheras)
{
    return leotheras && leotheras->HasAura(Id(SscSpells::SPELL_LEOTHERAS_BANISHED));
}

Creature* GetActiveLeotherasHumanoid(PlayerbotAI* botAI)
{
    Creature* leotheras = GetLeotheras(botAI);
    if (!leotheras || IsSpellbinderPhase(leotheras))
        return nullptr;

    if (!leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

bool IsLeotherasHumanoidPhase(PlayerbotAI* botAI)
{
    return GetActiveLeotherasHumanoid(botAI) && !GetShadowOfLeotheras(botAI);
}

Creature* GetLeotherasDemon(PlayerbotAI* botAI)
{
    Creature* leotheras = GetLeotheras(botAI);
    if (leotheras && leotheras->HasAura(Id(SscSpells::SPELL_METAMORPHOSIS)))
        return leotheras;

    return nullptr;
}

bool IsLeotherasDemonPhase(PlayerbotAI* botAI)
{
    return GetLeotherasDemon(botAI);
}

Creature* GetShadowOfLeotheras(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc shadow of leotheras");
}

bool IsLeotherasFinalPhase(PlayerbotAI* botAI)
{
    return GetShadowOfLeotheras(botAI);
}

Creature* GetLeotherasDemonOrShadow(PlayerbotAI* botAI)
{
    if (Creature* demon = GetLeotherasDemon(botAI))
        return demon;

    if (Creature* shadow = GetShadowOfLeotheras(botAI))
        return shadow;

    return nullptr;
}

Player* GetLeotherasWarlockTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK ||
            member->GetMapId() != SSC_MAP_ID)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

bool IsLeotherasWarlockTank(Player* bot)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    return GetLeotherasWarlockTank(bot) == bot;
}

bool IsLeotherasChannelingWhirlwind(Unit* leotheras)
{
    return leotheras && leotheras->HasAura(Id(SscSpells::SPELL_LEOTHERAS_WHIRLWIND));
}

Creature* GetLeotherasHumanoidToAvoid(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (HasInnerDemon(bot))
        return nullptr;

    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (!leotherasHumanoid || leotherasHumanoid->GetVictim() == bot ||
        bot->GetExactDist2d(leotherasHumanoid) >= LEOTHERAS_RANGED_SAFE_DISTANCE)
    {
        return nullptr;
    }

    return leotherasHumanoid;
}

Unit* GetDemonTargetToAvoid(Player* bot, Unit* demon)
{
    if (!demon)
        return nullptr;

    Unit* demonVictim = demon->GetVictim();
    if (!demonVictim || demonVictim == bot ||
        bot->GetExactDist2d(demonVictim) >= LEOTHERAS_CHAOS_BLAST_SAFE_DISTANCE)
    {
        return nullptr;
    }

    return demonVictim;
}

Unit* GetChaosBlastTargetToAvoid(PlayerbotAI* botAI)
{
    Creature* leotherasDemon = GetLeotherasDemonOrShadow(botAI);
    if (!leotherasDemon)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (Unit* demonVictim = GetDemonTargetToAvoid(bot, leotherasDemon))
        return demonVictim;

    Player* warlockTank = GetLeotherasWarlockTank(bot);
    if (warlockTank && warlockTank != bot &&
        bot->GetExactDist2d(warlockTank) < LEOTHERAS_CHAOS_BLAST_SAFE_DISTANCE)
    {
        return warlockTank;
    }

    return nullptr;
}

Unit* GetShadowTargetToSeparateFrom(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Creature* leotherasHumanoid = GetActiveLeotherasHumanoid(botAI);
    if (!leotherasHumanoid || leotherasHumanoid->GetVictim() != bot)
        return nullptr;

    Creature* shadow = GetShadowOfLeotheras(botAI);
    if (!shadow)
        return nullptr;

    Unit* shadowVictim = shadow->GetVictim();
    if (!shadowVictim || shadowVictim == bot ||
        bot->GetExactDist2d(shadowVictim) >= LEOTHERAS_SHADOW_SEPARATION_DISTANCE)
    {
        return nullptr;
    }

    return shadowVictim;
}

bool IsLeotherasDpsHoldActive(PlayerbotAI* botAI, Unit* leotheras)
{
    if (!leotheras)
        return false;

    Player* bot = botAI->GetBot();
    SscInstanceState const& state = SscState(leotheras->GetInstanceId());
    uint32 const now = getMSTime();

    auto const isJustAfterWhirlwind = [&state, now]()
    {
        std::optional<uint32> const& whirlwindEnd = state.leotherasWhirlwindEndTime;
        if (!whirlwindEnd || now < *whirlwindEnd)
            return false;

        return now - *whirlwindEnd < LEOTHERAS_WHIRLWIND_DPS_WAIT_MS;
    };

    if (IsLeotherasHumanoidPhase(botAI))
    {
        if (PlayerbotAI::IsTank(bot))
            return false;

        std::optional<uint32> const& start = state.leotherasHumanoidPhaseStartTime;
        if (!start || getMSTimeDiff(*start, now) < LEOTHERAS_HUMANOID_DPS_WAIT_MS)
        {
            return true;
        }

        return isJustAfterWhirlwind();
    }

    if (IsLeotherasDemonPhase(botAI))
    {
        if (IsLeotherasWarlockTank(bot))
            return false;

        if (PlayerbotAI::IsTank(bot) && !GetLeotherasWarlockTank(bot))
            return false;

        std::optional<uint32> const& start = state.leotherasDemonPhaseStartTime;
        if (!start)
            return true;

        return getMSTimeDiff(*start, now) < LEOTHERAS_DEMON_DPS_WAIT_MS;
    }

    if (IsLeotherasFinalPhase(botAI))
    {
        if (PlayerbotAI::IsTank(bot) || IsLeotherasWarlockTank(bot))
            return false;

        std::optional<uint32> const& start = state.leotherasFinalPhaseStartTime;
        if (!start || getMSTimeDiff(*start, now) < LEOTHERAS_FINAL_DPS_WAIT_MS)
        {
            return true;
        }

        return isJustAfterWhirlwind();
    }

    return false;
}

bool HasTooManyChaosBlastStacks(Player* bot)
{
    Aura* chaosBlast = bot->GetAura(Id(SscSpells::SPELL_CHAOS_BLAST));
    return chaosBlast && chaosBlast->GetStackAmount() >= 5;
}

bool HasInnerDemon(Player* bot)
{
    return bot->HasAura(Id(SscSpells::SPELL_INSIDIOUS_WHISPER));
}

Creature* GetPersonalInnerDemon(PlayerbotAI* botAI)
{
    ObjectGuid const botGuid = botAI->GetBot()->GetGUID();
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& targets = context->GetValue<GuidVector>("possible targets no los")->RefGet();

    Creature* innerDemon = nullptr;
    for (ObjectGuid const& guid : targets)
    {
        Creature* creature = botAI->GetCreature(guid);
        if (creature && creature->GetEntry() == Id(SscNpcs::NPC_INNER_DEMON) &&
            creature->GetSummonerGUID() == botGuid)
        {
            innerDemon = creature;
            break;
        }
    }

    return innerDemon;
}

// Fathom-Lord Karathress

ObjectGuid FindSpitfireTotemGuid(Player* bot)
{
    Creature* totem =
        bot->FindNearestCreature(Id(SscNpcs::NPC_SPITFIRE_TOTEM), SPITFIRE_TOTEM_SEARCH_DISTANCE);
    return totem ? totem->GetGUID() : ObjectGuid::Empty;
}

Creature* GetSpitfireTotem(PlayerbotAI* botAI)
{
    return GetCachedCreature(botAI, "ssc spitfire totem");
}

bool ShouldAttackSpitfireTotem(Player* bot, Unit* totem)
{
    return totem && (PlayerbotAI::IsMelee(bot) ||
        bot->GetDistance(totem) < SPITFIRE_TOTEM_RANGED_ATTACK_DISTANCE);
}

namespace // Karathress
{

// GetGroupAssistTank does not allow dead tanks to be indexed, so this helper serves that purpose.
Player* GetCouncilTank(Player* bot, int8 assistTankIndex)
{
    if (assistTankIndex < 0)
        return GetGroupMainTank(bot);

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() &&
            PlayerbotAI::IsAssistTankOfIndex(member, assistTankIndex, false))
        {
            return member;
        }
    }

    return nullptr;
}

Unit* GetAssignedCouncilMember(PlayerbotAI* botAI)
{
    Player* tank = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (KarathressCouncilAssignment const& assignment : KARATHRESS_COUNCIL)
    {
        bool const assigned = assignment.assistTankIndex < 0 ?
            PlayerbotAI::IsMainTank(tank) :
            PlayerbotAI::IsAssistTankOfIndex(tank, assignment.assistTankIndex, false);
        if (assigned)
            return AI_VALUE2(Unit*, "find target", assignment.name);
    }

    return nullptr;
}

} // end anonymous namespace (Karathress)

bool IsHoldingAnotherTanksCouncilMember(PlayerbotAI* botAI, Unit* ownTarget)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (KarathressCouncilAssignment const& assignment : KARATHRESS_COUNCIL)
    {
        Unit* member = AI_VALUE2(Unit*, "find target", assignment.name);
        if (!member || member == ownTarget || member->GetVictim() != bot)
            continue;

        Player* tank = GetCouncilTank(bot, assignment.assistTankIndex);
        if (tank && tank != bot)
            return true;
    }

    return false;
}

bool IsAnotherCouncilMemberWithin(PlayerbotAI* botAI, float range)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* ownMember = GetAssignedCouncilMember(botAI);
    for (KarathressCouncilAssignment const& assignment : KARATHRESS_COUNCIL)
    {
        Unit* member = AI_VALUE2(Unit*, "find target", assignment.name);
        if (member && member != ownMember && bot->GetDistance(member) < range)
            return true;
    }

    return false;
}

Unit* GetSharkkisTankTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis && sharkkis->GetVictim() != bot)
        return sharkkis;

    Unit* heldPet = nullptr;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* pet = ref->GetOwner();
        if (!pet || !pet->IsAlive())
            continue;

        uint32 const entry = pet->GetEntry();
        if (entry != Id(SscNpcs::NPC_FATHOM_LURKER) && entry != Id(SscNpcs::NPC_FATHOM_SPOREBAT))
            continue;

        if (pet->GetVictim() != bot)
            return pet;

        heldPet = pet;
    }

    if (sharkkis)
        return sharkkis;

    return heldPet;
}

Unit* GetSharkkisPet(Player* bot)
{
    Unit* lurker = nullptr;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* pet = ref->GetOwner();
        if (!pet || !pet->IsAlive())
            continue;

        uint32 const entry = pet->GetEntry();
        if (entry == Id(SscNpcs::NPC_FATHOM_SPOREBAT))
            return pet;

        if (entry == Id(SscNpcs::NPC_FATHOM_LURKER))
            lurker = pet;
    }

    return lurker;
}

// Morogrim Tidewalker

Position GetTidewalkerStackPoint(Player const& bot, Unit const& tidewalker)
{
    Unit* victim = tidewalker.GetVictim();
    float const behindAngle = (victim ? tidewalker.GetAngle(victim) :
        tidewalker.GetOrientation()) + static_cast<float>(M_PI);
    float const behindDistance = bot.getClass() == CLASS_HUNTER ?
        TIDEWALKER_HUNTER_BEHIND_DISTANCE : TIDEWALKER_RANGED_BEHIND_DISTANCE;

    return Position(
        tidewalker.GetPositionX() + std::cos(behindAngle) * behindDistance,
        tidewalker.GetPositionY() + std::sin(behindAngle) * behindDistance,
        tidewalker.GetPositionZ());
}

// Lady Vashj <Coilfang Matron>

namespace // Vashj
{

// Even-odd ray cast in 2D.
template <std::size_t N>
bool IsInPolygon(float x, float y, std::array<Position, N> const& polygon)
{
    bool inside = false;
    for (std::size_t i = 0, j = N - 1; i < N; j = i++)
    {
        float const xi = polygon[i].GetPositionX();
        float const yi = polygon[i].GetPositionY();
        float const xj = polygon[j].GetPositionX();
        float const yj = polygon[j].GetPositionY();
        if ((yi > y) != (yj > y) && x < (xj - xi) * (y - yi) / (yj - yi) + xi)
            inside = !inside;
    }

    return inside;
}

template <std::size_t N>
float DistanceToPolygonOutline(float x, float y, std::array<Position, N> const& polygon)
{
    float closest = std::numeric_limits<float>::max();
    for (std::size_t i = 0, j = N - 1; i < N; j = i++)
    {
        float const ax = polygon[j].GetPositionX();
        float const ay = polygon[j].GetPositionY();
        float const dx = polygon[i].GetPositionX() - ax;
        float const dy = polygon[i].GetPositionY() - ay;
        float const lengthSq = dx * dx + dy * dy;
        float const t = lengthSq > 0.0f ?
            std::clamp(((x - ax) * dx + (y - ay) * dy) / lengthSq, 0.0f, 1.0f) : 0.0f;
        closest = std::min(closest, std::hypot(x - (ax + t * dx), y - (ay + t * dy)));
    }

    return closest;
}

float SegmentLengthInCircle(
    Position const& a, Position const& b, Position const& center, float radius)
{
    float const dx = b.GetPositionX() - a.GetPositionX();
    float const dy = b.GetPositionY() - a.GetPositionY();
    float const fx = a.GetPositionX() - center.GetPositionX();
    float const fy = a.GetPositionY() - center.GetPositionY();

    // |a + t(b - a) - center| = radius, for t along the segment.
    float const qa = dx * dx + dy * dy;
    float const qb = 2.0f * (fx * dx + fy * dy);
    float const qc = fx * fx + fy * fy - radius * radius;
    float const discriminant = qb * qb - 4.0f * qa * qc;
    if (qa <= 0.0f || discriminant <= 0.0f)
        return 0.0f;

    float const root = std::sqrt(discriminant);
    float const enter = std::max(0.0f, (-qb - root) / (2.0f * qa));
    float const leave = std::min(1.0f, (-qb + root) / (2.0f * qa));
    return leave > enter ? (leave - enter) * std::sqrt(qa) : 0.0f;
}

bool IsVashjLineOnDais(
    Position const& a, Position const& b, float margin, float rockClearance)
{
    constexpr float sampleSpacing = 2.0f;
    uint8 const samples = static_cast<uint8>(a.GetExactDist2d(b) / sampleSpacing);
    for (uint8 s = 1; s < samples; ++s)
    {
        float const t = static_cast<float>(s) / samples;
        if (!IsOnVashjDais(
                a.GetPositionX() + (b.GetPositionX() - a.GetPositionX()) * t,
                a.GetPositionY() + (b.GetPositionY() - a.GetPositionY()) * t, margin,
                rockClearance))
        {
            return false;
        }
    }

    return true;
}

// Perpendicular to the nearest edge of the dais (which is a perfect dodecagon).
float GetVashjEdgeNormalDistance(float x, float y)
{
    float const dx = x - VASHJ_PLATFORM_CENTER_POSITION.GetPositionX();
    float const dy = y - VASHJ_PLATFORM_CENTER_POSITION.GetPositionY();

    constexpr float sector = static_cast<float>(M_PI) / 6.0f;
    float const angle = Position::NormalizeOrientation(std::atan2(dy, dx));
    float const offset = std::fmod(angle, sector) - sector / 2.0f;
    return std::hypot(dx, dy) * std::cos(offset);
}

template <std::size_t N>
bool SegmentCrossesPolygon(
    Position const& a, Position const& b, std::array<Position, N> const& polygon)
{
    // Positive when r is left of the line from p to q.
    auto side = [](Position const& p, Position const& q, Position const& r)
    {
        return (q.GetPositionX() - p.GetPositionX()) * (r.GetPositionY() - p.GetPositionY()) -
            (q.GetPositionY() - p.GetPositionY()) * (r.GetPositionX() - p.GetPositionX());
    };

    for (std::size_t i = 0, j = N - 1; i < N; j = i++)
    {
        Position const& c = polygon[j];
        Position const& d = polygon[i];
        if ((side(c, d, a) > 0.0f) != (side(c, d, b) > 0.0f) &&
            (side(a, b, c) > 0.0f) != (side(a, b, d) > 0.0f))
        {
            return true;
        }
    }

    return false;
}

float GetCastRingRadius(Player* bot, Unit* target, float castRange)
{
    constexpr float margin = 2.0f;
    return castRange + bot->GetCombatReach() + target->GetCombatReach() - margin;
}

Position const& GetVashjStationPosition(VashjStationSlot const& slot)
{
    VashjStation const& station = VASHJ_STATIONS[slot.station];
    return slot.slot == VASHJ_STATION_HEALER_SLOT ? station.healer : station.ranged[slot.slot];
}

std::vector<Player*> GetVashjStationRanged(Player* bot, int8 station)
{
    std::vector<Player*> ranged;
    std::optional<VashjStationHolders> const& holders =
        SscState(bot->GetInstanceId()).vashjStationHolders;
    if (!holders || station < 0)
        return ranged;

    for (size_t slot = 0; slot < VASHJ_STATION_RANGED_SLOTS; ++slot)
    {
        Player* holder = ObjectAccessor::GetPlayer(*bot, (*holders)[station][slot]);
        if (holder && holder->IsAlive())
            ranged.push_back(holder);
    }

    return ranged;
}

Player* GetVashjStationHealer(Player* bot, int8 station)
{
    std::optional<VashjStationHolders> const& holders =
        SscState(bot->GetInstanceId()).vashjStationHolders;
    if (!holders || station < 0)
        return nullptr;

    Player* holder =
        ObjectAccessor::GetPlayer(*bot, (*holders)[station][VASHJ_STATION_HEALER_SLOT]);
    return holder && holder->IsAlive() ? holder : nullptr;
}

std::vector<GameObject*> GetUsableVashjGenerators(Map* map)
{
    std::vector<GameObject*> generators;
    if (!map)
        return generators;

    for (uint32 const spawnId : VASHJ_SHIELD_GENERATOR_SPAWN_IDS)
    {
        auto const bounds = map->GetGameObjectBySpawnIdStore().equal_range(spawnId);
        if (bounds.first == bounds.second)
            continue;

        GameObject* generator = bounds.first->second;
        // A used generator retains GO_STATE_READY.
        if (generator && !generator->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE))
            generators.push_back(generator);
    }

    return generators;
}

float GetVashjGroundZ(Map* map, uint32 phaseMask, float x, float y)
{
    constexpr float centreZ = 43.0f;
    constexpr float rimZ = 41.1f;
    constexpr float stairRun = 33.15f;
    constexpr float stairDrop = 18.9f;

    float const distance = GetVashjEdgeNormalDistance(x, y);
    if (distance > VASHJ_STAIR_BASE_APOTHEM)
        return INVALID_HEIGHT;

    float const slopeZ = distance <= VASHJ_DAIS_APOTHEM ?
        centreZ - (centreZ - rimZ) * distance / VASHJ_DAIS_APOTHEM :
        rimZ - (distance - VASHJ_DAIS_APOTHEM) / stairRun * stairDrop;

    constexpr float searchAbove = 2.0f;
    constexpr float maxSearch = 5.0f;
    return map->GetHeight(phaseMask, x, y, slopeZ + searchAbove, true, maxSearch);
}

bool IsVashjCoreSpotClear(float x, float y, bool useSpot)
{
    constexpr float stairBaseMargin = 2.0f;
    if (GetVashjEdgeNormalDistance(x, y) > VASHJ_STAIR_BASE_APOTHEM - stairBaseMargin)
        return false;

    if (IsInPolygon(x, y, VASHJ_NORTH_ROCK) ||
        DistanceToPolygonOutline(x, y, VASHJ_NORTH_ROCK) < VASHJ_STANDING_ROCK_CLEARANCE ||
        IsInPolygon(x, y, VASHJ_SOUTH_WEST_ROCK) ||
        DistanceToPolygonOutline(x, y, VASHJ_SOUTH_WEST_ROCK) < VASHJ_STANDING_ROCK_CLEARANCE)
    {
        return false;
    }

    for (Position const& spawn : VASHJ_ADD_SPAWN_POSITIONS)
    {
        if (spawn.GetExactDist2d(x, y) < VASHJ_CORE_SPOT_SPAWN_CLEARANCE)
            return false;
    }

    if (!useSpot)
    {
        for (Position const& generator : VASHJ_SHIELD_GENERATOR_POSITIONS)
        {
            if (generator.GetExactDist2d(x, y) < VASHJ_CORE_SPOT_GENERATOR_CLEARANCE)
                return false;
        }
    }

    return true;
}

bool IsVashjCoreThrowInSight(
    Map* map, uint32 phaseMask, Position const& from, float fromEyeHeight, Position const& to)
{
    return map->isInLineOfSight(
        from.GetPositionX(), from.GetPositionY(), from.GetPositionZ() + fromEyeHeight,
        to.GetPositionX(), to.GetPositionY(), to.GetPositionZ() + VASHJ_CORE_PLAN_EYE_HEIGHT,
        phaseMask, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing);
}

bool FindVashjGeneratorUseSpot(GameObject* generator, uint32 phaseMask, float angle, Position& spot)
{
    constexpr float useSpotMargin = 0.5f;
    float const useRange = generator->GetInteractionDistance() - useSpotMargin; // 4.5y
    constexpr float stepLength = 0.25f;
    constexpr int steps = 48;

    for (int step = steps; step > 0; --step)
    {
        float const reach = step * stepLength;
        float const x = generator->GetPositionX() + std::cos(angle) * reach;
        float const y = generator->GetPositionY() + std::sin(angle) * reach;
        if (!IsVashjCoreSpotClear(x, y, true))
            continue;

        float const z = GetVashjGroundZ(generator->GetMap(), phaseMask, x, y);
        if (z <= INVALID_HEIGHT)
            continue;

        Position const candidate(x, y, z);
        if (generator->IsAtInteractDistance(candidate, useRange))
        {
            spot = candidate;
            return true;
        }
    }

    return false;
}

bool PlanVashjCoreLegs(
    Map* map, uint32 phaseMask, std::vector<Position> const& useSpots, Position const& from,
    float leg, float fromEyeHeight, std::vector<Position>& spots)
{
    for (Position const& useSpot : useSpots)
    {
        if (from.GetExactDist(useSpot) <= leg &&
            IsVashjCoreThrowInSight(map, phaseMask, from, fromEyeHeight, useSpot))
        {
            spots.push_back(useSpot);
            return true;
        }
    }

    if (spots.size() + 2 > VASHJ_CORE_MAX_CATCHERS)
        return false;

    constexpr float minGain = 5.0f;
    constexpr int reaches = 13;
    constexpr float reachStep = 3.0f;
    constexpr int angles = 6;
    constexpr float angleStep = static_cast<float>(M_PI) / 18.0f;

    Position const& aim = useSpots.front();
    float const toAim = from.GetAngle(&aim);
    float const maxDistance = from.GetExactDist2d(aim) - minGain;
    std::vector<std::pair<float, Position>> candidates;

    for (int r = 0; r < reaches; ++r)
    {
        float const reach = leg - r * reachStep;
        if (reach <= minGain)
            break;

        for (int i = 0; i <= angles; ++i)
        {
            for (int sign : { 1, -1 })
            {
                if (i == 0 && sign < 0)
                    continue;

                float const angle = toAim + sign * i * angleStep;
                float const x = from.GetPositionX() + std::cos(angle) * reach;
                float const y = from.GetPositionY() + std::sin(angle) * reach;
                float const distance = aim.GetExactDist2d(x, y);
                if (distance >= maxDistance || !IsVashjCoreSpotClear(x, y, false))
                    continue;

                float const z = GetVashjGroundZ(map, phaseMask, x, y);
                if (z <= INVALID_HEIGHT)
                    continue;

                Position const candidate(x, y, z);
                if (from.GetExactDist(candidate) <= leg)
                    candidates.emplace_back(distance, candidate);
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(),
        [](auto const& a, auto const& b) { return a.first < b.first; });

    constexpr size_t branches = 3;
    size_t followed = 0;
    for (auto const& entry : candidates)
    {
        Position const& candidate = entry.second;
        if (!IsVashjCoreThrowInSight(map, phaseMask, from, fromEyeHeight, candidate))
            continue;

        spots.push_back(candidate);
        if (PlanVashjCoreLegs(map, phaseMask, useSpots, candidate, VASHJ_CORE_THROW_PLAN_DISTANCE,
                VASHJ_CORE_PLAN_EYE_HEIGHT, spots))
        {
            return true;
        }

        spots.pop_back();
        if (++followed == branches)
            break;
    }

    return false;
}

std::vector<Position> PlanVashjCoreSpots(
    Player* bot, GameObject* generator, Position const& origin, float firstLeg,
    float originEyeHeight)
{
    uint32 const phaseMask = bot->GetPhaseMask();

    constexpr int useAngles = 6;
    constexpr float useAngleStep = static_cast<float>(M_PI) / 12.0f;
    float const toOrigin = generator->GetAngle(&origin);
    std::vector<Position> useSpots;
    for (int i = 0; i <= useAngles; ++i)
    {
        for (int sign : { 1, -1 })
        {
            if (i == 0 && sign < 0)
                continue;

            Position spot;
            float const angle = toOrigin + sign * i * useAngleStep;
            if (FindVashjGeneratorUseSpot(generator, phaseMask, angle, spot))
                useSpots.push_back(spot);
        }
    }

    std::vector<Position> spots;
    if (useSpots.empty() || !PlanVashjCoreLegs(bot->GetMap(), phaseMask, useSpots, origin,
            firstLeg, originEyeHeight, spots))
    {
        return {};
    }

    return spots;
}

GameObject* PlanVashjCoreRoute(
    Player* bot, Position const& origin, float firstLeg, float originEyeHeight,
    ObjectGuid preferred, std::vector<Position>& spots)
{
    std::vector<GameObject*> generators = GetUsableVashjGenerators(bot->GetMap());
    std::sort(generators.begin(), generators.end(), [&](GameObject* a, GameObject* b)
    {
        bool const aPreferred = a->GetGUID() == preferred;
        if (aPreferred != (b->GetGUID() == preferred))
            return aPreferred;

        return origin.GetExactDist2d(a) < origin.GetExactDist2d(b);
    });

    for (GameObject* generator : generators)
    {
        spots = PlanVashjCoreSpots(bot, generator, origin, firstLeg, originEyeHeight);
        if (!spots.empty())
            return generator;
    }

    return nullptr;
}

Player* FindVashjCoreCatcher(
    Player* bot, VashjCorePassingChain const& chain, Position const& spot, bool attackersOnly,
    ObjectGuid excluded)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != SSC_MAP_ID ||
            !GET_PLAYERBOT_AI(member) || PlayerbotAI::IsTank(member))
        {
            continue;
        }

        if (member->GetGUID() == chain.originBot || member->GetGUID() == excluded ||
            member->GetGUID() == chain.excluded || GetVashjCoreCatcherIndex(chain, member) >= 0)
        {
            continue;
        }

        float const distance = member->GetExactDist2d(spot);
        if (distance >= nearestDistance ||
            (attackersOnly && !GetTaintedElementalToKill(member)) || HasTaintedCore(member))
        {
            continue;
        }

        nearestDistance = distance;
        nearest = member;
    }

    return nearest;
}

void SetVashjCoreCatcher(VashjCoreCatcher& catcher, Player* player)
{
    catcher.bot = player ? player->GetGUID() : ObjectGuid::Empty;
    catcher.prepositions = !player || !GetTaintedElementalToKill(player);
    catcher.releaseTime = getMSTime();
    catcher.readyDelay = urand(1000, 2000);
    catcher.arrived = false;
}

void AssignVashjCoreCatchers(Player* bot, VashjCorePassingChain& chain)
{
    if (chain.catchers.empty())
        return;

    VashjCoreCatcher& first = chain.catchers.front();
    Player* player = FindVashjCoreCatcher(bot, chain, first.spot, true, ObjectGuid::Empty);
    if (!player)
        player = FindVashjCoreCatcher(bot, chain, first.spot, false, ObjectGuid::Empty);

    SetVashjCoreCatcher(first, player);
    first.released = true;
}

void ResetVashjCoreThrows(VashjCorePassingChain& chain)
{
    chain.reached = -1;
    chain.throwTarget.Clear();
    chain.failedThrows = 0;
    chain.waitTarget.Clear();
    chain.waitStart = 0;
    chain.blockedStart = 0;
}

bool IsVashjCorePassingChainLive(Player* bot, VashjCorePassingChain const& chain)
{
    if (chain.failed)
        return false;

    // The core lands in the catcher's bags at once; this covers the tick or so before its own
    // checks see it.
    constexpr uint32 throwGraceMs = 3 * IN_MILLISECONDS;
    if (chain.throwTime && getMSTimeDiff(chain.throwTime, getMSTime()) < throwGraceMs)
        return true;

    Creature* tainted = ObjectAccessor::GetCreature(*bot, chain.tainted);
    if (tainted && IsTaintedCoreStillToLoot(tainted))
        return true;

    ObjectGuid const holder =
        chain.reached >= 0 ? chain.catchers[chain.reached].bot : chain.originBot;
    for (ObjectGuid const guid : { holder, chain.throwTarget })
    {
        Player* player = ObjectAccessor::GetPlayer(*bot, guid);
        if (player && HasTaintedCore(player))
            return true;
    }

    return false;
}

} // end anonymous namespace (Vashj)

// Vashj: General

int8 GetLadyVashjPhase(Unit* vashj)
{
    if (!vashj)
        return -1;

    float const healthPct = vashj->GetHealthPct();

    if (healthPct > 70.0f)
        return 1;

    if (vashj->HasAura(Id(SscSpells::SPELL_MAGIC_BARRIER)))
        return 2;

    // Phase 3, else transitioning from Phase 1 to Phase 2.
    return healthPct <= 50.0f ? 3 : 0;
}

bool IsOnVashjDais(float x, float y, float margin, float rockClearance)
{
    return GetVashjEdgeNormalDistance(x, y) <= VASHJ_DAIS_APOTHEM - margin &&
        !IsInPolygon(x, y, VASHJ_NORTH_ROCK) &&
        DistanceToPolygonOutline(x, y, VASHJ_NORTH_ROCK) >= rockClearance;
}

// Vashj: Static Charge, Entangle and Shock Blast

bool HasVashjStaticCharge(Player* player)
{
    return player && player->HasAura(Id(SscSpells::SPELL_STATIC_CHARGE));
}

bool IsVashjPhase3RangedTooClose(Player* bot, Unit* vashj)
{
    if (!vashj)
        return false;

    if (bot->GetExactDist2d(vashj) < VASHJ_PHASE_3_RANGED_DISTANCE)
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->GetMapId() == SSC_MAP_ID &&
            bot->GetExactDist2d(member) < VASHJ_PHASE_3_RANGED_SPREAD_DISTANCE)
        {
            return true;
        }
    }

    return false;
}

bool ShouldAvoidVashjStaticCharge(Player* bot, Unit* vashj)
{
    if (!vashj)
        return false;

    Player* vashjVictim = vashj->GetVictim() ? vashj->GetVictim()->ToPlayer() : nullptr;
    if (bot == vashjVictim)
        return false;

    return HasVashjStaticCharge(bot) || (vashjVictim && HasVashjStaticCharge(vashjVictim));
}

bool IsInVashjStaticChargeReach(Player* bot, Unit* vashj)
{
    if (!ShouldAvoidVashjStaticCharge(bot, vashj))
        return false;

    if (HasVashjStaticCharge(bot))
        return GetNearestPlayerInRadius(bot, VASHJ_STATIC_CHARGE_SAFE_DISTANCE);

    return bot->GetExactDist2d(vashj->GetVictim()) < VASHJ_STATIC_CHARGE_SAFE_DISTANCE;
}

Player* GetVashjHandOfFreedomTarget(PlayerbotAI* botAI, Unit* vashj)
{
    if (!vashj)
        return nullptr;

    int8 const phase = GetLadyVashjPhase(vashj);
    if (phase != 1 && phase != 3)
        return nullptr;

    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* const skip = phase == 1 ? vashj->GetVictim() : nullptr;
    std::vector<Position> const& spores = GetToxicSporePositions(botAI);

    Player* mainTank = nullptr;
    bool mainTankKnown = false;
    auto isMainTank = [&](Player* member)
    {
        if (!mainTankKnown)
        {
            mainTank = GetGroupMainTank(bot);
            mainTankKnown = true;
        }

        return member == mainTank;
    };

    Player* inSpores = nullptr;
    Player* withStaticCharge = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == skip || !member->IsAlive() ||
            !member->HasAura(Id(SscSpells::SPELL_ENTANGLE)) || !PlayerbotAI::IsMelee(member))
        {
            continue;
        }

        bool const nearSpore = std::any_of(spores.begin(), spores.end(),
            [member](Position const& spore)
            {
                return member->GetExactDist2d(spore) < TOXIC_SPORES_HAZARD_RADIUS;
            });

        if (nearSpore && (!inSpores || isMainTank(member)))
            inSpores = member;

        if (HasVashjStaticCharge(member) && (!withStaticCharge || isMainTank(member)))
            withStaticCharge = member;
    }

    return inSpores ? inSpores : withStaticCharge;
}

Player* GetVashjGroundingShaman(Player* bot)
{
    std::optional<ObjectGuid> const& shamanGuid =
        SscState(bot->GetInstanceId()).vashjGroundingShaman;
    if (!shamanGuid)
        return nullptr;

    Player* shaman = ObjectAccessor::GetPlayer(*bot, *shamanGuid);
    return shaman && shaman->IsAlive() ? shaman : nullptr;
}

Player* FindVashjGroundingShaman(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->getClass() == CLASS_SHAMAN && member->IsAlive() &&
            member->GetMapId() == SSC_MAP_ID && group->SameSubGroup(mainTank, member) &&
            GET_PLAYERBOT_AI(member))
        {
            return member;
        }
    }

    return nullptr;
}

// Vashj: Toxic Spores

std::vector<Position> const& GetToxicSporePositions(PlayerbotAI* botAI)
{
    return GetCachedHazardPositions(botAI, "ssc toxic spores");
}

bool FindVashjDaisStepAwayFromPositions(
    Player* bot, std::vector<Position> const& positions, Unit* facing, float rockClearance,
    float& stepX, float& stepY, float& stepZ, bool& backwards,
    std::vector<Position> const* spores, float sporeRadius)
{
    constexpr uint8 directions = 24;

    auto closestPosition = [&positions](float x, float y)
    {
        float closest = std::numeric_limits<float>::max();
        for (Position const& position : positions)
            closest = std::min(closest, position.GetExactDist2d(x, y));

        return closest;
    };

    auto nearSpore = [spores, sporeRadius](float x, float y)
    {
        return spores && std::any_of(spores->begin(), spores->end(),
            [x, y, sporeRadius](Position const& spore)
            {
                return spore.GetExactDist2d(x, y) < sporeRadius;
            });
    };

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    std::vector<std::pair<float, float>> candidates;
    for (uint8 i = 0; i < directions; ++i)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * i / directions;
        float const x = botX + std::cos(angle) * PATH_STEP_DISTANCE;
        float const y = botY + std::sin(angle) * PATH_STEP_DISTANCE;
        if (IsOnVashjDais(x, y, VASHJ_DAIS_MARGIN, rockClearance) && !nearSpore(x, y))
            candidates.emplace_back(angle, closestPosition(x, y));
    }

    std::sort(candidates.begin(), candidates.end(),
        [](auto const& a, auto const& b) { return a.second > b.second; });

    bool const isTanking = facing && facing->GetVictim() == bot;
    float const current = closestPosition(botX, botY);
    for (auto const& [angle, closest] : candidates)
    {
        if (closest <= current)
            break;

        float const dirX = std::cos(angle);
        float const dirY = std::sin(angle);
        backwards = isTanking && dirX * (facing->GetPositionX() - botX) +
            dirY * (facing->GetPositionY() - botY) < 0.0f;

        float const moveDist = backwards ? PATH_BACKWARD_STEP_DISTANCE : PATH_STEP_DISTANCE;
        if (CanTakeStepTowards(
                bot, botX + dirX * PATH_STEP_DISTANCE, botY + dirY * PATH_STEP_DISTANCE,
                moveDist, stepX, stepY, stepZ))
        {
            return true;
        }
    }

    return false;
}

bool FindVashjDaisStepAwayFromUnits(
    Player* bot, std::vector<Unit*> const& units, Unit* facing, float rockClearance,
    float& stepX, float& stepY, float& stepZ, bool& backwards,
    std::vector<Position> const* spores, float sporeRadius)
{
    std::vector<Position> positions;
    positions.reserve(units.size());
    for (Unit* unit : units)
        positions.push_back(unit->GetPosition());

    return FindVashjDaisStepAwayFromPositions(
        bot, positions, facing, rockClearance, stepX, stepY, stepZ, backwards, spores,
        sporeRadius);
}

bool FindVashjTankBreakoutSpot(
    Player* bot, std::vector<Position> const& spores, Position& spot)
{
    constexpr uint8 directions = 24;
    constexpr uint8 rings = 7;
    constexpr float ringSpacing = 5.0f;
    // 1y through a pool backwards takes about 0.7s, dealing 2775-3225 nature damage per second.
    constexpr float poolYardCost = 3.0f;

    Position const from = bot->GetPosition();
    float bestCost = std::numeric_limits<float>::max();
    bool found = false;

    for (uint8 ring = 1; ring <= rings; ++ring)
    {
        float const radius = ringSpacing * ring;
        for (uint8 i = 0; i < directions; ++i)
        {
            float const angle = 2.0f * static_cast<float>(M_PI) * i / directions;
            Position const candidate(
                from.GetPositionX() + std::cos(angle) * radius,
                from.GetPositionY() + std::sin(angle) * radius, from.GetPositionZ());

            if (!IsOnVashjDais(candidate.GetPositionX(), candidate.GetPositionY(),
                    VASHJ_DAIS_MARGIN, VASHJ_NORTH_ROCK_CLEARANCE) ||
                std::any_of(spores.begin(), spores.end(), [&candidate](Position const& spore)
                {
                    return spore.GetExactDist2d(candidate) < TOXIC_SPORES_TANK_AVOID_RADIUS;
                }))
            {
                continue;
            }

            if (!IsVashjLineOnDais(from, candidate, VASHJ_DAIS_MARGIN, VASHJ_NORTH_ROCK_CLEARANCE))
                continue;

            float inPools = 0.0f;
            for (Position const& spore : spores)
            {
                inPools += SegmentLengthInCircle(
                    from, candidate, spore, TOXIC_SPORES_HAZARD_RADIUS);
            }

            float const cost = radius + poolYardCost * inPools;
            if (cost < bestCost)
            {
                bestCost = cost;
                spot = candidate;
                found = true;
            }
        }
    }

    return found;
}

bool IsVashjRingMelee(Player* bot, Unit* vashj)
{
    if (!vashj)
        return false;

    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsTank(bot) || HasVashjStaticCharge(bot))
        return false;

    Player* vashjVictim = vashj->GetVictim() ? vashj->GetVictim()->ToPlayer() : nullptr;
    return !vashjVictim || !HasVashjStaticCharge(vashjVictim);
}

bool IsNearToxicSpores(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    std::vector<Position> const& spores = GetToxicSporePositions(botAI);
    return std::any_of(spores.begin(), spores.end(), [bot, radius](Position const& spore)
    {
        return bot->GetExactDist2d(spore) < radius;
    });
}

bool IsInMeleeRangeClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius)
{
    return target && bot->IsWithinMeleeRange(target) &&
        IsOnVashjDais(bot->GetPositionX(), bot->GetPositionY(), VASHJ_DAIS_MARGIN,
            VASHJ_STANDING_ROCK_CLEARANCE) &&
        std::none_of(spores.begin(), spores.end(), [bot, radius](Position const& spore)
        {
            return bot->GetExactDist2d(spore) < radius;
        });
}

bool CanWalkThroughToxicSpores(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            return bot->HasAura(Id(SscSpells::SPELL_DIVINE_SHIELD));
        case CLASS_PRIEST:
            return bot->HasAura(Id(SscSpells::SPELL_DISPERSION));
        default:
            return false;
    }
}

bool GetMeleeRingStepClearOfSpores(
    Player* bot, Unit* target, std::vector<Position> const& spores, float radius, float& stepX,
    float& stepY, float& stepZ)
{
    if (!target || IsInMeleeRangeClearOfSpores(bot, target, spores, radius))
        return false;

    constexpr float meleeRangeInset = 1.0f;
    float const meleeRange = bot->GetMeleeRange(target);
    float const ringRadius = meleeRange - meleeRangeInset;
    float const targetX = target->GetPositionX();
    float const targetY = target->GetPositionY();

    std::vector<Position> nearby;
    for (Position const& spore : spores)
    {
        if (spore.GetExactDist2d(targetX, targetY) < meleeRange + radius)
            nearby.push_back(spore);
    }

    auto isClear = [&nearby, radius](float x, float y)
    {
        return IsOnVashjDais(x, y, VASHJ_DAIS_MARGIN, VASHJ_STANDING_ROCK_CLEARANCE) &&
            std::none_of(nearby.begin(), nearby.end(), [x, y, radius](Position const& spore)
            {
                return spore.GetExactDist2d(x, y) < radius;
            });
    };

    float const botAngle = std::atan2(
        bot->GetPositionY() - targetY, bot->GetPositionX() - targetX);
    constexpr uint8 samplesPerSide = 36;
    constexpr float sampleAngle = static_cast<float>(M_PI) / samplesPerSide;
    for (uint8 i = 0; i <= samplesPerSide; ++i)
    {
        for (int8 side = 1; side >= -1; side -= 2)
        {
            if ((i == 0 || i == samplesPerSide) && side < 0)
                continue;

            float const angle = botAngle + side * sampleAngle * i;
            float const x = targetX + std::cos(angle) * ringRadius;
            float const y = targetY + std::sin(angle) * ringRadius;
            if (!isClear(x, y))
                continue;

            constexpr float arrivalDistance = 0.5f;
            if (bot->GetExactDist2d(x, y) <= arrivalDistance)
                return false;

            if (CanTakeStepTowards(bot, x, y, PATH_STEP_DISTANCE, stepX, stepY, stepZ))
                return true;
        }
    }

    return false;
}

bool GetStepOutOfNearestSpore(
    Player* bot, std::vector<Position> const& spores, float radius, float& stepX, float& stepY,
    float& stepZ)
{
    auto const nearest = std::min_element(spores.begin(), spores.end(),
        [bot](Position const& a, Position const& b)
        {
            return bot->GetExactDist2dSq(a) < bot->GetExactDist2dSq(b);
        });

    if (nearest == spores.end())
        return false;

    float const sporeX = nearest->GetPositionX();
    float const sporeY = nearest->GetPositionY();
    float const botAngle = bot->GetExactDist2d(sporeX, sporeY) > 0.1f ?
        std::atan2(bot->GetPositionY() - sporeY, bot->GetPositionX() - sporeX) :
        bot->GetOrientation();

    constexpr uint8 samplesPerSide = 36;
    constexpr float sampleAngle = static_cast<float>(M_PI) / samplesPerSide;
    for (uint8 i = 0; i <= samplesPerSide; ++i)
    {
        for (int8 side = 1; side >= -1; side -= 2)
        {
            if ((i == 0 || i == samplesPerSide) && side < 0)
                continue;

            float const angle = botAngle + side * sampleAngle * i;
            float const x = sporeX + std::cos(angle) * radius;
            float const y = sporeY + std::sin(angle) * radius;
            if (IsOnVashjDais(x, y, VASHJ_DAIS_MARGIN, VASHJ_STANDING_ROCK_CLEARANCE) &&
                CanTakeStepTowards(bot, x, y, PATH_STEP_DISTANCE, stepX, stepY, stepZ))
            {
                return true;
            }
        }
    }

    return false;
}

bool GetVashjReachBlockedBySpores(PlayerbotAI* botAI, Unit*& target, float& range)
{
    Player* bot = botAI->GetBot();
    target = nullptr;
    range = 0.0f;

    if (!PlayerbotAI::IsCaster(bot) || HasVashjStaticCharge(bot) || CanWalkThroughToxicSpores(bot))
        return false;

    // ReachTargetAction will not interrupt a channeling spell, so apply the same treatment here.
    // The main two that should be completed are Evocation and Tranquility.
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    bool const isHealer = PlayerbotAI::IsHeal(bot);

    AiObjectContext* context = botAI->GetAiObjectContext();
    target = context->GetValue<Unit*>(isHealer ? "party member to heal" : "current target")->Get();
    range = botAI->GetRange(isHealer ? "heal" : "spell");
    if (!target || !target->IsAlive() || bot->IsWithinCombatRange(target, range))
        return false;

    float const ringRadius = GetCastRingRadius(bot, target, range);
    float const distance = bot->GetExactDist2d(target);
    if (distance <= ringRadius)
        return false;

    float const t = ringRadius / distance;
    Position const stop(
        target->GetPositionX() + (bot->GetPositionX() - target->GetPositionX()) * t,
        target->GetPositionY() + (bot->GetPositionY() - target->GetPositionY()) * t,
        bot->GetPositionZ());

    Position const from = bot->GetPosition();
    std::vector<Position> const& spores = GetToxicSporePositions(botAI);
    return std::any_of(spores.begin(), spores.end(), [&from, &stop](Position const& spore)
    {
        return SegmentLengthInCircle(from, stop, spore, TOXIC_SPORES_AVOID_RADIUS) > 0.0f;
    });
}

bool GetStepToCastRangeAroundSpores(
    Player* bot, Unit* target, float castRange, std::vector<Position> const& spores, float& stepX,
    float& stepY, float& stepZ)
{
    if (!target)
        return false;

    constexpr uint8 samples = 72;
    constexpr float poolYardCost = 3.0f;

    float const ringRadius = GetCastRingRadius(bot, target, castRange);
    Position const from = bot->GetPosition();
    bool const fromDais = IsOnVashjDais(from.GetPositionX(), from.GetPositionY(), 0.0f, 0.0f);
    float bestCost = std::numeric_limits<float>::max();
    float bestX = 0.0f;
    float bestY = 0.0f;
    bool found = false;

    for (uint8 i = 0; i < samples; ++i)
    {
        float const angle = 2.0f * static_cast<float>(M_PI) * i / samples;
        Position const candidate(
            target->GetPositionX() + std::cos(angle) * ringRadius,
            target->GetPositionY() + std::sin(angle) * ringRadius, from.GetPositionZ());

        if (!IsOnVashjDais(candidate.GetPositionX(), candidate.GetPositionY(), VASHJ_DAIS_MARGIN,
                VASHJ_STANDING_ROCK_CLEARANCE) ||
            std::any_of(spores.begin(), spores.end(), [&candidate](Position const& spore)
            {
                return spore.GetExactDist2d(candidate) < TOXIC_SPORES_AVOID_RADIUS;
            }))
        {
            continue;
        }

        if (fromDais ?
                !IsVashjLineOnDais(
                    from, candidate, VASHJ_DAIS_MARGIN, VASHJ_STANDING_ROCK_CLEARANCE) :
                SegmentCrossesPolygon(from, candidate, VASHJ_NORTH_ROCK) ||
                    SegmentCrossesPolygon(from, candidate, VASHJ_SOUTH_WEST_ROCK))
        {
            continue;
        }

        float const distance = from.GetExactDist2d(candidate);
        float inPools = 0.0f;
        for (Position const& spore : spores)
            inPools += SegmentLengthInCircle(from, candidate, spore, TOXIC_SPORES_AVOID_RADIUS);

        float const cost = distance + poolYardCost * inPools;
        if (cost < bestCost)
        {
            bestCost = cost;
            bestX = candidate.GetPositionX();
            bestY = candidate.GetPositionY();
            found = true;
        }
    }

    return found && CanTakeStepTowards(bot, bestX, bestY, PATH_STEP_DISTANCE, stepX, stepY, stepZ);
}

// Vashj: Phase 2 Ranged Stations

std::vector<VashjStationSlot> GetVashjStationFillOrder()
{
    std::vector<VashjStationSlot> order;
    for (size_t slot = 0; slot < VASHJ_STATION_RANGED_SLOTS; ++slot)
    {
        for (int8 station : VASHJ_STATION_FILL_ORDER)
            order.push_back({ station, static_cast<int8>(slot) });
    }

    for (int8 station : VASHJ_STATION_FILL_ORDER)
        order.push_back({ station, VASHJ_STATION_HEALER_SLOT });

    return order;
}

bool IsLiveVashjStationHolder(Player* bot, ObjectGuid guid)
{
    Player* holder = ObjectAccessor::GetPlayer(*bot, guid);
    return holder && holder->IsAlive();
}

bool HasVashjStationVacancy(Player* bot)
{
    std::optional<VashjStationHolders> const& holders =
        SscState(bot->GetInstanceId()).vashjStationHolders;
    if (!holders)
        return true;

    for (auto const& station : *holders)
    {
        for (ObjectGuid const& guid : station)
        {
            if (!guid.IsEmpty() && !IsLiveVashjStationHolder(bot, guid))
                return true;
        }
    }

    return false;
}

VashjStationSlot GetVashjStationSlot(Player* bot)
{
    VashjStationSlot result;
    std::optional<VashjStationHolders> const& holders =
        SscState(bot->GetInstanceId()).vashjStationHolders;
    if (!holders)
        return result;

    ObjectGuid const guid = bot->GetGUID();
    for (size_t station = 0; station < VASHJ_STATION_COUNT; ++station)
    {
        for (size_t slot = 0; slot <= VASHJ_STATION_RANGED_SLOTS; ++slot)
        {
            if ((*holders)[station][slot] == guid)
            {
                result.station = static_cast<int8>(station);
                result.slot = static_cast<int8>(slot);
                return result;
            }
        }
    }

    return result;
}

Position const* GetVashjStationPositionToReturnTo(Player* bot, Unit* currentTarget)
{
    VashjStationSlot const slot = GetVashjStationSlot(bot);
    if (slot.station < 0)
        return nullptr;

    Position const& stationPosition = GetVashjStationPosition(slot);
    if (bot->GetExactDist2d(stationPosition) <= VASHJ_STATION_ARRIVAL_DISTANCE)
        return nullptr;

    if (IsDesignatedCoreLooter(bot) && IsTaintedCoreStillToLoot(GetAssignedTaintedElemental(bot)))
        return nullptr;

    if (GetTaintedElementalToKill(bot))
        return nullptr;

    if (PlayerbotAI::IsRangedDps(bot) && IsTankedStriderInStepInReach(bot, currentTarget))
        return nullptr;

    return &stationPosition;
}

int8 GetNearestVashjStation(Unit* unit)
{
    if (!unit)
        return -1;

    int8 nearest = 0;
    float nearestDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < VASHJ_STATIONS.size(); ++i)
    {
        Position const& slot = VASHJ_STATIONS[i].ranged[0];
        float const distance = unit->GetExactDist2d(slot);
        if (distance < nearestDistance &&
            !SegmentCrossesPolygon(slot, unit->GetPosition(), VASHJ_NORTH_ROCK))
        {
            nearestDistance = distance;
            nearest = static_cast<int8>(i);
        }
    }

    return nearest;
}

// Vashj: Adds and Target Priority

VashjAddGuids FindVashjAddGuids(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    VashjAddGuids adds;
    for (auto const& guid : context->GetValue<GuidVector>("possible targets no los")->RefGet())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SscNpcs::NPC_ENCHANTED_ELEMENTAL):
                adds.enchanted.push_back(guid);
                break;
            case Id(SscNpcs::NPC_COILFANG_ELITE):
                adds.elites.push_back(guid);
                break;
            case Id(SscNpcs::NPC_COILFANG_STRIDER):
                adds.striders.push_back(guid);
                break;
            case Id(SscNpcs::NPC_TOXIC_SPOREBAT):
                adds.sporebats.push_back(guid);
                break;
            default:
                break;
        }
    }

    return adds;
}

std::vector<VashjTargetTier> const& GetVashjTargetTiers(Player* bot, int8 phase, bool killsTainted)
{
    bool const isTank = PlayerbotAI::IsTank(bot);

    if (phase == 2)
    {
        if (PlayerbotAI::IsRangedDps(bot))
        {
            return killsTainted ?
                VASHJ_PHASE_2_TAINTED_KILLER_TIERS : VASHJ_PHASE_2_STATION_RANGED_TIERS;
        }

        if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
            return VASHJ_PHASE_2_MELEE_TIERS;

        return isTank ? VASHJ_PHASE_2_TANK_TIERS : VASHJ_PHASE_2_HEALER_TIERS;
    }

    if (isTank)
    {
        return PlayerbotAI::IsMainTank(bot) ?
            VASHJ_PHASE_3_MAIN_TANK_TIERS : VASHJ_PHASE_3_TANK_TIERS;
    }

    if (PlayerbotAI::IsRanged(bot))
    {
        return bot->getClass() == CLASS_HUNTER ?
            VASHJ_PHASE_3_HUNTER_TIERS : VASHJ_PHASE_3_RANGED_TIERS;
    }

    return VASHJ_PHASE_3_MELEE_TIERS;
}

bool IsVashjAddHeldByTank(Unit* unit)
{
    if (!unit)
        return false;

    Player* victim = unit->GetVictim() ? unit->GetVictim()->ToPlayer() : nullptr;
    return victim && PlayerbotAI::IsTank(victim);
}

Player* GetVashjAddOwningTank(Player* bot, Unit* add)
{
    if (!add)
        return nullptr;

    Player* victim = add->GetVictim() ? add->GetVictim()->ToPlayer() : nullptr;
    if (victim && victim->IsAlive() && victim->GetVictim() == add && PlayerbotAI::IsTank(victim))
        return victim;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetVictim() == add &&
            PlayerbotAI::IsTank(member))
        {
            return member;
        }
    }

    return nullptr;
}

bool IsNearestFreeVashjTank(Player* bot, Unit* add, Unit* vashj, int8 phase)
{
    if (!add || !vashj)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    bool const eliteHolderIsFree =
        phase == 3 && add->GetEntry() == Id(SscNpcs::NPC_COILFANG_STRIDER);
    Unit const* vashjTank = phase == 3 ? vashj->GetVictim() : nullptr;
    float const botDistance = bot->GetExactDist2d(add);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || member == vashjTank || !member->IsAlive() ||
            member->GetMapId() != SSC_MAP_ID || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsTank(member) || member->GetExactDist2d(add) >= botDistance)
        {
            continue;
        }

        Unit* victim = member->GetVictim();
        if (!victim)
            return false;

        uint32 const entry = victim->GetEntry();
        bool const holdsAdd = (entry == Id(SscNpcs::NPC_COILFANG_STRIDER) ||
            (entry == Id(SscNpcs::NPC_COILFANG_ELITE) && !eliteHolderIsFree)) &&
            GetVashjAddOwningTank(bot, victim) == member;

        if (!holdsAdd)
            return false;
    }

    return true;
}

bool GetStepToBringTankedUnitTo(
    Player* bot, Unit* add, Position const& spot, float arrivalDistance, float& stepX,
    float& stepY, bool& backwards)
{
    if (!add)
        return false;

    float const addDistance = add->GetExactDist2d(spot);
    if (addDistance <= arrivalDistance)
        return false;

    float const lead = bot->GetExactDist2d(add) / addDistance;
    Position const destination(
        spot.GetPositionX() + (spot.GetPositionX() - add->GetPositionX()) * lead,
        spot.GetPositionY() + (spot.GetPositionY() - add->GetPositionY()) * lead,
        spot.GetPositionZ());

    return GetStepToPosition(bot, destination, arrivalDistance, add, stepX, stepY, backwards);
}

Position const& GetVashjStriderHoldPosition(Unit const& strider)
{
    return *std::min_element(
        VASHJ_STRIDER_HOLD_POSITIONS.begin(), VASHJ_STRIDER_HOLD_POSITIONS.end(),
        [&strider](Position const& a, Position const& b)
        {
            return strider.GetExactDist2d(a) < strider.GetExactDist2d(b);
        });
}

Position const& GetVashjEliteTankPosition(Unit const& elite)
{
    return *std::min_element(
        VASHJ_ELITE_TANK_POSITIONS.begin(), VASHJ_ELITE_TANK_POSITIONS.end(),
        [&elite](Position const& a, Position const& b)
        {
            return elite.GetExactDist2d(a) < elite.GetExactDist2d(b);
        });
}

bool ShouldPositionVashjStrider(Player* bot, Unit* strider, Unit* vashj, int8 phase)
{
    if (!vashj || !strider || !strider->IsAlive() ||
        strider->GetEntry() != Id(SscNpcs::NPC_COILFANG_STRIDER))
    {
        return false;
    }

    if (phase == 2)
    {
        if (strider->GetVictim() != bot)
            return false;

        return strider->GetExactDist2d(GetVashjStriderHoldPosition(*strider)) >
            VASHJ_ADD_TANK_ARRIVAL_DISTANCE;
    }

    if (phase != 3 || PlayerbotAI::IsMainTank(bot))
        return false;

    return strider->GetVictim() != bot ||
        bot->GetExactDist2d(vashj) < VASHJ_PHASE_3_STRIDER_DISTANCE_FROM_VASHJ;
}

bool IsTankedStriderInStepInReach(Player* bot, Unit* unit)
{
    return unit && unit->IsAlive() && unit->GetEntry() == Id(SscNpcs::NPC_COILFANG_STRIDER) &&
        bot->GetExactDist(unit) <= VASHJ_STRIDER_STEP_IN_DISTANCE && IsVashjAddHeldByTank(unit);
}

Unit* GetVashjPetTarget(PlayerbotAI* botAI, Creature* pet, Unit* vashj)
{
    if (!pet || !vashj)
        return nullptr;

    int8 const phase = GetLadyVashjPhase(vashj);
    uint32 const petEntry = pet->GetEntry();
    bool const petStaysAtRange = petEntry == Id(SscNpcs::NPC_IMP) ||
        petEntry == Id(SscNpcs::NPC_WATER_ELEMENTAL) ||
        petEntry == Id(SscNpcs::NPC_WATER_ELEMENTAL_PERM);

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsAlive())
    {
        uint32 const entry = target->GetEntry();
        bool const useless = (target == vashj && phase == 2) ||
            (entry == Id(SscNpcs::NPC_COILFANG_STRIDER) && !petStaysAtRange) ||
            entry == Id(SscNpcs::NPC_TOXIC_SPOREBAT);
        if (!useless)
            return target;
    }

    Unit* enchanted = nullptr;
    auto const& adds = context->GetValue<VashjAddGuids>("ssc vashj adds")->RefGet();
    for (ObjectGuid const& guid : adds.enchanted)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() &&
            (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted)))
        {
            enchanted = unit;
        }
    }

    if (enchanted)
        return enchanted;

    return phase == 3 ? vashj : nullptr;
}

// Vashj: Tainted Elemental

Player* FindTaintedCoreLooter(Player* bot, Unit* tainted, int8 station)
{
    if (!tainted)
        return nullptr;

    if (Player* healer = GetVashjStationHealer(bot, station))
        return healer;

    Player* looter = nullptr;
    float looterDistance = std::numeric_limits<float>::max();
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != SSC_MAP_ID ||
                !GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsHeal(member) ||
                GetVashjStationSlot(member).station >= 0)
            {
                continue;
            }

            float const distance = member->GetExactDist(tainted);
            if (distance < looterDistance)
            {
                looterDistance = distance;
                looter = member;
            }
        }
    }

    if (!looter)
    {
        for (Player* member : GetVashjStationRanged(bot, station))
        {
            float const distance = member->GetExactDist(tainted);
            if (distance < looterDistance)
            {
                looterDistance = distance;
                looter = member;
            }
        }
    }

    return looter;
}

Creature* GetAssignedTaintedElemental(Player* bot)
{
    std::optional<TaintedCoreLooter> const& looter =
        SscState(bot->GetInstanceId()).vashjTaintedCoreLooter;
    if (!looter)
        return nullptr;

    return ObjectAccessor::GetCreature(*bot, looter->tainted);
}

int8 GetTaintedCoreLootSlot(Creature* tainted)
{
    if (!tainted)
        return -1;

    std::vector<LootItem> const& items = tainted->loot.items;
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (items[i].itemid == Id(SscItems::ITEM_TAINTED_CORE) && !items[i].is_looted)
            return static_cast<int8>(i);
    }

    return -1;
}

bool IsTaintedCoreStillToLoot(Creature* tainted)
{
    return tainted && (tainted->IsAlive() || GetTaintedCoreLootSlot(tainted) >= 0);
}

Creature* GetTaintedElementalToKill(Player* bot)
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return nullptr;

    std::optional<TaintedCoreLooter> const& looter =
        SscState(bot->GetInstanceId()).vashjTaintedCoreLooter;
    if (!looter || GetVashjStationSlot(bot).station != looter->station)
        return nullptr;

    Creature* tainted = ObjectAccessor::GetCreature(*bot, looter->tainted);
    return tainted && tainted->IsAlive() ? tainted : nullptr;
}

bool IsDesignatedCoreLooter(Player* bot)
{
    std::optional<TaintedCoreLooter> const& looter =
        SscState(bot->GetInstanceId()).vashjTaintedCoreLooter;
    return looter && looter->looter == bot->GetGUID();
}

// Paralyze is present when the Core is held, and the aura check is much cheaper than a bag search.
bool HasTaintedCore(Player* player)
{
    return player && player->HasAura(Id(SscSpells::SPELL_TAINTED_CORE_PARALYZE));
}

// Vashj: Core Passing Chain

void PlanVashjCorePassingChain(Player* bot, Unit* tainted, Player* looter)
{
    if (!tainted || !looter)
        return;

    VashjCorePassingChain chain;
    chain.tainted = tainted->GetGUID();
    chain.originBot = looter->GetGUID();

    std::vector<Position> spots;
    if (GameObject* generator = PlanVashjCoreRoute(bot, tainted->GetPosition(),
            VASHJ_CORE_THROW_PLAN_DISTANCE - VASHJ_CORE_LOOTER_OFFSET, VASHJ_CORE_PLAN_EYE_HEIGHT,
            ObjectGuid::Empty, spots))
    {
        chain.generator = generator->GetGUID();
        for (Position const& spot : spots)
            chain.catchers.push_back(VashjCoreCatcher{ spot });
    }

    AssignVashjCoreCatchers(bot, chain);
    SscState(bot->GetInstanceId()).vashjCorePassingChain = std::move(chain);
}

bool ReplanVashjCorePassingChain(Player* holder, VashjCorePassingChain& chain, ObjectGuid excluded)
{
    constexpr uint8 maxReplans = 3;
    if (++chain.replans > maxReplans)
    {
        chain.failed = true;
        return false;
    }

    chain.originBot = holder->GetGUID();
    chain.catchers.clear();
    ResetVashjCoreThrows(chain);

    std::vector<Position> spots;
    if (GameObject* generator = PlanVashjCoreRoute(holder, holder->GetPosition(),
            VASHJ_CORE_THROW_PLAN_DISTANCE, holder->GetCollisionHeight(), chain.generator, spots))
    {
        chain.generator = generator->GetGUID();
        for (Position const& spot : spots)
            chain.catchers.push_back(VashjCoreCatcher{ spot });
    }

    chain.failed = chain.catchers.empty();
    if (!excluded.IsEmpty())
        chain.excluded = excluded;
    AssignVashjCoreCatchers(holder, chain);
    return !chain.failed;
}

bool ReassignVashjCoreCatcher(Player* bot, VashjCorePassingChain& chain, size_t index)
{
    VashjCoreCatcher& catcher = chain.catchers[index];
    Player* player = FindVashjCoreCatcher(bot, chain, catcher.spot, false, catcher.bot);

    chain.waitTarget.Clear();
    chain.blockedStart = 0;
    if (!player)
        return false;

    SetVashjCoreCatcher(catcher, player);
    return true;
}

void ReleaseVashjCoreCatcher(Player* bot, VashjCorePassingChain& chain, size_t index)
{
    VashjCoreCatcher& catcher = chain.catchers[index];
    if (catcher.released)
        return;

    Player* player = FindVashjCoreCatcher(bot, chain, catcher.spot, false, ObjectGuid::Empty);
    SetVashjCoreCatcher(catcher, player);
    catcher.released = true;
}

VashjCorePassingChain* GetVashjCorePassingChain(Player* bot)
{
    std::optional<VashjCorePassingChain>& chain =
        SscState(bot->GetInstanceId()).vashjCorePassingChain;
    return chain ? &*chain : nullptr;
}

int8 GetVashjCoreCatcherIndex(VashjCorePassingChain const& chain, Player* bot)
{
    for (size_t i = 0; i < chain.catchers.size(); ++i)
    {
        if (chain.catchers[i].bot == bot->GetGUID())
            return static_cast<int8>(i);
    }

    return -1;
}

bool IsVashjCoreCatcherActive(Player* bot, VashjCorePassingChain const& chain, int8 index)
{
    if (index < chain.reached)
        return false;

    VashjCoreCatcher const& catcher = chain.catchers[index];
    if (!catcher.released || getMSTimeDiff(catcher.releaseTime, getMSTime()) < catcher.readyDelay)
        return false;

    if (!catcher.prepositions)
    {
        Creature* tainted = ObjectAccessor::GetCreature(*bot, chain.tainted);
        if (tainted && tainted->IsAlive())
            return false;
    }

    return IsVashjCorePassingChainLive(bot, chain);
}

float GetVashjCoreSpotArrivalDistance(VashjCorePassingChain const& chain, int8 index)
{
    return static_cast<size_t>(index) + 1 == chain.catchers.size() ?
        VASHJ_CORE_USE_SPOT_ARRIVAL_DISTANCE : VASHJ_CORE_SPOT_ARRIVAL_DISTANCE;
}

// Shared encounter state

SscInstanceState& SscState(uint32 instanceId)
{
    std::lock_guard lock(sscStateMutex);
    return sscStates[instanceId];
}

bool SscResetInstance(uint32 instanceId)
{
    std::lock_guard lock(sscStateMutex);
    auto it = sscStates.find(instanceId);
    if (it == sscStates.end())
        return false;

    SscInstanceState const& state = it->second;
    bool const wasSet =
        state.hydrossFrostPhaseStartTime || state.hydrossNaturePhaseStartTime ||
        state.hydrossFrostMarkMaxedTime || state.hydrossNatureMarkMaxedTime ||
        state.lurkerGuardianTankAssignments || state.leotherasHumanoidPhaseStartTime ||
        state.leotherasWhirlwindEndTime || state.leotherasDemonPhaseStartTime ||
        state.leotherasFinalPhaseStartTime || state.karathressDpsWaitTimer ||
        state.vashjGroundingShaman || state.vashjStationHolders || state.vashjTaintedCoreLooter ||
        state.vashjCorePassingChain;

    sscStates.erase(it);
    return wasSet;
}

}
