/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINACTIONS_H
#define _PLAYERBOT_PALADINACTIONS_H

#include "AiObject.h"
#include "GenericSpellActions.h"
#include "SharedDefines.h"

class PlayerbotAI;
class Unit;

// seals
BUFF_ACTION(CastSealOfRighteousnessAction, "seal of righteousness");
BUFF_ACTION(CastSealOfJusticeAction, "seal of justice");
BUFF_ACTION(CastSealOfLightAction, "seal of light");
BUFF_ACTION(CastSealOfWisdomAction, "seal of wisdom");
BUFF_ACTION(CastSealOfCommandAction, "seal of command");
BUFF_ACTION(CastSealOfVengeanceAction, "seal of vengeance");
BUFF_ACTION(CastSealOfCorruptionAction, "seal of corruption");

// judgements
SPELL_ACTION(CastJudgementAction, "judgement");

SPELL_ACTION(CastJudgementOfLightAction, "judgement of light");
SPELL_ACTION(CastJudgementOfWisdomAction, "judgement of wisdom");
SPELL_ACTION(CastJudgementOfJusticeAction, "judgement of justice");

// auras
BUFF_ACTION(CastDevotionAuraAction, "devotion aura");
BUFF_ACTION(CastRetributionAuraAction, "retribution aura");
BUFF_ACTION(CastConcentrationAuraAction, "concentration aura");
BUFF_ACTION(CastShadowResistanceAuraAction, "shadow resistance aura");
BUFF_ACTION(CastFrostResistanceAuraAction, "frost resistance aura");
BUFF_ACTION(CastFireResistanceAuraAction, "fire resistance aura");
BUFF_ACTION(CastCrusaderAuraAction, "crusader aura");
BUFF_ACTION(CastSanctityAuraAction, "sanctity aura");

SPELL_ACTION(CastHolyShockAction, "holy shock");

// consecration
MELEE_ACTION(CastConsecrationAction, "consecration");

// repentance
SNARE_ACTION(CastRepentanceSnareAction, "repentance");
DEBUFF_ACTION(CastRepentanceAction, "repentance");
ENEMY_HEALER_ACTION(CastRepentanceOnHealerAction, "repentance");

// hamme of wrath
SPELL_ACTION(CastHammerOfWrathAction, "hammer of wrath");

// buffs
BUFF_ACTION(CastDivineFavorAction, "divine favor");

// blessings

// fury
BUFF_ACTION(CastRighteousFuryAction, "righteous fury");

class CastDivineStormAction : public CastMeleeSpellAction
{
public:
    CastDivineStormAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "divine storm") {}
};

class CastCrusaderStrikeAction : public CastMeleeSpellAction
{
public:
    CastCrusaderStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "crusader strike") {}
};

class CastSealSpellAction : public CastBuffSpellAction
{
public:
    CastSealSpellAction(PlayerbotAI* botAI, std::string const name) : CastBuffSpellAction(botAI, name) {}

    bool isUseful() override;
};

class CastBlessingOfMightAction : public CastBuffSpellAction
{
public:
    CastBlessingOfMightAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of might") {}

    bool Execute(Event event) override;
};

class CastBlessingOnPartyAction : public BuffOnPartyAction
{
public:
    CastBlessingOnPartyAction(PlayerbotAI* botAI, std::string const name) : BuffOnPartyAction(botAI, name), name(name)
    {
    }

    Value<Unit*>* GetTargetValue() override;

private:
    std::string name;
};

class CastBlessingOfMightOnPartyAction : public BuffOnPartyAction
{
public:
    CastBlessingOfMightOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "blessing of might") {}

    std::string const getName() override { return "blessing of might on party"; }
    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};

class CastBlessingOfWisdomAction : public CastBuffSpellAction
{
public:
    CastBlessingOfWisdomAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of wisdom") {}

    bool Execute(Event event) override;
};

class CastBlessingOfWisdomOnPartyAction : public BuffOnPartyAction
{
public:
    CastBlessingOfWisdomOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "blessing of wisdom") {}

    std::string const getName() override { return "blessing of wisdom on party"; }
    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};

class CastBlessingOfKingsAction : public CastBuffSpellAction
{
public:
    CastBlessingOfKingsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of kings") {}
};

class CastBlessingOfKingsOnPartyAction : public CastBlessingOnPartyAction
{
public:
    CastBlessingOfKingsOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of kings") {}

    std::string const getName() override { return "blessing of kings on party"; }
};

class CastBlessingOfSanctuaryAction : public CastBuffSpellAction
{
public:
    CastBlessingOfSanctuaryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of sanctuary") {}
};

class CastBlessingOfSanctuaryOnPartyAction : public CastBlessingOnPartyAction
{
public:
    CastBlessingOfSanctuaryOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of sanctuary")
    {
    }

    std::string const getName() override { return "blessing of sanctuary on party"; }
};

class CastHolyLightAction : public CastHealingSpellAction
{
public:
    CastHolyLightAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "holy light") {}
};

class CastHolyShockOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyShockOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "holy shock", 25.0f, HealingManaEfficiency::LOW)
    {
    }
};

class CastHolyLightOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyLightOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "holy light", 50.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastFlashOfLightAction : public CastHealingSpellAction
{
public:
    CastFlashOfLightAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "flash of light") {}
};

class CastFlashOfLightOnPartyAction : public HealPartyMemberAction
{
public:
    CastFlashOfLightOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "flash of light", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastLayOnHandsAction : public CastHealingSpellAction
{
public:
    CastLayOnHandsAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lay on hands") {}
};

class CastLayOnHandsOnPartyAction : public HealPartyMemberAction
{
public:
    CastLayOnHandsOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "lay on hands") {}
};

class CastDivineProtectionAction : public CastBuffSpellAction
{
public:
    CastDivineProtectionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine protection") {}
};

class CastDivineProtectionOnPartyAction : public HealPartyMemberAction
{
public:
    CastDivineProtectionOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "divine protection") {}

    std::string const getName() override { return "divine protection on party"; }
};

class CastDivineShieldAction : public CastBuffSpellAction
{
public:
    CastDivineShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine shield") {}
};

class CastHolyWrathAction : public CastMeleeSpellAction
{
public:
    CastHolyWrathAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "holy wrath") {}
};

class CastHammerOfJusticeAction : public CastMeleeSpellAction
{
public:
    CastHammerOfJusticeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of justice") {}
};

class CastHammerOfTheRighteousAction : public CastMeleeSpellAction
{
public:
    CastHammerOfTheRighteousAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of the righteous") {}
};

class CastPurifyPoisonAction : public CastCureSpellAction
{
public:
    CastPurifyPoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify") {}
};

class CastPurifyDiseaseAction : public CastCureSpellAction
{
public:
    CastPurifyDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify") {}
};

class CastPurifyPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastPurifyPoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify", DISPEL_POISON) {}

    std::string const getName() override { return "purify poison on party"; }
};

class CastPurifyDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastPurifyDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify", DISPEL_DISEASE) {}

    std::string const getName() override { return "purify disease on party"; }
};

class CastHandOfReckoningAction : public CastSpellAction
{
public:
    CastHandOfReckoningAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hand of reckoning") {}
};

class CastRighteousDefenseAction : public CastSpellAction
{
public:
    CastRighteousDefenseAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "righteous defense") {}
    virtual Unit* GetTarget() override;
};

class CastCleansePoisonAction : public CastCureSpellAction
{
public:
    CastCleansePoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") {}
};

class CastCleanseDiseaseAction : public CastCureSpellAction
{
public:
    CastCleanseDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") {}
};

class CastCleanseMagicAction : public CastCureSpellAction
{
public:
    CastCleanseMagicAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") {}
};

class CastCleansePoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleansePoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_POISON) {}

    std::string const getName() override { return "cleanse poison on party"; }
};

class CastCleanseDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_DISEASE) {}

    std::string const getName() override { return "cleanse disease on party"; }
};

class CastCleanseMagicOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseMagicOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_MAGIC) {}

    std::string const getName() override { return "cleanse magic on party"; }
};

BEGIN_SPELL_ACTION(CastAvengersShieldAction, "avenger's shield")
END_SPELL_ACTION()

BEGIN_SPELL_ACTION(CastExorcismAction, "exorcism")
END_SPELL_ACTION()

class CastHolyShieldAction : public CastBuffSpellAction
{
public:
    CastHolyShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "holy shield") {}
};

class CastRedemptionAction : public ResurrectPartyMemberAction
{
public:
    CastRedemptionAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "redemption") {}
};

class CastHammerOfJusticeOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastHammerOfJusticeOnEnemyHealerAction(PlayerbotAI* botAI)
        : CastSpellOnEnemyHealerAction(botAI, "hammer of justice")
    {
    }
};

class CastHammerOfJusticeSnareAction : public CastSnareSpellAction
{
public:
    CastHammerOfJusticeSnareAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "hammer of justice") {}
};

class CastTurnUndeadAction : public CastBuffSpellAction
{
public:
    CastTurnUndeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "turn undead") {}

    Value<Unit*>* GetTargetValue() override;
};

PROTECT_ACTION(CastBlessingOfProtectionProtectAction, "blessing of protection");

class CastDivinePleaAction : public CastBuffSpellAction
{
public:
    CastDivinePleaAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "divine plea") {}
};

class ShieldOfRighteousnessAction : public CastMeleeSpellAction
{
public:
    ShieldOfRighteousnessAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "shield of righteousness") {}
};

class CastBeaconOfLightOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastBeaconOfLightOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "beacon of light", true) {}
};

class CastSacredShieldOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastSacredShieldOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "sacred shield", false) {}
};

class CastAvengingWrathAction : public CastBuffSpellAction
{
public:
    CastAvengingWrathAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "avenging wrath") {}
};

class CastDivineIlluminationAction : public CastBuffSpellAction
{
public:
    CastDivineIlluminationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine illumination") {}
};

class CastDivineSacrificeAction : public CastBuffSpellAction
{
public:
    CastDivineSacrificeAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine sacrifice") {}
    bool isUseful() override;
};

class CastCancelDivineSacrificeAction : public Action
{
public:
    CastCancelDivineSacrificeAction(PlayerbotAI* botAI) : Action(botAI, "cancel divine sacrifice") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};
#endif
