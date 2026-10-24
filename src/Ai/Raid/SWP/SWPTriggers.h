/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPTRIGGERS_H
#define PLAYERBOTS_SWPTRIGGERS_H

#include "EncounterHelpers.h"
#include "SWPShared.h"
#include "Trigger.h"
#include <string>

// Shared

class SunwellEncounterTrigger : public Trigger
{
public:
    SunwellEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class SunwellNoEncounterInProgressTrigger : public Trigger
{
public:
    SunwellNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell no encounter in progress", 1000) {}
    bool IsActive() override;
};

class SunwellAuraToRemoveTrigger : public Trigger
{
public:
    SunwellAuraToRemoveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sunwell aura to remove", 1000) {}
    bool IsActive() override;
};

// Trash

class VolatileFiendSelfDestructsWhenNearTrigger : public Trigger
{
public:
    VolatileFiendSelfDestructsWhenNearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "volatile fiend self destructs when near") {}
    bool IsActive() override;
};

class ApocalypseGuardProtectedByInfernalDefenseTrigger : public Trigger
{
public:
    ApocalypseGuardProtectedByInfernalDefenseTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "apocalypse guard protected by infernal defense") {}
    bool IsActive() override;
};

// Shared Bosses

// For Misdirection before BOSS_ENGAGED_HEALTH_PCT (i.e., shortly after pull). Used for Kalecgos,
// Brutallus, and the Eredar Twins (on Alythess).
class SunwellHunterShouldMisdirectTrigger : public SunwellEncounterTrigger
{
public:
    SunwellHunterShouldMisdirectTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : SunwellEncounterTrigger(botAI, name), _bossName(bossName) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
};

// Kalecgos

class KalecgosShouldCommunicateBossHealthTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosShouldCommunicateBossHealthTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos should communicate boss health") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosRequiresTankRotationTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosRequiresTankRotationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos requires tank rotation") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosSpectralRiftIsOpenTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosSpectralRiftIsOpenTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos spectral rift is open") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosRangedShouldSpreadTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosTooManyArcaneBuffetStacksTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosTooManyArcaneBuffetStacksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos too many arcane buffet stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosHumanoidKalecTanksSathrovarrTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosHumanoidKalecTanksSathrovarrTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos humanoid kalec tanks sathrovarr") {}

protected:
    bool IsActiveInEncounter() override;
};

class KalecgosBotsDontObserveGravityTrigger : public SunwellEncounterTrigger
{
public:
    KalecgosBotsDontObserveGravityTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kalecgos bots don't observe gravity") {}

protected:
    bool IsActiveInEncounter() override;
};

// Brutallus

class BrutallusRequiresTwoTanksTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus requires two tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusMeleeShouldStandInPlaceTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusMeleeShouldStandInPlaceTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus melee should stand in place") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusRangedShouldSoakMeteorSlashTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusRangedShouldSoakMeteorSlashTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus ranged should soak meteor slash") {}

protected:
    bool IsActiveInEncounter() override;
};

class BrutallusBurnOnNonTankTrigger : public SunwellEncounterTrigger
{
public:
    BrutallusBurnOnNonTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "brutallus burn on non-tank") {}

protected:
    bool IsActiveInEncounter() override;
};

// Felmyst

class FelmystHunterShouldMisdirectTrigger : public SunwellEncounterTrigger
{
public:
    FelmystHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystGroundPhaseShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    FelmystGroundPhaseShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst ground phase should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystRangedShouldPositionToDispelAndFleeTrigger : public SunwellEncounterTrigger
{
public:
    FelmystRangedShouldPositionToDispelAndFleeTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "felmyst ranged should position to dispel and flee") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeShouldStayTogetherTrigger : public SunwellEncounterTrigger
{
public:
    FelmystMeleeShouldStayTogetherTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst melee should stay together") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystEncapsulateOnMageOrPaladinTrigger : public SunwellEncounterTrigger
{
public:
    FelmystEncapsulateOnMageOrPaladinTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst encapsulate on mage or paladin") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystNearEncapsulatedPlayerTrigger : public SunwellEncounterTrigger
{
public:
    FelmystNearEncapsulatedPlayerTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst near encapsulated player") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerHasGasNovaTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPlayerHasGasNovaTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst player has gas nova") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldAvoidDemonicVaporTrailsTrigger : public SunwellEncounterTrigger
{
public:
    FelmystShouldAvoidDemonicVaporTrailsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst should avoid demonic vapor trails") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystTargetedByDemonicVaporTrigger : public SunwellEncounterTrigger
{
public:
    FelmystTargetedByDemonicVaporTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst targeted by demonic vapor") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystFogOfCorruptionIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    FelmystFogOfCorruptionIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst fog of corruption is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystMeleeCannotReachFlyingBossTrigger : public SunwellEncounterTrigger
{
public:
    FelmystMeleeCannotReachFlyingBossTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst melee cannot reach flying boss") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystPlayerIsCharmedByFogTrigger : public SunwellEncounterTrigger
{
public:
    FelmystPlayerIsCharmedByFogTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst player is charmed by fog") {}

protected:
    bool IsActiveInEncounter() override;
};

class FelmystShouldHoldDpsWhileLandingTrigger : public SunwellEncounterTrigger
{
public:
    FelmystShouldHoldDpsWhileLandingTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "felmyst should hold dps while landing") {}

protected:
    bool IsActiveInEncounter() override;
};

// Eredar Twins

class EredarTwinsMeleeIsAtBalconyTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsMeleeIsAtBalconyTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins melee is at balcony") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldAnnounceAlythessTankTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldAnnounceAlythessTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins should announce alythess tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashRequiresTwoTanksTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsSacrolashRequiresTwoTanksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins sacrolash requires two tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsAlythessCastsBlazeOnTankTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsAlythessCastsBlazeOnTankTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins alythess casts blaze on tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsRangedNeedsLosTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsRangedNeedsLosTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins ranged needs los") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsOnlyAlythessRemainsTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsOnlyAlythessRemainsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins only alythess remains") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsTooManyFlameTouchedStacksTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsTooManyFlameTouchedStacksTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins too many flame touched stacks") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsShouldFocusDpsTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins should focus dps") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsActiveConflagrationTargetTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsActiveConflagrationTargetTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "eredar twins active conflagration target") {}

protected:
    bool IsActiveInEncounter() override;
};

class EredarTwinsSacrolashVictimHasConflagrationTrigger : public SunwellEncounterTrigger
{
public:
    EredarTwinsSacrolashVictimHasConflagrationTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "eredar twins sacrolash victim has conflagration") {}

protected:
    bool IsActiveInEncounter() override;
};

// M'uru

class MuruHunterShouldMisdirectNewEnemyTrigger : public SunwellEncounterTrigger
{
public:
    MuruHunterShouldMisdirectNewEnemyTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru hunter should misdirect new enemy") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruTransformedIntoEntropiusTrigger : public SunwellEncounterTrigger
{
public:
    MuruTransformedIntoEntropiusTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru transformed into entropius") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruRangedShouldStackOrSpreadTrigger : public SunwellEncounterTrigger
{
public:
    MuruRangedShouldStackOrSpreadTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru ranged should stack or spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruShouldAssignDpsPriorityTrigger : public SunwellEncounterTrigger
{
public:
    MuruShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru should assign dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSentinelShouldBeTankedTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSentinelShouldBeTankedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru void sentinel should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruAddsSpawnAtEntranceTrigger : public SunwellEncounterTrigger
{
public:
    MuruAddsSpawnAtEntranceTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru adds spawn at entrance") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarkFiendsSpawnedTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarkFiendsSpawnedTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru dark fiends spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruDarknessIsComingTrigger : public SunwellEncounterTrigger
{
public:
    MuruDarknessIsComingTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru darkness is coming") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruBerserkerHasFlurryTrigger : public SunwellEncounterTrigger
{
public:
    MuruBerserkerHasFlurryTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru berserker has flurry") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageCastingFelFireballTrigger : public SunwellEncounterTrigger
{
public:
    MuruFuryMageCastingFelFireballTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru fury mage casting fel fireball") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruFuryMageHasSpellFuryTrigger : public SunwellEncounterTrigger
{
public:
    MuruFuryMageHasSpellFuryTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru fury mage has spell fury") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruVoidSpawnAvailableForEnslaveTrigger : public SunwellEncounterTrigger
{
public:
    MuruVoidSpawnAvailableForEnslaveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru void spawn available for enslave") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruWarlockHasEnslavedVoidSpawnTrigger : public SunwellEncounterTrigger
{
public:
    MuruWarlockHasEnslavedVoidSpawnTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru warlock has enslaved void spawn") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruEntropiusSummonsVoidZonesTrigger : public SunwellEncounterTrigger
{
public:
    MuruEntropiusSummonsVoidZonesTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "m'uru entropius summons void zones") {}

protected:
    bool IsActiveInEncounter() override;
};

class MuruTheSingularityIsNearTrigger : public SunwellEncounterTrigger
{
public:
    MuruTheSingularityIsNearTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "m'uru the singularity is near") {}

protected:
    bool IsActiveInEncounter() override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenShouldCoordinateOrbUseTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenShouldCoordinateOrbUseTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden should coordinate orb use") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenHandsOfTheDeceiverAreActiveTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenHandsOfTheDeceiverAreActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden hands of the deceiver are active") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenTanksShouldHoldBossAndReflectionsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenTanksShouldHoldBossAndReflectionsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(
            botAI, "kil'jaeden tanks should hold boss and reflections") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenMeleeShouldSplitIntoTwoGroupsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden melee should split into two groups") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenRangedShouldSpreadInTwoArcsTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenRangedShouldSpreadInTwoArcsTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden ranged should spread in two arcs") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenFireBloomOnImmunityClassTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenFireBloomOnImmunityClassTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden fire bloom on immunity class") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenSaysChaosDestructionOblivionTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenSaysChaosDestructionOblivionTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden says: Chaos! Destruction! Oblivion!") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenDragonOrbIsActiveTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenDragonOrbIsActiveTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden dragon orb is active") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenBotControlsDragonTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenBotControlsDragonTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden bot controls dragon") {}

protected:
    bool IsActiveInEncounter() override;
};

class KiljaedenStaleRootAfterDragonTrigger : public SunwellEncounterTrigger
{
public:
    KiljaedenStaleRootAfterDragonTrigger(PlayerbotAI* botAI)
        : SunwellEncounterTrigger(botAI, "kil'jaeden stale root after dragon") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
