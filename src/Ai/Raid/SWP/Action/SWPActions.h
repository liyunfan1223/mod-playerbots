/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPACTIONS_H
#define PLAYERBOTS_SWPACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <limits>
#include <string>
#include <vector>

class Creature;

// General

class SunwellResetEncounterStatesAction : public Action
{
public:
    SunwellResetEncounterStatesAction(PlayerbotAI* botAI)
        : Action(botAI, "sunwell reset encounter states") {}
    bool Execute(Event event) override;
};

class SunwellRemoveAuraAction : public Action
{
public:
    SunwellRemoveAuraAction(PlayerbotAI* botAI) : Action(botAI, "sunwell remove aura") {}
    bool Execute(Event event) override;
};

// Trash

class VolatileFiendKeepEnemyAwayFromGroupAction : public AttackAction
{
public:
    VolatileFiendKeepEnemyAwayFromGroupAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "volatile fiend keep enemy away from group") {}
    bool Execute(Event event) override;
};

class ApocalypseGuardAttackWithHolyMagicAction : public Action
{
public:
    ApocalypseGuardAttackWithHolyMagicAction(PlayerbotAI* botAI)
        : Action(botAI, "apocalypse guard attack with holy magic") {}
    bool Execute(Event event) override;
};

// Shared Bosses

class SunwellRemoveDebuffWithImmunityAction : public Action
{
public:
    SunwellRemoveDebuffWithImmunityAction(PlayerbotAI* botAI, std::string const name)
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class SunwellMisdirectToMainTankAction : public Action
{
public:
    SunwellMisdirectToMainTankAction(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : Action(botAI, name), _bossName(bossName) {}
    bool Execute(Event event) override;

private:
    std::string const _bossName;
};

// Kalecgos

class KalecgosAnnounceBossHealthAction : public Action
{
public:
    KalecgosAnnounceBossHealthAction(PlayerbotAI* botAI)
        : Action(botAI, "kalecgos announce boss health") {}
    bool Execute(Event event) override;
};

class KalecgosSurfaceTankPositionDragonAction : public AttackAction
{
public:
    KalecgosSurfaceTankPositionDragonAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kalecgos surface tank position dragon") {}
    bool Execute(Event event) override;
};

class KalecgosEnterSpectralRiftAction : public MovementAction
{
public:
    KalecgosEnterSpectralRiftAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos enter spectral rift") {}
    bool Execute(Event event) override;

private:
    bool ShouldTankEnter();
};

class KalecgosSpreadRangedAction : public MovementAction
{
public:
    KalecgosSpreadRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos spread ranged") {}
    bool Execute(Event event) override;
    bool ResetInitialRangedPositionReached()
    {
        if (!_initialRangedPositionReached)
            return false;
        _initialRangedPositionReached = false;
        return true;
    }

private:
    bool _initialRangedPositionReached = false;
};

class KalecgosSathrovarrTankStandWithKalecAction : public MovementAction
{
public:
    KalecgosSathrovarrTankStandWithKalecAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos sathrovarr tank stand with kalec") {}
    bool Execute(Event event) override;
};

class KalecgosReturnToSpectralRealmGroundAction : public MovementAction
{
public:
    KalecgosReturnToSpectralRealmGroundAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kalecgos return to spectral realm ground") {}
    bool Execute(Event event) override;
};

// Brutallus

class BrutallusTanksPositionAndSwapAction : public AttackAction
{
public:
    BrutallusTanksPositionAndSwapAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "brutallus tanks position and swap") {}
    bool Execute(Event event) override;
    bool ResetInitialPositionReached()
    {
        if (!_mainTankInitialPositionReached)
            return false;
        _mainTankInitialPositionReached = false;
        return true;
    }

private:
    bool _mainTankInitialPositionReached = false;
};

class BrutallusPositionMeleeAtRearCenterAction : public MovementAction
{
public:
    BrutallusPositionMeleeAtRearCenterAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "brutallus position melee at rear center") {}
    bool Execute(Event event) override;

private:
    bool TryGetBrutallusMeleePosition(
        Unit* brutallus, Player* mainTank, Player* assistTank,
        uint8 meleeIndex, Position& position);
};

class BrutallusPositionRangedInTwoGroupsAction : public MovementAction
{
public:
    BrutallusPositionRangedInTwoGroupsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "brutallus position ranged in two groups") {}
    bool Execute(Event event) override;

private:
    bool StepToPosition(Position const& position, float arrivalDist);
};

class BrutallusIsolateBurnAction : public MovementAction
{
public:
    BrutallusIsolateBurnAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "brutallus isolate burn") {}
    bool Execute(Event event) override;

private:
    bool RemoveBurnWithCooldown();
    bool StepToPosition(Position const& position, float arrivalDist);
};

// Felmyst

class FelmystMainTankPositionBossOnGroundAction : public AttackAction
{
public:
    FelmystMainTankPositionBossOnGroundAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "felmyst main tank position boss on ground") {}
    bool Execute(Event event) override;
};

class FelmystRangedStackInThreeGroupsAction : public MovementAction
{
public:
    FelmystRangedStackInThreeGroupsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst ranged stack in three groups") {}
    bool Execute(Event event) override;
};

class FelmystMeleeStackBehindBossAction : public MovementAction
{
public:
    FelmystMeleeStackBehindBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst melee stack behind boss") {}
    bool Execute(Event event) override;
};

class FelmystRemoveEncapsulateAction : public Action
{
public:
    FelmystRemoveEncapsulateAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst remove encapsulate") {}
    bool Execute(Event event) override;
};

class FelmystRunAwayFromEncapsulatedPlayerAction : public MovementAction
{
public:
    FelmystRunAwayFromEncapsulatedPlayerAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst run away from encapsulated player") {}
    bool Execute(Event event) override;
};

class FelmystMassDispelGasNovaAction : public Action
{
public:
    FelmystMassDispelGasNovaAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst mass dispel gas nova") {}
    bool Execute(Event event) override;
};

class FelmystAvoidDemonicVaporAction : public MovementAction
{
public:
    FelmystAvoidDemonicVaporAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst avoid demonic vapor") {}
    bool Execute(Event event) override;

private:
    void AnnounceFlightLeader(Player* leader);
    bool MoveAwayFromVapor(bool unrestricted = false);
    bool MoveToFlightLeader(Player* leader);

    ObjectGuid _announcedFlightLeaderGuid;
};

class FelmystKiteDemonicVaporAction : public MovementAction
{
public:
    FelmystKiteDemonicVaporAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst kite demonic vapor") {}
    bool Execute(Event event) override;
};

class FelmystMoveToSafeFogLaneAction : public MovementAction
{
public:
    FelmystMoveToSafeFogLaneAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "felmyst move to safe fog lane") {}
    bool Execute(Event event) override;

private:
    Position _fogCrateStuckDestination;
    float _fogCrateStuckNearestDist = std::numeric_limits<float>::max();
    uint32 _fogCrateStuckSampleMs = 0;
    bool TryTeleportStuckBotOntoCrate(Position const& destination);
};

class FelmystMeleeClearTargetAction : public Action
{
public:
    FelmystMeleeClearTargetAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst melee clear target") {}
    bool Execute(Event event) override;
};

class FelmystKillCharmedPlayerAction : public AttackAction
{
public:
    FelmystKillCharmedPlayerAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "felmyst kill charmed player") {}
    bool Execute(Event event) override;
};

class FelmystManageLandingDpsTimerAction : public Action
{
public:
    FelmystManageLandingDpsTimerAction(PlayerbotAI* botAI)
        : Action(botAI, "felmyst manage landing dps timer") {}
    bool Execute(Event event) override;
};

// Eredar Twins

class EredarTwinsMeleeJumpFromBalconyAction : public MovementAction
{
public:
    EredarTwinsMeleeJumpFromBalconyAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins melee jump from balcony") {}
    bool Execute(Event event) override;
};

class EredarTwinsAnnounceAlythessTankAction : public Action
{
public:
    EredarTwinsAnnounceAlythessTankAction(PlayerbotAI* botAI)
        : Action(botAI, "eredar twins announce alythess tank") {}
    bool Execute(Event event) override;
};

class EredarTwinsMisdirectToTanksAction : public Action
{
public:
    EredarTwinsMisdirectToTanksAction(PlayerbotAI* botAI)
        : Action(botAI, "eredar twins misdirect to tanks") {}
    bool Execute(Event event) override;
};

class EredarTwinsPositionSacrolashTanksAction : public AttackAction
{
public:
    EredarTwinsPositionSacrolashTanksAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "eredar twins position sacrolash tanks") {}
    bool Execute(Event event) override;
};

class EredarTwinsAlythessTankMoveOutOfBlazeAction : public AttackAction
{
public:
    EredarTwinsAlythessTankMoveOutOfBlazeAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "eredar twins alythess tank move out of blaze") {}
    bool Execute(Event event) override;
    bool ResetAlythessTankState()
    {
        if (!_alythessTankStep && _lastBlazeGuid.IsEmpty())
            return false;
        _alythessTankStep = 0;
        _lastBlazeGuid.Clear();
        return true;
    }

private:
    bool StepToPosition(Unit* alythess, Position const& position, float arrivalDist);

    uint8 _alythessTankStep = 0;
    // The last Blaze the bot moved away from, so one Blaze moves it only one step
    ObjectGuid _lastBlazeGuid;
};

class EredarTwinsRangedStackAtBalconyEdgeAction : public MovementAction
{
public:
    EredarTwinsRangedStackAtBalconyEdgeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins ranged stack at balcony edge") {}
    bool Execute(Event event) override;
};

class EredarTwinsStackInRoomCenterAction : public MovementAction
{
public:
    EredarTwinsStackInRoomCenterAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins stack in room center") {}
    bool Execute(Event event) override;
};

class EredarTwinsDpsPrioritizeSacrolashAction : public AttackAction
{
public:
    EredarTwinsDpsPrioritizeSacrolashAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "eredar twins dps prioritize sacrolash") {}
    bool Execute(Event event) override;
};

class EredarTwinsConflagrationTargetMoveFromGroupAction : public MovementAction
{
public:
    EredarTwinsConflagrationTargetMoveFromGroupAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins conflagration target move from group") {}
    bool Execute(Event event) override;
};

class EredarTwinsMoveAwayFromSacrolashVictimAction : public MovementAction
{
public:
    EredarTwinsMoveAwayFromSacrolashVictimAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "eredar twins move away from sacrolash victim") {}
    bool Execute(Event event) override;
};

// M'uru

class MuruMisdirectEnemyToTankAction : public Action
{
public:
    MuruMisdirectEnemyToTankAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru misdirect enemy to tank") {}
    bool Execute(Event event) override;
};

class MuruMainTankPickUpEntropiusAction : public AttackAction
{
public:
    MuruMainTankPickUpEntropiusAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "m'uru main tank pick up entropius") {}
    bool Execute(Event event) override;
};

class MuruPositionRangedByPhaseAction : public MovementAction
{
public:
    MuruPositionRangedByPhaseAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru position ranged by phase") {}
    bool Execute(Event event) override;
    bool ResetEntropiusRangedPositionReached()
    {
        if (!_entropiusRangedPositionReached)
            return false;
        _entropiusRangedPositionReached = false;
        return true;
    }

private:
    bool _entropiusRangedPositionReached = false;
    bool TryGetEntropiusInitialRangedPosition(Position& position) const;
};

class MuruAssignDpsPriorityAction : public AttackAction
{
public:
    MuruAssignDpsPriorityAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "m'uru assign dps priority") {}
    bool Execute(Event event) override;

private:
    Unit* ResolveMuruDpsTarget(Unit* currentTarget);
};

class MuruKillDarkFiendsWithDispelAction : public Action
{
public:
    MuruKillDarkFiendsWithDispelAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru kill dark fiends with dispel") {}
    bool Execute(Event event) override;
};

class MuruTanksMoveSentinelToSafePositionAction : public AttackAction
{
public:
    MuruTanksMoveSentinelToSafePositionAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "m'uru tanks move sentinel to safe position") {}
    bool Execute(Event event) override;
};

class MuruSecondAssistTankGuardRangedAction : public MovementAction
{
public:
    MuruSecondAssistTankGuardRangedAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru second assist tank guard ranged") {}
    bool Execute(Event event) override;
};

class MuruMeleeFleeTheDarknessAction : public MovementAction
{
public:
    MuruMeleeFleeTheDarknessAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru melee flee the darkness") {}
    bool Execute(Event event) override;
};

class MuruCastStunOnBerserkerAction : public Action
{
public:
    MuruCastStunOnBerserkerAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru cast stun on berserker") {}
    bool Execute(Event event) override;
};

class MuruInterruptFelFireballAction : public Action
{
public:
    MuruInterruptFelFireballAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru interrupt fel fireball") {}
    bool Execute(Event event) override;

private:
    bool CastWarriorInterrupt(Unit* furyMage);
};

class MuruCastSpellStealOnSpellFuryAction : public Action
{
public:
    MuruCastSpellStealOnSpellFuryAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru cast spellsteal on spell fury") {}
    bool Execute(Event event) override;
};

class MuruWarlockEnslaveVoidSpawnAction : public Action
{
public:
    MuruWarlockEnslaveVoidSpawnAction(PlayerbotAI* botAI)
        : Action(botAI, "m'uru warlock enslave void spawn") {}
    bool Execute(Event event) override;
};

class MuruEnslavedVoidSpawnAttackAction : public Action
{
public:
    MuruEnslavedVoidSpawnAttackAction(PlayerbotAI* botAI, std::string const name)
        : Action(botAI, name) {}

protected:
    Unit* GetControlledVoidSpawn() const;
    Unit* GetVoidSpawnVolleyPriorityTarget(Unit* voidSpawn) const;
};

class MuruVoidSpawnCastShadowBoltVolleyAction : public MuruEnslavedVoidSpawnAttackAction
{
public:
    MuruVoidSpawnCastShadowBoltVolleyAction(PlayerbotAI* botAI)
        : MuruEnslavedVoidSpawnAttackAction(
            botAI, "m'uru void spawn cast shadow bolt volley") {}
    bool Execute(Event event) override;
};

class MuruKeepDistanceFromDarkFiendsAction : public MovementAction
{
public:
    MuruKeepDistanceFromDarkFiendsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru keep distance from dark fiends") {}
    bool Execute(Event event) override;
};

class MuruEscapeTheSingularityAction : public MovementAction
{
public:
    MuruEscapeTheSingularityAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "m'uru escape the singularity") {}
    bool Execute(Event event) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenAnnounceDragonOrbUserAction : public Action
{
public:
    KiljaedenAnnounceDragonOrbUserAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden announce dragon orb user") {}
    bool Execute(Event event) override;
};

class KiljaedenControlHandsOfTheDeceiverAction : public Action
{
public:
    KiljaedenControlHandsOfTheDeceiverAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden control hands of the deceiver") {}
    bool Execute(Event event) override;

private:
    bool CastStunOnHand(Unit* hand);
    bool CastSilenceOnHand(Unit* hand);
};

class KiljaedenMarkHandOfTheDeceiverAction : public Action
{
public:
    KiljaedenMarkHandOfTheDeceiverAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden mark hand of the deceiver") {}
    bool Execute(Event event) override;
};

class KiljaedenMoveHolyPaladinIntoStunRangeAction : public MovementAction
{
public:
    KiljaedenMoveHolyPaladinIntoStunRangeAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden move holy paladin into stun range") {}
    bool Execute(Event event) override;
};

class KiljaedenPositionAndMoveTanksAction : public AttackAction
{
public:
    KiljaedenPositionAndMoveTanksAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "kil'jaeden position and move tanks") {}
    bool Execute(Event event) override;

private:
    bool PickUpSinisterReflections(Creature* reflection);
};

class KiljaedenPositionMeleeAndAvoidArmageddonsAction : public MovementAction
{
public:
    KiljaedenPositionMeleeAndAvoidArmageddonsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden position melee and avoid armageddons") {}
    bool Execute(Event event) override;

private:
    bool TryGetMeleePosition(Position& position) const;
    bool TryAdjustMeleeForArmageddon(Position& position);
};

class KiljaedenPositionRangedAndAvoidArmageddonsAction : public MovementAction
{
public:
    KiljaedenPositionRangedAndAvoidArmageddonsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden position ranged and avoid armageddons") {}
    bool Execute(Event event) override;

private:
    bool TryGetRangedPosition(Position& position) const;
    bool TryAdjustRangedForArmageddon(Position& position);
};

class KiljaedenStackForShieldOfTheBlueAction : public MovementAction
{
public:
    KiljaedenStackForShieldOfTheBlueAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden stack for shield of the blue") {}
    bool Execute(Event event) override;
};

class KiljaedenUseDragonOrbAction : public MovementAction
{
public:
    KiljaedenUseDragonOrbAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "kil'jaeden use dragon orb") {}
    bool Execute(Event event) override;
};

class KiljaedenDragonBuffAndProtectRaidAction : public Action
{
public:
    KiljaedenDragonBuffAndProtectRaidAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden dragon buff and protect raid") {}
    bool Execute(Event event) override;

private:
    bool ExecuteDuringDarknessOfAThousandSouls(Unit* kiljaeden, Unit* dragon);
    bool ExecuteOutsideDarknessOfAThousandSouls(Unit* dragon);
};

class KiljaedenReleaseStaleRootAction : public Action
{
public:
    KiljaedenReleaseStaleRootAction(PlayerbotAI* botAI)
        : Action(botAI, "kil'jaeden release stale root") {}
    bool Execute(Event event) override;
};

#endif
