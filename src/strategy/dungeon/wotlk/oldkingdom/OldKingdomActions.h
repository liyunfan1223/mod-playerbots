#ifndef _PLAYERBOT_WOTLKDUNGEONOKACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONOKACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "OldKingdomTriggers.h"

class AttackNadoxGuardianAction : public AttackAction
{
public:
    AttackNadoxGuardianAction(PlayerbotAI* ai) : AttackAction(ai, "attack nadox guardian") {}
    bool Execute(Event event) override;
};

class AttackJedogaVolunteerAction : public AttackAction
{
public:
    AttackJedogaVolunteerAction(PlayerbotAI* ai) : AttackAction(ai, "attack jedoga volunteer") {}
    bool Execute(Event event) override;
};

class AvoidShadowCrashAction : public MovementAction
{
public:
    AvoidShadowCrashAction(PlayerbotAI* ai) : MovementAction(ai, "avoid shadow crash") {}
    bool Execute(Event event) override;
};

#endif
