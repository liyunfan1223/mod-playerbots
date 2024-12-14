/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DKACTIONS_H
#define _PLAYERBOT_DKACTIONS_H

#include "Event.h"
#include "GenericSpellActions.h"

class PlayerbotAI;

class CastBloodPresenceAction : public CastBuffSpellAction
{
public:
    CastBloodPresenceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blood presence") {}
};

class CastFrostPresenceAction : public CastBuffSpellAction
{
public:
    CastFrostPresenceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frost presence") {}
};

class CastUnholyPresenceAction : public CastBuffSpellAction
{
public:
    CastUnholyPresenceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "unholy presence") {}
};

class CastDeathchillAction : public CastBuffSpellAction
{
public:
    CastDeathchillAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "deathchill") {}

    NextAction** getPrerequisites() override;
};

class CastDarkCommandAction : public CastSpellAction
{
public:
    CastDarkCommandAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dark command") {}
};

BEGIN_RANGED_SPELL_ACTION(CastDeathGripAction, "death grip")
END_SPELL_ACTION()

// Unholy presence
class CastUnholyMeleeSpellAction : public CastMeleeSpellAction
{
public:
    CastUnholyMeleeSpellAction(PlayerbotAI* botAI, std::string const spell) : CastMeleeSpellAction(botAI, spell) {}

    NextAction** getPrerequisites() override;
};

// Frost presence
class CastFrostMeleeSpellAction : public CastMeleeSpellAction
{
public:
    CastFrostMeleeSpellAction(PlayerbotAI* botAI, std::string const spell) : CastMeleeSpellAction(botAI, spell) {}

    NextAction** getPrerequisites() override;
};

// Blood presence
class CastBloodMeleeSpellAction : public CastMeleeSpellAction
{
public:
    CastBloodMeleeSpellAction(PlayerbotAI* botAI, std::string const spell) : CastMeleeSpellAction(botAI, spell) {}

    NextAction** getPrerequisites() override;
};

class CastRuneStrikeAction : public CastMeleeSpellAction
{
public:
    CastRuneStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "rune strike") {}
};

// debuff
//  BEGIN_DEBUFF_ACTION(CastPestilenceAction, "pestilence")
//  END_SPELL_ACTION()

class CastPestilenceAction : public CastSpellAction
{
public:
    CastPestilenceAction(PlayerbotAI* ai) : CastSpellAction(ai, "pestilence") {}
    ActionThreatType getThreatType() override { return ActionThreatType::None; }
};

// debuff
//  BEGIN_DEBUFF_ACTION(CastHowlingBlastAction, "howling blast")
//  END_SPELL_ACTION()

class CastHowlingBlastAction : public CastSpellAction
{
public:
    CastHowlingBlastAction(PlayerbotAI* ai) : CastSpellAction(ai, "howling blast") {}
};

// debuff it
//  BEGIN_DEBUFF_ACTION(CastIcyTouchAction, "icy touch")
//  END_SPELL_ACTION()

class CastIcyTouchAction : public CastSpellAction
{
public:
    CastIcyTouchAction(PlayerbotAI* ai) : CastSpellAction(ai, "icy touch") {}
};

class CastIcyTouchOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastIcyTouchOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnAttackerAction(botAI, "icy touch", true, .0f)
    {
    }
};

// debuff ps

class CastPlagueStrikeAction : public CastSpellAction
{
public:
    CastPlagueStrikeAction(PlayerbotAI* ai) : CastSpellAction(ai, "plague strike") {}
};
// BEGIN_DEBUFF_ACTION(CastPlagueStrikeAction, "plague strike")
// END_SPELL_ACTION()

class CastPlagueStrikeOnAttackerAction : public CastDebuffSpellOnMeleeAttackerAction
{
public:
    CastPlagueStrikeOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnMeleeAttackerAction(botAI, "plague strike", true, .0f)
    {
    }
};

// debuff
BEGIN_DEBUFF_ACTION(CastMarkOfBloodAction, "mark of blood")
END_SPELL_ACTION()

class CastMarkOfBloodOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastMarkOfBloodOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "mark of blood", true)
    {
    }
};

class CastUnholyBlightAction : public CastBuffSpellAction
{
public:
    CastUnholyBlightAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "unholy blight") {}
};

class CastSummonGargoyleAction : public CastSpellAction
{
public:
    CastSummonGargoyleAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "summon gargoyle") {}
};

class CastGhoulFrenzyAction : public CastBuffSpellAction
{
public:
    CastGhoulFrenzyAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ghoul frenzy", false, 5000) {}
    std::string const GetTargetName() override { return "pet target"; }
};

BEGIN_MELEE_SPELL_ACTION(CastCorpseExplosionAction, "corpse explosion")
END_SPELL_ACTION()

BEGIN_MELEE_SPELL_ACTION(CastAntiMagicShellAction, "anti magic shell")
END_SPELL_ACTION()

BEGIN_MELEE_SPELL_ACTION(CastAntiMagicZoneAction, "anti magic zone")
END_SPELL_ACTION()

class CastChainsOfIceAction : public CastSpellAction
{
public:
    CastChainsOfIceAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chains of ice") {}
};

class CastHungeringColdAction : public CastMeleeSpellAction
{
public:
    CastHungeringColdAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hungering cold") {}
};

class CastHeartStrikeAction : public CastMeleeSpellAction
{
public:
    CastHeartStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "heart strike") {}
};

class CastBloodStrikeAction : public CastMeleeSpellAction
{
public:
    CastBloodStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "blood strike") {}
};

class CastFrostStrikeAction : public CastMeleeSpellAction
{
public:
    CastFrostStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "frost strike") {}
};

class CastObliterateAction : public CastMeleeSpellAction
{
public:
    CastObliterateAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "obliterate") {}
};

class CastDeathStrikeAction : public CastMeleeSpellAction
{
public:
    CastDeathStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "death strike") {}
};

class CastScourgeStrikeAction : public CastMeleeSpellAction
{
public:
    CastScourgeStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "scourge strike") {}
};

class CastDeathCoilAction : public CastSpellAction
{
public:
    CastDeathCoilAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "death coil") {}
};

class CastBloodBoilAction : public CastMeleeSpellAction
{
public:
    CastBloodBoilAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "blood boil") {}
};

class CastDeathAndDecayAction : public CastSpellAction
{
public:
    CastDeathAndDecayAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "death and decay") {}
    // ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastHornOfWinterAction : public CastSpellAction
{
public:
    CastHornOfWinterAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "horn of winter") {}
};

class CastImprovedIcyTalonsAction : public CastBuffSpellAction
{
public:
    CastImprovedIcyTalonsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "improved icy talons") {}
};

class CastBoneShieldAction : public CastBuffSpellAction
{
public:
    CastBoneShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bone shield") {}
};

class CastDeathPactAction : public CastBuffSpellAction
{
public:
    CastDeathPactAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "death pact") {}
};

class CastDeathRuneMasteryAction : public CastBuffSpellAction
{
public:
    CastDeathRuneMasteryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "death rune mastery") {}
};

class CastDancingRuneWeaponAction : public CastSpellAction
{
public:
    CastDancingRuneWeaponAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dancing rune weapon") {}
};

class CastEmpowerRuneWeaponAction : public CastBuffSpellAction
{
public:
    CastEmpowerRuneWeaponAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "empower rune weapon") {}
};

class CastArmyOfTheDeadAction : public CastBuffSpellAction
{
public:
    CastArmyOfTheDeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "army of the dead") {}
};

class CastRaiseDeadAction : public CastBuffSpellAction
{
public:
    CastRaiseDeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "raise dead") {}
    virtual bool Execute(Event event) override;
};

class CastKillingMachineAction : public CastBuffSpellAction
{
public:
    CastKillingMachineAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "killing machine") {}
};

class CastIceboundFortitudeAction : public CastBuffSpellAction
{
public:
    CastIceboundFortitudeAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "icebound fortitude") {}
};

class CastUnbreakableArmorAction : public CastBuffSpellAction
{
public:
    CastUnbreakableArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "unbreakable armor") {}
};

class CastVampiricBloodAction : public CastBuffSpellAction
{
public:
    CastVampiricBloodAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "vampiric blood") {}
};

class CastMindFreezeAction : public CastMeleeSpellAction
{
public:
    CastMindFreezeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mind freeze") {}
};

class CastStrangulateAction : public CastMeleeSpellAction
{
public:
    CastStrangulateAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "strangulate") {}
};

class CastMindFreezeOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastMindFreezeOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "mind freeze") {}
};

class CastRuneTapAction : public CastMeleeSpellAction
{
public:
    CastRuneTapAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "rune tap") {}
};

class CastBloodTapAction : public CastMeleeSpellAction
{
public:
    CastBloodTapAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "blood tap") {}
};

#endif
