/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAMULTIPLIERS_H
#define PLAYERBOTS_KARAMULTIPLIERS_H

#include "Multiplier.h"

class KarazhanSetTremorTotemMultiplier : public Multiplier
{
public:
    KarazhanSetTremorTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "karazhan set tremor totem") {}
    float GetValue(Action* action) override;
};

class AttumenTheHuntsmanDisableAutomaticTargetingMultiplier : public Multiplier
{
public:
    AttumenTheHuntsmanDisableAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "attumen the huntsman disable automatic targeting") {}
    float GetValue(Action* action) override;
};

class AttumenTheHuntsmanStayStackedMultiplier : public Multiplier
{
public:
    AttumenTheHuntsmanStayStackedMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "attumen the huntsman stay stacked") {}
    float GetValue(Action* action) override;
};

class AttumenTheHuntsmanWaitForDpsMultiplier : public Multiplier
{
public:
    AttumenTheHuntsmanWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "attumen the huntsman wait for dps") {}
    float GetValue(Action* action) override;
};

class MaidenOfVirtueDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    MaidenOfVirtueDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "maiden of virtue disable combat formation move") {}
    float GetValue(Action* action) override;
};

class MaidenOfVirtueSetGroundingTotemMultiplier : public Multiplier
{
public:
    MaidenOfVirtueSetGroundingTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "maiden of virtue set grounding totem") {}
    float GetValue(Action* action) override;
};

class TheCuratorDisableTankAssistMultiplier : public Multiplier
{
public:
    TheCuratorDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "the curator disable tank assist") {}
    float GetValue(Action* action) override;
};

class TheCuratorDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    TheCuratorDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "the curator disable combat formation move") {}
    float GetValue(Action* action) override;
};

class TheCuratorDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    TheCuratorDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "the curator delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

class TerestianIllhoofDontDotFiendishImpsMultiplier : public Multiplier
{
public:
    TerestianIllhoofDontDotFiendishImpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "terestian illhoof don't dot fiendish imps") {}
    float GetValue(Action* action) override;
};

class ShadeOfAranArcaneExplosionRunAwayMultiplier : public Multiplier
{
public:
    ShadeOfAranArcaneExplosionRunAwayMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "shade of aran arcane explosion run away") {}
    float GetValue(Action* action) override;
};

class ShadeOfAranFlameWreathDisableMovementMultiplier : public Multiplier
{
public:
    ShadeOfAranFlameWreathDisableMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "shade of aran flame wreath disable movement") {}
    float GetValue(Action* action) override;
};

class NetherspiteKeepBlockingBeamMultiplier : public Multiplier
{
public:
    NetherspiteKeepBlockingBeamMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "netherspite keep blocking beam") {}
    float GetValue(Action* action) override;
};

class NetherspiteWaitForDpsMultiplier : public Multiplier
{
public:
    NetherspiteWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "netherspite wait for dps") {}
    float GetValue(Action* action) override;
};

class PrinceMalchezaarEnfeebleMultiplier : public Multiplier
{
public:
    PrinceMalchezaarEnfeebleMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "prince malchezaar enfeeble") {}
    float GetValue(Action* action) override;
};

class PrinceMalchezaarDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    PrinceMalchezaarDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "prince malchezaar delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

class NightbaneDisablePetsMultiplier : public Multiplier
{
public:
    NightbaneDisablePetsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nightbane disable pets") {}
    float GetValue(Action* action) override;
};

class NightbaneWaitForDpsMultiplier : public Multiplier
{
public:
    NightbaneWaitForDpsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nightbane wait for dps") {}
    float GetValue(Action* action) override;
};

class NightbaneDisableAvoidAoeMultiplier : public Multiplier
{
public:
    NightbaneDisableAvoidAoeMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nightbane disable avoid aoe") {}
    float GetValue(Action* action) override;
};

class NightbaneDisableMovementMultiplier : public Multiplier
{
public:
    NightbaneDisableMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nightbane disable movement") {}
    float GetValue(Action* action) override;
};

#endif
