#ifndef _PLAYERBOT_WOTLKDUNGEONANTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONANTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum AzjolNerubIDs
{
    // Krik'thir the Gatewatcher
    NPC_KRIKTHIR                    = 28684,
    NPC_WATCHER_SILTHIK             = 28731,
    NPC_WATCHER_GASHRA              = 28730,
    NPC_WATCHER_NARJIL              = 28729,
    NPC_WATCHER_SKIRMISHER          = 28734,
    NPC_WATCHER_SHADOWCASTER        = 28733,
    NPC_WATCHER_WARRIOR             = 28732,
    DEBUFF_WEB_WRAP                 = 52086,
    NPC_WEB_WRAP                    = 28619,

    // Anub'arak
    // Not sure how to track this - first one is cast as a buff on himself,
    // which triggers periodic casts of the spikes spell.
    SPELL_IMPALE_PERIODIC           = 53456,
    SPELL_IMPALE_SPIKES             = 53457,
    SPELL_POUND_N                   = 53472,
    SPELL_POUND_H                   = 59433,
};

#define SPELL_POUND                 DUNGEON_MODE(bot, SPELL_POUND_N, SPELL_POUND_H)

class KrikthirWebWrapTrigger : public Trigger
{
public:
    KrikthirWebWrapTrigger(PlayerbotAI* ai) : Trigger(ai, "krik'thir web wrap") {}
    bool IsActive() override;
};

class KrikthirWatchersTrigger : public Trigger
{
public:
    KrikthirWatchersTrigger(PlayerbotAI* ai) : Trigger(ai, "krik'thir watchers") {}
    bool IsActive() override;
};

// class AnubarakImpaleTrigger : public Trigger
// {
// public:
//     AnubarakImpaleTrigger(PlayerbotAI* ai) : Trigger(ai, "anub'arak impale") {}
//     bool IsActive() override;
// };

class AnubarakPoundTrigger : public Trigger
{
public:
    AnubarakPoundTrigger(PlayerbotAI* ai) : Trigger(ai, "anub'arak pound") {}
    bool IsActive() override;
};

#endif
