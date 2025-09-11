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

// Groups the "class + archetype" info in the same place
struct SpecTraits
{
    uint8 cls = 0;
    std::string spec;
    bool isCaster     = false;  // everything that affects SP/INT/SPI/MP5
    bool isHealer     = false;
    bool isTank       = false;
    bool isPhysical   = false;  // profiles STR/AGI/AP/ARP/EXP…
    // Useful flags for fine rules
    bool isDKTank     = false;
    bool isWarProt    = false;
    bool isEnhSham    = false;
    bool isFeralTk    = false;
    bool isFeralDps   = false;
    bool isHunter     = false;
    bool isRogue      = false;
    bool isWarrior    = false;
	bool isRetPal     = false;
};

static SpecTraits GetSpecTraits(Player* bot)
{
    SpecTraits t;
    if (!bot) return t;
    t.cls  = bot->getClass();
    t.spec = AiFactory::GetPlayerSpecName(bot);

    auto specIs = [&](char const* s){ return t.spec == s; };

    // "Pure caster" classes
    const bool pureCasterClass = (t.cls == CLASS_MAGE || t.cls == CLASS_WARLOCK || t.cls == CLASS_PRIEST);

    // Paladin
    const bool holyPal  = (t.cls == CLASS_PALADIN && specIs("holy"));
    const bool protPal  = (t.cls == CLASS_PALADIN && (specIs("prot") || specIs("protection")));
	t.isRetPal          = (t.cls == CLASS_PALADIN && !holyPal && !protPal);
    // DK
    const bool dk       = (t.cls == CLASS_DEATH_KNIGHT);
    const bool dkBlood  = dk && specIs("blood");
    const bool dkFrost  = dk && specIs("frost");
    const bool dkUH     = dk && (specIs("unholy") || specIs("uh"));
    t.isDKTank          = (dkBlood || dkFrost) && !dkUH;  // tanks “blood/frost”
    // Warrior
    t.isWarrior         = (t.cls == CLASS_WARRIOR);
    t.isWarProt         = t.isWarrior && (specIs("prot") || specIs("protection"));
    // Hunter & Rogue
    t.isHunter          = (t.cls == CLASS_HUNTER);
    t.isRogue           = (t.cls == CLASS_ROGUE);
    // Shaman
    const bool eleSham  = (t.cls == CLASS_SHAMAN && specIs("elemental"));
    const bool restoSh  = (t.cls == CLASS_SHAMAN && (specIs("resto") || specIs("restoration")));
    t.isEnhSham         = (t.cls == CLASS_SHAMAN && (specIs("enhance") || specIs("enhancement")));
    // Druid
    const bool balance  = (t.cls == CLASS_DRUID && specIs("balance"));
    const bool restoDr  = (t.cls == CLASS_DRUID && (specIs("resto") || specIs("restoration")));
    t.isFeralTk         = (t.cls == CLASS_DRUID && (specIs("feraltank") || specIs("bear")));
    t.isFeralDps        = (t.cls == CLASS_DRUID && (specIs("feraldps") || specIs("cat") || specIs("kitty")));

    // Roles
    t.isHealer   = holyPal || restoSh || restoDr || (t.cls == CLASS_PRIEST && !specIs("shadow"));
    t.isTank     = protPal || t.isWarProt || t.isFeralTk || t.isDKTank;
    t.isCaster   = pureCasterClass || holyPal || eleSham || balance || restoDr || restoSh
                   || (t.cls == CLASS_PRIEST && specIs("shadow")); // Shadow = caster DPS
    t.isPhysical = !t.isCaster;
    return t;
}

// Local helper: identifies classic lockboxes the Rogue can pick.
// Heuristic: items with a LockID and “misc/junk” class often are lockboxes.
// Also fall back to a name check (“lockbox”, “coffre”) to cover edge DBs.
static bool IsLockbox(ItemTemplate const* proto)
{
    if (!proto) return false;
    if (proto->LockID)
    {
        // Most lockboxes are misc/junk and openable.
        if (proto->Class == ITEM_CLASS_MISC)
            return true;
    }
    // Fallback on localized name check
    static const std::vector<std::string> lockboxKeywords = {
        "lockbox",      // English
        "coffre",       // French
        "schließkassette", // German
        "caja fuerte",  // Spanish
        "сундук",       // Russian
        "잠긴",         // Korean
        "锁箱",         // Simplified Chinese
        "鎖箱"          // Traditional Chinese
    };

    std::string n = proto->Name1;
    std::transform(n.begin(), n.end(), n.begin(),
                   [](unsigned char c){ return (c >= 'A' && c <= 'Z') ? char(c + 32) : char(c); });
    for (auto const& kw : lockboxKeywords)
    {
        const bool ascii = std::all_of(kw.begin(), kw.end(), [](unsigned char c){ return c < 0x80; });
        if ((ascii && n.find(kw) != std::string::npos) || (!ascii && std::string(proto->Name1).find(kw) != std::string::npos))
            return true;
    }
    return false;
}

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

// Encode "random enchant" parameter for CalculateRollVote / ItemUsage
// >0 => randomPropertyId, <0 => randomSuffixId, 0 => none
static inline int32 EncodeRandomEnchantParam(uint32 randomPropertyId, uint32 randomSuffix)
{
    if (randomPropertyId) return static_cast<int32>(randomPropertyId);
    if (randomSuffix)     return -static_cast<int32>(randomSuffix);
    return 0;
}

// Weapon/shield/relic whitelist per class.
// Returns false when the item is a WEAPON / SHIELD / RELIC the class should NOT use.
static bool IsWeaponOrShieldOrRelicAllowedForClass(SpecTraits const& T, ItemTemplate const* proto)
{
    if (!proto) return true; // non-weapon items handled elsewhere

    // Shields (Armor + Shield): Paladin / Warrior / Shaman
    if ((proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD) ||
        proto->InventoryType == INVTYPE_SHIELD)
        return T.cls == CLASS_PALADIN || T.cls == CLASS_WARRIOR || T.cls == CLASS_SHAMAN;

     // Relics (Idol/Totem/Sigil/Libram)
    if (proto->InventoryType == INVTYPE_RELIC)
    {
        // DK (Sigil), Druid (Idol), Paladin (Libram), Shaman (Totem)
        return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_DRUID ||
               T.cls == CLASS_PALADIN      || T.cls == CLASS_SHAMAN;
    }

    // Not a weapon: nothing to filter here
    if (proto->Class != ITEM_CLASS_WEAPON)
        return true;

    switch (proto->SubClass)
    {
        // Axes
        case ITEM_SUBCLASS_WEAPON_AXE:
        case ITEM_SUBCLASS_WEAPON_AXE2:
            // 1H axes allowed for Rogue; 2H axes not (but same SubClass enum, handled by InventoryType later if needed)
            return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_HUNTER  ||
                   T.cls == CLASS_PALADIN      || T.cls == CLASS_SHAMAN  ||
                   T.cls == CLASS_WARRIOR      || T.cls == CLASS_ROGUE;  // Rogue: 1H axes

        // Swords
        case ITEM_SUBCLASS_WEAPON_SWORD:   // 1H swords
            return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_HUNTER  ||
                   T.cls == CLASS_MAGE         || T.cls == CLASS_PALADIN  ||
                   T.cls == CLASS_ROGUE        || T.cls == CLASS_WARRIOR  ||
                   T.cls == CLASS_WARLOCK; // Warlocks can use 1H swords
        case ITEM_SUBCLASS_WEAPON_SWORD2:  // 2H swords
            return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_HUNTER  ||
                   T.cls == CLASS_PALADIN      || T.cls == CLASS_WARRIOR;

        // Maces
        case ITEM_SUBCLASS_WEAPON_MACE:    // 1H maces
            return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_DRUID   ||
                   T.cls == CLASS_PALADIN      || T.cls == CLASS_PRIEST  ||
                   T.cls == CLASS_SHAMAN       || T.cls == CLASS_WARRIOR ||
                   T.cls == CLASS_ROGUE; // Rogue: 1H maces in WotLK
        case ITEM_SUBCLASS_WEAPON_MACE2:   // 2H maces
            return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_DRUID   ||
                   T.cls == CLASS_PALADIN      || T.cls == CLASS_WARRIOR; // Shaman: no 2H maces

        // Polearms
        case ITEM_SUBCLASS_WEAPON_POLEARM:
            return T.cls == CLASS_DEATH_KNIGHT || T.cls == CLASS_DRUID   ||
                   T.cls == CLASS_HUNTER       || T.cls == CLASS_PALADIN  ||
                   T.cls == CLASS_WARRIOR; // Shaman: cannot use polearms

        // Staves
        case ITEM_SUBCLASS_WEAPON_STAFF:
            return T.cls == CLASS_DRUID   || T.cls == CLASS_HUNTER  ||
                   T.cls == CLASS_MAGE    || T.cls == CLASS_PRIEST  ||
                   T.cls == CLASS_SHAMAN  || T.cls == CLASS_WARLOCK;

        // Daggers
        case ITEM_SUBCLASS_WEAPON_DAGGER:
            return T.cls == CLASS_DRUID   || T.cls == CLASS_HUNTER  ||
                   T.cls == CLASS_MAGE    || T.cls == CLASS_PRIEST  ||
                   T.cls == CLASS_ROGUE   || T.cls == CLASS_WARLOCK ||
                   T.cls == CLASS_WARRIOR; // Warriors can use daggers

        // Fist weapons
        case ITEM_SUBCLASS_WEAPON_FIST:
            return T.cls == CLASS_DRUID   || T.cls == CLASS_HUNTER  ||
                   T.cls == CLASS_ROGUE   || T.cls == CLASS_SHAMAN  ||
                   T.cls == CLASS_WARRIOR;

         // Ranged (bows / guns / crossbows) — Hunters primary; also usable by Warriors/Rogues
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_GUN:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            return T.cls == CLASS_HUNTER || T.cls == CLASS_WARRIOR || T.cls == CLASS_ROGUE;

        // Wands — only Mage/Priest/Warlock
        case ITEM_SUBCLASS_WEAPON_WAND:
            return T.cls == CLASS_MAGE || T.cls == CLASS_PRIEST || T.cls == CLASS_WARLOCK;

        // Thrown — Warriors/Rogues (Hunters rarely need them; bows/guns/xbows preferred)
        case ITEM_SUBCLASS_WEAPON_THROWN:
            return T.cls == CLASS_WARRIOR || T.cls == CLASS_ROGUE;

        // Exotic / fishing / misc — disallow
         case ITEM_SUBCLASS_WEAPON_EXOTIC:
         case ITEM_SUBCLASS_WEAPON_EXOTIC2:
         case ITEM_SUBCLASS_WEAPON_MISC:
         case ITEM_SUBCLASS_WEAPON_FISHING_POLE:
         default:
             return false;
    }
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

    const SpecTraits T = GetSpecTraits(bot);


    // Hard filter first: do not NEED weapons/shields/relics the class shouldn't use.
    // If this returns false, the caller will downgrade to GREED (off-spec/unsupported).
    if (!IsWeaponOrShieldOrRelicAllowedForClass(T, proto))
        return false;

    // Flags class/spec
    const bool isCasterSpec   = T.isCaster;
    const bool isTankLikeSpec = T.isTank;
    const bool isPhysicalSpec = T.isPhysical;

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

    // Quick profiles
    const bool looksCaster   = hasSP || hasSPI || hasMP5 || (hasINT && !hasSTR && !hasAGI && !hasAP);
    const bool looksPhysical = hasSTR || hasAGI || hasAP || hasARP || hasEXP;
    const bool hasDpsRatings = hasHIT || hasHASTE || hasCRIT; // Common to all DPS (physical & casters)

    // Tank-only profile: Defense / Avoidance (dodge/parry/block rating) / Block value
    // Do NOT tag all shields as "tank": there are caster shields (INT/SP/MP5)
    const bool hasBlockValue = HasAnyStat(proto, { ITEM_MOD_BLOCK_VALUE });
    const bool looksTank = hasDef || hasAvoid || hasBlockValue;

    // Non-tanks (DPS, casters/heals) never NEED purely tank items
    if (!isTankLikeSpec && looksTank)
        return false;

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

    // Extra weapon sanity for Hunters/Ferals (avoid wrong stat-sticks):
    // - Hunters: for melee weapons, require AGI (prevent Haste/AP-only daggers without AGI).
    // - Feral (tank/DPS): for melee weapons, require AGI or STR.
    if (proto->Class == ITEM_CLASS_WEAPON)
    {
        const bool meleeWeapon =
            proto->InventoryType == INVTYPE_WEAPON ||
            proto->InventoryType == INVTYPE_WEAPONMAINHAND ||
            proto->InventoryType == INVTYPE_WEAPONOFFHAND ||
            proto->InventoryType == INVTYPE_2HWEAPON;

        if (meleeWeapon && T.isHunter && !hasAGI)
            return false;

        if (meleeWeapon && (T.isFeralTk || T.isFeralDps) && !hasAGI && !hasSTR)
            return false;
    }

    // Class/spec specific adjustments (readable)
    // DK Unholy (DPS): allows STR/HIT/HASTE/CRIT/ARP; rejects all caster items
    if (/* DK Unholy */ (T.cls == CLASS_DEATH_KNIGHT && (T.spec == "unholy" || T.spec == "uh")))
    {
        if (looksCaster) return false;
    }
    // DK Blood/Frost tanks: DEF/AVOID/STA/STR are useful; reject caster items
    if (T.isDKTank)
    {
        if (looksCaster) return false;
        // Pure caster DPS rings/trinkets already filtered above.
    }
    // Hunter (BM/MM/SV): agi/hit/haste/AP/crit/arp → OK; avoid STR-only or caster items
    if (T.isHunter)
    {
        if (looksCaster) return false;
        // Avoid rings with "pure STR" without AGI/AP/DPS ratings
        if (isJewelry && hasSTR && !hasAGI && !hasAP && !hasDpsRatings)
            return false;
    }
    // Rogue (all specs): same strict physical filter (no caster items)
    if (T.isRogue)
    {
        if (looksCaster) return false;
    }
    // Warrior Arms/Fury : no caster items
    if (T.isWarrior && !T.isWarProt)
    {
        if (looksCaster) return false;
    }
    // Warrior Protection: DEF/AVOID/STA/STR are useful; no caster items
    if (T.isWarProt)
    {
        if (looksCaster) return false;
    }
    // Shaman Enhancement: no Spell Power weapons/shields, no pure INT/SP items
    if (T.isEnhSham)
    {
        if (looksCaster) return false;
        if ((proto->Class == ITEM_CLASS_WEAPON || (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD))
            && hasSP)
            return false;
    }
    // Druid Feral (tank/DPS): AGI/STA/AVOID/ARP/EXP -> OK; no caster items
    if (T.isFeralTk || T.isFeralDps)
    {
        if (looksCaster) return false;
    }

    // Paladin Retribution: physical DPS (no caster items; forbid SP weapons/shields; enforce 2H only)
    if (T.isRetPal)
    {
        if (looksCaster) return false;

        // No Spell Power weapons or shields for Ret
        if ((proto->Class == ITEM_CLASS_WEAPON ||
             (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)) && hasSP)
            return false;

        // Enforce 2H only (no 1H/off-hand/shields/holdables)
        switch (proto->InventoryType)
        {
            case INVTYPE_WEAPON:         // generic 1H
            case INVTYPE_WEAPONMAINHAND: // explicit main-hand 1H
            case INVTYPE_WEAPONOFFHAND:  // off-hand weapon
            case INVTYPE_SHIELD:         // shields
            case INVTYPE_HOLDABLE:       // tomes/orbs
                return false;            // never NEED for Ret
            default:
                break;                   // INVTYPE_2HWEAPON is allowed; others handled elsewhere
        }
    }

    // Global VETO: a "physical" spec never considers a caster profile as primary
    if (sPlayerbotAIConfig->smartNeedBySpec && T.isPhysical && looksCaster)
        return false;

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
        default:                 return EQUIPMENT_SLOT_END; // unknown/not applicable
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
        // Avoid server crash, key may not exit for the bot on login
        auto it = roll->playerVote.find(bot->GetGUID());
        if (it != roll->playerVote.end() && it->second != NOT_EMITED_YET)
        {
            continue;
        }
		
        ObjectGuid guid = roll->itemGUID;
        uint32 itemId = roll->itemid;
        int32 randomProperty = EncodeRandomEnchantParam(roll->itemRandomPropId, roll->itemRandomSuffix);

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
	
    // Lockboxes: if the item is a lockbox and the bot is a Rogue with Lockpicking, prefer NEED.
    // (Handled before BoE/BoU etiquette; BoE/BoU checks below ignore lockboxes.)
    const SpecTraits T = GetSpecTraits(bot);
    const bool isLockbox = IsLockbox(proto);
    if (isLockbox && T.isRogue && bot->HasSkill(SKILL_LOCKPICKING))
        vote = NEED;

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
    if (vote == NEED && !isLockbox && proto->Bonding == BIND_WHEN_EQUIPPED && !sPlayerbotAIConfig->allowBoENeedIfUpgrade)
    {
        vote = GREED;
    }
    if (vote == NEED && !isLockbox && proto->Bonding == BIND_WHEN_USE && !sPlayerbotAIConfig->allowBoUNeedIfUpgrade)
    {
        vote = GREED;
    }

    // Duplicate soft rule (non-unique): if the bot already owns at least one copy in bags, downgrade NEED to GREED.
    // This complements "unique-equip" and avoids ninja-need on duplicates that are not unique.
    if (vote == NEED)
    {
        // includeBank=true to catch banked duplicates as well; adjust if undesired.
        if (bot->GetItemCount(proto->ItemId, true) > 0)
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

    // return StoreLootAction::IsLootAllowed(proto->ItemId, GET_PLAYERBOT_AI(bot)) ? vote : PASS;
    // Final filter: loot strategy (protect a potential nil AI earlier on login
    if (PlayerbotAI* ai = GET_PLAYERBOT_AI(bot))
        return StoreLootAction::IsLootAllowed(proto->ItemId, ai) ? vote : PASS;
    return PASS;
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
        ss << "[Loot] " << bot->GetName() << " choose " << RollVoteToText(vote)
           << " on [" << ip->Name1 << "]";
    else
        ss << "[Loot] " << bot->GetName() << " choose " << RollVoteToText(vote)
           << " on item " << itemId;

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
        vote = CalculateRollVote(proto, EncodeRandomEnchantParam(randomPropertyId, randomSuffix));
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
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE)
        {
            bot->DoRandomRoll(0,100);
        }
    }
    return true;
}
