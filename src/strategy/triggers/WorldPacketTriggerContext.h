/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETTRIGGERCONTEXT_H
#define _PLAYERBOT_WORLDPACKETTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "WithinAreaTrigger.h"
#include "WorldPacketTrigger.h"

class WorldPacketTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WorldPacketTriggerContext()
    {
        creators["gossip hello"] = &WorldPacketTriggerContext::gossip_hello;
        creators["group invite"] = &WorldPacketTriggerContext::group_invite;
        creators["group set leader"] = &WorldPacketTriggerContext::group_set_leader;
        creators["not enough money"] = &WorldPacketTriggerContext::no_money;
        creators["not enough reputation"] = &WorldPacketTriggerContext::no_reputation;
        creators["cannot equip"] = &WorldPacketTriggerContext::cannot_equip;
        creators["use game object"] = &WorldPacketTriggerContext::use_game_object;
        creators["loot roll"] = &WorldPacketTriggerContext::loot_roll;
        creators["resurrect request"] = &WorldPacketTriggerContext::resurrect_request;
        creators["area trigger"] = &WorldPacketTriggerContext::area_trigger;
        creators["within area trigger"] = &WorldPacketTriggerContext::within_area_trigger;
        creators["check mount state"] = &WorldPacketTriggerContext::check_mount_state;
        creators["activate taxi"] = &WorldPacketTriggerContext::taxi;
        creators["trade status"] = &WorldPacketTriggerContext::trade_status;
        creators["trade status extended"] = &WorldPacketTriggerContext::trade_status_extended;
        creators["loot response"] = &WorldPacketTriggerContext::loot_response;
        creators["out of react range"] = &WorldPacketTriggerContext::out_of_react_range;

        // quest
        creators["complete quest"] = &WorldPacketTriggerContext::complete_quest;
        creators["accept quest"] = &WorldPacketTriggerContext::accept_quest;
        creators["confirm quest"] = &WorldPacketTriggerContext::quest_confirm_accept;
        creators["quest share"] = &WorldPacketTriggerContext::quest_share;
        creators["quest update add kill"] = &WorldPacketTriggerContext::quest_update_add_kill;
        creators["quest update add item"] = &WorldPacketTriggerContext::quest_update_add_item;
        creators["quest update failed"] = &WorldPacketTriggerContext::quest_update_failed;
        creators["quest update failed timer"] = &WorldPacketTriggerContext::quest_update_failed_timer;
        creators["quest update complete"] = &WorldPacketTriggerContext::quest_update_complete;
        creators["questgiver quest details"] = &WorldPacketTriggerContext::questgiver_quest_details;


        creators["item push result"] = &WorldPacketTriggerContext::item_push_result;
        creators["party command"] = &WorldPacketTriggerContext::party_command;
        creators["taxi done"] = &WorldPacketTriggerContext::taxi_done;
        creators["cast failed"] = &WorldPacketTriggerContext::cast_failed;
        creators["duel requested"] = &WorldPacketTriggerContext::duel_requested;
        creators["ready check"] = &WorldPacketTriggerContext::ready_check;
        creators["ready check finished"] = &WorldPacketTriggerContext::ready_check_finished;
        creators["uninvite"] = &WorldPacketTriggerContext::uninvite;
        creators["uninvite guid"] = &WorldPacketTriggerContext::uninvite_guid;
        creators["lfg join"] = &WorldPacketTriggerContext::lfg_update;
        creators["lfg proposal"] = &WorldPacketTriggerContext::lfg_proposal;
        creators["lfg role check"] = &WorldPacketTriggerContext::lfg_role_check;
        creators["lfg leave"] = &WorldPacketTriggerContext::lfg_leave;
        creators["guild invite"] = &WorldPacketTriggerContext::guild_invite;
        creators["petition offer"] = &WorldPacketTriggerContext::petition_offer;
        creators["lfg teleport"] = &WorldPacketTriggerContext::lfg_teleport;
        creators["inventory change failure"] = &WorldPacketTriggerContext::inventory_change_failure;
        creators["bg status"] = &WorldPacketTriggerContext::bg_status;
        creators["levelup"] = &WorldPacketTriggerContext::levelup;
        creators["xpgain"] = &WorldPacketTriggerContext::xpgain;
        creators["see spell"] = &WorldPacketTriggerContext::seespell;
        creators["release spirit"] = &WorldPacketTriggerContext::release_spirit;
        creators["revive from corpse"] = &WorldPacketTriggerContext::revive_from_corpse;
        creators["receive emote"] = &WorldPacketTriggerContext::receive_emote;
        creators["receive text emote"] = &WorldPacketTriggerContext::receive_text_emote;
        creators["arena team invite"] = &WorldPacketTriggerContext::arena_team_invite;
        creators["group destroyed"] = &WorldPacketTriggerContext::group_destroyed;
        creators["group list"] = &WorldPacketTriggerContext::group_list;
    }

private:
    static Trigger* inventory_change_failure(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "inventory change failure"); }
    static Trigger* guild_invite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "guild invite"); }
    static Trigger* lfg_teleport(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg teleport"); }
    static Trigger* lfg_leave(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg leave"); }
    static Trigger* lfg_proposal(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg proposal"); }
    static Trigger* lfg_role_check(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg role check"); }
    static Trigger* lfg_update(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg join"); }
    static Trigger* uninvite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "uninvite"); }
    static Trigger* uninvite_guid(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "uninvite guid"); }
    static Trigger* ready_check_finished(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "ready check finished"); }
    static Trigger* ready_check(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "ready check"); }
    static Trigger* duel_requested(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "duel requested"); }
    static Trigger* cast_failed(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "cast failed"); }
    static Trigger* taxi_done(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "taxi done"); }
    static Trigger* party_command(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "party command"); }
    static Trigger* item_push_result(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "item push result"); }

    // quest
    static Trigger* quest_update_add_kill(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add kill"); }
    static Trigger* quest_update_add_item(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add item"); }
    static Trigger* quest_update_failed(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed"); }
    static Trigger* quest_update_failed_timer(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed timer"); }
    static Trigger* quest_update_complete(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update complete"); }
    static Trigger* complete_quest(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "complete quest"); }
    static Trigger* accept_quest(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "accept quest"); }
    static Trigger* quest_confirm_accept(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "confirm quest"); }
    static Trigger* quest_share(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest share"); }
    static Trigger* questgiver_quest_details(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "questgiver quest details"); }

    static Trigger* out_of_react_range(PlayerbotAI* botAI) { return new OutOfReactRangeTrigger(botAI); }
    static Trigger* loot_response(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "loot response"); }
    static Trigger* trade_status(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "trade status"); }
    static Trigger* trade_status_extended(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "trade status extended"); }
    static Trigger* cannot_equip(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "cannot equip"); }
    static Trigger* check_mount_state(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "check mount state"); }
    static Trigger* area_trigger(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "area trigger"); }
    static Trigger* within_area_trigger(PlayerbotAI* botAI) { return new WithinAreaTrigger(botAI); }
    static Trigger* resurrect_request(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "resurrect request"); }
    static Trigger* gossip_hello(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "gossip hello"); }
    static Trigger* group_invite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "group invite"); }
    static Trigger* group_set_leader(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "group set leader"); }
    static Trigger* no_money(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "not enough money"); }
    static Trigger* no_reputation(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "not enough reputation"); }
    static Trigger* use_game_object(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "use game object"); }
    static Trigger* loot_roll(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "loot roll"); }
    static Trigger* taxi(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "activate taxi"); }
    static Trigger* bg_status(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "bg status"); }
    static Trigger* levelup(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "levelup"); }
    static Trigger* xpgain(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "xpgain"); }
    static Trigger* petition_offer(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "petition offer"); }
    static Trigger* seespell(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "see spell"); }
    static Trigger* release_spirit(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "release spirit"); }
    static Trigger* revive_from_corpse(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "revive from corpse"); }
    static Trigger* receive_emote(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "receive emote"); }
    static Trigger* receive_text_emote(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "receive text emote"); }
    static Trigger* arena_team_invite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "arena team invite"); }
    static Trigger* group_destroyed(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "group destroyed"); }
    static Trigger* group_list(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "group list"); }
};

#endif
