/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEACTIONS_DRAKES_H
#define PLAYERBOTS_EOEACTIONS_DRAKES_H

#include "Action.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"

#include <vector>

// P3: sole owner of the drake's position. Nothing else may steer a Skytalon.
class EoEFlyDrakeAction : public MovementAction
{
public:
    static constexpr char const* Name = "eoe fly drake";

    EoEFlyDrakeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}

    bool Execute(Event event) override;
    bool isPossible() override;

private:
    uint32 _stackCalcAtMs = 0;
    float _stackX = 0.0f;
    float _stackY = 0.0f;
    float _stackZ = 0.0f;
    bool _stackValid = false;

    // Last destination handed to the MotionMaster, so the same one is not restamped every tick.
    float _issuedX = 0.0f;
    float _issuedY = 0.0f;
    bool _issued = false;
};

class EoEDrakeAttackAction : public Action
{
public:
    static constexpr char const* Name = "eoe drake attack";

    EoEDrakeAttackAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;

protected:
    bool CastDrakeSpell(Unit* target, uint32 spellId);
    bool DpsRotation(Unit* drake, Unit* target);
    bool HealRotation(Unit* drake, std::vector<ObjectGuid> const& healers);
};

class DrakeSurgeShieldAction : public Action
{
public:
    static constexpr char const* Name = "eoe drake surge shield";

    DrakeSurgeShieldAction(PlayerbotAI* botAI) : Action(botAI, Name) {}

    bool Execute(Event event) override;
    bool isPossible() override;
};

#endif
