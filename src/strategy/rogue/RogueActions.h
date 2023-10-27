/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ROGUEACTIONS_H
#define _PLAYERBOT_ROGUEACTIONS_H

#include "GenericSpellActions.h"
#include "UseItemAction.h"

class PlayerbotAI;

class CastEvasionAction : public CastBuffSpellAction
{
	public:
		CastEvasionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "evasion") { }
};

class CastSprintAction : public CastBuffSpellAction
{
	public:
		CastSprintAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "sprint") { }

        std::string const GetTargetName() override { return "self target"; }
};

class CastStealthAction : public CastBuffSpellAction
{
    public:
        CastStealthAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "stealth") { }

        std::string const GetTargetName() override { return "self target"; }

        bool isPossible() override;
};

class UnstealthAction : public Action
{
    public:
        UnstealthAction(PlayerbotAI* botAI) : Action(botAI, "unstealth") { }

        bool Execute(Event event) override;
};

class CheckStealthAction : public Action
{
    public:
        CheckStealthAction(PlayerbotAI* botAI) : Action(botAI, "check stealth") { }

        bool isPossible() override { return true; }
        bool Execute(Event event) override;
};

class CastKickAction : public CastSpellAction
{
	public:
		CastKickAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "kick") { }
};

class CastFeintAction : public CastBuffSpellAction
{
	public:
		CastFeintAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "feint") { }
};

class CastDismantleAction : public CastSpellAction
{
	public:
		CastDismantleAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dismantle") { }
};

class CastDistractAction : public CastSpellAction
{
	public:
		CastDistractAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "distract") { }
};

class CastVanishAction : public CastBuffSpellAction
{
	public:
		CastVanishAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "vanish") { }

        bool isUseful() override;
};

class CastBlindAction : public CastDebuffSpellAction
{
	public:
		CastBlindAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "blind") { }
};

class CastBladeFlurryAction : public CastBuffSpellAction
{
	public:
		CastBladeFlurryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blade flurry") { }
};

class CastAdrenalineRushAction : public CastBuffSpellAction
{
	public:
		CastAdrenalineRushAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "adrenaline rush") { }
};

class CastKillingSpreeAction : public CastMeleeSpellAction
{
	public:
		CastKillingSpreeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "killing spree") { }
};

class CastKickOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
    public:
        CastKickOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "kick") { }
};

class EnvenomAction : public CastMeleeSpellAction
{
	public:
		EnvenomAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "envenom") {}
};

class CastTricksOfTheTradeOnMainTankAction : public BuffOnMainTankAction
{
	public:
		CastTricksOfTheTradeOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "tricks of the trade", true) {}
		virtual bool isUseful() override;
};

class UseDeadlyPoisonAction : public UseItemAction
{
	public:
		UseDeadlyPoisonAction(PlayerbotAI* ai) : UseItemAction(ai, "Deadly Poison") {}
		virtual bool Execute(Event event) override;
		virtual bool isPossible() override;
};

class UseInstantPoisonAction : public UseItemAction
{
	public:
		UseInstantPoisonAction(PlayerbotAI* ai) : UseItemAction(ai, "Instant Poison") {}
		virtual bool Execute(Event event) override;
		virtual bool isPossible() override;
};

class FanOfKnivesAction : public CastMeleeSpellAction
{
	public:
		FanOfKnivesAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "fan of knives") {}
		ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

#endif
