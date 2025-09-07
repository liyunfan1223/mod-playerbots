/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HUNTERTRIGGERS_H
#define _PLAYERBOT_HUNTERTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "Trigger.h"
#include "PlayerbotAI.h"
#include <set>

class PlayerbotAI;

// Buff and Out of Combat Triggers

class HunterAspectOfTheMonkeyTrigger : public BuffTrigger
{
public:
    HunterAspectOfTheMonkeyTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "aspect of the monkey") {}
};

class HunterAspectOfTheHawkTrigger : public BuffTrigger
{
public:
    HunterAspectOfTheHawkTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "aspect of the hawk") {}
    bool IsActive() override;
};

class HunterAspectOfTheWildTrigger : public BuffTrigger
{
public:
    HunterAspectOfTheWildTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "aspect of the wild") {}
};

class HunterAspectOfTheViperTrigger : public BuffTrigger
{
public:
    HunterAspectOfTheViperTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "aspect of the viper") {}
    bool IsActive() override;
};

class HunterAspectOfThePackTrigger : public BuffTrigger
{
public:
    HunterAspectOfThePackTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "aspect of the pack") {}
    bool IsActive() override;
};

class TrueshotAuraTrigger : public BuffTrigger
{
public:
    TrueshotAuraTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "trueshot aura") {}
};

class NoTrackTrigger : public BuffTrigger
{
public:
    NoTrackTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "no track") {}
    bool IsActive() override;
};

class HunterLowAmmoTrigger : public AmmoCountTrigger
{
public:
    HunterLowAmmoTrigger(PlayerbotAI* botAI) : AmmoCountTrigger(botAI, "ammo", 1, 30) {}

    bool IsActive() override;
};

class HunterNoAmmoTrigger : public AmmoCountTrigger
{
public:
    HunterNoAmmoTrigger(PlayerbotAI* botAI) : AmmoCountTrigger(botAI, "ammo", 1, 10) {}
};

class HunterHasAmmoTrigger : public AmmoCountTrigger
{
public:
    HunterHasAmmoTrigger(PlayerbotAI* botAI) : AmmoCountTrigger(botAI, "ammo", 1, 10) {}

    bool IsActive() override;
};

// Cooldown Triggers

class RapidFireTrigger : public BoostTrigger
{
public:
    RapidFireTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "rapid fire") {}
};

class BestialWrathTrigger : public BuffTrigger
{
public:
    BestialWrathTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "bestial wrath") {}
};

class IntimidationTrigger : public BuffTrigger
{
public:
    IntimidationTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "intimidation") {}
};

class KillCommandTrigger : public BuffTrigger
{
public:
    KillCommandTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "kill command") {}
    bool IsActive() override;
};

class LockAndLoadTrigger : public BuffTrigger
{
public:
    LockAndLoadTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "lock and load") {}

    bool IsActive() override
    {
        return botAI->HasAura("lock and load", botAI->GetBot());
    }
};

// CC Triggers

class FreezingTrapTrigger : public HasCcTargetTrigger
{
public:
    FreezingTrapTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "freezing trap") {}
};

class ConsussiveShotSnareTrigger : public SnareTargetTrigger
{
public:
    ConsussiveShotSnareTrigger(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, "concussive shot") {}
};

class ScareBeastTrigger : public HasCcTargetTrigger
{
public:
    ScareBeastTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "scare beast") {}
};

class SilencingShotTrigger : public InterruptSpellTrigger
{
public:
    SilencingShotTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "silencing shot") {}
};

// DoT/Debuff Triggers

class HuntersMarkTrigger : public DebuffTrigger
{
public:
    HuntersMarkTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "hunter's mark", 1, true, 0.5f) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class ExplosiveShotTrigger : public DebuffTrigger
{
public:
    ExplosiveShotTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "explosive shot", 1, true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class BlackArrowTrigger : public DebuffTrigger
{
public:
    BlackArrowTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "black arrow", 1, true) {}
    bool IsActive() override;
};

class HunterNoStingsActiveTrigger : public DebuffTrigger
{
public:
    HunterNoStingsActiveTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "no stings") {}
    bool IsActive() override;
};

class SerpentStingOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    SerpentStingOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "serpent sting", true) {}
    bool IsActive() override;
};

// Damage/Combat Triggers

class AutoShotTrigger : public Trigger
{
public:
    AutoShotTrigger(PlayerbotAI* botAI) : Trigger(botAI, "auto shot") {}
};

class SwitchToRangedTrigger : public Trigger
{
public:
    SwitchToRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "switch to ranged") {}

    bool IsActive() override;
};

class SwitchToMeleeTrigger : public Trigger
{
public:
    SwitchToMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "switch to melee") {}

    bool IsActive() override;
};

class MisdirectionOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    MisdirectionOnMainTankTrigger(PlayerbotAI* ai) : BuffOnMainTankTrigger(ai, "misdirection", true) {}
};

class TargetRemoveEnrageTrigger : public TargetAuraDispelTrigger
{
public:
    TargetRemoveEnrageTrigger(PlayerbotAI* ai) : TargetAuraDispelTrigger(ai, "tranquilizing shot", DISPEL_ENRAGE) {}
};

class TargetRemoveMagicTrigger : public TargetAuraDispelTrigger
{
public:
    TargetRemoveMagicTrigger(PlayerbotAI* ai) : TargetAuraDispelTrigger(ai, "tranquilizing shot", DISPEL_MAGIC) {}
};

class ImmolationTrapNoCdTrigger : public SpellNoCooldownTrigger
{
public:
    ImmolationTrapNoCdTrigger(PlayerbotAI* ai) : SpellNoCooldownTrigger(ai, "immolation trap") {}
};

BEGIN_TRIGGER(HuntersPetDeadTrigger, Trigger)
END_TRIGGER()

BEGIN_TRIGGER(HuntersPetLowHealthTrigger, Trigger)
END_TRIGGER()

BEGIN_TRIGGER(HuntersPetMediumHealthTrigger, Trigger)
END_TRIGGER()

BEGIN_TRIGGER(HunterPetNotHappy, Trigger)
END_TRIGGER()

class VolleyChannelCheckTrigger : public Trigger
{
public:
    VolleyChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 2)
        : Trigger(botAI, "volley channel check"), minEnemies(minEnemies)
    {
    }

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> VOLLEY_SPELL_IDS;
};

#endif
