/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WARLOCKACTIONS_H
#define PLAYERBOTS_WARLOCKACTIONS_H

#include "Action.h"
#include "GenericSpellActions.h"
#include "InventoryAction.h"
#include "UseItemAction.h"

class PlayerbotAI;
class Unit;

// Buff and Out of Combat Spells

class CastDemonSkinAction : public CastBuffSpellAction
{
public:
    CastDemonSkinAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "demon skin") {}
};

class CastDemonArmorAction : public CastBuffSpellAction
{
public:
    CastDemonArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "demon armor") {}
};

class CastFelArmorAction : public CastBuffSpellAction
{
public:
    CastFelArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fel armor") {}
};

class CastSoulLinkAction : public CastBuffSpellAction
{
public:
    CastSoulLinkAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "soul link", false, 5000) {}
    std::string const GetTargetName() override { return "pet target"; }
};

class CastUnendingBreathAction : public CastBuffSpellAction
{
public:
    CastUnendingBreathAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "unending breath") {}
};

class CastUnendingBreathOnPartyAction : public BuffOnPartyAction
{
public:
    CastUnendingBreathOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "unending breath") {}
};

class CreateSoulShardAction : public Action
{
public:
    CreateSoulShardAction(PlayerbotAI* botAI) : Action(botAI, "create soul shard") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class DestroySoulShardAction : public InventoryAction
{
public:
    DestroySoulShardAction(PlayerbotAI* botAI) : InventoryAction(botAI, "destroy soul shard") {}

    bool Execute(Event event) override;
};

class CastCreateHealthstoneAction : public CastBuffSpellAction
{
public:
    CastCreateHealthstoneAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "create healthstone") {}
};

class CastCreateFirestoneAction : public CastBuffSpellAction
{
public:
    CastCreateFirestoneAction(PlayerbotAI* botAI);
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    static const std::vector<uint32> firestoneSpellIds;
};

class CastCreateSpellstoneAction : public CastBuffSpellAction
{
public:
    CastCreateSpellstoneAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "create spellstone") {}
};

class CastCreateSoulstoneAction : public CastBuffSpellAction
{
public:
    CastCreateSoulstoneAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "create soulstone") {}
    bool isUseful() override;
};

class UseSoulstoneSelfAction : public UseSpellItemAction
{
public:
    UseSoulstoneSelfAction(PlayerbotAI* botAI) : UseSpellItemAction(botAI, "soulstone") {}
    bool Execute(Event event) override;
};

class UseSoulstoneMasterAction : public UseSpellItemAction
{
public:
    UseSoulstoneMasterAction(PlayerbotAI* botAI) : UseSpellItemAction(botAI, "soulstone") {}
    bool Execute(Event event) override;
};

class UseSoulstoneTankAction : public UseSpellItemAction
{
public:
    UseSoulstoneTankAction(PlayerbotAI* botAI) : UseSpellItemAction(botAI, "soulstone") {}
    bool Execute(Event event) override;
};

class UseSoulstoneHealerAction : public UseSpellItemAction
{
public:
    UseSoulstoneHealerAction(PlayerbotAI* botAI) : UseSpellItemAction(botAI, "soulstone") {}
    bool Execute(Event event) override;
};

// Summoning Spells

class CastSummonVoidwalkerAction : public CastBuffSpellAction
{
public:
    CastSummonVoidwalkerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon voidwalker") {}
};

class CastSummonFelguardAction : public CastBuffSpellAction
{
public:
    CastSummonFelguardAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon felguard") {}
};

class CastSummonFelhunterAction : public CastBuffSpellAction
{
public:
    CastSummonFelhunterAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon felhunter") {}
};

class CastSummonImpAction : public CastBuffSpellAction
{
public:
    CastSummonImpAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon imp") {}
};

class CastSummonSuccubusAction : public CastBuffSpellAction
{
public:
    CastSummonSuccubusAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon succubus") {}
};
class CastFelDominationAction : public CastBuffSpellAction
{
public:
    CastFelDominationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fel domination") {}
};

// CC and Pet Spells

class CastBanishOnCcAction : public CastCrowdControlSpellAction
{
public:
    CastBanishOnCcAction(PlayerbotAI* botAI) : CastCrowdControlSpellAction(botAI, "banish") {}
    Value<Unit*>* GetTargetValue() override;
    bool isPossible() override;
};

class CastFearOnCcAction : public CastCrowdControlSpellAction
{
public:
    CastFearOnCcAction(PlayerbotAI* botAI) : CastCrowdControlSpellAction(botAI, "fear") {}
    Value<Unit*>* GetTargetValue() override;
    bool isPossible() override;
};

class CastSpellLockAction : public CastSpellAction
{
public:
    CastSpellLockAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "spell lock") {}
};

class CastDevourMagicPurgeAction : public CastSpellAction
{
public:
    CastDevourMagicPurgeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "devour magic") {}

    std::string const GetTargetName() override { return "current target"; }
};

class CastDevourMagicCleanseAction : public CastSpellAction
{
public:
    CastDevourMagicCleanseAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "devour magic cleanse") {}
    std::string const getName() override { return "cleanse magic on party"; }
};

// Utility Spells

class CastShadowWardAction : public CastBuffSpellAction
{
public:
    CastShadowWardAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "shadow ward") {}
};

class CastSoulshatterAction : public CastSpellAction
{
public:
    CastSoulshatterAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "soulshatter") {}
    bool isUseful() override;
};

class CastLifeTapAction : public CastSpellAction
{
public:
    CastLifeTapAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "life tap") {}

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

class DemonChargeAction : public CastSpellAction
{
public:
    DemonChargeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "demon charge") {}
};

// Cooldown Spells

class CastMetamorphosisAction : public CastBuffSpellAction
{
public:
    CastMetamorphosisAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "metamorphosis") {}
};

class CastDemonicEmpowermentAction : public CastBuffSpellAction
{
public:
    CastDemonicEmpowermentAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "demonic empowerment") {}
    std::string const GetTargetName() override { return "pet target"; }
};

// DoT/Curse Spells

class CastCorruptionAction : public CastDebuffSpellAction
{
public:
    CastCorruptionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "corruption", true) {}
    bool isUseful() override
    {
        // Bypass TTL check and prevent casting if Seed of Corruption is present
        return CastAuraSpellAction::isUseful() && !botAI->HasAura("seed of corruption", GetTarget(), false, true);
    }
};

class CastCorruptionOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastCorruptionOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "corruption", true) {}
    bool isUseful() override
    {
        // Bypass TTL check and prevent casting if Seed of Corruption is present
        return CastAuraSpellAction::isUseful() && !botAI->HasAura("seed of corruption", GetTarget(), false, true);
    }
};

class CastImmolateAction : public CastDebuffSpellAction
{
public:
    CastImmolateAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "immolate", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastImmolateOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastImmolateOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "immolate", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastUnstableAfflictionAction : public CastDebuffSpellAction
{
public:
    CastUnstableAfflictionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "unstable affliction", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastUnstableAfflictionOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastUnstableAfflictionOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnAttackerAction(botAI, "unstable affliction", true)
    {
    }
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfAgonyAction : public CastDebuffSpellAction
{
public:
    CastCurseOfAgonyAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of agony", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfAgonyOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastCurseOfAgonyOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnAttackerAction(botAI, "curse of agony", true)
    {
    }
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfTheElementsAction : public CastDebuffSpellAction
{
public:
    CastCurseOfTheElementsAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of the elements", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfDoomAction : public CastDebuffSpellAction
{
public:
    CastCurseOfDoomAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of doom", true, 0) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfExhaustionAction : public CastDebuffSpellAction
{
public:
    CastCurseOfExhaustionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of exhaustion") {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfTonguesAction : public CastDebuffSpellAction
{
public:
    CastCurseOfTonguesAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of tongues") {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastCurseOfWeaknessAction : public CastDebuffSpellAction
{
public:
    CastCurseOfWeaknessAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of weakness") {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

// Damage Spells

class CastShadowBoltAction : public CastSpellAction
{
public:
    CastShadowBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shadow bolt") {}
};

class CastDrainSoulAction : public CastSpellAction
{
public:
    CastDrainSoulAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "drain soul") {}

    bool isUseful() override;
};

class CastDrainManaAction : public CastSpellAction
{
public:
    CastDrainManaAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "drain mana") {}
};

class CastDrainLifeAction : public CastSpellAction
{
public:
    CastDrainLifeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "drain life") {}
};

class CastConflagrateAction : public CastSpellAction
{
public:
    CastConflagrateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "conflagrate") {}
};

class CastIncinerateAction : public CastSpellAction
{
public:
    CastIncinerateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "incinerate") {}
};

class CastHauntAction : public CastSpellAction
{
public:
    CastHauntAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "haunt") {}
};

class CastSoulFireAction : public CastSpellAction
{
public:
    CastSoulFireAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "soul fire") {}
};

class CastShadowburnAction : public CastSpellAction
{
public:
    CastShadowburnAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shadowburn") {}
};

class CastChaosBoltAction : public CastSpellAction
{
public:
    CastChaosBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chaos bolt") {}
};

class CastSearingPainAction : public CastSpellAction
{
public:
    CastSearingPainAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "searing pain") {}
};

// AoE Spells

class CastSeedOfCorruptionAction : public CastDebuffSpellAction
{
public:
    CastSeedOfCorruptionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "seed of corruption", true, 0) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastSeedOfCorruptionOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastSeedOfCorruptionOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnAttackerAction(botAI, "seed of corruption", true, 0)
    {
    }
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastRainOfFireAction : public CastSpellAction
{
public:
    CastRainOfFireAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "rain of fire") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastHellfireAction : public CastSpellAction
{
public:
    CastHellfireAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hellfire") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastShadowflameAction : public CastSpellAction
{
public:
    CastShadowflameAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shadowflame") {}
    bool isUseful() override;
};

class CastShadowfuryAction : public CastSpellAction
{
public:
    CastShadowfuryAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shadowfury") {}
};

class CastImmolationAuraAction : public CastSpellAction
{
public:
    CastImmolationAuraAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "immolation aura") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class ShadowCleaveAction : public CastMeleeSpellAction
{
public:
    ShadowCleaveAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "shadow cleave") {}
};
#endif
