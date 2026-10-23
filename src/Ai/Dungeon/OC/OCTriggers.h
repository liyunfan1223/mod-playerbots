/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCTRIGGERS_H
#define PLAYERBOTS_OCTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum OculusIDs
{
    // Drakos the Interrogator
    NPC_UNSTABLE_SPHERE             = 28166,
    SPELL_UNSTABLE_SPHERE_PASSIVE   = 50756,
    SPELL_UNSTABLE_SPHERE_PULSE     = 50757,
    SPELL_UNSTABLE_SPHERE_TIMER     = 50758,

    // Drakes
    NPC_AMBER_DRAKE                 = 27755,
    NPC_EMERALD_DRAKE               = 27692,
    NPC_RUBY_DRAKE                  = 27756,
    ITEM_AMBER_ESSENCE              = 37859,
    ITEM_EMERALD_ESSENCE            = 37815,
    ITEM_RUBY_ESSENCE               = 37860,
    SPELL_AMBER_ESSENCE             = 49461,
    SPELL_EMERALD_ESSENCE           = 49345,
    SPELL_RUBY_ESSENCE              = 49462,
    // Abilities:
    // Amber
    SPELL_SHOCK_LANCE               = 49840,
    SPELL_SHOCK_CHARGE              = 49836,
    SPELL_STOP_TIME                 = 49838,
    SPELL_TEMPORAL_RIFT             = 49592,
    // Emerald
    SPELL_LEECHING_POISON           = 50328,
    SPELL_TOUCH_THE_NIGHTMARE       = 50341,
    SPELL_DREAM_FUNNEL              = 50344,
    // Ruby
    SPELL_SEARING_WRATH             = 50232,
    SPELL_EVASIVE_MANEUVERS         = 50240,
    SPELL_EVASIVE_CHARGES           = 50241,
    SPELL_MARTYR                    = 50253,

    // Varos Cloudstrider
    NPC_CENTRIFUGE_CORE             = 28183,

    // Mage-Lord Urom
    NPC_MAGE_LORD_UROM              = 27655,
    SPELL_TIME_BOMB_N               = 51121,
    SPELL_TIME_BOMB_H               = 59376,
    SPELL_EMPOWERED_ARCANE_EXPLOSION_N = 51110,
    SPELL_EMPOWERED_ARCANE_EXPLOSION_H = 59377,

    // Ley-Guardian Eregos
    SPELL_ENRAGED_ASSAULT           = 51170,
    SPELL_PLANAR_SHIFT              = 51162,
};

#define SPELL_EMPOWERED_ARCANE_EXPLOSION        DUNGEON_MODE(bot, SPELL_EMPOWERED_ARCANE_EXPLOSION_N, SPELL_EMPOWERED_ARCANE_EXPLOSION_H)
#define SPELL_TIME_BOMB                         DUNGEON_MODE(bot, SPELL_TIME_BOMB_N, SPELL_TIME_BOMB_H)

const std::vector<uint32> DRAKE_ITEMS = {ITEM_AMBER_ESSENCE, ITEM_EMERALD_ESSENCE, ITEM_RUBY_ESSENCE};
const std::vector<uint32> DRAKE_SPELLS = {SPELL_AMBER_ESSENCE, SPELL_EMERALD_ESSENCE, SPELL_RUBY_ESSENCE};
const uint32 OCULUS_MAP_ID = 578;

// const float uromCoords[4][4] =
// {
//     {1177.47f, 937.722f, 527.405f, 2.21657f},
//     {968.66f, 1042.53f, 527.32f, 0.077f},
//     {1164.02f, 1170.85f, 527.321f, 3.66f},
//     {1118.31f, 1080.377f, 508.361f, 4.25f}      // Inner ring, actual boss fight
// };

class DrakosUnstableSphereTrigger : public Trigger
{
public:
    DrakosUnstableSphereTrigger(PlayerbotAI* botAI) : Trigger(botAI, "drakos unstable sphere") {}
    bool IsActive() override;
};

class DrakeMountTrigger : public Trigger
{
public:
    DrakeMountTrigger(PlayerbotAI* botAI) : Trigger(botAI, "drake mount") {}
    bool IsActive() override;
};

class DrakeDismountTrigger : public Trigger
{
public:
    DrakeDismountTrigger(PlayerbotAI* botAI) : Trigger(botAI, "drake dismount") {}
    bool IsActive() override;
};

class GroupFlyingTrigger : public Trigger
{
public:
    GroupFlyingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "drake fly") {}
    bool IsActive() override;
};

class DrakeCombatTrigger : public Trigger
{
public:
    DrakeCombatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "drake combat") {}
    bool IsActive() override;
};

class VarosCloudstriderTrigger : public Trigger
{
public:
    VarosCloudstriderTrigger(PlayerbotAI* botAI) : Trigger(botAI, "varos cloudstrider") {}
    bool IsActive() override;
};

class UromArcaneExplosionTrigger : public Trigger
{
public:
    UromArcaneExplosionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "urom arcane explosion") {}
    bool IsActive() override;
};

class UromTimeBombTrigger : public Trigger
{
public:
    UromTimeBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "urom time bomb") {}
    bool IsActive() override;
};

#endif
