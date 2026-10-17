/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PriestValues.h"

#include <algorithm>
#include <list>

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "SpellMgr.h"

namespace
{
constexpr uint32 SPELL_MASS_DISPEL = 32375;
constexpr uint32 SPELL_DIVINE_SHIELD = 642;
constexpr uint32 SPELL_ICE_BLOCK = 45438;
}  // namespace

bool MassDispelTargetValue::IsValidTarget(PlayerbotAI* botAI, Unit* target)
{
    Player* bot = botAI->GetBot();
    if (!target || !target->IsPlayer() || !target->IsInWorld() || !target->IsAlive() ||
        (!target->HasAura(SPELL_DIVINE_SHIELD) && !target->HasAura(SPELL_ICE_BLOCK)))
        return false;

    bool const isDuel = bot->duel && bot->duel->Opponent == target && bot->duel->State == DUEL_STATE_IN_PROGRESS;
    if (!isDuel)
    {
        if (sPlayerbotAIConfig.IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) ||
            sPlayerbotAIConfig.IsPvpProhibited(target->GetZoneId(), target->GetAreaId()))
            return false;

        Player* master = botAI->GetMaster();
        if (master && !master->IsPvP() && !master->IsFFAPvP())
            return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_MASS_DISPEL);
    return spellInfo && bot->IsWithinDistInMap(target, spellInfo->GetMaxRange(false, bot)) &&
           bot->IsValidAttackTarget(target, spellInfo) && bot->CanSeeOrDetect(target) && bot->IsWithinLOSInMap(target);
}

ObjectGuid MassDispelTargetValue::Calculate()
{
    bool const isDuel = bot->duel && bot->duel->State == DUEL_STATE_IN_PROGRESS;
    if (!bot->IsInWorld() || !bot->IsAlive() || !bot->HasSpell(SPELL_MASS_DISPEL) ||
        bot->HasSpellCooldown(SPELL_MASS_DISPEL) || (!isDuel && !bot->IsPvP() && !bot->IsFFAPvP()) ||
        (!bot->IsInCombat() && !bot->InBattleground() && !isDuel))
        return ObjectGuid::Empty;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_MASS_DISPEL);
    if (!spellInfo)
        return ObjectGuid::Empty;

    float const range = std::min(sPlayerbotAIConfig.SightDistance, spellInfo->GetMaxRange(false, bot));
    // Normal attack-target lists exclude Divine Shield and Ice Block before the priest can dispel them.
    std::list<Player*> targets;
    Acore::AnyPlayerInObjectRangeCheck check(bot, range);
    Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(bot, targets, check);
    Cell::VisitObjects(bot, searcher, range);

    Unit* nearest = nullptr;
    for (Unit* target : targets)
        if (IsValidTarget(botAI, target) && (!nearest || bot->GetDistance(target) < bot->GetDistance(nearest)))
            nearest = target;

    return nearest ? nearest->GetGUID() : ObjectGuid::Empty;
}
