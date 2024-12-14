/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MageAiObjectContext.h"

#include "ArcaneMageStrategy.h"
#include "FireMageStrategy.h"
#include "FrostMageStrategy.h"
#include "GenericMageNonCombatStrategy.h"
#include "MageActions.h"
#include "MageTriggers.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "PullStrategy.h"

class MageStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageStrategyFactoryInternal()
    {
        creators["nc"] = &MageStrategyFactoryInternal::nc;
        creators["pull"] = &MageStrategyFactoryInternal::pull;
        creators["fire aoe"] = &MageStrategyFactoryInternal::fire_aoe;
        creators["frost aoe"] = &MageStrategyFactoryInternal::frost_aoe;
        creators["arcane aoe"] = &MageStrategyFactoryInternal::arcane_aoe;
        creators["cure"] = &MageStrategyFactoryInternal::cure;
        creators["buff"] = &MageStrategyFactoryInternal::buff;
        creators["boost"] = &MageStrategyFactoryInternal::boost;
        creators["cc"] = &MageStrategyFactoryInternal::cc;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericMageNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* fire_aoe(PlayerbotAI* botAI) { return new FireMageAoeStrategy(botAI); }
    static Strategy* frost_aoe(PlayerbotAI* botAI) { return new FrostMageAoeStrategy(botAI); }
    static Strategy* arcane_aoe(PlayerbotAI* botAI) { return new ArcaneMageAoeStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new MageCureStrategy(botAI); }
    static Strategy* buff(PlayerbotAI* botAI) { return new MageBuffStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new MageBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new MageCcStrategy(botAI); }
};

class MageCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["frost"] = &MageCombatStrategyFactoryInternal::frost;
        creators["fire"] = &MageCombatStrategyFactoryInternal::fire;
        creators["arcane"] = &MageCombatStrategyFactoryInternal::arcane;
    }

private:
    static Strategy* frost(PlayerbotAI* botAI) { return new FrostMageStrategy(botAI); }
    static Strategy* fire(PlayerbotAI* botAI) { return new FireMageStrategy(botAI); }
    static Strategy* arcane(PlayerbotAI* botAI) { return new ArcaneMageStrategy(botAI); }
};

class MageBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bmana"] = &MageBuffStrategyFactoryInternal::bmana;
        creators["bdps"] = &MageBuffStrategyFactoryInternal::bdps;
    }

private:
    static Strategy* bmana(PlayerbotAI* botAI) { return new MageBuffManaStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new MageBuffDpsStrategy(botAI); }
};

class MageTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    MageTriggerFactoryInternal()
    {
        creators["fireball"] = &MageTriggerFactoryInternal::fireball;
        creators["pyroblast"] = &MageTriggerFactoryInternal::pyroblast;
        creators["combustion"] = &MageTriggerFactoryInternal::combustion;
        creators["fingers of frost single"] = &MageTriggerFactoryInternal::fingers_of_frost_single;
        creators["fingers of frost double"] = &MageTriggerFactoryInternal::fingers_of_frost_double;
        creators["brain freeze"] = &MageTriggerFactoryInternal::brain_freeze;
        creators["icy veins"] = &MageTriggerFactoryInternal::icy_veins;
        creators["cold snap"] = &MageTriggerFactoryInternal::cold_snap;
        creators["ice barrier"] = &MageTriggerFactoryInternal::ice_barrier;
        creators["arcane intellect"] = &MageTriggerFactoryInternal::arcane_intellect;
        creators["arcane intellect on party"] = &MageTriggerFactoryInternal::arcane_intellect_on_party;
        creators["mage armor"] = &MageTriggerFactoryInternal::mage_armor;
        creators["remove curse"] = &MageTriggerFactoryInternal::remove_curse;
        creators["remove curse on party"] = &MageTriggerFactoryInternal::remove_curse_on_party;
        creators["counterspell"] = &MageTriggerFactoryInternal::counterspell;
        creators["polymorph"] = &MageTriggerFactoryInternal::polymorph;
        creators["spellsteal"] = &MageTriggerFactoryInternal::spellsteal;
        creators["hot streak"] = &MageTriggerFactoryInternal::hot_streak;
        creators["living bomb"] = &MageTriggerFactoryInternal::living_bomb;
        creators["missile barrage"] = &MageTriggerFactoryInternal::missile_barrage;
        creators["arcane blast"] = &MageTriggerFactoryInternal::arcane_blast;
        creators["counterspell on enemy healer"] = &MageTriggerFactoryInternal::counterspell_enemy_healer;
        creators["arcane power"] = &MageTriggerFactoryInternal::arcane_power;
        creators["presence of mind"] = &MageTriggerFactoryInternal::presence_of_mind;
        creators["fire ward"] = &MageTriggerFactoryInternal::fire_ward;
        creators["frost ward"] = &MageTriggerFactoryInternal::frost_ward;
        creators["arcane blast stack"] = &MageTriggerFactoryInternal::arcane_blast_stack;
        creators["mirror image"] = &MageTriggerFactoryInternal::mirror_image;
        creators["frost nova on target"] = &MageTriggerFactoryInternal::frost_nova_on_target;
        creators["frostbite on target"] = &MageTriggerFactoryInternal::frostbite_on_target;
    }

private:
    static Trigger* presence_of_mind(PlayerbotAI* botAI) { return new PresenceOfMindTrigger(botAI); }
    static Trigger* frost_ward(PlayerbotAI* botAI) { return new FrostWardTrigger(botAI); }
    static Trigger* fire_ward(PlayerbotAI* botAI) { return new FireWardTrigger(botAI); }
    static Trigger* arcane_power(PlayerbotAI* botAI) { return new ArcanePowerTrigger(botAI); }
    static Trigger* hot_streak(PlayerbotAI* botAI) { return new HotStreakTrigger(botAI); }
    static Trigger* fireball(PlayerbotAI* botAI) { return new FireballTrigger(botAI); }
    static Trigger* pyroblast(PlayerbotAI* botAI) { return new PyroblastTrigger(botAI); }
    static Trigger* combustion(PlayerbotAI* botAI) { return new CombustionTrigger(botAI); }
    static Trigger* fingers_of_frost_single(PlayerbotAI* botAI) { return new FingersOfFrostSingleTrigger(botAI); }
    static Trigger* fingers_of_frost_double(PlayerbotAI* botAI) { return new FingersOfFrostDoubleTrigger(botAI); }
    static Trigger* brain_freeze(PlayerbotAI* botAI) { return new BrainFreezeTrigger(botAI); }
    static Trigger* icy_veins(PlayerbotAI* botAI) { return new IcyVeinsTrigger(botAI); }
    static Trigger* cold_snap(PlayerbotAI* botAI) { return new ColdSnapTrigger(botAI); }
    static Trigger* ice_barrier(PlayerbotAI* botAI) { return new IceBarrierTrigger(botAI); }
    static Trigger* arcane_intellect(PlayerbotAI* botAI) { return new ArcaneIntellectTrigger(botAI); }
    static Trigger* arcane_intellect_on_party(PlayerbotAI* botAI) { return new ArcaneIntellectOnPartyTrigger(botAI); }
    static Trigger* mage_armor(PlayerbotAI* botAI) { return new MageArmorTrigger(botAI); }
    static Trigger* remove_curse(PlayerbotAI* botAI) { return new RemoveCurseTrigger(botAI); }
    static Trigger* remove_curse_on_party(PlayerbotAI* botAI) { return new PartyMemberRemoveCurseTrigger(botAI); }
    static Trigger* counterspell(PlayerbotAI* botAI) { return new CounterspellInterruptSpellTrigger(botAI); }
    static Trigger* polymorph(PlayerbotAI* botAI) { return new PolymorphTrigger(botAI); }
    static Trigger* spellsteal(PlayerbotAI* botAI) { return new SpellstealTrigger(botAI); }
    static Trigger* living_bomb(PlayerbotAI* botAI) { return new LivingBombTrigger(botAI); }
    static Trigger* missile_barrage(PlayerbotAI* botAI) { return new MissileBarrageTrigger(botAI); }
    static Trigger* arcane_blast(PlayerbotAI* botAI) { return new ArcaneBlastTrigger(botAI); }
    static Trigger* counterspell_enemy_healer(PlayerbotAI* botAI) { return new CounterspellEnemyHealerTrigger(botAI); }
    static Trigger* arcane_blast_stack(PlayerbotAI* botAI) { return new ArcaneBlastStackTrigger(botAI); }
    static Trigger* mirror_image(PlayerbotAI* botAI) { return new MirrorImageTrigger(botAI); }
    static Trigger* frost_nova_on_target(PlayerbotAI* botAI) { return new FrostNovaOnTargetTrigger(botAI); }
    static Trigger* frostbite_on_target(PlayerbotAI* botAI) { return new FrostbiteOnTargetTrigger(botAI); }
};

class MageAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    MageAiObjectContextInternal()
    {
        creators["arcane power"] = &MageAiObjectContextInternal::arcane_power;
        creators["presence of mind"] = &MageAiObjectContextInternal::presence_of_mind;
        creators["frostbolt"] = &MageAiObjectContextInternal::frostbolt;
        creators["frostfire bolt"] = &MageAiObjectContextInternal::frostfire_bolt;
        creators["ice lance"] = &MageAiObjectContextInternal::ice_lance;
        creators["deep freeze"] = &MageAiObjectContextInternal::deep_freeze;
        creators["blizzard"] = &MageAiObjectContextInternal::blizzard;
        creators["cone of cold"] = &MageAiObjectContextInternal::cone_of_cold;
        creators["frost nova"] = &MageAiObjectContextInternal::frost_nova;
        creators["arcane intellect"] = &MageAiObjectContextInternal::arcane_intellect;
        creators["arcane intellect on party"] = &MageAiObjectContextInternal::arcane_intellect_on_party;
        creators["conjure water"] = &MageAiObjectContextInternal::conjure_water;
        creators["conjure food"] = &MageAiObjectContextInternal::conjure_food;
        creators["molten armor"] = &MageAiObjectContextInternal::molten_armor;
        creators["mage armor"] = &MageAiObjectContextInternal::mage_armor;
        creators["ice armor"] = &MageAiObjectContextInternal::ice_armor;
        creators["frost armor"] = &MageAiObjectContextInternal::frost_armor;
        creators["fireball"] = &MageAiObjectContextInternal::fireball;
        creators["pyroblast"] = &MageAiObjectContextInternal::pyroblast;
        creators["flamestrike"] = &MageAiObjectContextInternal::flamestrike;
        creators["fire blast"] = &MageAiObjectContextInternal::fire_blast;
        creators["scorch"] = &MageAiObjectContextInternal::scorch;
        creators["counterspell"] = &MageAiObjectContextInternal::counterspell;
        creators["remove curse"] = &MageAiObjectContextInternal::remove_curse;
        creators["remove curse on party"] = &MageAiObjectContextInternal::remove_curse_on_party;
        creators["remove lesser curse"] = &MageAiObjectContextInternal::remove_lesser_curse;
        creators["remove lesser curse on party"] = &MageAiObjectContextInternal::remove_lesser_curse_on_party;
        creators["icy veins"] = &MageAiObjectContextInternal::icy_veins;
        creators["cold snap"] = &MageAiObjectContextInternal::cold_snap;
        creators["ice barrier"] = &MageAiObjectContextInternal::ice_barrier;
        creators["summon water elemental"] = &MageAiObjectContextInternal::summon_water_elemental;
        creators["combustion"] = &MageAiObjectContextInternal::combustion;
        creators["ice block"] = &MageAiObjectContextInternal::ice_block;
        creators["polymorph"] = &MageAiObjectContextInternal::polymorph;
        creators["spellsteal"] = &MageAiObjectContextInternal::spellsteal;
        creators["living bomb"] = &MageAiObjectContextInternal::living_bomb;
        creators["dragon's breath"] = &MageAiObjectContextInternal::dragons_breath;
        creators["blast wave"] = &MageAiObjectContextInternal::blast_wave;
        creators["invisibility"] = &MageAiObjectContextInternal::invisibility;
        creators["evocation"] = &MageAiObjectContextInternal::evocation;
        creators["arcane blast"] = &MageAiObjectContextInternal::arcane_blast;
        creators["arcane barrage"] = &MageAiObjectContextInternal::arcane_barrage;
        creators["arcane missiles"] = &MageAiObjectContextInternal::arcane_missiles;
        creators["counterspell on enemy healer"] = &MageAiObjectContextInternal::counterspell_on_enemy_healer;
        creators["fire ward"] = &MageAiObjectContextInternal::fire_ward;
        creators["frost ward"] = &MageAiObjectContextInternal::frost_ward;
        creators["mirror image"] = &MageAiObjectContextInternal::mirror_image;
    }

private:
    static Action* presence_of_mind(PlayerbotAI* botAI) { return new CastPresenceOfMindAction(botAI); }
    static Action* frost_ward(PlayerbotAI* botAI) { return new CastFrostWardAction(botAI); }
    static Action* fire_ward(PlayerbotAI* botAI) { return new CastFireWardAction(botAI); }
    static Action* arcane_power(PlayerbotAI* botAI) { return new CastArcanePowerAction(botAI); }
    static Action* arcane_missiles(PlayerbotAI* botAI) { return new CastArcaneMissilesAction(botAI); }
    static Action* arcane_barrage(PlayerbotAI* botAI) { return new CastArcaneBarrageAction(botAI); }
    static Action* arcane_blast(PlayerbotAI* botAI) { return new CastArcaneBlastAction(botAI); }
    static Action* frostbolt(PlayerbotAI* botAI) { return new CastFrostboltAction(botAI); }
    static Action* frostfire_bolt(PlayerbotAI* botAI) { return new CastFrostfireBoltAction(botAI); }
    static Action* ice_lance(PlayerbotAI* botAI) { return new CastIceLanceAction(botAI); }
    static Action* deep_freeze(PlayerbotAI* botAI) { return new CastDeepFreezeAction(botAI); }
    static Action* blizzard(PlayerbotAI* botAI) { return new CastBlizzardAction(botAI); }
    static Action* cone_of_cold(PlayerbotAI* botAI) { return new CastConeOfColdAction(botAI); }
    static Action* frost_nova(PlayerbotAI* botAI) { return new CastFrostNovaAction(botAI); }
    static Action* arcane_intellect(PlayerbotAI* botAI) { return new CastArcaneIntellectAction(botAI); }
    static Action* arcane_intellect_on_party(PlayerbotAI* botAI) { return new CastArcaneIntellectOnPartyAction(botAI); }
    static Action* conjure_water(PlayerbotAI* botAI) { return new CastConjureWaterAction(botAI); }
    static Action* conjure_food(PlayerbotAI* botAI) { return new CastConjureFoodAction(botAI); }
    static Action* molten_armor(PlayerbotAI* botAI) { return new CastMoltenArmorAction(botAI); }
    static Action* mage_armor(PlayerbotAI* botAI) { return new CastMageArmorAction(botAI); }
    static Action* ice_armor(PlayerbotAI* botAI) { return new CastIceArmorAction(botAI); }
    static Action* frost_armor(PlayerbotAI* botAI) { return new CastFrostArmorAction(botAI); }
    static Action* fireball(PlayerbotAI* botAI) { return new CastFireballAction(botAI); }
    static Action* pyroblast(PlayerbotAI* botAI) { return new CastPyroblastAction(botAI); }
    static Action* flamestrike(PlayerbotAI* botAI) { return new CastFlamestrikeAction(botAI); }
    static Action* fire_blast(PlayerbotAI* botAI) { return new CastFireBlastAction(botAI); }
    static Action* scorch(PlayerbotAI* botAI) { return new CastScorchAction(botAI); }
    static Action* counterspell(PlayerbotAI* botAI) { return new CastCounterspellAction(botAI); }
    static Action* remove_curse(PlayerbotAI* botAI) { return new CastRemoveCurseAction(botAI); }
    static Action* remove_curse_on_party(PlayerbotAI* botAI) { return new CastRemoveCurseOnPartyAction(botAI); }
    static Action* remove_lesser_curse(PlayerbotAI* botAI) { return new CastRemoveLesserCurseAction(botAI); }
    static Action* remove_lesser_curse_on_party(PlayerbotAI* botAI)
    {
        return new CastRemoveLesserCurseOnPartyAction(botAI);
    }
    static Action* icy_veins(PlayerbotAI* botAI) { return new CastIcyVeinsAction(botAI); }
    static Action* cold_snap(PlayerbotAI* botAI) { return new CastColdSnapAction(botAI); }
    static Action* ice_barrier(PlayerbotAI* botAI) { return new CastIceBarrierAction(botAI); }
    static Action* summon_water_elemental(PlayerbotAI* botAI) { return new CastSummonWaterElementalAction(botAI); }
    static Action* combustion(PlayerbotAI* botAI) { return new CastCombustionAction(botAI); }
    static Action* ice_block(PlayerbotAI* botAI) { return new CastIceBlockAction(botAI); }
    static Action* polymorph(PlayerbotAI* botAI) { return new CastPolymorphAction(botAI); }
    static Action* spellsteal(PlayerbotAI* botAI) { return new CastSpellstealAction(botAI); }
    static Action* living_bomb(PlayerbotAI* botAI) { return new CastLivingBombAction(botAI); }
    static Action* dragons_breath(PlayerbotAI* botAI) { return new CastDragonsBreathAction(botAI); }
    static Action* blast_wave(PlayerbotAI* botAI) { return new CastBlastWaveAction(botAI); }
    static Action* invisibility(PlayerbotAI* botAI) { return new CastInvisibilityAction(botAI); }
    static Action* evocation(PlayerbotAI* botAI) { return new CastEvocationAction(botAI); }
    static Action* counterspell_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new CastCounterspellOnEnemyHealerAction(botAI);
    }
    static Action* mirror_image(PlayerbotAI* botAI) { return new CastMirrorImageAction(botAI); }
};

MageAiObjectContext::MageAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new MageStrategyFactoryInternal());
    strategyContexts.Add(new MageCombatStrategyFactoryInternal());
    strategyContexts.Add(new MageBuffStrategyFactoryInternal());
    actionContexts.Add(new MageAiObjectContextInternal());
    triggerContexts.Add(new MageTriggerFactoryInternal());
}
