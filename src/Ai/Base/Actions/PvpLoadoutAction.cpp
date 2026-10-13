/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpLoadoutAction.h"
#include "Event.h"
#include "PlayerbotPvpLoadoutRepository.h"
#include "Playerbots.h"
#include "PvpLoadoutLog.h"
#include "PvpLoadoutMgr.h"

bool SwapPvpLoadoutAction::Execute(Event /*event*/)
{
    std::optional<PvpLoadout::Snapshot>& loadout = AI_VALUE_REF(std::optional<PvpLoadout::Snapshot>, "pvp loadout");
    uint32 const guid = bot->GetGUID().GetCounter();
    PlayerbotPvpLoadoutRepository& repository = PlayerbotPvpLoadoutRepository::instance();

    PvpLoadout::Transition const transition = PvpLoadoutMgr::GetTransition(bot, loadout);
    if (transition == PvpLoadout::Transition::None)
        return false;

    if (transition == PvpLoadout::Transition::Restore)
    {
        PvpLoadoutMgr::Restore(botAI);
        botAI->ResetStrategies();
        return true;
    }

    if (transition == PvpLoadout::Transition::ApplyGear)
    {
        // isUseful waits for the copies to be in the database before the PvE items they stand for are destroyed.
        PvpLoadoutMgr::ApplyPlannedGear(bot, *loadout);
        repository.Save(guid, *loadout);
        Wear(loadout->plannedRating);
        return true;
    }

    // Enter or re-apply: a skirmish waits here until its opponents are on the map.
    PvpLoadout::MatchRating const rating = PvpLoadoutMgr::ResolveMatchRating(bot);
    if (!rating.ready)
        return false;

    if (transition == PvpLoadout::Transition::Enter)
    {
        // Taken before the respec; saved with the planned gear's copies below.
        PvpLoadout::Snapshot snapshot{};
        PvpLoadoutMgr::CaptureTalents(bot, snapshot);
        snapshot.arenaInstanceId = PvpLoadoutMgr::GetArenaInstanceId(bot);
        loadout = std::move(snapshot);
    }
    else
    {
        // The stored snapshot is still the PvE state; only the match gear and the arena it belongs to change.
        PvpLoadoutMgr::RestoreGear(bot, *loadout);
        loadout->arenaInstanceId = PvpLoadoutMgr::GetArenaInstanceId(bot);
    }

    // Strategies follow the new talents before the gear is planned, which goes by the bot's role.
    PvpLoadoutMgr::ApplyPvpTalents(bot);
    botAI->ResetStrategies();
    PvpLoadoutMgr::PlanMatchGear(bot, rating.rating, *loadout);
    // With the copies of what it replaces; the gear goes on once this save is in the database.
    repository.Save(guid, *loadout);
    if (loadout->plannedItems.empty())
        Wear(rating.rating);

    return true;
}

void SwapPvpLoadoutAction::Wear(uint32 rating)
{
    PvpLoadoutMgr::Announce(botAI, rating);
    PvpLoadout::LogDebug("Bot {} wears its PvP loadout for opponents rated {}", bot->GetName(), rating);
}

bool SwapPvpLoadoutAction::isUseful()
{
    PvpLoadout::Transition const transition =
        PvpLoadoutMgr::GetTransition(bot, AI_VALUE_REF(std::optional<PvpLoadout::Snapshot>, "pvp loadout"));
    if (transition == PvpLoadout::Transition::ApplyGear)
        return PlayerbotPvpLoadoutRepository::instance().IsSaved(bot->GetGUID().GetCounter());

    return transition != PvpLoadout::Transition::None;
}

bool SwapPvpLoadoutAction::isPossible() { return !bot->IsInCombat(); }
