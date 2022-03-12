/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARLOCKTRIGGERS_H
#define _PLAYERBOT_WARLOCKTRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;

class DemonArmorTrigger : public BuffTrigger
{
	public:
		DemonArmorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "demon armor") { }

		bool IsActive() override;
};

class SpellstoneTrigger : public BuffTrigger
{
    public:
        SpellstoneTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "spellstone") { }

        bool IsActive() override;
};

DEBUFF_TRIGGER(CurseOfAgonyTrigger, "curse of agony");
DEBUFF_TRIGGER(CorruptionTrigger, "corruption");
DEBUFF_TRIGGER(SiphonLifeTrigger, "siphon life");

class CorruptionOnAttackerTrigger : public DebuffOnAttackerTrigger
{
    public:
        CorruptionOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "corruption") { }
};

class CastCurseOfAgonyOnAttackerTrigger : public DebuffOnAttackerTrigger
{
    public:
        CastCurseOfAgonyOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "curse of agony") { }
};

class SiphonLifeOnAttackerTrigger : public DebuffOnAttackerTrigger
{
    public:
        SiphonLifeOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "siphon life") { }
};

DEBUFF_TRIGGER(ImmolateTrigger, "immolate");

class ShadowTranceTrigger : public HasAuraTrigger
{
    public:
        ShadowTranceTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "shadow trance") { }
};

class BacklashTrigger : public HasAuraTrigger
{
    public:
        BacklashTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "backlash") { }
};

class BanishTrigger : public HasCcTargetTrigger
{
    public:
        BanishTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "banish") { }
};

class WarlockConjuredItemTrigger : public ItemCountTrigger
{
    public:
        WarlockConjuredItemTrigger(PlayerbotAI* botAI, std::string const item) : ItemCountTrigger(botAI, item, 1) { }

        bool IsActive() override;
};

class HasSpellstoneTrigger : public WarlockConjuredItemTrigger
{
    public:
        HasSpellstoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "spellstone") { }
};

class HasFirestoneTrigger : public WarlockConjuredItemTrigger
{
    public:
        HasFirestoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "firestone") { }
};

class HasHealthstoneTrigger : public WarlockConjuredItemTrigger
{
    public:
        HasHealthstoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "healthstone") { }
};

class FearTrigger : public HasCcTargetTrigger
{
    public:
        FearTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "fear") { }
};

class AmplifyCurseTrigger : public BuffTrigger
{
    public:
        AmplifyCurseTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "amplify curse") { }
};

#endif
