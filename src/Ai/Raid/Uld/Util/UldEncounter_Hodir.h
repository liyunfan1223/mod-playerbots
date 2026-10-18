/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDENCOUNTERHODIR_H
#define PLAYERBOTS_ULDENCOUNTERHODIR_H

#include "EncounterHelpers.h"
#include "Position.h"
#include "UldData.h"

#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

// Hodir. A dps race against the 3:00 cache timer. Icicles keep everyone dodging, so almost every point
// derived here is snapped to the floor before use.

enum UlduarHodirIds
{
    NPC_HODIR_ICICLE_SMALL = 33169,  // lands every 2 s, always dodged
    NPC_HODIR_ICICLE_DRIFT = 33173,  // Flash Freeze drift: 7 yd blast, dodged only while still falling
    NPC_SNOWPACKED_ICICLE = 33174,   // the only shelter: invisible dummy carrying the Safe Area aura
    NPC_TOASTY_FIRE = 33342,
    NPC_HODIR_FLASH_FREEZE_BLOCK = 32938,   // ice block encasing a frozen helper; kill it to free them
    NPC_HODIR_FLASH_FREEZE_PLAYER = 32926,  // same, on a raider: the next Flash Freeze instakills them
    // The helpers, by faction. The _10 row spawns in both sizes, the _25 row only in 25man. Each block
    // is summoned by the helper inside it, so the summoner's entry says who a block holds.
    NPC_HODIR_MAGE_ALLIANCE_10 = 32893,
    NPC_HODIR_MAGE_ALLIANCE_25 = 33327,
    NPC_HODIR_MAGE_HORDE_10 = 32946,
    NPC_HODIR_MAGE_HORDE_25 = 33331,
    NPC_HODIR_DRUID_ALLIANCE_10 = 32901,
    NPC_HODIR_DRUID_ALLIANCE_25 = 33325,
    NPC_HODIR_DRUID_HORDE_10 = 32941,
    NPC_HODIR_DRUID_HORDE_25 = 33333,
    NPC_HODIR_SHAMAN_ALLIANCE_10 = 32900,
    NPC_HODIR_SHAMAN_ALLIANCE_25 = 33328,
    NPC_HODIR_SHAMAN_HORDE_10 = 32950,
    NPC_HODIR_SHAMAN_HORDE_25 = 33332,
    NPC_HODIR_PRIEST_ALLIANCE_10 = 32897,
    NPC_HODIR_PRIEST_ALLIANCE_25 = 33326,
    NPC_HODIR_PRIEST_HORDE_10 = 32948,
    NPC_HODIR_PRIEST_HORDE_25 = 33330,
    SPELL_FLASH_FREEZE = 61968,
    SPELL_BITING_COLD_PLAYER_AURA = 62039,
    SPELL_HODIR_FLASH_FREEZE_TRAPPED = 61969,
    SPELL_HODIR_STARLIGHT = 62807,
    SPELL_HODIR_TOASTY_FIRE_AURA = 62821,
    SPELL_HODIR_FROZEN_BLOWS = 62478,  // base id, difficulty-mapped at runtime
    SPELL_HODIR_STORM_CLOUD = 65123,   // base id, difficulty-mapped at runtime
    SPELL_HODIR_STORM_POWER = 63711,   // what the carrier hands out; also difficulty-mapped
    SPELL_HODIR_SHATTER_CHEST_TIMER = 65272,  // on himself from the pull for 180 s, so its age is the pull clock
};

// Starlight's DBC row says 8, but the aura stops holding past 3 yd.
inline constexpr float ULDUAR_HODIR_STARLIGHT_RADIUS = 3.0f;
inline constexpr float ULDUAR_HODIR_TOASTY_FIRE_RADIUS = 11.0f;
inline constexpr float ULDUAR_HODIR_SAFE_AREA_RADIUS = 9.0f;  // 62464, radius index 40
// Park and release differ on purpose: MoveInside lands exactly at TOLERANCE, so one number at both
// ends would release the bot the tick it arrives.
inline constexpr float ULDUAR_HODIR_SAFE_AREA_TOLERANCE = 6.0f;
inline constexpr float ULDUAR_HODIR_SAFE_AREA_RELEASE = 8.0f;
static_assert(ULDUAR_HODIR_SAFE_AREA_RELEASE < ULDUAR_HODIR_SAFE_AREA_RADIUS,
              "the release ring has to stay inside what Safe Area actually covers");
static_assert(ULDUAR_HODIR_SAFE_AREA_TOLERANCE < ULDUAR_HODIR_SAFE_AREA_RELEASE,
              "the park ring has to sit inside the release ring or arriving releases the bot");

// Storm Power is a 3 yd pulse at the carrier's own feet (cast on a null target), so the raid gathers
// on a carrier that holds still.
inline constexpr float ULDUAR_HODIR_STORM_CLOUD_STACK_RADIUS = 3.0f;

// Receiver park and release, 2 yd apart like the shelter run so arriving doesn't release the bot.
// Park sits a yard inside the pulse, about what a bot covers in a quarter second.
inline constexpr float ULDUAR_HODIR_STORM_CLOUD_COLLECT_PARK = 2.0f;
inline constexpr float ULDUAR_HODIR_STORM_CLOUD_COLLECT_RELEASE = 4.0f;
static_assert(ULDUAR_HODIR_STORM_CLOUD_COLLECT_PARK < ULDUAR_HODIR_STORM_CLOUD_STACK_RADIUS,
              "a parked receiver has to be inside the pulse, not on its edge");

// Short on purpose: a melee carrier gathers the melee and a ranged one the casters, rather than
// walking half the raid across the room for six seconds of buff.
inline constexpr float ULDUAR_HODIR_STORM_CLOUD_COLLECT_LEASH = 15.0f;

// Each clear is its pool's radius plus 2 yd, since a dodging bot steps past the edge, not onto it.
inline constexpr float ULDUAR_HODIR_ICE_SHARDS_RADIUS = 4.0f;  // 62457, from the small icicle 33169
inline constexpr float ULDUAR_HODIR_ICE_SHARDS_CLEAR = 6.0f;
inline constexpr float ULDUAR_HODIR_BIG_SHARDS_RADIUS = 7.0f;  // 65370, from the drift 33173
inline constexpr float ULDUAR_HODIR_BIG_SHARDS_CLEAR = 9.0f;

// Icicles live 7000 ms but blow at 3700 (fall cast at 2000, then one 1700 ms tick), so the last 3300
// are inert.
inline constexpr uint32 ULDUAR_HODIR_ICICLE_SPENT_MS = 3300;

// Arrival tolerance for the tank hold spot, which doubles as the re-anchor threshold.
inline constexpr float ULDUAR_HODIR_TANK_HOLD_TOLERANCE = 3.0f;

// Book range is 30, but it's measured to his bounding radius and caster dps holds to 35. Starlight
// stands are tested against this too: one that can't reach him is worth nothing.
inline constexpr float ULDUAR_HODIR_CASTER_MAX_BOSS_GAP = 35.0f;

inline constexpr float ULDUAR_HODIR_DODGE_LEASH = 12.0f;
inline constexpr float ULDUAR_HODIR_DECLUMP_RADIUS = 4.5f;

// Ring and bearing steps of the hazard sweep behind the dodge and the caster stand.
inline constexpr float ULDUAR_HODIR_SWEEP_STEP = 2.0f;
inline constexpr float ULDUAR_HODIR_SWEEP_ANGLE_STEP = static_cast<float>(M_PI) / 8.0f;

// The dodge leaves on the lethal radius plus this and lands on the clear. Triggering on the clear too
// has bots fleeing pools they were never in.
inline constexpr float ULDUAR_HODIR_DODGE_TRIGGER_MARGIN = 0.5f;

// ARRIVE stays well under the dodge sweep's 2 yd step so a bot can't "arrive" still inside the trigger
// radius. Slipping SLIP back from its closest approach means something else is steering, so it re-issues.
inline constexpr float ULDUAR_HODIR_DODGE_ARRIVE = 0.8f;
inline constexpr float ULDUAR_HODIR_DODGE_SLIP = 1.0f;

// Short of the room on purpose: the sweep runs per bot per tick over every world object in range.
inline constexpr float ULDUAR_HODIR_STARLIGHT_SEARCH_RADIUS = 30.0f;

// Stand this far from the zone centre, on the bearing the bot came in on, so bots sharing a zone
// spread out. Trim this, never the tolerance: tolerance also stands the position trigger down.
inline constexpr float ULDUAR_HODIR_STARLIGHT_STAND_RADIUS = 1.5f;
inline constexpr float ULDUAR_HODIR_STARLIGHT_STAND_TOLERANCE = 1.0f;

// How close two sweeps have to agree before the second is the same zone as the first. Starlight zones
// are static dynamic objects, so this only has to absorb float noise, not movement.
inline constexpr float ULDUAR_HODIR_STARLIGHT_ZONE_MATCH = 1.0f;
static_assert(ULDUAR_HODIR_STARLIGHT_STAND_RADIUS + ULDUAR_HODIR_STARLIGHT_STAND_TOLERANCE <=
                  ULDUAR_HODIR_STARLIGHT_RADIUS,
              "a bot at the edge of its tolerance has to still be inside Starlight");

// How long a latched Starlight stand may sit outside the band before it's dropped. Covers a yard of
// wobble at the edge, not Hodir moving between the centre and a fire.
inline constexpr uint32 ULDUAR_HODIR_STARLIGHT_REJECT_DROP_MS = 3000;

// Shed shuttle ends in Starlight, a yard inside the edge so the shuttle keeps the buff. One 4 yd leg
// can't span two ticks, but the legs chain until the aura's gone.
inline constexpr float ULDUAR_HODIR_STARLIGHT_SHED_RADIUS = 2.0f;
static_assert(ULDUAR_HODIR_STARLIGHT_SHED_RADIUS < ULDUAR_HODIR_STARLIGHT_RADIUS,
              "both ends of the shed shuttle have to stay inside Starlight");

// Park 8, release 10, inside the fire's 11 yd with the same 2 yd hysteresis as the other runs.
inline constexpr float ULDUAR_HODIR_FIRE_STAND_RADIUS = 8.0f;
inline constexpr float ULDUAR_HODIR_FIRE_STAND_TOLERANCE = 2.0f;
static_assert(ULDUAR_HODIR_FIRE_STAND_RADIUS + ULDUAR_HODIR_FIRE_STAND_TOLERANCE <
                  ULDUAR_HODIR_TOASTY_FIRE_RADIUS,
              "a bot at the edge of its tolerance has to still be inside the fire");

// A bot already in Starlight or a fire gets the short walk, since it'd give one up to chase another.
// One with neither only loses the walk, and with Hodir on a fire most legal zones sit out on the helper arc.
inline constexpr float ULDUAR_HODIR_BUFF_WALK = 15.0f;
inline constexpr float ULDUAR_HODIR_BUFF_WALK_UNBUFFED = 30.0f;
static_assert(ULDUAR_HODIR_BUFF_WALK_UNBUFFED <= ULDUAR_HODIR_STARLIGHT_SEARCH_RADIUS,
              "walking further than the zone sweep looks would find nothing to walk to");

// Keeps casters off the melee pack, so an icicle or a Freeze on one misses the other, and out of his
// melee range, where aggro pulls at 110% of the tank's threat instead of 130%.
inline constexpr float ULDUAR_HODIR_RANGED_MIN_BOSS_GAP = 15.0f;
static_assert(ULDUAR_HODIR_RANGED_MIN_BOSS_GAP < ULDUAR_HODIR_CASTER_MAX_BOSS_GAP,
              "the caster band has to have room between its ends");

// +50% haste is worth standing near the melee for, but still outside his 7.8 yd melee range (combat
// reach 5, a raider's 1.5, plus 4/3).
inline constexpr float ULDUAR_HODIR_STARLIGHT_MIN_BOSS_GAP = 10.0f;
static_assert(ULDUAR_HODIR_STARLIGHT_MIN_BOSS_GAP <= ULDUAR_HODIR_RANGED_MIN_BOSS_GAP,
              "Starlight is the one stand allowed inside the caster gap, never one kept further out");

// He stops about 4 yd short of whoever holds him, so the tank stands this far past the point.
inline constexpr float ULDUAR_HODIR_HOLD_TANK_OFFSET = 4.5f;

// Measured from ULDUAR_HODIR_CENTRE, not from him, so he never ends up further out than this and every
// tank spot stays 40+ yd inside the y band he evades outside.
inline constexpr float ULDUAR_HODIR_HOLD_FIRE_LEASH = 25.0f;

// Closer than this and a bearing from Hodir to the point means nothing, so the tank takes the fixed
// outward one instead.
inline constexpr float ULDUAR_HODIR_HOLD_BEARING_MIN_GAP = 3.0f;

// He only walks toward his holder, so once shoved past the spot he parks with it in reach. He normally
// stops within 1-2 yd of the point, so 5 only catches that parked case.
inline constexpr float ULDUAR_HODIR_HOLD_REAIM_GAP = 5.0f;
inline constexpr uint32 ULDUAR_HODIR_HOLD_REAIM_MS = 2000;

// A Biting Cold stack only comes off on the second moving tick in a row, so shedding needs over a second
// of unbroken travel: 6 yd legs, chained. A jump in place can't fake it, MoveJump is airborne ~1.4 ms.
inline constexpr float ULDUAR_HODIR_SHUTTLE_HALF_LEG = 3.0f;
inline constexpr uint32 ULDUAR_HODIR_BITING_COLD_SHED_STACKS = 4;

// Standing in Starlight is worth a stack: +50% to every cast and swing against what the tick costs.
inline constexpr uint32 ULDUAR_HODIR_BITING_COLD_SHED_STACKS_IN_STARLIGHT = 5;
static_assert(ULDUAR_HODIR_BITING_COLD_SHED_STACKS_IN_STARLIGHT <= 6,
              "62039 caps at 8 stacks and 200*2^7 is 25600 a tick, so past 6 the shed arms too late to matter");

// Where a shed stops. The last stack ticks 400 and takes 2s of walking to drop, and standing 4s puts
// it straight back on.
inline constexpr uint32 ULDUAR_HODIR_BITING_COLD_SHED_FLOOR = 1;
static_assert(ULDUAR_HODIR_BITING_COLD_SHED_FLOOR < ULDUAR_HODIR_BITING_COLD_SHED_STACKS,
              "a shed has to have somewhere to go");

// Legs for a bot shedding inside a landed shelter while Flash Freeze is being cast. Both ends stay
// inside the park ring, so the shelter run never fires on them.
inline constexpr float ULDUAR_HODIR_SHELTER_SHED_RADIUS = 3.0f;
static_assert(ULDUAR_HODIR_SHELTER_SHED_RADIUS < ULDUAR_HODIR_SAFE_AREA_TOLERANCE,
              "the shelter shuttle has to stay inside the park ring");

// MoveTo refuses the same point for MaxWaitForMove (5 s) even after the bot stops, so a walk cut short
// by an item use or a Disengage is re-issued once the bot has stood still this long.
inline constexpr uint32 ULDUAR_HODIR_STALL_MS = 500;

// A trapped raider dies to the next Flash Freeze 48s later, so freeing them outranks the boss, but the
// block has little health, so only a few bots in range leave what they were doing.
inline constexpr float ULDUAR_HODIR_TRAPPED_ALLY_RANGE = 45.0f;
inline constexpr uint32 ULDUAR_HODIR_TRAPPED_ALLY_BREAKERS = 5;
// How much closer another trapped raider has to be before a breaker switches to it.
inline constexpr float ULDUAR_HODIR_TRAPPED_ALLY_SWITCH_MARGIN = 10.0f;

// Breaker ceiling across every helper block that's up. One Flash Freeze ices all eight helpers at
// once, so a per-block cap would multiply.
inline constexpr uint32 ULDUAR_HODIR_HELPER_BLOCK_BREAKERS = 8;

// Breakers per mage block, one on every other kind. The block has no mechanic, so how fast it breaks
// scales with how many shoot it, and the next fire has to land before Singed's 25 s runs out.
inline constexpr uint32 ULDUAR_HODIR_MAGE_BLOCK_BREAKERS = 5;

// The ceiling once every mage is free: the fire is what the wave was for, and the other helpers aren't
// worth the whole ranged group.
inline constexpr uint32 ULDUAR_HODIR_LATE_BLOCK_BREAKERS = 3;
static_assert(ULDUAR_HODIR_LATE_BLOCK_BREAKERS <= ULDUAR_HODIR_HELPER_BLOCK_BREAKERS,
              "the wave standing down cannot pull in more bots than it had while a mage was iced");

// Never empty the ranged group for ice. Sized off who is actually there rather than off raid size,
// so eight blocks against a 10-man's three ranged still leaves someone on the boss.
inline constexpr uint32 ULDUAR_HODIR_HELPER_BLOCK_MIN_FREE = 2;

// Below this he dies before freeing a helper pays: a freed mage's fire is ~15 s out at best, the last
// Singed runs 25 s anyway, and 12% is ~21.6 s at the cache's 216.5k dps pace.
inline constexpr float ULDUAR_HODIR_LAST_WAVE_HEALTH_PCT = 12.0f;

// A max Frozen Blows swing takes about two thirds of a tank's health and two land close together, so
// below this a taunt into an open window is a death.
inline constexpr float ULDUAR_HODIR_TAUNT_HEALTH_FLOOR = 50.0f;

// Lust waits for the first Toasty Fire, but no longer than this into the pull, past where first fires
// normally land. A 10 minute cooldown shouldn't sit idle.
inline constexpr uint32 ULDUAR_HODIR_LUST_FALLBACK_MS = 30000;

inline constexpr float ULDUAR_HODIR_ROOM_SEARCH_RADIUS = 100.0f;

extern const Position ULDUAR_HODIR_CENTRE;

// Hodir. By entry, not "find target": that value walks only the bot's own threat list, so any bot
// parked on an ice block would stop seeing the boss and silently lose its Flash Freeze shelter.
Unit* GetHodir(PlayerbotAI* botAI);

// Anything that pulls (targeting, the anchors) waits for this instead of mere presence. Hodir never
// calls SetInCombatWithZone, so his flag flips exactly when someone engages him.
bool IsHodirEngaged(PlayerbotAI* botAI);

// True for the whole Flash Freeze cast (61968, 9 s), from before the drift lands until the cast ends.
bool IsHodirFlashFreezeIncoming(PlayerbotAI* botAI);

// True while Heroism and Bloodlust wait: he's engaged, no Toasty Fire is up yet, and the pull is under
// ULDUAR_HODIR_LUST_FALLBACK_MS old.
bool IsHodirLustHeld(PlayerbotAI* botAI);

// True while Hodir carries Frozen Blows, one helper so the swap trigger and taunt guard agree. Keep the
// difficulty lookup: only the server's spelldifficulty_dbc maps 62478 to 63512 for 25man.
bool HodirFrozenBlowsActive(PlayerbotAI* botAI, Player* bot);

// True when a taunt now would kill this bot: Frozen Blows up, under ULDUAR_HODIR_TAUNT_HEALTH_FLOOR and
// someone else holding him. With nobody on him the taunt is the save, so it's allowed.
bool HodirTauntWouldBeSuicide(PlayerbotAI* botAI, Player* bot);

// Nearest Snowpacked Icicle Target, or during the cast the nearest drift still falling, since the
// target spawns on the drift's spot. Latched per bot until the pick leaves the floor.
Creature* GetHodirShelter(PlayerbotAI* botAI, Player* bot);

// Shelter run park and release radii. A drift still falling is a 14000 blast, so the bot parks at its
// clear and steps in once it lands, keeping the same 2 yd between park and release.
float GetHodirShelterPark(Creature* shelter);
float GetHodirShelterRelease(Creature* shelter);

// True when the bot's shelter is a landed Snowpacked Icicle Target, not a drift still falling, and the
// bot stands inside its park ring.
bool IsHodirInLandedShelter(PlayerbotAI* botAI, Player* bot);

// The bot carrying Storm Cloud, or nullptr. Read off the group, not the grid, so a far carrier still
// counts, and first in roster order so every bot agrees when two carry at once.
Player* GetHodirStormCloudCarrier(PlayerbotAI* botAI, Player* bot);

// The one point a carry gathers on, for carrier and receivers alike. Latched per instance on the
// carrier's aura apply time, since the carrier's position seeds it and keeps moving.
bool GetHodirStormCloudRally(PlayerbotAI* botAI, Player* bot, Player* carrier, Position& out);

// Tanks get a spot just past the hold point, ranged and healers a buff stand within walking reach if
// there is one, melee nothing. Trigger and action both call this so they can't disagree.
bool GetHodirAnchor(PlayerbotAI* botAI, Player* bot, Position& out, float& tolerance);

// The Starlight zone this bot is standing in, or false. The centre, not the bot's own spot, so the
// shuttle can be laid out around it.
bool GetHodirStarlightZoneAt(PlayerbotAI* botAI, Player* bot, Position& out);

// Tanks shuttle square to the line he's held on, so neither end drags him off it. A bot in a landed
// shelter or in Starlight shuttles across it, the rest take the nearest leg clear of raid and icicles.
bool GetHodirShuttleLeg(PlayerbotAI* botAI, Player* bot, Position& out);

// True when the shed would start walking a bot standing still out of a fire. Nodes that step aside for
// the shed ask this, not HodirBitingColdTrigger, which fires on any stack since the action owns the latch.
bool IsHodirBitingColdShedArmed(Player* bot);

// False once the icicle has blown. It lingers another 3.3 s, and counting it as live sends bots back
// and forth over a spot that's already safe.
bool IsHodirIcicleLethal(Creature* icicle);

// True when the straight walk to dest, both ends included, passes within smallRadius of a small icicle
// or bigRadius of a live drift. Movers ask first, since walking back into a pool re-arms the dodge.
bool IsHodirWalkThroughLiveIcicle(Player* bot, Position const& dest, float smallRadius, float bigRadius);

// Every icicle not yet blown, each with its own clear, drifts included until they land. The clears are
// parameters because the dodge sweeps twice, once with margin and once at the lethal radius.
std::vector<EncounterHelpers::HazardCircle> CollectHodirIcicleHazards(Player* bot, float radius, float smallClear,
                                                                     float bigClear);

// Tie-break between equally short dodge spots: melee lean toward the boss, ranged and healers toward
// the buff stand they were pulled off. No anchor, no preference.
bool GetHodirDodgePreference(PlayerbotAI* botAI, Player* bot, Position& out);

// The paladin on Frost Resistance Aura for this fight, or nullptr: a tank first, since tanks never
// leave the hold and take the frost that kills, then any non-healer, then whoever's left.
Player* GetHodirResistancePaladin(PlayerbotAI* botAI, Player* bot);

// True when this bot is one of the ULDUAR_HODIR_TRAPPED_ALLY_BREAKERS dps in range, ranked by guid.
// By distance the set would reshuffle every tick and bots would flick between block and boss.
bool IsHodirTrappedAllyBreaker(PlayerbotAI* botAI, Player* bot, Unit* block);

// The helper block this bot should break, or nullptr. Ranged only if there are any, mages first, one
// budget over every block, none under ULDUAR_HODIR_LAST_WAVE_HEALTH_PCT.
Unit* GetHodirAssignedHelperBlock(PlayerbotAI* botAI, Player* bot);

#endif
