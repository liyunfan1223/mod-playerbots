#ifndef _PLAYERBOT_WOTLKDUNGEONVHACTIONS_H
#define _PLAYERBOT_WOTLKDUNGEONVHACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "VioletHoldTriggers.h"

// const Position NOVOS_PARTY_POSITION = Position(-378.852f, -760.349f, 28.587f);

class AttackErekemAction : public AttackAction
{
public:
    AttackErekemAction(PlayerbotAI* ai) : AttackAction(ai, "attack erekem") {}
    bool Execute(Event event) override;
};

class AttackIchoronElementalsAction : public AttackAction
{
public:
    AttackIchoronElementalsAction(PlayerbotAI* ai) : AttackAction(ai, "attack ichoron elementals") {}
    bool Execute(Event event) override;
};

class AttackIchorGlobuleAction : public AttackAction
{
public:
    AttackIchorGlobuleAction(PlayerbotAI* ai) : AttackAction(ai, "attack ichor globule") {}
    bool Execute(Event event) override;
};

class AttackVoidSentryAction : public AttackAction
{
public:
    AttackVoidSentryAction(PlayerbotAI* ai) : AttackAction(ai, "attack void sentry") {}
    bool Execute(Event event) override;
};

class StopAttackAction : public Action
{
public:
    StopAttackAction(PlayerbotAI* ai) : Action(ai, "stop attack") {}
    bool Execute(Event event) override;
};

#endif
