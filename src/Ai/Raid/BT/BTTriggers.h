/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTTRIGGERS_H
#define PLAYERBOTS_BTTRIGGERS_H

#include "Trigger.h"

// Shared

class BlackTempleBotIsNotInCombatTrigger : public Trigger
{
public:
    BlackTempleBotIsNotInCombatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "black temple bot is not in combat") {}
    bool IsActive() override;
};

// High Warlord Naj'entus

class HighWarlordNajentusPullingBossTrigger : public Trigger
{
public:
    HighWarlordNajentusPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus pulling boss") {}
    bool IsActive() override;
};

class HighWarlordNajentusBossEngagedByTanksTrigger : public Trigger
{
public:
    HighWarlordNajentusBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus boss engaged by tanks") {}
    bool IsActive() override;
};

class HighWarlordNajentusCastsNeedleSpinesTrigger : public Trigger
{
public:
    HighWarlordNajentusCastsNeedleSpinesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus casts needle spines") {}
    bool IsActive() override;
};

class HighWarlordNajentusPlayerIsImpaledTrigger : public Trigger
{
public:
    HighWarlordNajentusPlayerIsImpaledTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus player is impaled") {}
    bool IsActive() override;
};

class HighWarlordNajentusBossHasTidalShieldTrigger : public Trigger
{
public:
    HighWarlordNajentusBossHasTidalShieldTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high warlord naj'entus boss has tidal shield") {}
    bool IsActive() override;
};

// Supremus

class SupremusPullingBossOrChangingPhaseTrigger : public Trigger
{
public:
    SupremusPullingBossOrChangingPhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus pulling boss or changing phase") {}
    bool IsActive() override;
};

class SupremusBossEngagedByRangedTrigger : public Trigger
{
public:
    SupremusBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus boss engaged by ranged") {}
    bool IsActive() override;
};

class SupremusBossIsFixatedOnBotTrigger : public Trigger
{
public:
    SupremusBossIsFixatedOnBotTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus boss is fixated on bot") {}
    bool IsActive() override;
};

class SupremusVolcanoIsNearbyTrigger : public Trigger
{
public:
    SupremusVolcanoIsNearbyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus volcano is nearby") {}
    bool IsActive() override;
};

class SupremusNeedToManagePhaseTimerTrigger : public Trigger
{
public:
    SupremusNeedToManagePhaseTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "supremus need to manage phase timer") {}
    bool IsActive() override;
};

// Shade of Akama

class ShadeOfAkamaKillingChannelersStartsPhase2Trigger : public Trigger
{
public:
    ShadeOfAkamaKillingChannelersStartsPhase2Trigger(
        PlayerbotAI* botAI) : Trigger(botAI, "shade of akama killing channelers starts phase 2") {}
    bool IsActive() override;
};

// Teron Gorefiend
class TeronGorefiendPullingBossTrigger : public Trigger
{
public:
    TeronGorefiendPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend pulling boss") {}
    bool IsActive() override;
};

class TeronGorefiendBossEngagedByTanksTrigger : public Trigger
{
public:
    TeronGorefiendBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss engaged by tanks") {}
    bool IsActive() override;
};

class TeronGorefiendBossEngagedByRangedTrigger : public Trigger
{
public:
    TeronGorefiendBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss engaged by ranged") {}
    bool IsActive() override;
};

class TeronGorefiendBossIsCastingShadowOfDeathTrigger : public Trigger
{
public:
    TeronGorefiendBossIsCastingShadowOfDeathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend boss is casting shadow of death") {}
    bool IsActive() override;
};

class TeronGorefiendBotHasShadowOfDeathTrigger : public Trigger
{
public:
    TeronGorefiendBotHasShadowOfDeathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend bot has shadow of death") {}
    bool IsActive() override;
};

class TeronGorefiendBotTransformedIntoVengefulSpiritTrigger : public Trigger
{
public:
    TeronGorefiendBotTransformedIntoVengefulSpiritTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "teron gorefiend bot transformed into vengeful spirit") {}
    bool IsActive() override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilPullingBossTrigger : public Trigger
{
public:
    GurtoggBloodboilPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil pulling boss") {}
    bool IsActive() override;
};

class GurtoggBloodboilBossEngagedByTanksTrigger : public Trigger
{
public:
    GurtoggBloodboilBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil boss engaged by tanks") {}
    bool IsActive() override;
};

class GurtoggBloodboilBossCastsBloodboilTrigger : public Trigger
{
public:
    GurtoggBloodboilBossCastsBloodboilTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil boss casts bloodboil") {}
    bool IsActive() override;
};

class GurtoggBloodboilBotHasFelRageTrigger : public Trigger
{
public:
    GurtoggBloodboilBotHasFelRageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil bot has fel rage") {}
    bool IsActive() override;
};

class GurtoggBloodboilNeedToManagePhaseTimerTrigger : public Trigger
{
public:
    GurtoggBloodboilNeedToManagePhaseTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gurtogg bloodboil need to manage phase timer") {}
    bool IsActive() override;
};

// Reliquary of Souls

class ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger : public Trigger
{
public:
    ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls aggro resets upon phase change") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of suffering fixates on closest target") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of suffering disables healing") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of desire has rune shield") {}
    bool IsActive() override;
};

class ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger : public Trigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "reliquary of souls essence of desire casting deaden") {}
    bool IsActive() override;
};

// Mother Shahraz

class MotherShahrazPullingBossTrigger : public Trigger
{
public:
    MotherShahrazPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz pulling boss") {}
    bool IsActive() override;
};

class MotherShahrazBossEngagedByTanksTrigger : public Trigger
{
public:
    MotherShahrazBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz boss engaged by tanks") {}
    bool IsActive() override;
};

class MotherShahrazTanksArePositioningBossTrigger : public Trigger
{
public:
    MotherShahrazTanksArePositioningBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz tanks are positioning boss") {}
    bool IsActive() override;
};

class MotherShahrazSinisterBeamKnocksBackPlayersTrigger : public Trigger
{
public:
    MotherShahrazSinisterBeamKnocksBackPlayersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz sinister beam knocks back players") {}
    bool IsActive() override;
};

class MotherShahrazBotsAreLinkedByFatalAttractionTrigger : public Trigger
{
public:
    MotherShahrazBotsAreLinkedByFatalAttractionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mother shahraz bots are linked by fatal attraction") {}
    bool IsActive() override;
};

// Illidari Council

class IllidariCouncilPullingBossesTrigger : public Trigger
{
public:
    IllidariCouncilPullingBossesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council pulling bosses") {}
    bool IsActive() override;
};

class IllidariCouncilGathiosEngagedByMainTankTrigger : public Trigger
{
public:
    IllidariCouncilGathiosEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council gathios engaged by main tank") {}
    bool IsActive() override;
};

class IllidariCouncilGathiosCastingJudgementOfCommandTrigger : public Trigger
{
public:
    IllidariCouncilGathiosCastingJudgementOfCommandTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council gathios casting judgement of command") {}
    bool IsActive() override;
};

class IllidariCouncilMalandeEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    IllidariCouncilMalandeEngagedByFirstAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council malande engaged by first assist tank") {}
    bool IsActive() override;
};

class IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger : public Trigger
{
public:
    IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council darkshadow engaged by second assist tank") {}
    bool IsActive() override;
};

class IllidariCouncilZerevorEngagedByMageTankTrigger : public Trigger
{
public:
    IllidariCouncilZerevorEngagedByMageTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council zerevor engaged by mage tank") {}
    bool IsActive() override;
};

class IllidariCouncilMageTankNeedsDedicatedHealerTrigger : public Trigger
{
public:
    IllidariCouncilMageTankNeedsDedicatedHealerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council mage tank needs dedicated healer") {}
    bool IsActive() override;
};

class IllidariCouncilZerevorCastsDangerousAoesTrigger : public Trigger
{
public:
    IllidariCouncilZerevorCastsDangerousAoesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council zerevor casts dangerous aoes") {}
    bool IsActive() override;
};

class IllidariCouncilPetsScrewUpThePullTrigger : public Trigger
{
public:
    IllidariCouncilPetsScrewUpThePullTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council pets screw up the pull") {}
    bool IsActive() override;
};

class IllidariCouncilNeedToManageDpsTimerTrigger : public Trigger
{
public:
    IllidariCouncilNeedToManageDpsTimerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council need to manage dps timer") {}
    bool IsActive() override;
};

class IllidariCouncilDeterminingDpsAssignmentsTrigger : public Trigger
{
public:
    IllidariCouncilDeterminingDpsAssignmentsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidari council determining dps assignments") {}
    bool IsActive() override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageTankNeedsAggroTrigger : public Trigger
{
public:
    IllidanStormrageTankNeedsAggroTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage tank needs aggro") {}
    bool IsActive() override;
};

class IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger : public Trigger
{
public:
    IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss casts flame crash in front of main tank") {}
    bool IsActive() override;
};

class IllidanStormrageBotHasParasiticShadowfiendTrigger : public Trigger
{
public:
    IllidanStormrageBotHasParasiticShadowfiendTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage bot has parasitic shadowfiend") {}
    bool IsActive() override;
};

class IllidanStormrageParasiticShadowfiendsRunWildTrigger : public Trigger
{
public:
    IllidanStormrageParasiticShadowfiendsRunWildTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage parasitic shadowfiends run wild") {}
    bool IsActive() override;
};

class IllidanStormrageBossSummonedFlamesOfAzzinothTrigger : public Trigger
{
public:
    IllidanStormrageBossSummonedFlamesOfAzzinothTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss summoned flames of azzinoth") {}
    bool IsActive() override;
};

class IllidanStormragePetsDieToFireTrigger : public Trigger
{
public:
    IllidanStormragePetsDieToFireTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage pets die to fire") {}
    bool IsActive() override;
};

class IllidanStormrageGrateIsSafeFromFlamesTrigger : public Trigger
{
public:
    IllidanStormrageGrateIsSafeFromFlamesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage grate is safe from flames") {}
    bool IsActive() override;
};

class IllidanStormrageBotStruckByDarkBarrageTrigger : public Trigger
{
public:
    IllidanStormrageBotStruckByDarkBarrageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage bot struck by dark barrage") {}
    bool IsActive() override;
};

class IllidanStormrageBossIsPreparingToLandTrigger : public Trigger
{
public:
    IllidanStormrageBossIsPreparingToLandTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss is preparing to land") {}
    bool IsActive() override;
};

class IllidanStormrageBossDealsSplashDamageTrigger : public Trigger
{
public:
    IllidanStormrageBossDealsSplashDamageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss deals splash damage") {}
    bool IsActive() override;
};

class IllidanStormrageThisExpansionHatesMeleeTrigger : public Trigger
{
public:
    IllidanStormrageThisExpansionHatesMeleeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage this expansion hates melee") {}
    bool IsActive() override;
};

class IllidanStormrageBossTransformsIntoDemonTrigger : public Trigger
{
public:
    IllidanStormrageBossTransformsIntoDemonTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss transforms into demon") {}
    bool IsActive() override;
};

class IllidanStormrageBossSpawnsAddsTrigger : public Trigger
{
public:
    IllidanStormrageBossSpawnsAddsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage boss spawns adds") {}
    bool IsActive() override;
};

class IllidanStormrageMaievPlacedShadowTrapTrigger : public Trigger
{
public:
    IllidanStormrageMaievPlacedShadowTrapTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage maiev placed shadow trap") {}
    bool IsActive() override;
};

class IllidanStormrageNeedToManageDpsTimerAndRtiTrigger : public Trigger
{
public:
    IllidanStormrageNeedToManageDpsTimerAndRtiTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage need to manage dps timer and rti") {}
    bool IsActive() override;
};

class IllidanStormrageNeedToClearHazardsBetweenPhasesTrigger : public Trigger
{
public:
    IllidanStormrageNeedToClearHazardsBetweenPhasesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage need to clear hazards between phases") {}
    bool IsActive() override;
};

class IllidanStormrageCheatTrigger : public Trigger
{
public:
    IllidanStormrageCheatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "illidan stormrage cheat") {}
    bool IsActive() override;
};

#endif
