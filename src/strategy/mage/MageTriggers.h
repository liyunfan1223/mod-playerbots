/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAGETRIGGERS_H
#define _PLAYERBOT_MAGETRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Playerbots.h"
#include "PlayerbotAI.h"
#include <set>
#include <unordered_set>

class PlayerbotAI;

// Buff and Out of Combat Triggers

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

class NoFocusMagicTrigger : public Trigger
{
public:
    NoFocusMagicTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no focus magic") {}
    bool IsActive() override;
};

class IceBarrierTrigger : public BuffTrigger
{
public:
    IceBarrierTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "ice barrier") {}
};

class NoManaGemTrigger : public Trigger
{
public:
    NoManaGemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no mana gem") {}

    bool IsActive() override;
};

class FireWardTrigger : public DeflectSpellTrigger
{
public:
    FireWardTrigger(PlayerbotAI* botAI) : DeflectSpellTrigger(botAI, "fire ward") {}
};

class FrostWardTrigger : public DeflectSpellTrigger
{
public:
    FrostWardTrigger(PlayerbotAI* botAI) : DeflectSpellTrigger(botAI, "frost ward") {}
};

// Proc and Boost Triggers

class HotStreakTrigger : public HasAuraTrigger
{
public:
    HotStreakTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "hot streak") {}
};

class FirestarterTrigger : public HasAuraTrigger
{
public:
    FirestarterTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "firestarter") {}
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

class ArcaneBlastStackTrigger : public HasAuraStackTrigger
{
public:
    ArcaneBlastStackTrigger(PlayerbotAI* botAI) : HasAuraStackTrigger(botAI, "arcane blast", 4, 1) {}
};

class ArcaneBlast4StacksAndMissileBarrageTrigger : public TwoTriggers
{
public:
    ArcaneBlast4StacksAndMissileBarrageTrigger(PlayerbotAI* ai)
        : TwoTriggers(ai, "arcane blast stack", "missile barrage")
    {
    }
};

class CombustionTrigger : public BoostTrigger
{
public:
    CombustionTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "combustion") {}
};

class IcyVeinsCooldownTrigger : public SpellCooldownTrigger
{
public:
    IcyVeinsCooldownTrigger(PlayerbotAI* botAI) : SpellCooldownTrigger(botAI, "icy veins") {}
};

class DeepFreezeCooldownTrigger : public SpellCooldownTrigger
{
public:
    DeepFreezeCooldownTrigger(PlayerbotAI* botAI) : SpellCooldownTrigger(botAI, "deep freeze") {}

    bool IsActive() override;
};

class ColdSnapTrigger : public TwoTriggers
{
public:
    ColdSnapTrigger(PlayerbotAI* ai) : TwoTriggers(ai, "icy veins on cd", "deep freeze on cd") {}
};

class MirrorImageTrigger : public BoostTrigger
{
public:
    MirrorImageTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "mirror image") {}
};

class IcyVeinsTrigger : public BoostTrigger
{
public:
    IcyVeinsTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "icy veins") {}
};

class ArcanePowerTrigger : public BoostTrigger
{
public:
    ArcanePowerTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "arcane power") {}
};
class PresenceOfMindTrigger : public BoostTrigger
{
public:
    PresenceOfMindTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "presence of mind") {}
};

// CC, Interrupt, and Dispel Triggers

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

class CounterspellInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    CounterspellInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "counterspell") {}
};

// Damage and Debuff Triggers

class LivingBombTrigger : public DebuffTrigger
{
public:
    LivingBombTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "living bomb", 1, true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class LivingBombOnAttackersTrigger : public DebuffOnAttackerTrigger
{
public:
    LivingBombOnAttackersTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "living bomb", true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class FireballTrigger : public DebuffTrigger
{
public:
    FireballTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "fireball", 1, true) {}
};

class ImprovedScorchTrigger : public DebuffTrigger
{
public:
    ImprovedScorchTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "improved scorch", 1, true, 0.5f) {}
    bool IsActive() override;
};

class PyroblastTrigger : public DebuffTrigger
{
public:
    PyroblastTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "pyroblast", 1, true) {}
};

class FrostfireBoltTrigger : public DebuffTrigger
{
public:
    FrostfireBoltTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "frostfire bolt", 1, true) {}
};

class FingersOfFrostTrigger : public HasAuraTrigger
{
public:
    FingersOfFrostTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "fingers of frost") {}
};

class BrainFreezeTrigger : public HasAuraTrigger
{
public:
    BrainFreezeTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "fireball!") {}
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

class FlamestrikeNearbyTrigger : public Trigger
{
public:
    FlamestrikeNearbyTrigger(PlayerbotAI* botAI, float radius = 30.0f)
        : Trigger(botAI, "flamestrike nearby"), radius(radius)
    {
    }
    bool IsActive() override;

protected:
    float radius;
    static const std::set<uint32> FLAMESTRIKE_SPELL_IDS;
};

class FlamestrikeBlizzardTrigger : public TwoTriggers
{
public:
    FlamestrikeBlizzardTrigger(PlayerbotAI* ai) : TwoTriggers(ai, "flamestrike nearby", "medium aoe") {}
};

class BlizzardChannelCheckTrigger : public Trigger
{
public:
    BlizzardChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 2)
        : Trigger(botAI, "blizzard channel check"), minEnemies(minEnemies) {}

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> BLIZZARD_SPELL_IDS;
};

class BlastWaveOffCdTrigger : public SpellNoCooldownTrigger
{
public:
    BlastWaveOffCdTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "blast wave") {}
};

class BlastWaveOffCdTriggerAndMediumAoeTrigger : public TwoTriggers
{
public:
    BlastWaveOffCdTriggerAndMediumAoeTrigger(PlayerbotAI* ai) : TwoTriggers(ai, "blast wave off cd", "medium aoe") {}
};

class NoFirestarterStrategyTrigger : public Trigger
{
public:
    NoFirestarterStrategyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no firestarter strategy") {}

    bool IsActive() override
    {
        return !botAI->HasStrategy("firestarter", BOT_STATE_COMBAT);
    }
};

class EnemyIsCloseAndNoFirestarterStrategyTrigger : public TwoTriggers
{
public:
    EnemyIsCloseAndNoFirestarterStrategyTrigger(PlayerbotAI* botAI)
        : TwoTriggers(botAI, "enemy is close", "no firestarter strategy") {}
};

class EnemyTooCloseForSpellAndNoFirestarterStrategyTrigger : public TwoTriggers
{
public:
    EnemyTooCloseForSpellAndNoFirestarterStrategyTrigger(PlayerbotAI* botAI)
        : TwoTriggers(botAI, "enemy too close for spell", "no firestarter strategy") {}
};

#endif
