/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PRIESTTRIGGERS_H
#define _PLAYERBOT_PRIESTTRIGGERS_H

#include "CureTriggers.h"
#include "SharedDefines.h"

class PlayerbotAI;

DEBUFF_CHECKISOWNER_TRIGGER(HolyFireTrigger, "holy fire");
DEBUFF_CHECKISOWNER_TRIGGER(PowerWordPainTrigger, "shadow word: pain");
DEBUFF_ENEMY_TRIGGER(PowerWordPainOnAttackerTrigger, "shadow word: pain");
DEBUFF_CHECKISOWNER_TRIGGER(VampiricTouchTrigger, "vampiric touch");
DEBUFF_ENEMY_TRIGGER(VampiricTouchOnAttackerTrigger, "vampiric touch on attacker");
BUFF_TRIGGER(VampiricEmbraceTrigger, "vampiric embrace");
CURE_TRIGGER(DispelMagicTrigger, "dispel magic", DISPEL_MAGIC);
CURE_PARTY_TRIGGER(DispelMagicPartyMemberTrigger, "dispel magic", DISPEL_MAGIC);
CURE_TRIGGER(CureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
CURE_PARTY_TRIGGER(PartyMemberCureDiseaseTrigger, "cure disease", DISPEL_DISEASE);
BUFF_TRIGGER_A(InnerFireTrigger, "inner fire");
BUFF_TRIGGER_A(ShadowformTrigger, "shadowform");
BOOST_TRIGGER(PowerInfusionTrigger, "power infusion");
BUFF_TRIGGER(InnerFocusTrigger, "inner focus");
BUFF_TRIGGER(ShadowProtectionTrigger, "shadow protection");
BUFF_PARTY_TRIGGER(ShadowProtectionOnPartyTrigger, "shadow protection");
CC_TRIGGER(ShackleUndeadTrigger, "shackle undead");
INTERRUPT_TRIGGER(SilenceTrigger, "silence");
INTERRUPT_HEALER_TRIGGER(SilenceEnemyHealerTrigger, "silence");

// racials
DEBUFF_CHECKISOWNER_TRIGGER(DevouringPlagueTrigger, "devouring plague");
BUFF_TRIGGER(TouchOfWeaknessTrigger, "touch of weakness");
DEBUFF_TRIGGER(HexOfWeaknessTrigger, "hex of weakness");
BUFF_TRIGGER(ShadowguardTrigger, "shadowguard");
BUFF_TRIGGER(FearWardTrigger, "fear ward");
DEFLECT_TRIGGER(FeedbackTrigger, "feedback");
SNARE_TRIGGER(ChastiseTrigger, "chastise");

BOOST_TRIGGER_A(ShadowfiendTrigger, "shadowfiend");

class PowerWordFortitudeOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    PowerWordFortitudeOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "power word: fortitude", 4 * 2000)
    {
    }

    bool IsActive() override;
};

class PowerWordFortitudeTrigger : public BuffTrigger
{
public:
    PowerWordFortitudeTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "power word: fortitude", 4 * 2000) {}

    bool IsActive() override;
};

class DivineSpiritOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    DivineSpiritOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "divine spirit", 4 * 2000) {}

    bool IsActive() override;
};

class DivineSpiritTrigger : public BuffTrigger
{
public:
    DivineSpiritTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "divine spirit", 4 * 2000) {}

    bool IsActive() override;
};

class PrayerOfFortitudeTrigger : public BuffOnPartyTrigger
{
public:
    PrayerOfFortitudeTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "prayer of fortitude", 3 * 2000) {}

    bool IsActive() override;
};

class PrayerOfSpiritTrigger : public BuffOnPartyTrigger
{
public:
    PrayerOfSpiritTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "prayer of spirit", 2 * 2000) {}

    bool IsActive() override;
};

class BindingHealTrigger : public PartyMemberLowHealthTrigger
{
public:
    BindingHealTrigger(PlayerbotAI* botAI);

    bool IsActive() override;
};

#endif
