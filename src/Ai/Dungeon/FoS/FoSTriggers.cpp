/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSTriggers.h"
#include "AiObjectContext.h"
#include "Group.h"
#include "Playerbots.h"
#include "Spell.h"
#include <mutex>

namespace
{
// Bots on different maps update in parallel
std::mutex devourerPetsLock;
std::unordered_map<ObjectGuid, std::unordered_map<ObjectGuid, ReactStates>> devourerSuspendedPets;
}

void DevourerRememberSuspendedPet(Player* bot, Creature* pet)
{
    std::lock_guard<std::mutex> guard(devourerPetsLock);
    devourerSuspendedPets[bot->GetGUID()].emplace(pet->GetGUID(), pet->GetReactState());
}

bool DevourerHasSuspendedPets(Player* bot)
{
    std::lock_guard<std::mutex> guard(devourerPetsLock);
    auto it = devourerSuspendedPets.find(bot->GetGUID());
    return it != devourerSuspendedPets.end() && !it->second.empty();
}

std::unordered_map<ObjectGuid, ReactStates> DevourerTakeSuspendedPets(Player* bot)
{
    std::lock_guard<std::mutex> guard(devourerPetsLock);
    auto node = devourerSuspendedPets.extract(bot->GetGUID());
    return node.empty() ? std::unordered_map<ObjectGuid, ReactStates>() : std::move(node.mapped());
}

bool IsCastingAt(Unit* caster, Unit* target)
{
    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL})
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (spell && spell->m_targets.GetUnitTargetGUID() == target->GetGUID())
            return true;
    }
    return false;
}

bool IsWailingSoulsActive(Unit* boss)
{
    return boss->FindCurrentSpellBySpellId(SPELL_WAILING_SOULS) || boss->HasAura(SPELL_WAILING_SOULS_SWEEP) ||
           boss->HasAura(SPELL_WAILING_SOULS_SWEEP_2);
}

bool IsPhantomBlastCasting(Unit* boss)
{
    Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell)
        return false;

    uint32 id = spell->m_spellInfo->Id;
    return id == SPELL_PHANTOM_BLAST || id == SPELL_PHANTOM_BLAST_H;
}

bool IsMirroredSoulActive(Player* bot, Unit* boss)
{
    if (boss->FindCurrentSpellBySpellId(SPELL_MIRRORED_SOUL))
        return true;

    // The debuff sits on the targeted player, not on the boss
    Group* group = bot->GetGroup();
    if (!group)
        return bot->HasAura(SPELL_MIRRORED_SOUL);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsInWorld() && member->HasAura(SPELL_MIRRORED_SOUL))
            return true;
    }

    return false;
}

Unit* FindWellOfSouls(WorldObject* searcher, float x, float y, float radius)
{
    std::list<Creature*> wells;
    searcher->GetCreatureListWithEntryInGrid(wells, uint32(NPC_WELL_OF_SOULS), 60.0f);
    for (Creature* well : wells)
    {
        if (well->IsInWorld() && well->GetExactDist2d(x, y) < radius)
            return well;
    }

    return nullptr;
}

bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (!boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
        return false;

    if (!bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

bool SwitchToSoulFragment::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    Unit* corruptedSoul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    bool activeSoulExists = corruptedSoul && corruptedSoul->IsAlive();

    if (!activeSoulExists)
        return false;

    return true;
}

bool BronjahmPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

bool DevourerWailingSoulsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsWailingSoulsActive(boss);
}

bool DevourerWellOfSoulsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    if (FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS))
        return true;

    return PlayerbotAI::IsTank(bot) && boss->GetVictim() == bot &&
           FindWellOfSouls(bot, boss->GetPositionX(), boss->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS);
}

bool DevourerMirroredSoulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsMirroredSoulActive(bot, boss);
}

bool DevourerMirroredSoulFadedTrigger::IsActive()
{
    // No boss needed: if he dies or evades mid-debuff the pets still get their stance back
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (boss && IsMirroredSoulActive(bot, boss))
        return false;

    return DevourerHasSuspendedPets(bot);
}

bool DevourerPhantomBlastTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsPhantomBlastCasting(boss);
}

bool DevourerStopCastTrigger::IsActive()
{
    // A reaction: the main engine waits out a cast in progress, so a bolt begun before the debuff would land
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss || !IsMirroredSoulActive(bot, boss))
        return false;

    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
    {
        Spell* spell = bot->GetCurrentSpell(slot);
        if (spell && spell->m_targets.GetUnitTargetGUID() == boss->GetGUID())
            return true;
    }
    return false;
}
