/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAGEACTIONS_H
#define _PLAYERBOT_MAGEACTIONS_H

#include "GenericSpellActions.h"
#include "SharedDefines.h"
#include "UseItemAction.h"

class PlayerbotAI;

// Buff and Out of Combat Actions

class CastMoltenArmorAction : public CastBuffSpellAction
{
public:
    CastMoltenArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "molten armor") {}
};

class CastMageArmorAction : public CastBuffSpellAction
{
public:
    CastMageArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mage armor") {}
};

class CastIceArmorAction : public CastBuffSpellAction
{
public:
    CastIceArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ice armor") {}
};

class CastFrostArmorAction : public CastBuffSpellAction
{
public:
    CastFrostArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frost armor") {}
};

class CastArcaneIntellectAction : public CastBuffSpellAction
{
public:
    CastArcaneIntellectAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane intellect") {}
};

class CastArcaneIntellectOnPartyAction : public BuffOnPartyAction
{
public:
    CastArcaneIntellectOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "arcane intellect") {}
};

class CastFocusMagicOnPartyAction : public CastSpellAction
{
public:
    CastFocusMagicOnPartyAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "focus magic") {}
    Unit* GetTarget() override;
};

class CastSummonWaterElementalAction : public CastBuffSpellAction
{
public:
    CastSummonWaterElementalAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "summon water elemental") {}
};

// Boost Actions

class CastCombustionAction : public CastBuffSpellAction
{
public:
    CastCombustionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "combustion") {}
};

class CastArcanePowerAction : public CastBuffSpellAction
{
public:
    CastArcanePowerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane power") {}
};

class CastPresenceOfMindAction : public CastBuffSpellAction
{
public:
    CastPresenceOfMindAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "presence of mind") {}
};

class CastIcyVeinsAction : public CastBuffSpellAction
{
public:
    CastIcyVeinsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "icy veins") {}
};

class CastColdSnapAction : public CastBuffSpellAction
{
public:
    CastColdSnapAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cold snap") {}
};

// Defensive Actions

class CastFireWardAction : public CastBuffSpellAction
{
public:
    CastFireWardAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fire ward") {}
};

class CastFrostWardAction : public CastBuffSpellAction
{
public:
    CastFrostWardAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frost ward") {}
};

class CastIceBarrierAction : public CastBuffSpellAction
{
public:
    CastIceBarrierAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ice barrier") {}
};

class CastInvisibilityAction : public CastBuffSpellAction
{
public:
    CastInvisibilityAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "invisibility") {}
};
class CastIceBlockAction : public CastBuffSpellAction
{
public:
    CastIceBlockAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ice block") {}
};

class CastMirrorImageAction : public CastBuffSpellAction
{
public:
    CastMirrorImageAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mirror image") {}
};

class CastBlinkBackAction : public CastSpellAction
{
public:
    CastBlinkBackAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blink") {}
    bool Execute(Event event) override;
};

class CastManaShieldAction : public CastBuffSpellAction
{
public:
    CastManaShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mana shield") {}
};

// Utility Actions

class CastEvocationAction : public CastSpellAction
{
public:
    CastEvocationAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "evocation") {}
    std::string const GetTargetName() override { return "self target"; }
};

class CastConjureManaGemAction : public CastBuffSpellAction
{
public:
    CastConjureManaGemAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "conjure mana gem") {}
};

class CastConjureFoodAction : public CastBuffSpellAction
{
public:
    CastConjureFoodAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "conjure food") {}
};

class CastConjureWaterAction : public CastBuffSpellAction
{
public:
    CastConjureWaterAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "conjure water") {}
};

class UseManaSapphireAction : public UseItemAction
{
public:
    UseManaSapphireAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana sapphire") {}
    bool isUseful() override;
};
class UseManaEmeraldAction : public UseItemAction
{
public:
    UseManaEmeraldAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana emerald") {}
    bool isUseful() override;
};

class UseManaRubyAction : public UseItemAction
{
public:
    UseManaRubyAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana ruby") {}
    bool isUseful() override;
};

class UseManaCitrineAction : public UseItemAction
{
public:
    UseManaCitrineAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana citrine") {}
    bool isUseful() override;
};

class UseManaJadeAction : public UseItemAction
{
public:
    UseManaJadeAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana jade") {}
    bool isUseful() override;
};

class UseManaAgateAction : public UseItemAction
{
public:
    UseManaAgateAction(PlayerbotAI* botAI) : UseItemAction(botAI, "mana agate") {}
    bool isUseful() override;
};

// CC, Interrupt, and Dispel Actions

class CastPolymorphAction : public CastBuffSpellAction
{
public:
    CastPolymorphAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "polymorph") {}
    Value<Unit*>* GetTargetValue() override;
};

class CastSpellstealAction : public CastSpellAction
{
public:
    CastSpellstealAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "spellsteal") {}
};

class CastCounterspellAction : public CastSpellAction
{
public:
    CastCounterspellAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "counterspell") {}
};

class CastCounterspellOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastCounterspellOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "counterspell") {}
};

class CastFrostNovaAction : public CastSpellAction
{
public:
    CastFrostNovaAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frost nova") {}
    bool isUseful() override;
};

class CastDeepFreezeAction : public CastSpellAction
{
public:
    CastDeepFreezeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "deep freeze") {}
    bool isPossible() override { return true; }
};

class CastRemoveCurseAction : public CastCureSpellAction
{
public:
    CastRemoveCurseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "remove curse") {}
};

class CastRemoveLesserCurseAction : public CastCureSpellAction
{
public:
    CastRemoveLesserCurseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "remove lesser curse") {}
};

class CastRemoveCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastRemoveCurseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "remove curse", DISPEL_CURSE) {}
};

class CastRemoveLesserCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastRemoveLesserCurseOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "remove lesser curse", DISPEL_CURSE)
    {
    }
};

// Damage and Debuff Actions

class CastFireballAction : public CastSpellAction
{
public:
    CastFireballAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "fireball") {}
};

class CastScorchAction : public CastSpellAction
{
public:
    CastScorchAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "scorch") {}
};

class CastFireBlastAction : public CastSpellAction
{
public:
    CastFireBlastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "fire blast") {}
};

class CastArcaneBlastAction : public CastBuffSpellAction
{
public:
    CastArcaneBlastAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane blast") {}
    std::string const GetTargetName() override { return "current target"; }
};

class CastArcaneBarrageAction : public CastSpellAction
{
public:
    CastArcaneBarrageAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "arcane barrage") {}
};

class CastArcaneMissilesAction : public CastSpellAction
{
public:
    CastArcaneMissilesAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "arcane missiles") {}
};

class CastPyroblastAction : public CastSpellAction
{
public:
    CastPyroblastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "pyroblast") {}
};

class CastLivingBombAction : public CastDebuffSpellAction
{
public:
    CastLivingBombAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "living bomb", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastLivingBombOnAttackersAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastLivingBombOnAttackersAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "living bomb", true) {}
    bool isUseful() override
    {
        // Bypass TTL check
        return CastAuraSpellAction::isUseful();
    }
};

class CastFrostboltAction : public CastSpellAction
{
public:
    CastFrostboltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frostbolt") {}
};

class CastFrostfireBoltAction : public CastSpellAction
{
public:
    CastFrostfireBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "frostfire bolt") {}
};

class CastIceLanceAction : public CastSpellAction
{
public:
    CastIceLanceAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "ice lance") {}
};

class CastBlizzardAction : public CastSpellAction
{
public:
    CastBlizzardAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blizzard") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastConeOfColdAction : public CastSpellAction
{
public:
    CastConeOfColdAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "cone of cold") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastFlamestrikeAction : public CastDebuffSpellAction
{
public:
    CastFlamestrikeAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "flamestrike", true, 0.0f) {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastDragonsBreathAction : public CastSpellAction
{
public:
    CastDragonsBreathAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "dragon's breath") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastBlastWaveAction : public CastSpellAction
{
public:
    CastBlastWaveAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "blast wave") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

#endif
