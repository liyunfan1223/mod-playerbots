/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MGTSHARED_H
#define PLAYERBOTS_MGTSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

namespace MagistersTerrace
{
constexpr uint32 MAP_MAGISTERS_TERRACE = 585;

constexpr float SELIN_LEASH_X   = 216.0f;
constexpr float SELIN_SAFE_X    = SELIN_LEASH_X + 4.0f;
constexpr float SELIN_REGROUP_X = 232.0f;
constexpr float SELIN_FLEE_SLACK = 6.0f;

constexpr float GROUND_TIER_STEP   = 3.0f;
constexpr float GROUND_SEARCH_UP   = 5.0f;
constexpr float GROUND_SEARCH_DOWN = 12.0f;

constexpr uint8 CONTROL_INTERRUPT_URGENCY = 2;

struct EscapeSpot
{
    float x;
    float y;
    float z;
};

using EscapeSpots = std::vector<EscapeSpot>;

struct LapseSpot
{
    float x;
    float y;
};

struct LapseWorld
{
    std::vector<Unit*> spheres;
    std::vector<LapseSpot> burns;
};

struct KiteState
{
    float turn = 0.0f;
    uint32 lastMs = 0;
};

enum class PhoenixRing : uint8
{
    None,
    Threatens,
    Covers
};

enum class FocusOrder
{
    None,
    Kael,
    Delrissa,
    Trash
};

Unit* GetSelin(Player* bot);
Unit* GetKaelthas(Player* bot);

Unit* GetActiveFelCrystal(Player* bot);

void CollectDampeningEscapes(Player* bot, EscapeSpots& out);

Unit* GetGlaiveThrowingMageGuard(Player* bot);

void CollectNovaEscapes(Player* bot, EscapeSpots& out);

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target, uint8 urgency);

Unit* GetInterruptTarget(Player* bot);

uint8 GetInterruptUrgency(Unit* caster);
uint8 GetRetinueInterruptUrgency(Unit* caster);

Unit* GetFocusTarget(Player* bot, ObjectGuid& latched);

ObjectGuid ResolveFocusOrder(PlayerbotAI* botAI);

FocusOrder ResolveFocusOrderOwner(PlayerbotAI* botAI);

void CollectFocusExclusions(Player* bot, std::vector<ObjectGuid>& out);

Unit* GetEnragedWretched(Player* bot);

std::vector<std::string> const& TauntSpellNames();

void ClampIntoRoom(float& x, float& y);

bool ShouldHoldTremorTotem(Player* bot);

Unit* GetDelrissaFocusTarget(Player* bot, ObjectGuid& latched);

void CollectDelrissaInterruptPreference(Player* bot, std::vector<Unit*>& out);

void CollectDelrissaPets(Player* bot, std::vector<ObjectGuid>& out);

void CollectFlameStrikeEscapes(Player* bot, EscapeSpots& out);

void CollectPhoenixEscapes(Player* bot, EscapeSpots& out);

PhoenixRing GetPhoenixRing(Player* bot);

Unit* GetKaelInterruptTarget(Player* bot);

bool IsKaelUnattackable(Player* bot);

Unit* GetKaelFocusTarget(Player* bot);

bool IsGravityLapseActive(Player* bot);

void CollectLapseWorld(Player* bot, LapseWorld& out);

bool GetGravityLapseKite(Player* bot, LapseWorld const& world, LapseSpot& out, KiteState& state);

bool GetGravityLapseSpot(Player* bot, LapseWorld const& world, LapseSpot& out);

bool GetGravityLapseDodge(Player* bot, LapseWorld const& world, LapseSpot& out);

bool IsLapseMeleeSlot(Player* bot);

bool OnLapseStation(Player* bot, Unit* kael, LapseSpot const& spot);

bool LapseThreatened(Player* bot, LapseWorld const& world);

bool GetLapseApproach(Player* bot, LapseWorld const& world, LapseSpot const& station, bool allowBlocked,
                      LapseSpot& out);

void CollectMeleePhoenixExclusions(Player* bot, std::vector<ObjectGuid>& out);
}

#endif
