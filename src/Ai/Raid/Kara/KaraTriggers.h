/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARATRIGGERS_H
#define PLAYERBOTS_KARATRIGGERS_H

#include "Trigger.h"

class KarazhanBotIsNotInCombatTrigger : public Trigger
{
public:
    KarazhanBotIsNotInCombatTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "karazhan bot is not in combat") {}
    bool IsActive() override;
};

class KarazhanEnemiesCastFearTrigger : public Trigger
{
public:
    KarazhanEnemiesCastFearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "karazhan enemies cast fear") {}
    bool IsActive() override;
};

class ManaWarpIsAboutToExplodeTrigger : public Trigger
{
public:
    ManaWarpIsAboutToExplodeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "mana warp is about to explode") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanPhaseOneActiveTrigger : public Trigger
{
public:
    AttumenTheHuntsmanPhaseOneActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "attumen the huntsman phase one active") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanPhaseTwoActiveTrigger : public Trigger
{
public:
    AttumenTheHuntsmanPhaseTwoActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "attumen the huntsman phase two active") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanPhaseTransitionTrigger : public Trigger
{
public:
    AttumenTheHuntsmanPhaseTransitionTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "attumen the huntsman phase transition") {}
    bool IsActive() override;
};

class MoroesShouldPrioritizeAddsTrigger : public Trigger
{
public:
    MoroesShouldPrioritizeAddsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "moroes should prioritize adds") {}
    bool IsActive() override;
};

class MaidenOfVirtueBossEngagedByTanksTrigger : public Trigger
{
public:
    MaidenOfVirtueBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "maiden of virtue boss engaged by tanks") {}
    bool IsActive() override;
};

class MaidenOfVirtueHolyWrathDealsChainDamageTrigger : public Trigger
{
public:
    MaidenOfVirtueHolyWrathDealsChainDamageTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "maiden of virtue holy wrath deals chain damage") {}
    bool IsActive() override;
};

class MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger : public Trigger
{
public:
    MaidenOfVirtueGroundingTotemConsumesHolyFireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "maiden of virtue grounding totem consumes holy fire") {}
    bool IsActive() override;
};

class BigBadWolfBossEngagedByTankTrigger : public Trigger
{
public:
    BigBadWolfBossEngagedByTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "big bad wolf boss engaged by tank") {}
    bool IsActive() override;
};

class BigBadWolfBossIsChasingLittleRedRidingHoodTrigger : public Trigger
{
public:
    BigBadWolfBossIsChasingLittleRedRidingHoodTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "big bad wolf boss is chasing little red riding hood") {}
    bool IsActive() override;
};

class RomuloAndJulianneBothBossesRevivedTrigger : public Trigger
{
public:
    RomuloAndJulianneBothBossesRevivedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "romulo and julianne both bosses revived") {}
    bool IsActive() override;
};

class WizardOfOzNeedTargetPriorityTrigger : public Trigger
{
public:
    WizardOfOzNeedTargetPriorityTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "wizard of oz need target priority") {}
    bool IsActive() override;
};

class WizardOfOzStrawmanIsVulnerableToFireTrigger : public Trigger
{
public:
    WizardOfOzStrawmanIsVulnerableToFireTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "wizard of oz strawman is vulnerable to fire") {}
    bool IsActive() override;
};
class TheCuratorAstralFlareSpawnedTrigger : public Trigger
{
public:
    TheCuratorAstralFlareSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the curator astral flare spawned") {}
    bool IsActive() override;
};

class TheCuratorBossEngagedByTanksTrigger : public Trigger
{
public:
    TheCuratorBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the curator boss engaged by tanks") {}
    bool IsActive() override;
};

class TheCuratorBossEngagedByRangedTrigger : public Trigger
{
public:
    TheCuratorBossEngagedByRangedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "the curator boss engaged by ranged") {}
    bool IsActive() override;
};

class TerestianIllhoofShouldPrioritizeChainsTrigger : public Trigger
{
public:
    TerestianIllhoofShouldPrioritizeChainsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "terestian illhoof should prioritize chains") {}
    bool IsActive() override;
};

class ShadeOfAranArcaneExplosionIsCastingTrigger : public Trigger
{
public:
    ShadeOfAranArcaneExplosionIsCastingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shade of aran arcane explosion is casting") {}
    bool IsActive() override;
};

class ShadeOfAranFlameWreathIsActiveTrigger : public Trigger
{
public:
    ShadeOfAranFlameWreathIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shade of aran flame wreath is active") {}
    bool IsActive() override;
};

class ShadeOfAranConjuredElementalsSummonedTrigger : public Trigger
{
public:
    ShadeOfAranConjuredElementalsSummonedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shade of aran conjured elementals summoned") {}
    bool IsActive() override;
};

class ShadeOfAranBossCastsCounterspellNearbyTrigger : public Trigger
{
public:
    ShadeOfAranBossCastsCounterspellNearbyTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shade of aran boss casts counterspell nearby") {}
    bool IsActive() override;
};

class NetherspiteRedBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteRedBeamIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "netherspite red beam is active") {}
    bool IsActive() override;
};

class NetherspiteBlueBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteBlueBeamIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "netherspite blue beam is active") {}
    bool IsActive() override;
};

class NetherspiteGreenBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteGreenBeamIsActiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "netherspite green beam is active") {}
    bool IsActive() override;
};

class NetherspiteBotIsNotBeamBlockerTrigger : public Trigger
{
public:
    NetherspiteBotIsNotBeamBlockerTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "netherspite bot is not beam blocker") {}
    bool IsActive() override;
};

class NetherspiteBossIsBanishedTrigger : public Trigger
{
public:
    NetherspiteBossIsBanishedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "netherspite boss is banished") {}
    bool IsActive() override;
};

class NetherspiteShouldManageTimersAndTrackersTrigger : public Trigger
{
public:
    NetherspiteShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "netherspite should manage timers and trackers") {}
    bool IsActive() override;
};

class PrinceMalchezaarBotIsEnfeebledTrigger : public Trigger
{
public:
    PrinceMalchezaarBotIsEnfeebledTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "prince malchezaar bot is enfeebled") {}
    bool IsActive() override;
};

class PrinceMalchezaarEngagedByNonTanksTrigger : public Trigger
{
public:
    PrinceMalchezaarEngagedByNonTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "prince malchezaar engaged by non-tanks") {}
    bool IsActive() override;
};

class PrinceMalchezaarBossEngagedByTanksTrigger : public Trigger
{
public:
    PrinceMalchezaarBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "prince malchezaar boss engaged by tanks") {}
    bool IsActive() override;
};

class NightbaneBossEngagedByTanksTrigger : public Trigger
{
public:
    NightbaneBossEngagedByTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "nightbane boss engaged by tanks") {}
    bool IsActive() override;
};

class NightbaneGroundPhaseEngagedByRangedTrigger : public Trigger
{
public:
    NightbaneGroundPhaseEngagedByRangedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "nightbane ground phase engaged by ranged") {}
    bool IsActive() override;
};

class NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger : public Trigger
{
public:
    NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "nightbane pets ignore collision to chase flying boss") {}
    bool IsActive() override;
};

class NightbaneBossIsFlyingTrigger : public Trigger
{
public:
    NightbaneBossIsFlyingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "nightbane boss is flying") {}
    bool IsActive() override;
};

class NightbaneBotWentOutOfBoundsTrigger : public Trigger
{
public:
    NightbaneBotWentOutOfBoundsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "nightbane bot went out of bounds") {}
    bool IsActive() override;
};

class NightbaneShouldManageTimersAndTrackersTrigger : public Trigger
{
public:
    NightbaneShouldManageTimersAndTrackersTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "nightbane should manage timers and trackers") {}
    bool IsActive() override;
};

#endif
