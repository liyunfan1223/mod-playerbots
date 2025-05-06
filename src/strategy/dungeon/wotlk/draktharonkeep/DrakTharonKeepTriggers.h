#ifndef _PLAYERBOT_WOTLKDUNGEONDTKTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONDTKTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum DrakTharonIDs
{
    // Trollgore
    NPC_DRAKKARI_INVADER               = 27709,

    // Novos the Summoner
    NPC_NOVOS                          = 26631,
    SPELL_ARCANE_FIELD                 = 47346,
    NPC_CRYSTAL_HANDLER                = 26627,
    NPC_HULKING_CORPSE                 = 27597,
    NPC_RISEN_SHADOWCASTER             = 27600,
    NPC_FETID_TROLL_CORPSE             = 27598,

    // The Prophet Tharon'ja
    SPELL_GIFT_OF_THARONJA             = 52509,
};

class CorpseExplodeTrigger : public Trigger
{
public:
    CorpseExplodeTrigger(PlayerbotAI* ai) : Trigger(ai, "corpse explode") {}
    bool IsActive() override;
};

class ArcaneFieldTrigger : public Trigger
{
public:
    ArcaneFieldTrigger(PlayerbotAI* ai) : Trigger(ai, "arcane field") {}
    bool IsActive() override;
};

// class CrystalHandlerTrigger : public Trigger
// {
// public:
//     CrystalHandlerTrigger(PlayerbotAI* ai) : Trigger(ai, "crystal handler") {}
//     bool IsActive() override;
// };

class GiftOfTharonjaTrigger : public Trigger
{
public:
    GiftOfTharonjaTrigger(PlayerbotAI* ai) : Trigger(ai, "gift of tharon'ja") {}
    bool IsActive() override;
};

#endif
