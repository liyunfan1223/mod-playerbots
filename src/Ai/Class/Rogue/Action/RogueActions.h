/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ROGUEACTIONS_H
#define PLAYERBOTS_ROGUEACTIONS_H

#include "GenericSpellActions.h"
#include "UseItemAction.h"

class PlayerbotAI;

class CastEvasionAction : public CastBuffSpellAction
{
public:
    CastEvasionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "evasion") {}
};

class CastCloakOfShadowsAction : public CastBuffSpellAction
{
public:
    CastCloakOfShadowsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cloak of shadows") {}
};

class CastHungerForBloodAction : public CastBuffSpellAction
{
public:
    CastHungerForBloodAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "hunger for blood") {}

    std::string const GetTargetName() override { return "current target"; }
};

class CastSprintAction : public CastBuffSpellAction
{
public:
    CastSprintAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "sprint") {}

    std::string const GetTargetName() override { return "self target"; }
};

class CastStealthAction : public CastBuffSpellAction
{
public:
    CastStealthAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "stealth") {}

    bool isUseful() override;
    bool isPossible() override;
    std::string const GetTargetName() override { return "self target"; }
};

class UnstealthAction : public Action
{
public:
    UnstealthAction(PlayerbotAI* botAI) : Action(botAI, "unstealth") {}

    bool Execute(Event event) override;
};

class CheckStealthAction : public Action
{
public:
    CheckStealthAction(PlayerbotAI* botAI) : Action(botAI, "check stealth") {}

    bool Execute(Event event) override;
    bool isPossible() override { return true; }
};

class CastKickAction : public CastSpellAction
{
public:
    CastKickAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "kick") {}
};

class CastFeintAction : public CastBuffSpellAction
{
public:
    CastFeintAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "feint") {}
};

class CastColdBloodAction : public CastBuffSpellAction
{
public:
    CastColdBloodAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cold blood") {}
};

class CastDismantleAction : public CastSpellAction
{
public:
    CastDismantleAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dismantle") {}
};

class CastDistractAction : public CastSpellAction
{
public:
    CastDistractAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "distract") {}
};

class CastVanishAction : public CastBuffSpellAction
{
public:
    CastVanishAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "vanish") {}

    bool isUseful() override;
};

class CastBlindAction : public CastDebuffSpellAction
{
public:
    CastBlindAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "blind") {}
};

class CastBladeFlurryAction : public CastBuffSpellAction
{
public:
    CastBladeFlurryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blade flurry") {}
};

class CastAdrenalineRushAction : public CastBuffSpellAction
{
public:
    CastAdrenalineRushAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "adrenaline rush") {}
};

class CastKillingSpreeAction : public CastMeleeSpellAction
{
public:
    CastKillingSpreeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "killing spree") {}
};

class CastKickOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastKickOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "kick") {}
};

class CastEnvenomAction : public CastMeleeSpellAction
{
public:
    CastEnvenomAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "envenom") {}

    bool isUseful() override;
    bool isPossible() override;
};

class CastTricksOfTheTradeOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastTricksOfTheTradeOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "tricks of the trade", true) {}

    bool isUseful() override;
};

class UseDeadlyPoisonAction : public UseItemAction
{
public:
    UseDeadlyPoisonAction(PlayerbotAI* botAI) : UseItemAction(botAI, "Deadly Poison") {}

    bool Execute(Event event) override;
    bool isPossible() override { return true; }
};

class UseInstantPoisonAction : public UseItemAction
{
public:
    UseInstantPoisonAction(PlayerbotAI* botAI) : UseItemAction(botAI, "Instant Poison") {}

    bool Execute(Event event) override;
    bool isPossible() override { return true; }
};

class UseInstantPoisonOffHandAction : public UseItemAction
{
public:
    UseInstantPoisonOffHandAction(PlayerbotAI* botAI)
        : UseItemAction(botAI, "Instant Poison Off Hand") {}

    bool Execute(Event event) override;
    bool isPossible() override { return true; }
};

class FanOfKnivesAction : public CastMeleeSpellAction
{
public:
    FanOfKnivesAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "fan of knives") {}

    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

#endif
