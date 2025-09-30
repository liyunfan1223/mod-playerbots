/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARLOCKTRIGGERS_H
#define _PLAYERBOT_WARLOCKTRIGGERS_H

#include "GenericTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "CureTriggers.h"
#include "Trigger.h"
#include <set>

class PlayerbotAI;

// Buff and Out of Combat Triggers

class DemonArmorTrigger : public BuffTrigger
{
public:
    DemonArmorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "demon armor") {}

    bool IsActive() override;
};

class SoulLinkTrigger : public BuffTrigger
{
public:
    SoulLinkTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "soul link") {}
    bool IsActive() override;
};

class OutOfSoulShardsTrigger : public Trigger
{
public:
    OutOfSoulShardsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no soul shard", 2) {}
    bool IsActive() override;
};

class TooManySoulShardsTrigger : public Trigger
{
public:
    TooManySoulShardsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "too many soul shards") {}
    bool IsActive() override;
};

class FirestoneTrigger : public BuffTrigger
{
public:
    FirestoneTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "firestone") {}
    bool IsActive() override;
};

class SpellstoneTrigger : public BuffTrigger
{
public:
    SpellstoneTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "spellstone") {}
    bool IsActive() override;
};

class OutOfSoulstoneTrigger : public Trigger
{
public:
    OutOfSoulstoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no soulstone") {}
    bool IsActive() override;
};

class SoulstoneTrigger : public Trigger
{
public:
    SoulstoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soulstone") {}

    bool IsActive() override
    {
        static const std::vector<uint32> soulstoneSpellIds = {20707, 20762, 20763, 20764, 20765, 27239, 47883};

        if (AI_VALUE2(uint32, "item count", "soulstone") == 0)
            return false;

        for (uint32 spellId : soulstoneSpellIds)
        {
            if (!bot->HasSpellCooldown(spellId))
                return true;  // Ready to use
        }

        return false;  // All are on cooldown
    }
};

class WarlockConjuredItemTrigger : public ItemCountTrigger
{
public:
    WarlockConjuredItemTrigger(PlayerbotAI* botAI, std::string const item) : ItemCountTrigger(botAI, item, 1) {}
    bool IsActive() override;
};

class HasSpellstoneTrigger : public WarlockConjuredItemTrigger
{
public:
    HasSpellstoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "spellstone") {}
};

class HasFirestoneTrigger : public WarlockConjuredItemTrigger
{
public:
    HasFirestoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "firestone") {}
};

class HasHealthstoneTrigger : public WarlockConjuredItemTrigger
{
public:
    HasHealthstoneTrigger(PlayerbotAI* botAI) : WarlockConjuredItemTrigger(botAI, "healthstone") {}
};

class WrongPetTrigger : public Trigger
{
public:
    WrongPetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wrong pet") {}
    bool IsActive() override;
};


// CC and Pet Triggers

class BanishTrigger : public HasCcTargetTrigger
{
public:
    BanishTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "banish") {}
    bool IsActive() override;
};

class FearTrigger : public HasCcTargetTrigger
{
public:
    FearTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "fear") {}
    bool IsActive() override;
};

class SpellLockInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    SpellLockInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "spell lock") {}
};

class DevourMagicPurgeTrigger : public TargetAuraDispelTrigger
{
public:
    DevourMagicPurgeTrigger(PlayerbotAI* botAI) : TargetAuraDispelTrigger(botAI, "devour magic", DISPEL_MAGIC) {}
};

class DevourMagicCleanseTrigger : public PartyMemberNeedCureTrigger
{
public:
    DevourMagicCleanseTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "devour magic", DISPEL_MAGIC) {}
};

// DoT/Curse Triggers

class CorruptionTrigger : public DebuffTrigger
{
public:
    CorruptionTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "corruption", 1, true, 0.5f) {}
    bool IsActive() override
    {
        return BuffTrigger::IsActive() && !botAI->HasAura("seed of corruption", GetTarget(), false, true);
    }
};

class CorruptionOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    CorruptionOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "corruption", true) {}
    bool IsActive() override
    {
        return BuffTrigger::IsActive() && !botAI->HasAura("seed of corruption", GetTarget(), false, true);
    }
};

class ImmolateTrigger : public DebuffTrigger
{
public:
    ImmolateTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "immolate", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class ImmolateOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    ImmolateOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "immolate", true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class UnstableAfflictionTrigger : public DebuffTrigger
{
public:
    UnstableAfflictionTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "unstable affliction", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class UnstableAfflictionOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    UnstableAfflictionOnAttackerTrigger(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, "unstable affliction", true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class HauntTrigger : public DebuffTrigger
{
public:
    HauntTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "haunt", 1, true, 0) {}
};

class CurseOfAgonyTrigger : public DebuffTrigger
{
public:
    CurseOfAgonyTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of agony", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class CurseOfAgonyOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    CurseOfAgonyOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "curse of agony", true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class CurseOfTheElementsTrigger : public DebuffTrigger
{
public:
    CurseOfTheElementsTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of the elements", 1, true, 0.5f) {}
    bool IsActive() override;
};

class CurseOfDoomTrigger : public DebuffTrigger
{
public:
    CurseOfDoomTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of doom", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class CurseOfExhaustionTrigger : public DebuffTrigger
{
public:
    CurseOfExhaustionTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of exhaustion", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class CurseOfTonguesTrigger : public DebuffTrigger
{
public:
    CurseOfTonguesTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of tongues", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class CurseOfWeaknessTrigger : public DebuffTrigger
{
public:
    CurseOfWeaknessTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "curse of weakness", 1, true, 0.5f) {}
    bool IsActive() override;
};

// Proc/Cooldown Triggers

class LifeTapTrigger : public Trigger
{
public:
    LifeTapTrigger(PlayerbotAI* ai) : Trigger(ai, "life tap") {}
    bool IsActive() override;
};

class LifeTapGlyphBuffTrigger : public BuffTrigger
{
public:
    LifeTapGlyphBuffTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "life tap") {}
    bool IsActive() override;
};

class MetamorphosisTrigger : public BoostTrigger
{
public:
    MetamorphosisTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "metamorphosis") {}
};

class DemonicEmpowermentTrigger : public BuffTrigger
{
public:
    DemonicEmpowermentTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "demonic empowerment") {}
    bool IsActive() override;
};

class ImmolationAuraActiveTrigger : public HasAuraTrigger
{
public:
    ImmolationAuraActiveTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "immolation aura") {}
};

class ShadowTranceTrigger : public HasAuraTrigger
{
public:
    ShadowTranceTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "shadow trance") {}
};

class BacklashTrigger : public HasAuraTrigger
{
public:
    BacklashTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "backlash") {}
};

class DecimationTrigger : public HasAuraTrigger
{
public:
    DecimationTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "decimation") {}
    bool IsActive() override;
};

class MoltenCoreTrigger : public HasAuraTrigger
{
public:
    MoltenCoreTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "molten core") {}
};

class MetamorphosisNotActiveTrigger : public HasNoAuraTrigger
{
public:
    MetamorphosisNotActiveTrigger(PlayerbotAI* ai) : HasNoAuraTrigger(ai, "metamorphosis") {}
};

class MetaMeleeEnemyTooCloseForSpellTrigger : public TwoTriggers
{
public:
    MetaMeleeEnemyTooCloseForSpellTrigger(PlayerbotAI* ai)
        : TwoTriggers(ai, "enemy too close for spell", "metamorphosis not active") {}
};

class RainOfFireChannelCheckTrigger : public Trigger
{
public:
    RainOfFireChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 2)
        : Trigger(botAI, "rain of fire channel check"), minEnemies(minEnemies)
    {
    }

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> RAIN_OF_FIRE_SPELL_IDS;
};

#endif
