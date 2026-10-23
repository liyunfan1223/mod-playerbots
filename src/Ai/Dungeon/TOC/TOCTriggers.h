/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCTRIGGERS_H
#define PLAYERBOTS_TOCTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum TocC_IDs
{
    // Spells

    // Horse
    SPELL_DEFEND                    = 66482,
    SPELL_BOSS_DEFEND               = 62719,
    SPELL_MINIONS_DEFEND            = 64100,

    //Eadric
    SPELL_RADIANCE                  = 66935,

    // Objects
    OBJECT_LANCE_RACK               = 196398,

    // Items
    ITEM_LANCE                      = 46106,

    // Vehicles
    NPC_ARGENT_WARHORSE             = 35644,
    NPC_ARGENT_BATTLEWORG           = 36558,

    // Horde Champions
    NPC_MOKRA                       = 35572,
    NPC_ERESSEA                     = 35569,
    NPC_RUNOK                       = 35571,
    NPC_ZULTORE                     = 35570,
    NPC_VISCERI                     = 35617,

    // Alliance Champions
    NPC_JACOB                       = 34705,
    NPC_AMBROSE                     = 34702,
    NPC_COLOSOS                     = 34701,
    NPC_JAELYNE                     = 34657,
    NPC_LANA                        = 34703,

    // Grand Champion Minions
    NPC_IRONFORGE_MINION            = 35329,
    NPC_STORMWIND_MINION            = 35328,
    NPC_GNOMEREGAN_MINION           = 35331,
    NPC_EXODAR_MINION               = 35330,
    NPC_DARNASSUS_MINION            = 35332,
    NPC_ORGRIMMAR_MINION            = 35314,
    NPC_SILVERMOON_MINION           = 35326,
    NPC_THUNDER_BLUFF_MINION        = 35325,
    NPC_SENJIN_MINION               = 35323,
    NPC_UNDERCITY_MINION            = 35327,

    // Rest of the bosses and npcs
    NPC_EADRIC                      = 35119,
    NPC_EADRIC_H                    = 35518,
    NPC_PALETRESS                   = 34928,
    NPC_PALETRESS_H                 = 35517,

    NPC_ARGENT_LIGHTWIELDER         = 35309,
    NPC_ARGENT_MONK                 = 35305,
    NPC_PRIESTESS                   = 35307,

    NPC_BLACK_KNIGHT                = 35451,

    NPC_JAEREN                      = 35004,  // Horde
    NPC_ARELAS                      = 35005,  // Alliance
    NPC_RISEN_JAEREN                = 35545,
    NPC_RISEN_ARELAS                = 35564,
    NPC_TIRION                      = 33628,  // Possibly wrong npc here, 34996 had already populated creature_text (from ToC25) that
                                              // matches. Needs a sniff to confirm.

};

const std::vector<uint32> availableChampions = {
    NPC_MOKRA,   NPC_ERESSEA, NPC_RUNOK,   NPC_ZULTORE, NPC_VISCERI,
    NPC_AMBROSE, NPC_COLOSOS, NPC_JAELYNE, NPC_LANA,    NPC_JACOB,
};

const std::vector<uint32> availableTargets = {
    NPC_MOKRA,            NPC_ERESSEA,           NPC_RUNOK,                NPC_ZULTORE,          NPC_VISCERI,
    NPC_AMBROSE,          NPC_COLOSOS,           NPC_JAELYNE,              NPC_LANA,             NPC_JACOB,
    NPC_STORMWIND_MINION, NPC_GNOMEREGAN_MINION, NPC_EXODAR_MINION,        NPC_DARNASSUS_MINION, NPC_IRONFORGE_MINION,
    NPC_ORGRIMMAR_MINION, NPC_SILVERMOON_MINION, NPC_THUNDER_BLUFF_MINION, NPC_SENJIN_MINION,    NPC_UNDERCITY_MINION,
};

class ToCLanceTrigger : public Trigger
{
public:
    ToCLanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc lance", 500) {}
    bool IsActive() override;
};

class ToCUELanceTrigger : public Trigger
{
public:
    ToCUELanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc ue lance", 500) {}
    bool IsActive() override;
};

class ToCMountedTrigger : public Trigger
{
public:
    ToCMountedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc mounted") {}
    bool IsActive() override;
};

class ToCMountNearTrigger : public Trigger
{
public:
    ToCMountNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc mount near", 500) {}
    bool IsActive() override;
};

class ToCEadricTrigger : public Trigger
{
public:
    ToCEadricTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc eadric") {}
    bool IsActive() override;
};

#endif
