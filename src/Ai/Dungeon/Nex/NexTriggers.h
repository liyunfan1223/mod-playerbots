/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXTRIGGERS_H
#define PLAYERBOTS_NEXTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum NexusIDs
{
    // Faction Commander
    NPC_ALLIANCE_COMMANDER          = 27949,
    NPC_HORDE_COMMANDER             = 27947,
    NPC_COMMANDER_STOUTBEARD        = 26796,
    NPC_COMMANDER_KOLURG            = 26798,
    // SPELL_FRIGHTENING_SHOUT         = 19134,
    SPELL_WHIRLWIND                 = 38618,

    // Grand Magus Telestra
    NPC_TELESTRA                    = 26731,
    NPC_FIRE_MAGUS                  = 26928,
    NPC_FROST_MAGUS                 = 26930,
    NPC_ARCANE_MAGUS                = 26929,

    // Anomalus
    BUFF_RIFT_SHIELD                = 47748,

    // Ormorok the Tree Shaper
    // NPC_CRYSTAL_SPIKE               = 27099,
    GO_CRYSTAL_SPIKE                = 188537,
};

class FactionCommanderWhirlwindTrigger : public Trigger
{
public:
    FactionCommanderWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction commander whirlwind") {}
    bool IsActive() override;
};

class TelestraFirebombTrigger : public Trigger
{
public:
    TelestraFirebombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "telestra firebomb spread") {}
    bool IsActive() override;
};

class TelestraSplitPhaseTrigger : public Trigger
{
public:
    TelestraSplitPhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "telestra split phase") {}
    bool IsActive() override;
};

class ChaoticRiftTrigger : public Trigger
{
public:
    ChaoticRiftTrigger(PlayerbotAI* botAI) : Trigger(botAI, "chaotic rift") {}
    bool IsActive() override;
};

class OrmorokSpikesTrigger : public Trigger
{
public:
    OrmorokSpikesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ormorok spikes") {}
    bool IsActive() override;
};

class OrmorokStackTrigger : public Trigger
{
public:
    OrmorokStackTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ormorok stack") {}
    bool IsActive() override;
};

class IntenseColdTrigger : public Trigger
{
public:
    IntenseColdTrigger(PlayerbotAI* botAI) : Trigger(botAI, "intense cold") {}
    bool IsActive() override;
};

class KeristraszaPositioningTrigger : public Trigger
{
public:
    KeristraszaPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "keristrasza positioning") {}
    bool IsActive() override;
};

#endif
