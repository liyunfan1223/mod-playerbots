/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSTRIGGERS_H
#define PLAYERBOTS_POSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"
#include <unordered_map>

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
    SPELL_TOXIC_WASTE                   = 69024,
    SPELL_TOXIC_WASTE_HC                = 70436,
    SPELL_ICY_BLAST_AREA                = 69238,
    SPELL_ICY_BLAST_AREA_HC             = 69628,
    SPELL_OVERLORDS_BRAND               = 69172,
    SPELL_PERMAFROST_DEBUFF             = 68786,
    SPELL_PERMAFROST_DEBUFF_HC          = 70336,
    SPELL_THROW_SARONITE                = 68788,
    SPELL_THUNDERING_STOMP              = 68771,
    SPELL_FORGE_BLADE                   = 68774,
    SPELL_FORGE_MACE                    = 68785,
    GO_SARONITE_ROCK                    = 196485,
};

// Melee DPS duck behind a boulder at this many Permafrost stacks; ticks every 2 s, gone 3 s after the last one
constexpr uint32 GARFROST_MELEE_HIDE_STACKS = 8;
// Throw Saronite lands with a 6 yd impact
constexpr float GARFROST_SARONITE_AVOID_RADIUS = 7.5f;

uint32 PermafrostStacks(Unit* unit);
bool IsGarfrostForging(Unit* boss);
bool IsShieldedFromGarfrost(Unit* boss, Unit* unit);
bool GarfrostWantsCover(Player* bot, Unit* boss);
bool GetSaroniteLanding(Unit* boss, Position& landing);
bool BrandedHealerMayHeal(Player* bot, Unit* boss);

// Persistent ground effects (Toxic Waste pools, Icy Blast patches): centre and radius
struct PoSGroundHazard
{
    float x, y, radius;
};

std::vector<PoSGroundHazard> FindPoSGroundHazards(WorldObject* searcher);
bool InPoSGroundHazard(std::vector<PoSGroundHazard> const& hazards, float x, float y, float margin);
Unit* FindPoSBoss(PlayerbotAI* botAI);
bool IsPoisonNovaCasting(Unit* boss);

// Poison Nova reaches 15 yd; bots clear it to this distance
constexpr float POISON_NOVA_SAFE_DISTANCE = 18.0f;

// Pets Overlord's Brand made passive, with the stance to give back once it fades
void TyrannusRememberSuspendedPet(Player* bot, Creature* pet);
bool TyrannusHasSuspendedPets(Player* bot);
std::unordered_map<ObjectGuid, ReactStates> TyrannusTakeSuspendedPets(Player* bot);

class IckAndKrickTrigger : public Trigger
{
public:
    IckAndKrickTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ick and krick") {}

    bool IsActive() override;
};

class TyrannusTrigger : public Trigger
{
public:
    TyrannusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "tyrannus") {}

    bool IsActive() override;
};

class GarfrostPermafrostTrigger : public Trigger
{
public:
    GarfrostPermafrostTrigger(PlayerbotAI* botAI) : Trigger(botAI, "garfrost permafrost") {}
    bool IsActive() override;
};

class GarfrostTankPositionTrigger : public Trigger
{
public:
    GarfrostTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "garfrost tank position") {}
    bool IsActive() override;
};

class GarfrostSaroniteTrigger : public Trigger
{
public:
    GarfrostSaroniteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "garfrost saronite") {}
    bool IsActive() override;
};

class PoSGroundHazardTrigger : public Trigger
{
public:
    PoSGroundHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "pos ground hazard") {}
    bool IsActive() override;
};

class TyrannusOverlordsBrandTrigger : public Trigger
{
public:
    TyrannusOverlordsBrandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "tyrannus overlords brand") {}
    bool IsActive() override;
};

class TyrannusOverlordsBrandFadedTrigger : public Trigger
{
public:
    TyrannusOverlordsBrandFadedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "tyrannus overlords brand faded") {}
    bool IsActive() override;
};

class TyrannusBrandStopCastTrigger : public Trigger
{
public:
    TyrannusBrandStopCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "tyrannus brand stop cast") {}
    bool IsActive() override;
};

#endif
