/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRIGGERCONTEXT_H
#define _PLAYERBOT_TRIGGERCONTEXT_H

#include "GenericTriggers.h"
#include "CureTriggers.h"
#include "GuildTriggers.h"
#include "LootTriggers.h"
#include "LfgTriggers.h"
#include "PvpTriggers.h"
#include "RpgTriggers.h"
#include "RtiTriggers.h"
#include "StuckTriggers.h"
#include "TravelTriggers.h"
#include "NamedObjectContext.h"
#include "RaidNaxxTriggers.h"

class PlayerbotAI;

class TriggerContext : public NamedObjectContext<Trigger>
{
    public:
        TriggerContext()
        {
            creators["return"] = &TriggerContext::_return;
            creators["sit"] = &TriggerContext::sit;
            creators["collision"] = &TriggerContext::collision;

            creators["timer"] = &TriggerContext::Timer;
            creators["random"] = &TriggerContext::Random;
            creators["seldom"] = &TriggerContext::seldom;
            creators["often"] = &TriggerContext::often;

            creators["target critical health"] = &TriggerContext::TargetCriticalHealth;

            creators["critical health"] = &TriggerContext::CriticalHealth;
            creators["low health"] = &TriggerContext::LowHealth;
            creators["medium health"] = &TriggerContext::MediumHealth;
            creators["almost full health"] = &TriggerContext::AlmostFullHealth;

            creators["low mana"] = &TriggerContext::LowMana;
            creators["medium mana"] = &TriggerContext::MediumMana;
            creators["high mana"] = &TriggerContext::HighMana;
            creators["almost full mana"] = &TriggerContext::AlmostFullMana;

            creators["party member critical health"] = &TriggerContext::PartyMemberCriticalHealth;
            creators["party member low health"] = &TriggerContext::PartyMemberLowHealth;
            creators["party member medium health"] = &TriggerContext::PartyMemberMediumHealth;
            creators["party member almost full health"] = &TriggerContext::PartyMemberAlmostFullHealth;

            creators["protect party member"]  = &TriggerContext::protect_party_member;

            creators["light rage available"] = &TriggerContext::LightRageAvailable;
            creators["medium rage available"] = &TriggerContext::MediumRageAvailable;
            creators["high rage available"] = &TriggerContext::HighRageAvailable;

            creators["light energy available"] = &TriggerContext::LightEnergyAvailable;
            creators["medium energy available"] = &TriggerContext::MediumEnergyAvailable;
            creators["high energy available"] = &TriggerContext::HighEnergyAvailable;

            creators["loot available"] = &TriggerContext::LootAvailable;
            creators["no attackers"] = &TriggerContext::NoAttackers;
            creators["no target"] = &TriggerContext::NoTarget;
            creators["target in sight"] = &TriggerContext::TargetInSight;
            creators["not dps target active"] = &TriggerContext::not_dps_target_active;
            creators["not dps aoe target active"] = &TriggerContext::not_dps_aoe_target_active;
            creators["has nearest adds"] = &TriggerContext::has_nearest_adds;
            creators["enemy player near"] = &TriggerContext::enemy_player_near;

            creators["tank assist"] = &TriggerContext::TankAssist;
            creators["lose aggro"] = &TriggerContext::LoseAggro;
            creators["has aggro"] = &TriggerContext::HasAggro;

            creators["light aoe"] = &TriggerContext::LightAoe;
            creators["medium aoe"] = &TriggerContext::MediumAoe;
            creators["high aoe"] = &TriggerContext::HighAoe;

            creators["has area debuff"] = &TriggerContext::HasAreaDebuff;


            creators["enemy out of melee"] = &TriggerContext::EnemyOutOfMelee;
            creators["enemy out of spell"] = &TriggerContext::EnemyOutOfSpell;
            creators["enemy too close for spell"] = &TriggerContext::enemy_too_close_for_spell;
            creators["enemy too close for shoot"] = &TriggerContext::enemy_too_close_for_shoot;
            creators["enemy too close for auto shot"] = &TriggerContext::enemy_too_close_for_auto_shot;
            creators["enemy too close for melee"] = &TriggerContext::enemy_too_close_for_melee;
            creators["enemy is close"] = &TriggerContext::enemy_is_close;
            creators["party member to heal out of spell range"] = &TriggerContext::party_member_to_heal_out_of_spell_range;

            creators["combo points available"] = &TriggerContext::ComboPointsAvailable;
            creators["combo points 3 available"] = &TriggerContext::ComboPoints3Available;

            creators["medium threat"] = &TriggerContext::MediumThreat;

            creators["dead"] = &TriggerContext::Dead;
            creators["corpse near"] = &TriggerContext::corpse_near;
            creators["party member dead"] = &TriggerContext::PartyMemberDead;
            creators["combat party member dead"] = &TriggerContext::CombatPartyMemberDead;
            creators["no pet"] = &TriggerContext::no_pet;
            creators["has pet"] = &TriggerContext::has_pet;
            creators["has attackers"] = &TriggerContext::has_attackers;
            creators["no possible targets"] = &TriggerContext::no_possible_targets;
            creators["possible adds"] = &TriggerContext::possible_adds;

            creators["no drink"] = &TriggerContext::no_drink;
            creators["no food"] = &TriggerContext::no_food;

            creators["panic"] = &TriggerContext::panic;
            creators["outnumbered"] = &TriggerContext::outnumbered;
            creators["behind target"] = &TriggerContext::behind_target;
            creators["not behind target"] = &TriggerContext::not_behind_target;
            creators["not facing target"] = &TriggerContext::not_facing_target;
            creators["far from master"] = &TriggerContext::far_from_master;
            creators["far from loot target"] = &TriggerContext::far_from_loot_target;
            creators["can loot"] = &TriggerContext::can_loot;
            creators["swimming"] = &TriggerContext::swimming;
            creators["target changed"] = &TriggerContext::target_changed;

            creators["critical aoe heal"] = &TriggerContext::critical_aoe_heal;
            creators["low aoe heal"] = &TriggerContext::low_aoe_heal;
            creators["medium aoe heal"] = &TriggerContext::medium_aoe_heal;
            creators["group heal occasion"] = &TriggerContext::group_heal_occasion;
            creators["medium group heal occasion"] = &TriggerContext::medium_group_heal_occasion;
            creators["invalid target"] = &TriggerContext::invalid_target;
            creators["lfg proposal active"] = &TriggerContext::lfg_proposal_active;

            creators["unknown dungeon"] = &TriggerContext::unknown_dungeon;

            creators["random bot update"] = &TriggerContext::random_bot_update_trigger;
            creators["no non bot players around"] = &TriggerContext::no_non_bot_players_around;
            creators["new player nearby"] = &TriggerContext::new_player_nearby;
            creators["no rpg target"] = &TriggerContext::no_rpg_target;
            creators["has rpg target"] = &TriggerContext::has_rpg_target;
            creators["far from rpg target"] = &TriggerContext::far_from_rpg_target;
            creators["near rpg target"] = &TriggerContext::near_rpg_target;
            creators["no travel target"] = &TriggerContext::no_travel_target;
            creators["far from travel target"] = &TriggerContext::far_from_travel_target;
            creators["no rti target"] = &TriggerContext::no_rti;

            creators["give food"] = &TriggerContext::give_food;
            creators["give water"] = &TriggerContext::give_water;

            creators["bg waiting"] = &TriggerContext::bg_waiting;
            creators["bg active"] = &TriggerContext::bg_active;
            creators["bg invite active"] = &TriggerContext::bg_invite_active;
            creators["inside bg"] = &TriggerContext::inside_bg;
            creators["player has no flag"] = &TriggerContext::player_has_no_flag;
            creators["player has flag"] = &TriggerContext::player_has_flag;
            creators["team has flag"] = &TriggerContext::team_has_flag;
            creators["enemy team has flag"] = &TriggerContext::enemy_team_has_flag;
            creators["enemy flagcarrier near"] = &TriggerContext::enemy_flagcarrier_near;
            creators["in Battleground"] = &TriggerContext::player_is_in_BATTLEGROUND;
            creators["in Battleground without flag"] = &TriggerContext::player_is_in_BATTLEGROUND_no_flag;
            creators["wants in bg"] = &TriggerContext::player_wants_in_bg;

            creators["mounted"] = &TriggerContext::mounted;

            // move to/enter dark portal if near
            creators["near dark portal"] = &TriggerContext::near_dark_portal;
            creators["at dark portal azeroth"] = &TriggerContext::at_dark_portal_azeroth;
            creators["at dark portal outland"] = &TriggerContext::at_dark_portal_outland;

            creators["vehicle near"] = &TriggerContext::vehicle_near;
            creators["in vehicle"] = &TriggerContext::in_vehicle;

            creators["need world buff"] = &TriggerContext::need_world_buff;
            creators["falling"] = &TriggerContext::falling;
            creators["falling far"] = &TriggerContext::falling_far;
            creators["move stuck"] = &TriggerContext::move_stuck;
            creators["move long stuck"] = &TriggerContext::move_long_stuck;
            creators["combat stuck"] = &TriggerContext::combat_stuck;
            creators["combat long stuck"] = &TriggerContext::combat_long_stuck;

            creators["petition signed"] = &TriggerContext::petition_signed;
            creators["buy tabard"] = &TriggerContext::buy_tabard;
            creators["leave large guild"] = &TriggerContext::leave_large_guild;

            creators["rpg"] = &TriggerContext::rpg;
            creators["rpg taxi"] = &TriggerContext::rpg_taxi;
            creators["rpg discover"] = &TriggerContext::rpg_discover;
            creators["rpg start quest"] = &TriggerContext::rpg_start_quest;
            creators["rpg end quest"] = &TriggerContext::rpg_end_quest;
            creators["rpg buy"] = &TriggerContext::rpg_buy;
            creators["rpg sell"] = &TriggerContext::rpg_sell;
            creators["rpg repair"] = &TriggerContext::rpg_repair;
            creators["rpg train"] = &TriggerContext::rpg_train;
            creators["rpg heal"] = &TriggerContext::rpg_heal;
            creators["rpg home bind"] = &TriggerContext::rpg_home_bind;
            creators["rpg queue bg"] = &TriggerContext::rpg_queue_bg;
            creators["rpg buy petition"] = &TriggerContext::rpg_buy_petition;
            creators["rpg use"] = &TriggerContext::rpg_use;
            creators["rpg spell"] = &TriggerContext::rpg_spell;
            creators["rpg craft"] = &TriggerContext::rpg_craft;
            creators["rpg trade useful"] = &TriggerContext::rpg_trade_useful;
            creators["rpg duel"] = &TriggerContext::rpg_duel;
        }

    private:
        static Trigger* give_food(PlayerbotAI* botAI) { return new GiveFoodTrigger(botAI); }
        static Trigger* give_water(PlayerbotAI* botAI) { return new GiveWaterTrigger(botAI); }
        static Trigger* no_rti(PlayerbotAI* botAI) { return new NoRtiTrigger(botAI); }
        static Trigger* _return(PlayerbotAI* botAI) { return new ReturnTrigger(botAI); }
        static Trigger* sit(PlayerbotAI* botAI) { return new SitTrigger(botAI); }
        static Trigger* far_from_rpg_target(PlayerbotAI* botAI) { return new FarFromRpgTargetTrigger(botAI); }
        static Trigger* near_rpg_target(PlayerbotAI* botAI) { return new NearRpgTargetTrigger(botAI); }
        static Trigger* far_from_travel_target(PlayerbotAI* botAI) { return new FarFromTravelTargetTrigger(botAI); }
        static Trigger* no_travel_target(PlayerbotAI* botAI) { return new NoTravelTargetTrigger(botAI); }
        static Trigger* no_rpg_target(PlayerbotAI* botAI) { return new NoRpgTargetTrigger(botAI); }
        static Trigger* has_rpg_target(PlayerbotAI* botAI) { return new HasRpgTargetTrigger(botAI); }
        static Trigger* collision(PlayerbotAI* botAI) { return new CollisionTrigger(botAI); }
        static Trigger* lfg_proposal_active(PlayerbotAI* botAI) { return new LfgProposalActiveTrigger(botAI); }
        static Trigger* unknown_dungeon(PlayerbotAI* botAI) { return new UnknownDungeonTrigger(botAI); }
        static Trigger* invalid_target(PlayerbotAI* botAI) { return new InvalidTargetTrigger(botAI); }
        static Trigger* critical_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "critical aoe heal", "critical", 2); }
        static Trigger* low_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "low aoe heal", "low", 2); }
        static Trigger* medium_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "medium aoe heal", "medium", 2); }
        static Trigger* group_heal_occasion(PlayerbotAI* ai) { return new AoeInGroupTrigger(ai, "group heal occasion", "almost full", 0.6); }
        static Trigger* medium_group_heal_occasion(PlayerbotAI* ai) { return new AoeInGroupTrigger(ai, "group heal occasion", "medium", 0.4); }
        static Trigger* target_changed(PlayerbotAI* botAI) { return new TargetChangedTrigger(botAI); }
        static Trigger* swimming(PlayerbotAI* botAI) { return new IsSwimmingTrigger(botAI); }
        static Trigger* no_possible_targets(PlayerbotAI* botAI) { return new NoPossibleTargetsTrigger(botAI); }
        static Trigger* possible_adds(PlayerbotAI* botAI) { return new PossibleAddsTrigger(botAI); }
        static Trigger* can_loot(PlayerbotAI* botAI) { return new CanLootTrigger(botAI); }
        static Trigger* far_from_loot_target(PlayerbotAI* botAI) { return new FarFromCurrentLootTrigger(botAI); }
        static Trigger* far_from_master(PlayerbotAI* botAI) { return new FarFromMasterTrigger(botAI); }
        static Trigger* behind_target(PlayerbotAI* botAI) { return new IsBehindTargetTrigger(botAI); }
        static Trigger* not_behind_target(PlayerbotAI* botAI) { return new IsNotBehindTargetTrigger(botAI); }
        static Trigger* not_facing_target(PlayerbotAI* botAI) { return new IsNotFacingTargetTrigger(botAI); }
        static Trigger* panic(PlayerbotAI* botAI) { return new PanicTrigger(botAI); }
        static Trigger* outnumbered(PlayerbotAI* botAI) { return new OutNumberedTrigger(botAI); }
        static Trigger* no_drink(PlayerbotAI* botAI) { return new NoDrinkTrigger(botAI); }
        static Trigger* no_food(PlayerbotAI* botAI) { return new NoFoodTrigger(botAI); }
        static Trigger* LightAoe(PlayerbotAI* botAI) { return new LightAoeTrigger(botAI); }
        static Trigger* MediumAoe(PlayerbotAI* botAI) { return new MediumAoeTrigger(botAI); }
        static Trigger* HighAoe(PlayerbotAI* botAI) { return new HighAoeTrigger(botAI); }
        static Trigger* HasAreaDebuff(PlayerbotAI* botAI) { return new HasAreaDebuffTrigger(botAI); }
        static Trigger* LoseAggro(PlayerbotAI* botAI) { return new LoseAggroTrigger(botAI); }
        static Trigger* HasAggro(PlayerbotAI* botAI) { return new HasAggroTrigger(botAI); }
        static Trigger* LowHealth(PlayerbotAI* botAI) { return new LowHealthTrigger(botAI); }
        static Trigger* MediumHealth(PlayerbotAI* botAI) { return new MediumHealthTrigger(botAI); }
        static Trigger* AlmostFullHealth(PlayerbotAI* botAI) { return new AlmostFullHealthTrigger(botAI); }
        static Trigger* CriticalHealth(PlayerbotAI* botAI) { return new CriticalHealthTrigger(botAI); }
        static Trigger* TargetCriticalHealth(PlayerbotAI* botAI) { return new TargetCriticalHealthTrigger(botAI); }
        static Trigger* LowMana(PlayerbotAI* botAI) { return new LowManaTrigger(botAI); }
        static Trigger* MediumMana(PlayerbotAI* botAI) { return new MediumManaTrigger(botAI); }
        static Trigger* HighMana(PlayerbotAI* botAI) { return new HighManaTrigger(botAI); }
        static Trigger* AlmostFullMana(PlayerbotAI* botAI) { return new AlmostFullManaTrigger(botAI); }
        static Trigger* LightRageAvailable(PlayerbotAI* botAI) { return new LightRageAvailableTrigger(botAI); }
        static Trigger* MediumRageAvailable(PlayerbotAI* botAI) { return new MediumRageAvailableTrigger(botAI); }
        static Trigger* HighRageAvailable(PlayerbotAI* botAI) { return new HighRageAvailableTrigger(botAI); }
        static Trigger* LightEnergyAvailable(PlayerbotAI* botAI) { return new LightEnergyAvailableTrigger(botAI); }
        static Trigger* MediumEnergyAvailable(PlayerbotAI* botAI) { return new MediumEnergyAvailableTrigger(botAI); }
        static Trigger* HighEnergyAvailable(PlayerbotAI* botAI) { return new HighEnergyAvailableTrigger(botAI); }
        static Trigger* LootAvailable(PlayerbotAI* botAI) { return new LootAvailableTrigger(botAI); }
        static Trigger* NoAttackers(PlayerbotAI* botAI) { return new NoAttackersTrigger(botAI); }
        static Trigger* TankAssist(PlayerbotAI* botAI) { return new TankAssistTrigger(botAI); }
        static Trigger* Timer(PlayerbotAI* botAI) { return new TimerTrigger(botAI); }
        static Trigger* NoTarget(PlayerbotAI* botAI) { return new NoTargetTrigger(botAI); }
        static Trigger* TargetInSight(PlayerbotAI* botAI) { return new TargetInSightTrigger(botAI); }
        static Trigger* not_dps_target_active(PlayerbotAI* botAI) { return new NotDpsTargetActiveTrigger(botAI); }
        static Trigger* not_dps_aoe_target_active(PlayerbotAI* botAI) { return new NotDpsAoeTargetActiveTrigger(botAI); }
        static Trigger* has_nearest_adds(PlayerbotAI* botAI) { return new HasNearestAddsTrigger(botAI); }
        static Trigger* enemy_player_near(PlayerbotAI* botAI) { return new EnemyPlayerNear(botAI); }
        static Trigger* Random(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "random", 20); }
        static Trigger* seldom(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "seldom", 300); }
        static Trigger* often(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "often", 5); }
        static Trigger* EnemyOutOfMelee(PlayerbotAI* botAI) { return new EnemyOutOfMeleeTrigger(botAI); }
        static Trigger* EnemyOutOfSpell(PlayerbotAI* botAI) { return new EnemyOutOfSpellRangeTrigger(botAI); }
        static Trigger* enemy_too_close_for_spell(PlayerbotAI* botAI) { return new EnemyTooCloseForSpellTrigger(botAI); }
        static Trigger* enemy_too_close_for_auto_shot(PlayerbotAI* botAI) { return new EnemyTooCloseForAutoShotTrigger(botAI); }
        static Trigger* enemy_too_close_for_shoot(PlayerbotAI* botAI) { return new EnemyTooCloseForShootTrigger(botAI); }
        static Trigger* enemy_too_close_for_melee(PlayerbotAI* botAI) { return new EnemyTooCloseForMeleeTrigger(botAI); }
        static Trigger* enemy_is_close(PlayerbotAI* botAI) { return new EnemyIsCloseTrigger(botAI); }
        static Trigger* party_member_to_heal_out_of_spell_range(PlayerbotAI* botAI) { return new PartyMemberToHealOutOfSpellRangeTrigger(botAI); }
        static Trigger* ComboPointsAvailable(PlayerbotAI* botAI) { return new ComboPointsAvailableTrigger(botAI); }
        static Trigger* ComboPoints3Available(PlayerbotAI* botAI) { return new ComboPointsAvailableTrigger(botAI, 3); }
        static Trigger* MediumThreat(PlayerbotAI* botAI) { return new MediumThreatTrigger(botAI); }
        static Trigger* Dead(PlayerbotAI* botAI) { return new DeadTrigger(botAI); }
        static Trigger* corpse_near(PlayerbotAI* botAI) { return new CorpseNearTrigger(botAI); }
        static Trigger* PartyMemberDead(PlayerbotAI* botAI) { return new PartyMemberDeadTrigger(botAI); }
        static Trigger* CombatPartyMemberDead(PlayerbotAI* botAI) { return new CombatPartyMemberDeadTrigger(botAI); }
        static Trigger* PartyMemberLowHealth(PlayerbotAI* botAI) { return new PartyMemberLowHealthTrigger(botAI); }
        static Trigger* PartyMemberMediumHealth(PlayerbotAI* botAI) { return new PartyMemberMediumHealthTrigger(botAI); }
        static Trigger* PartyMemberAlmostFullHealth(PlayerbotAI* botAI) { return new PartyMemberAlmostFullHealthTrigger(botAI); }
        static Trigger* PartyMemberCriticalHealth(PlayerbotAI* botAI) { return new PartyMemberCriticalHealthTrigger(botAI); }
        static Trigger* protect_party_member(PlayerbotAI* botAI) { return new ProtectPartyMemberTrigger(botAI); }
        static Trigger* no_pet(PlayerbotAI* botAI) { return new NoPetTrigger(botAI); }
        static Trigger* has_pet(PlayerbotAI* botAI) { return new HasPetTrigger(botAI); }
        static Trigger* has_attackers(PlayerbotAI* botAI) { return new HasAttackersTrigger(botAI); }
        static Trigger* random_bot_update_trigger(PlayerbotAI* botAI) { return new RandomBotUpdateTrigger(botAI); }
        static Trigger* no_non_bot_players_around(PlayerbotAI* botAI) { return new NoNonBotPlayersAroundTrigger(botAI); }
        static Trigger* new_player_nearby(PlayerbotAI* botAI) { return new NewPlayerNearbyTrigger(botAI); }
        static Trigger* bg_waiting(PlayerbotAI* botAI) { return new BgWaitingTrigger(botAI); }
        static Trigger* bg_active(PlayerbotAI* botAI) { return new BgActiveTrigger(botAI); }
        static Trigger* bg_invite_active(PlayerbotAI* botAI) { return new BgInviteActiveTrigger(botAI); }
        static Trigger* inside_bg(PlayerbotAI* botAI) { return new InsideBGTrigger(botAI); }
        static Trigger* player_has_no_flag(PlayerbotAI* botAI) { return new PlayerHasNoFlag(botAI); }
        static Trigger* player_has_flag(PlayerbotAI* botAI) { return new PlayerHasFlag(botAI); }
        static Trigger* team_has_flag(PlayerbotAI* botAI) { return new TeamHasFlag(botAI); }
        static Trigger* enemy_team_has_flag(PlayerbotAI* botAI) { return new EnemyTeamHasFlag(botAI); }
        static Trigger* enemy_flagcarrier_near(PlayerbotAI* botAI) { return new EnemyFlagCarrierNear(botAI); }
        static Trigger* player_is_in_BATTLEGROUND(PlayerbotAI* botAI) { return new PlayerIsInBattleground(botAI); }
        static Trigger* player_is_in_BATTLEGROUND_no_flag(PlayerbotAI* botAI) { return new PlayerIsInBattlegroundWithoutFlag(botAI); }
        static Trigger* mounted(PlayerbotAI* botAI) { return new IsMountedTrigger(botAI); }
        static Trigger* at_dark_portal_outland(PlayerbotAI* botAI) { return new AtDarkPortalOutlandTrigger(botAI); }
        static Trigger* at_dark_portal_azeroth(PlayerbotAI* botAI) { return new AtDarkPortalAzerothTrigger(botAI); }
        static Trigger* in_vehicle(PlayerbotAI* botAI) { return new InVehicleTrigger(botAI); }
        static Trigger* vehicle_near(PlayerbotAI* botAI) { return new VehicleNearTrigger(botAI); }
        static Trigger* near_dark_portal(PlayerbotAI* botAI) { return new NearDarkPortalTrigger(botAI); }
        static Trigger* need_world_buff(PlayerbotAI* botAI) { return new NeedWorldBuffTrigger(botAI); }
        static Trigger* falling(PlayerbotAI* botAI) { return new IsFallingTrigger(botAI); }
        static Trigger* falling_far(PlayerbotAI* botAI) { return new IsFallingFarTrigger(botAI); }
        static Trigger* move_stuck(PlayerbotAI* botAI) { return new MoveStuckTrigger(botAI); }
        static Trigger* move_long_stuck(PlayerbotAI* botAI) { return new MoveLongStuckTrigger(botAI); }
        static Trigger* combat_stuck(PlayerbotAI* botAI) { return new CombatStuckTrigger(botAI); }
        static Trigger* combat_long_stuck(PlayerbotAI* botAI) { return new CombatLongStuckTrigger(botAI); }
        static Trigger* player_wants_in_bg(PlayerbotAI* botAI) { return new PlayerWantsInBattlegroundTrigger(botAI); }
        static Trigger* petition_signed(PlayerbotAI* botAI) { return new PetitionTurnInTrigger(botAI); }
        static Trigger* buy_tabard(PlayerbotAI* botAI) { return new BuyTabardTrigger(botAI); }
        static Trigger* leave_large_guild(PlayerbotAI* botAI) { return new LeaveLargeGuildTrigger(botAI); }
        static Trigger* rpg(PlayerbotAI* botAI) { return new RpgTrigger(botAI); }
        static Trigger* rpg_taxi(PlayerbotAI* botAI) { return new RpgTaxiTrigger(botAI); }
        static Trigger* rpg_discover(PlayerbotAI* botAI) { return new RpgDiscoverTrigger(botAI); }
        static Trigger* rpg_start_quest(PlayerbotAI* botAI) { return new RpgStartQuestTrigger(botAI); }
        static Trigger* rpg_end_quest(PlayerbotAI* botAI) { return new RpgEndQuestTrigger(botAI); }
        static Trigger* rpg_buy(PlayerbotAI* botAI) { return new RpgBuyTrigger(botAI); }
        static Trigger* rpg_sell(PlayerbotAI* botAI) { return new RpgSellTrigger(botAI); }
        static Trigger* rpg_repair(PlayerbotAI* botAI) { return new RpgRepairTrigger(botAI); }
        static Trigger* rpg_train(PlayerbotAI* botAI) { return new RpgTrainTrigger(botAI); }
        static Trigger* rpg_heal(PlayerbotAI* botAI) { return new RpgHealTrigger(botAI); }
        static Trigger* rpg_home_bind(PlayerbotAI* botAI) { return new RpgHomeBindTrigger(botAI); }
        static Trigger* rpg_queue_bg(PlayerbotAI* botAI) { return new RpgQueueBGTrigger(botAI); }
        static Trigger* rpg_buy_petition(PlayerbotAI* botAI) { return new RpgBuyPetitionTrigger(botAI); }
        static Trigger* rpg_use(PlayerbotAI* botAI) { return new RpgUseTrigger(botAI); }
        static Trigger* rpg_spell(PlayerbotAI* botAI) { return new RpgUseTrigger(botAI); }
        static Trigger* rpg_craft(PlayerbotAI* botAI) { return new RpgCraftTrigger(botAI); }
        static Trigger* rpg_trade_useful(PlayerbotAI* botAI) { return new RpgTradeUsefulTrigger(botAI); }
        static Trigger* rpg_duel(PlayerbotAI* botAI) { return new RpgDuelTrigger(botAI); }
};

#endif

