/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "Chat.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "Group.h"
#include "ItemTemplate.h"
#include "ObjectDefines.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "WorldPacket.h"
#include <ctime>
#include <unordered_set>

using ai::buff::BuffBelowRefreshTarget;
using ai::buff::MakeAuraQualifierForBuff;
using ai::spell::HasSpellOrCategoryCooldown;

namespace
{
    std::unordered_set<uint32> const& GetMixedTriggerTrinketSpellIds()
    {
        static std::unordered_set<uint32> const mixedTriggerSpellIds = []()
        {
            std::unordered_set<uint32> onUseSpellIds;
            std::unordered_set<uint32> onEquipSpellIds;
            std::unordered_set<uint32> mixedSpellIds;

            auto const* itemTemplates = sObjectMgr->GetItemTemplateStore();
            if (!itemTemplates)
                return mixedSpellIds;

            auto const markSpellId = [&](int32 spellId, uint8 spellTrigger)
            {
                if (spellId <= 0)
                    return;

                if (spellTrigger == ITEM_SPELLTRIGGER_ON_USE)
                {
                    if (onEquipSpellIds.find(spellId) != onEquipSpellIds.end())
                        mixedSpellIds.insert(spellId);

                    onUseSpellIds.insert(spellId);
                }
                else if (spellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP)
                {
                    if (onUseSpellIds.find(spellId) != onUseSpellIds.end())
                        mixedSpellIds.insert(spellId);

                    onEquipSpellIds.insert(spellId);
                }
            };

            for (auto const& itr : *itemTemplates)
            {
                ItemTemplate const& proto = itr.second;
                if (proto.InventoryType != INVTYPE_TRINKET)
                    continue;

                for (uint8 spellIndex = 0; spellIndex < MAX_ITEM_PROTO_SPELLS; ++spellIndex)
                {
                    auto const& spellData = proto.Spells[spellIndex];
                    markSpellId(spellData.SpellId, spellData.SpellTrigger);
                }
            }

            return mixedSpellIds;
        }();

        return mixedTriggerSpellIds;
    }

    bool IsManaRestoreEffect(SpellEffectInfo const& effectInfo)
    {
        return (effectInfo.Effect == SPELL_EFFECT_ENERGIZE &&
                effectInfo.MiscValue == POWER_MANA) ||
               (effectInfo.Effect == SPELL_EFFECT_APPLY_AURA &&
                effectInfo.ApplyAuraName == SPELL_AURA_PERIODIC_ENERGIZE &&
                effectInfo.MiscValue == POWER_MANA);
    }

    bool IsManaEfficiencyEffect(SpellEffectInfo const& effectInfo)
    {
        return effectInfo.Effect == SPELL_EFFECT_APPLY_AURA &&
               (((effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_REGEN ||
                  effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_REGEN_PERCENT) &&
                 effectInfo.MiscValue == POWER_MANA) ||
                effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_COST_SCHOOL ||
                effectInfo.ApplyAuraName == SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT ||
                effectInfo.ApplyAuraName == SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    }

    bool IsDefensiveTankEffect(SpellEffectInfo const& effectInfo)
    {
        if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
            return false;

        uint32 const tankRatingsMask =
            (1u << CR_DEFENSE_SKILL) |
            (1u << CR_DODGE) |
            (1u << CR_PARRY) |
            (1u << CR_BLOCK) |
            (1u << CR_HIT_TAKEN_MELEE) |
            (1u << CR_HIT_TAKEN_RANGED) |
            (1u << CR_HIT_TAKEN_SPELL) |
            (1u << CR_CRIT_TAKEN_MELEE) |
            (1u << CR_CRIT_TAKEN_RANGED) |
            (1u << CR_CRIT_TAKEN_SPELL);

        switch (effectInfo.ApplyAuraName)
        {
            case SPELL_AURA_MOD_RESISTANCE:
                return (effectInfo.MiscValue & SPELL_SCHOOL_MASK_NORMAL) != 0;
            case SPELL_AURA_MOD_RATING:
                return (effectInfo.MiscValue & tankRatingsMask) != 0;
            case SPELL_AURA_MOD_INCREASE_HEALTH:
            case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
            case SPELL_AURA_MOD_PARRY_PERCENT:
            case SPELL_AURA_MOD_DODGE_PERCENT:
            case SPELL_AURA_MOD_BLOCK_PERCENT:
            case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
                return true;
            default:
                return false;
        }
    }
}

CastSpellAction::CastSpellAction(PlayerbotAI* botAI, std::string const spell)
    : Action(botAI, spell), spell(spell), range(botAI->GetRange("spell")) {}

bool CastSpellAction::Execute(Event /*event*/)
{
    if (spell == "conjure food" || spell == "conjure water")
    {
        // uint32 id = AI_VALUE2(uint32, "spell id", spell);
        // if (!id)
        //     return false;

        uint32 castId = 0;

        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            uint32 spellId = itr->first;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            std::string const namepart = spellInfo->SpellName[0];
            std::wstring wnamepart;
            if (!Utf8toWStr(namepart, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (!Utf8FitTo(spell, wnamepart) || spellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
                continue;

            uint32 itemId = spellInfo->Effects[0].ItemType;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto || bot->CanUseItem(proto) != EQUIP_ERR_OK)
                continue;

            if (spellInfo->Id > castId)
                castId = spellInfo->Id;
        }

        return botAI->CastSpell(castId, bot);
    }

    return botAI->CastSpell(spell, GetTarget());
}

bool CastSpellAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        bot->Dismount();
        return false;
    }

    Unit* spellTarget = GetTarget();
    if (!spellTarget || !spellTarget->IsInWorld() || spellTarget->GetMapId() != bot->GetMapId())
        return false;

    // float combatReach = bot->GetCombatReach() + target->GetCombatReach();
    // if (!PlayerbotAI::IsRanged(bot))
    //     combatReach += 4.0f / 3.0f;

    return AI_VALUE2(bool, "spell cast useful", spell);
           // && ServerFacade::instance().GetDistance2d(bot, target) <= (range + combatReach);
}

bool CastSpellAction::isPossible()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
    {
        if (!sPlayerbotAIConfig.LogInGroupOnly || (bot->GetGroup() && botAI->HasGameClientMaster()))
            LOG_DEBUG("playerbots", "Can cast spell failed. Vehicle. - bot name: {}", bot->GetName());

        return false;
    }

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        if (!sPlayerbotAIConfig.LogInGroupOnly || (bot->GetGroup() && botAI->HasGameClientMaster()))
            LOG_DEBUG("playerbots", "Can cast spell failed. Mount. - bot name: {}", bot->GetName());

        bot->Dismount();
        return false;
    }

    // Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL); //not used, line marked for removal.
    return botAI->CanCastSpell(spell, GetTarget());
}

CastMeleeSpellAction::CastMeleeSpellAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = ATTACK_DISTANCE;
}

bool CastMeleeSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !bot->IsWithinMeleeRange(target))
        return false;

    return CastSpellAction::isUseful();
}

CastMeleeDebuffSpellAction::CastMeleeDebuffSpellAction(
    PlayerbotAI* botAI, std::string const spell, bool isOwner, float needLifeTime) :
    CastDebuffSpellAction(botAI, spell, isOwner, needLifeTime)
{
    range = ATTACK_DISTANCE;
}

bool CastMeleeDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !bot->IsWithinMeleeRange(target))
        return false;

    return CastDebuffSpellAction::isUseful();
}

bool CastAuraSpellAction::isUseful()
{
    if (!GetTarget() || !CastSpellAction::isUseful())
        return false;

    Aura* aura = botAI->GetAura(spell, GetTarget(), isOwner, checkDuration);
    return BuffBelowRefreshTarget(botAI, aura, beforeDuration);
}

bool CastBuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !CastSpellAction::isUseful())
        return false;

    Aura* aura = botAI->GetAura(spell, target, isOwner, checkDuration);
    return BuffBelowRefreshTarget(botAI, aura, beforeDuration);
}

bool CastBuffSpellAction::Execute(Event /*event*/)
{
    botAI->forceRebuff.NoteBuffWork();

    return botAI->CastSpell(spell, GetTarget());
}

bool GroupBuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !CastSpellAction::isUseful())
        return false;

    if (ai::buff::IsGroupVariantEnabled(bot, spell))
    {
        std::string const groupVariant = ai::buff::GroupVariantFor(spell);
        if (!groupVariant.empty() && !BuffBelowRefreshTarget(
                botAI, botAI->GetAura(groupVariant, target, isOwner, checkDuration), beforeDuration))
            return false;
    }

    Aura* aura = botAI->GetAura(spell, target, isOwner, checkDuration);
    return BuffBelowRefreshTarget(botAI, aura, beforeDuration);
}

bool GroupBuffSpellAction::Execute(Event /*event*/)
{
    botAI->forceRebuff.NoteBuffWork();

    std::string missingReagentGroupName;
    std::string const castName = ai::buff::UpgradeToGroupIfAppropriate(
        bot, botAI, spell, &missingReagentGroupName);

    if (!botAI->CastSpell(castName, GetTarget()))
        return false;

    if (!missingReagentGroupName.empty())
        ai::buff::TryAnnounceMissingBuffReagents(botAI, spell, missingReagentGroupName);

    return true;
}

CastEnchantItemMainHandAction::CastEnchantItemMainHandAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

bool CastEnchantItemMainHandAction::Execute(Event /*event*/)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return item && botAI->CastSpell(spell, bot, item);
}

bool CastEnchantItemMainHandAction::isPossible()
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!item || item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_MISC ||
        item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE ||
        item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
    {
        return false;
    }

    return botAI->CanCastSpell(spell, bot, item);
}

CastEnchantItemOffHandAction::CastEnchantItemOffHandAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

bool CastEnchantItemOffHandAction::Execute(Event /*event*/)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    return item && botAI->CastSpell(spell, bot, item);
}

bool CastEnchantItemOffHandAction::isPossible()
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!item || item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_MISC ||
        item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
    {
        return false;
    }

    return botAI->CanCastSpell(spell, bot, item);
}

CastHealingSpellAction::CastHealingSpellAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount,
                                               HealingManaEfficiency manaEfficiency, bool isOwner)
    : CastAuraSpellAction(botAI, spell, isOwner), manaEfficiency(manaEfficiency), estAmount(estAmount)
{
    range = botAI->GetRange("heal");
}

bool CastHealingSpellAction::isUseful() { return CastAuraSpellAction::isUseful(); }

bool CastAoeHealSpellAction::isUseful() { return CastSpellAction::isUseful(); }

CastCureSpellAction::CastCureSpellAction(
    PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = botAI->GetRange("heal");
}

Value<Unit*>* CurePartyMemberAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member to dispel", dispelType);
}

Value<Unit*>* BuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

Value<Unit*>* GroupBuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", MakeAuraQualifierForBuff(spell));
}

CastShootAction::CastShootAction(
    PlayerbotAI* botAI) : CastSpellAction(botAI, "shoot"), shootSpellId(0)
{
    if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        spell = "shoot";

        switch (pItem->GetTemplate()->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_GUN:
                spell += " gun";
                shootSpellId = 3018;
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
                spell += " bow";
                shootSpellId = 3018;
                break;
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                spell += " crossbow";
                shootSpellId = 3018;
                break;
            case ITEM_SUBCLASS_WEAPON_THROWN:
                spell = "throw";
                shootSpellId = 2764;
                break;
        }
    }
}

bool CastShootAction::isPossible()
{
    if (shootSpellId)
        return botAI->CanCastSpell(shootSpellId, GetTarget(), false);

    return CastSpellAction::isPossible();
}

bool CastShootAction::Execute(Event /*event*/)
{
    if (shootSpellId)
        return botAI->CastSpell(shootSpellId, GetTarget());

    return botAI->CastSpell(spell, GetTarget());
}

Value<Unit*>* CastDebuffSpellOnAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("attacker without aura", spell);
}

Value<Unit*>* CastDebuffSpellOnMeleeAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("melee attacker without aura", spell);
}

CastBuffSpellAction::CastBuffSpellAction(
    PlayerbotAI* botAI, std::string const spell, bool checkIsOwner, uint32 beforeDuration)
    : CastAuraSpellAction(botAI, spell, checkIsOwner, false, beforeDuration)
{
    range = botAI->GetRange("spell");
}

Value<Unit*>* CastSpellOnEnemyHealerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

Value<Unit*>* CastSnareSpellAction::GetTargetValue() { return context->GetValue<Unit*>("snare target", spell); }

Value<Unit*>* CastCrowdControlSpellAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool CastCrowdControlSpellAction::Execute(Event /*event*/) { return botAI->CastSpell(getName(), GetTarget()); }

bool CastCrowdControlSpellAction::isPossible() { return botAI->CanCastSpell(getName(), GetTarget()); }

bool CastCrowdControlSpellAction::isUseful() { return true; }

std::string const CastProtectSpellAction::GetTargetName() { return "party member to protect"; }

bool CastProtectSpellAction::isUseful() { return GetTarget() && !botAI->HasAura(spell, GetTarget()); }

bool CastVehicleSpellAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CanCastVehicleSpell(spellId, GetTarget());
}

bool CastVehicleSpellAction::isUseful() { return botAI->IsInVehicle(false, true); }

bool CastVehicleSpellAction::Execute(Event /*event*/)
{
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CastVehicleSpell(spellId, GetTarget());
}

bool CastEveryManForHimselfAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && !HasSpellOrCategoryCooldown(bot, spellId);
}

bool CastWillOfTheForsakenAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && !HasSpellOrCategoryCooldown(bot, spellId);
}

bool UseTrinketAction::Execute(Event /*event*/)
{
    Item* trinket1 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET1);

    if (trinket1 && UseTrinket(trinket1))
        return true;

    Item* trinket2 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET2);
    if (trinket2 && UseTrinket(trinket2))
        return true;

    return false;
}

bool UseTrinketAction::UseTrinket(Item* item)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK || bot->IsNonMeleeSpellCast(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 targetFlag = TARGET_FLAG_NONE;
    uint32 spellId = 0;
    int32 itemSpellCooldown = 0;
    uint32 itemSpellCategory = 0;
    int32 itemSpellCategoryCooldown = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0 &&
            item->GetTemplate()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            itemSpellCooldown = item->GetTemplate()->Spells[i].SpellCooldown;
            itemSpellCategory = item->GetTemplate()->Spells[i].SpellCategory;
            itemSpellCategoryCooldown = item->GetTemplate()->Spells[i].SpellCategoryCooldown;
            uint64 const itemCooldownKey = (static_cast<uint64>(item->GetEntry()) << 32) | spellId;
            uint32 const now = getMSTime();

            if (itemSpellCooldown > 0)
            {
                auto const itemCooldownItr = trinketItemCooldownExpiries.find(itemCooldownKey);
                if (itemCooldownItr != trinketItemCooldownExpiries.end())
                {
                    if (itemCooldownItr->second > now)
                        return false;

                    trinketItemCooldownExpiries.erase(itemCooldownItr);
                }
            }

            if (itemSpellCategory && itemSpellCategoryCooldown > 0)
            {
                auto const categoryCooldownItr = trinketCategoryCooldownExpiries.find(itemSpellCategory);
                if (categoryCooldownItr != trinketCategoryCooldownExpiries.end())
                {
                    if (categoryCooldownItr->second > now)
                        return false;

                    trinketCategoryCooldownExpiries.erase(categoryCooldownItr);
                }
            }

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo || !spellInfo->IsPositive())
                return false;

            bool applyAura = false;
            bool restoresMana = false;
            bool improvesManaEfficiency = false;
            bool defensiveTankEffect = false;
            for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
            {
                SpellEffectInfo const& effectInfo = spellInfo->Effects[i];
                if (effectInfo.Effect == SPELL_EFFECT_APPLY_AURA)
                    applyAura = true;

                restoresMana = restoresMana || IsManaRestoreEffect(effectInfo);
                improvesManaEfficiency = improvesManaEfficiency || IsManaEfficiencyEffect(effectInfo);
                defensiveTankEffect = defensiveTankEffect || IsDefensiveTankEffect(effectInfo);
            }

            if (!applyAura && !restoresMana)
                return false;

            if (restoresMana || improvesManaEfficiency)
            {
                if (!AI_VALUE2(bool, "has mana", "self target"))
                    return false;

                uint8 const manaPct = AI_VALUE2(uint8, "mana", "self target");
                if ((restoresMana && manaPct >= sPlayerbotAIConfig.MediumMana) ||
                    manaPct >= sPlayerbotAIConfig.HighMana)
                {
                    return false;
                }
            }

            if (defensiveTankEffect)
            {
                uint8 const healthPct = AI_VALUE2(uint8, "health", "self target");
                if (healthPct > sPlayerbotAIConfig.LowHealth)
                    return false;
            }

            auto const& mixedTriggerTrinketSpellIds = GetMixedTriggerTrinketSpellIds();
            // Exclude trinkets that expose the same spell as both ON_EQUIP and ON_USE across
            // item templates. Those are equip/proc effects leaking into the active-use path,
            // as seen with the error versions of Oracle Talisman of Ablution (44870) and
            // Frenzyheart Insignia of Fury (44869).
            if (mixedTriggerTrinketSpellIds.find(spellId) != mixedTriggerTrinketSpellIds.end())
                return false;

            if (!botAI->CanCastSpell(spellId, bot, false, nullptr, item))
                return false;

            break;
        }
    }

    if (!spellId)
        return false;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    targetFlag = TARGET_FLAG_NONE;
    packet << targetFlag << bot->GetPackGUID();

    // HandleUseItemOpcode can destroy the trinket on its last charge.
    uint32 const itemId = item->GetEntry();
    bot->GetSession()->HandleUseItemOpcode(packet);

    uint32 const now = getMSTime();
    uint32 const cooldownDelay = bot->GetSpellCooldownDelay(spellId);
    if (cooldownDelay > 0)
    {
        if (itemSpellCooldown > 0)
        {
            uint64 const itemCooldownKey = (static_cast<uint64>(itemId) << 32) | spellId;
            trinketItemCooldownExpiries[itemCooldownKey] = now + static_cast<uint32>(itemSpellCooldown);
        }

        if (itemSpellCategory && itemSpellCategoryCooldown > 0)
        {
            trinketCategoryCooldownExpiries[itemSpellCategory] = now + static_cast<uint32>(itemSpellCategoryCooldown);
        }
    }

    return true;
}

Value<Unit*>* BuffOnMainTankAction::GetTargetValue() { return context->GetValue<Unit*>("main tank", spell); }

bool CastDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return CastAuraSpellAction::isUseful() &&
           (target->GetHealth() / AI_VALUE(float, "estimated group dps")) >= needLifeTime;
}
