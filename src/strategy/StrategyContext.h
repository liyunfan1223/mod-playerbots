/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_STRATEGYCONTEXT_H
#define _PLAYERBOT_STRATEGYCONTEXT_H

#include "AttackEnemyPlayersStrategy.h"
#include "BattlegroundStrategy.h"
#include "CastTimeStrategy.h"
#include "ChatCommandHandlerStrategy.h"
#include "ConserveManaStrategy.h"
#include "CustomStrategy.h"
#include "DeadStrategy.h"
#include "DebugStrategy.h"
#include "DpsAssistStrategy.h"
#include "DuelStrategy.h"
#include "EmoteStrategy.h"
#include "FleeStrategy.h"
#include "FollowMasterStrategy.h"
#include "GrindingStrategy.h"
#include "GroupStrategy.h"
#include "GuardStrategy.h"
#include "GuildStrategy.h"
#include "KiteStrategy.h"
#include "LfgStrategy.h"
#include "LootNonCombatStrategy.h"
#include "MaintenanceStrategy.h"
#include "MarkRtiStrategy.h"
#include "MeleeCombatStrategy.h"
#include "MoveFromGroupStrategy.h"
#include "NamedObjectContext.h"
#include "NonCombatStrategy.h"
#include "PassiveStrategy.h"
#include "PullStrategy.h"
#include "QuestStrategies.h"
#include "RTSCStrategy.h"
#include "RacialsStrategy.h"
#include "RangedCombatStrategy.h"
#include "ReturnStrategy.h"
#include "RpgStrategy.h"
#include "RunawayStrategy.h"
#include "StayStrategy.h"
#include "TankAssistStrategy.h"
#include "TellTargetStrategy.h"
#include "ThreatStrategy.h"
#include "TravelStrategy.h"
#include "UseFoodStrategy.h"
#include "UsePotionsStrategy.h"
#include "WorldPacketHandlerStrategy.h"

class StrategyContext : public NamedObjectContext<Strategy>
{
public:
    StrategyContext()
    {
        creators["racials"] = &StrategyContext::racials;
        creators["loot"] = &StrategyContext::loot;
        creators["gather"] = &StrategyContext::gather;
        creators["emote"] = &StrategyContext::emote;
        creators["passive"] = &StrategyContext::passive;
        creators["save mana"] = &StrategyContext::auto_save_mana;
        creators["food"] = &StrategyContext::food;
        creators["chat"] = &StrategyContext::chat;
        creators["default"] = &StrategyContext::world_packet;
        creators["ready check"] = &StrategyContext::ready_check;
        creators["dead"] = &StrategyContext::dead;
        creators["flee"] = &StrategyContext::flee;
        creators["duel"] = &StrategyContext::duel;
        creators["start duel"] = &StrategyContext::start_duel;
        creators["kite"] = &StrategyContext::kite;
        creators["potions"] = &StrategyContext::potions;
        creators["cast time"] = &StrategyContext::cast_time;
        creators["threat"] = &StrategyContext::threat;
        creators["focus"] = &StrategyContext::focus;
        creators["tell target"] = &StrategyContext::tell_target;
        creators["pvp"] = &StrategyContext::pvp;
        creators["return"] = &StrategyContext::_return;
        creators["lfg"] = &StrategyContext::lfg;
        creators["custom"] = &StrategyContext::custom;
        creators["reveal"] = &StrategyContext::reveal;
        creators["collision"] = &StrategyContext::collision;
        creators["rpg"] = &StrategyContext::rpg;
        creators["travel"] = &StrategyContext::travel;
        creators["explore"] = &StrategyContext::explore;
        creators["map"] = &StrategyContext::map;
        creators["map full"] = &StrategyContext::map_full;
        creators["sit"] = &StrategyContext::sit;
        creators["mark rti"] = &StrategyContext::mark_rti;
        creators["adds"] = &StrategyContext::possible_adds;
        creators["close"] = &StrategyContext::close;
        creators["ranged"] = &StrategyContext::ranged;
        creators["behind"] = &StrategyContext::behind;
        creators["bg"] = &StrategyContext::bg;
        creators["battleground"] = &StrategyContext::battleground;
        creators["warsong"] = &StrategyContext::warsong;
        creators["alterac"] = &StrategyContext::alterac;
        creators["arathi"] = &StrategyContext::arathi;
        creators["eye"] = &StrategyContext::eye;
        creators["isle"] = &StrategyContext::isle;
        creators["arena"] = &StrategyContext::arena;
        creators["mount"] = &StrategyContext::mount;
        creators["rtsc"] = &StrategyContext::rtsc;
        creators["attack tagged"] = &StrategyContext::attack_tagged;
        creators["debug"] = &StrategyContext::debug;
        creators["debug move"] = &StrategyContext::debug_move;
        creators["debug rpg"] = &StrategyContext::debug_rpg;
        creators["debug spell"] = &StrategyContext::debug_spell;
        creators["debug quest"] = &StrategyContext::debug_quest;
        creators["maintenance"] = &StrategyContext::maintenance;
        creators["group"] = &StrategyContext::group;
        creators["guild"] = &StrategyContext::guild;
        creators["grind"] = &StrategyContext::grind;
        creators["avoid aoe"] = &StrategyContext::avoid_aoe;
        creators["tank face"] = &StrategyContext::tank_face;
        creators["move random"] = &StrategyContext::move_random;
        creators["formation"] = &StrategyContext::combat_formation;
        creators["move from group"] = &StrategyContext::move_from_group;
    }

private:
    static Strategy* behind(PlayerbotAI* botAI) { return new SetBehindCombatStrategy(botAI); }
    static Strategy* ranged(PlayerbotAI* botAI) { return new RangedCombatStrategy(botAI); }
    static Strategy* close(PlayerbotAI* botAI) { return new MeleeCombatStrategy(botAI); }
    static Strategy* mark_rti(PlayerbotAI* botAI) { return new MarkRtiStrategy(botAI); }
    static Strategy* tell_target(PlayerbotAI* botAI) { return new TellTargetStrategy(botAI); }
    static Strategy* threat(PlayerbotAI* botAI) { return new ThreatStrategy(botAI); }
    static Strategy* focus(PlayerbotAI* botAI) { return new FocusStrategy(botAI); }
    static Strategy* cast_time(PlayerbotAI* botAI) { return new CastTimeStrategy(botAI); }
    static Strategy* potions(PlayerbotAI* botAI) { return new UsePotionsStrategy(botAI); }
    static Strategy* kite(PlayerbotAI* botAI) { return new KiteStrategy(botAI); }
    static Strategy* duel(PlayerbotAI* botAI) { return new DuelStrategy(botAI); }
    static Strategy* start_duel(PlayerbotAI* botAI) { return new StartDuelStrategy(botAI); }
    static Strategy* flee(PlayerbotAI* botAI) { return new FleeStrategy(botAI); }
    static Strategy* dead(PlayerbotAI* botAI) { return new DeadStrategy(botAI); }
    static Strategy* racials(PlayerbotAI* botAI) { return new RacialsStrategy(botAI); }
    static Strategy* loot(PlayerbotAI* botAI) { return new LootNonCombatStrategy(botAI); }
    static Strategy* gather(PlayerbotAI* botAI) { return new GatherStrategy(botAI); }
    static Strategy* emote(PlayerbotAI* botAI) { return new EmoteStrategy(botAI); }
    static Strategy* passive(PlayerbotAI* botAI) { return new PassiveStrategy(botAI); }
    // static Strategy* conserve_mana(PlayerbotAI* botAI) { return new ConserveManaStrategy(botAI); }
    static Strategy* auto_save_mana(PlayerbotAI* botAI) { return new HealerAutoSaveManaStrategy(botAI); }
    static Strategy* food(PlayerbotAI* botAI) { return new UseFoodStrategy(botAI); }
    static Strategy* chat(PlayerbotAI* botAI) { return new ChatCommandHandlerStrategy(botAI); }
    static Strategy* world_packet(PlayerbotAI* botAI) { return new WorldPacketHandlerStrategy(botAI); }
    static Strategy* ready_check(PlayerbotAI* botAI) { return new ReadyCheckStrategy(botAI); }
    static Strategy* pvp(PlayerbotAI* botAI) { return new AttackEnemyPlayersStrategy(botAI); }
    static Strategy* _return(PlayerbotAI* botAI) { return new ReturnStrategy(botAI); }
    static Strategy* lfg(PlayerbotAI* botAI) { return new LfgStrategy(botAI); }
    static Strategy* custom(PlayerbotAI* botAI) { return new CustomStrategy(botAI); }
    static Strategy* reveal(PlayerbotAI* botAI) { return new RevealStrategy(botAI); }
    static Strategy* collision(PlayerbotAI* botAI) { return new CollisionStrategy(botAI); }
    static Strategy* rpg(PlayerbotAI* botAI) { return new RpgStrategy(botAI); }
    static Strategy* travel(PlayerbotAI* botAI) { return new TravelStrategy(botAI); }
    static Strategy* explore(PlayerbotAI* botAI) { return new ExploreStrategy(botAI); }
    static Strategy* map(PlayerbotAI* botAI) { return new MapStrategy(botAI); }
    static Strategy* map_full(PlayerbotAI* botAI) { return new MapFullStrategy(botAI); }
    static Strategy* sit(PlayerbotAI* botAI) { return new SitStrategy(botAI); }
    static Strategy* possible_adds(PlayerbotAI* botAI) { return new PossibleAddsStrategy(botAI); }
    static Strategy* mount(PlayerbotAI* botAI) { return new MountStrategy(botAI); }
    static Strategy* bg(PlayerbotAI* botAI) { return new BGStrategy(botAI); }
    static Strategy* battleground(PlayerbotAI* botAI) { return new BattlegroundStrategy(botAI); }
    static Strategy* warsong(PlayerbotAI* botAI) { return new WarsongStrategy(botAI); }
    static Strategy* alterac(PlayerbotAI* botAI) { return new AlteracStrategy(botAI); }
    static Strategy* arathi(PlayerbotAI* botAI) { return new ArathiStrategy(botAI); }
    static Strategy* eye(PlayerbotAI* botAI) { return new EyeStrategy(botAI); }
    static Strategy* isle(PlayerbotAI* botAI) { return new IsleStrategy(botAI); }
    static Strategy* arena(PlayerbotAI* botAI) { return new ArenaStrategy(botAI); }
    static Strategy* rtsc(PlayerbotAI* botAI) { return new RTSCStrategy(botAI); }
    static Strategy* attack_tagged(PlayerbotAI* botAI) { return new AttackTaggedStrategy(botAI); }
    static Strategy* debug(PlayerbotAI* botAI) { return new DebugStrategy(botAI); }
    static Strategy* debug_move(PlayerbotAI* botAI) { return new DebugMoveStrategy(botAI); }
    static Strategy* debug_rpg(PlayerbotAI* botAI) { return new DebugRpgStrategy(botAI); }
    static Strategy* debug_spell(PlayerbotAI* botAI) { return new DebugSpellStrategy(botAI); }
    static Strategy* debug_quest(PlayerbotAI* botAI) { return new DebugQuestStrategy(botAI); }
    static Strategy* maintenance(PlayerbotAI* botAI) { return new MaintenanceStrategy(botAI); }
    static Strategy* group(PlayerbotAI* botAI) { return new GroupStrategy(botAI); }
    static Strategy* guild (PlayerbotAI* botAI) { return new GuildStrategy(botAI); }
    static Strategy* grind(PlayerbotAI* botAI) { return new GrindingStrategy(botAI); }
    static Strategy* avoid_aoe(PlayerbotAI* botAI) { return new AvoidAoeStrategy(botAI); }
    static Strategy* tank_face(PlayerbotAI* botAI) { return new TankFaceStrategy(botAI); }
    static Strategy* move_random(PlayerbotAI* ai) { return new MoveRandomStrategy(ai); }
    static Strategy* combat_formation(PlayerbotAI* ai) { return new CombatFormationStrategy(ai); }
    static Strategy* move_from_group(PlayerbotAI* botAI) { return new MoveFromGroupStrategy(botAI); }
};

class MovementStrategyContext : public NamedObjectContext<Strategy>
{
public:
    MovementStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        creators["follow"] = &MovementStrategyContext::follow_master;
        creators["stay"] = &MovementStrategyContext::stay;
        creators["runaway"] = &MovementStrategyContext::runaway;
        creators["flee from adds"] = &MovementStrategyContext::flee_from_adds;
        creators["guard"] = &MovementStrategyContext::guard;
    }

private:
    static Strategy* guard(PlayerbotAI* botAI) { return new GuardStrategy(botAI); }
    static Strategy* follow_master(PlayerbotAI* botAI) { return new FollowMasterStrategy(botAI); }
    static Strategy* stay(PlayerbotAI* botAI) { return new StayStrategy(botAI); }
    static Strategy* runaway(PlayerbotAI* botAI) { return new RunawayStrategy(botAI); }
    static Strategy* flee_from_adds(PlayerbotAI* botAI) { return new FleeFromAddsStrategy(botAI); }
};

class AssistStrategyContext : public NamedObjectContext<Strategy>
{
public:
    AssistStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        creators["dps assist"] = &AssistStrategyContext::dps_assist;
        creators["dps aoe"] = &AssistStrategyContext::dps_aoe;
        creators["tank assist"] = &AssistStrategyContext::tank_assist;
    }

private:
    static Strategy* dps_assist(PlayerbotAI* botAI) { return new DpsAssistStrategy(botAI); }
    static Strategy* dps_aoe(PlayerbotAI* botAI) { return new DpsAoeStrategy(botAI); }
    static Strategy* tank_assist(PlayerbotAI* botAI) { return new TankAssistStrategy(botAI); }
};

class QuestStrategyContext : public NamedObjectContext<Strategy>
{
public:
    QuestStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        creators["quest"] = &QuestStrategyContext::quest;
        creators["accept all quests"] = &QuestStrategyContext::accept_all_quests;
    }

private:
    static Strategy* quest(PlayerbotAI* botAI) { return new DefaultQuestStrategy(botAI); }
    static Strategy* accept_all_quests(PlayerbotAI* botAI) { return new AcceptAllQuestsStrategy(botAI); }
};

#endif
