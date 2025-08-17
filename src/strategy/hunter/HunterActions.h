/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HUNTERACTIONS_H
#define _PLAYERBOT_HUNTERACTIONS_H

#include "AiObject.h"
#include "Event.h"
#include "GenericSpellActions.h"
#include "Unit.h"

class PlayerbotAI;
class Unit;

// Buff and Out of Combat Spells

class CastTrueshotAuraAction : public CastBuffSpellAction
{
public:
    CastTrueshotAuraAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "trueshot aura") {}
};

class CastAspectOfTheHawkAction : public CastBuffSpellAction
{
public:
    CastAspectOfTheHawkAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the hawk") {}
    bool isUseful() override;
};

class CastAspectOfTheMonkeyAction : public CastBuffSpellAction
{
public:
    CastAspectOfTheMonkeyAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the monkey") {}
};

class CastAspectOfTheDragonhawkAction : public CastBuffSpellAction
{
public:
    CastAspectOfTheDragonhawkAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the dragonhawk") {}
};

class CastAspectOfTheWildAction : public CastBuffSpellAction
{
public:
    CastAspectOfTheWildAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the wild") {}
};

class CastAspectOfTheCheetahAction : public CastBuffSpellAction
{
public:
    CastAspectOfTheCheetahAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the cheetah") {}

    bool isUseful() override;
};

class CastAspectOfThePackAction : public CastBuffSpellAction
{
public:
    CastAspectOfThePackAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the pack") {}
};

class CastAspectOfTheViperAction : public CastBuffSpellAction
{
public:
    CastAspectOfTheViperAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aspect of the viper") {}
};

// Cooldown Spells

class CastRapidFireAction : public CastBuffSpellAction
{
public:
    CastRapidFireAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "rapid fire") {}
};

class CastDeterrenceAction : public CastBuffSpellAction
{
public:
    CastDeterrenceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "deterrence") {}
};

class CastReadinessAction : public CastBuffSpellAction
{
public:
    CastReadinessAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "readiness") {}
};

class CastDisengageAction : public CastSpellAction
{
public:
    CastDisengageAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "disengage") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// CC Spells

class CastScareBeastAction : public CastSpellAction
{
public:
    CastScareBeastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "scare beast") {}
};

class CastScareBeastCcAction : public CastSpellAction
{
public:
    CastScareBeastCcAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "scare beast on cc") {}

    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};

class CastFreezingTrap : public CastDebuffSpellAction
{
public:
    CastFreezingTrap(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "freezing trap") {}

    Value<Unit*>* GetTargetValue() override;
};

class CastWyvernStingAction : public CastDebuffSpellAction
{
public:
    CastWyvernStingAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "wyvern sting", true) {}
};

class CastSilencingShotAction : public CastSpellAction
{
public:
    CastSilencingShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "silencing shot") {}
};

class CastConcussiveShotAction : public CastSnareSpellAction
{
public:
    CastConcussiveShotAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "concussive shot") {}
};

class CastIntimidationAction : public CastBuffSpellAction
{
public:
    CastIntimidationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "intimidation", false, 5000) {}
    std::string const GetTargetName() override { return "pet target"; }
};

// Threat Spells

class CastDistractingShotAction : public CastSpellAction
{
public:
    CastDistractingShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "distracting shot") {}
};

class CastMisdirectionOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastMisdirectionOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "misdirection", true) {}
};

class CastFeignDeathAction : public CastBuffSpellAction
{
public:
    CastFeignDeathAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "feign death") {}
};

// Pet Spells

class FeedPetAction : public Action
{
public:
    FeedPetAction(PlayerbotAI* botAI) : Action(botAI, "feed pet") {}

    bool Execute(Event event) override;
};

class CastCallPetAction : public CastBuffSpellAction
{
public:
    CastCallPetAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "call pet") {}
};

class CastMendPetAction : public CastAuraSpellAction
{
public:
    CastMendPetAction(PlayerbotAI* botAI) : CastAuraSpellAction(botAI, "mend pet") {}
    std::string const GetTargetName() override { return "pet target"; }
};

class CastRevivePetAction : public CastBuffSpellAction
{
public:
    CastRevivePetAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "revive pet") {}
};

class CastKillCommandAction : public CastBuffSpellAction
{
public:
    CastKillCommandAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "kill command", false, 5000) {}
    std::string const GetTargetName() override { return "pet target"; }
};

class CastBestialWrathAction : public CastBuffSpellAction
{
public:
    CastBestialWrathAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bestial wrath", false, 5000) {}
    std::string const GetTargetName() override { return "pet target"; }
};

// Direct Damage Spells

class CastAutoShotAction : public CastSpellAction
{
public:
    CastAutoShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "auto shot") {}
    ActionThreatType getThreatType() override { return ActionThreatType::None; }
    bool isUseful() override;
};

class CastArcaneShotAction : public CastSpellAction
{
public:
    CastArcaneShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "arcane shot") {}
    bool isUseful() override;
};

class CastAimedShotAction : public CastSpellAction
{
public:
    CastAimedShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "aimed shot") {}
};

class CastChimeraShotAction : public CastSpellAction
{
public:
    CastChimeraShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chimera shot") {}
};

class CastSteadyShotAction : public CastSpellAction
{
public:
    CastSteadyShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "steady shot") {}
};

class CastKillShotAction : public CastSpellAction
{
public:
    CastKillShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "kill shot") {}
};

// DoT/Debuff Spells

class CastHuntersMarkAction : public CastDebuffSpellAction
{
public:
    CastHuntersMarkAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "hunter's mark") {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastTranquilizingShotAction : public CastSpellAction
{
public:
    CastTranquilizingShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "tranquilizing shot") {}
};

class CastViperStingAction : public CastDebuffSpellAction
{
public:
    CastViperStingAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "viper sting", true) {}
    bool isUseful() override;
};

class CastSerpentStingAction : public CastDebuffSpellAction
{
public:
    CastSerpentStingAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "serpent sting", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastScorpidStingAction : public CastDebuffSpellAction
{
public:
    CastScorpidStingAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "scorpid sting", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastSerpentStingOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastSerpentStingOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "serpent sting", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastImmolationTrapAction : public CastSpellAction
{
public:
    CastImmolationTrapAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "immolation trap") {}
    bool isUseful() override;
};

class CastExplosiveTrapAction : public CastSpellAction
{
public:
    CastExplosiveTrapAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "explosive trap") {}
};

class CastBlackArrow : public CastDebuffSpellAction
{
public:
    CastBlackArrow(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "black arrow", true) {}
    bool isUseful() override
    {
        if (botAI->HasStrategy("trap weave", BOT_STATE_COMBAT))
            return false;
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastExplosiveShotAction : public CastDebuffSpellAction
{
public:
    CastExplosiveShotAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "explosive shot", true, 0.0f) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

// Rank 4
class CastExplosiveShotRank4Action : public CastDebuffSpellAction
{
public:
    CastExplosiveShotRank4Action(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "explosive shot", true, 0.0f) {}

    bool Execute(Event event) override { return botAI->CastSpell(60053, GetTarget()); }
    bool isUseful() override
    {
        Unit* target = GetTarget();
        if (!target)
            return false;
        return !target->HasAura(60053);
    }
};

// Rank 3
class CastExplosiveShotRank3Action : public CastDebuffSpellAction
{
public:
    CastExplosiveShotRank3Action(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "explosive shot", true, 0.0f) {}

    bool Execute(Event event) override { return botAI->CastSpell(60052, GetTarget()); }
    bool isUseful() override
    {
        Unit* target = GetTarget();
        if (!target)
            return false;
        return !target->HasAura(60052);
    }
};

// Rank 2
class CastExplosiveShotRank2Action : public CastDebuffSpellAction
{
public:
    CastExplosiveShotRank2Action(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "explosive shot", true, 0.0f) {}

    bool Execute(Event event) override { return botAI->CastSpell(60051, GetTarget()); }
    bool isUseful() override
    {
        Unit* target = GetTarget();
        if (!target)
            return false;
        return !target->HasAura(60051);
    }
};

// Rank 1
class CastExplosiveShotRank1Action : public CastDebuffSpellAction
{
public:
    CastExplosiveShotRank1Action(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "explosive shot", true, 0.0f) {}

    bool Execute(Event event) override { return botAI->CastSpell(53301, GetTarget()); }
    bool isUseful() override
    {
        Unit* target = GetTarget();
        if (!target)
            return false;
        return !target->HasAura(53301);
    }
};

// Melee Spells

class CastWingClipAction : public CastSpellAction
{
public:
    CastWingClipAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "wing clip") {}

    bool isUseful() override;
    NextAction** getPrerequisites() override;
};

class CastRaptorStrikeAction : public CastSpellAction
{
public:
    CastRaptorStrikeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "raptor strike") {}
};

class CastMongooseBiteAction : public CastSpellAction
{
public:
    CastMongooseBiteAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "mongoose bite") {}
};

// AoE Spells

class CastMultiShotAction : public CastSpellAction
{
public:
    CastMultiShotAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "multi-shot") {}
};

class CastVolleyAction : public CastSpellAction
{
public:
    CastVolleyAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "volley") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

#endif
