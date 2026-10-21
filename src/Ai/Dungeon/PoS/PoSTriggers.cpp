/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSTriggers.h"
#include "AiObjectContext.h"
#include "Cell.h"
#include "CellImpl.h"
#include "DynamicObject.h"
#include "GridNotifiers.h"
#include "GameObject.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include <mutex>

namespace
{
// Bots on different maps update in parallel
std::mutex tyrannusPetsLock;
std::unordered_map<ObjectGuid, std::unordered_map<ObjectGuid, ReactStates>> tyrannusSuspendedPets;
}

void TyrannusRememberSuspendedPet(Player* bot, Creature* pet)
{
    std::lock_guard<std::mutex> guard(tyrannusPetsLock);
    tyrannusSuspendedPets[bot->GetGUID()].emplace(pet->GetGUID(), pet->GetReactState());
}

bool TyrannusHasSuspendedPets(Player* bot)
{
    std::lock_guard<std::mutex> guard(tyrannusPetsLock);
    auto it = tyrannusSuspendedPets.find(bot->GetGUID());
    return it != tyrannusSuspendedPets.end() && !it->second.empty();
}

std::unordered_map<ObjectGuid, ReactStates> TyrannusTakeSuspendedPets(Player* bot)
{
    std::lock_guard<std::mutex> guard(tyrannusPetsLock);
    auto node = tyrannusSuspendedPets.extract(bot->GetGUID());
    return node.empty() ? std::unordered_map<ObjectGuid, ReactStates>() : std::move(node.mapped());
}

bool IsPoisonNovaCasting(Unit* boss)
{
    return boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS) ||
                                                      boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS_HC));
}

std::vector<PoSGroundHazard> FindPoSGroundHazards(WorldObject* searcher)
{
    std::list<WorldObject*> objects;
    Acore::AllWorldObjectsInRange check(searcher, 60.0f);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> list(searcher, objects, check,
                                                                        GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(searcher, list, 60.0f);

    std::vector<PoSGroundHazard> hazards;
    for (WorldObject* object : objects)
    {
        DynamicObject* dynObj = object->ToDynObject();
        if (!dynObj || !dynObj->IsInWorld())
            continue;

        uint32 spell = dynObj->GetSpellId();
        if (spell == SPELL_TOXIC_WASTE || spell == SPELL_TOXIC_WASTE_HC || spell == SPELL_ICY_BLAST_AREA ||
            spell == SPELL_ICY_BLAST_AREA_HC)
            hazards.push_back({dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetRadius()});
    }
    return hazards;
}

bool InPoSGroundHazard(std::vector<PoSGroundHazard> const& hazards, float x, float y, float margin)
{
    for (PoSGroundHazard const& hazard : hazards)
    {
        float dx = hazard.x - x;
        float dy = hazard.y - y;
        if (dx * dx + dy * dy < (hazard.radius + margin) * (hazard.radius + margin))
            return true;
    }
    return false;
}

Unit* FindPoSBoss(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (Unit* tyrannus = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus"))
        return tyrannus;
    return AI_VALUE2(Unit*, "find target", "ick");
}

uint32 PermafrostStacks(Unit* unit)
{
    for (uint32 spell : {uint32(SPELL_PERMAFROST_DEBUFF), uint32(SPELL_PERMAFROST_DEBUFF_HC)})
        if (Aura* aura = unit->GetAura(spell))
            return aura->GetStackAmount();
    return 0;
}

bool IsGarfrostForging(Unit* boss)
{
    // Stomp, leap to the anvil, forge: he is passive and untanked for these seconds
    Creature* creature = boss->ToCreature();
    return (creature && creature->GetReactState() == REACT_PASSIVE) ||
           boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STOMP) || boss->FindCurrentSpellBySpellId(SPELL_FORGE_BLADE) ||
           boss->FindCurrentSpellBySpellId(SPELL_FORGE_MACE);
}

bool IsShieldedFromGarfrost(Unit* boss, Unit* unit)
{
    // Same test as spell_garfrost_permafrost: a boulder within 4 yd of the line, and not in his melee range
    if (boss->IsWithinMeleeRange(unit))
        return false;

    std::list<GameObject*> rocks;
    boss->GetGameObjectListWithEntryInGrid(rocks, GO_SARONITE_ROCK, 100.0f);
    for (GameObject* rock : rocks)
        if (!rock->IsInvisibleDueToDespawn() && rock->IsInBetween(boss, unit, 4.0f))
            return true;
    return false;
}

bool GarfrostWantsCover(Player* bot, Unit* boss)
{
    uint32 stacks = PermafrostStacks(bot);
    if (IsGarfrostForging(boss))
        return stacks > 0 || !PlayerbotAI::IsTank(bot);

    if (PlayerbotAI::IsTank(bot))
        return false;

    if (PlayerbotAI::IsRanged(bot))
        return true;

    // Melee: hide at the threshold and stay hidden until the stacks are gone
    return stacks >= GARFROST_MELEE_HIDE_STACKS || (stacks > 0 && IsShieldedFromGarfrost(boss, bot));
}

bool GetSaroniteLanding(Unit* boss, Position& landing)
{
    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_THROW_SARONITE);
    if (!spell)
        return false;

    // The boulder lands where its target stands when the cast ends (TARGET_DEST_TARGET_ANY), so it follows them
    if (Unit* target = spell->m_targets.GetUnitTarget())
    {
        landing.Relocate(*target);
        return true;
    }
    if (spell->m_targets.HasDst())
    {
        landing.Relocate(*spell->m_targets.GetDstPos());
        return true;
    }
    return false;
}

bool BrandedHealerMayHeal(Player* bot, Unit* boss)
{
    // Guides split on a branded healer; the usual compromise: hold heals unless the tank is in danger
    Unit* tank = boss->GetVictim();
    return PlayerbotAI::IsHeal(bot) && tank && tank->GetHealthPct() < 50.0f;
}

bool GarfrostPermafrostTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    return boss && GarfrostWantsCover(bot, boss) && !IsShieldedFromGarfrost(boss, bot);
}

bool GarfrostTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    return boss && PlayerbotAI::IsTank(bot) && boss->GetVictim() == bot && !IsGarfrostForging(boss);
}

bool GarfrostSaroniteTrigger::IsActive()
{
    // The target moves off the group; everyone else gets clear of the target
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    Position landing;
    return boss && GetSaroniteLanding(boss, landing) &&
           bot->GetExactDist2d(landing.GetPositionX(), landing.GetPositionY()) < GARFROST_SARONITE_AVOID_RADIUS;
}

bool IckAndKrickTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "Ick");
    if (!boss)
        return false;

    return true;
}

bool TyrannusTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return false;

    return true;
}

bool PoSGroundHazardTrigger::IsActive()
{
    Unit* boss = FindPoSBoss(botAI);
    if (!boss)
        return false;

    std::vector<PoSGroundHazard> hazards = FindPoSGroundHazards(bot);
    if (InPoSGroundHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), 1.0f))
        return true;

    return PlayerbotAI::IsTank(bot) && boss->GetVictim() == bot &&
           InPoSGroundHazard(hazards, boss->GetPositionX(), boss->GetPositionY(), 0.0f);
}

bool TyrannusOverlordsBrandTrigger::IsActive()
{
    // A branded tank keeps tanking: its damage only lands on itself
    return bot->HasAura(SPELL_OVERLORDS_BRAND) && !PlayerbotAI::IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
}

bool TyrannusOverlordsBrandFadedTrigger::IsActive()
{
    // No boss needed: Tyrannus often dies while someone is branded, and the pets still need their stance back
    return !bot->HasAura(SPELL_OVERLORDS_BRAND) && TyrannusHasSuspendedPets(bot);
}

bool TyrannusBrandStopCastTrigger::IsActive()
{
    // A reaction: the main engine waits out a cast in progress, so a heal begun just before the brand would land
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss || !bot->HasAura(SPELL_OVERLORDS_BRAND) || PlayerbotAI::IsTank(bot))
        return false;

    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
    {
        Spell* spell = bot->GetCurrentSpell(slot);
        if (!spell)
            continue;

        // Damage is copied onto his target, healing onto him; plain buffs may finish
        SpellInfo const* info = spell->GetSpellInfo();
        if (!info->IsPositive())
            return true;

        bool heal = info->HasEffect(SPELL_EFFECT_HEAL) || info->HasAura(SPELL_AURA_PERIODIC_HEAL);
        if (heal && !BrandedHealerMayHeal(bot, boss))
            return true;
    }
    return false;
}
