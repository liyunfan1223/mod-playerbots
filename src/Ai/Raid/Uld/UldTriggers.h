/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_H
#define PLAYERBOTS_ULDTRIGGERS_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"
#include "UldBossHelper.h"

//
// Flame Levi
//
class FlameLeviathanOnVehicleTrigger : public Trigger
{
public:
    FlameLeviathanOnVehicleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "flame leviathan on vehicle") {}
    bool IsActive() override;
};

class FlameLeviathanVehicleNearTrigger : public Trigger
{
public:
    FlameLeviathanVehicleNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "flame leviathan vehicle near") {}
    bool IsActive() override;
};

//
// Razorscale
//
class RazorscaleFlyingAloneTrigger : public Trigger
{
public:
    RazorscaleFlyingAloneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale flying alone") {}
    bool IsActive() override;
};

class RazorscaleDevouringFlamesTrigger : public Trigger
{
public:
    RazorscaleDevouringFlamesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale avoid devouring flames") {}
    bool IsActive() override;
};

class RazorscaleAvoidSentinelTrigger : public Trigger
{
public:
    RazorscaleAvoidSentinelTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale avoid sentinel") {}
    bool IsActive() override;
};

class RazorscaleAvoidWhirlwindTrigger : public Trigger
{
public:
    RazorscaleAvoidWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale avoid whirlwind") {}
    bool IsActive() override;
};

class RazorscaleGroundedTrigger : public Trigger
{
public:
    RazorscaleGroundedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale grounded") {}
    bool IsActive() override;
};

class RazorscaleHarpoonAvailableTrigger : public Trigger
{
public:
    RazorscaleHarpoonAvailableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale harpoon trigger") {}
    bool IsActive() override;
};

class RazorscaleFuseArmorTrigger : public Trigger
{
public:
    RazorscaleFuseArmorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorscale fuse armor trigger") {}
    bool IsActive() override;
};

//
// Iron Assembly
//
class IronAssemblyLightningTendrilsTrigger : public Trigger
{
public:
    IronAssemblyLightningTendrilsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "iron assembly lightning tendrils trigger") {}
    bool IsActive() override;
};

class IronAssemblyOverloadTrigger : public Trigger
{
public:
    IronAssemblyOverloadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "iron assembly overload trigger") {}
    bool IsActive() override;
};

class IronAssemblyRuneOfPowerTrigger : public Trigger
{
public:
    IronAssemblyRuneOfPowerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "iron assembly rune of power trigger") {}
    bool IsActive() override;
};

//
// Kologarn
//
class KologarnMarkDpsTargetTrigger : public Trigger
{
public:
    KologarnMarkDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn mark dps target trigger") {}
    bool IsActive() override;
};

class KologarnFallFromFloorTrigger : public Trigger
{
public:
    KologarnFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn fall from floor trigger") {}
    bool IsActive() override;
};

class KologarnRubbleSlowdownTrigger : public Trigger
{
public:
    KologarnRubbleSlowdownTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn rubble slowdown trigger") {}
    bool IsActive() override;
};

class KologarnEyebeamTrigger : public Trigger
{
public:
    KologarnEyebeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn eyebeam trigger") {}
    bool IsActive() override;
};

class KologarnAttackDpsTargetTrigger : public Trigger
{
public:
    KologarnAttackDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn attack dps target trigger") {}
    bool IsActive() override;
};

class KologarnRtiTargetTrigger : public Trigger
{
public:
    KologarnRtiTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn rti target trigger") {}
    bool IsActive() override;
};

class KologarnCrunchArmorTrigger : public Trigger
{
public:
    KologarnCrunchArmorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "kologarn crunch armor trigger") {}
    bool IsActive() override;
};

//
// Auriaya
//
class AuriayaFallFromFloorTrigger : public Trigger
{
public:
    AuriayaFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "auriaya fall from floor trigger") {}
    bool IsActive() override;
};

//
// Hodir
//
class HodirBitingColdTrigger : public Trigger
{
public:
    HodirBitingColdTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hodir biting cold") {}
    bool IsActive() override;
};

class HodirNearSnowpackedIcicleTrigger : public Trigger
{
public:
    HodirNearSnowpackedIcicleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hodir near snowpacked icicle") {}
    bool IsActive() override;
};

//
// Freya
//
class FreyaNearNatureBombTrigger : public Trigger
{
public:
    FreyaNearNatureBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "freya near nature bomb") {}
    bool IsActive() override;
};

class FreyaMarkDpsTargetTrigger : public Trigger
{
public:
    FreyaMarkDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "freya mark dps target trigger") {}
    bool IsActive() override;
};

class FreyaMoveToHealingSporeTrigger : public Trigger
{
public:
    FreyaMoveToHealingSporeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "freya move to healing spore trigger") {}
    bool IsActive() override;
};

//
// Thorim
//
class ThorimUnbalancingStrikeTrigger : public Trigger
{
public:
    ThorimUnbalancingStrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorim unbalancing strike trigger") {}
    bool IsActive() override;
};

class ThorimMarkDpsTargetTrigger : public Trigger
{
public:
    ThorimMarkDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorim mark dps target trigger") {}
    bool IsActive() override;
};

class ThorimGauntletPositioningTrigger : public Trigger
{
public:
    ThorimGauntletPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorim gauntlet positioning trigger") {}
    bool IsActive() override;
};

class ThorimArenaPositioningTrigger : public Trigger
{
public:
    ThorimArenaPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorim arena positioning trigger") {}
    bool IsActive() override;
};

class ThorimFallFromFloorTrigger : public Trigger
{
public:
    ThorimFallFromFloorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorim fall from floor trigger") {}
    bool IsActive() override;
};

class ThorimPhase2PositioningTrigger : public Trigger
{
public:
    ThorimPhase2PositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "thorim phase 2 positioning trigger") {}
    bool IsActive() override;
};

//
// Mimiron
//
class MimironShockBlastTrigger : public Trigger
{
public:
    MimironShockBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron shock blast trigger") {}
    bool IsActive() override;
};

class MimironPhase1PositioningTrigger : public Trigger
{
public:
    MimironPhase1PositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron phase 1 positioning trigger") {}
    bool IsActive() override;
};

class MimironP3Wx2LaserBarrageTrigger : public Trigger
{
public:
    MimironP3Wx2LaserBarrageTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron p3wx2 laser barrage trigger") {}
    bool IsActive() override;
};

class MimironRapidBurstTrigger : public Trigger
{
public:
    MimironRapidBurstTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron rapid burst trigger") {}
    bool IsActive() override;
};

class MimironAerialCommandUnitTrigger : public Trigger
{
public:
    MimironAerialCommandUnitTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron aerial command unit trigger") {}
    bool IsActive() override;
};

class MimironRocketStrikeTrigger : public Trigger
{
public:
    MimironRocketStrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron rocket strike trigger") {}
    bool IsActive() override;
};

class MimironPhase4MarkDpsTrigger : public Trigger
{
public:
    MimironPhase4MarkDpsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron phase 4 mark dps trigger") {}
    bool IsActive() override;
};

class MimironCheatTrigger : public Trigger
{
public:
    MimironCheatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mimiron cheat trigger") {}
    bool IsActive() override;
};

//
// General Vezax
//
class VezaxCheatTrigger : public Trigger
{
public:
    VezaxCheatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vezax cheat trigger") {}
    bool IsActive() override;
};

class VezaxShadowCrashTrigger : public Trigger
{
public:
    VezaxShadowCrashTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vezax shadow crash trigger") {}
    bool IsActive() override;
};

class VezaxMarkOfTheFacelessTrigger : public Trigger
{
public:
    VezaxMarkOfTheFacelessTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vezax mark of the faceless trigger") {}
    bool IsActive() override;
};

//
// Yogg-Saron
//
class YoggSaronTrigger : public Trigger
{
public:
    YoggSaronTrigger(PlayerbotAI* botAI, std::string const name = "yogg saron trigger", int32 checkInteval = 1)
        : Trigger(botAI, name, checkInteval) {}

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
    YoggSaronOminousCloudCheatTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron ominous cloud cheat trigger") {}
    bool IsActive() override;
};

class YoggSaronGuardianPositioningTrigger : public YoggSaronTrigger
{
public:
    YoggSaronGuardianPositioningTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron guardian positioning trigger") {}
    bool IsActive() override;
};

class YoggSaronSanityTrigger : public YoggSaronTrigger
{
public:
    YoggSaronSanityTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron sanity trigger") {}
    bool IsActive() override;
};

class YoggSaronDeathOrbTrigger : public YoggSaronTrigger
{
public:
    YoggSaronDeathOrbTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron death orb trigger") {}
    bool IsActive() override;
};

class YoggSaronMaladyOfTheMindTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMaladyOfTheMindTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron malady of the mind trigger") {}
    bool IsActive() override;
};

class YoggSaronMarkTargetTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMarkTargetTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron mark target trigger") {}
    bool IsActive() override;
};

class YoggSaronBrainLinkTrigger : public YoggSaronTrigger
{
public:
    YoggSaronBrainLinkTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron brain link trigger") {}
    bool IsActive() override;
};

class YoggSaronMoveToEnterPortalTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMoveToEnterPortalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron move to enter portal trigger") {}
    bool IsActive() override;
};

class YoggSaronFallFromFloorTrigger : public YoggSaronTrigger
{
public:
    YoggSaronFallFromFloorTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron fall from floor trigger") {}
    bool IsActive() override;
};

class YoggSaronBossRoomMovementCheatTrigger : public YoggSaronTrigger
{
public:
    YoggSaronBossRoomMovementCheatTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron boss room movement cheat trigger") {}
    bool IsActive() override;
};

class YoggSaronUsePortalTrigger : public YoggSaronTrigger
{
public:
    YoggSaronUsePortalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron use portal trigger") {}
    bool IsActive() override;
};

class YoggSaronIllusionRoomTrigger : public YoggSaronTrigger
{
public:
    YoggSaronIllusionRoomTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron illusion room trigger") {}
    bool IsActive() override;

private:
    bool GoToBrainRoomRequired();
    bool SetRtiMarkRequired();
    bool SetRtiTargetRequired();
};

class YoggSaronMoveToExitPortalTrigger : public YoggSaronTrigger
{
public:
    YoggSaronMoveToExitPortalTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron move to exit portal trigger") {}
    bool IsActive() override;
};

class YoggSaronLunaticGazeTrigger : public YoggSaronTrigger
{
public:
    YoggSaronLunaticGazeTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron lunatic gaze trigger") {}
    bool IsActive() override;
};

class YoggSaronPhase3PositioningTrigger : public YoggSaronTrigger
{
public:
    YoggSaronPhase3PositioningTrigger(PlayerbotAI* botAI) : YoggSaronTrigger(botAI, "yogg-saron phase 3 positioning trigger") {}
    bool IsActive() override;
};

#endif
