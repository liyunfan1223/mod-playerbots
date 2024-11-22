#ifndef _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum PitOfSaronIckIDs
{
    // Boss1
    NPC_EXPLODING_ORB = 36610,

    SPELL_PURSUIT = 68987,
    SPELL_POISON_NOVA_NORMAL = 68989,
    SPELL_POISON_NOVA_HEROIC = 70434,
    SPELL_EXPLOSIVE_BARRAGE_NORMAL = 69012,
    SPELL_EXPLOSIVE_BARRAGE_HEROIC = 69263,
};

class MoveFromIckPursuitTrigger : public Trigger
{
public:
    MoveFromIckPursuitTrigger(PlayerbotAI* ai) : Trigger(ai, "move from ick pursuit") {}

    bool IsActive() override;
};

class MoveFromPoisonNovaTrigger : public Trigger
{
public:
    MoveFromPoisonNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "move from poison nova") {}

    bool IsActive() override;
};

class MoveFromExplodingOrbTrigger : public Trigger
{
public:
    MoveFromExplodingOrbTrigger(PlayerbotAI* ai) : Trigger(ai, "move from exploding orb") {}
    bool IsActive() override;
};

#endif
