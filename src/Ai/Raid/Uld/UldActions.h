/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_H
#define PLAYERBOTS_ULDACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldBossHelper.h"
#include "UldTriggers.h"
#include "Vehicle.h"

//
//  Flame Leviathan
//

class FlameLeviathanVehicleAction : public MovementAction
{
public:
    FlameLeviathanVehicleAction(PlayerbotAI* botAI) : MovementAction(botAI, "flame leviathan vehicle") {}
    bool Execute(Event event) override;

protected:
    bool MoveAvoidChasing(Unit* target);
    bool DemolisherAction(Unit* target);
    bool DemolisherTurretAction(Unit* target);
    bool SiegeEngineAction(Unit* target);
    bool SiegeEngineTurretAction(Unit* target);
    bool ChopperAction(Unit* target);
    Unit* GetAttacker();
    Unit* vehicleBase_;
    Vehicle* vehicle_;
    int avoidChaseIdx = -1;
};

class FlameLeviathanEnterVehicleAction : public MovementAction
{
public:
    FlameLeviathanEnterVehicleAction(PlayerbotAI* botAI) : MovementAction(botAI, "flame leviathan enter vehicle") {}
    bool Execute(Event event);

protected:
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
    bool ShouldEnter(Unit* vehicleBase);
    bool AllMainVehiclesOnUse();
};

//
//  Razorscale
//

class RazorscaleAvoidDevouringFlameAction : public MovementAction
{
public:
    RazorscaleAvoidDevouringFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, "razorscale avoid devouring flames") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleAvoidSentinelAction : public MovementAction
{
public:
    RazorscaleAvoidSentinelAction(PlayerbotAI* botAI) : MovementAction(botAI, "razorscale avoid sentinel") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleIgnoreBossAction : public AttackAction
{
public:
    RazorscaleIgnoreBossAction(PlayerbotAI* botAI) : AttackAction(botAI, "razorscale ignore flying alone") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleAvoidWhirlwindAction : public MovementAction
{
public:
    RazorscaleAvoidWhirlwindAction(PlayerbotAI* botAI) : MovementAction(botAI, "razorscale avoid whirlwind") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleGroundedAction : public AttackAction
{
public:
    RazorscaleGroundedAction(PlayerbotAI* botAI) : AttackAction(botAI, "razorscale grounded") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleHarpoonAction : public MovementAction
{
public:
    RazorscaleHarpoonAction(PlayerbotAI* botAI) : MovementAction(botAI, "razorscale harpoon action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class RazorscaleFuseArmorAction : public MovementAction
{
public:
    RazorscaleFuseArmorAction(PlayerbotAI* botAI) : MovementAction(botAI, "razorscale fuse armor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class HodirMoveSnowpackedIcicleAction : public MovementAction
{
public:
    HodirMoveSnowpackedIcicleAction(PlayerbotAI* botAI) : MovementAction(botAI, "hodir move snowpacked icicle") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IronAssemblyLightningTendrilsAction : public MovementAction
{
public:
    IronAssemblyLightningTendrilsAction(PlayerbotAI* botAI) : MovementAction(botAI, "iron assembly lightning tendrils action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IronAssemblyOverloadAction : public MovementAction
{
public:
    IronAssemblyOverloadAction(PlayerbotAI* botAI) : MovementAction(botAI, "iron assembly overload action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class IronAssemblyRuneOfPowerAction : public MovementAction
{
public:
    IronAssemblyRuneOfPowerAction(PlayerbotAI* botAI) : MovementAction(botAI, "iron assembly rune of power action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnMarkDpsTargetAction : public Action
{
public:
    KologarnMarkDpsTargetAction(PlayerbotAI* botAI) : Action(botAI, "kologarn mark dps target action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnFallFromFloorAction : public Action
{
public:
    KologarnFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, "kologarn fall from floor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnRubbleSlowdownAction : public Action
{
public:
    KologarnRubbleSlowdownAction(PlayerbotAI* botAI) : Action(botAI, "kologarn rubble slowdown action") {}
    bool Execute(Event event) override;
};

class KologarnEyebeamAction : public MovementAction
{
public:
    KologarnEyebeamAction(PlayerbotAI* botAI) : MovementAction(botAI, "kologarn eyebeam action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnRtiTargetAction : public Action
{
public:
    KologarnRtiTargetAction(PlayerbotAI* botAI) : Action(botAI, "kologarn rti target action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class KologarnCrunchArmorAction : public Action
{
public:
    KologarnCrunchArmorAction(PlayerbotAI* botAI) : Action(botAI, "kologarn crunch armor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AuriayaFallFromFloorAction : public Action
{
public:
    AuriayaFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, "auriaya fall from floor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class HodirBitingColdJumpAction : public MovementAction
{
public:
    HodirBitingColdJumpAction(PlayerbotAI* botAI) : MovementAction(botAI, "hodir biting cold jump") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FreyaMoveAwayNatureBombAction : public MovementAction
{
public:
    FreyaMoveAwayNatureBombAction(PlayerbotAI* botAI) : MovementAction(botAI, "freya move away nature bomb") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FreyaMarkDpsTargetAction : public MovementAction
{
public:
    FreyaMarkDpsTargetAction(PlayerbotAI* botAI) : MovementAction(botAI, "freya mark dps target action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FreyaMoveToHealingSporeAction : public MovementAction
{
public:
    FreyaMoveToHealingSporeAction(PlayerbotAI* botAI) : MovementAction(botAI, "freya move to healing spore action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimUnbalancingStrikeAction : public Action
{
public:
    ThorimUnbalancingStrikeAction(PlayerbotAI* botAI) : Action(botAI, "thorim unbalancing strike action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimMarkDpsTargetAction : public Action
{
public:
    ThorimMarkDpsTargetAction(PlayerbotAI* botAI) : Action(botAI, "thorim mark dps target action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimArenaPositioningAction : public MovementAction
{
public:
    ThorimArenaPositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, "thorim arena positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimGauntletPositioningAction : public MovementAction
{
public:
    ThorimGauntletPositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, "thorim gauntlet positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimFallFromFloorAction : public Action
{
public:
    ThorimFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, "thorim fall from floor action") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimPhase2PositioningAction : public MovementAction
{
public:
    ThorimPhase2PositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, "thorim phase 2 positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironShockBlastAction : public MovementAction
{
public:
    MimironShockBlastAction(PlayerbotAI* botAI) : MovementAction(botAI, "mimiron shock blast action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironPhase1PositioningAction : public MovementAction
{
public:
    MimironPhase1PositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, "mimiron phase 1 positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironP3Wx2LaserBarrageAction : public MovementAction
{
public:
    MimironP3Wx2LaserBarrageAction(PlayerbotAI* botAI, float distance = 24.0f, float delta_angle = M_PI / 8)
        : MovementAction(botAI, "mimiron p3wx2 laser barrage action")
    {
        this->distance = distance;
        this->delta_angle = delta_angle;
    }
    virtual bool Execute(Event event);

protected:
    float distance, delta_angle;
};

class MimironRapidBurstAction : public MovementAction
{
public:
    MimironRapidBurstAction(PlayerbotAI* botAI) : MovementAction(botAI, "mimiron rapid burst action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironAerialCommandUnitAction : public Action
{
public:
    MimironAerialCommandUnitAction(PlayerbotAI* botAI) : Action(botAI, "mimiron aerial command unit action") {}

    bool Execute(Event event) override;
};

class MimironRocketStrikeAction : public MovementAction
{
public:
    MimironRocketStrikeAction(PlayerbotAI* botAI) : MovementAction(botAI, "mimiron rocket strike action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironPhase4MarkDpsAction : public Action
{
public:
    MimironPhase4MarkDpsAction(PlayerbotAI* botAI) : Action(botAI, "mimiron phase 4 mark dps action") {}

    bool Execute(Event event) override;
};

class MimironCheatAction : public Action
{
public:
    MimironCheatAction(PlayerbotAI* botAI) : Action(botAI, "mimiron cheat action") {}

    bool Execute(Event event) override;
};

class VezaxCheatAction : public Action
{
public:
    VezaxCheatAction(PlayerbotAI* botAI) : Action(botAI, "vezax cheat action") {}

    bool Execute(Event event) override;
};

class VezaxShadowCrashAction : public MovementAction
{
public:
    VezaxShadowCrashAction(PlayerbotAI* botAI) : MovementAction(botAI, "vezax shadow crash action") {}

    bool Execute(Event event) override;
};

class VezaxMarkOfTheFacelessAction : public MovementAction
{
public:
    VezaxMarkOfTheFacelessAction(PlayerbotAI* botAI) : MovementAction(botAI, "vezax mark of the faceless action") {}

    bool Execute(Event event) override;
};

class YoggSaronOminousCloudCheatAction : public Action
{
public:
    YoggSaronOminousCloudCheatAction(PlayerbotAI* botAI) : Action(botAI, "yogg-saron ominous cloud cheat action") {}

    bool Execute(Event event) override;
};

class YoggSaronGuardianPositioningAction : public MovementAction
{
public:
    YoggSaronGuardianPositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron guardian positioning action") {}

    bool Execute(Event event) override;
};

class YoggSaronSanityAction : public MovementAction
{
public:
    YoggSaronSanityAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron sanity action") {}

    bool Execute(Event event) override;
};

class YoggSaronDeathOrbAction : public MoveAwayFromCreatureAction
{
public:
    YoggSaronDeathOrbAction(PlayerbotAI* botAI) : MoveAwayFromCreatureAction(botAI, "yogg-saron death orb action", NPC_DEATH_ORB, 10.0f) {}
};

class YoggSaronMaladyOfTheMindAction : public MoveAwayFromPlayerWithDebuffAction
{
public:
    YoggSaronMaladyOfTheMindAction(PlayerbotAI* botAI) : MoveAwayFromPlayerWithDebuffAction(botAI, "yogg-saron malady of the mind action", SPELL_MALADY_OF_THE_MIND, 15.0f) {}
};

class YoggSaronMarkTargetAction : public Action
{
public:
    YoggSaronMarkTargetAction(PlayerbotAI* botAI) : Action(botAI, "yogg-saron mark target action") {}

    bool Execute(Event event) override;
};

class YoggSaronBrainLinkAction : public MovementAction
{
public:
    YoggSaronBrainLinkAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron brain link action") {}

    bool Execute(Event event) override;
};

class YoggSaronMoveToEnterPortalAction : public MovementAction
{
public:
    YoggSaronMoveToEnterPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron move to enter portal action") {}

    bool Execute(Event event) override;
};

class YoggSaronFallFromFloorAction : public MovementAction
{
public:
    YoggSaronFallFromFloorAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron fall from floor action") {}

    bool Execute(Event event) override;
};

class YoggSaronBossRoomMovementCheatAction : public MovementAction
{
public:
    YoggSaronBossRoomMovementCheatAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron boss room movement cheat action") {}

    bool Execute(Event event) override;
};

class YoggSaronUsePortalAction : public Action
{
public:
    YoggSaronUsePortalAction(PlayerbotAI* botAI) : Action(botAI, "yogg-saron use portal action") {}

    bool Execute(Event event) override;
};

class YoggSaronIllusionRoomAction : public MovementAction
{
public:
    YoggSaronIllusionRoomAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron illusion room action") {}

    bool Execute(Event event) override;

private:
    bool SetRtiMark(YoggSaronTrigger yoggSaronTrigger);
    bool SetIllusionRtiTarget(YoggSaronTrigger yoggSaronTrigger);
    bool SetBrainRtiTarget(YoggSaronTrigger yoggSaronTrigger);
};

class YoggSaronMoveToExitPortalAction : public MovementAction
{
public:
    YoggSaronMoveToExitPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron move to exit portal action") {}

    bool Execute(Event event) override;
};

class YoggSaronLunaticGazeAction : public MovementAction
{
public:
    YoggSaronLunaticGazeAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron lunatic gaze action") {}

    bool Execute(Event event) override;
};

class YoggSaronPhase3PositioningAction : public MovementAction
{
public:
    YoggSaronPhase3PositioningAction(PlayerbotAI* botAI) : MovementAction(botAI, "yogg-saron phase 3 positioning action") {}

    bool Execute(Event event) override;
};

#endif
