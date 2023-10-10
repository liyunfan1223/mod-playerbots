/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATACTIONCONTEXTACTION_H
#define _PLAYERBOT_CHATACTIONCONTEXTACTION_H

#include "AddLootAction.h"
#include "AttackAction.h"
#include "BankAction.h"
#include "BuffAction.h"
#include "BuyAction.h"
#include "CastCustomSpellAction.h"
#include "ChangeChatAction.h"
#include "ChangeTalentsAction.h"
#include "ChangeStrategyAction.h"
#include "ChatShortcutActions.h"
#include "CheatAction.h"
#include "CustomStrategyEditAction.h"
#include "DebugAction.h"
#include "DestroyItemAction.h"
#include "DropQuestAction.h"
#include "EquipAction.h"
#include "FlagAction.h"
#include "GoAction.h"
#include "GossipHelloAction.h"
#include "GuildBankAction.h"
#include "GuildManagementActions.h"
#include "HelpAction.h"
#include "HireAction.h"
#include "InviteToGroupAction.h"
#include "LeaveGroupAction.h"
#include "ListQuestsActions.h"
#include "ListSpellsAction.h"
#include "LogLevelAction.h"
#include "LootStrategyAction.h"
#include "MailAction.h"
#include "QueryItemUsageAction.h"
#include "QueryQuestAction.h"
#include "PassLeadershipToMasterAction.h"
#include "PositionAction.h"
#include "RangeAction.h"
#include "ReleaseSpiritAction.h"
#include "RepairAllAction.h"
#include "ResetAiAction.h"
#include "ReviveFromCorpseAction.h"
#include "RewardAction.h"
#include "RtiAction.h"
#include "RtscAction.h"
#include "SaveManaAction.h"
#include "SellAction.h"
#include "SetCraftAction.h"
#include "SendMailAction.h"
#include "SetHomeAction.h"
#include "ShareQuestAction.h"
#include "SkipSpellsListAction.h"
#include "StatsAction.h"
#include "TaxiAction.h"
#include "TeleportAction.h"
#include "TellCastFailedAction.h"
#include "TellItemCountAction.h"
#include "TellLosAction.h"
#include "TellReputationAction.h"
#include "TellTargetAction.h"
#include "TradeAction.h"
#include "TrainerAction.h"
#include "UnequipAction.h"
#include "UseItemAction.h"
#include "UseMeetingStoneAction.h"
#include "WhoAction.h"
#include "WtsAction.h"
#include "NamedObjectContext.h"
#include "Formations.h"
#include "Stances.h"

class ChatActionContext : public NamedObjectContext<Action>
{
    public:
        ChatActionContext()
        {
            creators["range"] = &ChatActionContext::range;
            creators["stats"] = &ChatActionContext::stats;
            creators["quests"] = &ChatActionContext::quests;
            creators["leave"] = &ChatActionContext::leave;
            creators["reputation"] = &ChatActionContext::reputation;
            creators["log"] = &ChatActionContext::log;
            creators["los"] = &ChatActionContext::los;
            creators["aura"] = &ChatActionContext::aura;
            creators["drop"] = &ChatActionContext::drop;
            creators["clean quest log"] = &ChatActionContext::clean_quest_log;
            creators["share"] = &ChatActionContext::share;
            creators["query quest"] = &ChatActionContext::query_quest;
            creators["query item usage"] = &ChatActionContext::query_item_usage;
            creators["ll"] = &ChatActionContext::ll;
            creators["ss"] = &ChatActionContext::ss;
            creators["add all loot"] = &ChatActionContext::add_all_loot;
            creators["release"] = &ChatActionContext::release;
            creators["repop"] = &ChatActionContext::repop;
            creators["teleport"] = &ChatActionContext::teleport;
            creators["taxi"] = &ChatActionContext::taxi;
            creators["repair"] = &ChatActionContext::repair;
            creators["use"] = &ChatActionContext::use;
            creators["item count"] = &ChatActionContext::item_count;
            creators["equip"] = &ChatActionContext::equip;
            creators["equip upgrades"] = &ChatActionContext::equip_upgrades;
            creators["unequip"] = &ChatActionContext::unequip;
            creators["sell"] = &ChatActionContext::sell;
            creators["buy"] = &ChatActionContext::buy;
            creators["reward"] = &ChatActionContext::reward;
            creators["trade"] = &ChatActionContext::trade;
            creators["talents"] = &ChatActionContext::talents;
            creators["spells"] = &ChatActionContext::spells;
            creators["co"] = &ChatActionContext::co;
            creators["nc"] = &ChatActionContext::nc;
            creators["de"] = &ChatActionContext::dead;
            creators["trainer"] = &ChatActionContext::trainer;
            creators["attack my target"] = &ChatActionContext::attack_my_target;
            creators["chat"] = &ChatActionContext::chat;
            creators["home"] = &ChatActionContext::home;
            creators["destroy"] = &ChatActionContext::destroy;
            creators["reset botAI"] = &ChatActionContext::reset_ai;
            creators["buff"] = &ChatActionContext::buff;
            creators["help"] = &ChatActionContext::help;
            creators["gb"] = &ChatActionContext::gb;
            creators["bank"] = &ChatActionContext::bank;
            creators["follow chat shortcut"] = &ChatActionContext::follow_chat_shortcut;
            creators["stay chat shortcut"] = &ChatActionContext::stay_chat_shortcut;
            creators["flee chat shortcut"] = &ChatActionContext::flee_chat_shortcut;
            creators["runaway chat shortcut"] = &ChatActionContext::runaway_chat_shortcut;
            creators["grind chat shortcut"] = &ChatActionContext::grind_chat_shortcut;
            creators["tank attack chat shortcut"] = &ChatActionContext::tank_attack_chat_shortcut;
            creators["gossip hello"] = &ChatActionContext::gossip_hello;
            creators["cast custom spell"] = &ChatActionContext::cast_custom_spell;
            creators["cast custom nc spell"] = &ChatActionContext::cast_custom_nc_spell;
            creators["invite"] = &ChatActionContext::invite;
            creators["spell"] = &ChatActionContext::spell;
            creators["rti"] = &ChatActionContext::rti;
            creators["spirit healer"] = &ChatActionContext::spirit_healer;
            creators["position"] = &ChatActionContext::position;
            creators["tell target"] = &ChatActionContext::tell_target;
            creators["summon"] = &ChatActionContext::summon;
            creators["who"] = &ChatActionContext::who;
            creators["save mana"] = &ChatActionContext::save_mana;
            creators["max dps chat shortcut"] = &ChatActionContext::max_dps_chat_shortcut;
            creators["tell attackers"] = &ChatActionContext::tell_attackers;
            creators["tell target"] = &ChatActionContext::tell_target;
            creators["formation"] = &ChatActionContext::formation;
            creators["stance"] = &ChatActionContext::stance;
            creators["sendmail"] = &ChatActionContext::sendmail;
            creators["mail"] = &ChatActionContext::mail;
            creators["go"] = &ChatActionContext::go;
            creators["debug"] = &ChatActionContext::debug;
            creators["cdebug"] = &ChatActionContext::debug;
            creators["cs"] = &ChatActionContext::cs;
            creators["wts"] = &ChatActionContext::wts;
            creators["hire"] = &ChatActionContext::hire;
            creators["craft"] = &ChatActionContext::craft;
            creators["flag"] = &ChatActionContext::flag;
            creators["give leader"] = &ChatActionContext::give_leader;
            creators["cheat"] = &ChatActionContext::cheat;
            creators["ginvite"] = &ChatActionContext::ginvite;
            creators["guild promote"] = &ChatActionContext::guild_promote;
            creators["guild demote"] = &ChatActionContext::guild_demote;
            creators["guild remove"] = &ChatActionContext::guild_remove;
            creators["guild leave"] = &ChatActionContext::guild_leave;
            creators["rtsc"] = &ChatActionContext::rtsc;
            creators["naxx chat shortcut"] = &ChatActionContext::naxx_chat_shortcut;
            creators["bwl chat shortcut"] = &ChatActionContext::bwl_chat_shortcut;
            creators["tell expected dps"] = &ChatActionContext::tell_expected_dps;
            
        }

    private:
        static Action* range(PlayerbotAI* botAI) { return new RangeAction(botAI); }
        static Action* flag(PlayerbotAI* botAI) { return new FlagAction(botAI); }
        static Action* craft(PlayerbotAI* botAI) { return new SetCraftAction(botAI); }
        static Action* hire(PlayerbotAI* botAI) { return new HireAction(botAI); }
        static Action* wts(PlayerbotAI* botAI) { return new WtsAction(botAI); }
        static Action* cs(PlayerbotAI* botAI) { return new CustomStrategyEditAction(botAI); }
        static Action* debug(PlayerbotAI* botAI) { return new DebugAction(botAI); }
        static Action* mail(PlayerbotAI* botAI) { return new MailAction(botAI); }
        static Action* go(PlayerbotAI* botAI) { return new GoAction(botAI); }
        static Action* sendmail(PlayerbotAI* botAI) { return new SendMailAction(botAI); }
        static Action* formation(PlayerbotAI* botAI) { return new SetFormationAction(botAI); }
        static Action* stance(PlayerbotAI* botAI) { return new SetStanceAction(botAI); }
        static Action* tell_attackers(PlayerbotAI* botAI) { return new TellAttackersAction(botAI); }
        static Action* max_dps_chat_shortcut(PlayerbotAI* botAI) { return new MaxDpsChatShortcutAction(botAI); }
        static Action* save_mana(PlayerbotAI* botAI) { return new SaveManaAction(botAI); }
        static Action* who(PlayerbotAI* botAI) { return new WhoAction(botAI); }
        static Action* summon(PlayerbotAI* botAI) { return new SummonAction(botAI); }
        static Action* tell_target(PlayerbotAI* botAI) { return new TellTargetAction(botAI); }
        static Action* position(PlayerbotAI* botAI) { return new PositionAction(botAI); }
        static Action* spirit_healer(PlayerbotAI* botAI) { return new SpiritHealerAction(botAI); }
        static Action* rti(PlayerbotAI* botAI) { return new RtiAction(botAI); }
        static Action* invite(PlayerbotAI* botAI) { return new InviteToGroupAction(botAI); }
        static Action* spell(PlayerbotAI* botAI) { return new TellSpellAction(botAI); }
        static Action* cast_custom_spell(PlayerbotAI* botAI) { return new CastCustomSpellAction(botAI); }
        static Action* cast_custom_nc_spell(PlayerbotAI* botAI) { return new CastCustomNcSpellAction(botAI); }
        static Action* tank_attack_chat_shortcut(PlayerbotAI* botAI) { return new TankAttackChatShortcutAction(botAI); }
        static Action* grind_chat_shortcut(PlayerbotAI* botAI) { return new GrindChatShortcutAction(botAI); }
        static Action* flee_chat_shortcut(PlayerbotAI* botAI) { return new FleeChatShortcutAction(botAI); }
        static Action* runaway_chat_shortcut(PlayerbotAI* botAI) { return new GoawayChatShortcutAction(botAI); }
        static Action* stay_chat_shortcut(PlayerbotAI* botAI) { return new StayChatShortcutAction(botAI); }
        static Action* follow_chat_shortcut(PlayerbotAI* botAI) { return new FollowChatShortcutAction(botAI); }
        static Action* gb(PlayerbotAI* botAI) { return new GuildBankAction(botAI); }
        static Action* bank(PlayerbotAI* botAI) { return new BankAction(botAI); }
        static Action* help(PlayerbotAI* botAI) { return new HelpAction(botAI); }
        static Action* buff(PlayerbotAI* botAI) { return new BuffAction(botAI); }
        static Action* destroy(PlayerbotAI* botAI) { return new DestroyItemAction(botAI); }
        static Action* home(PlayerbotAI* botAI) { return new SetHomeAction(botAI); }
        static Action* chat(PlayerbotAI* botAI) { return new ChangeChatAction(botAI); }
        static Action* attack_my_target(PlayerbotAI* botAI) { return new AttackMyTargetAction(botAI); }
        static Action* trainer(PlayerbotAI* botAI) { return new TrainerAction(botAI); }
        static Action* co(PlayerbotAI* botAI) { return new ChangeCombatStrategyAction(botAI); }
        static Action* nc(PlayerbotAI* botAI) { return new ChangeNonCombatStrategyAction(botAI); }
        static Action* dead(PlayerbotAI* botAI) { return new ChangeDeadStrategyAction(botAI); }
        static Action* spells(PlayerbotAI* botAI) { return new ListSpellsAction(botAI); }
        static Action* talents(PlayerbotAI* botAI) { return new ChangeTalentsAction(botAI); }

        static Action* equip(PlayerbotAI* botAI) { return new EquipAction(botAI); }
        static Action* equip_upgrades(PlayerbotAI* botAI) { return new EquipUpgradesAction(botAI); }
        static Action* unequip(PlayerbotAI* botAI) { return new UnequipAction(botAI); }
        static Action* sell(PlayerbotAI* botAI) { return new SellAction(botAI); }
        static Action* buy(PlayerbotAI* botAI) { return new BuyAction(botAI); }
        static Action* reward(PlayerbotAI* botAI) { return new RewardAction(botAI); }
        static Action* trade(PlayerbotAI* botAI) { return new TradeAction(botAI); }

        static Action* item_count(PlayerbotAI* botAI) { return new TellItemCountAction(botAI); }
        static Action* use(PlayerbotAI* botAI) { return new UseItemAction(botAI); }
        static Action* repair(PlayerbotAI* botAI) { return new RepairAllAction(botAI); }
        static Action* taxi(PlayerbotAI* botAI) { return new TaxiAction(botAI); }
        static Action* teleport(PlayerbotAI* botAI) { return new TeleportAction(botAI); }
        static Action* release(PlayerbotAI* botAI) { return new ReleaseSpiritAction(botAI); }
        static Action* repop(PlayerbotAI* botAI) { return new RepopAction(botAI); }
        static Action* query_item_usage(PlayerbotAI* botAI) { return new QueryItemUsageAction(botAI); }
        static Action* query_quest(PlayerbotAI* botAI) { return new QueryQuestAction(botAI); }
        static Action* drop(PlayerbotAI* botAI) { return new DropQuestAction(botAI); }
        static Action* clean_quest_log(PlayerbotAI* botAI) { return new CleanQuestLogAction(botAI); }
        static Action* share(PlayerbotAI* botAI) { return new ShareQuestAction(botAI); }
        static Action* stats(PlayerbotAI* botAI) { return new StatsAction(botAI); }
        static Action* quests(PlayerbotAI* botAI) { return new ListQuestsAction(botAI); }
        static Action* leave(PlayerbotAI* botAI) { return new LeaveGroupAction(botAI); }
        static Action* reputation(PlayerbotAI* botAI) { return new TellReputationAction(botAI); }
        static Action* log(PlayerbotAI* botAI) { return new LogLevelAction(botAI); }
        static Action* los(PlayerbotAI* botAI) { return new TellLosAction(botAI); }
        static Action* aura(PlayerbotAI* ai) { return new TellAuraAction(ai); }
        static Action* ll(PlayerbotAI* botAI) { return new LootStrategyAction(botAI); }
        static Action* ss(PlayerbotAI* botAI) { return new SkipSpellsListAction(botAI); }
        static Action* add_all_loot(PlayerbotAI* botAI) { return new AddAllLootAction(botAI); }
        static Action* reset_ai(PlayerbotAI* botAI) { return new ResetAiAction(botAI); }
        static Action* gossip_hello(PlayerbotAI* botAI) { return new GossipHelloAction(botAI); }
        static Action* give_leader(PlayerbotAI* botAI) { return new GiveLeaderAction(botAI); }
        static Action* cheat(PlayerbotAI* botAI) { return new CheatAction(botAI); }
        static Action* ginvite(PlayerbotAI* botAI) { return new GuildInviteAction(botAI); }
        static Action* guild_promote(PlayerbotAI* botAI) { return new GuildPromoteAction(botAI); }
        static Action* guild_demote(PlayerbotAI* botAI) { return new GuildDemoteAction(botAI); }
        static Action* guild_remove(PlayerbotAI* botAI) { return new GuildRemoveAction(botAI); }
        static Action* guild_leave(PlayerbotAI* botAI) { return new GuildLeaveAction(botAI); }
        static Action* rtsc(PlayerbotAI* botAI) { return new RTSCAction(botAI); }
        static Action* naxx_chat_shortcut(PlayerbotAI* ai) { return new NaxxChatShortcutAction(ai); }
        static Action* bwl_chat_shortcut(PlayerbotAI* ai) { return new BwlChatShortcutAction(ai); }
        static Action* tell_expected_dps(PlayerbotAI* ai) { return new TellExpectedDpsAction(ai); }
};

#endif
