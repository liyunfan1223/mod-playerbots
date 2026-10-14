/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BWLACTIONS_H
#define PLAYERBOTS_BWLACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// General

class BwlOnyxiaScaleCloakAuraCheckAction : public Action
{
public:
    BwlOnyxiaScaleCloakAuraCheckAction(PlayerbotAI* botAI) : Action(botAI, "bwl onyxia scale cloak aura check") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class BwlTurnOffSuppressionDeviceAction : public Action
{
public:
    BwlTurnOffSuppressionDeviceAction(PlayerbotAI* botAI) : Action(botAI, "bwl turn off suppression device") {}
    bool Execute(Event event) override;
};

// Razorgore the Untamed

class BwlRazorgoreAvoidAoeAction : public MovementAction
{
public:
    BwlRazorgoreAvoidAoeAction(PlayerbotAI* botAI) : MovementAction(botAI, "bwl razorgore avoid aoe") {}
    bool Execute(Event event) override;
};

class BwlRazorgoreMarkBossAction : public AttackAction
{
public:
    BwlRazorgoreMarkBossAction(PlayerbotAI* botAI) : AttackAction(botAI, "bwl razorgore mark boss") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Vaelastrasz the Corrupt

class BwlVaelastraszMoveAwayAction : public MovementAction
{
public:
    BwlVaelastraszMoveAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "bwl vaelastrasz move away") {}
    bool Execute(Event event) override;

private:
    bool CalculateFleeDirection(Unit const* boss, float& fleeX, float& fleeY) const;
    bool MoveAlongFleeDirection(Unit const* boss, float fleeX, float fleeY);
};

// Broodlord Lashlayer

class BwlBroodlordRangedMoveAwayAction : public MovementAction
{
public:
    BwlBroodlordRangedMoveAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "bwl broodlord ranged move away") {}
    bool Execute(Event event) override;
};

// Firemaw / Ebonroc / Flamegor

class BwlBlackDrakeAvoidBreathAction : public BossRearFlankAction
{
public:
    BwlBlackDrakeAvoidBreathAction(PlayerbotAI* botAI, std::string const& bossName,
                                   float minAngle = ANGLE_45_DEG)
        : BossRearFlankAction(botAI, bossName, 0.0f, minAngle, M_PI) {}
    bool isUseful() override;
};

// Chromaggus

class BwlUseHourglassSandAction : public Action
{
public:
    BwlUseHourglassSandAction(PlayerbotAI* botAI) : Action(botAI, "bwl use hourglass sand") {}
    bool Execute(Event event) override;
};

// Trash

class BwlDeathTalonWyrmguardTankMoveAwayAction : public MovementAction
{
public:
    BwlDeathTalonWyrmguardTankMoveAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "bwl death talon wyrmguard tank move away") {}
    Unit* GetTarget() override;
    bool isUseful() override;
    bool Execute(Event event) override;
};

class BwlDeathTalonWyrmguardRangedMoveAwayAction : public MovementAction
{
public:
    BwlDeathTalonWyrmguardRangedMoveAwayAction(PlayerbotAI* botAI) : MovementAction(botAI, "bwl death talon wyrmguard ranged move away") {}
    Unit* GetTarget() override;
    bool Execute(Event event) override;
};

#endif
