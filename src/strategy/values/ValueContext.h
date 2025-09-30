/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VALUECONTEXT_H
#define _PLAYERBOT_VALUECONTEXT_H

#include "ActiveSpellValue.h"
#include "AlwaysLootListValue.h"
#include "AoeHealValues.h"
#include "AoeValues.h"
#include "AttackerCountValues.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "AttackersValue.h"
#include "AvailableLootValue.h"
#include "BudgetValues.h"
#include "CcTargetValue.h"
#include "ChatValue.h"
#include "CollisionValue.h"
#include "CraftValue.h"
#include "CurrentCcTargetValue.h"
#include "CurrentTargetValue.h"
#include "DistanceValue.h"
#include "DpsTargetValue.h"
#include "DuelTargetValue.h"
#include "EnemyHealerTargetValue.h"
#include "EnemyPlayerValue.h"
#include "EstimatedLifetimeValue.h"
#include "Formations.h"
#include "GrindTargetValue.h"
#include "GroupValues.h"
#include "GuildValues.h"
#include "HasAvailableLootValue.h"
#include "HasTotemValue.h"
#include "InvalidTargetValue.h"
#include "IsBehindValue.h"
#include "IsFacingValue.h"
#include "IsMovingValue.h"
#include "ItemCountValue.h"
#include "ItemForSpellValue.h"
#include "ItemUsageValue.h"
#include "LastMovementValue.h"
#include "LastSaidValue.h"
#include "LastSpellCastTimeValue.h"
#include "LastSpellCastValue.h"
#include "LeastHpTargetValue.h"
#include "LfgValues.h"
#include "LineTargetValue.h"
#include "LogLevelValue.h"
#include "LootStrategyValue.h"
#include "MaintenanceValues.h"
#include "ManaSaveLevelValue.h"
#include "MasterTargetValue.h"
#include "NearestAdsValue.h"
#include "NearestCorpsesValue.h"
#include "NearestFriendlyPlayersValue.h"
#include "NearestGameObjects.h"
#include "NearestNonBotPlayersValue.h"
#include "NearestNpcsValue.h"
#include "NewPlayerNearbyValue.h"
#include "OutfitListValue.h"
#include "PartyMemberToDispel.h"
#include "PartyMemberToHeal.h"
#include "PartyMemberToResurrect.h"
#include "PartyMemberWithoutAuraValue.h"
#include "PartyMemberWithoutItemValue.h"
#include "PetTargetValue.h"
#include "PositionValue.h"
#include "PossibleRpgTargetsValue.h"
#include "PossibleTargetsValue.h"
#include "PvpValues.h"
#include "QuestValues.h"
#include "RTSCValues.h"
#include "RandomBotUpdateValue.h"
#include "RangeValues.h"
#include "RpgValues.h"
#include "RtiTargetValue.h"
#include "RtiValue.h"
#include "SelfTargetValue.h"
#include "SkipSpellsListValue.h"
#include "SnareTargetValue.h"
#include "SpellCastUsefulValue.h"
#include "SpellIdValue.h"
#include "Stances.h"
#include "StatsValues.h"
#include "TankTargetValue.h"
#include "ThreatValues.h"
#include "TradeValues.h"
#include "Value.h"

class PlayerbotAI;

class ValueContext : public NamedObjectContext<UntypedValue>
{
public:
    ValueContext()
    {
        creators["active spell"] = &ValueContext::active_spell;
        creators["craft"] = &ValueContext::craft;
        creators["collision"] = &ValueContext::collision;
        creators["skip spells list"] = &ValueContext::skip_spells_list_value;
        creators["nearest game objects"] = &ValueContext::nearest_game_objects;
        creators["nearest game objects no los"] = &ValueContext::nearest_game_objects_no_los;
        creators["closest game objects"] = &ValueContext::closest_game_objects;
        creators["nearest npcs"] = &ValueContext::nearest_npcs;
        creators["nearest hostile npcs"] = &ValueContext::nearest_hostile_npcs;
        creators["nearest totems"] = &ValueContext::nearest_totems;
        creators["nearest vehicles"] = &ValueContext::nearest_vehicles;
        creators["nearest vehicles far"] = &ValueContext::nearest_vehicles_far;
        creators["nearest friendly players"] = &ValueContext::nearest_friendly_players;
        creators["closest friendly players"] = &ValueContext::closest_friendly_players;
        creators["nearest enemy players"] = &ValueContext::nearest_enemy_players;
        creators["possible targets"] = &ValueContext::possible_targets;
        creators["possible targets no los"] = &ValueContext::possible_targets_no_los;
        creators["possible triggers"] = &ValueContext::possible_triggers;
        creators["possible adds"] = &ValueContext::possible_adds;
        creators["prioritized targets"] = &ValueContext::prioritized_targets;
        creators["all targets"] = &ValueContext::all_targets;
        creators["possible rpg targets"] = &ValueContext::possible_rpg_targets;
        creators["possible new rpg targets"] = &ValueContext::possible_new_rpg_targets;
        creators["possible new rpg game objects"] = &ValueContext::possible_new_rpg_game_objects;
        creators["nearest adds"] = &ValueContext::nearest_adds;
        creators["nearest corpses"] = &ValueContext::nearest_corpses;
        creators["log level"] = &ValueContext::log_level;
        creators["party member without aura"] = &ValueContext::party_member_without_aura;
        creators["attacker without aura"] = &ValueContext::attacker_without_aura;
        creators["melee attacker without aura"] = &ValueContext::melee_attacker_without_aura;
        creators["party member to heal"] = &ValueContext::party_member_to_heal;
        creators["party member to resurrect"] = &ValueContext::party_member_to_resurrect;
        creators["current target"] = &ValueContext::current_target;
        creators["self target"] = &ValueContext::self_target;
        creators["master target"] = &ValueContext::master;
        creators["line target"] = &ValueContext::line_target;
        creators["tank target"] = &ValueContext::tank_target;
        creators["dps target"] = &ValueContext::dps_target;
        creators["dps aoe target"] = &ValueContext::dps_aoe_target;
        creators["least hp target"] = &ValueContext::least_hp_target;
        creators["enemy player target"] = &ValueContext::enemy_player_target;
        creators["cc target"] = &ValueContext::cc_target;
        creators["current cc target"] = &ValueContext::current_cc_target;
        creators["pet target"] = &ValueContext::pet_target;
        creators["old target"] = &ValueContext::old_target;
        creators["grind target"] = &ValueContext::grind_target;
        creators["rti target"] = &ValueContext::rti_target;
        creators["rti cc target"] = &ValueContext::rti_cc_target;
        creators["duel target"] = &ValueContext::duel_target;
        creators["party member to dispel"] = &ValueContext::party_member_to_dispel;
        creators["party member to protect"] = &ValueContext::party_member_to_protect;
        creators["health"] = &ValueContext::health;
        creators["rage"] = &ValueContext::rage;
        creators["energy"] = &ValueContext::energy;
        creators["mana"] = &ValueContext::mana;
        creators["combo"] = &ValueContext::combo;
        creators["dead"] = &ValueContext::dead;
        creators["pet dead"] = &ValueContext::pet_dead;
        creators["pet happy"] = &ValueContext::pet_happy;
        creators["has mana"] = &ValueContext::has_mana;
        creators["attacker count"] = &ValueContext::attacker_count;
        creators["my attacker count"] = &ValueContext::my_attacker_count;
        creators["has aggro"] = &ValueContext::has_aggro;
        creators["mounted"] = &ValueContext::mounted;
        creators["custom_glyphs"] = &ValueContext::custom_glyphs;   // Added for custom glyphs

        creators["can loot"] = &ValueContext::can_loot;
        creators["loot target"] = &ValueContext::loot_target;
        creators["available loot"] = &ValueContext::available_loot;
        creators["has available loot"] = &ValueContext::has_available_loot;
        creators["always loot list"] = &ValueContext::always_loot_list;
        creators["loot strategy"] = &ValueContext::loot_strategy;
        creators["last movement"] = &ValueContext::last_movement;
        creators["stay time"] = &ValueContext::stay_time;
        creators["last taxi"] = &ValueContext::last_movement;
        creators["last area trigger"] = &ValueContext::last_movement;
        creators["distance"] = &ValueContext::distance;
        creators["moving"] = &ValueContext::moving;
        creators["swimming"] = &ValueContext::swimming;
        creators["behind"] = &ValueContext::behind;
        creators["facing"] = &ValueContext::facing;

        creators["item count"] = &ValueContext::item_count;
        creators["inventory items"] = &ValueContext::inventory_item;

        creators["spell id"] = &ValueContext::spell_id;
        creators["vehicle spell id"] = &ValueContext::vehicle_spell_id;
        creators["item for spell"] = &ValueContext::item_for_spell;
        creators["spell cast useful"] = &ValueContext::spell_cast_useful;
        creators["last spell cast"] = &ValueContext::last_spell_cast;
        creators["last spell cast time"] = &ValueContext::last_spell_cast_time;
        creators["chat"] = &ValueContext::chat;
        creators["has totem"] = &ValueContext::has_totem;

        creators["aoe heal"] = &ValueContext::aoe_heal;

        creators["rti cc"] = &ValueContext::rti_cc;
        creators["rti"] = &ValueContext::rti;
        creators["position"] = &ValueContext::position;
        creators["pos"] = &ValueContext::pos;
        creators["current position"] = &ValueContext::current_position;
        creators["threat"] = &ValueContext::threat;

        creators["balance"] = &ValueContext::balance;
        creators["attackers"] = &ValueContext::attackers;
        creators["invalid target"] = &ValueContext::invalid_target;
        creators["mana save level"] = &ValueContext::mana_save_level;
        creators["combat"] = &ValueContext::combat;
        creators["lfg proposal"] = &ValueContext::lfg_proposal;
        creators["bag space"] = &ValueContext::bag_space;
        creators["durability"] = &ValueContext::durability;
        creators["max repair cost"] = &ValueContext::max_repair_cost;
        creators["repair cost"] = &ValueContext::repair_cost;
        creators["train cost"] = &ValueContext::train_cost;
        creators["enemy healer target"] = &ValueContext::enemy_healer_target;
        creators["snare target"] = &ValueContext::snare_target;
        creators["formation"] = &ValueContext::formation;
        creators["stance"] = &ValueContext::stance;
        creators["item usage"] = &ValueContext::item_usage;
        creators["speed"] = &ValueContext::speed;
        creators["last said"] = &ValueContext::last_said;
        creators["last emote"] = &ValueContext::last_emote;

        creators["aoe count"] = &ValueContext::aoe_count;
        creators["aoe position"] = &ValueContext::aoe_position;
        creators["outfit list"] = &ValueContext::outfit_list_value;

        creators["random bot update"] = &ValueContext::random_bot_update_value;
        creators["nearest non bot players"] = &ValueContext::nearest_non_bot_players;
        creators["new player nearby"] = &ValueContext::new_player_nearby;
        creators["already seen players"] = &ValueContext::already_seen_players;
        creators["rpg target"] = &ValueContext::rpg_target;
        creators["ignore rpg target"] = &ValueContext::ignore_rpg_target;
        creators["next rpg action"] = &ValueContext::next_rpg_action;
        creators["travel target"] = &ValueContext::travel_target;
        creators["talk target"] = &ValueContext::talk_target;
        creators["pull target"] = &ValueContext::pull_target;
        creators["group"] = &ValueContext::group;
        creators["range"] = &ValueContext::range;
        creators["inside target"] = &ValueContext::inside_target;
        creators["party member without item"] = &ValueContext::party_member_without_item;
        creators["party member without food"] = &ValueContext::party_member_without_food;
        creators["party member without water"] = &ValueContext::party_member_without_water;
        creators["death count"] = &ValueContext::death_count;

        creators["bg type"] = &ValueContext::bg_type;
        creators["rpg bg type"] = &ValueContext::rpg_bg_type;
        creators["arena type"] = &ValueContext::arena_type;
        creators["bg role"] = &ValueContext::bg_role;
        creators["bg master"] = &ValueContext::bg_master;
        creators["enemy flag carrier"] = &ValueContext::enemy_fc;
        creators["team flag carrier"] = &ValueContext::team_fc;

        creators["home bind"] = &ValueContext::home_bind;
        creators["last long move"] = &ValueContext::last_long_move;

        creators["free quest log slots"] = &ValueContext::free_quest_log_slots;
        creators["dialog status"] = &ValueContext::dialog_status;
        creators["dialog status quest"] = &ValueContext::dialog_status_quest;
        creators["can accept quest npc"] = &ValueContext::can_accept_quest_npc;
        creators["can accept quest low level npc"] = &ValueContext::can_accept_quest_low_level_npc;
        creators["can turn in quest npc"] = &ValueContext::can_turn_in_quest_npc;

        creators["money needed for"] = &ValueContext::money_needed_for;
        creators["total money needed for"] = &ValueContext::total_money_needed_for;
        creators["free money for"] = &ValueContext::free_money_for;
        creators["should get money"] = &ValueContext::should_get_money;

        creators["can move around"] = &ValueContext::can_move_around;
        creators["should home bind"] = &ValueContext::should_home_bind;
        creators["should repair"] = &ValueContext::should_repair;
        creators["can repair"] = &ValueContext::can_repair;
        creators["should sell"] = &ValueContext::should_sell;
        creators["can sell"] = &ValueContext::can_sell;
        creators["can fight equal"] = &ValueContext::can_fight_equal;
        creators["can fight elite"] = &ValueContext::can_fight_elite;
        creators["can fight boss"] = &ValueContext::can_fight_boss;

        creators["group members"] = &ValueContext::group_members;
        creators["following party"] = &ValueContext::following_party;
        creators["near leader"] = &ValueContext::near_leader;

        creators["and"] = &ValueContext::and_value;
        creators["group count"] = &ValueContext::group_count;
        creators["group and"] = &ValueContext::group_and;
        creators["group or"] = &ValueContext::group_or;
        creators["group ready"] = &ValueContext::group_ready;

        creators["petition signs"] = &ValueContext::petition_signs;

        creators["experience"] = &ValueContext::experience;

        creators["entry loot usage"] = &ValueContext::entry_loot_usage;
        creators["has upgrade"] = &ValueContext::has_upgrade;
        creators["items useful to give"] = &ValueContext::items_useful_to_give;

        creators["see spell location"] = &ValueContext::see_spell_location;
        creators["RTSC selected"] = &ValueContext::RTSC_selected;
        creators["RTSC next spell action"] = &ValueContext::RTSC_next_spell_action;
        creators["RTSC saved location"] = &ValueContext::RTSC_saved_location;

        creators["has area debuff"] = &ValueContext::has_area_debuff;

        creators["main tank"] = &ValueContext::main_tank;
        creators["find target"] = &ValueContext::find_target;
        creators["boss target"] = &ValueContext::boss_target;
        creators["nearest triggers"] = &ValueContext::nearest_triggers;
        creators["neglect threat"] = &ValueContext::neglect_threat;
        creators["estimated lifetime"] = &ValueContext::expected_lifetime;
        creators["estimated group dps"] = &ValueContext::expected_group_dps;
        creators["area debuff"] = &ValueContext::area_debuff;
        creators["nearest trap with damage"] = &ValueContext::nearest_trap_with_damange;
        creators["disperse distance"] = &ValueContext::disperse_distance;
        creators["last flee angle"] = &ValueContext::last_flee_angle;
        creators["last flee timestamp"] = &ValueContext::last_flee_timestamp;
        creators["recently flee info"] = &ValueContext::recently_flee_info;
    }

private:
    static UntypedValue* party_member_without_water(PlayerbotAI* botAI)
    {
        return new PartyMemberWithoutWaterValue(botAI);
    }
    static UntypedValue* party_member_without_food(PlayerbotAI* botAI)
    {
        return new PartyMemberWithoutFoodValue(botAI);
    }
    static UntypedValue* party_member_without_item(PlayerbotAI* botAI)
    {
        return new PartyMemberWithoutItemValue(botAI);
    }
    static UntypedValue* inside_target(PlayerbotAI* botAI) { return new InsideTargetValue(botAI); }
    static UntypedValue* range(PlayerbotAI* botAI) { return new RangeValue(botAI); }
    static UntypedValue* active_spell(PlayerbotAI* botAI) { return new ActiveSpellValue(botAI); }
    static UntypedValue* group(PlayerbotAI* botAI) { return new IsInGroupValue(botAI); }
    static UntypedValue* craft(PlayerbotAI* botAI) { return new CraftValue(botAI); }
    static UntypedValue* collision(PlayerbotAI* botAI) { return new CollisionValue(botAI); }
    static UntypedValue* already_seen_players(PlayerbotAI* botAI) { return new AlreadySeenPlayersValue(botAI); }
    static UntypedValue* new_player_nearby(PlayerbotAI* botAI) { return new NewPlayerNearbyValue(botAI); }
    static UntypedValue* item_usage(PlayerbotAI* botAI) { return new ItemUsageValue(botAI); }
    static UntypedValue* formation(PlayerbotAI* botAI) { return new FormationValue(botAI); }
    static UntypedValue* stance(PlayerbotAI* botAI) { return new StanceValue(botAI); }
    static UntypedValue* mana_save_level(PlayerbotAI* botAI) { return new ManaSaveLevelValue(botAI); }
    static UntypedValue* invalid_target(PlayerbotAI* botAI) { return new InvalidTargetValue(botAI); }
    static UntypedValue* balance(PlayerbotAI* botAI) { return new BalancePercentValue(botAI); }
    static UntypedValue* attackers(PlayerbotAI* botAI) { return new AttackersValue(botAI); }

    static UntypedValue* position(PlayerbotAI* botAI) { return new PositionValue(botAI); }
    static UntypedValue* pos(PlayerbotAI* ai) { return new SinglePositionValue(ai); }
    static UntypedValue* current_position(PlayerbotAI* botAI) { return new CurrentPositionValue(botAI); }
    static UntypedValue* rti(PlayerbotAI* botAI) { return new RtiValue(botAI); }
    static UntypedValue* rti_cc(PlayerbotAI* botAI) { return new RtiCcValue(botAI); }

    static UntypedValue* aoe_heal(PlayerbotAI* botAI) { return new AoeHealValue(botAI); }

    static UntypedValue* chat(PlayerbotAI* botAI) { return new ChatValue(botAI); }
    static UntypedValue* last_spell_cast(PlayerbotAI* botAI) { return new LastSpellCastValue(botAI); }
    static UntypedValue* last_spell_cast_time(PlayerbotAI* botAI) { return new LastSpellCastTimeValue(botAI); }
    static UntypedValue* spell_cast_useful(PlayerbotAI* botAI) { return new SpellCastUsefulValue(botAI); }
    static UntypedValue* item_for_spell(PlayerbotAI* botAI) { return new ItemForSpellValue(botAI); }
    static UntypedValue* spell_id(PlayerbotAI* botAI) { return new SpellIdValue(botAI); }
    static UntypedValue* vehicle_spell_id(PlayerbotAI* botAI) { return new VehicleSpellIdValue(botAI); }
    static UntypedValue* inventory_item(PlayerbotAI* botAI) { return new InventoryItemValue(botAI); }
    static UntypedValue* item_count(PlayerbotAI* botAI) { return new ItemCountValue(botAI); }
    static UntypedValue* behind(PlayerbotAI* botAI) { return new IsBehindValue(botAI); }
    static UntypedValue* facing(PlayerbotAI* botAI) { return new IsFacingValue(botAI); }
    static UntypedValue* moving(PlayerbotAI* botAI) { return new IsMovingValue(botAI); }
    static UntypedValue* swimming(PlayerbotAI* botAI) { return new IsSwimmingValue(botAI); }
    static UntypedValue* distance(PlayerbotAI* botAI) { return new DistanceValue(botAI); }
    static UntypedValue* last_movement(PlayerbotAI* botAI) { return new LastMovementValue(botAI); }
    static UntypedValue* stay_time(PlayerbotAI* botAI) { return new StayTimeValue(botAI); }

    static UntypedValue* can_loot(PlayerbotAI* botAI) { return new CanLootValue(botAI); }
    static UntypedValue* available_loot(PlayerbotAI* botAI) { return new AvailableLootValue(botAI); }
    static UntypedValue* loot_target(PlayerbotAI* botAI) { return new LootTargetValue(botAI); }
    static UntypedValue* has_available_loot(PlayerbotAI* botAI) { return new HasAvailableLootValue(botAI); }
    static UntypedValue* always_loot_list(PlayerbotAI* botAI) { return new AlwaysLootListValue(botAI); }
    static UntypedValue* loot_strategy(PlayerbotAI* botAI) { return new LootStrategyValue(botAI); }

    static UntypedValue* attacker_count(PlayerbotAI* botAI) { return new AttackerCountValue(botAI); }
    static UntypedValue* my_attacker_count(PlayerbotAI* botAI) { return new MyAttackerCountValue(botAI); }
    static UntypedValue* has_aggro(PlayerbotAI* botAI) { return new HasAggroValue(botAI); }
    static UntypedValue* mounted(PlayerbotAI* botAI) { return new IsMountedValue(botAI); }
    static UntypedValue* health(PlayerbotAI* botAI) { return new HealthValue(botAI); }
    static UntypedValue* rage(PlayerbotAI* botAI) { return new RageValue(botAI); }
    static UntypedValue* energy(PlayerbotAI* botAI) { return new EnergyValue(botAI); }
    static UntypedValue* mana(PlayerbotAI* botAI) { return new ManaValue(botAI); }
    static UntypedValue* combo(PlayerbotAI* botAI) { return new ComboPointsValue(botAI); }
    static UntypedValue* dead(PlayerbotAI* botAI) { return new IsDeadValue(botAI); }
    static UntypedValue* pet_happy(PlayerbotAI* botAI) { return new PetIsHappyValue(botAI); }
    static UntypedValue* pet_dead(PlayerbotAI* botAI) { return new PetIsDeadValue(botAI); }
    static UntypedValue* has_mana(PlayerbotAI* botAI) { return new HasManaValue(botAI); }
    static UntypedValue* nearest_game_objects(PlayerbotAI* botAI) { return new NearestGameObjects(botAI); }
    static UntypedValue* nearest_game_objects_no_los(PlayerbotAI* botAI)
    {
        return new NearestGameObjects(botAI, sPlayerbotAIConfig->sightDistance, true);
    }
    static UntypedValue* closest_game_objects(PlayerbotAI* botAI)
    {
        return new NearestGameObjects(botAI, INTERACTION_DISTANCE);
    }
    static UntypedValue* log_level(PlayerbotAI* botAI) { return new LogLevelValue(botAI); }
    static UntypedValue* nearest_npcs(PlayerbotAI* botAI) { return new NearestNpcsValue(botAI); }
    static UntypedValue* nearest_hostile_npcs(PlayerbotAI* botAI) { return new NearestHostileNpcsValue(botAI); }
    static UntypedValue* nearest_totems(PlayerbotAI* botAI) { return new NearestTotemsValue(botAI); }
    static UntypedValue* nearest_vehicles(PlayerbotAI* botAI) { return new NearestVehiclesValue(botAI); }
    static UntypedValue* nearest_vehicles_far(PlayerbotAI* botAI) { return new NearestVehiclesValue(botAI, 200.0f); }
    static UntypedValue* nearest_friendly_players(PlayerbotAI* botAI) { return new NearestFriendlyPlayersValue(botAI); }
    static UntypedValue* closest_friendly_players(PlayerbotAI* botAI)
    {
        return new NearestFriendlyPlayersValue(botAI, INTERACTION_DISTANCE);
    }
    static UntypedValue* nearest_enemy_players(PlayerbotAI* botAI) { return new NearestEnemyPlayersValue(botAI); }
    static UntypedValue* nearest_corpses(PlayerbotAI* botAI) { return new NearestCorpsesValue(botAI); }
    static UntypedValue* possible_rpg_targets(PlayerbotAI* botAI) { return new PossibleRpgTargetsValue(botAI); }
    static UntypedValue* possible_new_rpg_targets(PlayerbotAI* botAI) { return new PossibleNewRpgTargetsValue(botAI); }
    static UntypedValue* possible_new_rpg_game_objects(PlayerbotAI* botAI) { return new PossibleNewRpgGameObjectsValue(botAI); }
    static UntypedValue* possible_targets(PlayerbotAI* botAI) { return new PossibleTargetsValue(botAI); }
    static UntypedValue* possible_triggers(PlayerbotAI* botAI) { return new PossibleTriggersValue(botAI); }
    static UntypedValue* possible_targets_no_los(PlayerbotAI* botAI)
    {
        return new PossibleTargetsValue(botAI, "possible targets", sPlayerbotAIConfig->sightDistance, true);
    }
    static UntypedValue* possible_adds(PlayerbotAI* botAI) { return new PossibleAddsValue(botAI); }
    static UntypedValue* prioritized_targets(PlayerbotAI* botAI) { return new PrioritizedTargetsValue(botAI); }
    static UntypedValue* all_targets(PlayerbotAI* botAI) { return new AllTargetsValue(botAI); }
    static UntypedValue* nearest_adds(PlayerbotAI* botAI) { return new NearestAddsValue(botAI); }
    static UntypedValue* party_member_without_aura(PlayerbotAI* botAI)
    {
        return new PartyMemberWithoutAuraValue(botAI);
    }
    static UntypedValue* attacker_without_aura(PlayerbotAI* botAI) { return new AttackerWithoutAuraTargetValue(botAI); }
    static UntypedValue* melee_attacker_without_aura(PlayerbotAI* botAI)
    {
        return new MeleeAttackerWithoutAuraTargetValue(botAI);
    }
    static UntypedValue* party_member_to_heal(PlayerbotAI* botAI) { return new PartyMemberToHeal(botAI); }
    static UntypedValue* party_member_to_resurrect(PlayerbotAI* botAI) { return new PartyMemberToResurrect(botAI); }
    static UntypedValue* party_member_to_dispel(PlayerbotAI* botAI) { return new PartyMemberToDispel(botAI); }
    static UntypedValue* party_member_to_protect(PlayerbotAI* botAI) { return new PartyMemberToProtect(botAI); }
    static UntypedValue* current_target(PlayerbotAI* botAI) { return new CurrentTargetValue(botAI); }
    static UntypedValue* old_target(PlayerbotAI* botAI) { return new CurrentTargetValue(botAI); }
    static UntypedValue* self_target(PlayerbotAI* botAI) { return new SelfTargetValue(botAI); }
    static UntypedValue* master(PlayerbotAI* botAI) { return new MasterTargetValue(botAI); }
    static UntypedValue* line_target(PlayerbotAI* botAI) { return new LineTargetValue(botAI); }
    static UntypedValue* tank_target(PlayerbotAI* botAI) { return new TankTargetValue(botAI); }
    static UntypedValue* dps_target(PlayerbotAI* botAI) { return new DpsTargetValue(botAI); }
    static UntypedValue* dps_aoe_target(PlayerbotAI* botAI) { return new DpsAoeTargetValue(botAI); }
    static UntypedValue* least_hp_target(PlayerbotAI* botAI) { return new LeastHpTargetValue(botAI); }
    static UntypedValue* enemy_player_target(PlayerbotAI* botAI) { return new EnemyPlayerValue(botAI); }
    static UntypedValue* cc_target(PlayerbotAI* botAI) { return new CcTargetValue(botAI); }
    static UntypedValue* current_cc_target(PlayerbotAI* botAI) { return new CurrentCcTargetValue(botAI); }
    static UntypedValue* pet_target(PlayerbotAI* botAI) { return new PetTargetValue(botAI); }
    static UntypedValue* grind_target(PlayerbotAI* botAI) { return new GrindTargetValue(botAI); }
    static UntypedValue* rti_target(PlayerbotAI* botAI) { return new RtiTargetValue(botAI); }
    static UntypedValue* rti_cc_target(PlayerbotAI* botAI) { return new RtiCcTargetValue(botAI); }
    static UntypedValue* duel_target(PlayerbotAI* botAI) { return new DuelTargetValue(botAI); }
    static UntypedValue* has_totem(PlayerbotAI* botAI) { return new HasTotemValue(botAI); }
    static UntypedValue* threat(PlayerbotAI* botAI) { return new ThreatValue(botAI); }
    static UntypedValue* combat(PlayerbotAI* botAI) { return new IsInCombatValue(botAI); }
    static UntypedValue* lfg_proposal(PlayerbotAI* botAI) { return new LfgProposalValue(botAI); }
    static UntypedValue* bag_space(PlayerbotAI* botAI) { return new BagSpaceValue(botAI); }
    static UntypedValue* durability(PlayerbotAI* botAI) { return new DurabilityValue(botAI); }
    static UntypedValue* max_repair_cost(PlayerbotAI* botAI) { return new MaxGearRepairCostValue(botAI); }
    static UntypedValue* repair_cost(PlayerbotAI* botAI) { return new RepairCostValue(botAI); }
    static UntypedValue* train_cost(PlayerbotAI* botAI) { return new TrainCostValue(botAI); }
    static UntypedValue* enemy_healer_target(PlayerbotAI* botAI) { return new EnemyHealerTargetValue(botAI); }
    static UntypedValue* snare_target(PlayerbotAI* botAI) { return new SnareTargetValue(botAI); }
    static UntypedValue* speed(PlayerbotAI* botAI) { return new SpeedValue(botAI); }
    static UntypedValue* last_said(PlayerbotAI* botAI) { return new LastSaidValue(botAI); }
    static UntypedValue* last_emote(PlayerbotAI* botAI) { return new LastEmoteValue(botAI); }
    static UntypedValue* aoe_count(PlayerbotAI* botAI) { return new AoeCountValue(botAI); }
    static UntypedValue* aoe_position(PlayerbotAI* botAI) { return new AoePositionValue(botAI); }
    static UntypedValue* outfit_list_value(PlayerbotAI* botAI) { return new OutfitListValue(botAI); }
    static UntypedValue* random_bot_update_value(PlayerbotAI* botAI) { return new RandomBotUpdateValue(botAI); }
    static UntypedValue* nearest_non_bot_players(PlayerbotAI* botAI) { return new NearestNonBotPlayersValue(botAI); }
    static UntypedValue* skip_spells_list_value(PlayerbotAI* botAI) { return new SkipSpellsListValue(botAI); }
    static UntypedValue* rpg_target(PlayerbotAI* botAI) { return new RpgTargetValue(botAI); }
    static UntypedValue* ignore_rpg_target(PlayerbotAI* botAI) { return new IgnoreRpgTargetValue(botAI); }
    static UntypedValue* talk_target(PlayerbotAI* botAI) { return new TalkTargetValue(botAI); }
    static UntypedValue* next_rpg_action(PlayerbotAI* botAI) { return new NextRpgActionValue(botAI); }
    static UntypedValue* travel_target(PlayerbotAI* botAI) { return new TravelTargetValue(botAI); }
    static UntypedValue* pull_target(PlayerbotAI* botAI) { return new PullTargetValue(botAI); }

    static UntypedValue* bg_master(PlayerbotAI* botAI) { return new BgMasterValue(botAI); }
    static UntypedValue* bg_role(PlayerbotAI* botAI) { return new BgRoleValue(botAI); }
    static UntypedValue* arena_type(PlayerbotAI* botAI) { return new ArenaTypeValue(botAI); }
    static UntypedValue* bg_type(PlayerbotAI* botAI) { return new BgTypeValue(botAI); }
    static UntypedValue* rpg_bg_type(PlayerbotAI* botAI) { return new RpgBgTypeValue(botAI); }
    static UntypedValue* team_fc(PlayerbotAI* botAI) { return new FlagCarrierValue(botAI, true, true); }
    static UntypedValue* enemy_fc(PlayerbotAI* botAI) { return new FlagCarrierValue(botAI, false, true); }

    static UntypedValue* last_long_move(PlayerbotAI* botAI) { return new LastLongMoveValue(botAI); }
    static UntypedValue* home_bind(PlayerbotAI* botAI) { return new HomeBindValue(botAI); }

    static UntypedValue* free_quest_log_slots(PlayerbotAI* botAI) { return new FreeQuestLogSlotValue(botAI); }
    static UntypedValue* dialog_status(PlayerbotAI* botAI) { return new DialogStatusValue(botAI); }
    static UntypedValue* dialog_status_quest(PlayerbotAI* botAI) { return new DialogStatusQuestValue(botAI); }
    static UntypedValue* can_accept_quest_npc(PlayerbotAI* botAI) { return new CanAcceptQuestValue(botAI); }
    static UntypedValue* can_accept_quest_low_level_npc(PlayerbotAI* botAI)
    {
        return new CanAcceptQuestLowLevelValue(botAI);
    }
    static UntypedValue* can_turn_in_quest_npc(PlayerbotAI* botAI) { return new CanTurnInQuestValue(botAI); }

    static UntypedValue* money_needed_for(PlayerbotAI* botAI) { return new MoneyNeededForValue(botAI); }
    static UntypedValue* total_money_needed_for(PlayerbotAI* botAI) { return new TotalMoneyNeededForValue(botAI); }
    static UntypedValue* free_money_for(PlayerbotAI* botAI) { return new FreeMoneyForValue(botAI); }
    static UntypedValue* should_get_money(PlayerbotAI* botAI) { return new ShouldGetMoneyValue(botAI); }

    static UntypedValue* can_move_around(PlayerbotAI* botAI) { return new CanMoveAroundValue(botAI); }
    static UntypedValue* should_home_bind(PlayerbotAI* botAI) { return new ShouldHomeBindValue(botAI); }
    static UntypedValue* should_repair(PlayerbotAI* botAI) { return new ShouldRepairValue(botAI); }
    static UntypedValue* can_repair(PlayerbotAI* botAI) { return new CanRepairValue(botAI); }
    static UntypedValue* should_sell(PlayerbotAI* botAI) { return new ShouldSellValue(botAI); }
    static UntypedValue* can_sell(PlayerbotAI* botAI) { return new CanSellValue(botAI); }
    static UntypedValue* can_fight_equal(PlayerbotAI* botAI) { return new CanFightEqualValue(botAI); }
    static UntypedValue* can_fight_elite(PlayerbotAI* botAI) { return new CanFightEliteValue(botAI); }
    static UntypedValue* can_fight_boss(PlayerbotAI* botAI) { return new CanFightBossValue(botAI); }

    static UntypedValue* death_count(PlayerbotAI* botAI) { return new DeathCountValue(botAI); }

    static UntypedValue* group_members(PlayerbotAI* botAI) { return new GroupMembersValue(botAI); }
    static UntypedValue* following_party(PlayerbotAI* botAI) { return new IsFollowingPartyValue(botAI); }
    static UntypedValue* near_leader(PlayerbotAI* botAI) { return new IsNearLeaderValue(botAI); }
    static UntypedValue* and_value(PlayerbotAI* botAI) { return new BoolANDValue(botAI); }
    static UntypedValue* group_count(PlayerbotAI* botAI) { return new GroupBoolCountValue(botAI); }
    static UntypedValue* group_and(PlayerbotAI* botAI) { return new GroupBoolANDValue(botAI); }
    static UntypedValue* group_or(PlayerbotAI* botAI) { return new GroupBoolORValue(botAI); }
    static UntypedValue* group_ready(PlayerbotAI* botAI) { return new GroupReadyValue(botAI); }

    static UntypedValue* petition_signs(PlayerbotAI* botAI) { return new PetitionSignsValue(botAI); }

    static UntypedValue* experience(PlayerbotAI* botAI) { return new ExperienceValue(botAI); }

    static UntypedValue* entry_loot_usage(PlayerbotAI* botAI) { return new EntryLootUsageValue(botAI); }
    static UntypedValue* has_upgrade(PlayerbotAI* botAI) { return new HasUpgradeValue(botAI); }
    static UntypedValue* items_useful_to_give(PlayerbotAI* botAI) { return new ItemsUsefulToGiveValue(botAI); }

    static UntypedValue* see_spell_location(PlayerbotAI* botAI) { return new SeeSpellLocationValue(botAI); }
    static UntypedValue* RTSC_selected(PlayerbotAI* botAI) { return new RTSCSelectedValue(botAI); }
    static UntypedValue* RTSC_next_spell_action(PlayerbotAI* botAI) { return new RTSCNextSpellActionValue(botAI); }
    static UntypedValue* RTSC_saved_location(PlayerbotAI* botAI) { return new RTSCSavedLocationValue(botAI); }

    static UntypedValue* has_area_debuff(PlayerbotAI* botAI) { return new HasAreaDebuffValue(botAI); }

    static UntypedValue* main_tank(PlayerbotAI* ai) { return new PartyMemberMainTankValue(ai); }
    static UntypedValue* find_target(PlayerbotAI* ai) { return new FindTargetValue(ai); }
    static UntypedValue* boss_target(PlayerbotAI* ai) { return new BossTargetValue(ai); }
    static UntypedValue* nearest_triggers(PlayerbotAI* ai) { return new NearestTriggersValue(ai); }
    static UntypedValue* neglect_threat(PlayerbotAI* ai) { return new NeglectThreatResetValue(ai); }
    static UntypedValue* expected_lifetime(PlayerbotAI* ai) { return new EstimatedLifetimeValue(ai); }
    static UntypedValue* expected_group_dps(PlayerbotAI* ai) { return new EstimatedGroupDpsValue(ai); }
    static UntypedValue* area_debuff(PlayerbotAI* ai) { return new AreaDebuffValue(ai); }
    static UntypedValue* nearest_trap_with_damange(PlayerbotAI* ai) { return new NearestTrapWithDamageValue(ai); }
    static UntypedValue* disperse_distance(PlayerbotAI* ai) { return new DisperseDistanceValue(ai); }
    static UntypedValue* last_flee_angle(PlayerbotAI* ai) { return new LastFleeAngleValue(ai); }
    static UntypedValue* last_flee_timestamp(PlayerbotAI* ai) { return new LastFleeTimestampValue(ai); }
    static UntypedValue* recently_flee_info(PlayerbotAI* ai) { return new RecentlyFleeInfo(ai); }
    // -------------------------------------------------------
    // Flag for cutom glyphs : true when /w bot glyph equip
    // -------------------------------------------------------
    static UntypedValue* custom_glyphs(PlayerbotAI* ai)
    {
        return new ManualSetValue<bool>(ai, false, "custom_glyphs");
    }
};

#endif
