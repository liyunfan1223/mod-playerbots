/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_AURIAYA_H
#define PLAYERBOTS_ULDACTIONS_AURIAYA_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidAntiFear.h"
#include "UldEncounter_Auriaya.h"
#include "UldTriggers.h"
#include "Vehicle.h"

class AuriayaFallFromFloorAction : public Action
{
public:
    static constexpr char const* Name = "auriaya fall from floor action";

    AuriayaFallFromFloorAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Leashed to the anchor. Not MoveAwayFromCreatureAction: maximising distance from the nearest pool
// walks bots out of the room once the pools pile up.
class AuriayaSeepingEssenceAction : public MovementAction
{
public:
    static constexpr char const* Name = "auriaya seeping essence action";

    AuriayaSeepingEssenceAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AuriayaSentryTauntAction : public Action
{
public:
    static constexpr char const* Name = "auriaya sentry taunt action";

    AuriayaSentryTauntAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AuriayaRaidPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "auriaya raid position action";

    AuriayaRaidPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class AuriayaSetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "auriaya set dps priority action";

    AuriayaSetDpsPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool IsAllowedPriorityTarget(Unit* boss, Unit* candidate);
};

class AuriayaAntiFearAction : public RaidAntiFearAction
{
public:
    static constexpr char const* Name = "auriaya anti fear action";

    AuriayaAntiFearAction(PlayerbotAI* botAI) : RaidAntiFearAction(botAI, Name) {}

protected:
    bool FearWindowActive() override { return AuriayaFearWindowActive(botAI); }
};

#endif
