/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETACTIONCONTEXT_H
#define _PLAYERBOT_WORLDPACKETACTIONCONTEXT_H

#include "AcceptBattlegroundInvitationAction.h"
#include "AcceptDuelAction.h"
#include "AcceptInvitationAction.h"
#include "AcceptQuestAction.h"
#include "AcceptResurrectAction.h"
#include "AreaTriggerAction.h"
#include "ArenaTeamActions.h"
#include "BattleGroundTactics.h"
#include "CheckMountStateAction.h"
#include "GuildAcceptAction.h"
#include "GuildCreateActions.h"
#include "InventoryChangeFailureAction.h"
#include "LeaveGroupAction.h"
#include "LfgActions.h"
#include "LootAction.h"
#include "LootRollAction.h"
#include "QuestAction.h"
#include "PassLeadershipToMasterAction.h"
#include "PetitionSignAction.h"
#include "QuestConfirmAcceptAction.h"
#include "ReadyCheckAction.h"
#include "RememberTaxiAction.h"
#include "ReviveFromCorpseAction.h"
#include "RewardAction.h"
#include "SeeSpellAction.h"
#include "SecurityCheckAction.h"
#include "TalkToQuestGiverAction.h"
#include "TellCastFailedAction.h"
#include "TellMasterAction.h"
#include "TradeStatusAction.h"
#include "UseMeetingStoneAction.h"
#include "NamedObjectContext.h"
#include "QuestConfirmAcceptAction.h"

class PlayerbotAI;

class WorldPacketActionContext : public NamedObjectContext<Action>
{
    public:
        WorldPacketActionContext()
        {
            creators["accept invitation"] = &WorldPacketActionContext::accept_invitation;
            creators["give leader in dungeon"] = &WorldPacketActionContext::give_leader_in_dungeon;
            creators["leader"] = &WorldPacketActionContext::pass_leadership_to_master;
            creators["tell not enough money"] = &WorldPacketActionContext::tell_not_enough_money;
            creators["tell not enough reputation"] = &WorldPacketActionContext::tell_not_enough_reputation;
            creators["tell cannot equip"] = &WorldPacketActionContext::tell_cannot_equip;
            creators["talk to quest giver"] = &WorldPacketActionContext::turn_in_quest;
            creators["accept quest"] = &WorldPacketActionContext::accept_quest;
            creators["accept all quests"] = &WorldPacketActionContext::accept_all_quests;
            creators["accept quest share"] = &WorldPacketActionContext::accept_quest_share;
            creators["loot roll"] = &WorldPacketActionContext::loot_roll;
            creators["master loot roll"] = &WorldPacketActionContext::master_loot_roll;
            creators["revive from corpse"] = &WorldPacketActionContext::revive_from_corpse;
            creators["find corpse"] = &WorldPacketActionContext::find_corpse;
            creators["auto release"] = &WorldPacketActionContext::auto_release;
            creators["accept resurrect"] = &WorldPacketActionContext::accept_resurrect;
            creators["use meeting stone"] = &WorldPacketActionContext::use_meeting_stone;
            creators["area trigger"] = &WorldPacketActionContext::area_trigger;
            creators["reach area trigger"] = &WorldPacketActionContext::reach_area_trigger;
            creators["check mount state"] = &WorldPacketActionContext::check_mount_state;
            creators["remember taxi"] = &WorldPacketActionContext::remember_taxi;
            creators["accept trade"] = &WorldPacketActionContext::accept_trade;
            creators["store loot"] = &WorldPacketActionContext::store_loot;
            creators["quest objective completed"] = &WorldPacketActionContext::quest_objective_completed;
            creators["party command"] = &WorldPacketActionContext::party_command;
            creators["tell cast failed"] = &WorldPacketActionContext::tell_cast_failed;
            creators["accept duel"] = &WorldPacketActionContext::accept_duel;
            creators["ready check"] = &WorldPacketActionContext::ready_check;
            creators["ready check finished"] = &WorldPacketActionContext::ready_check_finished;
            creators["uninvite"] = &WorldPacketActionContext::uninvite;
            creators["security check"] = &WorldPacketActionContext::security_check;
            creators["guild accept"] = &WorldPacketActionContext::guild_accept;
            creators["inventory change failure"] = &WorldPacketActionContext::inventory_change_failure;
            creators["bg status check"] = &WorldPacketActionContext::bg_status_check;
            creators["bg strategy check"] = &WorldPacketActionContext::bg_strategy_check;
            creators["bg status"] = &WorldPacketActionContext::bg_status;
            creators["bg join"] = &WorldPacketActionContext::bg_join;
            creators["bg leave"] = &WorldPacketActionContext::bg_leave;
            creators["arena tactics"] = &WorldPacketActionContext::arena_tactics;
            creators["petition sign"] = &WorldPacketActionContext::petition_sign;
            creators["lfg join"] = &WorldPacketActionContext::lfg_join;
            creators["lfg accept"] = &WorldPacketActionContext::lfg_accept;
            creators["lfg role check"] = &WorldPacketActionContext::lfg_role_check;
            creators["lfg leave"] = &WorldPacketActionContext::lfg_leave;
            creators["lfg teleport"] = &WorldPacketActionContext::lfg_teleport;
            creators["see spell"] = &WorldPacketActionContext::see_spell;
            creators["arena team accept"] = &WorldPacketActionContext::arena_team_accept;
            creators["turn in query quest"] = &WorldPacketActionContext::turn_in_query_quest;
            creators["quest confirm accept"] = &WorldPacketActionContext::quest_confirm_accept;
        }

    private:
        static Action* inventory_change_failure(PlayerbotAI* botAI) { return new InventoryChangeFailureAction(botAI); }
        static Action* guild_accept(PlayerbotAI* botAI) { return new GuildAcceptAction(botAI); }
        static Action* security_check(PlayerbotAI* botAI) { return new SecurityCheckAction(botAI); }
        static Action* uninvite(PlayerbotAI* botAI) { return new UninviteAction(botAI); }
        static Action* ready_check_finished(PlayerbotAI* botAI) { return new FinishReadyCheckAction(botAI); }
        static Action* ready_check(PlayerbotAI* botAI) { return new ReadyCheckAction(botAI); }
        static Action* accept_duel(PlayerbotAI* botAI) { return new AcceptDuelAction(botAI); }
        static Action* tell_cast_failed(PlayerbotAI* botAI) { return new TellCastFailedAction(botAI); }
        static Action* party_command(PlayerbotAI* botAI) { return new PartyCommandAction(botAI); }
        static Action* quest_objective_completed(PlayerbotAI* botAI) { return new QuestObjectiveCompletedAction(botAI); }
        static Action* store_loot(PlayerbotAI* botAI) { return new StoreLootAction(botAI); }
        static Action* accept_trade(PlayerbotAI* botAI) { return new TradeStatusAction(botAI); }
        static Action* remember_taxi(PlayerbotAI* botAI) { return new RememberTaxiAction(botAI); }
        static Action* check_mount_state(PlayerbotAI* botAI) { return new CheckMountStateAction(botAI); }
        static Action* area_trigger(PlayerbotAI* botAI) { return new AreaTriggerAction(botAI); }
        static Action* reach_area_trigger(PlayerbotAI* botAI) { return new ReachAreaTriggerAction(botAI); }
        static Action* use_meeting_stone(PlayerbotAI* botAI) { return new UseMeetingStoneAction(botAI); }
        static Action* accept_resurrect(PlayerbotAI* botAI) { return new AcceptResurrectAction(botAI); }
        static Action* find_corpse(PlayerbotAI* botAI) { return new FindCorpseAction(botAI); }
        static Action* auto_release(PlayerbotAI* botAI) { return new AutoReleaseSpiritAction(botAI); }
        static Action* revive_from_corpse(PlayerbotAI* botAI) { return new ReviveFromCorpseAction(botAI); }
        static Action* accept_invitation(PlayerbotAI* botAI) { return new AcceptInvitationAction(botAI); }
        static Action* give_leader_in_dungeon(PlayerbotAI* botAI) { return new GiveLeaderAction(botAI, "I don't know this dungeon, lead the way!"); }
        static Action* pass_leadership_to_master(PlayerbotAI* botAI) { return new PassLeadershipToMasterAction(botAI); }
        static Action* tell_not_enough_money(PlayerbotAI* botAI) { return new TellMasterAction(botAI, "Not enough money"); }
        static Action* tell_not_enough_reputation(PlayerbotAI* botAI) { return new TellMasterAction(botAI, "Not enough reputation"); }
        static Action* tell_cannot_equip(PlayerbotAI* botAI) { return new InventoryChangeFailureAction(botAI); }
        static Action* turn_in_quest(PlayerbotAI* botAI) { return new TalkToQuestGiverAction(botAI); }
        static Action* accept_quest(PlayerbotAI* botAI) { return new AcceptQuestAction(botAI); }
        static Action* accept_all_quests(PlayerbotAI* botAI) { return new AcceptAllQuestsAction(botAI); }
        static Action* accept_quest_share(PlayerbotAI* botAI) { return new AcceptQuestShareAction(botAI); }
        static Action* loot_roll(PlayerbotAI* botAI) { return new LootRollAction(botAI); }
        static Action* master_loot_roll(PlayerbotAI* botAI) { return new MasterLootRollAction(botAI); }
        static Action* bg_join(PlayerbotAI* botAI) { return new BGJoinAction(botAI); }
        static Action* bg_leave(PlayerbotAI* botAI) { return new BGLeaveAction(botAI); }
        static Action* bg_status(PlayerbotAI* botAI) { return new BGStatusAction(botAI); }
        static Action* bg_status_check(PlayerbotAI* botAI) { return new BGStatusCheckAction(botAI); }
        static Action* bg_strategy_check(PlayerbotAI* botAI) { return new BGStrategyCheckAction(botAI); }
        static Action* arena_tactics(PlayerbotAI* botAI) { return new ArenaTactics(botAI); }
        static Action* petition_sign(PlayerbotAI* botAI) { return new PetitionSignAction(botAI); }
        static Action* lfg_teleport(PlayerbotAI* botAI) { return new LfgTeleportAction(botAI); }
        static Action* lfg_leave(PlayerbotAI* botAI) { return new LfgLeaveAction(botAI); }
        static Action* lfg_accept(PlayerbotAI* botAI) { return new LfgAcceptAction(botAI); }
        static Action* lfg_role_check(PlayerbotAI* botAI) { return new LfgRoleCheckAction(botAI); }
        static Action* lfg_join(PlayerbotAI* botAI) { return new LfgJoinAction(botAI); }
        static Action* see_spell(PlayerbotAI* botAI) { return new SeeSpellAction(botAI); }
        static Action* arena_team_accept(PlayerbotAI* botAI) { return new ArenaTeamAcceptAction(botAI); }
        static Action* turn_in_query_quest(PlayerbotAI* botAI) { return new TurnInQueryQuestAction(botAI); }
        static Action* quest_confirm_accept(PlayerbotAI* botAI) { return new QuestConfirmAcceptAction(botAI); }
};

#endif
