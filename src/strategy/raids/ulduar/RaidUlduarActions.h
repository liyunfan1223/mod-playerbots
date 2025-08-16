#ifndef _PLAYERBOT_RAIDULDUARACTIONS_H
#define _PLAYERBOT_RAIDULDUARACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidUlduarBossHelper.h"
#include "RaidUlduarTriggers.h"
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
    HodirBitingColdJumpAction(PlayerbotAI* ai) : MovementAction(ai, "hodir biting cold jump") {}
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
    FreyaMoveToHealingSporeAction(PlayerbotAI* ai) : MovementAction(ai, "freya move to healing spore action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimUnbalancingStrikeAction : public Action
{
public:
    ThorimUnbalancingStrikeAction(PlayerbotAI* ai) : Action(ai, "thorim unbalancing strike action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimMarkDpsTargetAction : public Action
{
public:
    ThorimMarkDpsTargetAction(PlayerbotAI* ai) : Action(ai, "thorim mark dps target action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimArenaPositioningAction : public MovementAction
{
public:
    ThorimArenaPositioningAction(PlayerbotAI* ai) : MovementAction(ai, "thorim arena positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class ThorimGauntletPositioningAction : public MovementAction
{
public:
    ThorimGauntletPositioningAction(PlayerbotAI* ai) : MovementAction(ai, "thorim gauntlet positioning action") {}

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
    ThorimPhase2PositioningAction(PlayerbotAI* ai) : MovementAction(ai, "thorim phase 2 positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironShockBlastAction : public MovementAction
{
public:
    MimironShockBlastAction(PlayerbotAI* ai) : MovementAction(ai, "mimiron shock blast action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironPhase1PositioningAction : public MovementAction
{
public:
    MimironPhase1PositioningAction(PlayerbotAI* ai) : MovementAction(ai, "mimiron phase 1 positioning action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironP3Wx2LaserBarrageAction : public MovementAction
{
public:
    MimironP3Wx2LaserBarrageAction(PlayerbotAI* ai, float distance = 24.0f, float delta_angle = M_PI / 8)
        : MovementAction(ai, "mimiron p3wx2 laser barrage action")
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
    MimironRapidBurstAction(PlayerbotAI* ai) : MovementAction(ai, "mimiron rapid burst action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironAerialCommandUnitAction : public Action
{
public:
    MimironAerialCommandUnitAction(PlayerbotAI* ai) : Action(ai, "mimiron aerial command unit action") {}

    bool Execute(Event event) override;
};

class MimironRocketStrikeAction : public MovementAction
{
public:
    MimironRocketStrikeAction(PlayerbotAI* ai) : MovementAction(ai, "mimiron rocket strike action") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MimironPhase4MarkDpsAction : public Action
{
public:
    MimironPhase4MarkDpsAction(PlayerbotAI* ai) : Action(ai, "mimiron phase 4 mark dps action") {}

    bool Execute(Event event) override;
};

class MimironCheatAction : public Action
{
public:
    MimironCheatAction(PlayerbotAI* ai) : Action(ai, "mimiron cheat action") {}

    bool Execute(Event event) override;
};

class VezaxCheatAction : public Action
{
public:
    VezaxCheatAction(PlayerbotAI* ai) : Action(ai, "vezax cheat action") {}

    bool Execute(Event event) override;
};

class VezaxShadowCrashAction : public MovementAction
{
public:
    VezaxShadowCrashAction(PlayerbotAI* ai) : MovementAction(ai, "vezax shadow crash action") {}

    bool Execute(Event event) override;
};

class VezaxMarkOfTheFacelessAction : public MovementAction
{
public:
    VezaxMarkOfTheFacelessAction(PlayerbotAI* ai) : MovementAction(ai, "vezax mark of the faceless action") {}

    bool Execute(Event event) override;
};

class YoggSaronOminousCloudCheatAction : public Action
{
public:
    YoggSaronOminousCloudCheatAction(PlayerbotAI* ai) : Action(ai, "yogg-saron ominous cloud cheat action") {}

    bool Execute(Event event) override;
};

class YoggSaronGuardianPositioningAction : public MovementAction
{
public:
    YoggSaronGuardianPositioningAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron guardian positioning action") {}

    bool Execute(Event event) override;
};

class YoggSaronSanityAction : public MovementAction
{
public:
    YoggSaronSanityAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron sanity action") {}

    bool Execute(Event event) override;
};

class YoggSaronDeathOrbAction : public MoveAwayFromCreatureAction
{
public:
    YoggSaronDeathOrbAction(PlayerbotAI* ai) : MoveAwayFromCreatureAction(ai, "yogg-saron death orb action", NPC_DEATH_ORB, 10.0f) {}
};

class YoggSaronMaladyOfTheMindAction : public MoveAwayFromPlayerWithDebuffAction
{
public:
    YoggSaronMaladyOfTheMindAction(PlayerbotAI* ai) : MoveAwayFromPlayerWithDebuffAction(ai, "yogg-saron malady of the mind action", SPELL_MALADY_OF_THE_MIND, 15.0f) {}
};

class YoggSaronMarkTargetAction : public Action
{
public:
    YoggSaronMarkTargetAction(PlayerbotAI* ai) : Action(ai, "yogg-saron mark target action") {}

    bool Execute(Event event) override;
};

class YoggSaronBrainLinkAction : public MovementAction
{
public:
    YoggSaronBrainLinkAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron brain link action") {}

    bool Execute(Event event) override;
};

class YoggSaronMoveToEnterPortalAction : public MovementAction
{
public:
    YoggSaronMoveToEnterPortalAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron move to enter portal action") {}

    bool Execute(Event event) override;
};

class YoggSaronFallFromFloorAction : public MovementAction
{
public:
    YoggSaronFallFromFloorAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron fall from floor action") {}

    bool Execute(Event event) override;
};

class YoggSaronBossRoomMovementCheatAction : public MovementAction
{
public:
    YoggSaronBossRoomMovementCheatAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron boss room movement cheat action") {}

    bool Execute(Event event) override;
};

class YoggSaronUsePortalAction : public Action
{
public:
    YoggSaronUsePortalAction(PlayerbotAI* ai) : Action(ai, "yogg-saron use portal action") {}

    bool Execute(Event event) override;
};

class YoggSaronIllusionRoomAction : public MovementAction
{
public:
    YoggSaronIllusionRoomAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron illusion room action") {}

    bool Execute(Event event) override;

private:
    bool SetRtiMark(YoggSaronTrigger yoggSaronTrigger);
    bool SetIllusionRtiTarget(YoggSaronTrigger yoggSaronTrigger);
    bool SetBrainRtiTarget(YoggSaronTrigger yoggSaronTrigger);
};

class YoggSaronMoveToExitPortalAction : public MovementAction
{
public:
    YoggSaronMoveToExitPortalAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron move to exit portal action") {}

    bool Execute(Event event) override;
};

class YoggSaronLunaticGazeAction : public MovementAction
{
public:
    YoggSaronLunaticGazeAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron lunatic gaze action") {}

    bool Execute(Event event) override;
};

class YoggSaronPhase3PositioningAction : public MovementAction
{
public:
    YoggSaronPhase3PositioningAction(PlayerbotAI* ai) : MovementAction(ai, "yogg-saron phase 3 positioning action") {}

    bool Execute(Event event) override;
};

#endif
