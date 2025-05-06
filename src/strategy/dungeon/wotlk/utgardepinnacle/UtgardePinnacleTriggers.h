#ifndef _PLAYERBOT_WOTLKDUNGEONUPTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONUPTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum UtgardePinnacleIDs
{
    // Skadi the Ruthless
    SPELL_FREEZING_CLOUD_N          = 47579,
    SPELL_FREEZING_CLOUD_H          = 60020,
    SPELL_FREEZING_CLOUD_BREATH     = 47592,
    NPC_BREATH_TRIGGER              = 28351,
    SPELL_SKADI_WHIRLWIND_N         = 50228,
    SPELL_SKADI_WHIRLWIND_H         = 59322,

    // King Ymiron
    SPELL_BANE_N                    = 48294,
    SPELL_BANE_H                    = 59301,
};

#define SPELL_FREEZING_CLOUD        DUNGEON_MODE(bot, SPELL_FREEZING_CLOUD_N, SPELL_FREEZING_CLOUD_H)
#define SPELL_SKADI_WHIRLWIND       DUNGEON_MODE(bot, SPELL_SKADI_WHIRLWIND_N, SPELL_SKADI_WHIRLWIND_H)
#define SPELL_BANE                  DUNGEON_MODE(bot, SPELL_BANE_N, SPELL_BANE_H)

// const float SKADI_BREATH_CENTRELINE = -512.46875f;

class SkadiFreezingCloudTrigger : public Trigger
{
public:
    SkadiFreezingCloudTrigger(PlayerbotAI* ai) : Trigger(ai, "skadi freezing cloud") {}
    bool IsActive() override;
};

class SkadiWhirlwindTrigger : public Trigger
{
public:
    SkadiWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "skadi whirlwind") {}
    bool IsActive() override;
};

class YmironBaneTrigger : public Trigger
{
public:
    YmironBaneTrigger(PlayerbotAI* ai) : Trigger(ai, "ymiron bane") {}
    bool IsActive() override;
};

#endif
