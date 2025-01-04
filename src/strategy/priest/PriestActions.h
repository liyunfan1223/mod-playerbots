/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PRIESTACTIONS_H
#define _PLAYERBOT_PRIESTACTIONS_H

#include "GenericSpellActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

class PlayerbotAI;

// disc
BUFF_ACTION(CastPowerWordFortitudeAction, "power word: fortitude");
BUFF_PARTY_ACTION(CastPowerWordFortitudeOnPartyAction, "power word: fortitude");
BUFF_PARTY_ACTION(CastPrayerOfFortitudeOnPartyAction, "prayer of fortitude");
BUFF_ACTION(CastPowerWordShieldAction, "power word: shield");

BUFF_ACTION(CastInnerFireAction, "inner fire");
CURE_ACTION(CastDispelMagicAction, "dispel magic");
CURE_PARTY_ACTION(CastDispelMagicOnPartyAction, "dispel magic", DISPEL_MAGIC);
SPELL_ACTION(CastDispelMagicOnTargetAction, "dispel magic");
CC_ACTION(CastShackleUndeadAction, "shackle undead");
SPELL_ACTION_U(CastManaBurnAction, "mana burn",
               AI_VALUE2(uint8, "mana", "self target") < 50 && AI_VALUE2(uint8, "mana", "current target") >= 20);
BUFF_ACTION(CastLevitateAction, "levitate");
BUFF_ACTION(CastDivineSpiritAction, "divine spirit");
BUFF_PARTY_ACTION(CastDivineSpiritOnPartyAction, "divine spirit");
BUFF_PARTY_ACTION(CastPrayerOfSpiritOnPartyAction, "prayer of spirit");
// disc 2.4.3
SPELL_ACTION(CastMassDispelAction, "mass dispel");

// disc talents
BUFF_ACTION(CastPowerInfusionAction, "power infusion");
BUFF_PARTY_ACTION(CastPowerInfusionOnPartyAction, "power infusion");
BUFF_ACTION(CastInnerFocusAction, "inner focus");
// disc 2.4.3 talents
BUFF_ACTION(CastPainSuppressionAction, "pain suppression");
PROTECT_ACTION(CastPainSuppressionProtectAction, "pain suppression");

// holy
HEAL_ACTION(CastLesserHealAction, "lesser heal");
HEAL_ACTION(CastHealAction, "heal");
HEAL_ACTION(CastGreaterHealAction, "greater heal");
HEAL_ACTION(CastFlashHealAction, "flash heal");
HEAL_ACTION(CastRenewAction, "renew");
HEAL_PARTY_ACTION(CastLesserHealOnPartyAction, "lesser heal", 50.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastHealOnPartyAction, "heal", 50.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastGreaterHealOnPartyAction, "greater heal", 50.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastPowerWordShieldOnPartyAction, "power word: shield", 15.0f, HealingManaEfficiency::VERY_HIGH);
HEAL_PARTY_ACTION(CastFlashHealOnPartyAction, "flash heal", 15.0f, HealingManaEfficiency::LOW);
HEAL_PARTY_ACTION(CastRenewOnPartyAction, "renew", 15.0f, HealingManaEfficiency::VERY_HIGH);
// HEAL_PARTY_ACTION(CastPrayerOfMendingAction, "prayer of mending", 10.0f, HealingManaEfficiency::HIGH);
class CastPrayerOfMendingAction : public HealPartyMemberAction
{
public:
    CastPrayerOfMendingAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "prayer of mending", 10.0f, HealingManaEfficiency::HIGH, false) {}
};

HEAL_PARTY_ACTION(CastBindingHealAction, "binding heal", 15.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastPrayerOfHealingAction, "prayer of healing", 15.0f, HealingManaEfficiency::MEDIUM);
// AOE_HEAL_ACTION(CastCircleOfHealingAction, "circle of healing", 15.0f, HealingManaEfficiency::HIGH);
class CastCircleOfHealingAction : public HealPartyMemberAction
{
public:
    CastCircleOfHealingAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "circle of healing", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};
AOE_HEAL_ACTION(CastLightwellAction, "lightwell", 15.0f, HealingManaEfficiency::MEDIUM);

SPELL_ACTION(CastSmiteAction, "smite");
SPELL_ACTION(CastHolyNovaAction, "holy nova");

RESS_ACTION(CastResurrectionAction, "resurrection");

CURE_ACTION(CastCureDiseaseAction, "cure disease");
CURE_PARTY_ACTION(CastCureDiseaseOnPartyAction, "cure disease", DISPEL_DISEASE);
CURE_ACTION(CastAbolishDiseaseAction, "abolish disease");
CURE_PARTY_ACTION(CastAbolishDiseaseOnPartyAction, "abolish disease", DISPEL_DISEASE);

// DEBUFF_CHECKISOWNER_ACTION(CastHolyFireAction, "holy fire");
class CastHolyFireAction : public CastDebuffSpellAction
{
public:
    CastHolyFireAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "holy fire", true, 0.0f) {}
};

// shadow 2.4.3
// BUFF_ACTION(CastShadowfiendAction, "shadowfiend");
SPELL_ACTION(CastShadowWordDeathAction, "shadow word: death");

// shadow
DEBUFF_CHECKISOWNER_ACTION(CastPowerWordPainAction, "shadow word: pain");
DEBUFF_ENEMY_ACTION(CastPowerWordPainOnAttackerAction, "shadow word: pain");
SPELL_ACTION(CastMindBlastAction, "mind blast");
SPELL_ACTION(CastPsychicScreamAction, "psychic scream");
DEBUFF_ACTION(CastMindSootheAction, "mind soothe");
BUFF_ACTION_U(CastFadeAction, "fade", bot->GetGroup());
BUFF_ACTION(CastShadowProtectionAction, "shadow protection");
BUFF_PARTY_ACTION(CastShadowProtectionOnPartyAction, "shadow protection");
BUFF_PARTY_ACTION(CastPrayerOfShadowProtectionAction, "prayer of shadow protection");

// shadow talents
SPELL_ACTION(CastMindFlayAction, "mind flay");
DEBUFF_ACTION(CastVampiricEmbraceAction, "vampiric embrace");
BUFF_ACTION(CastShadowformAction, "shadowform");
SPELL_ACTION(CastSilenceAction, "silence");
ENEMY_HEALER_ACTION(CastSilenceOnEnemyHealerAction, "silence");
// shadow talents 2.4.3
DEBUFF_CHECKISOWNER_ACTION(CastVampiricTouchAction, "vampiric touch");
DEBUFF_ENEMY_ACTION(CastVampiricTouchOnAttackerAction, "vampiric touch");
// racials
DEBUFF_CHECKISOWNER_ACTION(CastDevouringPlagueAction, "devouring plague");
BUFF_ACTION(CastTouchOfWeaknessAction, "touch of weakness");
DEBUFF_ACTION(CastHexOfWeaknessAction, "hex of weakness");
BUFF_ACTION(CastShadowguardAction, "shadowguard");
HEAL_ACTION(CastDesperatePrayerAction, "desperate prayer");
BUFF_ACTION(CastFearWardAction, "fear ward");
BUFF_PARTY_ACTION(CastFearWardOnPartyAction, "fear ward");
SPELL_ACTION_U(CastStarshardsAction, "starshards",
               (AI_VALUE2(uint8, "mana", "self target") > 50 && AI_VALUE(Unit*, "current target") &&
                AI_VALUE2(float, "distance", "current target") > 15.0f));
BUFF_ACTION(CastElunesGraceAction, "elune's grace");
BUFF_ACTION(CastFeedbackAction, "feedback");
BUFF_ACTION(CastSymbolOfHopeAction, "symbol of hope");
SPELL_ACTION(CastConsumeMagicAction, "consume magic");
SNARE_ACTION(CastChastiseAction, "chastise");

class CastRemoveShadowformAction : public Action
{
public:
    CastRemoveShadowformAction(PlayerbotAI* botAI) : Action(botAI, "remove shadowform") {}

    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class CastDispersionAction : public CastSpellAction
{
public:
    CastDispersionAction(PlayerbotAI* ai) : CastSpellAction(ai, "dispersion") {}
    virtual std::string const GetTargetName() { return "self target"; }
};

class CastPenanceOnPartyAction : public HealPartyMemberAction
{
public:
    CastPenanceOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "penance", 25.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastHymnOfHopeAction : public CastSpellAction
{
public:
    CastHymnOfHopeAction(PlayerbotAI* ai) : CastSpellAction(ai, "hymn of hope") {}
    virtual std::string const GetTargetName() { return "self target"; }
};

class CastDivineHymnAction : public CastSpellAction
{
public:
    CastDivineHymnAction(PlayerbotAI* ai) : CastSpellAction(ai, "divine hymn") {}
    virtual std::string const GetTargetName() { return "self target"; }
};

class CastShadowfiendAction : public CastSpellAction
{
public:
    CastShadowfiendAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowfiend") {}
    virtual std::string const GetTargetName() { return "current target"; }
};

class CastPowerWordShieldOnAlmostFullHealthBelowAction : public HealPartyMemberAction
{
public:
    CastPowerWordShieldOnAlmostFullHealthBelowAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "power word: shield", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
    bool isUseful() override;
    Unit* GetTarget() override;
};

class CastPowerWordShieldOnNotFullAction : public HealPartyMemberAction
{
public:
    CastPowerWordShieldOnNotFullAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "power word: shield", 5.0f, HealingManaEfficiency::HIGH)
    {
    }
    bool isUseful() override;
    Unit* GetTarget() override;
};

class CastMindSearAction : public CastSpellAction
{
public:
    CastMindSearAction(PlayerbotAI* ai) : CastSpellAction(ai, "mind sear") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastGuardianSpiritOnPartyAction : public HealPartyMemberAction
{
public:
    CastGuardianSpiritOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "guardian spirit", 40.0f, HealingManaEfficiency::MEDIUM) {}
};

#endif
