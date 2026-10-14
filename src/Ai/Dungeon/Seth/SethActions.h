/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHACTIONS_H
#define PLAYERBOTS_SETHACTIONS_H

#include "Action.h"
#include "MovementActions.h"

class TimeLostControllerMarkCharmingTotemWithSkullAction : public Action
{
public:
    TimeLostControllerMarkCharmingTotemWithSkullAction(PlayerbotAI* botAI)
        : Action(botAI, "time-lost controller mark charming totem with skull") {}
    bool Execute(Event event) override;
};

class SethekkProphetSetTremorTotemAction : public Action
{
public:
    SethekkProphetSetTremorTotemAction(PlayerbotAI* botAI)
        : Action(botAI, "sethekk prophet set tremor totem") {}
    bool Execute(Event event) override;
};

class DarkweaverSythMarkElementalsWithSkullAction : public Action
{
public:
    DarkweaverSythMarkElementalsWithSkullAction(PlayerbotAI* botAI)
        : Action(botAI, "darkweaver syth mark elementals with skull") {}
    bool Execute(Event event) override;
};

class AnzuAlternateMarksOnBossAction : public Action
{
public:
    AnzuAlternateMarksOnBossAction(PlayerbotAI* botAI)
        : Action(botAI, "anzu alternate marks on boss") {}
    bool Execute(Event event) override;
};

class AnzuCastHealOverTimeSpellOnBirdSpiritAction : public Action
{
public:
    AnzuCastHealOverTimeSpellOnBirdSpiritAction(PlayerbotAI* botAI)
        : Action(botAI, "anzu cast heal over time spell on bird spirit") {}
    bool Execute(Event event) override;
};

class TalonKingIkissTankMoveBossToPillarPositionAction : public MovementAction
{
public:
    TalonKingIkissTankMoveBossToPillarPositionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "talon king ikiss tank move boss to pillar position") {}
    bool Execute(Event event) override;

private:
    bool _hasReachedPillarPosition = false;
};

class TalonKingIkissRangedStayNearVictimOfBossAction : public MovementAction
{
public:
    TalonKingIkissRangedStayNearVictimOfBossAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "talon king ikiss ranged stay near victim of boss") {}
    bool Execute(Event event) override;
};

class TalonKingIkissLosArcaneExplosionAction : public MovementAction
{
public:
    TalonKingIkissLosArcaneExplosionAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "talon king ikiss los arcane explosion") {}
    bool Execute(Event event) override;

private:
    bool MoveToPillar(Position const& pillarCenter, float botAngle, float distToPillar);
    bool MoveAroundPillar(Position const& pillarCenter, float distToPillar);
};

class TalonKingIkissMoveToWithinLosAction : public MovementAction
{
public:
    TalonKingIkissMoveToWithinLosAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "talon king ikiss move to within los") {}
    bool Execute(Event event) override;
};

#endif
