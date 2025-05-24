#ifndef _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAIConfig.h"
#include "GenericTriggers.h"
#include "DungeonStrategyUtils.h"

enum PitOfSaronIDs
{
    //NPCs
    NPC_GARFROST                        = 36494,
    NPC_KRICK                           = 36477,
    NPC_ICK                             = 36476,
    NPC_TYRANNUS                        = 36658,
    NPC_RIMEFANG                        = 36661,

    //Spells
    SPELL_PURSUIT                       = 68987,
    SPELL_POISON_NOVA_POS               = 68989,
    SPELL_POISON_NOVA_POS_HC            = 70434,
    SPELL_EXPLOSIVE_BARRAGE_KRICK       = 69012,
    SPELL_EXPLOSIVE_BARRAGE_ICK         = 69263,
    SPELL_EXPLOSIVE_BARRAGE_SUMMON      = 69015,
    SPELL_EXPLODING_ORB_VISUAL          = 69017,
    SPELL_MARK_OF_RIMEFANG              = 69275,
    RIMEFANG_SPELL_HOARFROST            = 69246,
    RIMEFANG_SPELL_HOARFROST_HC         = 69245,
    RIMEFANG_SPELL_HOARFROST_HC2        = 69645,
};

class IckAndKrickTrigger : public Trigger
{
public:
    IckAndKrickTrigger(PlayerbotAI* ai) : Trigger(ai, "ick and krick") {}

    bool IsActive() override;
};

class TyrannusTrigger : public Trigger
{
public:
    TyrannusTrigger(PlayerbotAI* ai) : Trigger(ai, "tyrannus") {}

    bool IsActive() override;
};

#endif
