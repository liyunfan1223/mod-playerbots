/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LootRollAction.h"

#include "Event.h"
#include "Group.h"
#include "ItemUsageValue.h"
#include "LootAction.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Player.h"
#include "StatsWeightCalculator.h"
#include <algorithm>
#include <string>
#include <sstream>
#include <cctype>
#include "SharedDefines.h"
#include "AiFactory.h"
#include <initializer_list>
#include <vector>

// Local helper: not a class member
static bool HasAnyStat(ItemTemplate const* proto,
                       std::initializer_list<ItemModType> mods)
{
    if (!proto) return false;
    for (uint32 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
    {
        if (!proto->ItemStat[i].ItemStatValue) continue;
        ItemModType t = ItemModType(proto->ItemStat[i].ItemStatType);
        for (auto m : mods)
            if (t == m) return true;
    }
    return false;
}

static bool IsPrimaryForSpec(Player* bot, ItemTemplate const* proto)
{
    if (!bot || !proto) return false;

    // Jewelry/cloaks: focus mainly on the stat profile (stat set)
    const bool isJewelry =
        proto->InventoryType == INVTYPE_TRINKET ||
        proto->InventoryType == INVTYPE_FINGER  ||
        proto->InventoryType == INVTYPE_NECK    ||
        proto->InventoryType == INVTYPE_CLOAK;

    const std::string spec = AiFactory::GetPlayerSpecName(bot);
    const uint8 cls = bot->getClass();

    auto specIs = [&](char const* s){ return spec == s; };
    auto specIn = [&](std::initializer_list<char const*> names){
        for (auto n : names) if (spec == n) return true;
        return false;
    };

    // Flags class/spec
    const bool isPureCasterClass = (cls == CLASS_MAGE || cls == CLASS_WARLOCK || cls == CLASS_PRIEST);
    const bool isHolyPal = (cls == CLASS_PALADIN && spec == "holy");
    const bool isProtPal = (cls == CLASS_PALADIN && (specIs("prot") || specIs("protection")));
    const bool isRetPal  = (cls == CLASS_PALADIN && (specIs("retrib") || specIs("ret") || specIs("retribution")));

    const bool isDK      = (cls == CLASS_DEATH_KNIGHT);
    const bool isDKBlood = (isDK && specIs("blood"));
    const bool isDKFrost = (isDK && specIs("frost"));
    const bool isDKUH    = (isDK && (specIs("unholy") || specIs("uh")));
    const bool isDKTank  = (isDKBlood || isDKFrost) && !isDKUH; // DK tank heuristic (Blood/Frost)

    const bool isWarrior = (cls == CLASS_WARRIOR);
    const bool isWarProt = (isWarrior && (specIs("prot") || specIs("protection")));

    const bool isHunter  = (cls == CLASS_HUNTER); // // BM/MM/SV -> physical DPS
    const bool isRogue   = (cls == CLASS_ROGUE);  // Assassination/Combat/Subtlety -> physical DPS

    const bool isShaman  = (cls == CLASS_SHAMAN);
    const bool isEleSham = (isShaman && specIs("elemental"));
    const bool isRestoSh = (isShaman && (specIs("resto") || specIs("restoration")));
    const bool isEnhSham = (isShaman && (specIs("enhance") || specIs("enhancement")));

    const bool isDruid   = (cls == CLASS_DRUID);
    const bool isBalance = (isDruid && specIs("balance"));
    const bool isRestoDr = (isDruid && (specIs("resto") || specIs("restoration")));
    const bool isFeralTk = (isDruid && (specIs("feraltank") || specIs("bear")));
    const bool isFeralDps= (isDruid && (specIs("feraldps")  || specIs("cat")  || specIs("kitty")));

    const bool isCasterSpec   = isPureCasterClass || isHolyPal || isEleSham || isRestoSh || isBalance || isRestoDr;
    const bool isTankLikeSpec = isProtPal || isWarProt || isFeralTk || isDKTank;
    const bool isPhysicalSpec = !isCasterSpec; // Everything that is not a caster -> physical profiles (DK/War/Rogue/Hunter/Ret/Enh/Feral/Prot…)

    // Loot Stats
    const bool hasINT   = HasAnyStat(proto, { ITEM_MOD_INTELLECT });
    const bool hasSPI   = HasAnyStat(proto, { ITEM_MOD_SPIRIT });
    const bool hasMP5   = HasAnyStat(proto, { ITEM_MOD_MANA_REGENERATION });
    const bool hasSP    = HasAnyStat(proto, { ITEM_MOD_SPELL_POWER });
    const bool hasSTR   = HasAnyStat(proto, { ITEM_MOD_STRENGTH });
    const bool hasAGI   = HasAnyStat(proto, { ITEM_MOD_AGILITY });
    const bool hasSTA   = HasAnyStat(proto, { ITEM_MOD_STAMINA });
    const bool hasAP    = HasAnyStat(proto, { ITEM_MOD_ATTACK_POWER, ITEM_MOD_RANGED_ATTACK_POWER });
    const bool hasARP   = HasAnyStat(proto, { ITEM_MOD_ARMOR_PENETRATION_RATING });
    const bool hasEXP   = HasAnyStat(proto, { ITEM_MOD_EXPERTISE_RATING });
    const bool hasHIT   = HasAnyStat(proto, { ITEM_MOD_HIT_RATING });
    const bool hasHASTE = HasAnyStat(proto, { ITEM_MOD_HASTE_RATING });
    const bool hasCRIT  = HasAnyStat(proto, { ITEM_MOD_CRIT_RATING });
    const bool hasDef   = HasAnyStat(proto, { ITEM_MOD_DEFENSE_SKILL_RATING });
    const bool hasAvoid = HasAnyStat(proto, { ITEM_MOD_DODGE_RATING, ITEM_MOD_PARRY_RATING, ITEM_MOD_BLOCK_RATING });

    /// Quick profiles
    const bool looksCaster   = hasSP || hasSPI || hasMP5 || (hasINT && !hasSTR && !hasAGI && !hasAP);
    const bool looksPhysical = hasSTR || hasAGI || hasAP || hasARP || hasEXP;
    const bool hasDpsRatings = hasHIT || hasHASTE || hasCRIT; // Common to all DPS (physical & casters)

    // Generic rules by role/family
    if (isPhysicalSpec)
    {
        // (1) All physicals/tanks: never Spell Power/Spirit/MP5 (even if plate/mail)
        if (looksCaster)
            return false;
        // (2) Weapon/shield with Spell Power: always off-spec for DK/War/Rogue/Hunter/Ret/Enh/Feral/Prot
        if ((proto->Class == ITEM_CLASS_WEAPON || (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD))
            && hasSP)
            return false;
        // (3) Jewelry/cloaks with caster stats (SP/SPI/MP5/pure INT) -> off-spec
        if (isJewelry && looksCaster)
            return false;
    }
    else // Caster/Healer
    {
        // (1) Casters/healers should not NEED pure melee items (STR/AP/ARP/EXP) without INT/SP
        if (looksPhysical && !hasSP && !hasINT)
            return false;
        // (2) Melee jewelry (AP/ARP/EXP/STR/AGI) without INT/SP -> off-spec
        if (isJewelry && looksPhysical && !hasSP && !hasINT)
            return false;
        // Paladin Holy (plate INT+SP/MP5), Shaman Elemental/Restoration (mail INT+SP/MP5),
        // Druid Balance/Restoration (leather/cloth caster) → OK
    }

    // Class/spec specific adjustments (readable)
    // DK Unholy (DPS): allows STR/HIT/HASTE/CRIT/ARP; rejects all caster items
    if (isDKUH)
    {
        if (looksCaster) return false;
    }
    // DK Blood/Frost tanks: DEF/AVOID/STA/STR are useful; reject caster items
    if (isDKTank)
    {
        if (looksCaster) return false;
        // Pure caster DPS rings/trinkets already filtered above.
    }
    // Hunter (BM/MM/SV): agi/hit/haste/AP/crit/arp → OK; avoid STR-only or caster items
    if (isHunter)
    {
        if (looksCaster) return false;
        // Avoid rings with "pure STR" without AGI/AP/DPS ratings
        if (isJewelry && hasSTR && !hasAGI && !hasAP && !hasDpsRatings)
            return false;
    }
    // Rogue (all specs): same strict physical filter (no caster items)
    if (isRogue)
    {
        if (looksCaster) return false;
    }
    // Warrior Arms/Fury : no caster items
    if (isWarrior && !isWarProt)
    {
        if (looksCaster) return false;
    }
    // Warrior Protection: DEF/AVOID/STA/STR are useful; no caster items
    if (isWarProt)
    {
        if (looksCaster) return false;
    }
    // Shaman Enhancement: no Spell Power weapons/shields, no pure INT/SP items
    if (isEnhSham)
    {
        if (looksCaster) return false;
        if ((proto->Class == ITEM_CLASS_WEAPON || (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD))
            && hasSP)
            return false;
    }
    // Druid Feral (tank/DPS): AGI/STA/AVOID/ARP/EXP → OK; no caster items
    if (isFeralTk || isFeralDps)
    {
        if (looksCaster) return false;
    }

    // Let the cross-armor rules (CrossArmorExtraMargin) decide for major off-armor upgrades.
    return true;
}


// Local mini-helper: maps an InventoryType (INVTYPE_*) to an EquipmentSlot (EQUIPMENT_SLOT_*)
// Only covers the slots relevant for T7-T10 tokens (head/shoulders/chest/hands/legs).
static uint8 EquipmentSlotByInvTypeSafe(uint8 invType)
{
    switch (invType)
    {
        case INVTYPE_HEAD:       return EQUIPMENT_SLOT_HEAD;
        case INVTYPE_SHOULDERS:  return EQUIPMENT_SLOT_SHOULDERS;
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:       return EQUIPMENT_SLOT_CHEST;
        case INVTYPE_HANDS:      return EQUIPMENT_SLOT_HANDS;
        case INVTYPE_LEGS:       return EQUIPMENT_SLOT_LEGS;
        default:                 return EQUIPMENT_SLOT_END; // inconnu/nonnull
    }
}

// All equippable items -> corresponding slots
static void GetEquipSlotsForInvType(uint8 invType, std::vector<uint8>& out)
{
    out.clear();
    switch (invType)
    {
        case INVTYPE_HEAD:         out = {EQUIPMENT_SLOT_HEAD}; break;
        case INVTYPE_NECK:         out = {EQUIPMENT_SLOT_NECK}; break;
        case INVTYPE_SHOULDERS:    out = {EQUIPMENT_SLOT_SHOULDERS}; break;
        case INVTYPE_BODY:         /* shirt, ignore */ break;
        case INVTYPE_CHEST:
        case INVTYPE_ROBE:         out = {EQUIPMENT_SLOT_CHEST}; break;
        case INVTYPE_WAIST:        out = {EQUIPMENT_SLOT_WAIST}; break;
        case INVTYPE_LEGS:         out = {EQUIPMENT_SLOT_LEGS}; break;
        case INVTYPE_FEET:         out = {EQUIPMENT_SLOT_FEET}; break;
        case INVTYPE_WRISTS:       out = {EQUIPMENT_SLOT_WRISTS}; break;
        case INVTYPE_HANDS:        out = {EQUIPMENT_SLOT_HANDS}; break;
        case INVTYPE_FINGER:       out = {EQUIPMENT_SLOT_FINGER1,  EQUIPMENT_SLOT_FINGER2}; break;
        case INVTYPE_TRINKET:      out = {EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2}; break;
        case INVTYPE_CLOAK:        out = {EQUIPMENT_SLOT_BACK}; break;
        case INVTYPE_WEAPON:       out = {EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND}; break;
        case INVTYPE_2HWEAPON:     out = {EQUIPMENT_SLOT_MAINHAND}; break;
        case INVTYPE_SHIELD:       out = {EQUIPMENT_SLOT_OFFHAND}; break;
        case INVTYPE_WEAPONMAINHAND: out = {EQUIPMENT_SLOT_MAINHAND}; break;
        case INVTYPE_WEAPONOFFHAND:  out = {EQUIPMENT_SLOT_OFFHAND}; break;
        case INVTYPE_HOLDABLE:     out = {EQUIPMENT_SLOT_OFFHAND}; break;  // tome/orb
        case INVTYPE_RANGED:
        case INVTYPE_THROWN:
        case INVTYPE_RANGEDRIGHT:  out = {EQUIPMENT_SLOT_RANGED}; break;
        case INVTYPE_RELIC:        out = {EQUIPMENT_SLOT_RANGED}; break;  // totem/idol/sigil/libram
        case INVTYPE_TABARD:
        case INVTYPE_BAG:
        case INVTYPE_AMMO:
        case INVTYPE_QUIVER:
        default: break; // not relevant for gear
    }
}

// Internal prototypes
static bool CanBotUseToken(ItemTemplate const* proto, Player* bot);
static bool RollUniqueCheck(ItemTemplate const* proto, Player* bot);

// Internal helpers
// Deduces the target slot from the token's name.
// Returns an expected InventoryType (HEAD/SHOULDERS/CHEST/HANDS/LEGS) or -1 if unknown.
static int8 TokenSlotFromName(ItemTemplate const* proto)
{
    if (!proto) return -1;
    std::string n = std::string(proto->Name1);
    std::transform(n.begin(), n.end(), n.begin(),
        [](unsigned char c){
            return static_cast<char>(std::tolower(c));
        });
    if (n.find("helm") != std::string::npos || n.find("head") != std::string::npos)
        return INVTYPE_HEAD;
    if (n.find("shoulder") != std::string::npos || n.find("mantle") != std::string::npos || n.find("spauld") != std::string::npos)
        return INVTYPE_SHOULDERS;
    if (n.find("chest") != std::string::npos || n.find("tunic") != std::string::npos || n.find("robe") != std::string::npos || n.find("breastplate") != std::string::npos || n.find("chestguard") != std::string::npos)
        return INVTYPE_CHEST;
    if (n.find("glove") != std::string::npos || n.find("handguard") != std::string::npos || n.find("gauntlet") != std::string::npos)
        return INVTYPE_HANDS;
    if (n.find("leg") != std::string::npos || n.find("pant") != std::string::npos || n.find("trouser") != std::string::npos)
        return INVTYPE_LEGS;
    return -1;
}

// Upgrade heuristic for a token: if the slot is known,
// consider it a "likely upgrade" if ilvl(token) >= ilvl(best equipped item in that slot) + margin.
static bool IsTokenLikelyUpgrade(ItemTemplate const* token, uint8 invTypeSlot, Player* bot)
{
    if (!token || !bot) return false;
    uint8 eq = EquipmentSlotByInvTypeSafe(invTypeSlot);
    if (eq >= EQUIPMENT_SLOT_END)
        return true; // unknown slot -> do not block Need
    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, eq);
    if (!oldItem) return true; // empty slot -> guaranteed upgrade
    ItemTemplate const* oldProto = oldItem->GetTemplate();
    if (!oldProto) return true;
    float margin = sPlayerbotAIConfig->tokenILevelMargin; // configurable
    return (float)token->ItemLevel >= (float)oldProto->ItemLevel + margin;
}

bool LootRollAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Roll*> rolls = group->GetRolls();
    for (Roll*& roll : rolls)
    {
        if (roll->playerVote.find(bot->GetGUID())->second != NOT_EMITED_YET)
        {
            continue;
        }
        ObjectGuid guid = roll->itemGUID;
        uint32 itemId = roll->itemid;
        int32 randomProperty = 0;
        if (roll->itemRandomPropId)
            randomProperty = roll->itemRandomPropId;
        else if (roll->itemRandomSuffix)
            randomProperty = -((int)roll->itemRandomSuffix);

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        RollVote vote = PASS;
        std::string itemUsageParam;
		
        if (randomProperty != 0) {
            itemUsageParam = std::to_string(itemId) + "," + std::to_string(randomProperty);
        } else {
            itemUsageParam = std::to_string(itemId);
        }
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemUsageParam);
        
        // Armor Tokens are classed as MISC JUNK (Class 15, Subclass 0), but no other items have class bits and epic quality.
        // - CanBotUseToken(proto, bot) => NEED
        // - else => GREED
        if (proto->Class == ITEM_CLASS_MISC && proto->SubClass == ITEM_SUBCLASS_JUNK && proto->Quality == ITEM_QUALITY_EPIC)
        {
            if (CanBotUseToken(proto, bot))
            {
                // vote = NEED; // Eligible for Need
                // Token mainspec: NEED only if the corresponding slot piece would be a real upgrade
                int8 tokenSlot = TokenSlotFromName(proto);
                if (tokenSlot >= 0)
                {
                    if (IsTokenLikelyUpgrade(proto, (uint8)tokenSlot, bot))
                        vote = NEED;
                    else
                        vote = GREED;
                }
                else
                {
                    // Unknown slot (e.g. T10 sanctification tokens)
                    vote = GREED;
                }
            }
            else
            {
                vote = GREED; // Not eligible, so Greed
            }
        }
        else
        {
            // Let CalculateRollVote decide (includes SmartNeedBySpec, BoE/BoU, unique, cross-armor)
            vote = CalculateRollVote(proto, randomProperty);
        }
		
        // Disenchant button (Need-Before-Greed): if useful for DE, prefer DE over Greed
        if (vote == GREED && usage == ITEM_USAGE_DISENCHANT && sPlayerbotAIConfig->useDEButton)
        {
            if (group && group->GetLootMethod() == NEED_BEFORE_GREED)
                vote = DISENCHANT;
        }

        if (sPlayerbotAIConfig->lootRollLevel == 0)
        {
            vote = PASS;
        }
        else if (sPlayerbotAIConfig->lootRollLevel == 1)
        {
            if (vote == NEED)
            {
                if (RollUniqueCheck(proto, bot))
                    {
                        vote = PASS;
                    }
                else 
                    {
                        vote = GREED;
                    }
            }
            else if (vote == GREED)
            {
                vote = PASS;
            }
        }
        // Announce + send the roll vote (if ML/FFA => PASS)
        RollVote sent = vote;
        if (group->GetLootMethod() == MASTER_LOOT || group->GetLootMethod() == FREE_FOR_ALL)
            sent = PASS;
        AnnounceRollChoice(sent, itemId);
        group->CountRollVote(bot->GetGUID(), guid, sent);
        // One item at a time
        return true;
    }

    return false;
}


RollVote LootRollAction::CalculateRollVote(ItemTemplate const* proto, int32 randomProperty)
{
    // Player mimic: upgrade => NEED; useful => GREED; otherwise => PASS
    std::ostringstream out;
    if (randomProperty != 0)
        out << proto->ItemId << "," << randomProperty;
    else
        out << proto->ItemId;
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());

	RollVote vote = PASS;
    switch (usage)
    {
        case ITEM_USAGE_EQUIP:
        case ITEM_USAGE_REPLACE:
           vote = NEED;
           // Downgrade to GREED if the item does not match the main spec
           if (sPlayerbotAIConfig->smartNeedBySpec && !IsPrimaryForSpec(bot, proto))
               vote = GREED;
           break;
        case ITEM_USAGE_BAD_EQUIP:
        case ITEM_USAGE_GUILD_TASK:
        case ITEM_USAGE_SKILL:
        case ITEM_USAGE_USE:
        case ITEM_USAGE_DISENCHANT:
        case ITEM_USAGE_AH:
        case ITEM_USAGE_VENDOR:
        case ITEM_USAGE_KEEP:
        case ITEM_USAGE_AMMO:
            vote = GREED;
            break;
        default:
            vote = PASS;
            break;
    }
    // VETO “physical -> caster” (safety net):
    // even if ItemUsage returned EQUIP/REPLACE, a physical character should never NEED a caster-profile item.
    if (vote == NEED && sPlayerbotAIConfig->smartNeedBySpec)
    {
        const std::string spec = AiFactory::GetPlayerSpecName(bot);
        const uint8 cls = bot->getClass();
        auto specIs = [&](char const* s){ return spec == s; };
        const bool isPhysicalSpec =
            !(cls == CLASS_MAGE || cls == CLASS_WARLOCK || cls == CLASS_PRIEST) &&
            !(cls == CLASS_PALADIN && spec == "holy") &&
            !(cls == CLASS_SHAMAN  && (specIs("elemental") || specIs("resto") || specIs("restoration"))) &&
            !(cls == CLASS_DRUID   && (specIs("balance")   || specIs("resto") || specIs("restoration")));

        const bool hasINT   = HasAnyStat(proto, { ITEM_MOD_INTELLECT });
        const bool hasSPI   = HasAnyStat(proto, { ITEM_MOD_SPIRIT });
        const bool hasMP5   = HasAnyStat(proto, { ITEM_MOD_MANA_REGENERATION });
        const bool hasSP    = HasAnyStat(proto, { ITEM_MOD_SPELL_POWER });
        const bool hasMelee = HasAnyStat(proto, { ITEM_MOD_STRENGTH, ITEM_MOD_AGILITY,
                                                  ITEM_MOD_ATTACK_POWER, ITEM_MOD_RANGED_ATTACK_POWER });
        const bool looksCaster = hasSP || hasSPI || hasMP5 || (hasINT && !hasMelee);

        if (isPhysicalSpec && looksCaster)
            vote = GREED; // force GREED for rogue/hunter/warrior/DK/retribution/enhancement/feral/protection, etc.
    }

    // Generic BoP rule: if the item is BoP, equippable, matches the spec
    // AND at least one relevant slot is empty -> allow NEED
    constexpr uint32 BIND_WHEN_PICKED_UP = 1;
    if (vote != NEED && proto->Bonding == BIND_WHEN_PICKED_UP)
    {
        std::vector<uint8> slots; GetEquipSlotsForInvType(proto->InventoryType, slots);
        if (!slots.empty())
        {
            const bool specOk = !sPlayerbotAIConfig->smartNeedBySpec || IsPrimaryForSpec(bot, proto);
            if (specOk)
            {
                for (uint8 s : slots)
                {
                    if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, s))
                    {
                        vote = NEED; // fills an empty slot -> NEED
                        break;
                    }
                }
            }
        }
    }
	
    // BoE/BoU rule: by default, avoid NEED on Bind-on-Equip / Bind-on-Use (raid etiquette)
    constexpr uint32 BIND_WHEN_EQUIPPED = 2; // BoE
    constexpr uint32 BIND_WHEN_USE      = 3; // BoU
    if (vote == NEED && proto->Bonding == BIND_WHEN_EQUIPPED && !sPlayerbotAIConfig->allowBoENeedIfUpgrade)
    {
        vote = GREED;
    }
    if (vote == NEED && proto->Bonding == BIND_WHEN_USE && !sPlayerbotAIConfig->allowBoUNeedIfUpgrade)
    {
        vote = GREED;
    }

    // Unique-equip: never NEED a duplicate (already equipped/owned)
    if (vote == NEED && RollUniqueCheck(proto, bot))
    {
        vote = PASS;
    }

    // Cross-armor: if BAD_EQUIP (e.g. cloth for paladin), allow NEED only if it's a massive upgrade
    if (vote == GREED && usage == ITEM_USAGE_BAD_EQUIP && proto->Class == ITEM_CLASS_ARMOR)
    {
        StatsWeightCalculator calc(bot);
        float newScore = calc.CalculateItem(proto->ItemId);
        float bestOld = 0.0f;
        // Find the best currently equipped item of the same InventoryType
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!oldItem) continue;
            ItemTemplate const* oldProto = oldItem->GetTemplate();
            if (!oldProto) continue;
            if (oldProto->Class != ITEM_CLASS_ARMOR) continue;
            if (oldProto->InventoryType != proto->InventoryType) continue;
            float oldScore = calc.CalculateItem(oldProto->ItemId, oldItem->GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
            if (oldScore > bestOld) bestOld = oldScore;
        }
        if (bestOld > 0.0f && newScore >= bestOld * sPlayerbotAIConfig->crossArmorExtraMargin)
            vote = NEED;
    }

    // Final filter: loot strategy
    return StoreLootAction::IsLootAllowed(proto->ItemId, GET_PLAYERBOT_AI(bot)) ? vote : PASS;
}

// Helpers d'annonce
const char* LootRollAction::RollVoteToText(RollVote v) const
{
    switch (v)
    {
        case NEED:        return "NEED";
        case GREED:       return "GREED";
        case PASS:        return "PASS";
        case DISENCHANT:  return "DISENCHANT";
        default:          return "PASS";
    }
}

void LootRollAction::AnnounceRollChoice(RollVote vote, uint32 itemId)
{
    if (!sPlayerbotAIConfig->announceLootRollsToMaster)
        return;

    Player* master = botAI->GetMaster();
    if (!master)
        return;

    std::ostringstream ss;
    if (ItemTemplate const* ip = sObjectMgr->GetItemTemplate(itemId))
        ss << "[Loot] " << bot->GetName() << " choisit " << RollVoteToText(vote)
           << " sur [" << ip->Name1 << "]";
    else
        ss << "[Loot] " << bot->GetName() << " choisit " << RollVoteToText(vote)
           << " sur item " << itemId;

    // Message to Master
    botAI->TellMaster(ss.str());
}


bool MasterLootRollAction::isUseful() { return !botAI->HasActivePlayerMaster(); }

bool MasterLootRollAction::Execute(Event event)
{
    Player* bot = QueryItemUsageAction::botAI->GetBot();

    WorldPacket p(event.getPacket());  // WorldPacket packet for CMSG_LOOT_ROLL, (8+4+1)
    ObjectGuid creatureGuid;
    uint32 mapId;
    uint32 itemSlot;
    uint32 itemId;
    uint32 randomSuffix;
    uint32 randomPropertyId;
    uint32 count;
    uint32 timeout;

    p.rpos(0);              // reset packet pointer
    p >> creatureGuid;      // creature guid what we're looting
    p >> mapId;             // 3.3.3 mapid
    p >> itemSlot;          // the itemEntryId for the item that shall be rolled for
    p >> itemId;            // the itemEntryId for the item that shall be rolled for
    p >> randomSuffix;      // randomSuffix
    p >> randomPropertyId;  // item random property ID
    p >> count;             // items in stack
    p >> timeout;           // the countdown time to choose "need" or "greed"

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 1) Token heuristic: ONLY NEED if the target slot is a likely upgrade
    RollVote vote = PASS;
    if (proto->Class == ITEM_CLASS_MISC && proto->SubClass == ITEM_SUBCLASS_JUNK && proto->Quality == ITEM_QUALITY_EPIC)
    {
        if (CanBotUseToken(proto, bot))
        {
            int8 tokenSlot = TokenSlotFromName(proto); // Internal helper
            if (tokenSlot >= 0)
                vote = IsTokenLikelyUpgrade(proto, (uint8)tokenSlot, bot) ? NEED : GREED;
            else
                vote = GREED; // Unknow slot
        }
        else
        {
            vote = GREED;
        }
    }
    else
    {
        vote = CalculateRollVote(proto, randomPropertyId ? (int32)randomPropertyId : (randomSuffix ? -(int32)randomSuffix : 0));
    }

    // 2) Disenchant button in Need-Before-Greed if the usage is "DISENCHANT"
    if (vote == GREED && sPlayerbotAIConfig->useDEButton && group->GetLootMethod() == NEED_BEFORE_GREED)
    {
        std::ostringstream out; out << itemId;
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
        if (usage == ITEM_USAGE_DISENCHANT)
            vote = DISENCHANT;
    }

    RollVote sent = vote;
    if (group->GetLootMethod() == MASTER_LOOT || group->GetLootMethod() == FREE_FOR_ALL)
        sent = PASS;
    AnnounceRollChoice(sent, itemId);
    group->CountRollVote(bot->GetGUID(), creatureGuid, sent);

    return true;
}

static bool CanBotUseToken(ItemTemplate const* proto, Player* bot)
{
    // Get the bitmask for the bot's class
    uint32 botClassMask = (1 << (bot->getClass() - 1));

    // Check if the bot's class is allowed to use the token
    if (proto->AllowableClass & botClassMask)
    {
        return true; // Bot's class is eligible to use this token
    }

    return false; // Bot's class cannot use this token
}

static bool RollUniqueCheck(ItemTemplate const* proto, Player* bot)
{
    // Count the total number of the item (equipped + in bags)
    uint32 totalItemCount = bot->GetItemCount(proto->ItemId, true);

    // Count the number of the item in bags only
    uint32 bagItemCount = bot->GetItemCount(proto->ItemId, false);

    // Determine if the unique item is already equipped
    bool isEquipped = (totalItemCount > bagItemCount);
    if (isEquipped && proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE))
    {
        return true;  // Unique Item is already equipped
    }
    else if (proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE) && (bagItemCount > 1))
    {
        return true; // Unique item already in bag, don't roll for it
    }
    return false; // Item is not equipped or in bags, roll for it
}

bool RollAction::Execute(Event event)
{
    std::string link = event.getParam();
    
    if (link.empty())
    {
        bot->DoRandomRoll(0,100);
        return false;
    }
    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;
    uint32 itemId = *itemIds.begin();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
    {
        return false;
    }
    std::string itemUsageParam;
    itemUsageParam = std::to_string(itemId);
        
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemUsageParam);
    switch (proto->Class)
    {
        case ITEM_CLASS_WEAPON:
        case ITEM_CLASS_ARMOR:
        //if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP)
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE)
        {
            bot->DoRandomRoll(0,100);
        }
    }
    return true;
}
