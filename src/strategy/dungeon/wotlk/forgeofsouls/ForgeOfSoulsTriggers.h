#ifndef _PLAYERBOT_WOTLKDUNGEONFOSTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONFOSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum ForgeOfSoulsBronjahmIDs
{
    // Boss1
    NPC_CORRUPTED_SOUL_FRAGMENT = 36535,

    SPELL_CORRUPT_SOUL = 68839
};

class MoveFromBronjahmTrigger : public Trigger
{
public:
    MoveFromBronjahmTrigger(PlayerbotAI* ai) : Trigger(ai, "move from bronjahm") {}

    bool IsActive() override;
};

class SwitchToSoulFragment : public Trigger
{
public:
    SwitchToSoulFragment(PlayerbotAI* ai) : Trigger(ai, "switch to soul fragment") {}

    bool IsActive() override;
};

class BronjahmPositionTrigger : public Trigger
{
public:
    BronjahmPositionTrigger(PlayerbotAI* ai) : Trigger(ai, "bronjahm position") {}
    bool IsActive() override;
};

#endif
