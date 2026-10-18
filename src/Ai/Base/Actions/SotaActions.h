/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SOTAACTIONS_H
#define PLAYERBOTS_SOTAACTIONS_H

#include "AttackAction.h"
#include "SotaTacticsValue.h"

bool IsSotaGroundPathClear(PlayerbotAI* botAI, Unit* mover, Movement::PointsArray const& points);

class SotaPrepareAction : public Action
{
public:
    explicit SotaPrepareAction(PlayerbotAI* botAI) : Action(botAI, "sota prepare") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class SotaLandingAction : public MovementAction
{
public:
    explicit SotaLandingAction(PlayerbotAI* botAI) : MovementAction(botAI, "sota land") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class SotaMoveAction : public MovementAction
{
public:
    explicit SotaMoveAction(PlayerbotAI* botAI, std::string const name = "sota move") : MovementAction(botAI, name) {}
    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    virtual Position GetDestination(SotaTactics const& tactics) const { return tactics.destination; }
};

class SotaDeployAction : public SotaMoveAction
{
public:
    explicit SotaDeployAction(PlayerbotAI* botAI) : SotaMoveAction(botAI, "sota deploy") {}
    bool isUseful() override;
    bool Execute(Event event) override;

protected:
    Position GetDestination(SotaTactics const& tactics) const override { return tactics.deploymentWaypoint; }

private:
    uint32 _lastDeploymentReport = 0;
};

class SotaFallBackAction : public SotaMoveAction
{
public:
    explicit SotaFallBackAction(PlayerbotAI* botAI) : SotaMoveAction(botAI, "sota fall back") {}
    bool isUseful() override;
};

class SotaInteractAction : public Action
{
public:
    explicit SotaInteractAction(PlayerbotAI* botAI) : Action(botAI, "sota interact") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    ObjectGuid _objectGuid;
    uint32 _startedAt = 0;
    uint32 _delay = 0;
};

class SotaEnterVehicleAction : public MovementAction
{
public:
    explicit SotaEnterVehicleAction(PlayerbotAI* botAI) : MovementAction(botAI, "sota enter vehicle") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastPathReport = 0;
};

class SotaDriveVehicleAction : public MovementAction
{
public:
    explicit SotaDriveVehicleAction(PlayerbotAI* botAI) : MovementAction(botAI, "sota drive vehicle") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastPathReport = 0;
};

class SotaLeaveVehicleAction : public Action
{
public:
    explicit SotaLeaveVehicleAction(PlayerbotAI* botAI) : Action(botAI, "sota leave vehicle") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class SotaFireVehicleAction : public Action
{
public:
    explicit SotaFireVehicleAction(PlayerbotAI* botAI) : Action(botAI, "sota fire vehicle") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class SotaAttackDemolisherAction : public AttackAction
{
public:
    explicit SotaAttackDemolisherAction(PlayerbotAI* botAI) : AttackAction(botAI, "sota attack demolisher") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
