/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TOTEMSSHAMANSTRATEGY_H
#define _PLAYERBOT_TOTEMSSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"
#include <set>
#include <vector>
#include <string>

// This is the header with all of the totem-related constants and arrays used in the Shaman strategies.

// Totem Bar Slot Constants
#define TOTEM_BAR_SLOT_FIRE 132
#define TOTEM_BAR_SLOT_EARTH 133
#define TOTEM_BAR_SLOT_WATER 134
#define TOTEM_BAR_SLOT_AIR 135

// Strength of Earth Totem
static const uint32 STRENGTH_OF_EARTH_TOTEM[] = {
    58643,  // Rank 8
    57622,  // Rank 7
    25528,  // Rank 6
    25361,  // Rank 5
    10442,  // Rank 4
    8161,   // Rank 3
    8160,   // Rank 2
    8075    // Rank 1
};
static const size_t STRENGTH_OF_EARTH_TOTEM_COUNT = sizeof(STRENGTH_OF_EARTH_TOTEM) / sizeof(uint32);

// Stoneskin Totem
static const uint32 STONESKIN_TOTEM[] = {
    58753,  // Rank 10
    58751,  // Rank 9
    25509,  // Rank 8
    25508,  // Rank 7
    10408,  // Rank 6
    10407,  // Rank 5
    10406,  // Rank 4
    8155,   // Rank 3
    8154,   // Rank 2
    8071    // Rank 1
};
static const size_t STONESKIN_TOTEM_COUNT = sizeof(STONESKIN_TOTEM) / sizeof(uint32);

// Tremor Totem
static const uint32 TREMOR_TOTEM[] = {
    8143  // Rank 1
};
static const size_t TREMOR_TOTEM_COUNT = sizeof(TREMOR_TOTEM) / sizeof(uint32);

// Earthbind Totem
static const uint32 EARTHBIND_TOTEM[] = {
    2484  // Rank 1
};
static const size_t EARTHBIND_TOTEM_COUNT = sizeof(EARTHBIND_TOTEM) / sizeof(uint32);

// Stoneclaw Totem
static const uint32 STONECLAW_TOTEM[] = {
    58582,  // Rank 10
    58581,  // Rank 9
    58580,  // Rank 8
    25525,  // Rank 7
    10428,  // Rank 6
    10427,  // Rank 5
    6392,   // Rank 4
    6391,   // Rank 3
    6390,   // Rank 2
    5730    // Rank 1
};
static const size_t STONECLAW_TOTEM_COUNT = sizeof(STONECLAW_TOTEM) / sizeof(uint32);

// Searing Totem
static const uint32 SEARING_TOTEM[] = {
    58704,  // Rank 10
    58703,  // Rank 9
    58699,  // Rank 8
    25533,  // Rank 7
    10438,  // Rank 6
    10437,  // Rank 5
    6365,   // Rank 4
    6364,   // Rank 3
    6363,   // Rank 2
    3599    // Rank 1
};
static const size_t SEARING_TOTEM_COUNT = sizeof(SEARING_TOTEM) / sizeof(uint32);

// Magma Totem
static const uint32 MAGMA_TOTEM[] = {
    58734,  // Rank 7
    58731,  // Rank 6
    25552,  // Rank 5
    10587,  // Rank 4
    10586,  // Rank 3
    10585,  // Rank 2
    8190    // Rank 1
};
static const size_t MAGMA_TOTEM_COUNT = sizeof(MAGMA_TOTEM) / sizeof(uint32);

// Flametongue Totem
static const uint32 FLAMETONGUE_TOTEM[] = {
    58656,  // Rank 8
    58652,  // Rank 7
    58649,  // Rank 6
    25557,  // Rank 5
    16387,  // Rank 4
    10526,  // Rank 3
    8249,   // Rank 2
    8227    // Rank 1
};
static const size_t FLAMETONGUE_TOTEM_COUNT = sizeof(FLAMETONGUE_TOTEM) / sizeof(uint32);

// Totem of Wrath
static const uint32 TOTEM_OF_WRATH[] = {
    57722,  // Rank 4
    57721,  // Rank 3
    57720,  // Rank 2
    30706   // Rank 1
};
static const size_t TOTEM_OF_WRATH_COUNT = sizeof(TOTEM_OF_WRATH) / sizeof(uint32);

// Frost Resistance Totem
static const uint32 FROST_RESISTANCE_TOTEM[] = {
    58745,  // Rank 6
    58741,  // Rank 5
    25560,  // Rank 4
    10479,  // Rank 3
    10478,  // Rank 2
    8181    // Rank 1
};
static const size_t FROST_RESISTANCE_TOTEM_COUNT = sizeof(FROST_RESISTANCE_TOTEM) / sizeof(uint32);

// Fire Elemental Totem
static const uint32 FIRE_ELEMENTAL_TOTEM[] = {
    2894  // Rank 1
};
static const size_t FIRE_ELEMENTAL_TOTEM_COUNT = sizeof(FIRE_ELEMENTAL_TOTEM) / sizeof(uint32);

// Healing Stream Totem
static const uint32 HEALING_STREAM_TOTEM[] = {
    58757,  // Rank 9
    58756,  // Rank 8
    58755,  // Rank 7
    25567,  // Rank 6
    10463,  // Rank 5
    10462,  // Rank 4
    6377,   // Rank 3
    6375,   // Rank 2
    5394    // Rank 1
};
static const size_t HEALING_STREAM_TOTEM_COUNT = sizeof(HEALING_STREAM_TOTEM) / sizeof(uint32);

// Mana Spring Totem
static const uint32 MANA_SPRING_TOTEM[] = {
    58774,  // Rank 8
    58773,  // Rank 7
    58771,  // Rank 6
    25570,  // Rank 5
    10497,  // Rank 4
    10496,  // Rank 3
    10495,  // Rank 2
    5675    // Rank 1
};
static const size_t MANA_SPRING_TOTEM_COUNT = sizeof(MANA_SPRING_TOTEM) / sizeof(uint32);

// Cleansing Totem
static const uint32 CLEANSING_TOTEM[] = {
    8170  // Rank 1
};
static const size_t CLEANSING_TOTEM_COUNT = sizeof(CLEANSING_TOTEM) / sizeof(uint32);

// Fire Resistance Totem
static const uint32 FIRE_RESISTANCE_TOTEM[] = {
    58739,  // Rank 6
    58737,  // Rank 5
    25563,  // Rank 4
    10538,  // Rank 3
    10537,  // Rank 2
    8184    // Rank 1
};
static const size_t FIRE_RESISTANCE_TOTEM_COUNT = sizeof(FIRE_RESISTANCE_TOTEM) / sizeof(uint32);

// Mana Tide Totem
static const uint32 MANA_TIDE_TOTEM[] = {
    16190  // Rank 1
};
static const size_t MANA_TIDE_TOTEM_COUNT = sizeof(MANA_TIDE_TOTEM) / sizeof(uint32);

// Wrath of Air Totem
static const uint32 WRATH_OF_AIR_TOTEM[] = {
    3738  // Rank 1
};
static const size_t WRATH_OF_AIR_TOTEM_COUNT = sizeof(WRATH_OF_AIR_TOTEM) / sizeof(uint32);

// Windfury Totem
static const uint32 WINDFURY_TOTEM[] = {
    8512  // Rank 1
};
static const size_t WINDFURY_TOTEM_COUNT = sizeof(WINDFURY_TOTEM) / sizeof(uint32);

// Nature Resistance Totem
static const uint32 NATURE_RESISTANCE_TOTEM[] = {
    58749,  // Rank 6
    58746,  // Rank 5
    25574,  // Rank 4
    10601,  // Rank 3
    10600,  // Rank 2
    10595   // Rank 1
};
static const size_t NATURE_RESISTANCE_TOTEM_COUNT = sizeof(NATURE_RESISTANCE_TOTEM) / sizeof(uint32);

// Grounding Totem
static const uint32 GROUNDING_TOTEM[] = {
    8177  // Rank 1
};
static const size_t GROUNDING_TOTEM_COUNT = sizeof(GROUNDING_TOTEM) / sizeof(uint32);

class PlayerbotAI;

// Earth Totem Strategies
class StrengthOfEarthTotemStrategy : public GenericShamanStrategy
{
public:
    StrengthOfEarthTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "strength of earth"; }
};

class StoneclawTotemStrategy : public GenericShamanStrategy
{
public:
    StoneclawTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "stoneskin"; }
};

class EarthTotemStrategy : public GenericShamanStrategy
{
public:
    EarthTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tremor"; }
};

class EarthbindTotemStrategy : public GenericShamanStrategy
{
public:
    EarthbindTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "earthbind"; }
};

// Fire Totem Strategies
class SearingTotemStrategy : public GenericShamanStrategy
{
public:
    SearingTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "searing"; }
};

class MagmaTotemStrategy : public GenericShamanStrategy
{
public:
    MagmaTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "magma"; }
};

class FlametongueTotemStrategy : public GenericShamanStrategy
{
public:
    FlametongueTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "flametongue"; }
};

class TotemOfWrathStrategy : public GenericShamanStrategy
{
public:
    TotemOfWrathStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "wrath"; }
};

class FrostResistanceTotemStrategy : public GenericShamanStrategy
{
public:
    FrostResistanceTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost resistance"; }
};

// Water Totem Strategies
class HealingStreamTotemStrategy : public GenericShamanStrategy
{
public:
    HealingStreamTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "healing stream"; }
};

class ManaSpringTotemStrategy : public GenericShamanStrategy
{
public:
    ManaSpringTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "mana spring"; }
};

class CleansingTotemStrategy : public GenericShamanStrategy
{
public:
    CleansingTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cleansing"; }
};

class FireResistanceTotemStrategy : public GenericShamanStrategy
{
public:
    FireResistanceTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fire resistance"; }
};

// Air Totem Strategies
class WrathOfAirTotemStrategy : public GenericShamanStrategy
{
public:
    WrathOfAirTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "wrath of air"; }
};

class WindfuryTotemStrategy : public GenericShamanStrategy
{
public:
    WindfuryTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "windfury"; }
};

class NatureResistanceTotemStrategy : public GenericShamanStrategy
{
public:
    NatureResistanceTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "nature resistance"; }
};

class GroundingTotemStrategy : public GenericShamanStrategy
{
public:
    GroundingTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "grounding"; }
};

#endif
