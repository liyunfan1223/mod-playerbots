/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCACTIONS_H
#define PLAYERBOTS_TOCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "EquipAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemUsageValue.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "StatsWeightCalculator.h"
#include "TOCTriggers.h"

class ToCLanceAction : public AttackAction
{
public:
    ToCLanceAction(PlayerbotAI* botAI) : AttackAction(botAI, "toc lance") {}
    bool Execute(Event event) override;
};

class ToCUELanceAction : public AttackAction
{
public:
    ToCUELanceAction(PlayerbotAI* botAI) : AttackAction(botAI, "toc ue lance") {}
    bool Execute(Event event) override;
};

class ToCMountedAction : public MovementAction
{
public:
    ToCMountedAction(PlayerbotAI* botAI, std::string const name = "toc mounted")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    enum JoustMove : uint8
    {
        JOUST_MOVE_NONE,
        JOUST_MOVE_FLEE,
        JOUST_MOVE_KITE,
        JOUST_MOVE_APPROACH
    };

    void ScanArena(std::vector<Unit*>& hostiles, std::vector<Unit*>& walking);
    Unit* FindJoustTarget(std::vector<Unit*>& hostiles);
    Unit* FindTrampleTarget(std::vector<Unit*>& walking);
    std::vector<ObjectGuid> MountedRiders(bool tramplersOnly);
    size_t RiderLane(std::vector<ObjectGuid> const& riders);
    uint32 JoustSpellId(Unit* vehicleBase, uint8 slot);
    bool CastJoustSpell(uint32 spellId, Unit* target);
    bool AimAndCast(Unit* vehicleBase, uint32 spellId, Unit* target);
    bool DelaysCharge(Unit* vehicleBase, uint32 spellId, uint32 chargeWait);
    bool CastDefend(Unit* vehicleBase, uint8 belowStacks, uint32 chargeWait);
    bool RefreshDefend(Unit* vehicleBase, uint32 now);
    bool UseFillers(Unit* vehicleBase, Unit* target, float dist, uint32 chargeWait);
    bool ChargeCycle(Unit* vehicleBase, Unit* target, std::vector<Unit*> const& hostiles, float dist, uint32 now);
    void FleeDestination(Unit* target, std::vector<Unit*> const& hostiles, float& x, float& y);
    bool StartFlee(Unit* target, std::vector<Unit*> const& hostiles, uint32 now);
    void KiteDestination(Unit* target, float& x, float& y);
    bool Kite(Unit* vehicleBase, Unit* target, uint32 now);
    bool Approach(Unit* vehicleBase, Unit* target, uint32 now);
    bool KeepMoving(Unit* vehicleBase, JoustMove mode, uint32 now);
    bool MoveJoust(JoustMove mode, float x, float y, uint32 now);

    ObjectGuid spellIdsBase;
    uint32 spellIds[4] = {};
    Position moveDest;
    JoustMove moveMode = JOUST_MOVE_NONE;
    uint32 moveRefreshAt = 0;
    int8 kiteDir = 0;
    uint32 fleeUntil = 0;
    uint32 fleeBlockedUntil = 0;
    uint32 idleUntil = 0;
    uint32 defendCheckAt = 0;
};

class ToCMountAction : public MovementAction
{
public:
    ToCMountAction(PlayerbotAI* botAI, std::string const name = "toc mount")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
    static Unit* FindMount(PlayerbotAI* botAI);
};

class ToCEadricAction : public MovementAction
{
public:
    ToCEadricAction(PlayerbotAI* botAI, std::string const name = "toc eadric")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};
#endif
