/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAGETRIGGERS_H
#define _PLAYERBOT_MAGETRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"

class PlayerbotAI;

DEFLECT_TRIGGER(FireWardTrigger, "fire ward");
DEFLECT_TRIGGER(FrostWardTrigger, "frost ward");

class ArcaneIntellectOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    ArcaneIntellectOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "arcane intellect", 2 * 2000) {}

    bool IsActive() override;
};

class ArcaneIntellectTrigger : public BuffTrigger
{
public:
    ArcaneIntellectTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "arcane intellect", 2 * 2000) {}
    bool IsActive() override;
};

class MageArmorTrigger : public BuffTrigger
{
public:
    MageArmorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "mage armor", 5 * 2000) {}
    bool IsActive() override;
};

class LivingBombTrigger : public DebuffTrigger
{
public:
    LivingBombTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "living bomb", 1, true) {}
};

class FireballTrigger : public DebuffTrigger
{
public:
    FireballTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "fireball", 1, true) {}
};

class PyroblastTrigger : public DebuffTrigger
{
public:
    PyroblastTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "pyroblast", 1, true) {}
};

class HotStreakTrigger : public HasAuraTrigger
{
public:
    HotStreakTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "hot streak") {}
};

class MissileBarrageTrigger : public HasAuraTrigger
{
public:
    MissileBarrageTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "missile barrage") {}
};

class ArcaneBlastTrigger : public BuffTrigger
{
public:
    ArcaneBlastTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "arcane blast") {}
};

class FingersOfFrostSingleTrigger : public HasAuraStackTrigger
{
public:
    FingersOfFrostSingleTrigger(PlayerbotAI* ai) : HasAuraStackTrigger(ai, "fingers of frost", 1, 1) {}
    bool IsActive() override;
};

class FingersOfFrostDoubleTrigger : public HasAuraStackTrigger
{
public:
    FingersOfFrostDoubleTrigger(PlayerbotAI* ai) : HasAuraStackTrigger(ai, "fingers of frost", 2, 1) {}
    // bool IsActive() override;
};

class BrainFreezeTrigger : public HasAuraTrigger
{
public:
    BrainFreezeTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "fireball!") {}
};

class CounterspellInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    CounterspellInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "counterspell") {}
};

class CombustionTrigger : public BoostTrigger
{
public:
    CombustionTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "combustion") {}
};

class IcyVeinsTrigger : public BoostTrigger
{
public:
    IcyVeinsTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "icy veins") {}
};

class ColdSnapTrigger : public BoostTrigger
{
public:
    ColdSnapTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "cold snap") {}
};

class IceBarrierTrigger : public BuffTrigger
{
public:
    IceBarrierTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "ice barrier") {}
};

class PolymorphTrigger : public HasCcTargetTrigger
{
public:
    PolymorphTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "polymorph") {}
};

class RemoveCurseTrigger : public NeedCureTrigger
{
public:
    RemoveCurseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "remove curse", DISPEL_CURSE) {}
};

class PartyMemberRemoveCurseTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberRemoveCurseTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "remove curse", DISPEL_CURSE)
    {
    }
};

class SpellstealTrigger : public TargetAuraDispelTrigger
{
public:
    SpellstealTrigger(PlayerbotAI* botAI) : TargetAuraDispelTrigger(botAI, "spellsteal", DISPEL_MAGIC) {}
};

class CounterspellEnemyHealerTrigger : public InterruptEnemyHealerTrigger
{
public:
    CounterspellEnemyHealerTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "counterspell") {}
};

class ArcanePowerTrigger : public BuffTrigger
{
public:
    ArcanePowerTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "arcane power") {}
};

class PresenceOfMindTrigger : public BuffTrigger
{
public:
    PresenceOfMindTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "presence of mind") {}
};

class ArcaneBlastStackTrigger : public HasAuraStackTrigger
{
public:
    ArcaneBlastStackTrigger(PlayerbotAI* botAI) : HasAuraStackTrigger(botAI, "arcane blast", 3, 1) {}
};

class MirrorImageTrigger : public BoostTrigger
{
public:
    MirrorImageTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "mirror image") {}
};

class FrostNovaOnTargetTrigger : public DebuffTrigger
{
public:
    FrostNovaOnTargetTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "frost nova", 1, false) {}
    bool IsActive() override;
};

class FrostbiteOnTargetTrigger : public DebuffTrigger
{
public:
    FrostbiteOnTargetTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "frostbite", 1, false) {}
    bool IsActive() override;
};

#endif
