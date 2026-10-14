/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBSHARED_H
#define PLAYERBOTS_UBSHARED_H

#include "Define.h"
#include "ObjectGuid.h"

class AiObjectContext;
class Creature;
class Player;
class Unit;

namespace UnderbogHungarfen
{
constexpr uint32 NPC_HUNGARFEN = 17770;
constexpr uint32 NPC_UNDERBOG_MUSHROOM = 17990;
constexpr uint32 NPC_UNDERBAT = 17724;

constexpr uint32 SPELL_FOUL_SPORES = 31673;
constexpr uint32 SPELL_GROW        = 31698;
constexpr uint32 SPELL_SPORE_CLOUD = 34168;

constexpr float SPORE_CLOUD_RADIUS_FALLBACK = 8.0f;
constexpr float FOUL_SPORES_RADIUS_FALLBACK = 20.0f;

constexpr float SPORE_CLOUD_MARGIN = 1.5f;
constexpr float FOUL_SPORES_MARGIN = 2.0f;

constexpr uint8 GROW_STACKS_DANGER = 8;

constexpr float MUSHROOM_SCAN_RANGE = 40.0f;

constexpr float UNDERBAT_LASH_RANGE = 5.0f;
constexpr float UNDERBAT_LASH_MARGIN = 1.5f;
constexpr float UNDERBAT_SAFE_ARC = 2.6f;
constexpr uint32 UNDERBAT_REPOSITION_COOLDOWN = 750;

constexpr float UNDERBAT_RALLY_TOLERANCE = 3.0f;
constexpr float UNDERBAT_RALLY_MAX_RANGE = 20.0f;

constexpr uint32 MUSHROOM_SCAN_INTERVAL = 1000;
constexpr uint8 MUSHROOM_IDLE_SCAN_SKIPS = 9;
constexpr uint32 MUSHROOM_LINGER_TIME = 30000;

float MaxEffectRadius(uint32 spellId, float fallback);

Unit* HungarfenTarget(AiObjectContext* context);

ObjectGuid FindHungarfenGuid(Player* bot);

bool HungarfenGone(Player* bot, ObjectGuid guid);

float MushroomDangerRange(Player* bot);

GuidVector FindMushroomGuids(Player* bot);

bool IsMushroom(Unit* unit);

bool AnyMushroomAlive(Player* bot, GuidVector const& mushrooms);

Creature* GetNearestDangerousMushroom(Player* bot, GuidVector const& mushrooms, float range);

bool RetreatPathUnsafe(Player* bot, GuidVector const& mushrooms, float destX, float destY);

bool AnyUnderbatInLashRange(Player* bot, GuidVector const& attackers);

Creature* GetNearestUnderbatInLashRange(Player* bot, GuidVector const& attackers);

Creature* GetLashingUnderbat(Player* bot, GuidVector const& attackers);

Unit* UnderbatRallyUnit(Player* bot, GuidVector const& attackers);

bool SpotClearOfUnderbats(Player* bot, GuidVector const& attackers, float x, float y, float z);
}

#endif
