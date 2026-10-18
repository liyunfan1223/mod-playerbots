/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_YoggSaron.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <limits>
#include <list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AiObjectContext.h"
#include "RaidDifficulty.h"
#include "RaidHumanTankCall.h"
#include "RaidInstanceState.h"
#include "RaidTankDefensive.h"
#include "CellImpl.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Creature.h"
#include "DBCEnums.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Item.h"
#include "GameObject.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include "UldEncounterGate.h"
#include "UldHardMode.h"
#include "Unit.h"
#include "Vehicle.h"
#include "WorldSession.h"

const std::vector<uint32> ULDUAR_YOGG_SARON_ILLUSION_MOBS = {
    NPC_INFLUENCE_TENTACLE, NPC_SUIT_OF_ARMOR,   NPC_DEATHSWORN_ZEALOT,  NPC_RUBY_CONSORT,
    NPC_AZURE_CONSORT,      NPC_BRONZE_CONSORT,  NPC_EMERALD_CONSORT,    NPC_OBSIDIAN_CONSORT};

// Which disguise a tentacle wears is decided by where it spawned, not by which illusion is running:
// x in (2000, 2150) is a Consort, otherwise y in (-150, -90) is a Zealot, otherwise a Suit of Armor.
const Position ULDUAR_YOGG_SARON_PORTAL_SPOTS[ULDUAR_YOGG_SARON_PORTAL_SPOTS_25MAN] = {
    Position(1964.60f, -42.71f, 325.08f), Position(1986.94f, -46.21f, 324.98f),
    Position(1989.50f, -6.70f, 325.08f),  Position(1965.52f, -8.09f, 324.95f),
    Position(2000.84f, -25.40f, 325.19f), Position(1960.22f, -26.14f, 325.01f),
    Position(1976.30f, -47.83f, 325.11f), Position(1997.69f, -37.46f, 325.04f),
    Position(1998.07f, -13.36f, 325.17f), Position(1976.99f, -3.96f, 325.17f)};

const Position ULDUAR_YOGG_SARON_MIDDLE = Position(1980.28f, -25.5868f, 329.397f);
const Position ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE = Position(1927.1511f, 68.507256f, 242.37657f);
const Position ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE = Position(1925.6553f, -121.59296f, 239.98965f);
const Position ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE = Position(2104.5667f, -25.509348f, 242.64679f);
const Position ULDUAR_YOGG_SARON_BRAIN_ROOM_MIDDLE = Position(1980.1971f, -27.854689f, 236.06789f);
const Position ULDUAR_YOGG_SARON_ICECROWN_CLUSTERS[ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT] = {
    Position(1901.60f, -107.58f, 240.09f), Position(1916.27f, -137.88f, 240.07f),
    Position(1952.63f, -135.08f, 240.10f)};
const Position ULDUAR_YOGG_SARON_ICECROWN_STAGE_SPOT = Position(1952.10f, -73.62f, 240.008f);
const Position ULDUAR_YOGG_SARON_STORMWIND_STAGE_SPOT = Position(1955.17f, 18.87f, 239.721f);
const Position ULDUAR_YOGG_SARON_BRAIN_MELEE_SPOT = Position(1969.0f, -25.4f, 237.474f);
const Position ULDUAR_YOGG_SARON_BRAIN_WEST_SPOT = Position(1948.05f, -23.76f, 241.785f);
const Position ULDUAR_YOGG_SARON_BRAIN_SOUTH_SPOT = Position(1992.4f, -52.8f, 240.585f);
const Position ULDUAR_YOGG_SARON_BRAIN_NORTH_SPOT = Position(1994.6f, -0.1f, 240.292f);
const Position ULDUAR_YOGG_SARON_STORMWIND_KEEPER_ENTRANCE = Position(1954.06f, 21.66f, 239.71f);
const Position ULDUAR_YOGG_SARON_ICECROWN_CITADEL_ENTRANCE = Position(1950.11f, -79.284f, 239.98982f);
const Position ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_ENTRANCE = Position(2048.63f, -25.5f, 239.72f);
const Position ULDUAR_YOGG_SARON_P1_RANGED_SPOT = Position(1958.78f, -25.587f, 324.889f);
const Position ULDUAR_YOGG_SARON_HANDOVER_MELEE_SPOT = Position(1973.315f, -5.246f, 324.889f);
const Position ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT = Position(1998.5377f, -22.90317f, 324.8895f);
const Position ULDUAR_YOGG_SARON_PHASE_3_RANGED_SPOT = Position(2018.7628f, -18.896868f, 327.07245f);
const std::vector<Position> ULDUAR_YOGG_SARON_PHASE_3_RANGED_STANDBY_SPOTS = {
    Position(2019.18f, -29.08f, 327.154f), Position(2015.72f, -9.16f, 327.155f),
    Position(2016.95f, -39.03f, 327.678f), Position(2010.26f, -0.55f, 327.291f)};
const Position ULDUAR_YOGG_SARON_BEACON_DRAG_NORTH = Position(1983.68f, 13.27f, 327.472f);
const Position ULDUAR_YOGG_SARON_BEACON_DRAG_SOUTH = Position(1993.62f, -62.23f, 327.625f);
const Position ULDUAR_YOGG_SARON_BEACON_DRAG_NORTHEAST = Position(2017.05f, 5.26f, 328.781f);
const Position ULDUAR_YOGG_SARON_BEACON_HUNTER_NORTH = Position(1986.0f, 7.63f, 326.797f);
const Position ULDUAR_YOGG_SARON_BEACON_HUNTER_SOUTH = Position(1994.44f, -55.65f, 326.394f);
const Position ULDUAR_YOGG_SARON_BEACON_STAGE_NORTH = Position(1990.94f, -4.40f, 325.104f);
const Position ULDUAR_YOGG_SARON_BEACON_STAGE_SOUTH = Position(1996.06f, -42.75f, 324.889f);
const Position ULDUAR_YOGG_SARON_BEACON_STAGE_NORTHEAST = Position(2009.52f, -6.19f, 326.989f);

namespace
{
struct YoggSaronBeaconLane
{
    Position const* drag;
    Position const* hunter;
    Position const* stage;
    char const* name;
};

// North-east is inside Distracting Shot's reach of the melee spot, so it is its own hunter spot.
constexpr uint8 YOGG_SARON_BEACON_LANE_NORTHEAST = 2;
std::array<YoggSaronBeaconLane, 3> const YOGG_SARON_BEACON_LANES = {
    {{&ULDUAR_YOGG_SARON_BEACON_DRAG_NORTH, &ULDUAR_YOGG_SARON_BEACON_HUNTER_NORTH,
      &ULDUAR_YOGG_SARON_BEACON_STAGE_NORTH, "north"},
     {&ULDUAR_YOGG_SARON_BEACON_DRAG_SOUTH, &ULDUAR_YOGG_SARON_BEACON_HUNTER_SOUTH,
      &ULDUAR_YOGG_SARON_BEACON_STAGE_SOUTH, "south"},
     {&ULDUAR_YOGG_SARON_BEACON_DRAG_NORTHEAST, &ULDUAR_YOGG_SARON_BEACON_DRAG_NORTHEAST,
      &ULDUAR_YOGG_SARON_BEACON_STAGE_NORTHEAST, "northeast"}}};

// Raid-wide answers folded once per instance.
struct YoggSaronEncounterState
{
    GuidUnorderedSet brainTeam{};
    std::unordered_map<ObjectGuid, uint8> portalSlot{};
    // Damage dealers off the tentacles until healed. See YoggSaronTickReprisal.
    GuidUnorderedSet reprisalHold{};

    // Phase 2's clock. nextWaveMs is a prediction: it starts at +60 s and re-anchors on every wave a
    // bot actually sees, so a delayed wave corrects itself rather than compounding.
    uint32 phase2StartMs = 0;
    uint32 lastWaveMs = 0;
    uint32 nextWaveMs = 0;
    uint32 waveOrdinal = 0;
    uint32 slotWave = 0;
    bool slotPortalsUp = false;

    // Under ULDUAR_YOGG_SARON_BRAIN_TEAM_MIN_SANITY for the wave the plan is for. Decided when the team is back
    // and reused at the portal rebuild, so a Psychosis tick can't swap somebody out at the last second.
    GuidUnorderedSet benched{};
    // The wave whose plan was already rebuilt off the returned team's Sanity.
    uint32 returnPlanWave = 0;

    // When the Induce Madness each bot decided to leave for ends. See YoggSaronLeavingBrainLevel.
    std::unordered_map<ObjectGuid, uint32> leavingUntilMs;

    // The illusion room each bot was last in, which picks its brain spot after it has walked out.
    std::unordered_map<ObjectGuid, YoggSaronRoom> lastIllusionRoom;

    // Who clears what in Icecrown and Stormwind, for the wave in shareWave. See
    // YoggSaronIllusionTentacleAllowed.
    uint32 shareWave = 0;
    std::unordered_map<ObjectGuid, uint8> icecrownCluster;
    enum StormwindRole : uint8
    {
        STORMWIND_FIRST,
        STORMWIND_HELPER,
        STORMWIND_DONE,
    };
    struct StormwindShare
    {
        ObjectGuid held;
        bool heldNear = false;
        StormwindRole role = STORMWIND_FIRST;
    };
    std::unordered_map<ObjectGuid, StormwindShare> stormwindShare;
    // What each Icecrown or Stormwind bot may take, latched every tick in YoggSaronTickIllusionShare so
    // the resolver, the room state and the stage hold read one answer. No entry means no limit.
    struct IllusionShare
    {
        std::vector<ObjectGuid> allowed;
        bool staged = false;
    };
    std::unordered_map<ObjectGuid, IllusionShare> illusionShare;
    // Whether each portal spot (west, south, north) sees the Brain: -1 not asked yet. Neither moves.
    std::array<int8, 3> brainSpotSight{-1, -1, -1};

    // Start: first tick Yogg is seen without his barrier (nothing counts the dialogue down). Ring: the tick the
    // ring lit, which closes the window. The walk out has to outlive it or melee eat the knock back.
    uint32 handoverStartMs = 0;
    uint32 handoverRingMs = 0;

    // Which Squeeze victim each rescuer has called, so three paladins do not spend three cooldowns on
    // one tentacle. Keyed by victim rather than by window because grabs overlap.
    std::unordered_map<ObjectGuid, uint32> squeezeClaims;

    // One external and one raid-wide cover per Guardian, keyed by the Guardian whose nova is coming, not by victim
    // or clock. The cover keeps its owner so the same bot can retry across ticks.
    std::unordered_map<ObjectGuid, uint32> novaClaims;
    std::unordered_map<ObjectGuid, std::pair<ObjectGuid, uint32>> novaCovers;

    // Phase 1's raid-wide kill target. See YoggSaronPhase1Focus.
    ObjectGuid phase1Focus;

    // Stamped by each Guardian's 65719. 0 until the first one dies.
    uint32 lastGuardianDeathMs = 0;

    // Phase 3 without Thorim: per marked Guardian, its dragger, when its mark ended (0 while it lasts), its lane
    // in YOGG_SARON_BEACON_LANES and the last taunt. See YoggSaronBeaconDragGuardian.
    struct BeaconDrag
    {
        ObjectGuid dragger;
        uint32 markEndedMs = 0;
        uint8 lane = 0;
        uint32 tauntMs = 0;
    };
    std::unordered_map<ObjectGuid, BeaconDrag> beaconDrags;
    // beaconClockMs is phase 3's start as first seen, lastMarkMs the last beacon's first mark, stageDueMs the
    // beacon the stagers were picked for, so a timed-out stage isn't picked again. See YoggSaronTickBeacon.
    uint32 beaconClockMs = 0;
    uint32 lastMarkMs = 0;
    uint32 stageDueMs = 0;
    uint32 beaconScanMs = 0;
    bool markWindow = false;
    std::unordered_map<ObjectGuid, uint8> beaconStages;
    std::vector<ObjectGuid> markKickers;
    // Released drags, keyed by the Guardian: who dragged it and when the heal let it go. The dragger walks
    // it back to the tank from here, and a hunter feigns off one that stayed on it.
    struct BeaconReturn
    {
        ObjectGuid dragger;
        uint32 releasedMs = 0;
    };
    std::unordered_map<ObjectGuid, BeaconReturn> beaconReturns;
    // One external per victim of a healed Guardian, keyed by the victim.
    std::unordered_map<ObjectGuid, uint32> beaconExternalClaims;
    uint32 humanCallScanMs = 0;

    // Deafening Roar, see YoggSaronTickRoar. The claim is keyed by the roar's cast start.
    bool roarExpected = false;
    uint32 lastRoarMs = 0;
    uint32 roarClaimKey = 0;
    ObjectGuid roarClaimOwner;

    // Per brain-room bot, latched in YoggSaronTickBrainHold. A bot that started the finish stays on it
    // until that Induce Madness ends, so a tentacle turning up later can't stop it short.
    enum BrainHoldMode : uint8
    {
        BRAIN_HOLD_NONE,
        BRAIN_HOLD_DOTS,
        BRAIN_HOLD_FULL,
        BRAIN_HOLD_FINISH,
    };
    struct BrainHold
    {
        BrainHoldMode mode = BRAIN_HOLD_NONE;
        uint32 seenMs = 0;
        uint32 finishCastEndMs = 0;
    };
    std::unordered_map<ObjectGuid, BrainHold> brainHold;

    // Which bot's kick holds which cast, keyed by the caster, and when it was taken.
    std::unordered_map<ObjectGuid, std::pair<ObjectGuid, uint32>> interruptClaims;
    // Drain Life channels already given a last-resort nudge, keyed by the Guardian, and when any bot last
    // looked. See YoggSaronNudgeDrainLifeKicker.
    std::unordered_map<ObjectGuid, uint32> kickNudges;
    uint32 kickScanMs = 0;

    // Phase 3 tank swap, from the tank holding the pile to the one taking it. See YoggSaronTankSwapTake.
    struct TankSwap
    {
        ObjectGuid from;
        ObjectGuid to;
        uint32 startMs = 0;
    };
    TankSwap tankSwap;
    // One Lay on Hands per tank, keyed by the tank. The owner keeps it so a failed cast can retry.
    std::unordered_map<ObjectGuid, std::pair<ObjectGuid, uint32>> tankLohClaims;

    // First time phase 3 was read for Heroism, cleared in phase 2.
    uint32 phase3StartMs = 0;
};

RaidInstanceState<YoggSaronEncounterState> yoggSaronStates;

YoggSaronEncounterState& YoggSaronStateFor(Player* bot) { return yoggSaronStates.For(bot->GetInstanceId()); }

// Sweeps every Yogg trigger repeats, answered once per pass under the id EncounterTriggerPassId hands out.
// thread_local is fine: a pass runs on one thread. Never turn this into a latch.
constexpr uint32 YOGG_SARON_CACHED_CREATURES[] = {NPC_SARA_PHASE_1, NPC_YOGG_SARON, NPC_BRAIN, NPC_SANITY_WELL,
                                                  NPC_GUARDIAN_OF_YS, NPC_CRUSHER_TENTACLE, NPC_CONSTRICTOR_TENTACLE,
                                                  NPC_CORRUPTOR_TENTACLE};
constexpr uint32 YOGG_SARON_CACHED_DOORS[] = {GO_CHAMBER_ILLUSION_DOORS, GO_ICECROWN_ILLUSION_DOORS,
                                              GO_STORMWIND_ILLUSION_DOORS};

struct YoggSaronPassReads
{
    PlayerbotAI* botAI = nullptr;
    uint32 passId = 0;

    std::array<bool, std::size(YOGG_SARON_CACHED_CREATURES)> creatureRead{};
    std::array<ObjectGuid, std::size(YOGG_SARON_CACHED_CREATURES)> creature{};

    std::array<bool, std::size(YOGG_SARON_CACHED_DOORS)> doorRead{};
    std::array<ObjectGuid, std::size(YOGG_SARON_CACHED_DOORS)> door{};

    // YoggSaronInfluenceTentaclesCleared asks about a room's own middle from inside an illusion room
    // and about the bot from the brain room, so the centre is part of the key.
    struct ClearRead
    {
        float x;
        float y;
        float radius;
        bool cleared;
    };
    std::vector<ClearRead> tentaclesCleared;

    bool leaveBrainLevelRead = false;
    bool leaveBrainLevel = false;

    bool platformTentacleRead = false;
    bool platformTentacleAlive = false;

    bool skullsInArcRead = false;
    std::vector<ObjectGuid> skullsInArc;

    // The healer's station trigger and its action both ask, and the answer is a 60 yd grid sweep.
    bool healerStationRead = false;
    bool healerStationFound = false;
    Position healerStation;

};

thread_local YoggSaronPassReads yoggSaronPassReads;

// Null outside a trigger pass, meaning read live.
YoggSaronPassReads* YoggSaronPassReadsFor(PlayerbotAI* botAI)
{
    uint32 const passId = EncounterTriggerPassId(botAI);
    if (!passId)
        return nullptr;

    if (yoggSaronPassReads.botAI != botAI || yoggSaronPassReads.passId != passId)
    {
        yoggSaronPassReads = YoggSaronPassReads();
        yoggSaronPassReads.botAI = botAI;
        yoggSaronPassReads.passId = passId;
    }

    return &yoggSaronPassReads;
}

// -1 for an entry the pass cache doesn't hold.
int32 YoggSaronCacheSlot(uint32 const* entries, size_t count, uint32 entry)
{
    for (size_t slot = 0; slot < count; ++slot)
        if (entries[slot] == entry)
            return static_cast<int32>(slot);

    return -1;
}

// FindNearestCreature(entry, 200, true), read off the instance script for the three it tracks, one each per instance.
// Only inside |dx| + |dy| < 200: the sweep skips the corner cells of its 7x7 block, and all of them lie past that.
Creature* YoggSaronFindNearestCreature(Player* bot, uint32 entry)
{
    constexpr float range = ULDUAR_YOGG_SARON_SEARCH_RANGE;

    uint32 type = 0;
    switch (entry)
    {
        case NPC_YOGG_SARON:
            type = ULD_BOSS_YOGGSARON;
            break;
        case NPC_SARA_PHASE_1:
            type = ULD_DATA_SARA;
            break;
        case NPC_BRAIN:
            type = ULD_DATA_BRAIN_OF_YOGG_SARON;
            break;
        default:
            return bot->FindNearestCreature(entry, range, true);
    }

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return bot->FindNearestCreature(entry, range, true);

    Creature* creature = instance->GetCreature(type);
    if (!creature)
        return nullptr;

    float const taxicab = std::abs(bot->GetPositionX() - creature->GetPositionX()) +
                          std::abs(bot->GetPositionY() - creature->GetPositionY());
    if (creature->GetEntry() != entry || taxicab >= range)
        return bot->FindNearestCreature(entry, range, true);

    // The sweep's own three tests.
    return creature->IsAlive() && bot->IsWithinDist(creature, range) && bot->InSamePhase(creature) ? creature : nullptr;
}

GameObject* YoggSaronNearestDoor(PlayerbotAI* botAI, uint32 entry)
{
    Player* bot = botAI->GetBot();

    int32 const slot =
        YoggSaronCacheSlot(YOGG_SARON_CACHED_DOORS, std::size(YOGG_SARON_CACHED_DOORS), entry);
    YoggSaronPassReads* reads = slot < 0 ? nullptr : YoggSaronPassReadsFor(botAI);
    if (!reads)
        return bot->FindNearestGameObject(entry, ULDUAR_YOGG_SARON_SEARCH_RANGE);

    if (!reads->doorRead[slot])
    {
        GameObject* found = bot->FindNearestGameObject(entry, ULDUAR_YOGG_SARON_SEARCH_RANGE);
        reads->doorRead[slot] = true;
        reads->door[slot] = found ? found->GetGUID() : ObjectGuid::Empty;
        return found;
    }

    return reads->door[slot] ? ObjectAccessor::GetGameObject(*bot, reads->door[slot]) : nullptr;
}
}  // namespace

Creature* YoggSaronNearestCreature(PlayerbotAI* botAI, uint32 entry)
{
    Player* bot = botAI->GetBot();

    int32 const slot =
        YoggSaronCacheSlot(YOGG_SARON_CACHED_CREATURES, std::size(YOGG_SARON_CACHED_CREATURES), entry);
    YoggSaronPassReads* reads = slot < 0 ? nullptr : YoggSaronPassReadsFor(botAI);
    if (!reads)
        return YoggSaronFindNearestCreature(bot, entry);

    if (!reads->creatureRead[slot])
    {
        Creature* found = YoggSaronFindNearestCreature(bot, entry);
        reads->creatureRead[slot] = true;
        reads->creature[slot] = found ? found->GetGUID() : ObjectGuid::Empty;
        return found;
    }

    return reads->creature[slot] ? ObjectAccessor::GetCreature(*bot, reads->creature[slot]) : nullptr;
}

uint32 YoggSaronPhase(PlayerbotAI* botAI)
{
    // Boss state, not Sara's combat flag: she is FACTION_FRIENDLY in phase 1, so SetInCombatWithZone skips her.
    // Ahead of the lookups, so a bot elsewhere in Ulduar pays one boss state read.
    if (!BossStateGateLive(botAI, ULD_BOSS_YOGGSARON))
        return 0;

    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);

    uint32 phase = 0;
    if (yogg && yogg->IsAlive() && yogg->HasAura(SPELL_SHADOW_BARRIER))
        phase = 2;
    // The Brain, not the absence of a phase-1 Guardian: the last one dies ~9.5 s before the Shadow Barrier
    // lands and absences are true in that gap. The Brain spawns with the first tentacle wave and lives to the end.
    else if (yogg && yogg->IsAlive() && YoggSaronNearestCreature(botAI, NPC_BRAIN))
        phase = 3;
    // Sara last: only phase 1 needs her.
    else if (YoggSaronNearestCreature(botAI, NPC_SARA_PHASE_1))
        phase = 1;

    return phase;
}

bool YoggSaronEncounterActive(PlayerbotAI* botAI) { return YoggSaronPhase(botAI) != 0; }
bool YoggSaronInPhase1(PlayerbotAI* botAI) { return YoggSaronPhase(botAI) == 1; }
bool YoggSaronInPhase2(PlayerbotAI* botAI) { return YoggSaronPhase(botAI) == 2; }
bool YoggSaronInPhase3(PlayerbotAI* botAI) { return YoggSaronPhase(botAI) == 3; }

bool YoggSaronOnBrainLevel(Player* player)
{
    return player && player->GetPositionZ() > ULDUAR_YOGG_SARON_BRAIN_LEVEL_MIN_Z &&
           player->GetPositionZ() < ULDUAR_YOGG_SARON_BRAIN_LEVEL_MAX_Z;
}

YoggSaronRoom YoggSaronRoomOf(Player* player)
{
    if (!player)
        return YOGG_SARON_ROOM_NONE;

    if (player->GetPositionZ() >= ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return YOGG_SARON_ROOM_ARENA;

    if (!YoggSaronOnBrainLevel(player))
        return YOGG_SARON_ROOM_NONE;

    // Brain room first. It is 108-124 yd from all three illusion middles, so any radius wide enough
    // to overlap them makes whichever room is tested first swallow it.
    if (player->GetDistance2d(ULDUAR_YOGG_SARON_BRAIN_ROOM_MIDDLE.GetPositionX(),
                              ULDUAR_YOGG_SARON_BRAIN_ROOM_MIDDLE.GetPositionY()) <
        ULDUAR_YOGG_SARON_BRAIN_ROOM_RADIUS)
        return YOGG_SARON_ROOM_BRAIN;

    if (player->GetDistance2d(ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE.GetPositionX(),
                              ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE.GetPositionY()) <
        ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS)
        return YOGG_SARON_ROOM_STORMWIND;

    if (player->GetDistance2d(ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE.GetPositionX(),
                              ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE.GetPositionY()) <
        ULDUAR_YOGG_SARON_ICECROWN_CITADEL_RADIUS)
        return YOGG_SARON_ROOM_ICECROWN;

    if (player->GetDistance2d(ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE.GetPositionX(),
                              ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE.GetPositionY()) <
        ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_RADIUS)
        return YOGG_SARON_ROOM_CHAMBER;

    return YOGG_SARON_ROOM_NONE;
}

bool YoggSaronRoomMiddle(Player* player, Position& middle)
{
    switch (YoggSaronRoomOf(player))
    {
        case YOGG_SARON_ROOM_STORMWIND:
            middle = ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE;
            return true;
        case YOGG_SARON_ROOM_ICECROWN:
            middle = ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE;
            return true;
        case YOGG_SARON_ROOM_CHAMBER:
            middle = ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE;
            return true;
        default:
            return false;
    }
}

bool YoggSaronBrainRoomApproachable(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    if (!YoggSaronInfluenceTentaclesCleared(botAI))
        return false;

    uint32 doorEntry = 0;
    switch (YoggSaronRoomOf(bot))
    {
        // Any open door means this wave is done: the Brain shuts all three on prepare and opens one when that
        // room's last tentacle dies. A radius can't tell from in here, the Chamber's far tentacles are 167 yd out.
        case YOGG_SARON_ROOM_BRAIN:
            for (uint32 entry = GO_CHAMBER_ILLUSION_DOORS; entry <= GO_STORMWIND_ILLUSION_DOORS; ++entry)
                if (GameObject* open = YoggSaronNearestDoor(botAI, entry))
                    if (open->GetGoState() == GO_STATE_ACTIVE)
                        return true;

            return false;
        case YOGG_SARON_ROOM_STORMWIND:
            doorEntry = GO_STORMWIND_ILLUSION_DOORS;
            break;
        case YOGG_SARON_ROOM_ICECROWN:
            doorEntry = GO_ICECROWN_ILLUSION_DOORS;
            break;
        case YOGG_SARON_ROOM_CHAMBER:
            doorEntry = GO_CHAMBER_ILLUSION_DOORS;
            break;
        default:
            return false;
    }

    GameObject* door = YoggSaronNearestDoor(botAI, doorEntry);

    return door && door->GetGoState() == GO_STATE_ACTIVE;
}

YoggSaronRoomState YoggSaronRoomStateOf(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    YoggSaronRoomState state = YOGG_SARON_ROOM_STATE_NONE;
    switch (YoggSaronRoomOf(bot))
    {
        case YOGG_SARON_ROOM_BRAIN:
            state = YOGG_SARON_ROOM_STATE_AT_BRAIN;
            break;
        case YOGG_SARON_ROOM_STORMWIND:
        case YOGG_SARON_ROOM_ICECROWN:
        case YOGG_SARON_ROOM_CHAMBER:
        {
            if (YoggSaronInfluenceTentaclesCleared(botAI))
            {
                state = YoggSaronBrainRoomApproachable(botAI) ? YOGG_SARON_ROOM_STATE_TO_BRAIN
                                                              : YOGG_SARON_ROOM_STATE_DOOR_SHUT;
                break;
            }

            if (YoggSaronReprisalHold(botAI))
            {
                state = YOGG_SARON_ROOM_STATE_REPRISAL;
                break;
            }

            // Line of sight is the difference: two rooms put tentacles behind a doorway, the dps resolver won't pick
            // what it can't see, and nothing else moves a bot once the portal dropped it.
            Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            if (target && target->IsAlive() && bot->IsWithinLOSInMap(target))
                state = YOGG_SARON_ROOM_STATE_FIGHTING;
            else if (YoggSaronIllusionStaged(botAI))
                state = YOGG_SARON_ROOM_STATE_STAGED;
            else
                state = YOGG_SARON_ROOM_STATE_WALKING_IN;
            break;
        }
        default:
            break;
    }

    return state;
}

namespace
{

bool YoggSaronLeaveBrainLevelNow(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Creature const* brain = YoggSaronNearestCreature(botAI, NPC_BRAIN);
    if (!brain || !brain->IsAlive())
        return false;

    bool leave = false;

    if (brain->HasUnitState(UNIT_STATE_CASTING))
    {
        Spell* induceMadness = brain->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (induceMadness && induceMadness->m_spellInfo->Id == SPELL_INDUCE_MADNESS)
        {
            uint32 const castingTimeLeft = induceMadness->GetCastTimeRemaining();
            uint32 const now = getMSTime();

            // Once leaving, stays leaving: asked fresh each tick the lead shrinks as the bot walks, reads safe again
            // and the brain spot walk pulls it back.
            bool const latched = YoggSaronLeavingBrainLevel(bot);

            // Only decided in the brain room. From an illusion room the way out runs through it, and from the back of
            // the Chamber the lead overreads the walk.
            bool const inBrainRoom = YoggSaronRoomOf(bot) == YOGG_SARON_ROOM_BRAIN;

            // Every millisecond of lead is damage the Brain does not take, so it is measured against
            // the walk the bot actually faces rather than set flat for the worst case.
            uint32 lead = ULDUAR_YOGG_SARON_EXIT_LEAD_FLOOR_MS;
            GameObject* portal =
                latched || !inBrainRoom
                    ? nullptr
                    : bot->FindNearestGameObject(GO_FLEE_TO_THE_SURFACE_PORTAL, ULDUAR_YOGG_SARON_SEARCH_RANGE);
            if (portal)
                lead = std::max(lead, static_cast<uint32>(bot->GetDistance2d(portal) / baseMoveSpeed[MOVE_RUN] *
                                                          ULDUAR_YOGG_SARON_EXIT_LEAD_SAFETY * IN_MILLISECONDS));

            if (latched || (inBrainRoom && castingTimeLeft < lead))
            {
                leave = true;
                YoggSaronStateFor(bot).leavingUntilMs[bot->GetGUID()] = now + castingTimeLeft;
            }
        }
    }
    else if (brain->GetHealth() < brain->GetMaxHealth() * ULDUAR_YOGG_SARON_BRAIN_PHASE_3_SHARE)
        leave = true;

    return leave;
}

}  // namespace

bool YoggSaronShouldLeaveBrainLevel(PlayerbotAI* botAI)
{
    // The exit node and the brain spot node both ask this in the same pass, and the answer costs a
    // game object search.
    YoggSaronPassReads* reads = YoggSaronPassReadsFor(botAI);
    if (reads && reads->leaveBrainLevelRead)
        return reads->leaveBrainLevel;

    bool const leave = YoggSaronLeaveBrainLevelNow(botAI);

    if (reads)
    {
        reads->leaveBrainLevelRead = true;
        reads->leaveBrainLevel = leave;
    }

    return leave;
}

bool YoggSaronLeavingBrainLevel(Player* bot)
{
    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);
    auto const latch = state.leavingUntilMs.find(bot->GetGUID());

    // Signed: an end still in the future must not read as a timer wrap.
    return latch != state.leavingUntilMs.end() && static_cast<int32>(latch->second - getMSTime()) > 0;
}

uint32 YoggSaronSanityOf(Player* player)
{
    Aura const* sanity = player ? player->GetAura(SPELL_SANITY) : nullptr;
    return sanity ? sanity->GetStackAmount() : 0;
}

Position YoggSaronBrainSpot(Player* bot)
{
    if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsHeal(bot))
        return ULDUAR_YOGG_SARON_BRAIN_MELEE_SPOT;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);

    YoggSaronRoom const here = YoggSaronRoomOf(bot);
    if (here == YOGG_SARON_ROOM_STORMWIND || here == YOGG_SARON_ROOM_ICECROWN || here == YOGG_SARON_ROOM_CHAMBER)
        state.lastIllusionRoom[bot->GetGUID()] = here;

    auto const came = state.lastIllusionRoom.find(bot->GetGUID());
    YoggSaronRoom const room = came == state.lastIllusionRoom.end() ? YOGG_SARON_ROOM_NONE : came->second;

    uint8 index = 0;
    Position const* spot = nullptr;
    switch (room)
    {
        case YOGG_SARON_ROOM_STORMWIND:
            index = 0;
            spot = &ULDUAR_YOGG_SARON_BRAIN_WEST_SPOT;
            break;
        case YOGG_SARON_ROOM_ICECROWN:
            index = 1;
            spot = &ULDUAR_YOGG_SARON_BRAIN_SOUTH_SPOT;
            break;
        case YOGG_SARON_ROOM_CHAMBER:
            index = 2;
            spot = &ULDUAR_YOGG_SARON_BRAIN_NORTH_SPOT;
            break;
        default:
            break;
    }

    // The spot table isn't checked for line of sight up to the Brain, and without it the resolver never picks
    // the Brain, so each spot earns it once. Same ray IsWithinLOSInMap casts.
    if (spot && state.brainSpotSight[index] < 0)
    {
        if (Creature* brain = YoggSaronFindNearestCreature(bot, NPC_BRAIN))
        {
            Position const eye(spot->GetPositionX(), spot->GetPositionY(),
                               spot->GetPositionZ() + bot->GetCollisionHeight());
            Position const hit = brain->GetHitSpherePointFor(eye);
            state.brainSpotSight[index] =
                brain->GetMap()->isInLineOfSight(eye.GetPositionX(), eye.GetPositionY(), eye.GetPositionZ(),
                                                 hit.GetPositionX(), hit.GetPositionY(), hit.GetPositionZ(),
                                                 brain->GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                                 VMAP::ModelIgnoreFlags::Nothing)
                    ? 1
                    : 0;
        }
    }

    if (!spot || state.brainSpotSight[index] != 1)
        spot = &ULDUAR_YOGG_SARON_BRAIN_ROOM_MIDDLE;

    return *spot;
}

Position const& YoggSaronPhaseThreeSpot(Player* bot)
{
    if (!PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot))
        return ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT;

    // Wide enough to see any Crusher near a standby spot from wherever the bot is, so one still walking
    // over from the brain team's exit picks the same spot as the rest.
    float const standbyReach = 40.0f;
    std::list<Creature*> crushers;
    bot->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE,
                                        bot->GetExactDist(&ULDUAR_YOGG_SARON_PHASE_3_RANGED_SPOT) + standbyReach);

    auto const clearance = [&crushers](Position const& spot)
    {
        float nearest = std::numeric_limits<float>::max();
        for (Creature* crusher : crushers)
            if (crusher->IsAlive())
                nearest = std::min(nearest, crusher->GetExactDist2d(spot));

        return nearest;
    };

    Position const* best = &ULDUAR_YOGG_SARON_PHASE_3_RANGED_SPOT;
    float bestClearance = clearance(*best);
    if (bestClearance >= ULDUAR_YOGG_SARON_PHASE_3_RANGED_CRUSHER_CLEARANCE)
        return *best;

    for (Position const& spot : ULDUAR_YOGG_SARON_PHASE_3_RANGED_STANDBY_SPOTS)
    {
        float const spotClearance = clearance(spot);
        if (spotClearance >= ULDUAR_YOGG_SARON_PHASE_3_RANGED_CRUSHER_CLEARANCE)
            return spot;

        if (spotClearance > bestClearance)
        {
            best = &spot;
            bestClearance = spotClearance;
        }
    }

    return *best;
}

std::vector<Unit*> GetYoggSaronSkullsInArc(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    // Both brain level facing triggers ask this every pass, and nothing in a pass turns the bot.
    YoggSaronPassReads* reads = YoggSaronPassReadsFor(botAI);
    if (reads && reads->skullsInArcRead)
    {
        std::vector<Unit*> cached;
        for (ObjectGuid const& guid : reads->skullsInArc)
            if (Creature* skull = ObjectAccessor::GetCreature(*bot, guid))
                cached.push_back(skull);

        return cached;
    }

    std::list<Creature*> skulls;
    bot->GetCreatureListWithEntryInGrid(skulls, NPC_LAUGHING_SKULL, ULDUAR_YOGG_SARON_LAUGHING_SKULL_RADIUS);

    std::vector<Unit*> inArc;
    for (Creature* skull : skulls)
    {
        if (!skull->IsAlive())
            continue;

        // The grid sweep above measures with both objects' bounding radii, so it hands back skulls the
        // 30 yd gaze cannot reach. Measure it plainly before answering for one.
        if (bot->GetExactDist2d(skull) > ULDUAR_YOGG_SARON_LAUGHING_SKULL_RADIUS)
            continue;

        // The exact filter the spell uses, so what the node answers for and what the raid is hit by
        // are the same set.
        if (bot->HasInArc(static_cast<float>(M_PI), skull))
            inArc.push_back(skull);
    }

    if (reads)
    {
        reads->skullsInArcRead = true;
        for (Unit* skull : inArc)
            reads->skullsInArc.push_back(skull->GetGUID());
    }

    return inArc;
}

std::vector<Position> GetYoggSaronSkullsInRange(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    std::list<Creature*> skulls;
    bot->GetCreatureListWithEntryInGrid(skulls, NPC_LAUGHING_SKULL, ULDUAR_YOGG_SARON_LAUGHING_SKULL_RADIUS);

    std::vector<Position> found;
    for (Creature* skull : skulls)
        if (skull->IsAlive() && bot->GetExactDist2d(skull) <= ULDUAR_YOGG_SARON_LAUGHING_SKULL_RADIUS)
            found.push_back(skull->GetPosition());

    return found;
}

float YoggSaronHeadingGap(float from, float to)
{
    float const gap = Position::NormalizeOrientation(from - to);

    return gap > static_cast<float>(M_PI) ? 2.0f * static_cast<float>(M_PI) - gap : gap;
}

uint32 YoggSaronSkullsAhead(std::vector<Position> const& skulls, float x, float y, float heading)
{
    uint32 ahead = 0;
    for (Position const& skull : skulls)
        if (YoggSaronHeadingGap(std::atan2(skull.GetPositionY() - y, skull.GetPositionX() - x), heading) <
            static_cast<float>(M_PI) / 2.0f)
            ++ahead;

    return ahead;
}

uint32 YoggSaronAngledSkullHeading(std::vector<Position> const& skulls, float x, float y, float targetBearing,
                                   float& heading)
{
    // Outward from the target, both sides at each step, so the first hit is also the smallest turn.
    heading = Position::NormalizeOrientation(targetBearing);
    uint32 fewest = YoggSaronSkullsAhead(skulls, x, y, heading);

    int32 const steps =
        static_cast<int32>(ULDUAR_YOGG_SARON_SKULL_TARGET_TURN / ULDUAR_YOGG_SARON_SKULL_TARGET_STEP + 0.5f);
    for (int32 step = 1; fewest && step <= steps; ++step)
    {
        float const turn = step * ULDUAR_YOGG_SARON_SKULL_TARGET_STEP;
        for (float const side : {turn, -turn})
        {
            float const candidate = Position::NormalizeOrientation(targetBearing + side);
            uint32 const ahead = YoggSaronSkullsAhead(skulls, x, y, candidate);
            if (ahead < fewest)
            {
                fewest = ahead;
                heading = candidate;
            }
        }
    }

    return fewest;
}

bool YoggSaronFacingClearOfSkulls(std::vector<Position> const& skulls, float x, float y, float targetX,
                                  float targetY)
{
    // Judged on the headings the target allows there, not on how far the walk moved the bot away from a
    // skull: it keeps hitting from wherever it stands.
    float heading = 0.0f;
    return YoggSaronAngledSkullHeading(skulls, x, y, std::atan2(targetY - y, targetX - x), heading) == 0;
}

YoggSaronPortalWave YoggSaronPortalWaveState(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    uint32 const now = getMSTime();

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);

    if (YoggSaronPhase(botAI) != 2)
    {
        state.phase2StartMs = 0;
        state.lastWaveMs = 0;
        state.nextWaveMs = 0;
        return YoggSaronPortalWave();
    }

    state.phase3StartMs = 0;
    state.lastRoarMs = 0;
    state.tankSwap = YoggSaronEncounterState::TankSwap();
    state.tankLohClaims.clear();
    state.kickNudges.clear();

    bool const portalsUp =
        bot->FindNearestCreature(NPC_DESCEND_INTO_MADNESS, ULDUAR_YOGG_SARON_PORTAL_SEARCH_RADIUS, true) != nullptr;

    if (!state.phase2StartMs)
    {
        state.phase2StartMs = now;
        state.nextWaveMs = now + ULDUAR_YOGG_SARON_PORTAL_FIRST_WAVE_MS;
    }

    // Only a sighting moves the clock. A bot underground sees no portal, and reading that silence as
    // "the wave ended" would restart the count every time the brain team went down.
    if (portalsUp &&
        (!state.lastWaveMs || getMSTimeDiff(state.lastWaveMs, now) > ULDUAR_YOGG_SARON_PORTAL_WAVE_DEBOUNCE_MS))
    {
        state.lastWaveMs = now;
        state.nextWaveMs = now + ULDUAR_YOGG_SARON_PORTAL_WAVE_PERIOD_MS;
        ++state.waveOrdinal;
    }
    // An unconfirmed prediction rolls on a whole period, else it reads 'any moment now' forever and parks the team.
    // Signed, not getMSTimeDiff: that reads a future time as a timer wrap, ~4.29e9 ms late.
    else if (!portalsUp && state.nextWaveMs &&
             static_cast<int32>(now - state.nextWaveMs) > static_cast<int32>(ULDUAR_YOGG_SARON_PORTAL_WAVE_DEBOUNCE_MS))
    {
        state.nextWaveMs += ULDUAR_YOGG_SARON_PORTAL_WAVE_PERIOD_MS;
    }

    YoggSaronPortalWave answer;
    answer.active = true;
    answer.ordinal = state.waveOrdinal;
    answer.portalsUp = portalsUp;

    int32 const remaining = static_cast<int32>(state.nextWaveMs - now);
    answer.msToNextWave = remaining > 0 ? static_cast<uint32>(remaining) : 0;

    return answer;
}

std::vector<Player*> GetYoggSaronBrainTeam(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Group* group = bot->GetGroup();
    if (!group)
        return {};

    uint32 size = IsRaid25Man(bot) ? ULDUAR_YOGG_SARON_PORTAL_SPOTS_25MAN
                                  : ULDUAR_YOGG_SARON_PORTAL_SPOTS_10MAN;

    // A non-tank master takes a portal of its own, and it is not ours to move.
    Player* master = botAI->GetMaster();
    bool const masterTakesOne = master && !PlayerbotAI::IsTank(master);
    if (masterTakesOne && size)
        --size;

    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);

    std::vector<Player*> melee, healers, ranged;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || PlayerbotAI::IsTank(member))
            continue;

        if (masterTakesOne && member == master)
            continue;

        if (state.benched.count(member->GetGUID()))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else if (PlayerbotAI::IsMelee(member))
            melee.push_back(member);
        else
            ranged.push_back(member);
    }

    // By GUID inside each band, because every bot derives this list for itself and group order is not
    // the same walk from every seat.
    auto const byGuid = [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); };
    std::sort(melee.begin(), melee.end(), byGuid);
    std::sort(ranged.begin(), ranged.end(), byGuid);

    // Val'anyr's wielder first, its heals also shield and item level can't tell it from the next healer.
    // Then by heals on the move (a moving bot can't start a cast-time spell), then GUID.
    auto const wieldsValanyr = [](Player* healer)
    {
        Item* mainHand = healer->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        return mainHand && mainHand->GetEntry() == ITEM_VALANYR;
    };
    auto const mobileRank = [](Player* healer)
    {
        switch (healer->getClass())
        {
            case CLASS_PRIEST:
                return 0;
            case CLASS_DRUID:
                return 1;
            case CLASS_SHAMAN:
                return 2;
            case CLASS_PALADIN:
                return 3;
            default:
                return 4;
        }
    };
    std::sort(healers.begin(), healers.end(),
              [&](Player* left, Player* right)
              {
                  bool const leftValanyr = wieldsValanyr(left);
                  if (leftValanyr != wieldsValanyr(right))
                      return leftValanyr;

                  int const leftRank = mobileRank(left);
                  int const rightRank = mobileRank(right);
                  return leftRank != rightRank ? leftRank < rightRank : left->GetGUID() < right->GetGUID();
              });

    std::vector<Player*> team = melee;
    if (!healers.empty())
        team.push_back(healers.front());
    team.insert(team.end(), ranged.begin(), ranged.end());
    team.insert(team.end(), healers.begin() + (healers.empty() ? 0 : 1), healers.end());

    if (team.size() > size)
        team.resize(size);

    return team;
}

namespace
{
void YoggSaronBenchLowSanity(Player* bot, YoggSaronEncounterState& state)
{
    std::unordered_set<ObjectGuid> low;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && YoggSaronSanityOf(member) < ULDUAR_YOGG_SARON_BRAIN_TEAM_MIN_SANITY)
                low.insert(member->GetGUID());
        }
    }

    std::vector<ObjectGuid> const was(state.benched.begin(), state.benched.end());
    for (ObjectGuid const& guid : was)
        if (!low.count(guid))
            state.benched.erase(guid);

    for (ObjectGuid const& guid : low)
        state.benched.insert(guid);
}
}  // namespace

YoggSaronPortalIntent YoggSaronPortalPlan(PlayerbotAI* botAI, Position& spot)
{
    Player* bot = botAI->GetBot();

    YoggSaronPortalWave const wave = YoggSaronPortalWaveState(botAI);
    if (!wave.active)
        return YOGG_SARON_PORTAL_NOT_TEAM;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);

    uint32 const spotCount = IsRaid25Man(bot) ? ULDUAR_YOGG_SARON_PORTAL_SPOTS_25MAN
                                             : ULDUAR_YOGG_SARON_PORTAL_SPOTS_10MAN;

    // The wave the spread is for: the one that is up, or the next one if none is.
    uint32 const assignmentWave = wave.ordinal + (wave.portalsUp ? 0 : 1);

    // The team is back once this wave's Induce Madness is over, the first moment its Sanity says who
    // can go again. Rebuilt then as well, so a replacement still has the ~20 s before the portals.
    bool teamBack = false;
    if (!wave.portalsUp && wave.ordinal && state.returnPlanWave != assignmentWave)
    {
        Creature* brain = YoggSaronNearestCreature(botAI, NPC_BRAIN);
        teamBack = brain && brain->IsAlive() && !brain->FindCurrentSpellBySpellId(SPELL_INDUCE_MADNESS);
    }

    bool const newWave = state.slotWave != assignmentWave;

    // Latched per wave: nearest-first churns as bots walk, and a mid-approach swap costs both bots the window.
    // Rebuilt once when the portals arrive, else the slots stand from the previous wave's despawn, 55 s earlier.
    if (newWave || teamBack || (wave.portalsUp && !state.slotPortalsUp))
    {
        if (teamBack)
            state.returnPlanWave = assignmentWave;

        // Not when the portals arrive. That rebuild only moves spots.
        if (newWave || teamBack)
            YoggSaronBenchLowSanity(bot, state);

        state.slotWave = assignmentWave;
        state.slotPortalsUp = wave.portalsUp;
        state.portalSlot.clear();
        state.brainTeam.clear();

        std::vector<bool> taken(spotCount, false);
        for (Player* member : GetYoggSaronBrainTeam(botAI))
        {
            state.brainTeam.insert(member->GetGUID());

            uint32 best = spotCount;
            float bestDistance = 0.0f;
            for (uint32 candidate = 0; candidate < spotCount; ++candidate)
            {
                if (taken[candidate])
                    continue;

                float const distance = member->GetExactDist2d(ULDUAR_YOGG_SARON_PORTAL_SPOTS[candidate]);
                if (best == spotCount || distance < bestDistance)
                {
                    best = candidate;
                    bestDistance = distance;
                }
            }

            if (best == spotCount)
                break;

            taken[best] = true;
            state.portalSlot[member->GetGUID()] = static_cast<uint8>(best);
        }
    }

    uint8 slot = 0;
    bool assigned = false;
    auto const assignment = state.portalSlot.find(bot->GetGUID());
    if (assignment != state.portalSlot.end())
    {
        slot = assignment->second;
        assigned = true;
    }

    YoggSaronPortalIntent intent = YOGG_SARON_PORTAL_NOT_TEAM;
    if (assigned)
    {
        spot = ULDUAR_YOGG_SARON_PORTAL_SPOTS[slot];

        // A portal is one use and the click takes the nearest, so a slot is only a suggestion.
        // Walk to a portal that is still there, not the one handed out.
        if (wave.portalsUp)
        {
            std::list<Creature*> portals;
            bot->GetCreatureListWithEntryInGrid(portals, NPC_DESCEND_INTO_MADNESS,
                                                ULDUAR_YOGG_SARON_PORTAL_SEARCH_RADIUS);

            Creature* mine = nullptr;
            Creature* nearest = nullptr;
            for (Creature* portal : portals)
            {
                if (!portal->IsAlive())
                    continue;

                if (portal->GetExactDist2d(spot) <= ULDUAR_YOGG_SARON_PORTAL_CLICK_RADIUS)
                    mine = portal;

                if (!nearest || bot->GetExactDist2d(portal) < bot->GetExactDist2d(nearest))
                    nearest = portal;
            }

            if (!mine && nearest)
                spot = nearest->GetPosition();
        }

        float const distance = bot->GetExactDist2d(spot);
        if (distance <= ULDUAR_YOGG_SARON_PORTAL_CLICK_RADIUS)
            intent = YOGG_SARON_PORTAL_HOLDING;
        else if (wave.portalsUp)
            intent = YOGG_SARON_PORTAL_LATE;
        else
        {
            float const speed = bot->GetSpeed(MOVE_RUN);
            uint32 lead = ULDUAR_YOGG_SARON_PORTAL_SPREAD_LEAD_FLOOR_MS;
            if (speed > 0.0f)
                lead = std::max(lead, static_cast<uint32>(distance / speed *
                                                          ULDUAR_YOGG_SARON_PORTAL_SPREAD_LEAD_SAFETY *
                                                          IN_MILLISECONDS));

            intent = wave.msToNextWave > lead ? YOGG_SARON_PORTAL_WAITING : YOGG_SARON_PORTAL_SPREADING;
        }
    }

    return intent;
}

bool YoggSaronPortalWalkPending(PlayerbotAI* botAI)
{
    Position spot;
    YoggSaronPortalIntent const intent = YoggSaronPortalPlan(botAI, spot);

    return intent == YOGG_SARON_PORTAL_SPREADING || intent == YOGG_SARON_PORTAL_HOLDING ||
           intent == YOGG_SARON_PORTAL_LATE;
}

bool YoggSaronHoldingPortalSpot(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return false;

    // A slot left over from the last wave only lets the plan be asked; the plan has the answer.
    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);
    auto const assignment = state.portalSlot.find(bot->GetGUID());
    if (assignment == state.portalSlot.end() ||
        bot->GetExactDist2d(ULDUAR_YOGG_SARON_PORTAL_SPOTS[assignment->second]) > ULDUAR_YOGG_SARON_PORTAL_CLICK_RADIUS)
        return false;

    Position spot;
    return YoggSaronPortalPlan(botAI, spot) == YOGG_SARON_PORTAL_HOLDING;
}

namespace
{
// Yogg without the barrier is also phase 3, where ACTION_YOGG_SARON_START_P3 strips it. The Brain tells them
// apart: summoned the tick the barrier lands, so absent for this whole window. The live check spares other bots.
bool YoggSaronHandoverOpen(PlayerbotAI* botAI)
{
    if (!BossStateGateLive(botAI, ULD_BOSS_YOGGSARON))
        return false;

    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    return yogg && yogg->IsAlive() && !yogg->HasAura(SPELL_SHADOW_BARRIER) &&
           !YoggSaronNearestCreature(botAI, NPC_BRAIN);
}

void YoggSaronTickHandover(PlayerbotAI* botAI)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(botAI->GetBot());
    uint32 const now = getMSTime();

    if (YoggSaronHandoverOpen(botAI))
    {
        state.handoverRingMs = 0;
        if (!state.handoverStartMs)
            state.handoverStartMs = now;

        return;
    }

    // The window and the ring are the same tick, so the moment the window closes is the moment to
    // start the hold from.
    if (state.handoverStartMs)
    {
        state.handoverRingMs = now;
        state.handoverStartMs = 0;
    }

    if (state.handoverRingMs && getMSTimeDiff(state.handoverRingMs, now) >= ULDUAR_YOGG_SARON_HANDOVER_HOLD_MS)
        state.handoverRingMs = 0;
}

// Only the exit portal resets the mark to skull, so Induce Madness or a death strands a room mark.
// A bot on the platform is not in an illusion room, whatever its mark says.
void YoggSaronTickRtiMark(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (YoggSaronRoomOf(bot) != YOGG_SARON_ROOM_ARENA)
        return;

    // Diamond is the walk-to-a-portal mark and it is set up here, so it is the one that belongs.
    std::string const mark = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    if (mark == "skull" || mark == "diamond")
        return;

    botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("skull");
}
}  // namespace

YoggSaronHandover YoggSaronHandoverState(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);

    YoggSaronHandover answer;
    uint32 const now = getMSTime();
    answer.active = state.handoverStartMs != 0;

    bool const holding = !answer.active && state.handoverRingMs &&
                         getMSTimeDiff(state.handoverRingMs, now) < ULDUAR_YOGG_SARON_HANDOVER_HOLD_MS;

    if (!answer.active && !holding)
        return answer;

    // No radius in the test: the anchor is outside the clearance radius, so an arrived bot would read as not
    // needing the handover and hand itself back to the leash.
    answer.clearing = PlayerbotAI::IsTank(bot) || PlayerbotAI::IsMelee(bot);

    return answer;
}

bool YoggSaronInBodyKnockback(Player* player)
{
    // Height counts as much as radius: the brain room's middle is 2.3 yd from the arena's in x and y but
    // 93 yd below, so a flat 2D ring swallows the room and spacing pushes bots off their spot.
    return player && player->GetPositionZ() > ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT &&
           player->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(),
                                 ULDUAR_YOGG_SARON_MIDDLE.GetPositionY()) <
               ULDUAR_YOGG_SARON_BODY_KNOCKBACK_RADIUS;
}

namespace
{
// Closest approach of the walk from (fromX, fromY) to (toX, toY) to a point, clamped to the segment.
float ClosestApproach(float fromX, float fromY, float toX, float toY, Position const& to)
{
    float const legX = toX - fromX;
    float const legY = toY - fromY;
    float const legSquared = legX * legX + legY * legY;

    float along = 0.0f;
    if (legSquared > 0.0f)
    {
        along = ((to.GetPositionX() - fromX) * legX + (to.GetPositionY() - fromY) * legY) / legSquared;
        along = std::max(0.0f, std::min(1.0f, along));
    }

    return to.GetExactDist2d(fromX + legX * along, fromY + legY * along);
}
}  // namespace

bool YoggSaronRouteClearOfBody(Player* bot, float x, float y)
{
    // No opinion from inside it. Every short step out of the ring passes close to the middle by
    // definition, so a filter that judged those would reject the only walks that end the problem.
    if (YoggSaronInBodyKnockback(bot))
        return true;

    return ClosestApproach(bot->GetPositionX(), bot->GetPositionY(), x, y, ULDUAR_YOGG_SARON_MIDDLE) >=
           ULDUAR_YOGG_SARON_BODY_KNOCKBACK_CLEAR_RADIUS;
}

bool YoggSaronRouteClearOfDeathRays(Player* bot, std::vector<Position> const& rays, float x, float y)
{
    // A ray turns every 1.6 s at ~5.5 yd/s, so a spot clear of where it is now isn't enough when the walk
    // there crosses its lane. Inside the clear radius the walk may only keep or gain distance.
    constexpr float slack = 0.05f;
    for (Position const& ray : rays)
    {
        float const now = ray.GetExactDist2d(bot->GetPositionX(), bot->GetPositionY());
        float const keep = std::min(now, ULDUAR_YOGG_SARON_DEATH_RAY_CLEAR_RADIUS);
        if (ClosestApproach(bot->GetPositionX(), bot->GetPositionY(), x, y, ray) < keep - slack)
            return false;
    }

    return true;
}

bool YoggSaronDeathRayNear(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    return bot->GetPositionZ() > ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT &&
           bot->FindNearestCreature(NPC_DEATH_RAY, ULDUAR_YOGG_SARON_DEATH_RAY_CLEAR_RADIUS, true);
}

bool YoggSaronBodyDetour(Player* bot, Position const& destination, Position& waypoint)
{
    if (YoggSaronRouteClearOfBody(bot, destination.GetPositionX(), destination.GetPositionY()))
        return false;

    float const middleX = ULDUAR_YOGG_SARON_MIDDLE.GetPositionX();
    float const middleY = ULDUAR_YOGG_SARON_MIDDLE.GetPositionY();

    float const from = std::atan2(bot->GetPositionY() - middleY, bot->GetPositionX() - middleX);
    float const to = std::atan2(destination.GetPositionY() - middleY, destination.GetPositionX() - middleX);

    // Halfway round the shorter arc. Walking it leaves the bot facing at most a quarter turn to the
    // destination, whose own chord clears the ring, so one waypoint is the whole detour.
    float turn = Position::NormalizeOrientation(to - from);
    if (turn > static_cast<float>(M_PI))
        turn -= 2.0f * static_cast<float>(M_PI);

    float const heading = from + turn / 2.0f;

    waypoint = Position(middleX + std::cos(heading) * ULDUAR_YOGG_SARON_BODY_DETOUR_RADIUS,
                        middleY + std::sin(heading) * ULDUAR_YOGG_SARON_BODY_DETOUR_RADIUS,
                        destination.GetPositionZ());

    return true;
}

Position YoggSaronBodyRoute(Player* bot, Position const& spot)
{
    Position waypoint;
    if (!YoggSaronBodyDetour(bot, spot, waypoint))
        return spot;

    return waypoint;
}

bool IsYoggSaronHoldableGuardian(Unit* unit)
{
    if (!unit || !unit->IsAlive())
        return false;

    uint32 const entry = unit->GetEntry();
    if (entry != NPC_IMMORTAL_GUARDIAN && entry != NPC_MARKED_IMMORTAL_GUARDIAN)
        return false;

    if (unit->HasAura(SPELL_WEAKENED))
        return false;

    // One being dragged off a beacon has a dragger on it, so guardian control taunting it back to the
    // pile would undo the drag.
    return !yoggSaronStates.For(unit->GetInstanceId()).beaconDrags.count(unit->GetGUID());
}

bool YoggSaronHoldableGuardianWithin(Player* bot, float radius)
{
    for (uint32 const entry : {NPC_IMMORTAL_GUARDIAN, NPC_MARKED_IMMORTAL_GUARDIAN})
    {
        std::list<Creature*> guardians;
        bot->GetCreatureListWithEntryInGrid(guardians, entry, radius);
        for (Creature* guardian : guardians)
            if (IsYoggSaronHoldableGuardian(guardian))
                return true;
    }

    return false;
}

namespace
{
// The check behind "nearest npcs", cut down to the Immortal Guardians before the range test.
struct ImmortalGuardianInRangeCheck
{
    Acore::AnyUnitInObjectRangeCheck inRange;

    bool operator()(Unit* unit)
    {
        uint32 const entry = unit->GetEntry();
        return (entry == NPC_IMMORTAL_GUARDIAN || entry == NPC_MARKED_IMMORTAL_GUARDIAN) && inRange(unit);
    }
};
}  // namespace

GuidVector GetYoggSaronGuardiansInSight(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    float const range = sPlayerbotAIConfig.SightDistance;

    std::vector<Unit*> found;
    ImmortalGuardianInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(bot, range)};
    Acore::UnitListSearcher<ImmortalGuardianInRangeCheck> searcher(bot, found, check);
    Cell::VisitObjects(bot, searcher, range);

    GuidVector guardians;
    for (Unit* unit : found)
        if (bot->IsWithinLOSInMap(unit))
            guardians.push_back(unit->GetGUID());

    return guardians;
}

bool YoggSaronGuardianLoose(PlayerbotAI* botAI)
{
    return YoggSaronGuardianLoose(botAI, GetYoggSaronGuardiansInSight(botAI));
}

bool YoggSaronGuardianLoose(PlayerbotAI* botAI, GuidVector const& guardians)
{
    for (ObjectGuid const& guid : guardians)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsYoggSaronHoldableGuardian(unit))
            continue;

        Player* victim = unit->GetVictim() ? unit->GetVictim()->ToPlayer() : nullptr;
        if (!victim || !PlayerbotAI::IsTank(victim))
            return true;
    }

    return false;
}

namespace
{
// The taunt a class can point at one Guardian, by id so another player's cooldown can be read.
uint32 YoggSaronTauntSpellId(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_WARRIOR:
            return SPELL_WARRIOR_TAUNT;
        case CLASS_PALADIN:
            return SPELL_PALADIN_HAND_OF_RECKONING;
        case CLASS_DEATH_KNIGHT:
            return SPELL_DEATH_KNIGHT_DARK_COMMAND;
        case CLASS_DRUID:
            return SPELL_DRUID_GROWL;
        case CLASS_HUNTER:
            return SPELL_DISTRACTING_SHOT;
        default:
            return 0;
    }
}

bool IsYoggSaronBot(Player* player) { return player && player->GetSession() && player->GetSession()->IsHeadless(); }

// Claim order: bot tanks while another tank holds the pile, then ret paladins and dps DKs, then hunters,
// each by GUID. Never one an unmarked Guardian is hitting, and only with taunt ready within `tauntWithinMs`.
std::vector<Player*> YoggSaronBeaconDraggerOrder(Player* bot, std::list<Creature*> const& unmarked,
                                                 uint32 tauntWithinMs)
{
    std::vector<Player*> tanks;
    Group* group = bot->GetGroup();
    if (!group)
        return tanks;

    std::unordered_set<ObjectGuid> held;
    for (Creature* guardian : unmarked)
        if (guardian->IsAlive() && guardian->GetVictim())
            held.insert(guardian->GetVictim()->GetGUID());

    uint32 tanksAlive = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        if (Player* member = ref->GetSource(); member && member->IsAlive() && PlayerbotAI::IsTank(member))
            ++tanksAlive;

    std::vector<Player*> backups;
    // Last resort, for a mark nobody with a real taunt is left for: an undragged one heals the pile to full.
    // Distracting Shot is only 6 s.
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsYoggSaronBot(member) || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
            member->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT ||
            held.count(member->GetGUID()) || YoggSaronBeaconDragging(member))
            continue;

        uint32 const taunt = YoggSaronTauntSpellId(member);
        if (!taunt || !member->HasSpell(taunt) || member->GetSpellCooldownDelay(taunt) > tauntWithinMs)
            continue;

        // A bot tank only while another tank is left holding the pile.
        if (PlayerbotAI::IsTank(member))
        {
            if (tanksAlive > 1)
                tanks.push_back(member);
        }
        else if (!PlayerbotAI::IsHeal(member) &&
                 (member->getClass() == CLASS_PALADIN || member->getClass() == CLASS_DEATH_KNIGHT))
            backups.push_back(member);
        else if (member->getClass() == CLASS_HUNTER)
            hunters.push_back(member);
    }

    auto const byGuid = [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); };
    std::sort(tanks.begin(), tanks.end(), byGuid);
    std::sort(backups.begin(), backups.end(), byGuid);
    std::sort(hunters.begin(), hunters.end(), byGuid);
    tanks.insert(tanks.end(), backups.begin(), backups.end());
    tanks.insert(tanks.end(), hunters.begin(), hunters.end());

    return tanks;
}

bool YoggSaronDrainLifeChannelling(Unit* guardian)
{
    return guardian->FindCurrentSpellBySpellId(SPELL_GUARDIAN_DRAIN_LIFE) ||
           guardian->FindCurrentSpellBySpellId(SPELL_GUARDIAN_DRAIN_LIFE_H);
}

// How long the Guardian has been channelling Drain Life, -1 when it isn't.
int32 YoggSaronDrainLifeElapsed(Unit* guardian)
{
    Spell* channel = guardian->FindCurrentSpellBySpellId(SPELL_GUARDIAN_DRAIN_LIFE);
    if (!channel)
        channel = guardian->FindCurrentSpellBySpellId(SPELL_GUARDIAN_DRAIN_LIFE_H);

    return channel ? std::max(0, channel->GetSpellInfo()->GetDuration() - channel->GetCastTimeRemaining()) : -1;
}

// The Drain Life a kick can stop. A channel has no cast time, so Spell::EffectInterruptCast only takes it once it's
// CASTING. A kick in its first update is spent for nothing, cooldown and all.
Spell* YoggSaronDrainLifeKickable(Unit* guardian)
{
    Spell* channel = guardian->FindCurrentSpellBySpellId(SPELL_GUARDIAN_DRAIN_LIFE);
    if (!channel)
        channel = guardian->FindCurrentSpellBySpellId(SPELL_GUARDIAN_DRAIN_LIFE_H);

    return channel && channel->getState() == SPELL_STATE_CASTING ? channel : nullptr;
}

// Only the strongest cut counts in Unit::SpellHealingBonusTaken, a second one adds nothing.
bool YoggSaronHealingCut(Unit* guardian)
{
    return guardian->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_HEALING_PCT) <= ULDUAR_YOGG_SARON_HEALING_CUT_PCT;
}

bool YoggSaronBeaconHealOn(Unit* guardian)
{
    return guardian->HasAura(SPELL_EMPOWERING_SHADOWS_25) || guardian->HasAura(SPELL_EMPOWERING_SHADOWS_10);
}
}  // namespace

Unit* YoggSaronBeaconDragGuardian(PlayerbotAI* botAI, bool claim)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);

    if (!YoggSaronInPhase3(botAI) || YoggThorimKeeperActive(botAI))
    {
        state.beaconDrags.clear();
        state.beaconReturns.clear();
        state.beaconExternalClaims.clear();
        return nullptr;
    }

    // A drag ends with its Guardian or its dragger, or once the heal has landed. Yogg's missile goes out
    // as the mark ends and the entry swaps back, so the mark alone lets go a second early.
    uint32 const now = getMSTime();
    for (auto it = state.beaconDrags.begin(); it != state.beaconDrags.end();)
    {
        Creature* guardian = ObjectAccessor::GetCreature(*bot, it->first);
        Player* dragger = ObjectAccessor::GetPlayer(*bot, it->second.dragger);
        bool done = !guardian || !guardian->IsAlive() || !dragger || !dragger->IsAlive();

        // A taunt that didn't take goes to the next bot with one ready. Not under a Drain Life already running at
        // the taunt: a casting Guardian skips its victim update. One started later doesn't hide it.
        int32 const channelMs = done ? -1 : YoggSaronDrainLifeElapsed(guardian);
        uint32 const sinceTaunt = getMSTimeDiff(it->second.tauntMs, now);
        if (!done && it->second.tauntMs && guardian->GetVictim() != dragger &&
            sinceTaunt >= ULDUAR_YOGG_SARON_BEACON_TAUNT_CHECK_MS &&
            (channelMs < 0 || sinceTaunt > static_cast<uint32>(channelMs) + ULDUAR_YOGG_SARON_BEACON_TAUNT_LAND_MS))
        {
            it = state.beaconDrags.erase(it);
            continue;
        }

        if (!done && guardian->GetEntry() != NPC_MARKED_IMMORTAL_GUARDIAN)
        {
            if (!it->second.markEndedMs)
                it->second.markEndedMs = now;

            done = guardian->HasAura(SPELL_EMPOWERING_SHADOWS_25) ||
                   guardian->HasAura(SPELL_EMPOWERING_SHADOWS_10) ||
                   getMSTimeDiff(it->second.markEndedMs, now) >= ULDUAR_YOGG_SARON_BEACON_HEAL_WAIT_MS;

            if (done && dragger)
                state.beaconReturns[it->first] = {dragger->GetGUID(), now};
        }

        it = done ? state.beaconDrags.erase(it) : std::next(it);
    }

    for (auto it = state.beaconReturns.begin(); it != state.beaconReturns.end();)
        it = getMSTimeDiff(it->second.releasedMs, now) > ULDUAR_YOGG_SARON_BEACON_RETURN_MS
                 ? state.beaconReturns.erase(it)
                 : std::next(it);

    for (auto const& [guid, drag] : state.beaconDrags)
        if (drag.dragger == bot->GetGUID())
            return ObjectAccessor::GetCreature(*bot, guid);

    // Swept from Yogg so every bot pairs the same list.
    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    if (!yogg)
        return nullptr;

    std::list<Creature*> found;
    yogg->GetCreatureListWithEntryInGrid(found, NPC_MARKED_IMMORTAL_GUARDIAN,
                                         ULDUAR_YOGG_SARON_BEACON_DRAG_SEARCH_RADIUS);

    std::vector<Creature*> marked;
    for (Creature* guardian : found)
        if (guardian->IsAlive() && !state.beaconDrags.count(guardian->GetGUID()))
            marked.push_back(guardian);

    if (marked.empty())
        return nullptr;

    std::list<Creature*> unmarked;
    yogg->GetCreatureListWithEntryInGrid(unmarked, NPC_IMMORTAL_GUARDIAN, ULDUAR_YOGG_SARON_BEACON_DRAG_SEARCH_RADIUS);

    std::vector<Player*> const tanks = YoggSaronBeaconDraggerOrder(bot, unmarked, 0);
    std::sort(marked.begin(), marked.end(),
              [](Creature* left, Creature* right) { return left->GetGUID() < right->GetGUID(); });

    for (size_t i = 0; i < marked.size() && i < tanks.size(); ++i)
    {
        if (tanks[i] != bot)
            continue;

        if (claim)
        {
            // A lane per mark: the heal stacks per caster on Guardians within 20 yd, up to 112.5k/s.
            // Staged lane if free, else furthest free; N and S beat NE on a tie (55 deg off N), then nearer.
            std::array<bool, YOGG_SARON_BEACON_LANES.size()> taken{};
            for (auto const& [guid, drag] : state.beaconDrags)
                taken[drag.lane] = true;

            auto const clearance = [&unmarked](Position const& spot)
            {
                float nearest = std::numeric_limits<float>::max();
                for (Creature* guardian : unmarked)
                    if (guardian->IsAlive())
                        nearest = std::min(nearest, guardian->GetExactDist2d(spot));

                return nearest;
            };

            uint8 lane = 0;
            auto const staged = state.beaconStages.find(bot->GetGUID());
            if (staged != state.beaconStages.end() && !taken[staged->second])
                lane = staged->second;
            else
            {
                std::tuple<bool, float, bool, float> best;
                for (uint8 index = 0; index < YOGG_SARON_BEACON_LANES.size(); ++index)
                {
                    Position const& spot = *YOGG_SARON_BEACON_LANES[index].drag;
                    auto const key = std::make_tuple(taken[index], -clearance(spot),
                                                     index == YOGG_SARON_BEACON_LANE_NORTHEAST,
                                                     bot->GetExactDist2d(spot));
                    if (!index || key < best)
                    {
                        lane = index;
                        best = key;
                    }
                }
            }

            state.beaconDrags[marked[i]->GetGUID()] = {bot->GetGUID(), 0, lane, 0};
        }

        return marked[i];
    }

    return nullptr;
}

void YoggSaronBeaconTaunted(Player* bot)
{
    for (auto& [guid, drag] : YoggSaronStateFor(bot).beaconDrags)
        if (drag.dragger == bot->GetGUID())
            drag.tauntMs = getMSTime();
}

bool YoggSaronBeaconDragging(Player* bot)
{
    for (auto const& [guid, drag] : YoggSaronStateFor(bot).beaconDrags)
        if (drag.dragger == bot->GetGUID())
            return true;

    return false;
}

Position const* YoggSaronBeaconStageSpot(Player* bot)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    if (state.markWindow)
        return nullptr;

    auto const it = state.beaconStages.find(bot->GetGUID());
    if (it == state.beaconStages.end())
        return nullptr;

    YoggSaronBeaconLane const& lane = YOGG_SARON_BEACON_LANES[it->second];

    return bot->getClass() == CLASS_HUNTER ? lane.hunter : lane.stage;
}

namespace
{
YoggSaronBeaconLane const& YoggSaronBeaconLaneOf(Player* bot)
{
    for (auto const& [guid, drag] : YoggSaronStateFor(bot).beaconDrags)
        if (drag.dragger == bot->GetGUID())
            return YOGG_SARON_BEACON_LANES[drag.lane];

    return YOGG_SARON_BEACON_LANES.front();
}
}  // namespace

Position const& YoggSaronBeaconDragSpot(Player* bot) { return *YoggSaronBeaconLaneOf(bot).drag; }

Position const& YoggSaronBeaconHunterSpot(Player* bot) { return *YoggSaronBeaconLaneOf(bot).hunter; }

Unit* YoggSaronBeaconReturning(Player* bot)
{
    uint32 const now = getMSTime();
    for (auto const& [guid, release] : YoggSaronStateFor(bot).beaconReturns)
    {
        if (release.dragger != bot->GetGUID() ||
            getMSTimeDiff(release.releasedMs, now) > ULDUAR_YOGG_SARON_BEACON_RETURN_MS)
            continue;

        Creature* guardian = ObjectAccessor::GetCreature(*bot, guid);
        if (guardian && guardian->IsAlive() && guardian->GetVictim() == bot)
            return guardian;
    }

    return nullptr;
}

namespace
{
// Still marked, or carrying the heal, and past the danger line.
bool IsYoggSaronHealedGuardian(Unit* unit)
{
    if (!unit || !unit->IsAlive() || unit->GetHealthPct() < ULDUAR_YOGG_SARON_BEACON_DANGER_HEALTH_PCT)
        return false;

    uint32 const entry = unit->GetEntry();
    if (entry == NPC_MARKED_IMMORTAL_GUARDIAN)
        return true;

    return entry == NPC_IMMORTAL_GUARDIAN &&
           (unit->HasAura(SPELL_EMPOWERING_SHADOWS_25) || unit->HasAura(SPELL_EMPOWERING_SHADOWS_10));
}
}  // namespace

Unit* YoggSaronHealedGuardianOn(PlayerbotAI* /*botAI*/, Player* victim)
{
    if (!victim)
        return nullptr;

    // The victim's own attacker set, no sweep: this is asked from block predicates.
    for (Unit* attacker : victim->getAttackers())
        if (attacker->GetVictim() == victim && IsYoggSaronHealedGuardian(attacker))
            return attacker;

    return nullptr;
}

uint32 YoggSaronGuardiansOn(Player* tank)
{
    if (!tank)
        return 0;

    uint32 count = 0;
    for (Unit* attacker : tank->getAttackers())
        if (attacker->GetVictim() == tank && IsYoggSaronHoldableGuardian(attacker))
            ++count;

    return count;
}

uint32 YoggSaronPileHealedOn(Player* tank)
{
    if (!tank)
        return 0;

    // Every application, not GetAura: Empowering Shadows stacks per caster, and a returned mark carries its
    // own next to anyone else's.
    auto const healedByOther = [](Unit* guardian)
    {
        for (uint32 const spell : {SPELL_EMPOWERING_SHADOWS_25, SPELL_EMPOWERING_SHADOWS_10})
        {
            auto const range = guardian->GetAppliedAuras().equal_range(spell);
            for (auto it = range.first; it != range.second; ++it)
                if (it->second->GetBase()->GetCasterGUID() != guardian->GetGUID())
                    return true;
        }

        return false;
    };

    uint32 count = 0;
    for (Unit* attacker : tank->getAttackers())
    {
        uint32 const entry = attacker->GetEntry();
        if (attacker->GetVictim() == tank && attacker->IsAlive() &&
            (entry == NPC_IMMORTAL_GUARDIAN || entry == NPC_MARKED_IMMORTAL_GUARDIAN) && healedByOther(attacker))
            ++count;
    }

    return count;
}

Unit* YoggSaronMortalStrikeTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (YoggSaronOnBrainLevel(bot) || !YoggSaronInPhase3(botAI) || YoggThorimKeeperActive(botAI))
        return nullptr;

    // The grid radius counts both object sizes, so this covers melee reach. IsWithinMeleeRange decides.
    constexpr float searchRadius = 10.0f;
    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_IMMORTAL_GUARDIAN, searchRadius);

    bool const gaze = YoggSaronGazeActive(botAI);
    Creature* yogg = gaze ? YoggSaronNearestCreature(botAI, NPC_YOGG_SARON) : nullptr;

    Unit* best = nullptr;
    size_t bestTier = 0;
    for (Creature* guardian : guardians)
    {
        if (!guardian->IsAlive() || YoggSaronHealingCut(guardian) || !bot->IsWithinMeleeRange(guardian))
            continue;

        // Low enough to drain means its next channel comes within 10.5-14.5 s, mostly under the 10 s cut. Better
        // than Yogg, where the cut does nothing.
        size_t tier = 2;
        if (YoggSaronBeaconHealOn(guardian))
            tier = 0;
        else if (YoggSaronDrainLifeChannelling(guardian))
            tier = 1;
        else if (guardian->GetHealthPct() >= ULDUAR_YOGG_SARON_DRAIN_LIFE_HEALTH_PCT)
            continue;

        if ((gaze && (!yogg || !YoggSaronOutsideGaze(bot, yogg, guardian))) ||
            !YoggSaronGuardianThreatAllows(bot, guardian, guardian->GetGUID() == bot->GetTarget()))
            continue;

        if (!best || std::make_tuple(tier, guardian->GetHealth()) < std::make_tuple(bestTier, best->GetHealth()))
        {
            best = guardian;
            bestTier = tier;
        }
    }

    return best;
}

char const* YoggSaronWarlockSummon(PlayerbotAI* botAI)
{
    static constexpr std::pair<char const*, char const*> pets[] = {{"felhunter", "summon felhunter"},
                                                                   {"felguard", "summon felguard"},
                                                                   {"imp", "summon imp"},
                                                                   {"voidwalker", "summon voidwalker"},
                                                                   {"succubus", "summon succubus"}};
    for (auto const& [strategy, summon] : pets)
        if (botAI->HasStrategy(strategy, BOT_STATE_NON_COMBAT))
            return summon;

    return nullptr;
}

Player* YoggSaronBeaconExternalVictim(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();

    Player* best = nullptr;
    bool bestDragger = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
            !YoggSaronHealedGuardianOn(botAI, member))
            continue;

        bool const dragger = YoggSaronBeaconDragging(member) || YoggSaronBeaconReturning(member);
        if (PlayerbotAI::IsTank(member) && !dragger &&
            member->GetHealthPct() >= ULDUAR_YOGG_SARON_BEACON_TANK_EXTERNAL_PCT)
            continue;

        auto const claim = state.beaconExternalClaims.find(member->GetGUID());
        if (claim != state.beaconExternalClaims.end() &&
            getMSTimeDiff(claim->second, now) < ULDUAR_YOGG_SARON_BEACON_EXTERNAL_CLAIM_MS)
            continue;

        // A bot presses its own first. A human gets one whatever they hold, since nothing says they'll press it.
        if (IsYoggSaronBot(member) && (RunningTankDefensive(member, true, 0) || ReadyTankDefensive(member, true)))
            continue;

        if (!best || (dragger && !bestDragger) ||
            (dragger == bestDragger && member->GetHealthPct() < best->GetHealthPct()))
        {
            best = member;
            bestDragger = dragger;
        }
    }

    return best;
}

bool ClaimYoggSaronBeaconExternal(PlayerbotAI* botAI, Player* victim)
{
    if (!victim)
        return false;

    YoggSaronEncounterState& state = YoggSaronStateFor(botAI->GetBot());
    uint32 const now = getMSTime();

    auto const claim = state.beaconExternalClaims.find(victim->GetGUID());
    if (claim != state.beaconExternalClaims.end() &&
        getMSTimeDiff(claim->second, now) < ULDUAR_YOGG_SARON_BEACON_EXTERNAL_CLAIM_MS)
        return false;

    state.beaconExternalClaims[victim->GetGUID()] = now;

    return true;
}

namespace
{
// Ends a swap once its window runs out, the holder dies, or the taker drops under the swap line.
bool YoggSaronTankSwapLive(Player* bot, YoggSaronEncounterState& state)
{
    YoggSaronEncounterState::TankSwap& swap = state.tankSwap;
    if (!swap.startMs)
        return false;

    Player* from = ObjectAccessor::GetPlayer(*bot, swap.from);
    Player* to = ObjectAccessor::GetPlayer(*bot, swap.to);
    if (getMSTimeDiff(swap.startMs, getMSTime()) >= ULDUAR_YOGG_SARON_TANK_SWAP_MS || !from || !from->IsAlive() ||
        !to || !to->IsAlive() || to->GetHealthPct() < ULDUAR_YOGG_SARON_TANK_SWAP_PCT)
    {
        swap = YoggSaronEncounterState::TankSwap();
        return false;
    }

    return true;
}
}  // namespace

Player* YoggSaronTankSwapTake(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    if (YoggSaronTankSwapLive(bot, state))
        return state.tankSwap.to == bot->GetGUID() ? ObjectAccessor::GetPlayer(*bot, state.tankSwap.from) : nullptr;

    Group* group = bot->GetGroup();
    if (!group || bot->GetHealthPct() < ULDUAR_YOGG_SARON_TANK_SWAP_TAKER_PCT || YoggSaronBeaconDragging(bot) ||
        YoggSaronBeaconReturning(bot))
        return nullptr;

    uint32 const mine = YoggSaronGuardiansOn(bot);
    Player* holder = nullptr;
    uint32 holderCount = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
            !PlayerbotAI::IsTank(member) || member->GetHealthPct() >= ULDUAR_YOGG_SARON_TANK_SWAP_PCT)
            continue;

        uint32 const count = YoggSaronGuardiansOn(member);
        if (count >= ULDUAR_YOGG_SARON_TANK_SWAP_MIN_GUARDIANS && count > mine && count > holderCount)
        {
            holder = member;
            holderCount = count;
        }
    }

    if (!holder)
        return nullptr;

    state.tankSwap = {holder->GetGUID(), bot->GetGUID(), getMSTime()};

    return holder;
}

bool YoggSaronTankShedding(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);

    return state.tankSwap.from == bot->GetGUID() && YoggSaronTankSwapLive(bot, state);
}

bool YoggSaronTankLowHold(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || bot->GetHealthPct() >= ULDUAR_YOGG_SARON_TANK_SWAP_PCT)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->GetMap() == bot->GetMap() &&
            PlayerbotAI::IsTank(member) && member->GetHealthPct() >= ULDUAR_YOGG_SARON_TANK_SWAP_TAKER_PCT)
            return true;
    }

    return false;
}

bool YoggSaronTankWrathHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    return bot->getClass() == CLASS_PALADIN &&
           ReadyTankDefensive(bot, true, ULDUAR_YOGG_SARON_TANK_WRATH_LOCK_MS) && YoggSaronInPhase3(botAI);
}

bool YoggSaronBeforePhase3(PlayerbotAI* botAI)
{
    uint32 const phase = YoggSaronPhase(botAI);

    return phase == 1 || phase == 2;
}

Player* YoggSaronTankLayOnHandsTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_PALADIN ||
        bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return nullptr;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();

    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMap() != bot->GetMap() || !PlayerbotAI::IsTank(member) ||
            member->GetHealthPct() >= ULDUAR_YOGG_SARON_TANK_LOH_PCT ||
            !bot->IsWithinDistInMap(member, ULDUAR_YOGG_SARON_TANK_LOH_RANGE))
            continue;

        auto const claim = state.tankLohClaims.find(member->GetGUID());
        if (claim != state.tankLohClaims.end() && claim->second.first != bot->GetGUID() &&
            getMSTimeDiff(claim->second.second, now) < ULDUAR_YOGG_SARON_TANK_LOH_CLAIM_MS)
            continue;

        // Forbearance from a tank's own Divine Protection bars it, and CanCastSpell reads that.
        if ((!best || member->GetHealthPct() < best->GetHealthPct()) && botAI->CanCastSpell("lay on hands", member))
            best = member;
    }

    return best;
}

bool ClaimYoggSaronTankLayOnHands(PlayerbotAI* botAI, Player* tank)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();

    auto const claim = state.tankLohClaims.find(tank->GetGUID());
    if (claim != state.tankLohClaims.end() && claim->second.first != bot->GetGUID() &&
        getMSTimeDiff(claim->second.second, now) < ULDUAR_YOGG_SARON_TANK_LOH_CLAIM_MS)
        return false;

    state.tankLohClaims[tank->GetGUID()] = {bot->GetGUID(), now};

    return true;
}

bool YoggSaronHealerTargetless(PlayerbotAI* botAI)
{
    return botAI->GetBot()->IsInCombat() &&
           !botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() && YoggSaronEncounterActive(botAI);
}

bool YoggSaronBeaconImmunityHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_MAGE)
        return false;

    return YoggSaronHealedGuardianOn(botAI, bot) != nullptr;
}

bool YoggSaronBeaconHunterHold(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    return bot->getClass() == CLASS_HUNTER && YoggSaronBeaconDragging(bot);
}

namespace
{
// Until the next roar, 0 once it's due or late. Max with no phase 3 clock yet.
uint32 YoggSaronMsToRoar(YoggSaronEncounterState const& state, uint32 now)
{
    if (!state.phase3StartMs)
        return std::numeric_limits<uint32>::max();

    uint32 const next = state.lastRoarMs ? state.lastRoarMs + ULDUAR_YOGG_SARON_ROAR_PERIOD_MS
                                         : state.phase3StartMs + ULDUAR_YOGG_SARON_ROAR_FIRST_MS;

    return next > now ? next - now : 0;
}
}  // namespace

bool YoggSaronRoarExpected(PlayerbotAI* botAI) { return YoggSaronStateFor(botAI->GetBot()).roarExpected; }

bool YoggSaronRoarCasting(PlayerbotAI* botAI)
{
    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    if (!yogg || !yogg->IsAlive() || !yogg->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* cast = yogg->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    return cast && cast->m_spellInfo->Id == SPELL_DEAFENING_ROAR;
}

Player* YoggSaronRoarPaladin(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    Group* group = bot->GetGroup();
    if (!state.roarExpected || !group)
        return nullptr;

    uint32 const msToRoar = YoggSaronRoarCasting(botAI) ? 0 : YoggSaronMsToRoar(state, getMSTime());

    struct Candidate
    {
        Player* player;
        bool ready;
        bool dragging;
        bool aura;
    };
    std::vector<Candidate> candidates;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsYoggSaronBot(member) || member->getClass() != CLASS_PALADIN || !member->IsAlive() ||
            member->GetMap() != bot->GetMap() ||
            member->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT ||
            !member->HasSpell(SPELL_AURA_MASTERY) || !member->HasSpell(SPELL_CONCENTRATION_AURA))
            continue;

        candidates.push_back({member, member->GetSpellCooldownDelay(SPELL_AURA_MASTERY) <= msToRoar,
                              YoggSaronBeaconDragging(member) || YoggSaronBeaconReturning(member) != nullptr,
                              member->HasAura(SPELL_CONCENTRATION_AURA, member->GetGUID())});
    }

    auto const best = std::min_element(candidates.begin(), candidates.end(),
                                       [](Candidate const& left, Candidate const& right)
                                       {
                                           return std::make_tuple(!left.ready, left.dragging, !left.aura,
                                                                  left.player->GetGUID()) <
                                                  std::make_tuple(!right.ready, right.dragging, !right.aura,
                                                                  right.player->GetGUID());
                                       });

    return best == candidates.end() ? nullptr : best->player;
}

bool YoggSaronRoarAuraHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_PALADIN)
        return false;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    if (!state.roarExpected ||
        (YoggSaronMsToRoar(state, getMSTime()) > ULDUAR_YOGG_SARON_ROAR_PRESWITCH_MS && !YoggSaronRoarCasting(botAI)))
        return false;

    return YoggSaronRoarPaladin(botAI) == bot;
}

bool ClaimYoggSaronRoar(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    if (!state.lastRoarMs || !YoggSaronRoarCasting(botAI))
        return false;

    if (state.roarClaimKey == state.lastRoarMs)
        return state.roarClaimOwner == bot->GetGUID();

    if (YoggSaronRoarPaladin(botAI) != bot &&
        getMSTimeDiff(state.lastRoarMs, getMSTime()) < ULDUAR_YOGG_SARON_ROAR_HANDOFF_MS)
        return false;

    state.roarClaimKey = state.lastRoarMs;
    state.roarClaimOwner = bot->GetGUID();

    return true;
}

bool YoggSaronGazeActive(PlayerbotAI* botAI)
{
    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);

    return yogg && yogg->IsAlive() && yogg->HasAura(SPELL_LUNATIC_GAZE_YS);
}

bool YoggSaronOutsideGaze(Player* bot, Unit* yogg, Unit* target)
{
    if (!bot || !yogg || !target)
        return false;

    float const pi = static_cast<float>(M_PI);
    float const offset = Position::NormalizeOrientation(bot->GetAngle(target) - bot->GetAngle(yogg));
    float const separation = offset > pi ? 2.0f * pi - offset : offset;

    return separation >= ULDUAR_YOGG_SARON_GAZE_SAFE_SEPARATION;
}

bool YoggSaronPhase3LustReady(PlayerbotAI* botAI)
{
    if (!YoggSaronInPhase3(botAI))
        return false;

    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();
    if (!state.phase3StartMs)
        state.phase3StartMs = now;

    if (getMSTimeDiff(state.phase3StartMs, now) >= ULDUAR_YOGG_SARON_LUST_FALLBACK_MS)
        return true;

    Group* group = bot->GetGroup();
    if (!group)
        return true;

    // Placed is at the station, or standing in reach of a live target: ranged the station lets go of for a
    // leftover tentacle are where their fight is. Anyone still below is out of Heroism's 100 yd.
    uint32 living = 0;
    uint32 placed = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMap() != bot->GetMap())
            continue;

        ++living;
        if (member->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
            continue;

        if (member->GetExactDist2d(YoggSaronPhaseThreeSpot(member)) <= ULDUAR_YOGG_SARON_PHASE_3_STATION_RADIUS)
        {
            ++placed;
            continue;
        }

        Unit* target = member->isMoving() ? nullptr : ObjectAccessor::GetUnit(*member, member->GetTarget());
        if (!target || !target->IsAlive())
            continue;

        bool const inReach = PlayerbotAI::IsRanged(member)
                                 ? member->GetDistance(target) <= sPlayerbotAIConfig.SpellDistance
                                 : member->IsWithinMeleeRange(target);
        if (inReach)
            ++placed;
    }

    return living && placed >= ULDUAR_YOGG_SARON_LUST_POSITION_SHARE * living;
}

bool YoggSaronGuardianThreatAllows(Player* bot, Unit* guardian, bool holding)
{
    // Heal threat reaches every Guardian in combat, so a fresh one leaves its threatless first victim for a
    // healer about a second after spawning. A non-tank victim means nobody hits it until a taunt lands.
    Player* tank = guardian->GetVictim() ? guardian->GetVictim()->ToPlayer() : nullptr;
    if (!tank || !PlayerbotAI::IsTank(tank))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return true;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && PlayerbotAI::IsTank(member))
                return false;
        }

        return true;
    }

    float const share =
        holding ? ULDUAR_YOGG_SARON_GUARDIAN_THREAT_KEEP_SHARE : ULDUAR_YOGG_SARON_GUARDIAN_THREAT_START_SHARE;

    return guardian->GetThreatMgr().GetThreat(bot) < share * guardian->GetThreatMgr().GetThreat(tank);
}

float YoggSaronTankThreatLead(Player* tank, Unit* guardian)
{
    float highest = 0.0f;
    for (ThreatReference const* ref : guardian->GetThreatMgr().GetUnsortedThreatList())
    {
        Player* player = ref->GetVictim() ? ref->GetVictim()->ToPlayer() : nullptr;
        if (player && PlayerbotAI::IsTank(player))
            continue;

        highest = std::max(highest, ref->GetThreat());
    }

    float const own = guardian->GetThreatMgr().GetThreat(tank);
    if (highest <= 0.0f)
        return own > 0.0f ? std::numeric_limits<float>::max() : 0.0f;

    return own / highest;
}

bool YoggSaronConstrictorHolding(Unit* unit)
{
    // The grab is a Lunge onto the tentacle's seat, so the passenger is the whole signal.
    if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_CONSTRICTOR_TENTACLE)
        return false;

    Vehicle* seat = unit->GetVehicleKit();
    return seat && seat->IsVehicleInUse();
}

namespace
{
struct YoggSaronWalkLatch
{
    Position destination;
    float bestDistance = 0.0f;
    uint32 lastProgressMs = 0;
    uint32 lastAskedMs = 0;
    uint32 gaveUpMs = 0;
};

// Per bot, per node, never pruned while the instance lives. Only detour's waypoint moves with the bot
// (a new latch per yard), so each node gets its own list.
using YoggSaronWalkLatches =
    std::unordered_map<ObjectGuid, std::unordered_map<std::string, std::vector<YoggSaronWalkLatch>>>;
RaidInstanceState<YoggSaronWalkLatches> yoggSaronWalkLatches;
}  // namespace

bool YoggSaronWalkMakingProgress(PlayerbotAI* botAI, char const* node, Position const& destination)
{
    Player* bot = botAI->GetBot();
    uint32 const now = getMSTime();
    float const distance = bot->GetExactDist(destination);

    bool giveUp = false;
    std::vector<YoggSaronWalkLatch>& latches = yoggSaronWalkLatches.For(bot->GetInstanceId())[bot->GetGUID()][node];

    YoggSaronWalkLatch* latch = nullptr;
    for (YoggSaronWalkLatch& candidate : latches)
    {
        if (candidate.destination.GetExactDist(destination) < 1.0f)
        {
            latch = &candidate;
            break;
        }
    }

    if (!latch)
    {
        latches.push_back(YoggSaronWalkLatch{destination, distance, now, now});
        latch = &latches.back();
    }

    // A gap in the asking starts a new attempt, else a give-up outlives its walk and every later walk
    // reads as 'no closer than last time' forever.
    bool const fresh = getMSTimeDiff(latch->lastAskedMs, now) >= ULDUAR_YOGG_SARON_WALK_GIVE_UP_MS;
    bool const arrived = distance <= ULDUAR_YOGG_SARON_WALK_ARRIVED_RADIUS;
    latch->lastAskedMs = now;

    // A give-up has to run out: the gap above rarely clears one, the walks that matter ask every tick. An
    // uncleared illusion room has no way out, so retrying a maybe-unreachable destination beats stopping for good.
    if (latch->gaveUpMs && getMSTimeDiff(latch->gaveUpMs, now) >= ULDUAR_YOGG_SARON_WALK_RETRY_MS)
    {
        latch->gaveUpMs = 0;
        latch->bestDistance = distance;
        latch->lastProgressMs = now;
    }

    if (latch->gaveUpMs)
    {
        giveUp = true;
    }
    else if (fresh || arrived || distance < latch->bestDistance)
    {
        latch->bestDistance = distance;
        latch->lastProgressMs = now;
    }
    else if (getMSTimeDiff(latch->lastProgressMs, now) >= ULDUAR_YOGG_SARON_WALK_GIVE_UP_MS)
    {
        latch->gaveUpMs = now;
        giveUp = true;
    }

    return !giveUp;
}

void YoggSaronReleaseStalledWalk(PlayerbotAI* botAI, char const* node, Position const& destination)
{
    Player* bot = botAI->GetBot();

    LastMovement& last = botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get();
    if (bot->isMoving() || !last.msTime ||
        getMSTimeDiff(last.msTime, getMSTime()) <= ULDUAR_YOGG_SARON_WALK_STALL_MS)
        return;

    last.clear();
}

bool YoggSaronMarkCaster(Unit* caster)
{
    return caster->GetEntry() == NPC_MARKED_IMMORTAL_GUARDIAN ||
           yoggSaronStates.For(caster->GetInstanceId()).beaconDrags.count(caster->GetGUID());
}

bool YoggSaronMarkKicker(Player* bot)
{
    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);

    return state.markWindow &&
           std::find(state.markKickers.begin(), state.markKickers.end(), bot->GetGUID()) != state.markKickers.end();
}

namespace
{
// Every caster around the bot, marks first, whoever is asking.
std::vector<Unit*> YoggSaronInterruptCastersAround(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, ULDUAR_YOGG_SARON_INTERRUPT_SEARCH_RADIUS);

    std::vector<Unit*> casters;
    for (Creature* guardian : guardians)
    {
        if (!guardian->IsAlive())
            continue;

        if (guardian->FindCurrentSpellBySpellId(SPELL_DARK_VOLLEY) ||
            guardian->FindCurrentSpellBySpellId(SPELL_DARK_VOLLEY_H))
            casters.push_back(guardian);
    }

    // Drain Life heals an Immortal Guardian 166,650 over 5 s, 31.6% of its health, three Empowered stacks back.
    // The class interrupts only kick their own target.
    if (!YoggSaronInPhase3(botAI))
        return casters;

    // Kicks run on cooldown, each saves only the ticks its channel has left. So most time left first, and none on a
    // channel down to its last tick while a fresh one is about a second away.
    std::vector<std::pair<int32, Unit*>> drains;
    for (uint32 const entry : {NPC_IMMORTAL_GUARDIAN, NPC_MARKED_IMMORTAL_GUARDIAN})
    {
        std::list<Creature*> immortals;
        bot->GetCreatureListWithEntryInGrid(immortals, entry, ULDUAR_YOGG_SARON_INTERRUPT_SEARCH_RADIUS);
        for (Creature* guardian : immortals)
        {
            Spell* channel = guardian->IsAlive() ? YoggSaronDrainLifeKickable(guardian) : nullptr;
            if (channel && channel->GetCastTimeRemaining() >= ULDUAR_YOGG_SARON_KICK_MIN_LEFT_MS)
                drains.emplace_back(channel->GetCastTimeRemaining(), guardian);
        }
    }

    std::stable_sort(drains.begin(), drains.end(), [](auto const& a, auto const& b) { return a.first > b.first; });
    for (auto const& [left, guardian] : drains)
        casters.push_back(guardian);

    // A channelling Guardian doesn't move, so a mark's Drain Life during the drag keeps it in the pile
    // under the dragger's taunt, where its heal lands on the whole pile.
    std::stable_partition(casters.begin(), casters.end(), YoggSaronMarkCaster);

    return casters;
}
}  // namespace

std::vector<Unit*> GetYoggSaronInterruptCasters(PlayerbotAI* botAI)
{
    std::vector<Unit*> casters = YoggSaronInterruptCastersAround(botAI);

    if (YoggSaronMarkKicker(botAI->GetBot()))
        casters.erase(std::find_if_not(casters.begin(), casters.end(), YoggSaronMarkCaster), casters.end());

    return casters;
}

std::vector<char const*> YoggSaronInterruptSpells(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            return {"mind freeze", "strangulate"};
        case CLASS_HUNTER:
            return {"silencing shot"};
        case CLASS_MAGE:
            return {"counterspell"};
        case CLASS_ROGUE:
            return {"kick"};
        case CLASS_SHAMAN:
            return {"wind shear"};
        case CLASS_WARRIOR:
            return {"pummel", "shield bash"};
        case CLASS_WARLOCK:
            return {"spell lock"};
        default:
            if (bot->getRace() == RACE_BLOODELF)
                return {"arcane torrent"};

            return {};
    }
}

bool YoggSaronCanInterrupt(Player* bot) { return !YoggSaronInterruptSpells(bot).empty(); }

namespace
{
// Off cooldown, affordable, in stance and in range, without CanCastSpell, which can't answer for a bot mid
// cast. Spell Lock is the felhunter's but the bot casts it, so its cooldown and range are the bot's.
bool YoggSaronKickInReach(PlayerbotAI* botAI, char const* spell, Unit* caster)
{
    Player* bot = botAI->GetBot();
    uint32 const id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
    SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
    if (!info || bot->HasSpellCooldown(id) || info->CheckShapeshift(bot->GetShapeshiftForm()) != SPELL_CAST_OK ||
        !bot->HasItemFitToSpellRequirements(info))
        return false;

    if (info->PowerType < MAX_POWERS &&
        info->CalcPowerCost(bot, info->GetSchoolMask()) > static_cast<int32>(bot->GetPower(Powers(info->PowerType))))
        return false;

    if (info->RangeEntry && (info->RangeEntry->Flags & SPELL_RANGE_MELEE))
        return bot->IsWithinMeleeRange(caster);

    return bot->IsWithinCombatRange(caster, bot->GetSpellMaxRangeForTarget(caster, info));
}
}  // namespace

bool YoggSaronInterruptReady(PlayerbotAI* botAI, char const* spell, Unit* caster)
{
    if (!YoggSaronKickInReach(botAI, spell, caster))
        return false;

    return botAI->CanCastSpell(botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get(), caster);
}

std::vector<char const*> YoggSaronInterruptSpellsFor(Player* bot, Unit* caster)
{
    std::vector<char const*> spells = YoggSaronInterruptSpells(bot);

    uint32 const entry = caster ? caster->GetEntry() : 0;
    if (entry != NPC_IMMORTAL_GUARDIAN && entry != NPC_MARKED_IMMORTAL_GUARDIAN)
        return spells;

    spells.erase(std::remove_if(spells.begin(), spells.end(),
                                [](char const* spell)
                                {
                                    std::string const name(spell);
                                    return name == "strangulate" || name == "silencing shot" ||
                                           name == "arcane torrent";
                                }),
                 spells.end());

    return spells;
}

bool YoggSaronInterruptClaimedByOther(Player* bot, Unit* caster)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    auto const it = state.interruptClaims.find(caster->GetGUID());
    if (it == state.interruptClaims.end())
        return false;

    if (it->second.first == bot->GetGUID() ||
        getMSTimeDiff(it->second.second, getMSTime()) >= ULDUAR_YOGG_SARON_INTERRUPT_CLAIM_MS)
        return false;

    return true;
}

void ClaimYoggSaronInterrupt(Player* bot, Unit* caster)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();

    // Lapsed claims go here rather than on a timer: casters come and go all phase.
    for (auto it = state.interruptClaims.begin(); it != state.interruptClaims.end();)
        it = getMSTimeDiff(it->second.second, now) >= ULDUAR_YOGG_SARON_INTERRUPT_CLAIM_MS
                 ? state.interruptClaims.erase(it)
                 : std::next(it);

    state.interruptClaims[caster->GetGUID()] = {bot->GetGUID(), now};
}

std::vector<Unit*> GetYoggSaronChannellingCrushers(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    // Grid radius counts both object sizes, so 25 already covers Judgement's ~19.5 yd. CanCastSpell
    // makes the real range call.
    std::list<Creature*> crushers;
    bot->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE, ULDUAR_YOGG_SARON_CRUSH_RANGE);

    std::vector<Unit*> channelling;
    for (Creature* crusher : crushers)
    {
        if (!crusher->IsAlive())
            continue;

        Spell const* channel = crusher->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (channel && channel->GetSpellInfo()->Id == SPELL_DIMINISH_POWER &&
            channel->getState() == SPELL_STATE_CASTING)
            channelling.push_back(crusher);
    }

    return channelling;
}

std::vector<char const*> YoggSaronJudgementSpells(Player* bot)
{
    if (bot->getClass() != CLASS_PALADIN)
        return {};

    return {"judgement of wisdom", "judgement of light", "judgement of justice"};
}

namespace
{
// Living Guardians at or under `gate`, swept once around `from`. Sweeping around every member instead
// would cost each paladin 25 grid searches a tick.
std::vector<Creature*> YoggSaronLowGuardians(Unit* from, float gate, float radius)
{
    std::list<Creature*> guardians;
    from->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, radius);

    std::vector<Creature*> low;
    for (Creature* guardian : guardians)
    {
        if (guardian->IsAlive() && guardian->GetHealthPct() <= gate)
            low.push_back(guardian);
    }

    return low;
}

// What one nova is worth against this target. The unresisted roll is only for a bot with no shadow resistance;
// Prayer, paladin aura, Aura Mastery and gear share one exclusive aura, so only the highest counts.
uint32 YoggSaronNovaDamageEach(Unit* target)
{
    return target->GetResistance(SPELL_SCHOOL_SHADOW) >= ULDUAR_YOGG_SARON_NOVA_RESISTED_AT
               ? ULDUAR_YOGG_SARON_NOVA_RESISTED_DAMAGE
               : ULDUAR_YOGG_SARON_NOVA_EXPECTED_DAMAGE;
}

// Worth a cooldown: two Guardians low (Fervor doubles the estimate), the second nova of a pair (a Guardian
// death in the last 6 s), or Sara's Blessing (shadow tail lands anyway). Per-nova estimate, never a constant.
bool YoggSaronNovaChained(Player* bot, Unit* member, uint32 expected)
{
    if (!expected)
        return false;

    return expected >= ULDUAR_YOGG_SARON_NOVA_MAX_CHAIN * YoggSaronNovaDamageEach(member) ||
           member->HasAura(SPELL_SARAS_BLESSING) ||
           YoggSaronGuardianDiedWithin(bot, ULDUAR_YOGG_SARON_P1_NOVA_GAP_MS);
}

uint32 YoggSaronNovaDamageFrom(std::vector<Creature*> const& low, Unit* target)
{
    uint32 count = 0;
    for (Creature* guardian : low)
    {
        if (target->GetExactDist2d(guardian) <= ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS)
            ++count;
    }

    if (!count)
        return 0;

    count = std::min<uint32>(count, ULDUAR_YOGG_SARON_NOVA_MAX_CHAIN);

    uint32 const expected = count * YoggSaronNovaDamageEach(target);

    // Fervor is +100% damage taken and DispelType 0, so it cannot be lifted. A holder caught in the
    // stack is the one bot a single nova kills from full health.
    return target->HasAura(SPELL_SARAS_FERVOR) ? expected * 2 : expected;
}

bool YoggSaronNovaLeavesNothing(uint32 expected, Unit* target)
{
    return expected && target->GetHealth() <= expected;
}

// Read off the list already swept, so asking this for all 25 raiders costs no extra grid search.
uint32 YoggSaronNovaNeedFrom(std::vector<Creature*> const& low, Unit* target)
{
    for (Creature* guardian : low)
    {
        if (guardian->GetHealthPct() <= ULDUAR_YOGG_SARON_NOVA_NOW_HEALTH_PCT &&
            target->GetExactDist2d(guardian) <= ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS)
            return ULDUAR_YOGG_SARON_NOVA_NOW_WINDOW_MS;
    }

    return ULDUAR_YOGG_SARON_NOVA_SOON_WINDOW_MS;
}

// What the nova still does to this raider after whatever it can put up by itself. A human has no bot AI
// and therefore nothing it will cast on cue, which is the honest answer for one.
uint32 YoggSaronNovaAfterOwnButton(std::vector<Creature*> const& low, Player* member, uint32 expected)
{
    PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
    if (!memberAI)
        return expected;

    uint8 const cut = BestMagicDefensiveCut(memberAI, member, YoggSaronNovaNeedFrom(low, member));

    return expected - expected * cut / 100;
}

// Ranged and healers hold the 21.5 yd station and are outside the blast by design, so a cooldown spent
// on one is a cooldown the melee pile does not get.
bool YoggSaronNovaCandidate(Player* member)
{
    return member && member->IsAlive() && !PlayerbotAI::IsRanged(member) && !PlayerbotAI::IsHeal(member);
}

// Bots only, since Aura Mastery doubles the caster's own aura and nothing here can press a human's.
// Lowest GUID, so every bot picks the same paladin without a latch to keep.
Player* ElectShadowAuraPaladin(Group* group)
{
    Player* best = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || !member->IsAlive() || member->getClass() != CLASS_PALADIN ||
            !member->HasActiveSpell(SPELL_AURA_MASTERY) ||
            !memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", "shadow resistance aura")->Get())
            continue;

        if (!best || member->GetGUID() < best->GetGUID())
            best = member;
    }

    return best;
}
}  // namespace

std::vector<Unit*> GetYoggSaronNovaThreats(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();

    // Fervor (+100% damage taken for 15 s) makes a nova lethal even from full health, so a holder always leaves.
    bool const fervor = bot->HasAura(SPELL_SARAS_FERVOR);
    // Melee leave only once phase 1 is over, whatever their pool: in it the walk costs Guardians. Never a tank,
    // walking out drags its Guardian along and its pool is the one that can eat the blast.
    bool const atRange = PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsHeal(bot);
    // Both reads sit ahead of the phase gate, which is a boss state read and a creature search.
    if (!fervor && !atRange && (PlayerbotAI::IsTank(bot) || YoggSaronInPhase1(botAI)))
        return {};

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, radius);

    std::vector<Creature*> low;
    for (Creature* guardian : guardians)
    {
        if (!guardian->IsAlive())
            continue;

        // Two gates, widest first. A Fervor holder has to be clear of the blast rather than mitigate it,
        // so it leaves on the longest warning anyone gets.
        float const gate = fervor ? ULDUAR_YOGG_SARON_FERVOR_NOVA_HEALTH_PCT
                                  : ULDUAR_YOGG_SARON_GUARDIAN_NOVA_HEALTH_PCT;
        if (guardian->GetHealthPct() > gate)
            continue;

        // At spell range the station already clears a blast on the stack by about a yard, so dodging one fights it.
        // Off the stack they can reach. Fervor reaches wherever the Guardian heads, and melee are in the blast anyway.
        if (atRange && !fervor && guardian->GetVictim() != bot && YoggSaronGuardianOnTheStack(guardian))
            continue;

        low.push_back(guardian);
    }

    return std::vector<Unit*>(low.begin(), low.end());
}

uint32 YoggSaronNovaDamageOn(Unit* target)
{
    if (!target || !target->IsAlive())
        return 0;

    std::vector<Creature*> const low = YoggSaronLowGuardians(target, ULDUAR_YOGG_SARON_NOVA_SOON_HEALTH_PCT,
                                                             ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS);

    return YoggSaronNovaDamageFrom(low, target);
}

bool YoggSaronNovaWouldHurt(Unit* target)
{
    return YoggSaronNovaLeavesNothing(YoggSaronNovaDamageOn(target), target);
}

uint32 YoggSaronNovaNeedMs(Unit* target, bool now)
{
    if (!target || !target->IsAlive())
        return 0;

    float const gate = now ? ULDUAR_YOGG_SARON_NOVA_NOW_HEALTH_PCT
                           : ULDUAR_YOGG_SARON_NOVA_SOON_HEALTH_PCT;
    std::vector<Creature*> const low =
        YoggSaronLowGuardians(target, gate, ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS);
    if (!YoggSaronNovaLeavesNothing(YoggSaronNovaDamageFrom(low, target), target))
        return 0;

    // Under the tighter gate a nova lands within 3.7 s, which a 5 s button covers; just past 20% it can take
    // 11.5 s, so only a 12 s button is worth starting.
    return YoggSaronNovaNeedFrom(low, target);
}

bool YoggSaronNovaNearby(PlayerbotAI* botAI)
{
    return !YoggSaronLowGuardians(botAI->GetBot(), 100.0f,
                                  ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS).empty();
}

bool YoggSaronNovaHold(PlayerbotAI* botAI)
{
    // Phase 1, and after it while any Guardian in the room lives. Not 17 yd: that lets go while Guardians left from
    // phase 1 are still alive.
    return YoggSaronInPhase1(botAI) ||
           !YoggSaronLowGuardians(botAI->GetBot(), 100.0f, ULDUAR_YOGG_SARON_P1_ROOM_RADIUS).empty();
}

uint32 YoggSaronNovaHurtCount(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    std::vector<Creature*> const low = YoggSaronLowGuardians(bot, ULDUAR_YOGG_SARON_NOVA_SOON_HEALTH_PCT,
                                                             ULDUAR_YOGG_SARON_P1_ROOM_RADIUS);
    if (low.empty())
        return 0;

    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!YoggSaronNovaCandidate(member))
            continue;

        // A chained nova only: every pool in the raid is under one nova's unresisted figure, so a single low
        // Guardian anywhere would count the whole melee group.
        uint32 const expected = YoggSaronNovaDamageFrom(low, member);
        if (YoggSaronNovaChained(bot, member, expected) && YoggSaronNovaLeavesNothing(expected, member))
            ++count;
    }

    return count;
}

bool IsYoggSaronShadowAuraPaladin(Player* bot)
{
    // The room check is distance only, so without the roar test this would still lock a paladin to Shadow
    // Resistance Aura in phase 3, when Aura Mastery is wanted with Concentration Aura instead.
    if (!bot || bot->getClass() != CLASS_PALADIN || !YoggSaronInPhase1Room(bot) ||
        YoggSaronStateFor(bot).roarExpected)
        return false;

    Group* group = bot->GetGroup();

    return group && ElectShadowAuraPaladin(group) == bot;
}

bool YoggSaronShadowAuraHeld(PlayerbotAI* botAI)
{
    return botAI && IsYoggSaronShadowAuraPaladin(botAI->GetBot());
}

bool YoggSaronShadowAuraOtherHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_PALADIN || !YoggSaronInPhase1Room(bot) ||
        YoggSaronStateFor(bot).roarExpected)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Only while somebody is elected, so a raid with no Aura Mastery anywhere keeps the generic nodes.
    Player* const elected = ElectShadowAuraPaladin(group);

    return elected && elected != bot;
}

bool YoggSaronBattleRezHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !YoggSaronInPhase1Room(bot))
        return false;

    // Released for the handover, which is the first stretch of the fight with no nova in it.
    return !YoggSaronHandoverState(botAI).clearing && YoggSaronInPhase1(botAI);
}

char const* YoggSaronNovaRaidCover(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->getClass() != CLASS_PALADIN)
        return nullptr;

    // Aura Mastery first: free for the caster, where Divine Sacrifice redirects 30% of the raid's damage onto it.
    // Only with its own aura up, the one it doubles: 130 resistance becoming 260 for 6 s beats the 130 Prayer gives.
    if (IsYoggSaronShadowAuraPaladin(bot) && botAI->HasAura("shadow resistance aura", bot, false, true) &&
        botAI->CanCastSpell("aura mastery", bot))
        return "aura mastery";

    return botAI->CanCastSpell("divine sacrifice", bot) ? "divine sacrifice" : nullptr;
}

Player* YoggSaronNovaVictim(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    std::vector<Creature*> const low = YoggSaronLowGuardians(bot, ULDUAR_YOGG_SARON_NOVA_NOW_HEALTH_PCT,
                                                             ULDUAR_YOGG_SARON_P1_ROOM_RADIUS);
    if (low.empty())
        return nullptr;

    Player* victim = nullptr;
    int64 worst = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!YoggSaronNovaCandidate(member) || member == bot)
            continue;

        if (bot->GetExactDist2d(member) > sPlayerbotAIConfig.SpellDistance)
            continue;

        // Chained only: one nova against these pools reads lethal on any bot under about 86% of its bar.
        // Same test as the raid cover, so the two can't disagree about which nova is worth a cooldown.
        uint32 const expected = YoggSaronNovaDamageFrom(low, member);
        if (!YoggSaronNovaChained(bot, member, expected) || !YoggSaronNovaLeavesNothing(expected, member))
            continue;

        // Whoever their own button still saves is not worth a three-minute external: a rogue who dodges
        // the blast outright with a 90 s Cloak of Shadows has to come behind a warrior who has nothing.
        uint32 const after = YoggSaronNovaAfterOwnButton(low, member, expected);
        if (member->GetHealth() > after)
            continue;

        // How far past dead the nova leaves them even so. A Fervor holder is furthest out because its
        // expected hit doubles, and among the rest the smallest health pool loses by the most.
        int64 const margin = int64(member->GetHealth()) - int64(after);
        if (!victim || margin < worst)
        {
            victim = member;
            worst = margin;
        }
    }

    return victim;
}

namespace
{

// The Guardian whose nova is coming: the lowest of the ones already past the wider gate. Every paladin
// in the pile sweeps the same 17 yd, so they agree on it, which is what makes it usable as a claim key.
ObjectGuid NovaGuardian(Player* bot)
{
    std::vector<Creature*> const low = YoggSaronLowGuardians(bot, ULDUAR_YOGG_SARON_NOVA_SOON_HEALTH_PCT,
                                                             ULDUAR_YOGG_SARON_SHADOW_NOVA_TRIGGER_RADIUS);
    Creature* next = nullptr;
    for (Creature* guardian : low)
    {
        if (!next || guardian->GetHealth() < next->GetHealth())
            next = guardian;
    }

    return next ? next->GetGUID() : ObjectGuid::Empty;
}

}  // namespace

bool ClaimYoggSaronNovaExternal(PlayerbotAI* botAI)
{
    ObjectGuid const guardian = NovaGuardian(botAI->GetBot());
    if (!guardian)
        return false;

    YoggSaronEncounterState& state = YoggSaronStateFor(botAI->GetBot());

    uint32 const now = getMSTime();
    uint32& claimed = state.novaClaims[guardian];
    if (claimed && getMSTimeDiff(claimed, now) < ULDUAR_YOGG_SARON_NOVA_WINDOW_MS)
        return false;

    claimed = now;

    return true;
}

bool ClaimYoggSaronNovaWindow(Player* bot)
{
    ObjectGuid const guardian = NovaGuardian(bot);
    if (!guardian)
        return false;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);

    auto const held = state.novaCovers.find(guardian);
    if (held != state.novaCovers.end() &&
        getMSTimeDiff(held->second.second, getMSTime()) < ULDUAR_YOGG_SARON_NOVA_WINDOW_MS)
        return held->second.first == bot->GetGUID();

    state.novaCovers[guardian] = {bot->GetGUID(), getMSTime()};

    return true;
}

void AppendYoggSaronTargetExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    // Guardians only exist during the fight, so the sweep is for nothing outside it.
    if (!BossStateGateLive(botAI, ULD_BOSS_YOGGSARON))
        return;

    std::vector<Creature*> const low =
        YoggSaronLowGuardians(botAI->GetBot(), ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT,
                              ULDUAR_YOGG_SARON_P1_ROOM_RADIUS);

    for (Creature* guardian : low)
    {
        for (Creature* other : low)
        {
            if (other == guardian ||
                other->GetExactDist2d(guardian) > ULDUAR_YOGG_SARON_P1_NOVA_CHAIN_RADIUS)
                continue;

            // Shield whichever is further from death. Equal health happens, so the guid settles it instead of both
            // being excluded and the pair waiting on each other.
            if (guardian->GetHealth() > other->GetHealth() ||
                (guardian->GetHealth() == other->GetHealth() && other->GetGUID() < guardian->GetGUID()))
            {
                exclusions.insert(guardian->GetGUID());
                break;
            }
        }
    }
}

namespace
{
float YoggSaronDistanceFromMiddle(Unit* unit)
{
    return unit->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(),
                               ULDUAR_YOGG_SARON_MIDDLE.GetPositionY());
}
}  // namespace

bool YoggSaronGuardianCountsForSara(Unit* guardian)
{
    return guardian && YoggSaronDistanceFromMiddle(guardian) <= ULDUAR_YOGG_SARON_P1_SARA_NOVA_RADIUS;
}

bool YoggSaronGuardianOnTheStack(Unit* guardian)
{
    return guardian && YoggSaronDistanceFromMiddle(guardian) <= ULDUAR_YOGG_SARON_P1_LEASH;
}

bool YoggSaronPhase1GuardianPreferred(Unit* candidate, Unit* incumbent)
{
    if (!candidate)
        return false;

    if (!incumbent)
        return true;

    bool const candidateIn = YoggSaronGuardianOnTheStack(candidate);
    if (candidateIn != YoggSaronGuardianOnTheStack(incumbent))
        return candidateIn;

    return candidate->GetHealth() < incumbent->GetHealth();
}

Unit* YoggSaronPhase1TauntTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, sPlayerbotAIConfig.SightDistance);

    Unit* focus = nullptr;
    for (Creature* guardian : guardians)
    {
        if (!guardian->IsAlive())
            continue;

        // Already walking at a tank, so the taunt would buy nothing. Its own victim rather than the
        // threat table: a taunt is a forced victim, and that is the state this is checking for.
        Player* victim = botAI->GetPlayer(guardian->GetTarget());
        if (victim && PlayerbotAI::IsTank(victim))
            continue;

        if (guardian->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(),
                                    ULDUAR_YOGG_SARON_MIDDLE.GetPositionY()) <= ULDUAR_YOGG_SARON_P1_LEASH)
            continue;

        // Out of taunt range is not a candidate: the cast would fail, and a failed cast still leaves
        // the bot selected on what it aimed at, which is then what every reach walks toward.
        if (bot->GetDistance(guardian) > ULDUAR_YOGG_SARON_P1_TAUNT_RANGE)
            continue;

        if (YoggSaronPhase1GuardianPreferred(guardian, focus))
            focus = guardian;
    }

    return focus;
}

Unit* YoggSaronPhase1StackTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, ULDUAR_YOGG_SARON_P1_TAUNT_RANGE);

    Unit* lowest = nullptr;
    for (Creature* guardian : guardians)
    {
        // In reach only: anything further is the taunt's job, and walking at it is what the reach
        // guard stands the bot down from.
        if (!guardian->IsAlive() || !bot->IsWithinMeleeRange(guardian))
            continue;

        if (!lowest || guardian->GetHealth() < lowest->GetHealth())
            lowest = guardian;
    }

    return lowest;
}

bool YoggSaronBotTankAlive(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return PlayerbotAI::IsTank(botAI->GetBot());

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member))
            continue;

        if (member->GetSession() && member->GetSession()->IsHeadless())
            return true;
    }

    return false;
}

bool YoggSaronInPhase1Room(Player* bot)
{
    return bot->GetDistance2d(ULDUAR_YOGG_SARON_MIDDLE.GetPositionX(), ULDUAR_YOGG_SARON_MIDDLE.GetPositionY()) <=
           ULDUAR_YOGG_SARON_P1_ROOM_RADIUS;
}

void YoggSaronNoteGuardianDeath(Unit* guardian)
{
    if (!guardian)
        return;

    yoggSaronStates.For(guardian->GetInstanceId()).lastGuardianDeathMs = getMSTime();
}

bool YoggSaronGuardianDiedWithin(Player* bot, uint32 ms)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    return state.lastGuardianDeathMs && getMSTimeDiff(state.lastGuardianDeathMs, getMSTime()) < ms;
}

namespace
{
// Health first, so the lock is only taken for a Guardian close to dying.
bool YoggSaronGuardianWaitsOutNovaGap(Player* bot, Unit* guardian)
{
    return guardian->GetHealthPct() < ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT &&
           YoggSaronGuardianDiedWithin(bot, ULDUAR_YOGG_SARON_P1_NOVA_GAP_MS);
}

// A lower Guardian inside nova chain range dies first: finishing this one novas it for ~2.7% of its health.
// The lowest is never held. Health first, so the sweep only runs for a Guardian close to dying.
bool YoggSaronGuardianWaitsOnLowerNeighbour(Unit* guardian)
{
    if (guardian->GetHealthPct() >= ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT)
        return false;

    std::list<Creature*> guardians;
    guardian->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, ULDUAR_YOGG_SARON_P1_NOVA_CHAIN_RADIUS);
    for (Creature* other : guardians)
    {
        if (other != guardian && other->IsAlive() && other->GetHealth() < guardian->GetHealth() &&
            other->GetExactDist2d(guardian) <= ULDUAR_YOGG_SARON_P1_NOVA_CHAIN_RADIUS)
            return true;
    }

    return false;
}
}  // namespace

bool YoggSaronPhase1GuardianKillable(PlayerbotAI* botAI, Unit* guardian)
{
    if (!guardian || !guardian->IsAlive())
        return false;

    if (YoggSaronGuardianWaitsOutNovaGap(botAI->GetBot(), guardian) || YoggSaronGuardianWaitsOnLowerNeighbour(guardian))
        return false;

    // No floor in the handover: the tanks walk to the anchor and take every Guardian off the stack with them.
    // Group walk last, so it only runs for a Guardian that is actually parked.
    return YoggSaronGuardianOnTheStack(guardian) ||
           guardian->GetHealthPct() > ULDUAR_YOGG_SARON_P1_PARK_HEALTH_PCT ||
           YoggSaronHandoverState(botAI).active || !YoggSaronBotTankAlive(botAI);
}

Unit* YoggSaronPhase1Focus(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    ObjectGuid const heldGuid = state.phase1Focus;

    // Guid lookup rather than the sweep, so a bot at the edge of sight range still sees the raid's pick.
    Creature* held = heldGuid ? ObjectAccessor::GetCreature(*bot, heldGuid) : nullptr;
    bool const heldKillable = YoggSaronPhase1GuardianKillable(botAI, held);

    // Only bots in the room re-pick. From outside it the sweep can miss half the room and would hand
    // the raid a worse target than the one it has.
    if (!YoggSaronInPhase1Room(bot))
        return heldKillable ? held : nullptr;

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, sPlayerbotAIConfig.SightDistance);

    // The handover moved the pile to the anchor and no kill counts for Sara any more, so the stack test
    // means nothing there. Lowest first instead.
    bool const handover = YoggSaronHandoverState(botAI).active;

    Unit* best = nullptr;
    bool killableOnStack = false;
    for (Creature* guardian : guardians)
    {
        if (!YoggSaronPhase1GuardianKillable(botAI, guardian))
            continue;

        killableOnStack = killableOnStack || YoggSaronGuardianOnTheStack(guardian);
        if (handover ? !best || guardian->GetHealth() < best->GetHealth()
                     : YoggSaronPhase1GuardianPreferred(guardian, best))
            best = guardian;
    }

    // Give up at 15 yd, pick up only inside 6.5: the gap stops a boundary Guardian flipping the raid's target.
    // Neither radius applies in the handover, where focus only moves for a low one freed from its gap.
    bool const lower = handover && heldKillable && best && best != held && YoggSaronLowGuardianTakesOver(best, held);
    bool const abandon =
        heldKillable && (handover ? lower : !YoggSaronGuardianCountsForSara(held) && killableOnStack);
    if (heldKillable && !abandon)
        return held;

    state.phase1Focus = best ? best->GetGUID() : ObjectGuid::Empty;
    return best;
}

bool YoggSaronPhase1AoeHold(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    ObjectGuid const focusGuid = YoggSaronStateFor(bot).phase1Focus;

    Creature* focus = focusGuid ? ObjectAccessor::GetCreature(*bot, focusGuid) : nullptr;
    if (focus && !focus->IsAlive())
        focus = nullptr;

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, sPlayerbotAIConfig.SightDistance);

    for (Creature* guardian : guardians)
    {
        if (guardian == focus || !guardian->IsAlive() ||
            guardian->GetHealthPct() >= ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT)
            continue;

        // The focus side covers a parked Guardian standing next to a focus burned out at the station.
        if (YoggSaronDistanceFromMiddle(guardian) <= ULDUAR_YOGG_SARON_P1_AOE_HOLD_RADIUS ||
            (focus && guardian->GetExactDist2d(focus) <= ULDUAR_YOGG_SARON_P1_AOE_HOLD_RADIUS))
            return true;
    }

    return false;
}

bool YoggSaronLeftoverGuardianKillable(PlayerbotAI* botAI, Unit* guardian)
{
    return guardian && guardian->IsAlive() && !YoggSaronGuardianWaitsOutNovaGap(botAI->GetBot(), guardian) &&
           !YoggSaronGuardianWaitsOnLowerNeighbour(guardian);
}

bool YoggSaronLowGuardianTakesOver(Unit* candidate, Unit* current)
{
    return candidate->GetHealthPct() < ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT &&
           candidate->GetHealth() < current->GetHealth();
}

bool YoggSaronLeftoverAoeHold(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (target && !target->IsAlive())
        target = nullptr;

    std::list<Creature*> guardians;
    bot->GetCreatureListWithEntryInGrid(guardians, NPC_GUARDIAN_OF_YS, sPlayerbotAIConfig.SightDistance);

    for (Creature* guardian : guardians)
    {
        if (guardian == target || !guardian->IsAlive() ||
            guardian->GetHealthPct() >= ULDUAR_YOGG_SARON_P1_NOVA_GAP_HEALTH_PCT)
            continue;

        if (guardian->GetExactDist2d(bot) <= ULDUAR_YOGG_SARON_P1_AOE_HOLD_RADIUS ||
            (target && guardian->GetExactDist2d(target) <= ULDUAR_YOGG_SARON_P1_AOE_HOLD_RADIUS))
            return true;
    }

    return false;
}

Position YoggSaronCloudLead(Creature* cloud, uint32 leadMs)
{
    // An escort-AI creature faces the leg it is walking, so its own orientation is the heading, with no
    // need to track the orbit or know which way round it was sent.
    float const travel = cloud->GetSpeed(MOVE_RUN) * leadMs / IN_MILLISECONDS;
    float const heading = cloud->GetOrientation();

    return Position(cloud->GetPositionX() + std::cos(heading) * travel,
                    cloud->GetPositionY() + std::sin(heading) * travel, cloud->GetPositionZ());
}

bool YoggSaronRouteClearOfClouds(Player* bot, std::vector<Position> const& clouds, float x, float y)
{
    float const originX = bot->GetPositionX();
    float const originY = bot->GetPositionY();
    float const legX = x - originX;
    float const legY = y - originY;
    float const legSquared = legX * legX + legY * legY;

    for (Position const& cloud : clouds)
    {
        float along = 0.0f;
        if (legSquared > 0.0f)
        {
            along = ((cloud.GetPositionX() - originX) * legX + (cloud.GetPositionY() - originY) * legY) / legSquared;
            along = std::max(0.0f, std::min(1.0f, along));
        }

        if (cloud.GetExactDist2d(originX + legX * along, originY + legY * along) <
            ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS)
            return false;
    }

    return true;
}

bool YoggSaronWalkCrossesCloud(Player* bot, Position const& to, Position const* exempt, float exemptRadius)
{
    float const leg = bot->GetExactDist2d(to.GetPositionX(), to.GetPositionY());

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, NPC_OMINOUS_CLOUD, leg + ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS);
    if (found.empty())
        return false;

    // Matched in time, step by step: at each step the bot is part way along the leg and the cloud is where it
    // will be. The whole leg against every cloud position refuses almost every route, an orbit crosses any radial path.
    float const speed = bot->GetSpeed(MOVE_RUN);
    uint32 const arrivalMs = speed > 0.0f ? static_cast<uint32>(leg / speed * IN_MILLISECONDS) : 0;

    float const originX = bot->GetPositionX();
    float const originY = bot->GetPositionY();
    float const legX = to.GetPositionX() - originX;
    float const legY = to.GetPositionY() - originY;

    for (Creature* cloud : found)
    {
        if (!cloud->IsAlive())
            continue;

        if (bot->GetExactDist2d(cloud) < ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS)
            return false;

        if (exempt && cloud->GetExactDist2d(exempt) <= exemptRadius)
            continue;

        for (uint32 step = 0; step <= ULDUAR_YOGG_SARON_WALK_CLOUD_STEPS; ++step)
        {
            float const share = static_cast<float>(step) / ULDUAR_YOGG_SARON_WALK_CLOUD_STEPS;
            Position const at = YoggSaronCloudLead(cloud, static_cast<uint32>(arrivalMs * share));
            if (at.GetExactDist2d(originX + legX * share, originY + legY * share) <
                ULDUAR_YOGG_SARON_CLOUD_AVOID_RADIUS)
                return true;
        }
    }

    return false;
}

namespace
{

// The check behind "nearest npcs", cut down to the illusion entries before the range test.
struct IllusionMobInRangeCheck
{
    Acore::AnyUnitInObjectRangeCheck inRange;
    Position centre;
    float radius;

    bool operator()(Unit* unit)
    {
        if (!unit->IsAlive())
            return false;

        return std::find(ULDUAR_YOGG_SARON_ILLUSION_MOBS.begin(), ULDUAR_YOGG_SARON_ILLUSION_MOBS.end(),
                         unit->GetEntry()) != ULDUAR_YOGG_SARON_ILLUSION_MOBS.end() &&
               centre.GetExactDist2d(unit) <= radius && inRange(unit);
    }
};

// Everything a pet may be pointed at instead of a Crusher Tentacle: the dps resolver's phase 2 ladder minus it.
// No Influence Tentacles: they live 93 yd below the platform, in a room the owner isn't in.
const std::vector<uint32> yoggSaronPetTargets = {NPC_GUARDIAN_OF_YS, NPC_CONSTRICTOR_TENTACLE,
                                                 NPC_CORRUPTOR_TENTACLE, NPC_MARKED_IMMORTAL_GUARDIAN,
                                                 NPC_IMMORTAL_GUARDIAN};

struct PetTargetInRangeCheck
{
    Acore::AnyUnitInObjectRangeCheck inRange;

    bool operator()(Unit* unit)
    {
        if (!unit->IsAlive())
            return false;

        return std::find(yoggSaronPetTargets.begin(), yoggSaronPetTargets.end(), unit->GetEntry()) !=
                   yoggSaronPetTargets.end() &&
               inRange(unit);
    }
};

// Whether a Crusher Tentacle can produce a Crush cone at all: 64146 is a self-buff procced by its white swing
// at 100%, and it only swings at a victim in melee range (both combat reaches plus 4/3, at least NOMINAL_MELEE_RANGE).
bool YoggSaronCrusherCanSwing(PlayerbotAI* botAI, Creature* crusher)
{
    // Stunned can't swing. It comes back inside the stun's exit lead, so the wedge is up before the stun
    // lifts.
    if (YoggSaronCrusherStunHolds(botAI, crusher))
        return false;

    Unit* victim = crusher->GetVictim();

    return victim && victim->IsAlive() && crusher->IsWithinMeleeRange(victim);
}

}  // namespace

Unit* YoggSaronLiveIllusionMob(PlayerbotAI* botAI, float radius, Position const* centre)
{
    Player* bot = botAI->GetBot();
    Position const around = centre ? *centre : bot->GetPosition();

    // One sweep anchored on the bot, since it carries the map and the phase, graded by distance from `around`.
    // It must reach the far side of the circle; the membership test keeps the next room out.
    float const sweep = bot->GetExactDist2d(around.GetPositionX(), around.GetPositionY()) + radius;

    std::vector<Unit*> found;
    IllusionMobInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(bot, sweep), around, radius};
    Acore::UnitListSearcher<IllusionMobInRangeCheck> searcher(bot, found, check);
    Cell::VisitObjects(bot, searcher, sweep);

    return found.empty() ? nullptr : found.front();
}

std::vector<Unit*> GetYoggSaronRoomIllusionMobs(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Position middle;
    if (!YoggSaronRoomMiddle(bot, middle))
        return {};

    float const radius = ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS;
    float const sweep = bot->GetExactDist2d(middle.GetPositionX(), middle.GetPositionY()) + radius;

    std::vector<Unit*> found;
    IllusionMobInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(bot, sweep), middle, radius};
    Acore::UnitListSearcher<IllusionMobInRangeCheck> searcher(bot, found, check);
    Cell::VisitObjects(bot, searcher, sweep);

    std::sort(found.begin(), found.end(),
              [bot](Unit* left, Unit* right) { return bot->GetExactDist(left) < bot->GetExactDist(right); });

    return found;
}

namespace
{
bool YoggSaronIllusionHealerStationNow(PlayerbotAI* botAI, Position& station)
{
    Player* bot = botAI->GetBot();

    Position middle;
    if (!YoggSaronRoomMiddle(bot, middle))
        return false;

    std::vector<Unit*> const mobs = GetYoggSaronRoomIllusionMobs(botAI);
    if (mobs.empty())
    {
        station = middle;
        return true;
    }

    float x = 0.0f;
    float y = 0.0f;
    for (Unit* mob : mobs)
    {
        x += mob->GetPositionX();
        y += mob->GetPositionY();
    }

    x /= static_cast<float>(mobs.size());
    y /= static_cast<float>(mobs.size());

    // The room middle's own Z, not the tentacles' or their average: the Chamber floor ramps from 239.7 to 244,
    // so an average puts the station in the air on one side.
    station = Position(x, y, middle.GetPositionZ());

    return true;
}
}  // namespace

bool YoggSaronIllusionHealerStation(PlayerbotAI* botAI, Position& station)
{
    YoggSaronPassReads* reads = YoggSaronPassReadsFor(botAI);
    if (reads && reads->healerStationRead)
    {
        station = reads->healerStation;
        return reads->healerStationFound;
    }

    bool const found = YoggSaronIllusionHealerStationNow(botAI, station);

    if (reads)
    {
        reads->healerStationRead = true;
        reads->healerStationFound = found;
        reads->healerStation = station;
    }

    return found;
}

Unit* YoggSaronPetFallbackTarget(PlayerbotAI* botAI, Creature* pet)
{
    if (!pet)
        return nullptr;

    std::vector<Unit*> found;
    PetTargetInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(pet, ULDUAR_YOGG_SARON_PET_TARGET_RADIUS)};
    Acore::UnitListSearcher<PetTargetInRangeCheck> searcher(pet, found, check);
    Cell::VisitObjects(pet, searcher, ULDUAR_YOGG_SARON_PET_TARGET_RADIUS);

    Unit* nearest = nullptr;
    float best = 0.0f;
    for (Unit* candidate : found)
    {
        float const distance = pet->GetExactDist2d(candidate);
        if (!nearest || distance < best)
        {
            nearest = candidate;
            best = distance;
        }
    }

    return nearest;
}

bool YoggSaronBeaconHealReaches(Unit* unit)
{
    float const reach =
        ULDUAR_YOGG_SARON_BEACON_HEAL_RADIUS + ULDUAR_YOGG_SARON_BEACON_HEAL_SLACK + unit->GetObjectSize();

    for (YoggSaronBeaconLane const& lane : YOGG_SARON_BEACON_LANES)
        for (Position const* spot : {lane.drag, lane.hunter})
            if (unit->GetExactDist2d(spot->GetPositionX(), spot->GetPositionY()) <= reach)
                return true;

    return false;
}

Unit* YoggSaronPhase3PetLeftover(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (YoggSaronOnBrainLevel(bot) || !YoggSaronInPhase3(botAI))
        return nullptr;

    bool const thorim = YoggThorimKeeperActive(botAI);

    // From Yogg, so every owner sends its pet to the same one.
    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    if (!yogg)
        return nullptr;

    std::list<Creature*> leftovers;
    yogg->GetCreatureListWithEntryInGrid(leftovers, {NPC_CORRUPTOR_TENTACLE, NPC_CONSTRICTOR_TENTACLE},
                                         ULDUAR_YOGG_SARON_PET_LEFTOVER_SEARCH_RADIUS);
    if (leftovers.empty())
        return nullptr;

    std::list<Creature*> crushers;
    yogg->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE, ULDUAR_YOGG_SARON_PET_LEFTOVER_SEARCH_RADIUS);

    // One a beacon heal reaches has to die between two beacons or it's back to full, so the pets help the
    // ranged on it first.
    auto const rank = [thorim](Creature* tentacle)
    {
        return std::make_tuple(!YoggSaronConstrictorHolding(tentacle), thorim || !YoggSaronBeaconHealReaches(tentacle),
                               tentacle->GetHealth(), tentacle->GetGUID());
    };

    Creature* best = nullptr;
    for (Creature* leftover : leftovers)
    {
        if (!leftover->IsAlive())
            continue;

        bool const nearCrusher = std::any_of(crushers.begin(), crushers.end(), [botAI, leftover](Creature* crusher)
            {
                return crusher->IsAlive() && !YoggSaronCrusherStunHolds(botAI, crusher) &&
                       crusher->GetExactDist2d(leftover) <= ULDUAR_YOGG_SARON_CRUSH_RANGE;
            });
        if (nearCrusher)
            continue;

        if (!best || rank(leftover) < rank(best))
            best = leftover;
    }

    return best;
}

bool YoggSaronInfluenceTentaclesCleared(PlayerbotAI* botAI)
{
    // Room radius, not 200 yd: the Stormwind and Chamber middles are 200.8 yd apart. Damaging the Brain while one
    // lives is Unit::Kill(who, who) on the dealer. From the brain room the reach covers whichever room it came from.
    Player* bot = botAI->GetBot();
    bool const brainRoom = YoggSaronRoomOf(bot) == YOGG_SARON_ROOM_BRAIN;
    float const radius = brainRoom ? ULDUAR_YOGG_SARON_BRAIN_ROOM_RADIUS + ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS
                                   : ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS;

    Position middle;
    // Inside an illusion room the circle sits on that room's middle, not the bot: the Chamber is 96 yd deep and
    // a bot in its doorway reads only the near half.
    Position const around = !brainRoom && YoggSaronRoomMiddle(bot, middle) ? middle : bot->GetPosition();

    YoggSaronPassReads* reads = YoggSaronPassReadsFor(botAI);
    if (reads)
        for (YoggSaronPassReads::ClearRead const& answer : reads->tentaclesCleared)
            if (answer.radius == radius && answer.x == around.GetPositionX() &&
                answer.y == around.GetPositionY())
                return answer.cleared;

    bool const cleared = !YoggSaronLiveIllusionMob(botAI, radius, &around);
    if (reads)
        reads->tentaclesCleared.push_back(
            {around.GetPositionX(), around.GetPositionY(), radius, cleared});

    return cleared;
}

bool YoggSaronPlatformIdle(PlayerbotAI* botAI)
{
    for (uint32 entry : {NPC_GUARDIAN_OF_YS, NPC_CRUSHER_TENTACLE, NPC_CONSTRICTOR_TENTACLE, NPC_CORRUPTOR_TENTACLE})
        if (YoggSaronNearestCreature(botAI, entry))
            return false;

    return true;
}

std::vector<Position> GetYoggSaronCrushWedges(PlayerbotAI* botAI, float searchRadius)
{
    Player* bot = botAI->GetBot();

    std::list<Creature*> crushers;
    bot->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE, searchRadius);

    std::vector<Position> wedges;
    for (Creature* crusher : crushers)
    {
        if (!crusher->IsAlive() || crusher->GetVictim() == bot)
            continue;

        if (!YoggSaronCrusherCanSwing(botAI, crusher))
            continue;

        wedges.push_back(crusher->GetPosition());
    }

    return wedges;
}

bool InYoggSaronCrushWedge(std::vector<Position> const& wedges, float x, float y, float arcDegrees)
{
    float const arc = arcDegrees * static_cast<float>(M_PI) / 180.0f;

    for (Position const& wedge : wedges)
    {
        if (wedge.GetExactDist2d(x, y) > ULDUAR_YOGG_SARON_CRUSH_RANGE)
            continue;

        float const bearing = std::atan2(y - wedge.GetPositionY(), x - wedge.GetPositionX());
        float offset = Position::NormalizeOrientation(bearing - wedge.GetOrientation());
        if (offset > static_cast<float>(M_PI))
            offset = 2.0f * static_cast<float>(M_PI) - offset;

        if (offset <= arc)
            return true;
    }

    return false;
}

std::vector<Position> GetYoggSaronCrusherReaches(PlayerbotAI* botAI, float searchRadius)
{
    Player* bot = botAI->GetBot();

    std::list<Creature*> crushers;
    bot->GetCreatureListWithEntryInGrid(crushers, NPC_CRUSHER_TENTACLE, searchRadius);

    bool const melee = PlayerbotAI::IsMelee(bot);

    std::vector<Position> reaches;
    for (Creature* crusher : crushers)
    {
        if (!crusher->IsAlive())
            continue;

        // Melee are only ever let onto a stunned one, and a Constrictor next to a live one would have
        // this circle and reach melee trade the bot every tick.
        if (melee &&
            (!crusher->HasAura(SPELL_SHATTERED_ILLUSION) || YoggSaronCrusherStunHolds(botAI, crusher)))
            continue;

        reaches.push_back(crusher->GetPosition());
    }

    return reaches;
}

namespace
{
// Some brain-room bot is holding off the Brain, seen in the last couple of seconds so a team that died
// below doesn't hold it forever.
bool YoggSaronBrainHoldInForce(Player* bot)
{
    uint32 const now = getMSTime();
    for (auto const& [guid, hold] : YoggSaronStateFor(bot).brainHold)
        if ((hold.mode == YoggSaronEncounterState::BRAIN_HOLD_FULL ||
             hold.mode == YoggSaronEncounterState::BRAIN_HOLD_DOTS) &&
            getMSTimeDiff(hold.seenMs, now) < ULDUAR_YOGG_SARON_BRAIN_HOLD_SEEN_MS)
            return true;

    return false;
}
}  // namespace

bool YoggSaronCrusherStunHolds(PlayerbotAI* botAI, Unit* crusher)
{
    if (!crusher || !crusher->IsAlive() || !crusher->HasAura(SPELL_SHATTERED_ILLUSION))
        return false;

    // The stun comes off in the same tick Induce Madness lands, or when phase 3 starts. A held Brain only
    // goes under once the brain team finishes it, so the stun lasts until shortly before that instead.
    Creature* brain = YoggSaronNearestCreature(botAI, NPC_BRAIN);
    if (!brain || !brain->IsAlive())
        return false;

    bool const held = brain->GetHealthPct() > ULDUAR_YOGG_SARON_BRAIN_HOLD_GUARD_PCT &&
                      YoggSaronBrainHoldInForce(botAI->GetBot());
    if (!held && brain->GetHealthPct() <= ULDUAR_YOGG_SARON_STUN_BRAIN_FLOOR_PCT)
        return false;

    uint32 const lead = held ? ULDUAR_YOGG_SARON_BRAIN_RESUME_MS + ULDUAR_YOGG_SARON_STUN_EXIT_LEAD_MS
                             : ULDUAR_YOGG_SARON_STUN_EXIT_LEAD_MS;

    Spell* induceMadness = brain->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return induceMadness && induceMadness->m_spellInfo->Id == SPELL_INDUCE_MADNESS &&
           induceMadness->GetCastTimeRemaining() > static_cast<int32>(lead);
}

bool YoggSaronInCrusherReach(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();

    // The grid radius adds both object sizes, 8 yd of it the tentacle's, so this list overshoots.
    // Centre distance is the real test.
    for (Position const& reach : GetYoggSaronCrusherReaches(botAI, radius))
        if (reach.GetExactDist2d(bot) <= radius)
            return true;

    return false;
}

namespace
{
struct YoggSaronBrainLinkPair
{
    ObjectGuid owner;
    ObjectGuid partner;
    uint32 seenMs = 0;
};

// Per instance, a handful of entries at a time: one link is up at once and each is 30 s long.
RaidInstanceState<std::vector<YoggSaronBrainLinkPair>> yoggSaronBrainLinkPairs;
}  // namespace

void YoggSaronNoteBrainLinkPair(Unit* owner, Unit* partner)
{
    if (!owner || !partner || owner == partner)
        return;

    uint32 const now = getMSTime();
    std::vector<YoggSaronBrainLinkPair>& pairs = yoggSaronBrainLinkPairs.For(owner->GetInstanceId());

    for (YoggSaronBrainLinkPair& pair : pairs)
    {
        if (pair.owner == owner->GetGUID())
        {
            pair.partner = partner->GetGUID();
            pair.seenMs = now;
            return;
        }
    }

    pairs.push_back(YoggSaronBrainLinkPair{owner->GetGUID(), partner->GetGUID(), now});
}

Player* YoggSaronBrainLinkTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    // The aura script drops the link outright once the two ends are more than 10 yd apart vertically,
    // so a bot that has taken a portal down has nothing left to close on.
    if (bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return nullptr;

    // Whichever end of the pair the bot is: only the owner carries 63802 and the partner's GUID lives in the aura
    // script, so a HasAura test would leave the partner standing still while the owner chased it.
    uint32 const now = getMSTime();
    std::vector<YoggSaronBrainLinkPair>& pairs = yoggSaronBrainLinkPairs.For(bot->GetInstanceId());

    // Three missed ticks. The link casts on the partner once a second whether the two are apart
    // (63803) or together (63804), so silence for that long means the aura has gone.
    pairs.erase(std::remove_if(pairs.begin(), pairs.end(),
                               [now](YoggSaronBrainLinkPair const& pair)
                               { return getMSTimeDiff(pair.seenMs, now) > ULDUAR_YOGG_SARON_BRAIN_LINK_PAIR_TTL_MS; }),
                pairs.end());

    ObjectGuid other;
    for (YoggSaronBrainLinkPair const& pair : pairs)
    {
        if (pair.owner == bot->GetGUID())
            other = pair.partner;
        else if (pair.partner == bot->GetGUID())
            other = pair.owner;
    }

    Player* partner = other ? ObjectAccessor::FindPlayer(other) : nullptr;
    if (partner && (!partner->IsAlive() || partner->GetMapId() != bot->GetMapId()))
        partner = nullptr;

    bool const far = partner && bot->GetExactDist2d(partner) > ULDUAR_YOGG_SARON_BRAIN_LINK_CLOSE;

    return far ? partner : nullptr;
}

Player* YoggSaronSqueezeVictim(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    uint32 const squeeze = sSpellMgr->GetSpellIdForDifficulty(SPELL_SQUEEZE, bot);

    Player* victim = nullptr;
    float best = 0.0f;
    float bestDistance = 0.0f;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || !member->HasAura(squeeze))
            continue;

        float const distance = bot->GetExactDist2d(member);
        if (distance > ULDUAR_YOGG_SARON_HAND_OF_PROTECTION_RANGE)
            continue;

        // Lowest health first, because Squeeze is a flat tick and the question is who runs out of room
        // soonest. Nearest breaks the tie, which is also the least likely to walk out of range mid-cast.
        float const health = member->GetHealthPct();
        if (!victim || health < best || (health == best && distance < bestDistance))
        {
            victim = member;
            best = health;
            bestDistance = distance;
        }
    }

    return victim;
}

bool ClaimYoggSaronSqueezeRescue(PlayerbotAI* botAI, Player* victim)
{
    if (!victim)
        return false;

    YoggSaronEncounterState& state = YoggSaronStateFor(botAI->GetBot());

    uint32 const now = getMSTime();
    uint32& claimed = state.squeezeClaims[victim->GetGUID()];
    if (claimed && getMSTimeDiff(claimed, now) < ULDUAR_YOGG_SARON_SQUEEZE_CLAIM_MS)
        return false;

    claimed = now;

    return true;
}

bool YoggSaronFearWindowActive(PlayerbotAI* botAI)
{
    uint32 const phase = YoggSaronPhase(botAI);

    return phase == 2 || phase == 3;
}

bool YoggSaronReprisalHold(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return YoggSaronStateFor(bot).reprisalHold.count(bot->GetGUID()) != 0;
}

Player* YoggSaronRoomHealer(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronRoom const room = YoggSaronRoomOf(bot);
    Group* group = bot->GetGroup();
    if (!group || (room != YOGG_SARON_ROOM_STORMWIND && room != YOGG_SARON_ROOM_ICECROWN &&
                   room != YOGG_SARON_ROOM_CHAMBER))
        return nullptr;

    Player* nearest = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || !PlayerbotAI::IsHeal(member) ||
            YoggSaronRoomOf(member) != room)
            continue;

        if (!nearest || bot->GetExactDist2d(member) < bot->GetExactDist2d(nearest))
            nearest = member;
    }

    return nearest;
}

namespace
{
// Induce Madness has this little left, or less. False with no cast going, so no deadline then.
bool YoggSaronInduceMadnessWithin(PlayerbotAI* botAI, uint32 ms)
{
    Creature* brain = YoggSaronNearestCreature(botAI, NPC_BRAIN);
    if (!brain || !brain->IsAlive() || !brain->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* induceMadness = brain->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return induceMadness && induceMadness->m_spellInfo->Id == SPELL_INDUCE_MADNESS &&
           induceMadness->GetCastTimeRemaining() <= static_cast<int32>(ms);
}

// Any Crusher, Corruptor or Constrictor alive on the platform. Swept from Yogg, which a bot on the brain
// level still finds inside its 200 yd.
bool YoggSaronPlatformTentacleAlive(PlayerbotAI* botAI)
{
    YoggSaronPassReads* reads = YoggSaronPassReadsFor(botAI);
    if (reads && reads->platformTentacleRead)
        return reads->platformTentacleAlive;

    bool alive = false;
    if (Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON))
    {
        std::list<Creature*> tentacles;
        yogg->GetCreatureListWithEntryInGrid(tentacles,
                                             {NPC_CRUSHER_TENTACLE, NPC_CORRUPTOR_TENTACLE, NPC_CONSTRICTOR_TENTACLE},
                                             ULDUAR_YOGG_SARON_PET_LEFTOVER_SEARCH_RADIUS);
        alive = std::any_of(tentacles.begin(), tentacles.end(), [](Creature* tentacle) { return tentacle->IsAlive(); });
    }

    if (reads)
    {
        reads->platformTentacleRead = true;
        reads->platformTentacleAlive = alive;
    }

    return alive;
}

// Latched per tick for the resolver, rules and multiplier. DoTs stop first, all damage at the hold line while a
// tentacle is on the platform; finish with ULDUAR_YOGG_SARON_BRAIN_RESUME_MS of Induce Madness left or when clear.
void YoggSaronTickBrainHold(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    ObjectGuid const guid = bot->GetGUID();

    if (!bot->IsAlive() || PlayerbotAI::IsHeal(bot) || YoggSaronRoomOf(bot) != YOGG_SARON_ROOM_BRAIN)
    {
        state.brainHold.erase(guid);
        return;
    }

    Creature* brain = YoggSaronNearestCreature(botAI, NPC_BRAIN);
    Spell* induceMadness = brain && brain->IsAlive() ? brain->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    if (!induceMadness || induceMadness->m_spellInfo->Id != SPELL_INDUCE_MADNESS)
    {
        state.brainHold.erase(guid);
        return;
    }

    uint32 const now = getMSTime();
    uint32 const left = static_cast<uint32>(std::max(0, induceMadness->GetCastTimeRemaining()));
    uint32 const castEndMs = now + left;

    YoggSaronEncounterState::BrainHold& hold = state.brainHold[guid];
    hold.seenMs = now;

    // The same cast as the finish if its end lands within a second of it.
    bool const finishing = hold.mode == YoggSaronEncounterState::BRAIN_HOLD_FINISH &&
                           (castEndMs > hold.finishCastEndMs ? castEndMs - hold.finishCastEndMs
                                                             : hold.finishCastEndMs - castEndMs) < IN_MILLISECONDS;
    if (finishing)
        return;

    float const health = brain->GetHealthPct();
    YoggSaronEncounterState::BrainHoldMode mode = YoggSaronEncounterState::BRAIN_HOLD_NONE;
    if (health > ULDUAR_YOGG_SARON_BRAIN_DOT_STOP_PCT)
        mode = YoggSaronEncounterState::BRAIN_HOLD_NONE;
    else if (!YoggSaronPlatformTentacleAlive(botAI))
    {
        mode = YoggSaronEncounterState::BRAIN_HOLD_FINISH;
    }
    else if (left <= ULDUAR_YOGG_SARON_BRAIN_RESUME_MS)
    {
        mode = YoggSaronEncounterState::BRAIN_HOLD_FINISH;
    }
    else if (health <= ULDUAR_YOGG_SARON_BRAIN_HOLD_PCT)
    {
        mode = YoggSaronEncounterState::BRAIN_HOLD_FULL;
    }
    else
        mode = YoggSaronEncounterState::BRAIN_HOLD_DOTS;

    hold.mode = mode;
    if (mode == YoggSaronEncounterState::BRAIN_HOLD_FINISH)
        hold.finishCastEndMs = castEndMs;
}

// Deafening Roar's clock, paladins only since only they read it. The cast start is back-dated off Yogg's cast
// bar, so a late first look keys the same roar.
void YoggSaronTickRoar(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_PALADIN)
        return;

    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    state.roarExpected = bot->GetMap()->Is25ManRaid() && YoggSaronInPhase3(botAI) &&
                         YoggKeepersFreedCount(botAI) < ULDUAR_YOGG_SARON_KEEPER_COUNT;
    if (!state.roarExpected)
        return;

    uint32 const now = getMSTime();
    if (!state.phase3StartMs)
        state.phase3StartMs = now;

    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    Spell* cast = yogg && yogg->IsAlive() ? yogg->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    if (!cast || cast->m_spellInfo->Id != SPELL_DEAFENING_ROAR)
        return;

    uint32 const elapsed = static_cast<uint32>(std::max(0, cast->GetCastTime() - cast->GetCastTimeRemaining()));
    uint32 const start = now - elapsed;
    // Forward only. Paladins read the cast bar a few ms apart, and a start just before the stored one
    // would wrap getMSTimeDiff, re-key the roar and let a second paladin press.
    int32 const sinceLast = static_cast<int32>(start - state.lastRoarMs);
    if (!state.lastRoarMs || sinceLast > static_cast<int32>(ULDUAR_YOGG_SARON_ROAR_PERIOD_MS / 2))
        state.lastRoarMs = start;
}

// Latched before the triggers so resolver, rules and room state read one answer per tick. Stops under the stop
// line, resumes at the resume line. Only with a healer in the room, never in the last seconds (the room charms anyway).
void YoggSaronTickReprisal(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    ObjectGuid const guid = bot->GetGUID();
    bool const held = state.reprisalHold.count(guid) != 0;

    YoggSaronRoom const room = YoggSaronRoomOf(bot);
    bool const inIllusion =
        room == YOGG_SARON_ROOM_STORMWIND || room == YOGG_SARON_ROOM_ICECROWN || room == YOGG_SARON_ROOM_CHAMBER;

    // Cheap tests first, the sweeps only for a bot that would be held.
    float const line = held ? ULDUAR_YOGG_SARON_REPRISAL_RESUME_PCT : ULDUAR_YOGG_SARON_REPRISAL_STOP_PCT;
    bool hold = inIllusion && bot->IsAlive() && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot) &&
                bot->GetHealthPct() < line;
    hold = hold && YoggSaronRoomHealer(botAI) && !YoggSaronInfluenceTentaclesCleared(botAI) &&
           !YoggSaronInduceMadnessWithin(botAI, ULDUAR_YOGG_SARON_REPRISAL_ALL_IN_MS);

    if (hold)
        state.reprisalHold.insert(guid);
    else if (held)
        state.reprisalHold.erase(guid);
}

// The share plans are per wave: the next team lands with nothing assigned.
YoggSaronEncounterState& YoggSaronShareStateFor(Player* bot)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    if (state.shareWave != state.waveOrdinal)
    {
        state.shareWave = state.waveOrdinal;
        state.icecrownCluster.clear();
        state.stormwindShare.clear();
    }

    return state;
}

uint8 YoggSaronIcecrownClusterAt(Unit const* unit)
{
    uint8 nearest = 0;
    for (uint8 cluster = 1; cluster < ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT; ++cluster)
        if (unit->GetExactDist2d(ULDUAR_YOGG_SARON_ICECROWN_CLUSTERS[cluster]) <
            unit->GetExactDist2d(ULDUAR_YOGG_SARON_ICECROWN_CLUSTERS[nearest]))
            nearest = cluster;

    return nearest;
}

// Joins the group with the fewest bots so far, nearest the landing first on a tie. Bots land one after
// another, so nine come out 3/3/3 and a 10-man's three 1/1/1.
uint8 YoggSaronIcecrownClusterOf(YoggSaronEncounterState& state, Player* bot)
{
    auto const it = state.icecrownCluster.find(bot->GetGUID());
    if (it != state.icecrownCluster.end())
        return it->second;

    std::array<uint32, ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT> planned{};
    for (auto const& [guid, cluster] : state.icecrownCluster)
        ++planned[cluster];

    // East, west, south.
    constexpr std::array<uint8, ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT> byLandingDistance = {2, 0, 1};
    uint8 joined = byLandingDistance[0];
    for (uint8 const cluster : byLandingDistance)
        if (planned[cluster] < planned[joined])
            joined = cluster;

    state.icecrownCluster[bot->GetGUID()] = joined;
    return joined;
}

// Somebody planned for this group is still alive in the room. Reprisal-held counts, it comes back.
bool YoggSaronIcecrownClusterManned(YoggSaronEncounterState const& state, Player* bot, uint8 cluster)
{
    for (auto const& [guid, planned] : state.icecrownCluster)
    {
        if (planned != cluster || guid == bot->GetGUID())
            continue;

        Player* member = ObjectAccessor::FindPlayer(guid);
        if (member && member->IsAlive() && YoggSaronRoomOf(member) == YOGG_SARON_ROOM_ICECROWN)
            return true;
    }

    return false;
}

// Another live damage dealer in the room has this tentacle targeted, humans included.
bool YoggSaronTentacleHeldByOther(Player* bot, Unit* tentacle, YoggSaronRoom room)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && !PlayerbotAI::IsHeal(member) &&
            member->GetTarget() == tentacle->GetGUID() && YoggSaronRoomOf(member) == room)
            return true;
    }

    return false;
}

// Latched here, before the triggers run, so the resolver, the room state and the stage hold all read
// one answer per tick. See YoggSaronIllusionTentacleAllowed for the rules.
void YoggSaronTickIllusionShare(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronRoom const room = YoggSaronRoomOf(bot);
    bool const applies = (room == YOGG_SARON_ROOM_ICECROWN || room == YOGG_SARON_ROOM_STORMWIND) &&
                         bot->IsAlive() && !PlayerbotAI::IsHeal(bot);

    YoggSaronEncounterState& state = YoggSaronShareStateFor(bot);
    if (!applies)
    {
        state.illusionShare.erase(bot->GetGUID());
        return;
    }

    std::vector<Unit*> const mobs = GetYoggSaronRoomIllusionMobs(botAI);
    YoggSaronEncounterState::IllusionShare& share = state.illusionShare[bot->GetGUID()];
    share.allowed.clear();

    auto const allow = [&share](Unit* mob) { share.allowed.push_back(mob->GetGUID()); };

    if (YoggSaronInduceMadnessWithin(botAI, ULDUAR_YOGG_SARON_SHARE_LIFT_MS))
    {
        std::for_each(mobs.begin(), mobs.end(), allow);
    }
    else if (room == YOGG_SARON_ROOM_ICECROWN)
    {
        uint8 const own = YoggSaronIcecrownClusterOf(state, bot);

        std::array<bool, ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT> live{};
        for (Unit* mob : mobs)
            live[YoggSaronIcecrownClusterAt(mob)] = true;

        if (live[own])
        {
            for (Unit* mob : mobs)
                if (YoggSaronIcecrownClusterAt(mob) == own)
                    allow(mob);
        }
        else
        {
            for (uint8 cluster = 0; cluster < ULDUAR_YOGG_SARON_ICECROWN_CLUSTER_COUNT; ++cluster)
            {
                if (!live[cluster] || YoggSaronIcecrownClusterManned(state, bot, cluster))
                    continue;

                for (Unit* mob : mobs)
                    if (YoggSaronIcecrownClusterAt(mob) == cluster)
                        allow(mob);
            }
        }
    }
    else
    {
        YoggSaronEncounterState::StormwindShare& stormwind = state.stormwindShare[bot->GetGUID()];
        Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        bool const onTentacle =
            target && target->IsAlive() &&
            std::find(ULDUAR_YOGG_SARON_ILLUSION_MOBS.begin(), ULDUAR_YOGG_SARON_ILLUSION_MOBS.end(),
                      target->GetEntry()) != ULDUAR_YOGG_SARON_ILLUSION_MOBS.end();

        // The role comes from the first tentacle the bot was on when it died, whoever killed it.
        if (stormwind.role == YoggSaronEncounterState::STORMWIND_FIRST)
        {
            if (onTentacle)
            {
                stormwind.held = target->GetGUID();
                stormwind.heldNear = target->GetExactDist2d(ULDUAR_YOGG_SARON_STORMWIND_STAGE_SPOT) <=
                                     ULDUAR_YOGG_SARON_STORMWIND_HELPER_RADIUS;
            }
            else if (!stormwind.held.IsEmpty())
            {
                Unit* held = ObjectAccessor::GetUnit(*bot, stormwind.held);
                if (!held || !held->IsAlive())
                    stormwind.role = stormwind.heldNear ? YoggSaronEncounterState::STORMWIND_HELPER
                                                        : YoggSaronEncounterState::STORMWIND_DONE;
            }
        }

        if (stormwind.role == YoggSaronEncounterState::STORMWIND_DONE)
        {
            for (Unit* mob : mobs)
                if ((onTentacle && mob == target) || !YoggSaronTentacleHeldByOther(bot, mob, room))
                    allow(mob);
        }
        else
            std::for_each(mobs.begin(), mobs.end(), allow);
    }

    // A player's mark still wins.
    Unit* marked = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
    if (marked && std::find(mobs.begin(), mobs.end(), marked) != mobs.end() &&
        std::find(share.allowed.begin(), share.allowed.end(), marked->GetGUID()) == share.allowed.end())
        allow(marked);

    share.staged = !mobs.empty() && share.allowed.empty();
}

// Shortest cooldown first, so the cast that gets stopped buys the kick that's back soonest.
uint32 YoggSaronKickRank(char const* spell)
{
    static constexpr char const* order[] = {"wind shear",  "kick",         "pummel",    "mind freeze",
                                            "shield bash", "counterspell", "spell lock"};
    for (uint32 rank = 0; rank < std::size(order); ++rank)
        if (std::string(spell) == order[rank])
            return rank;

    return std::size(order);
}

// A bot mid cast never runs its kick node, so an unkicked Drain Life gets one bot's cast stopped. Last resort:
// not while a free bot can kick it or a ready one finishes within the wait, and never a healer's cast.
void YoggSaronNudgeDrainLifeKicker(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !bot->IsAlive() || bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return;

    // Raid-wide, so one bot a scan is plenty.
    constexpr uint32 scanIntervalMs = 200;
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();
    if (getMSTimeDiff(state.kickScanMs, now) < scanIntervalMs)
        return;

    state.kickScanMs = now;
    for (auto it = state.kickNudges.begin(); it != state.kickNudges.end();)
        it = getMSTimeDiff(it->second, now) >= ULDUAR_YOGG_SARON_KICK_NUDGE_MS ? state.kickNudges.erase(it)
                                                                              : std::next(it);

    if (!YoggSaronInPhase3(botAI))
        return;

    std::unordered_set<ObjectGuid> nudged;
    for (Unit* guardian : YoggSaronInterruptCastersAround(botAI))
    {
        Spell* channel = YoggSaronDrainLifeKickable(guardian);

        // A mark's channel gets no grace: the drag stands still for as long as it runs.
        bool const mark = YoggSaronMarkCaster(guardian);
        if (!channel || state.kickNudges.count(guardian->GetGUID()) ||
            (!mark && channel->GetSpellInfo()->GetDuration() - channel->GetCastTimeRemaining() <
                          static_cast<int32>(ULDUAR_YOGG_SARON_KICK_LAST_RESORT_MS)))
            continue;

        auto const claim = state.interruptClaims.find(guardian->GetGUID());
        if (claim != state.interruptClaims.end() &&
            getMSTimeDiff(claim->second.second, now) < ULDUAR_YOGG_SARON_INTERRUPT_CLAIM_MS)
            continue;

        bool waiting = false;
        Player* pick = nullptr;
        char const* pickSpell = nullptr;
        std::tuple<uint32, int32> pickKey;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            PlayerbotAI* memberAI = IsYoggSaronBot(member) ? GET_PLAYERBOT_AI(member) : nullptr;
            if (!memberAI || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
                nudged.count(member->GetGUID()) || (!mark && YoggSaronMarkKicker(member)))
                continue;

            char const* ready = nullptr;
            for (char const* spell : YoggSaronInterruptSpellsFor(member, guardian))
            {
                if (YoggSaronKickInReach(memberAI, spell, guardian))
                {
                    ready = spell;
                    break;
                }
            }

            if (!ready)
                continue;

            Spell* cast = member->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (cast && cast->getState() != SPELL_STATE_PREPARING)
                cast = nullptr;

            if (!cast)
                cast = member->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

            if (!cast || cast->GetCastTimeRemaining() <= static_cast<int32>(ULDUAR_YOGG_SARON_KICK_WAIT_MS))
            {
                waiting = true;
                break;
            }

            if (PlayerbotAI::IsHeal(member))
                continue;

            // Ties to the longest cast left, the bot that would otherwise be busy longest.
            std::tuple<uint32, int32> const key{YoggSaronKickRank(ready), -cast->GetCastTimeRemaining()};
            if (!pick || key < pickKey)
            {
                pick = member;
                pickSpell = ready;
                pickKey = key;
            }
        }

        if (waiting || !pick)
            continue;

        GET_PLAYERBOT_AI(pick)->RequestSpellInterrupt();
        state.kickNudges[guardian->GetGUID()] = now;
        nudged.insert(pick->GetGUID());
    }
}

// Longest reach first: from the ranged station Counterspell and Spell Lock reach the pile and the
// north-east lane, Wind Shear the pile. Then the shorter cooldown, healers last.
void YoggSaronPickMarkKickers(Player* bot, YoggSaronEncounterState& state, Unit* mark)
{
    state.markKickers.clear();

    using Rank = std::tuple<float, uint32, bool, ObjectGuid>;
    std::vector<std::pair<Rank, Player*>> ranked;
    for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        PlayerbotAI* memberAI = IsYoggSaronBot(member) ? GET_PLAYERBOT_AI(member) : nullptr;
        if (!memberAI || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
            member->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT ||
            YoggSaronBeaconDragging(member) || state.beaconStages.count(member->GetGUID()))
            continue;

        bool found = false;
        Rank best;
        for (char const* spell : YoggSaronInterruptSpellsFor(member, mark))
        {
            uint32 const id = memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", spell)->Get();
            SpellInfo const* info = id ? sSpellMgr->GetSpellInfo(id) : nullptr;
            if (!info || member->GetSpellCooldownDelay(id) > ULDUAR_YOGG_SARON_MARK_KICKER_READY_MS)
                continue;

            Rank const rank{-info->GetMaxRange(false), std::max(info->RecoveryTime, info->CategoryRecoveryTime),
                            PlayerbotAI::IsHeal(member), member->GetGUID()};
            if (!found || rank < best)
            {
                best = rank;
                found = true;
            }
        }

        if (found)
            ranked.emplace_back(best, member);
    }

    std::sort(ranked.begin(), ranked.end(),
              [](auto const& left, auto const& right) { return left.first < right.first; });

    for (size_t i = 0; i < ranked.size() && i < ULDUAR_YOGG_SARON_MARK_KICKERS; ++i)
        state.markKickers.push_back(ranked[i].second->GetGUID());
}

// The beacon clock, read off each beacon's first mark, raid-wide and one bot a scan. Picks the stagers
// ULDUAR_YOGG_SARON_BEACON_STAGE_LEAD_MS before the next beacon is due, and the kickers held while a mark lasts.
void YoggSaronTickBeacon(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!bot->IsAlive() || !bot->GetGroup() ||
        bot->GetPositionZ() < ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT)
        return;

    constexpr uint32 scanIntervalMs = 200;
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();
    if (getMSTimeDiff(state.beaconScanMs, now) < scanIntervalMs)
        return;

    state.beaconScanMs = now;

    if (!YoggSaronInPhase3(botAI) || YoggThorimKeeperActive(botAI))
    {
        state.beaconClockMs = 0;
        state.lastMarkMs = 0;
        state.stageDueMs = 0;
        state.markWindow = false;
        state.beaconStages.clear();
        state.markKickers.clear();
        return;
    }

    if (!state.beaconClockMs)
        state.beaconClockMs = now;

    Creature* yogg = YoggSaronNearestCreature(botAI, NPC_YOGG_SARON);
    if (!yogg)
        return;

    std::list<Creature*> found;
    yogg->GetCreatureListWithEntryInGrid(found, NPC_MARKED_IMMORTAL_GUARDIAN,
                                         ULDUAR_YOGG_SARON_BEACON_DRAG_SEARCH_RADIUS);
    Unit* mark = nullptr;
    for (Creature* guardian : found)
        if (guardian->IsAlive())
            mark = guardian;

    // The stages stay until the window closes: the claim reads them for the lane.
    bool const window = mark || !state.beaconDrags.empty();
    if (window && !state.markWindow && mark)
    {
        state.markWindow = true;
        state.lastMarkMs = now;
        YoggSaronPickMarkKickers(bot, state, mark);
    }
    else if (!window && state.markWindow)
    {
        state.markWindow = false;
        state.markKickers.clear();
        state.beaconStages.clear();
    }

    if (window)
        return;

    uint32 const due = (state.lastMarkMs ? state.lastMarkMs : state.beaconClockMs) + ULDUAR_YOGG_SARON_BEACON_PERIOD_MS;
    int32 const left = static_cast<int32>(due - now);
    if (left < -static_cast<int32>(ULDUAR_YOGG_SARON_BEACON_STAGE_TIMEOUT_MS))
    {
        state.beaconStages.clear();
        return;
    }

    if (left > static_cast<int32>(ULDUAR_YOGG_SARON_BEACON_STAGE_LEAD_MS) || state.stageDueMs == due)
        return;

    state.stageDueMs = due;

    std::list<Creature*> unmarked;
    yogg->GetCreatureListWithEntryInGrid(unmarked, NPC_IMMORTAL_GUARDIAN, ULDUAR_YOGG_SARON_BEACON_DRAG_SEARCH_RADIUS);
    std::vector<Player*> const order =
        YoggSaronBeaconDraggerOrder(bot, unmarked, static_cast<uint32>(std::max(0, left)));

    size_t const stagers =
        bot->GetMap()->Is25ManRaid() ? ULDUAR_YOGG_SARON_BEACON_STAGERS_25 : ULDUAR_YOGG_SARON_BEACON_STAGERS_10;
    size_t const count = std::min<size_t>(stagers, YOGG_SARON_BEACON_LANES.size());
    for (size_t i = 0; i < order.size() && i < count; ++i)
        state.beaconStages[order[i]->GetGUID()] = static_cast<uint8>(i);
}
}  // namespace

bool YoggSaronIllusionTentacleAllowed(PlayerbotAI* botAI, Unit* tentacle)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);
    auto const it = state.illusionShare.find(bot->GetGUID());
    if (it == state.illusionShare.end())
        return true;

    std::vector<ObjectGuid> const& allowed = it->second.allowed;
    return std::find(allowed.begin(), allowed.end(), tentacle->GetGUID()) != allowed.end();
}

bool YoggSaronIllusionStaged(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    YoggSaronEncounterState const& state = YoggSaronStateFor(bot);
    auto const it = state.illusionShare.find(bot->GetGUID());

    return it != state.illusionShare.end() && it->second.staged;
}

bool YoggSaronIllusionStageSpot(Player* bot, Position& spot)
{
    switch (YoggSaronRoomOf(bot))
    {
        case YOGG_SARON_ROOM_ICECROWN:
            spot = ULDUAR_YOGG_SARON_ICECROWN_STAGE_SPOT;
            return true;
        case YOGG_SARON_ROOM_STORMWIND:
            spot = ULDUAR_YOGG_SARON_STORMWIND_STAGE_SPOT;
            return true;
        default:
            return false;
    }
}

namespace
{
bool YoggSaronHoldsPhase1Guardian(Player* human)
{
    for (Unit* attacker : human->getAttackers())
        if (attacker->GetVictim() == human && attacker->GetEntry() == NPC_GUARDIAN_OF_YS && attacker->IsAlive())
            return true;

    return false;
}

// Only when the nova would kill him, like the bots' own rule. Anti-Magic Shell lasts 5 s so it waits for the
// 10% gate; a 12 s button goes out at 20%, which leaves a human time to react.
void YoggSaronCallHumanNova(Player* sender, Player* human)
{
    if (!YoggSaronInPhase1Room(human) || !YoggSaronHoldsPhase1Guardian(human))
        return;

    uint32 const weakest = ReadyTankDefensiveSpell(human, false, 0, 0);
    uint32 const needMs = YoggSaronNovaNeedMs(human, weakest && IsMagicOnlyTankDefensive(weakest));
    if (!needMs)
        return;

    CallHumanTankDefensive(sender, human, ReadyTankDefensiveSpell(human, false, 0, needMs), 0,
                           "a Guardian next to you is about to explode");
}

// Same lines the bots use: a pile the beacon healed, or the tank swap line.
void YoggSaronCallHumanPile(Player* sender, Player* human)
{
    if (human->getAttackers().empty() || YoggSaronOnBrainLevel(human))
        return;

    uint32 const spellId = ReadyTankDefensiveSpell(human, true, 0, 0);
    if (!spellId)
        return;

    uint32 const healed = YoggSaronPileHealedOn(human);
    if (healed >= ULDUAR_YOGG_SARON_BEACON_PILE_RESET_GUARDIANS)
    {
        CallHumanTankDefensive(sender, human, spellId, 0,
                               "the beacon healed " + std::to_string(healed) + " Guardians on you");
        return;
    }

    uint32 const pile = YoggSaronGuardiansOn(human);
    if (pile >= ULDUAR_YOGG_SARON_TANK_SWAP_MIN_GUARDIANS && human->GetHealthPct() < ULDUAR_YOGG_SARON_TANK_SWAP_PCT)
        CallHumanTankDefensive(sender, human, spellId, 0, std::to_string(pile) + " Guardians on you and you are low");
}

// A human tank has no trigger to press his own defensive, so a bot tells him.
void YoggSaronTickHumanTankCalls(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group || !bot->IsAlive())
        return;

    constexpr uint32 scanIntervalMs = 250;
    YoggSaronEncounterState& state = YoggSaronStateFor(bot);
    uint32 const now = getMSTime();
    if (getMSTimeDiff(state.humanCallScanMs, now) < scanIntervalMs)
        return;

    state.humanCallScanMs = now;

    uint32 const phase = YoggSaronPhase(botAI);
    if (phase != 1 && phase != 3)
        return;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* human = ref->GetSource();
        if (!human || IsYoggSaronBot(human) || !human->IsAlive() || human->GetMap() != bot->GetMap())
            continue;

        if (phase == 1)
            YoggSaronCallHumanNova(bot, human);
        else
            YoggSaronCallHumanPile(bot, human);
    }
}
}  // namespace

void YoggSaronTick(PlayerbotAI* botAI)
{
    YoggSaronTickHumanTankCalls(botAI);
    YoggSaronTickHandover(botAI);
    YoggSaronTickRtiMark(botAI);
    YoggSaronTickReprisal(botAI);
    YoggSaronTickIllusionShare(botAI);
    YoggSaronTickBrainHold(botAI);
    YoggSaronTickRoar(botAI);
    YoggSaronTickBeacon(botAI);
    YoggSaronNudgeDrainLifeKicker(botAI);
}

bool YoggSaronBrainHeld(PlayerbotAI* botAI)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(botAI->GetBot());
    auto const it = state.brainHold.find(botAI->GetBot()->GetGUID());

    return it != state.brainHold.end() && it->second.mode == YoggSaronEncounterState::BRAIN_HOLD_FULL;
}

bool YoggSaronBrainDotHeld(PlayerbotAI* botAI)
{
    YoggSaronEncounterState& state = YoggSaronStateFor(botAI->GetBot());
    auto const it = state.brainHold.find(botAI->GetBot()->GetGUID());

    return it != state.brainHold.end() && (it->second.mode == YoggSaronEncounterState::BRAIN_HOLD_FULL ||
                                           it->second.mode == YoggSaronEncounterState::BRAIN_HOLD_DOTS);
}
