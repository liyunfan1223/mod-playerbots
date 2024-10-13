#ifndef _PLAYERBOT_WOTLKDUNGEONGDTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONGDTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum GundrakIDs
{
    // Slad'ran
    SPELL_POISON_NOVA_N             = 55081,
    SPELL_POISON_NOVA_H             = 59842,
    
    // Gal'darah
    SPELL_WHIRLING_SLASH_N          = 55250,
    SPELL_WHIRLING_SLASH_H          = 59824,
};

#define SPELL_POISON_NOVA           DUNGEON_MODE(bot, SPELL_POISON_NOVA_N, SPELL_POISON_NOVA_H)
#define SPELL_WHIRLING_SLASH        DUNGEON_MODE(bot, SPELL_WHIRLING_SLASH_N, SPELL_WHIRLING_SLASH_H)

class SladranPoisonNovaTrigger : public Trigger
{
public:
    SladranPoisonNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "slad'ran poison nova") {}
    bool IsActive() override;
};

class GaldarahWhirlingSlashTrigger : public Trigger
{
public:
    GaldarahWhirlingSlashTrigger(PlayerbotAI* ai) : Trigger(ai, "gal'darah whirling slash") {}
    bool IsActive() override;
};

#endif
