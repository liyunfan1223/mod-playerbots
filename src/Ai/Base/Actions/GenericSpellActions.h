/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICSPELLACTIONS_H
#define PLAYERBOTS_GENERICSPELLACTIONS_H

#include "Action.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "UseItemAction.h"
#include "Value.h"

class PlayerbotAI;
class Unit;
class WorldObject;

class CastSpellAction : public Action
{
public:
    CastSpellAction(PlayerbotAI* botAI, std::string const spell);

    std::string const GetTargetName() override { return "current target"; };
    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
    ActionThreatType getThreatType() override { return ActionThreatType::Single; }

    std::vector<NextAction> getPrerequisites() override
    {
        return {};
    }

    std::string const getSpell() { return spell; }

protected:
    std::string spell;
    float range;
};

class CastAuraSpellAction : public CastSpellAction
{
public:
    CastAuraSpellAction(PlayerbotAI* botAI, std::string const spell, bool isOwner = false, bool checkDuration = false, uint32 beforeDuration = 0)
        : CastSpellAction(botAI, spell)
    {
        this->isOwner = isOwner;
        this->beforeDuration = beforeDuration;
        this->checkDuration = checkDuration;
    }

    bool isUseful() override;

protected:
    bool isOwner;
    bool checkDuration;
    uint32 beforeDuration;
};

class CastMeleeSpellAction : public CastSpellAction
{
public:
    CastMeleeSpellAction(PlayerbotAI* botAI, std::string const spell);
    bool isUseful() override;
};

class CastDebuffSpellAction : public CastAuraSpellAction
{
public:
    CastDebuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool isOwner = false, float needLifeTime = 8.0f)
        : CastAuraSpellAction(botAI, spell, isOwner), needLifeTime(needLifeTime) {}
    bool isUseful() override;

private:
    float needLifeTime;
};

class CastMeleeDebuffSpellAction : public CastDebuffSpellAction
{
public:
    CastMeleeDebuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool isOwner = false, float needLifeTime = 8.0f);
    bool isUseful() override;
};

class CastDebuffSpellOnAttackerAction : public CastDebuffSpellAction
{
public:
    CastDebuffSpellOnAttackerAction(PlayerbotAI* botAI, std::string const spell, bool isOwner = true,
                                    float needLifeTime = 8.0f)
        : CastDebuffSpellAction(botAI, spell, isOwner, needLifeTime) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on attacker"; }
    // ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastDebuffSpellOnMeleeAttackerAction : public CastDebuffSpellAction
{
public:
    CastDebuffSpellOnMeleeAttackerAction(PlayerbotAI* botAI, std::string const spell, bool isOwner = true,
                                         float needLifeTime = 8.0f)
        : CastDebuffSpellAction(botAI, spell, isOwner, needLifeTime) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on attacker"; }
    // ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastBuffSpellAction : public CastAuraSpellAction
{
public:
    CastBuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool checkIsOwner = false, uint32 beforeDuration = 0);

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
    bool Execute(Event event) override;
};

class GroupBuffSpellAction : public CastBuffSpellAction
{
public:
    GroupBuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool checkIsOwner = false,
                         uint32 beforeDuration = 0)
        : CastBuffSpellAction(botAI, spell, checkIsOwner, beforeDuration) {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class CastEnchantItemMainHandAction : public CastSpellAction
{
public:
    CastEnchantItemMainHandAction(PlayerbotAI* botAI, std::string const spell);
    std::string const GetTargetName() override { return "self target"; }
    bool Execute(Event event) override;
    bool isPossible() override;
};

class CastEnchantItemOffHandAction : public CastSpellAction
{
public:
    CastEnchantItemOffHandAction(PlayerbotAI* botAI, std::string const spell);
    std::string const GetTargetName() override { return "self target"; }
    bool Execute(Event event) override;
    bool isPossible() override;
};

class CastHealingSpellAction : public CastAuraSpellAction
{
public:
    CastHealingSpellAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount = 15.0f,
                           HealingManaEfficiency manaEfficiency = HealingManaEfficiency::MEDIUM, bool isOwner = true);

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    // Yunfan: Mana efficiency tell the bot how to save mana. The higher the better.
    HealingManaEfficiency manaEfficiency;
    uint8 estAmount;
};

class CastAoeHealSpellAction : public CastHealingSpellAction
{
public:
    CastAoeHealSpellAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount = 15.0f,
                           HealingManaEfficiency manaEfficiency = HealingManaEfficiency::MEDIUM)
        : CastHealingSpellAction(botAI, spell, estAmount, manaEfficiency)
    {
    }

    std::string const GetTargetName() override { return "party member to heal"; }
    bool isUseful() override;
};

class CastCureSpellAction : public CastSpellAction
{
public:
    CastCureSpellAction(PlayerbotAI* botAI, std::string const spell);

    std::string const GetTargetName() override { return "self target"; }
};

class PartyMemberActionNameSupport
{
public:
    PartyMemberActionNameSupport(std::string const spell) { name = std::string(spell + " on party"); }

    std::string const getName() { return name; }

private:
    std::string name;
};

class HealPartyMemberAction : public CastHealingSpellAction, public PartyMemberActionNameSupport
{
public:
    HealPartyMemberAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount = 15.0f,
                          HealingManaEfficiency manaEfficiency = HealingManaEfficiency::MEDIUM, bool isOwner = true)
        : CastHealingSpellAction(botAI, spell, estAmount, manaEfficiency, isOwner), PartyMemberActionNameSupport(spell) {}

    std::string const GetTargetName() override { return "party member to heal"; }
    std::string const getName() override { return PartyMemberActionNameSupport::getName(); }
};

class ResurrectPartyMemberAction : public CastSpellAction
{
public:
    ResurrectPartyMemberAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

    std::string const GetTargetName() override { return "party member to resurrect"; }
    std::vector<NextAction> getPrerequisites() override
    {
        return NextAction::merge(
            { NextAction("reach party member to resurrect") },
            Action::getPrerequisites()
        );
    }
};

class CurePartyMemberAction : public CastSpellAction, public PartyMemberActionNameSupport
{
public:
    CurePartyMemberAction(PlayerbotAI* botAI, std::string const spell, uint32 dispelType)
        : CastSpellAction(botAI, spell), PartyMemberActionNameSupport(spell), dispelType(dispelType) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return PartyMemberActionNameSupport::getName(); }

protected:
    uint32 dispelType;
};

class BuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
{
public:
    BuffOnPartyAction(PlayerbotAI* botAI, std::string const spell)
        : CastBuffSpellAction(botAI, spell), PartyMemberActionNameSupport(spell) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return PartyMemberActionNameSupport::getName(); }
};

class GroupBuffOnPartyAction : public GroupBuffSpellAction, public PartyMemberActionNameSupport
{
public:
    GroupBuffOnPartyAction(PlayerbotAI* botAI, std::string const spell)
        : GroupBuffSpellAction(botAI, spell), PartyMemberActionNameSupport(spell) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return PartyMemberActionNameSupport::getName(); }
};

class CastShootAction : public CastSpellAction
{
public:
    CastShootAction(PlayerbotAI* botAI);

    bool isPossible() override;
    bool Execute(Event event) override;
    ActionThreatType getThreatType() override { return ActionThreatType::None; }

private:
    uint32 shootSpellId;
};

class CastLifeBloodAction : public CastHealingSpellAction
{
public:
    CastLifeBloodAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lifeblood") {}
};

class CastGiftOfTheNaaruAction : public CastHealingSpellAction
{
public:
    CastGiftOfTheNaaruAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "gift of the naaru") {}
};

class CastArcaneTorrentAction : public CastBuffSpellAction
{
public:
    CastArcaneTorrentAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "arcane torrent") {}
};

class CastWarStompAction : public CastMeleeSpellAction
{
public:
    CastWarStompAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "war stomp") {}
};

class CastBloodFuryAction : public CastBuffSpellAction
{
public:
    CastBloodFuryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blood fury") {}
};

class CastBerserkingAction : public CastBuffSpellAction
{
public:
    CastBerserkingAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "berserking") {}
};

class CastEveryManForHimselfAction : public CastSpellAction
{
public:
    CastEveryManForHimselfAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "every man for himself") {}

    std::string const GetTargetName() override { return "self target"; }
    bool isPossible() override;
};

class CastWillOfTheForsakenAction : public CastSpellAction
{
public:
    CastWillOfTheForsakenAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "will of the forsaken") {}

    std::string const GetTargetName() override { return "self target"; }
    bool isPossible() override;
};

class CastStoneformAction : public CastSpellAction
{
public:
    CastStoneformAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "stoneform") {}

    std::string const GetTargetName() override { return "self target"; }
};

class CastEscapeArtistAction : public CastSpellAction
{
public:
    CastEscapeArtistAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "escape artist") {}

    std::string const GetTargetName() override { return "self target"; }
};

class UseTrinketAction : public Action
{
public:
    UseTrinketAction(PlayerbotAI* botAI) : Action(botAI, "use trinket") {}
    bool Execute(Event event) override;

protected:
    bool UseTrinket(Item* trinket);

private:
    std::unordered_map<uint64, uint32> trinketItemCooldownExpiries;
    std::unordered_map<uint32, uint32> trinketCategoryCooldownExpiries;
};

class CastSpellOnEnemyHealerAction : public CastSpellAction
{
public:
    CastSpellOnEnemyHealerAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on enemy healer"; }
};

class CastSnareSpellAction : public CastDebuffSpellAction
{
public:
    CastSnareSpellAction(PlayerbotAI* botAI, std::string const spell) : CastDebuffSpellAction(botAI, spell) {}

    Value<Unit*>* GetTargetValue() override;
    std::string const getName() override { return spell + " on snare target"; }
    ActionThreatType getThreatType() override { return ActionThreatType::None; }
};

class CastCrowdControlSpellAction : public CastBuffSpellAction
{
public:
    CastCrowdControlSpellAction(PlayerbotAI* botAI, std::string const spell) : CastBuffSpellAction(botAI, spell) {}

    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
    bool isPossible() override;
    bool isUseful() override;
    ActionThreatType getThreatType() override { return ActionThreatType::None; }
};

class CastProtectSpellAction : public CastSpellAction
{
public:
    CastProtectSpellAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

    std::string const GetTargetName() override;
    bool isUseful() override;
    ActionThreatType getThreatType() override { return ActionThreatType::None; }
};

class CastVehicleSpellAction : public CastSpellAction
{
public:
    CastVehicleSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell)
    {
        range = 120.0f;
    }

    std::string const GetTargetName() override { return "current target"; }
    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
    ActionThreatType getThreatType() override { return ActionThreatType::None; }

protected:
    WorldObject* spellTarget;
};

class CastHurlBoulderAction : public CastVehicleSpellAction
{
public:
    CastHurlBoulderAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "hurl boulder") {}
};

class CastSteamRushAction : public CastVehicleSpellAction
{
public:
    CastSteamRushAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "steam rush") {}
};

class CastRamAction : public CastVehicleSpellAction
{
public:
    CastRamAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "ram") {}
};

class CastNapalmAction : public CastVehicleSpellAction
{
public:
    CastNapalmAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "napalm") {}
};

class CastFireCannonAction : public CastVehicleSpellAction
{
public:
    CastFireCannonAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "fire cannon") {}
};

class CastSteamBlastAction : public CastVehicleSpellAction
{
public:
    CastSteamBlastAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "steam blast") {}
};

class CastIncendiaryRocketAction : public CastVehicleSpellAction
{
public:
    CastIncendiaryRocketAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "incendiary rocket") {}
};

class CastRocketBlastAction : public CastVehicleSpellAction
{
public:
    CastRocketBlastAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "rocket blast") {}
};

class CastGlaiveThrowAction : public CastVehicleSpellAction
{
public:
    CastGlaiveThrowAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "glaive throw") {}
};

class CastBladeSalvoAction : public CastVehicleSpellAction
{
public:
    CastBladeSalvoAction(PlayerbotAI* botAI) : CastVehicleSpellAction(botAI, "blade salvo") {}
};

class MainTankActionNameSupport
{
public:
    MainTankActionNameSupport(std::string spell) { name = std::string(spell) + " on main tank"; }

    virtual std::string const getName() { return name; }

private:
    std::string name;
};

class BuffOnMainTankAction : public CastBuffSpellAction, public MainTankActionNameSupport
{
public:
    BuffOnMainTankAction(PlayerbotAI* botAI, std::string spell, bool checkIsOwner = false)
        : CastBuffSpellAction(botAI, spell, checkIsOwner), MainTankActionNameSupport(spell) {}

public:
    virtual Value<Unit*>* GetTargetValue();
    virtual std::string const getName() { return MainTankActionNameSupport::getName(); }
};

#endif
