/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINTRIGGERS_H
#define _PLAYERBOT_PALADINTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"
#include "Unit.h"

class PlayerbotAI;

inline std::string const GetActualBlessingOfMight(Unit* target)
{
    switch (target->getClass())
    {
        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return "blessing of wisdom";
    }

    return "blessing of might";
}

inline std::string const GetActualBlessingOfWisdom(Unit* target)
{
    switch (target->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
            return "blessing of might";
    }

    return "blessing of wisdom";
}

BUFF_TRIGGER(HolyShieldTrigger, "holy shield");
BUFF_TRIGGER(RighteousFuryTrigger, "righteous fury");

BUFF_TRIGGER(RetributionAuraTrigger, "retribution aura");
BUFF_TRIGGER(SanctityAuraTrigger, "sanctity aura");

class CrusaderAuraTrigger : public BuffTrigger
{
public:
    CrusaderAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "crusader aura") {}

    bool IsActive() override;
};

class SealTrigger : public BuffTrigger
{
public:
    SealTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "seal") {}

    bool IsActive() override;
};

// judgements
DEBUFF_TRIGGER(JudgementTrigger, "judgement");
DEBUFF_TRIGGER(JudgementOfLightTrigger, "judgement of light");
DEBUFF_TRIGGER(JudgementOfWisdomTrigger, "judgement of wisdom");

DEBUFF_TRIGGER(ConsecrationTrigger, "consecration");

// repentance triggers
INTERRUPT_HEALER_TRIGGER(RepentanceOnHealerTrigger, "repentance on enemy healer");
SNARE_TRIGGER(RepentanceSnareTrigger, "repentance on snare target");
INTERRUPT_TRIGGER(RepentanceInterruptTrigger, "repentance");

class BlessingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    BlessingOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "blessing of kings,blessing of might,blessing of wisdom", 2 * 2000)
    {
    }
};

class BlessingTrigger : public BuffTrigger
{
public:
    BlessingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "blessing of sanctuary", 2 * 2000) {}

    bool IsActive() override;
};

class HammerOfJusticeInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    HammerOfJusticeInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "hammer of justice") {}
};

class HammerOfJusticeSnareTrigger : public SnareTargetTrigger
{
public:
    HammerOfJusticeSnareTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "hammer of justice") {}
};

class ArtOfWarTrigger : public HasAuraTrigger
{
public:
    ArtOfWarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "the art of war") {}
};

class ShadowResistanceAuraTrigger : public BuffTrigger
{
public:
    ShadowResistanceAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "shadow resistance aura") {}
};

class FrostResistanceAuraTrigger : public BuffTrigger
{
public:
    FrostResistanceAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "frost resistance aura") {}
};

class FireResistanceAuraTrigger : public BuffTrigger
{
public:
    FireResistanceAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "fire resistance aura") {}
};

class DevotionAuraTrigger : public BuffTrigger
{
public:
    DevotionAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "devotion aura") {}
};

BUFF_TRIGGER(ConcentrationAuraTrigger, "concentration aura");

class CleanseCureDiseaseTrigger : public NeedCureTrigger
{
public:
    CleanseCureDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_DISEASE) {}
};

class CleanseCurePartyMemberDiseaseTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleanseCurePartyMemberDiseaseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_DISEASE)
    {
    }
};

class CleanseCurePoisonTrigger : public NeedCureTrigger
{
public:
    CleanseCurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_POISON) {}
};

class CleanseCurePartyMemberPoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleanseCurePartyMemberPoisonTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_POISON)
    {
    }
};

class CleanseCureMagicTrigger : public NeedCureTrigger
{
public:
    CleanseCureMagicTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_MAGIC) {}
};

class CleanseCurePartyMemberMagicTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleanseCurePartyMemberMagicTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_MAGIC)
    {
    }
};

class HammerOfJusticeEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
public:
    HammerOfJusticeEnemyHealerTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "hammer of justice") {}
};

class DivineFavorTrigger : public BuffTrigger
{
public:
    DivineFavorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "divine favor") {}
};

class TurnUndeadTrigger : public HasCcTargetTrigger
{
public:
    TurnUndeadTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "turn undead") {}
};

DEBUFF_TRIGGER(AvengerShieldTrigger, "avenger's shield");

class BeaconOfLightOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    BeaconOfLightOnMainTankTrigger(PlayerbotAI* ai) : BuffOnMainTankTrigger(ai, "beacon of light", true) {}
};

class SacredShieldOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    SacredShieldOnMainTankTrigger(PlayerbotAI* ai) : BuffOnMainTankTrigger(ai, "sacred shield", false) {}
};

class BlessingOfKingsOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    BlessingOfKingsOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "blessing of kings", 2 * 2000) {}
};

class BlessingOfWisdomOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    BlessingOfWisdomOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "blessing of might,blessing of wisdom", 2 * 2000)
    {
    }
};

class BlessingOfMightOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    BlessingOfMightOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "blessing of might,blessing of wisdom", 2 * 2000)
    {
    }
};

class BlessingOfSanctuaryOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    BlessingOfSanctuaryOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "blessing of sanctuary", 2 * 2000)
    {
    }
};

class AvengingWrathTrigger : public BoostTrigger
{
public:
    AvengingWrathTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "avenging wrath") {}
};
#endif
