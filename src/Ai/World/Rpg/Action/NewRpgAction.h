/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGACTION_H
#define PLAYERBOTS_NEWRPGACTION_H

#include "Duration.h"
#include "MovementActions.h"
#include "NewRpgBaseAction.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "Object.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "QuestDef.h"
#include "TravelMgr.h"
#include <string>

class Player;

class TellRpgStatusAction : public NewRpgBaseAction
{
public:
    TellRpgStatusAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "rpg status") {}

    bool Execute(Event event) override;

private:
    static constexpr char const* RPG_STATUS_CHANGED_KEY = "rpg_status_changed";
    static constexpr char const* RPG_STATUS_CHANGED_DEFAULT = "rpg status -> %status";

    void WhisperStatusChange(Player* owner, std::string const& statusName);
};

class StartRpgDoQuestAction : public Action
{
public:
    StartRpgDoQuestAction(PlayerbotAI* botAI) : Action(botAI, "start rpg do quest") {}

    bool Execute(Event event) override;
};

class NewRpgStatusUpdateAction : public NewRpgBaseAction
{
public:
    NewRpgStatusUpdateAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg status update")
    {
        // int statusCount = RPG_STATUS_END - 1;

        // transitionMat.resize(statusCount, std::vector<int>(statusCount, 0));

        // transitionMat[RPG_IDLE][RPG_GO_GRIND] = 20;
        // transitionMat[RPG_IDLE][RPG_GO_CAMP] = 15;
        // transitionMat[RPG_IDLE][RPG_WANDER_NPC] = 30;
        // transitionMat[RPG_IDLE][RPG_DO_QUEST] = 35;
    }
    bool Execute(Event event) override;

protected:
    // static NewRpgStatusTransitionProb transitionMat;
    const int32 statusWanderNpcDuration = 5 * MINUTE  * IN_MILLISECONDS ;
    const int32 statusWanderRandomDuration = 5 * MINUTE  * IN_MILLISECONDS ;
    const int32 statusRestDuration = 30 * IN_MILLISECONDS ;
    const int32 statusDoQuestDuration = 30 * MINUTE  * IN_MILLISECONDS ;
    const int32 statusOutDoorPvPDuration = HOUR * IN_MILLISECONDS ;
    const int32 statusDoGatherDuration = 15 * MINUTE * IN_MILLISECONDS;
};

class NewRpgGoGrindAction : public NewRpgBaseAction
{
public:
    NewRpgGoGrindAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg go grind") {}
    bool Execute(Event event) override;
};

class NewRpgGoCampAction : public NewRpgBaseAction
{
public:
    NewRpgGoCampAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg go camp") {}
    bool Execute(Event event) override;
};

class NewRpgWanderRandomAction : public NewRpgBaseAction
{
public:
    NewRpgWanderRandomAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg wander random") {}
    bool Execute(Event event) override;
};

class NewRpgWanderNpcAction : public NewRpgBaseAction
{
public:
    NewRpgWanderNpcAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg move npcs") {}
    bool Execute(Event event) override;

    const uint32 npcStayTime = 8 * IN_MILLISECONDS;
};

class NewRpgDoQuestAction : public NewRpgBaseAction
{
public:
    NewRpgDoQuestAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg do quest") {}
    bool Execute(Event event) override;

protected:
    bool DoIncompleteQuest(NewRpgInfo::DoQuest& data);
    bool DoCompletedQuest(NewRpgInfo::DoQuest& data);

    const uint32 poiStayTime = 5 * MINUTE * IN_MILLISECONDS;
};

class NewRpgTravelFlightAction : public NewRpgBaseAction
{
public:
    NewRpgTravelFlightAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg travel flight") {}
    bool Execute(Event event) override;

protected:
    void ContinueCrossMapTaxi();
};

// Used by the "grab" strategy (QuestGrabStrategy), independent of "new rpg" --
// reuses NewRpgBaseAction purely for its movement/quest-interaction helpers.
// Four rules, tried in order: accept/turn in at a nearby questgiver if one's
// ready (existing, unmodified SearchQuestGiverAndAcceptOrReward); else use a
// carried quest item on the specific creature/GO a current objective still
// needs (UseQuestItemOnRequiredTarget); else try a carried quest item that
// has no target at all -- a spell-focus-gated "fill this near the right
// spot" item like Jade Phial (UseQuestItemToCreateRequiredItem); else walk
// to and use the nearest quest-relevant GameObject already in sight.
class GrabQuestItemAction : public NewRpgBaseAction
{
public:
    GrabQuestItemAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "grab quest item") {}
    bool Execute(Event event) override;

protected:
    // Quest data (Quest::RequiredNpcOrGo[]) tells us exactly which creature (>0) or
    // gameobject (<0) entry each objective slot still needs -- the same array
    // NewRpgDoQuestAction already reads via RequiredNpcOrGoCount. What quest data does
    // NOT tell us is which carried item resolves that objective (this codebase has no
    // TrinityCore-style RequiredSourceItemId; classic "use item on target" quests grant
    // credit via per-item hardcoded SpellScripts, e.g. spell_item_branns_communicator).
    // So this tries each non-StartQuest "quest" category item against a nearby target
    // matching an outstanding objective and lets the server's own item/spell target
    // validation decide -- identical to what the manual "use <item> <target>" chat
    // command already relies on.
    bool UseQuestItemOnRequiredTarget();

    // Covers RequiredItemId-only objectives satisfied by a *different* item than the one
    // in the bot's bag -- e.g. quest 929 needs "Filled Jade Phial" (5639), obtained by
    // using the empty "Jade Phial" (5619) while standing near a spell-focus gameobject
    // the quest data never names. No script/table anywhere links 5619 -> 5639 -> quest
    // 929 directly; the only place that link exists is the tool item's own on-use spell
    // effect (SPELL_EFFECT_CREATE_ITEM), which ItemUsageValue::IsItemUsefulForQuest
    // already walks for a different purpose. Reuses that same walk to find candidate
    // tool items, then just calls UseItemAuto() with no target and lets the server's own
    // spell-focus/range check decide -- there is nothing to path to or aim at.
    bool UseQuestItemToCreateRequiredItem();
};

#endif
