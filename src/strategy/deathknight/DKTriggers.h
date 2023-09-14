/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DKTRIGGERS_H
#define _PLAYERBOT_DKTRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;

BUFF_TRIGGER(HornOfWinterTrigger, "horn of winter");
BUFF_TRIGGER(BoneShieldTrigger, "bone shield");
BUFF_TRIGGER(ImprovedIcyTalonsTrigger, "improved icy talons");
// DEBUFF_CHECKISOWNER_TRIGGER(PlagueStrikeDebuffTrigger, "blood plague");
class PlagueStrikeDebuffTrigger : public DebuffTrigger
{
	public:
		PlagueStrikeDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "blood plague", true, .0f) { }
};

// DEBUFF_CHECKISOWNER_TRIGGER(IcyTouchDebuffTrigger, "frost fever");
class IcyTouchDebuffTrigger : public DebuffTrigger
{
	public:
		IcyTouchDebuffTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "frost fever", true, .0f) { }
};

BUFF_TRIGGER(UnbreakableArmorTrigger, "unbreakable armor");
class PlagueStrikeDebuffOnAttackerTrigger : public DebuffOnMeleeAttackerTrigger
{
	public:
		PlagueStrikeDebuffOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnMeleeAttackerTrigger(botAI, "blood plague", true, .0f) { }
};

class IcyTouchDebuffOnAttackerTrigger : public DebuffOnMeleeAttackerTrigger
{
	public:
		IcyTouchDebuffOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnMeleeAttackerTrigger(botAI, "frost fever", true, .0f) { }
};

class DKPresenceTrigger : public BuffTrigger
{
    public:
        DKPresenceTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "blood presence") { }

        bool IsActive() override;
};

class BloodTapTrigger : public BuffTrigger
{
	public:
		BloodTapTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "blood tap") { }
};

class RaiseDeadTrigger : public BuffTrigger
{
	public:
	    RaiseDeadTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "raise dead") { }
};

class RuneStrikeTrigger : public SpellCanBeCastTrigger
{
	public:
		RuneStrikeTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "rune strike") { }
};

class DeathCoilTrigger : public SpellCanBeCastTrigger
{
	public:
		DeathCoilTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "death coil") { }
};

class PestilenceTrigger : public DebuffTrigger
{
	public:
		PestilenceTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "pestilence") { }
		virtual bool IsActive() override;
};

class BloodStrikeTrigger : public DebuffTrigger
{
	public:
	    BloodStrikeTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "blood strike", 1, true) { }
};

class HowlingBlastTrigger : public DebuffTrigger
{
	public:
		HowlingBlastTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "howling blast", 1, true) { }
};

class MindFreezeInterruptSpellTrigger : public InterruptSpellTrigger
{
    public:
		MindFreezeInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "mind freeze") { }
};

class StrangulateInterruptSpellTrigger : public InterruptSpellTrigger
{
	public:
		StrangulateInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "strangulate") { }
};

class KillingMachineTrigger : public BoostTrigger
{
    public:
		KillingMachineTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "killing machine") { }
};

class MindFreezeOnEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
    public:
		MindFreezeOnEnemyHealerTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "mind freeze") { }
};

class ChainsOfIceSnareTrigger : public SnareTargetTrigger
{
	public:
		ChainsOfIceSnareTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "chains of ice") { }
};

class StrangulateOnEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
	public:
		StrangulateOnEnemyHealerTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "strangulate") { }
};

class HighBloodRuneTrigger : public Trigger
{
	public:
		HighBloodRuneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high blood rune") { }
		bool IsActive() override;
};

class FreezingFogTrigger : public HasAuraTrigger
{
	public:
		FreezingFogTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "freezing fog") { }
};

#endif
