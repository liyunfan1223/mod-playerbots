#ifndef _PLAYERBOT_RAIDULDUARTRIGGERS_H
#define _PLAYERBOT_RAIDULDUARTRIGGERS_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "RaidUlduarBossHelper.h"
#include "Trigger.h"

enum UlduarIDs
{
    // Iron Assembly
    SPELL_LIGHTNING_TENDRILS_10_MAN = 61887,
    SPELL_LIGHTNING_TENDRILS_25_MAN = 63486,
    SPELL_OVERLOAD_10_MAN = 61869,
    SPELL_OVERLOAD_25_MAN = 63481,
    SPELL_OVERLOAD_10_MAN_2 = 63485,
    SPELL_OVERLOAD_25_MAN_2 = 61886,
    SPELL_RUNE_OF_POWER = 64320,

    // Kologarn
    NPC_RIGHT_ARM = 32934,
    NPC_RUBBLE = 33768,
    SPELL_CRUNCH_ARMOR = 64002,

    SPELL_FOCUSED_EYEBEAM_10_2 = 63346,
    SPELL_FOCUSED_EYEBEAM_10 = 63347,
    SPELL_FOCUSED_EYEBEAM_25_2 = 63976,
    SPELL_FOCUSED_EYEBEAM_25 = 63977,

    // Hodir
    NPC_SNOWPACKED_ICICLE = 33174,
    NPC_TOASTY_FIRE = 33342,
    SPELL_FLASH_FREEZE = 61968,
    SPELL_BITING_COLD_PLAYER_AURA = 62039,

    // Freya
    NPC_SNAPLASHER = 32916,
    NPC_STORM_LASHER = 32919,
    NPC_DETONATING_LASHER = 32918,
    NPC_ANCIENT_WATER_SPIRIT = 33202,
    NPC_ANCIENT_CONSERVATOR = 33203,
    NPC_HEALTHY_SPORE = 33215,
    NPC_EONARS_GIFT = 33228,
    GOBJECT_NATURE_BOMB = 194902,

    // Thorim
    NPC_DARK_RUNE_ACOLYTE_I = 32886,
    NPC_CAPTURED_MERCENARY_SOLDIER_ALLY = 32885,
    NPC_CAPTURED_MERCENARY_SOLDIER_HORDE = 32883,
    NPC_CAPTURED_MERCENARY_CAPTAIN_ALLY = 32908,
    NPC_CAPTURED_MERCENARY_CAPTAIN_HORDE = 32907,
    NPC_JORMUNGAR_BEHEMOT = 32882,
    NPC_DARK_RUNE_WARBRINGER = 32877,
    NPC_DARK_RUNE_EVOKER = 32878,
    NPC_DARK_RUNE_CHAMPION = 32876,
    NPC_DARK_RUNE_COMMONER = 32904,
    NPC_IRON_RING_GUARD = 32874,
    NPC_RUNIC_COLOSSUS = 32872,
    NPC_ANCIENT_RUNE_GIANT = 32873,
    NPC_DARK_RUNE_ACOLYTE_G = 33110,
    NPC_IRON_HONOR_GUARD = 32875,
    SPELL_UNBALANCING_STRIKE = 62130,

    // Mimiron
    NPC_LEVIATHAN_MKII = 33432,
    NPC_VX001 = 33651,
    NPC_AERIAL_COMMAND_UNIT = 33670,
    NPC_BOMB_BOT = 33836,
    NPC_ROCKET_STRIKE_N = 34047,
    NPC_ASSAULT_BOT = 34057,
    NPC_PROXIMITY_MINE = 34362,
    SPELL_P3WX2_LASER_BARRAGE_1 = 63293,
    SPELL_P3WX2_LASER_BARRAGE_2 = 63297,
    SPELL_SPINNING_UP = 63414,
    SPELL_SHOCK_BLAST = 63631,
    SPELL_P3WX2_LASER_BARRAGE_3 = 64042,
    SPELL_P3WX2_LASER_BARRAGE_AURA_1 = 63274,
    SPELL_P3WX2_LASER_BARRAGE_AURA_2 = 63300,

    // General Vezax
    SPELL_MARK_OF_THE_FACELESS = 63276,
    SPELL_SHADOW_CRASH = 63277,

    // Yogg-Saron
    ACTION_ILLUSION_DRAGONS = 1,
    ACTION_ILLUSION_ICECROWN = 2,
    ACTION_ILLUSION_STORMWIND = 3,
    NPC_GUARDIAN_OF_YS = 33136,
    NPC_YOGG_SARON = 33288,
    NPC_OMINOUS_CLOUD = 33292,
    NPC_RUBY_CONSORT = 33716,
    NPC_AZURE_CONSORT = 33717,
    NPC_BRONZE_CONSORT = 33718,
    NPC_EMERALD_CONSORT = 33719,
    NPC_OBSIDIAN_CONSORT = 33720,
    NPC_ALEXTRASZA = 33536,
    NPC_MALYGOS_ILLUSION = 33535,
    NPC_NELTHARION = 33523,
    NPC_YSERA = 33495,
    GO_DRAGON_SOUL = 194462,
    NPC_SARA_PHASE_1 = 33134,
    NPC_LICH_KING_ILLUSION = 33441,
    NPC_IMMOLATED_CHAMPION = 33442,
    NPC_SUIT_OF_ARMOR = 33433,
    NPC_GARONA = 33436,
    NPC_KING_LLANE = 33437,
    NPC_DEATHSWORN_ZEALOT = 33567,
    NPC_INFLUENCE_TENTACLE = 33943,
    NPC_DEATH_ORB = 33882,
    NPC_BRAIN = 33890,
    NPC_CRUSHER_TENTACLE = 33966,
    NPC_CONSTRICTOR_TENTACLE = 33983,
    NPC_CORRUPTOR_TENTACLE = 33985,
    NPC_IMMORTAL_GUARDIAN = 33988,
    NPC_LAUGHING_SKULL = 33990,
    NPC_SANITY_WELL = 33991,
    NPC_DESCEND_INTO_MADNESS = 34072,
    NPC_MARKED_IMMORTAL_GUARDIAN = 36064,
    SPELL_SANITY = 63050,
    SPELL_BRAIN_LINK = 63802,
    SPELL_MALADY_OF_THE_MIND = 63830,
    SPELL_SHADOW_BARRIER = 63894,
    SPELL_TELEPORT_TO_CHAMBER = 63997,
    SPELL_TELEPORT_TO_ICECROWN = 63998,
    SPELL_TELEPORT_TO_STORMWIND = 63989,
    SPELL_TELEPORT_BACK = 63992,
    SPELL_CANCEL_ILLUSION_AURA = 63993,
    SPELL_INDUCE_MADNESS = 64059,
    SPELL_LUNATIC_GAZE_YS = 64163,
    GO_FLEE_TO_THE_SURFACE_PORTAL = 194625,
    
    // Buffs
    SPELL_FROST_TRAP = 13809
};

const float ULDUAR_KOLOGARN_AXIS_Z_PATHING_ISSUE_DETECT = 420.0f;
const float ULDUAR_KOLOGARN_EYEBEAM_RADIUS = 3.0f;
const float ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD = 429.6094f;
const float ULDUAR_THORIM_AXIS_Z_PATHING_ISSUE_DETECT = 410.0f;
const float ULDUAR_AURIAYA_AXIS_Z_PATHING_ISSUE_DETECT = 410.0f;
const float ULDUAR_YOGG_SARON_BOSS_ROOM_AXIS_Z_PATHING_ISSUE_DETECT = 300.0f;
const float ULDUAR_YOGG_SARON_BRAIN_ROOM_AXIS_Z_PATHING_ISSUE_DETECT = 200.0f;
const float ULDUAR_YOGG_SARON_STORMWIND_KEEPER_RADIUS = 150.0f;
const float ULDUAR_YOGG_SARON_ICECROWN_CITADEL_RADIUS = 150.0f;
const float ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_RADIUS = 150.0f;
const float ULDUAR_YOGG_SARON_BRAIN_ROOM_RADIUS = 50.0f;

const Position ULDUAR_THORIM_NEAR_ARENA_CENTER = Position(2134.9854f, -263.11853f, 419.8465f);
const Position ULDUAR_THORIM_NEAR_ENTRANCE_POSITION = Position(2172.4355f, -258.27957f, 418.47162f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1 = Position(2237.6187f, -265.08844f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2 = Position(2237.2498f, -275.81122f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1 = Position(2236.895f, -294.62448f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1 = Position(2242.1162f, -310.15308f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2 = Position(2242.018f, -318.66003f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3 = Position(2242.1904f, -329.0533f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1 = Position(2219.5417f, -264.77167f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2 = Position(2217.446f, -275.85248f, 412.17548f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1 = Position(2217.8877f, -295.01193f, 412.13434f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1 = Position(2212.193f, -307.44992f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2 = Position(2212.1353f, -318.20795f, 412.1348f);
const Position ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3 = Position(2212.1956f, -328.0144f, 412.1348f);
const Position ULDUAR_THORIM_JUMP_END_POINT = Position(2137.8818f, -278.18942f, 419.66653f);
const Position ULDUAR_THORIM_PHASE2_TANK_SPOT = Position(2134.8572f, -287.0291f, 419.4935f);
const Position ULDUAR_THORIM_PHASE2_RANGE1_SPOT = Position(2112.8752f, -267.69305f, 419.52814f);
const Position ULDUAR_THORIM_PHASE2_RANGE2_SPOT = Position(2134.1296f, -257.3316f, 419.8462f);
const Position ULDUAR_THORIM_PHASE2_RANGE3_SPOT = Position(2156.798f, -267.57434f, 419.52722f);
const Position ULDUAR_MIMIRON_PHASE2_SIDE1RANGE_SPOT = Position(2753.708f, 2583.9617f, 364.31357f);
const Position ULDUAR_MIMIRON_PHASE2_SIDE1MELEE_SPOT = Position(2746.9792f, 2573.6716f, 364.31357f);
const Position ULDUAR_MIMIRON_PHASE2_SIDE2RANGE_SPOT = Position(2727.7224f, 2569.527f, 364.31357f);
const Position ULDUAR_MIMIRON_PHASE2_SIDE2MELEE_SPOT = Position(2739.4746f, 2569.4106f, 364.31357f);
const Position ULDUAR_MIMIRON_PHASE2_SIDE3RANGE_SPOT = Position(2754.1294f, 2553.9954f, 364.31357f);
const Position ULDUAR_MIMIRON_PHASE2_SIDE3MELEE_SPOT = Position(2746.8513f, 2565.4263f, 364.31357f);
const Position ULDUAR_MIMIRON_PHASE4_TANK_SPOT = Position(2744.5754f, 2570.8657f, 364.3138f);
const Position ULDUAR_VEZAX_MARK_OF_THE_FACELESS_SPOT = Position(1913.6501f, 122.93989f, 342.38083f);
const Position ULDUAR_YOGG_SARON_MIDDLE = Position(1980.28f, -25.5868f, 329.397f);
const Position ULDUAR_YOGG_SARON_STORMWIND_KEEPER_MIDDLE = Position(1927.1511f, 68.507256f, 242.37657f);
const Position ULDUAR_YOGG_SARON_ICECROWN_CITADEL_MIDDLE = Position(1925.6553f, -121.59296f, 239.98965f);
const Position ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_MIDDLE = Position(2104.5667f, -25.509348f, 242.64679f);
const Position ULDUAR_YOGG_SARON_BRAIN_ROOM_MIDDLE = Position(1980.1971f, -27.854689f, 236.06789f);
const Position ULDUAR_YOGG_SARON_STORMWIND_KEEPER_ENTRANCE = Position(1954.06f, 21.66f, 239.71f);
const Position ULDUAR_YOGG_SARON_ICECROWN_CITADEL_ENTRANCE = Position(1950.11f, -79.284f, 239.98982f);
const Position ULDUAR_YOGG_SARON_CHAMBER_OF_ASPECTS_ENTRANCE = Position(2048.63f, -25.5f, 239.72f);
const Position ULDUAR_YOGG_SARON_PHASE_3_MELEE_SPOT = Position(1998.5377f, -22.90317f, 324.8895f);
const Position ULDUAR_YOGG_SARON_PHASE_3_RANGED_SPOT = Position(2018.7628f, -18.896868f, 327.07245f);

//
// Flame Levi
//
class FlameLeviathanOnVehicleTrigger : public Trigger
{
public:
    FlameLeviathanOnVehicleTrigger(PlayerbotAI* ai) : Trigger(ai, "flame leviathan on vehicle") {}
    bool IsActive() override;
};

class FlameLeviathanVehicleNearTrigger : public Trigger
{
public:
    FlameLeviathanVehicleNearTrigger(PlayerbotAI* ai) : Trigger(ai, "flame leviathan vehicle near") {}
    bool IsActive() override;
};

//
// Razorscale
//
class RazorscaleFlyingAloneTrigger : public Trigger
{
public:
    RazorscaleFlyingAloneTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale flying alone") {}
    bool IsActive() override;
};

class RazorscaleDevouringFlamesTrigger : public Trigger
{
public:
    RazorscaleDevouringFlamesTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale avoid devouring flames") {}
    bool IsActive() override;
};

class RazorscaleAvoidSentinelTrigger : public Trigger
{
public:
    RazorscaleAvoidSentinelTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale avoid sentinel") {}
    bool IsActive() override;
};

class RazorscaleAvoidWhirlwindTrigger : public Trigger
{
public:
    RazorscaleAvoidWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale avoid whirlwind") {}
    bool IsActive() override;
};

class RazorscaleGroundedTrigger : public Trigger
{
public:
    RazorscaleGroundedTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale grounded") {}
    bool IsActive() override;
};

class RazorscaleHarpoonAvailableTrigger : public Trigger
{
public:
    RazorscaleHarpoonAvailableTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale harpoon trigger") {}
    bool IsActive() override;
};

class RazorscaleFuseArmorTrigger : public Trigger
{
public:
    RazorscaleFuseArmorTrigger(PlayerbotAI* ai) : Trigger(ai, "razorscale fuse armor trigger") {}
    bool IsActive() override;
};

//
// Iron Assembly
//
class IronAssemblyLightningTendrilsTrigger : public Trigger
{
public:
    IronAssemblyLightningTendrilsTrigger(PlayerbotAI* ai) : Trigger(ai, "iron assembly lightning tendrils trigger") {}
    bool IsActive() override;
};

class IronAssemblyOverloadTrigger : public Trigger
{
public:
    IronAssemblyOverloadTrigger(PlayerbotAI* ai) : Trigger(ai, "iron assembly overload trigger") {}
    bool IsActive() override;
};

class IronAssemblyRuneOfPowerTrigger : public Trigger
{
public:
    IronAssemblyRuneOfPowerTrigger(PlayerbotAI* ai) : Trigger(ai, "iron assembly rune of power trigger") {}
    bool IsActive() override;
};

//
// Kologarn
//
class KologarnMarkDpsTargetTrigger : public Trigger
{
public:
    KologarnMarkDpsTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn mark dps target trigger") {}
    bool IsActive() override;
};

class KologarnFallFromFloorTrigger : public Trigger
{
public:
    KologarnFallFromFloorTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn fall from floor trigger") {}
    bool IsActive() override;
};

class KologarnRubbleSlowdownTrigger : public Trigger
{
public:
    KologarnRubbleSlowdownTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn rubble slowdown trigger") {}
    bool IsActive() override;
};

class KologarnEyebeamTrigger : public Trigger
{
public:
    KologarnEyebeamTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn eyebeam trigger") {}
    bool IsActive() override;
};

class KologarnAttackDpsTargetTrigger : public Trigger
{
public:
    KologarnAttackDpsTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn attack dps target trigger") {}
    bool IsActive() override;
};

class KologarnRtiTargetTrigger : public Trigger
{
public:
    KologarnRtiTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn rti target trigger") {}
    bool IsActive() override;
};

class KologarnCrunchArmorTrigger : public Trigger
{
public:
    KologarnCrunchArmorTrigger(PlayerbotAI* ai) : Trigger(ai, "kologarn crunch armor trigger") {}
    bool IsActive() override;
};

//
// Auriaya
//
class AuriayaFallFromFloorTrigger : public Trigger
{
public:
    AuriayaFallFromFloorTrigger(PlayerbotAI* ai) : Trigger(ai, "auriaya fall from floor trigger") {}
    bool IsActive() override;
};

//
// Hodir
//
class HodirBitingColdTrigger : public Trigger
{
public:
    HodirBitingColdTrigger(PlayerbotAI* ai) : Trigger(ai, "hodir biting cold") {}
    bool IsActive() override;
};

class HodirNearSnowpackedIcicleTrigger : public Trigger
{
public:
    HodirNearSnowpackedIcicleTrigger(PlayerbotAI* ai) : Trigger(ai, "hodir near snowpacked icicle") {}
    bool IsActive() override;
};

//
// Freya
//
class FreyaNearNatureBombTrigger : public Trigger
{
public:
    FreyaNearNatureBombTrigger(PlayerbotAI* ai) : Trigger(ai, "freya near nature bomb") {}
    bool IsActive() override;
};

class FreyaMarkDpsTargetTrigger : public Trigger
{
public:
    FreyaMarkDpsTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "freya mark dps target trigger") {}
    bool IsActive() override;
};

class FreyaMoveToHealingSporeTrigger : public Trigger
{
public:
    FreyaMoveToHealingSporeTrigger(PlayerbotAI* ai) : Trigger(ai, "freya move to healing spore trigger") {}
    bool IsActive() override;
};

//
// Thorim
//
class ThorimUnbalancingStrikeTrigger : public Trigger
{
public:
    ThorimUnbalancingStrikeTrigger(PlayerbotAI* ai) : Trigger(ai, "thorim unbalancing strike trigger") {}
    bool IsActive() override;
};

class ThorimMarkDpsTargetTrigger : public Trigger
{
public:
    ThorimMarkDpsTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "thorim mark dps target trigger") {}
    bool IsActive() override;
};

class ThorimGauntletPositioningTrigger : public Trigger
{
public:
    ThorimGauntletPositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "thorim gauntlet positioning trigger") {}
    bool IsActive() override;
};

class ThorimArenaPositioningTrigger : public Trigger
{
public:
    ThorimArenaPositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "thorim arena positioning trigger") {}
    bool IsActive() override;
};

class ThorimFallFromFloorTrigger : public Trigger
{
public:
    ThorimFallFromFloorTrigger(PlayerbotAI* ai) : Trigger(ai, "thorim fall from floor trigger") {}
    bool IsActive() override;
};

class ThorimPhase2PositioningTrigger : public Trigger
{
public:
    ThorimPhase2PositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "thorim phase 2 positioning trigger") {}
    bool IsActive() override;
};

//
// Mimiron
//
class MimironShockBlastTrigger : public Trigger
{
public:
    MimironShockBlastTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron shock blast trigger") {}
    bool IsActive() override;
};

class MimironPhase1PositioningTrigger : public Trigger
{
public:
    MimironPhase1PositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron phase 1 positioning trigger") {}
    bool IsActive() override;
};

class MimironP3Wx2LaserBarrageTrigger : public Trigger
{
public:
    MimironP3Wx2LaserBarrageTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron p3wx2 laser barrage trigger") {}
    bool IsActive() override;
};

class MimironRapidBurstTrigger : public Trigger
{
public:
    MimironRapidBurstTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron rapid burst trigger") {}
    bool IsActive() override;
};

class MimironAerialCommandUnitTrigger : public Trigger
{
public:
    MimironAerialCommandUnitTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron aerial command unit trigger") {}
    bool IsActive() override;
};

class MimironRocketStrikeTrigger : public Trigger
{
public:
    MimironRocketStrikeTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron rocket strike trigger") {}
    bool IsActive() override;
};

class MimironPhase4MarkDpsTrigger : public Trigger
{
public:
    MimironPhase4MarkDpsTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron phase 4 mark dps trigger") {}
    bool IsActive() override;
};

class MimironCheatTrigger : public Trigger
{
public:
    MimironCheatTrigger(PlayerbotAI* ai) : Trigger(ai, "mimiron cheat trigger") {}
    bool IsActive() override;
};

//
// General Vezax
//
class VezaxCheatTrigger : public Trigger
{
public:
    VezaxCheatTrigger(PlayerbotAI* ai) : Trigger(ai, "vezax cheat trigger") {}
    bool IsActive() override;
};

class VezaxShadowCrashTrigger : public Trigger
{
public:
    VezaxShadowCrashTrigger(PlayerbotAI* ai) : Trigger(ai, "vezax shadow crash trigger") {}
    bool IsActive() override;
};

class VezaxMarkOfTheFacelessTrigger : public Trigger
{
public:
    VezaxMarkOfTheFacelessTrigger(PlayerbotAI* ai) : Trigger(ai, "vezax mark of the faceless trigger") {}
    bool IsActive() override;
};

//
// Yogg-Saron
//
class YoggSaronTrigger : public Trigger
{
public:
    YoggSaronTrigger(PlayerbotAI* ai, std::string const name = "yogg saron trigger", int32 checkInteval = 1)
        : Trigger(ai, name, checkInteval) {}

    bool IsYoggSaronFight();
    bool IsPhase2();
    bool IsPhase3();
    bool IsInBrainLevel();
    bool IsInIllusionRoom();
    bool IsInStormwindKeeperIllusion();
    bool IsInIcecrownKeeperIllusion();
    bool IsInChamberOfTheAspectsIllusion();
    bool IsMasterIsInIllusionGroup();
    bool IsMasterIsInBrainRoom();
    Position GetIllusionRoomEntrancePosition();
    Unit* GetIllusionRoomRtiTarget();
    Unit* GetNextIllusionRoomRtiTarget();
    Unit* GetSaraIfAlive();
};

class YoggSaronOminousCloudCheatTrigger : public YoggSaronTrigger
{
public:
    YoggSaronOminousCloudCheatTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron ominous cloud cheat trigger") {}
    bool IsActive() override;
};

class YoggSaronGuardianPositioningTrigger : public YoggSaronTrigger
{
public:
    YoggSaronGuardianPositioningTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron guardian positioning trigger") {}
    bool IsActive() override;
};

class YoggSaronSanityTrigger : public YoggSaronTrigger
{
public:
    YoggSaronSanityTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron sanity trigger") {}
    bool IsActive() override;
};

class YoggSaronDeathOrbTrigger : public YoggSaronTrigger
{
public:
    YoggSaronDeathOrbTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron death orb trigger") {}
    bool IsActive() override;
};

class YoggSaronMaladyOfTheMindTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMaladyOfTheMindTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron malady of the mind trigger") {}
    bool IsActive() override;
};

class YoggSaronMarkTargetTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMarkTargetTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron mark target trigger") {}
    bool IsActive() override;
};

class YoggSaronBrainLinkTrigger : public YoggSaronTrigger
{
public:
    YoggSaronBrainLinkTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron brain link trigger") {}
    bool IsActive() override;
};

class YoggSaronMoveToEnterPortalTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMoveToEnterPortalTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron move to enter portal trigger") {}
    bool IsActive() override;
};

class YoggSaronFallFromFloorTrigger : public YoggSaronTrigger
{
public:
    YoggSaronFallFromFloorTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron fall from floor trigger") {}
    bool IsActive() override;
};

class YoggSaronBossRoomMovementCheatTrigger : public YoggSaronTrigger
{
public:
    YoggSaronBossRoomMovementCheatTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron boss room movement cheat trigger") {}
    bool IsActive() override;
};

class YoggSaronUsePortalTrigger : public YoggSaronTrigger
{
public:
    YoggSaronUsePortalTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron use portal trigger") {}
    bool IsActive() override;
};

class YoggSaronIllusionRoomTrigger : public YoggSaronTrigger
{
public:
    YoggSaronIllusionRoomTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron illusion room trigger") {}
    bool IsActive() override;

private:
    bool GoToBrainRoomRequired();
    bool SetRtiMarkRequired();
    bool SetRtiTargetRequired();
};

class YoggSaronMoveToExitPortalTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMoveToExitPortalTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron move to exit portal trigger") {}
    bool IsActive() override;
};

class YoggSaronLunaticGazeTrigger : public YoggSaronTrigger
{
public:
    YoggSaronLunaticGazeTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron lunatic gaze trigger") {}
    bool IsActive() override;
};

class YoggSaronPhase3PositioningTrigger : public YoggSaronTrigger
{
public:
    YoggSaronPhase3PositioningTrigger(PlayerbotAI* ai) : YoggSaronTrigger(ai, "yogg-saron phase 3 positioning trigger") {}
    bool IsActive() override;
};

#endif
