/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PALADINTRIGGERS_H
#define PLAYERBOTS_PALADINTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"
#include "Unit.h"

class PlayerbotAI;

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

class HammerOfJusticeInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    HammerOfJusticeInterruptSpellTrigger(PlayerbotAI* botAI)
        : InterruptSpellTrigger(botAI, "hammer of justice") {}
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
        : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_DISEASE) {}
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
        : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_POISON) {}
};

class CleanseCureMagicTrigger : public NeedCureTrigger
{
public:
    CleanseCureMagicTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse", DISPEL_MAGIC) {}
};

class CleanseCurePartyMemberMagicTrigger : public PartyMemberNeedCureTrigger
{
public:
    CleanseCurePartyMemberMagicTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse", DISPEL_MAGIC) {}
};

class HammerOfJusticeEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
public:
    HammerOfJusticeEnemyHealerTrigger(PlayerbotAI* botAI)
        : InterruptEnemyHealerTrigger(botAI, "hammer of justice") {}
};

class DivineFavorTrigger : public BuffTrigger
{
public:
    DivineFavorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "divine favor") {}
};

class DivineShieldLowHealthTrigger : public Trigger
{
public:
    DivineShieldLowHealthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "divine shield low health") {}

    bool IsActive() override;
};

class NotSensingUndeadTrigger : public BuffTrigger
{
public:
    NotSensingUndeadTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "not sensing undead") {}

    bool IsActive() override;
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
    BeaconOfLightOnMainTankTrigger(PlayerbotAI* botAI)
        : BuffOnMainTankTrigger(botAI, "beacon of light", true) {}
};

class SacredShieldOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    SacredShieldOnMainTankTrigger(PlayerbotAI* botAI)
        : BuffOnMainTankTrigger(botAI, "sacred shield", false) {}
};

class HandOfFreedomOnPartyTrigger : public Trigger
{
public:
    HandOfFreedomOnPartyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hand of freedom on party", 1) {}

    Unit* GetTarget() override;
    std::string const GetTargetName() override { return "party member snared target"; }
    bool IsActive() override;
};

class AvengingWrathTrigger : public BoostTrigger
{
public:
    AvengingWrathTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "avenging wrath") {}
};

class BlessingNeededTrigger : public Trigger
{
public:
    BlessingNeededTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "blessing needed", 4) {}

    bool IsActive() override;
};

#endif
