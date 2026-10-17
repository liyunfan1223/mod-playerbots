/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DTKTRIGGERS_H
#define PLAYERBOTS_DTKTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

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
    CorpseExplodeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "corpse explode") {}
    bool IsActive() override;
};

class ArcaneFieldTrigger : public Trigger
{
public:
    ArcaneFieldTrigger(PlayerbotAI* botAI) : Trigger(botAI, "arcane field") {}
    bool IsActive() override;
};

// class CrystalHandlerTrigger : public Trigger
// {
// public:
//     CrystalHandlerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "crystal handler") {}
//     bool IsActive() override;
// };

class GiftOfTharonjaTrigger : public Trigger
{
public:
    GiftOfTharonjaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gift of tharon'ja") {}
    bool IsActive() override;
};

#endif
