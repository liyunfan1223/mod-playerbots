/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SOTATACTICSVALUE_H
#define PLAYERBOTS_SOTATACTICSVALUE_H

#include "Value.h"

class BattlegroundSA;

enum class SotaObjective : uint8
{
    None,
    Gate,
    Graveyard,
    Relic,
    Defend
};

struct SotaTactics
{
    bool enabled = false;
    bool active = false;
    bool attacker = false;
    bool landingRequired = false;
    bool pilot = false;
    bool deploying = false;
    Position deploymentStart;
    Position deploymentEnd;
    Position deploymentWaypoint;
    uint8 lane = 0;
    uint8 stage = 0;
    uint32 gate = 0;
    bool transiting = false;
    uint32 transitGate = 0;
    SotaObjective objective = SotaObjective::None;
    Position destination;
    ObjectGuid objectiveGuid;
    ObjectGuid boardingVehicle;
    ObjectGuid enemyVehicle;
};

class SotaTacticsValue : public CalculatedValue<SotaTactics>
{
public:
    explicit SotaTacticsValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "sota tactics", 1000) {}

    void MarkLanded();

protected:
    SotaTactics Calculate() override;

private:
    uint32 _instanceId = 0;
    TeamId _attackerTeam = TEAM_NEUTRAL;
    uint8 _lane = 2;
    bool _landed = false;
    bool _defenderLaneAssigned = false;
};

BattlegroundSA* GetSotaBattleground(Player* bot);
SotaTacticsValue* GetSotaTacticsValue(PlayerbotAI* botAI);
SotaTactics GetSotaTactics(PlayerbotAI* botAI);
bool IsSotaInfantryReady(PlayerbotAI* botAI, SotaTactics const& tactics);
Unit* GetSotaDefenseTarget(PlayerbotAI* botAI, SotaTactics const& tactics);

#endif
