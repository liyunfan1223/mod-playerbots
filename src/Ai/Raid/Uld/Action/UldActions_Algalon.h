/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_ALGALON_H
#define PLAYERBOTS_ULDACTIONS_ALGALON_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Position.h"
#include "RaidRedirectThreat.h"
#include "UldTriggers.h"
#include "Vehicle.h"

// Algalon the Observer

class AlgalonBigBangSoakAction : public Action
{
public:
    static constexpr char const* Name = "algalon big bang soak action";

    AlgalonBigBangSoakAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonBigBangExternalAction : public Action
{
public:
    static constexpr char const* Name = "algalon big bang external action";

    AlgalonBigBangExternalAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonSoakerPrepAction : public Action
{
public:
    static constexpr char const* Name = "algalon soaker prep action";

    AlgalonSoakerPrepAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonSoakerRescueAction : public Action
{
public:
    static constexpr char const* Name = "algalon soaker rescue action";

    AlgalonSoakerRescueAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonTankHealAction : public Action
{
public:
    static constexpr char const* Name = "algalon tank heal action";

    AlgalonTankHealAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonTankBeaconAction : public Action
{
public:
    static constexpr char const* Name = "algalon tank beacon action";

    AlgalonTankBeaconAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonTankDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "algalon tank defensive action";

    AlgalonTankDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonExplosionCoverAction : public Action
{
public:
    static constexpr char const* Name = "algalon explosion cover action";

    AlgalonExplosionCoverAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonExplosionPersonalAction : public Action
{
public:
    static constexpr char const* Name = "algalon explosion personal action";

    AlgalonExplosionPersonalAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonShadowAuraAction : public Action
{
public:
    static constexpr char const* Name = "algalon shadow aura action";

    AlgalonShadowAuraAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonBigBangHideAction : public MovementAction
{
public:
    static constexpr char const* Name = "algalon big bang hide action";

    AlgalonBigBangHideAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    // Its own last walk, so a booking another action made can be told apart and dropped.
    Position _walk;
};

class AlgalonCosmicSmashAction : public MovementAction
{
public:
    static constexpr char const* Name = "algalon cosmic smash action";

    AlgalonCosmicSmashAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonLeaveBlackHoleAction : public MovementAction
{
public:
    static constexpr char const* Name = "algalon leave black hole action";

    AlgalonLeaveBlackHoleAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    // The exit is searched round the bot every tick, so the booking can't say whose it is.
    Position _walk;
};

class AlgalonYieldBossAction : public Action
{
public:
    static constexpr char const* Name = "algalon yield boss action";

    AlgalonYieldBossAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonThreatRedirectAction : public RaidRedirectThreatAction
{
public:
    static constexpr char const* Name = "algalon threat redirect action";

    AlgalonThreatRedirectAction(PlayerbotAI* botAI) : RaidRedirectThreatAction(botAI, Name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

// Taunts Algalon onto this tank, and walks to him if the taunt can't reach.
class AlgalonTakeBossAction : public AttackAction
{
public:
    AlgalonTakeBossAction(PlayerbotAI* botAI, char const* name) : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AlgalonTankPickupAction : public AlgalonTakeBossAction
{
public:
    static constexpr char const* Name = "algalon tank pickup action";

    AlgalonTankPickupAction(PlayerbotAI* botAI) : AlgalonTakeBossAction(botAI, Name) {}
};

class AlgalonPhasePunchSwapAction : public AlgalonTakeBossAction
{
public:
    static constexpr char const* Name = "algalon phase punch swap action";

    AlgalonPhasePunchSwapAction(PlayerbotAI* botAI) : AlgalonTakeBossAction(botAI, Name) {}
};

class AlgalonConstellationTauntAction : public AttackAction
{
public:
    static constexpr char const* Name = "algalon constellation taunt action";

    AlgalonConstellationTauntAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonDarkMatterTankAction : public AttackAction
{
public:
    static constexpr char const* Name = "algalon dark matter tank action";

    AlgalonDarkMatterTankAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonConstellationKiteAction : public MovementAction
{
public:
    static constexpr char const* Name = "algalon constellation kite action";

    AlgalonConstellationKiteAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    Position _walk;
};

class AlgalonStarTeamAction : public AttackAction
{
public:
    static constexpr char const* Name = "algalon star team action";

    AlgalonStarTeamAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonStarMarkAction : public Action
{
public:
    static constexpr char const* Name = "algalon star mark action";

    AlgalonStarMarkAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class AlgalonRaidPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "algalon raid position action";

    AlgalonRaidPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    bool _slotReached = false;
};

#endif
