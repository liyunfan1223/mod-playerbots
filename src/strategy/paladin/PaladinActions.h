/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINACTIONS_H
#define _PLAYERBOT_PALADINACTIONS_H

#include "GenericSpellActions.h"
#include "SharedDefines.h"

class PlayerbotAI;
class Unit;

class CastJudgementAction : public CastMeleeSpellAction
{
    public:
        CastJudgementAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "judgement") { }
};

class CastJudgementOfLightAction : public CastMeleeSpellAction
{
    public:
        CastJudgementOfLightAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "judgement of light") { }
};

class CastJudgementOfWisdomAction : public CastMeleeSpellAction
{
    public:
        CastJudgementOfWisdomAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "judgement of wisdom") { }
};

class CastJudgementOfJusticeAction : public CastMeleeSpellAction
{
    public:
        CastJudgementOfJusticeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "judgement of justice") { }
};

class CastRighteousFuryAction : public CastBuffSpellAction
{
	public:
		CastRighteousFuryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "righteous fury") { }
};

class CastDevotionAuraAction : public CastBuffSpellAction
{
	public:
		CastDevotionAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "devotion aura") { }
};

class CastRetributionAuraAction : public CastBuffSpellAction
{
	public:
		CastRetributionAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "retribution aura") { }
};

class CastConcentrationAuraAction : public CastBuffSpellAction
{
	public:
		CastConcentrationAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "concentration aura") { }
};

class CastDivineStormAction : public CastBuffSpellAction
{
	public:
		CastDivineStormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine storm") { }
};

class CastCrusaderStrikeAction : public CastMeleeSpellAction
{
	public:
		CastCrusaderStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "crusader strike") { }
};

class CastShadowResistanceAuraAction : public CastBuffSpellAction
{
	public:
		CastShadowResistanceAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shadow resistance aura") { }
};

class CastFrostResistanceAuraAction : public CastBuffSpellAction
{
	public:
		CastFrostResistanceAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frost resistance aura") { }
};

class CastFireResistanceAuraAction : public CastBuffSpellAction
{
	public:
		CastFireResistanceAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fire resistance aura") { }
};

class CastCrusaderAuraAction : public CastBuffSpellAction
{
	public:
		CastCrusaderAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "crusader aura") { }
};

class CastSealSpellAction : public CastBuffSpellAction
{
    public:
        CastSealSpellAction(PlayerbotAI* botAI, std::string const name) : CastBuffSpellAction(botAI, name) { }

        bool isUseful() override;
};

class CastSealOfRighteousnessAction : public CastSealSpellAction
{
	public:
		CastSealOfRighteousnessAction(PlayerbotAI* botAI) : CastSealSpellAction(botAI, "seal of righteousness") { }
};

class CastSealOfJusticeAction : public CastSealSpellAction
{
	public:
		CastSealOfJusticeAction(PlayerbotAI* botAI) : CastSealSpellAction(botAI, "seal of justice") { }
};

class CastSealOfLightAction : public CastSealSpellAction
{
	public:
		CastSealOfLightAction(PlayerbotAI* botAI) : CastSealSpellAction(botAI, "seal of light") { }
};

class CastSealOfWisdomAction : public CastSealSpellAction
{
	public:
		CastSealOfWisdomAction(PlayerbotAI* botAI) : CastSealSpellAction(botAI, "seal of wisdom") { }
};

class CastSealOfCommandAction : public CastSealSpellAction
{
	public:
		CastSealOfCommandAction(PlayerbotAI* botAI) : CastSealSpellAction(botAI, "seal of command") { }
};

class CastSealOfVengeanceAction : public CastSealSpellAction
{
	public:
	    CastSealOfVengeanceAction(PlayerbotAI* botAI) : CastSealSpellAction(botAI, "seal of vengeance") { }
};

class CastBlessingOfMightAction : public CastBuffSpellAction
{
	public:
		CastBlessingOfMightAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of might") { }

		bool Execute(Event event) override;
};

class CastBlessingOnPartyAction : public BuffOnPartyAction
{
    public:
	    CastBlessingOnPartyAction(PlayerbotAI* botAI, std::string const name) : BuffOnPartyAction(botAI, name) { }

        Value<Unit*>* GetTargetValue() override;
};

class CastBlessingOfMightOnPartyAction : public CastBlessingOnPartyAction
{
	public:
		CastBlessingOfMightOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of might") { }

        std::string const getName() override { return "blessing of might on party";}
        bool Execute(Event event) override;
};

class CastBlessingOfWisdomAction : public CastBuffSpellAction
{
	public:
		CastBlessingOfWisdomAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of wisdom") { }

		bool Execute(Event event) override;
};

class CastBlessingOfWisdomOnPartyAction : public CastBlessingOnPartyAction
{
	public:
		CastBlessingOfWisdomOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of wisdom") { }

        std::string const getName() override { return "blessing of wisdom on party";}
        bool Execute(Event event) override;
};

class CastBlessingOfKingsAction : public CastBuffSpellAction
{
	public:
		CastBlessingOfKingsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of kings") { }
};

class CastBlessingOfKingsOnPartyAction : public CastBlessingOnPartyAction
{
	public:
		CastBlessingOfKingsOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of kings") { }

        std::string const getName() override { return "blessing of kings on party";}
};

class CastBlessingOfSanctuaryAction : public CastBuffSpellAction
{
	public:
		CastBlessingOfSanctuaryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of sanctuary") { }
};

class CastBlessingOfSanctuaryOnPartyAction : public CastBlessingOnPartyAction
{
	public:
		CastBlessingOfSanctuaryOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of sanctuary") { }

        std::string const getName() override { return "blessing of sanctuary on party";}
};

class CastHolyLightAction : public CastHealingSpellAction
{
    public:
        CastHolyLightAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "holy light") { }
};

class CastHolyLightOnPartyAction : public HealPartyMemberAction
{
    public:
        CastHolyLightOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "holy light") { }

        std::string const getName() override { return "holy light on party"; }
};

class CastFlashOfLightAction : public CastHealingSpellAction
{
    public:
        CastFlashOfLightAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "flash of light") { }
};

class CastFlashOfLightOnPartyAction : public HealPartyMemberAction
{
    public:
        CastFlashOfLightOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "flash of light") { }

        std::string const getName() override { return "flash of light on party"; }
};

class CastLayOnHandsAction : public CastHealingSpellAction
{
    public:
        CastLayOnHandsAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lay on hands") { }
};

class CastLayOnHandsOnPartyAction : public HealPartyMemberAction
{
    public:
        CastLayOnHandsOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "lay on hands") { }

        std::string const getName() override { return "lay on hands on party"; }
};

class CastDivineProtectionAction : public CastBuffSpellAction
{
	public:
		CastDivineProtectionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine protection") { }
};

class CastDivineProtectionOnPartyAction : public HealPartyMemberAction
{
    public:
        CastDivineProtectionOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "divine protection") { }

        std::string const getName() override { return "divine protection on party"; }
};

class CastDivineShieldAction: public CastBuffSpellAction
{
	public:
		CastDivineShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine shield") { }
};

class CastConsecrationAction : public CastMeleeSpellAction
{
    public:
	    CastConsecrationAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "consecration") { }
};

class CastHolyWrathAction : public CastMeleeSpellAction
{
    public:
        CastHolyWrathAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "holy wrath") { }
};

class CastHammerOfJusticeAction : public CastMeleeSpellAction
{
    public:
        CastHammerOfJusticeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of justice") { }
};

class CastHammerOfWrathAction : public CastMeleeSpellAction
{
	public:
		CastHammerOfWrathAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of wrath") { }
};

class CastHammerOfTheRighteousAction : public CastMeleeSpellAction
{
	public:
		CastHammerOfTheRighteousAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of the righteous") { }
};

class CastPurifyPoisonAction : public CastCureSpellAction
{
	public:
		CastPurifyPoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify") { }
};

class CastPurifyDiseaseAction : public CastCureSpellAction
{
	public:
		CastPurifyDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify") { }
};

class CastPurifyPoisonOnPartyAction : public CurePartyMemberAction
{
    public:
        CastPurifyPoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify", DISPEL_POISON) { }

        std::string const getName() override { return "purify poison on party"; }
};

class CastPurifyDiseaseOnPartyAction : public CurePartyMemberAction
{
	public:
		CastPurifyDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify", DISPEL_DISEASE) { }

		std::string const getName() override { return "purify disease on party"; }
};

class CastHandOfReckoningAction : public CastSpellAction
{
	public:
		CastHandOfReckoningAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hand of reckoning") { }
};

class CastRighteousDefenseAction : public CastSpellAction
{
    public:
        CastRighteousDefenseAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "righteous defense") { }
};

class CastCleansePoisonAction : public CastCureSpellAction
{
	public:
		CastCleansePoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") { }
};

class CastCleanseDiseaseAction : public CastCureSpellAction
{
	public:
		CastCleanseDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") { }
};

class CastCleanseMagicAction : public CastCureSpellAction
{
	public:
		CastCleanseMagicAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") { }
};

class CastCleansePoisonOnPartyAction : public CurePartyMemberAction
{
    public:
        CastCleansePoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_POISON) { }

        std::string const getName() override { return "cleanse poison on party"; }
};

class CastCleanseDiseaseOnPartyAction : public CurePartyMemberAction
{
	public:
		CastCleanseDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_DISEASE) { }

		std::string const getName() override { return "cleanse disease on party"; }
};

class CastCleanseMagicOnPartyAction : public CurePartyMemberAction
{
	public:
		CastCleanseMagicOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_MAGIC) { }

		std::string const getName() override { return "cleanse magic on party"; }
};

BEGIN_SPELL_ACTION(CastAvengersShieldAction, "avenger's shield")
END_SPELL_ACTION()

BEGIN_SPELL_ACTION(CastExorcismAction, "exorcism")
END_SPELL_ACTION()

class CastHolyShieldAction : public CastBuffSpellAction
{
	public:
		CastHolyShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "holy shield") { }
};

class CastRedemptionAction : public ResurrectPartyMemberAction
{
	public:
		CastRedemptionAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "redemption") { }
};

class CastHammerOfJusticeOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
    public:
        CastHammerOfJusticeOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "hammer of justice") { }
};

class CastHammerOfJusticeSnareAction : public CastSnareSpellAction
{
    public:
        CastHammerOfJusticeSnareAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "hammer of justice") { }
};

class CastDivineFavorAction : public CastBuffSpellAction
{
    public:
        CastDivineFavorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine favor") { }
};

class CastTurnUndeadAction : public CastBuffSpellAction
{
    public:
        CastTurnUndeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "turn undead") { }

        Value<Unit*>* GetTargetValue() override;
};

PROTECT_ACTION(CastBlessingOfProtectionProtectAction, "blessing of protection");

#endif
