/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RandomItemMgr.h"

#include "ItemTemplate.h"
#include "LootValues.h"
#include "Playerbots.h"

char* strstri(char const* str1, char const* str2);
std::set<uint32> RandomItemMgr::itemCache;

uint64 BotEquipKey::GetKey() { return level + 100 * clazz + 10000 * slot + 1000000 * quality; }

class RandomItemGuildTaskPredicate : public RandomItemPredicate
{
public:
    bool Apply(ItemTemplate const* proto) override
    {
        if (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM ||
            proto->Bonding == BIND_WHEN_USE)
            return false;

        if (proto->Quality < ITEM_QUALITY_NORMAL)
            return false;

        if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) &&
            proto->Quality >= ITEM_QUALITY_RARE)
            return true;

        if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_CONSUMABLE)
            return true;

        return false;
    }
};

class RandomItemGuildTaskRewardPredicate : public RandomItemPredicate
{
public:
    RandomItemGuildTaskRewardPredicate(bool equip, bool rare) : equip(equip), rare(rare) {}

    bool Apply(ItemTemplate const* proto) override
    {
        if (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_QUEST_ITEM ||
            proto->Bonding == BIND_WHEN_USE)
            return false;

        if (proto->Class == ITEM_CLASS_QUEST)
            return false;

        if (equip)
        {
            uint32 desiredQuality = rare ? ITEM_QUALITY_RARE : ITEM_QUALITY_UNCOMMON;
            if (proto->Quality < desiredQuality || proto->Quality >= ITEM_QUALITY_EPIC)
                return false;

            if (proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON)
                return true;
        }
        else
        {
            uint32 desiredQuality = rare ? ITEM_QUALITY_UNCOMMON : ITEM_QUALITY_NORMAL;
            if (proto->Quality < desiredQuality || proto->Quality >= ITEM_QUALITY_RARE)
                return false;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_CONSUMABLE)
                return true;
        }

        return false;
    }

private:
    bool equip;
    bool rare;
};

RandomItemMgr::RandomItemMgr()
{
    predicates[RANDOM_ITEM_GUILD_TASK] = new RandomItemGuildTaskPredicate();
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN] = new RandomItemGuildTaskRewardPredicate(true, false);
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE] = new RandomItemGuildTaskRewardPredicate(true, true);
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_TRADE] = new RandomItemGuildTaskRewardPredicate(false, false);
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE] = new RandomItemGuildTaskRewardPredicate(false, true);

    viableSlots[EQUIPMENT_SLOT_HEAD].insert(INVTYPE_HEAD);
    viableSlots[EQUIPMENT_SLOT_NECK].insert(INVTYPE_NECK);
    viableSlots[EQUIPMENT_SLOT_SHOULDERS].insert(INVTYPE_SHOULDERS);
    viableSlots[EQUIPMENT_SLOT_BODY].insert(INVTYPE_BODY);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_CHEST);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_ROBE);
    viableSlots[EQUIPMENT_SLOT_WAIST].insert(INVTYPE_WAIST);
    viableSlots[EQUIPMENT_SLOT_LEGS].insert(INVTYPE_LEGS);
    viableSlots[EQUIPMENT_SLOT_FEET].insert(INVTYPE_FEET);
    viableSlots[EQUIPMENT_SLOT_WRISTS].insert(INVTYPE_WRISTS);
    viableSlots[EQUIPMENT_SLOT_HANDS].insert(INVTYPE_HANDS);
    viableSlots[EQUIPMENT_SLOT_FINGER1].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_FINGER2].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_TRINKET1].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_TRINKET2].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_SHIELD);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_HOLDABLE);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGED);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_THROWN);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGEDRIGHT);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RELIC);
    viableSlots[EQUIPMENT_SLOT_TABARD].insert(INVTYPE_TABARD);
    viableSlots[EQUIPMENT_SLOT_BACK].insert(INVTYPE_CLOAK);

    weightStatLink["sta"] = ITEM_MOD_STAMINA;
    weightStatLink["str"] = ITEM_MOD_STRENGTH;
    weightStatLink["agi"] = ITEM_MOD_AGILITY;
    weightStatLink["int"] = ITEM_MOD_INTELLECT;
    weightStatLink["spi"] = ITEM_MOD_SPIRIT;

    weightStatLink["splpwr"] = ITEM_MOD_SPELL_POWER;
    weightStatLink["atkpwr"] = ITEM_MOD_ATTACK_POWER;

    weightStatLink["exprtng"] = ITEM_MOD_EXPERTISE_RATING;
    weightStatLink["critstrkrtng"] = ITEM_MOD_CRIT_RATING;
    weightStatLink["hitrtng"] = ITEM_MOD_HIT_RATING;
    weightStatLink["hastertng"] = ITEM_MOD_HASTE_RATING;
    weightStatLink["armorpenrtng"] = ITEM_MOD_ARMOR_PENETRATION_RATING;

    weightStatLink["defrtng"] = ITEM_MOD_DEFENSE_SKILL_RATING;
    weightStatLink["dodgertng"] = ITEM_MOD_DODGE_RATING;
    weightStatLink["block"] = ITEM_MOD_BLOCK_VALUE;
    weightStatLink["blockrtng"] = ITEM_MOD_BLOCK_RATING;
    weightStatLink["parryrtng"] = ITEM_MOD_PARRY_RATING;

    weightStatLink["manargn"] = ITEM_MOD_MANA_REGENERATION;

    weightRatingLink["exprtng"] = CR_EXPERTISE;
    weightRatingLink["critstrkrtng"] = CR_CRIT_MELEE;
    weightRatingLink["hitrtng"] = CR_HIT_MELEE;
    weightRatingLink["hastertng"] = CR_HASTE_MELEE;
    weightRatingLink["armorpenrtng"] = CR_ARMOR_PENETRATION;

    weightRatingLink["defrtng"] = CR_DEFENSE_SKILL;
    weightRatingLink["dodgertng"] = CR_DODGE;
    weightRatingLink["blockrtng"] = CR_BLOCK;
    weightRatingLink["parryrtng"] = CR_PARRY;
}

void RandomItemMgr::Init()
{
    BuildItemInfoCache();
    // BuildEquipCache();
    BuildEquipCacheNew();
    BuildAmmoCache();
    BuildPotionCache();
    BuildFoodCache();
    BuildTradeCache();
}

void RandomItemMgr::InitAfterAhBot()
{
    BuildRandomItemCache();
    // BuildRarityCache();
}

RandomItemMgr::~RandomItemMgr()
{
    for (std::map<RandomItemType, RandomItemPredicate*>::iterator i = predicates.begin(); i != predicates.end(); ++i)
        delete i->second;

    predicates.clear();
}

bool RandomItemMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        LOG_ERROR("playerbots", "Usage: rnditem");
        return false;
    }

    return false;
}

RandomItemList RandomItemMgr::Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList& list = randomItemCache[(level - 1) / 10][type];

    RandomItemList result;
    for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
    {
        uint32 itemId = *i;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        if (predicate && !predicate->Apply(proto))
            continue;

        result.push_back(itemId);
    }

    return result;
}

void RandomItemMgr::BuildRandomItemCache()
{
    if (PreparedQueryResult result =
            PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RNDITEM_CACHE)))
    {
        LOG_INFO("server.loading", "Loading random item cache");
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 level = fields[0].Get<uint32>();
            uint32 type = fields[1].Get<uint32>();
            uint32 itemId = fields[2].Get<uint32>();

            RandomItemType rit = (RandomItemType)type;
            randomItemCache[level][rit].push_back(itemId);
            ++count;

        } while (result->NextRow());

        LOG_INFO("server.loading", "Equipment cache loaded from {} records", count);
    }
    else
    {
        ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
        LOG_INFO("server.loading", "Building random item cache from {} items", itemTemplates->size());

        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (strstri(proto->Name1.c_str(), "qa") || strstri(proto->Name1.c_str(), "test") ||
                strstri(proto->Name1.c_str(), "deprecated"))
                continue;

            if (!proto->ItemLevel)
                continue;

            if (!proto->SellPrice)
                continue;

            uint32 level = proto->ItemLevel;
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemType rit = (RandomItemType)type;
                if (predicates[rit] && !predicates[rit]->Apply(proto))
                    continue;

                randomItemCache[level / 10][rit].push_back(itr.first);

                PlayerbotsDatabasePreparedStatement* stmt =
                    PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_RNDITEM_CACHE);
                stmt->SetData(0, level / 10);
                stmt->SetData(1, type);
                stmt->SetData(2, itr.first);
                PlayerbotsDatabase.Execute(stmt);
            }
        }

        uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
        if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

        for (uint32 level = 0; level < maxLevel / 10; level++)
        {
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemList list = randomItemCache[level][(RandomItemType)type];
                LOG_INFO("playerbots", "    Level {}..{} Type {} - {} random items cached", level * 10, level * 10 + 9,
                         type, list.size());

                for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
                {
                    uint32 itemId = *i;
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (!proto)
                        continue;

                    LOG_DEBUG("playerbots", "        [{}] {}", itemId, proto->Name1.c_str());
                }
            }
        }
    }
}

uint32 RandomItemMgr::GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList const& list = Query(level, type, predicate);
    if (list.empty())
        return 0;

    uint32 index = urand(0, list.size() - 1);
    uint32 itemId = list[index];

    return itemId;
}

bool RandomItemMgr::CanEquipItem(BotEquipKey key, ItemTemplate const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Quality != key.quality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    std::set<InventoryType> slots = viableSlots[(EquipmentSlots)key.slot];
    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
        return false;

    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
    {
        requiredLevel = GetMinLevelFromCache(proto->ItemId);
    }

    if (!requiredLevel)
        requiredLevel = key.level;

    uint32 level = key.level;

    uint32 delta = 2;
    if (level < 15)
        delta = 15;
    else if (level < 40)
        delta = 10;  // urand(5, 10);
    else if (level < 60)
        delta = 6;  // urand(3, 7);
    else if (level < 70)
        delta = 9;  // urand(2, 5);
    else if (level < 80)
        delta = 9;  // urand(2, 4);
    else if (level == 80)
        delta = 9;  // urand(2, 4);

    if (key.quality > ITEM_QUALITY_NORMAL && (requiredLevel > level || requiredLevel < level - delta))
        return false;

    // for (uint32 gap = 60; gap <= 80; gap += 10)
    // {
    //     if (level > gap && requiredLevel <= gap)
    //         return false;
    // }

    return true;
}

bool RandomItemMgr::CanEquipItemNew(ItemTemplate const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return false;

    bool properSlot = false;
    for (std::map<EquipmentSlots, std::set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end();
         ++i)
    {
        std::set<InventoryType> const& slots = viableSlots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
            properSlot = true;
    }

    return properSlot;
}

void RandomItemMgr::AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank)
{
    switch (mod)
    {
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
        case ITEM_MOD_MANA:
        case ITEM_MOD_INTELLECT:
        case ITEM_MOD_SPIRIT:
            ++sp;
            break;
    }

    switch (mod)
    {
        case ITEM_MOD_AGILITY:
        case ITEM_MOD_STRENGTH:
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
            ++tank;
            break;
    }

    switch (mod)
    {
        case ITEM_MOD_HEALTH:
        case ITEM_MOD_STAMINA:
        case ITEM_MOD_AGILITY:
        case ITEM_MOD_STRENGTH:
            ++ap;
            break;
    }
}

bool RandomItemMgr::CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank)
{
    switch (clazz)
    {
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            if (!sp || ap > sp || tank > sp)
                return false;
            break;
        case CLASS_PALADIN:
        case CLASS_WARRIOR:
            if ((!ap && !tank) || sp > ap || sp > tank)
                return false;
            break;
        case CLASS_HUNTER:
        case CLASS_ROGUE:
            if (!ap || sp > ap || sp > tank)
                return false;
            break;
    }

    return sp || ap || tank;
}

std::vector<uint32> RandomItemMgr::GetCachedEquipments(uint32 requiredLevel, uint32 inventoryType)
{
    return equipCacheNew[requiredLevel][inventoryType];
}

bool RandomItemMgr::ShouldEquipArmorForSpec(uint8 playerclass, uint8 spec, ItemTemplate const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if (!m_weightScales[playerclass][spec].info.id)
        return false;

    std::unordered_set<uint32> resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_CLOTH};

    switch (playerclass)
    {
        case CLASS_WARRIOR:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            if (m_weightScales[playerclass][spec].info.name == "arms" ||
                m_weightScales[playerclass][spec].info.name == "fury")
            {
                resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL,
                                       ITEM_SUBCLASS_ARMOR_PLATE};
            }
            else
                resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE};
            break;
        }
        case CLASS_DEATH_KNIGHT:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_SIGIL, ITEM_SUBCLASS_ARMOR_PLATE};
        }
        case CLASS_PALADIN:
        {
            if (m_weightScales[playerclass][spec].info.name != "holy" && proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            if (m_weightScales[playerclass][spec].info.name != "holy")
                resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE, ITEM_SUBCLASS_ARMOR_LIBRAM};
            else
                resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL,
                                       ITEM_SUBCLASS_ARMOR_PLATE, ITEM_SUBCLASS_ARMOR_LIBRAM};
            break;
        }
        case CLASS_HUNTER:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL};
            break;
        }
        case CLASS_ROGUE:
        {
            if (proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER};
            break;
        }
        case CLASS_PRIEST:
        {
            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_CLOTH};
            break;
        }
        case CLASS_SHAMAN:
        {
            if (m_weightScales[playerclass][spec].info.name == "enhance" && proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER,
                                   ITEM_SUBCLASS_ARMOR_MAIL};
            break;
        }
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        {
            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_CLOTH};
            break;
        }
        case CLASS_DRUID:
        {
            if ((m_weightScales[playerclass][spec].info.name == "feraltank" ||
                 m_weightScales[playerclass][spec].info.name == "feraldps") &&
                proto->InventoryType == INVTYPE_HOLDABLE)
                return false;

            resultArmorSubClass = {ITEM_SUBCLASS_ARMOR_IDOL, ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER};
            break;
        }
    }

    return resultArmorSubClass.find(proto->SubClass) != resultArmorSubClass.end();
}

bool RandomItemMgr::ShouldEquipWeaponForSpec(uint8 playerclass, uint8 spec, ItemTemplate const* proto)
{
    EquipmentSlots slot_mh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_oh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_rh = EQUIPMENT_SLOT_START;
    for (std::map<EquipmentSlots, std::set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end();
         ++i)
    {
        std::set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
        {
            if (i->first == EQUIPMENT_SLOT_MAINHAND)
                slot_mh = i->first;
            if (i->first == EQUIPMENT_SLOT_OFFHAND)
                slot_oh = i->first;
            if (i->first == EQUIPMENT_SLOT_RANGED)
                slot_rh = i->first;
        }
    }

    if (slot_mh == EQUIPMENT_SLOT_START && slot_oh == EQUIPMENT_SLOT_START && slot_rh == EQUIPMENT_SLOT_START)
        return false;

    if (!m_weightScales[playerclass][spec].info.id)
        return false;

    std::unordered_set<uint32> mh_weapons;
    std::unordered_set<uint32> oh_weapons;
    std::unordered_set<uint32> r_weapons;

    switch (playerclass)
    {
        case CLASS_WARRIOR:
        {
            if (m_weightScales[playerclass][spec].info.name == "prot")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE,
                              ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_FIST};
                oh_weapons = {ITEM_SUBCLASS_ARMOR_SHIELD};
                r_weapons = {ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN};
            }
            else
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2,
                              ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_POLEARM};
                r_weapons = {ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN};
            }
            break;
        }
        case CLASS_PALADIN:
        {
            if (m_weightScales[playerclass][spec].info.name == "prot")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE};
                oh_weapons = {ITEM_SUBCLASS_ARMOR_SHIELD};
                r_weapons = {ITEM_SUBCLASS_ARMOR_LIBRAM};
            }
            else if (m_weightScales[playerclass][spec].info.name == "holy")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE};
                oh_weapons = {ITEM_SUBCLASS_ARMOR_SHIELD, ITEM_SUBCLASS_ARMOR_MISC};
                r_weapons = {ITEM_SUBCLASS_ARMOR_LIBRAM};
            }
            else
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2,
                              ITEM_SUBCLASS_WEAPON_POLEARM};
                r_weapons = {ITEM_SUBCLASS_ARMOR_LIBRAM};
            }
            break;
        }
        case CLASS_HUNTER:
        {
            mh_weapons = {ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_STAFF,
                          ITEM_SUBCLASS_WEAPON_POLEARM};
            r_weapons = {ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN};
            break;
        }
        case CLASS_ROGUE:
        {
            mh_weapons = {ITEM_SUBCLASS_WEAPON_DAGGER};
            oh_weapons = {ITEM_SUBCLASS_WEAPON_DAGGER};
            r_weapons = {ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN};
            break;
        }
        case CLASS_PRIEST:
        {
            mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE};
            oh_weapons = {ITEM_SUBCLASS_ARMOR_MISC};
            r_weapons = {ITEM_SUBCLASS_WEAPON_WAND};
            break;
        }
        case CLASS_SHAMAN:
        {
            if (m_weightScales[playerclass][spec].info.name == "resto")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE,
                              ITEM_SUBCLASS_WEAPON_FIST};
                oh_weapons = {ITEM_SUBCLASS_ARMOR_MISC, ITEM_SUBCLASS_ARMOR_SHIELD};
                r_weapons = {ITEM_SUBCLASS_ARMOR_TOTEM};
            }
            else if (m_weightScales[playerclass][spec].info.name == "enhance")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_DAGGER,
                              ITEM_SUBCLASS_WEAPON_AXE,   ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST};
                oh_weapons = {ITEM_SUBCLASS_ARMOR_SHIELD};
                r_weapons = {ITEM_SUBCLASS_ARMOR_TOTEM};
            }
            else
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF};
                r_weapons = {ITEM_SUBCLASS_ARMOR_TOTEM};
            }
            break;
        }
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        {
            mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD};
            oh_weapons = {ITEM_SUBCLASS_ARMOR_MISC};
            r_weapons = {ITEM_SUBCLASS_WEAPON_WAND};
            break;
        }
        case CLASS_DRUID:
        {
            if (m_weightScales[playerclass][spec].info.name == "feraltank")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2};
                r_weapons = {ITEM_SUBCLASS_ARMOR_IDOL};
            }
            else if (m_weightScales[playerclass][spec].info.name == "resto")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_FIST,
                              ITEM_SUBCLASS_WEAPON_MACE};
                oh_weapons = {ITEM_SUBCLASS_ARMOR_MISC};
                r_weapons = {ITEM_SUBCLASS_ARMOR_IDOL};
            }
            else if (m_weightScales[playerclass][spec].info.name == "feraldps")
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2};
                r_weapons = {ITEM_SUBCLASS_ARMOR_IDOL};
            }
            else
            {
                mh_weapons = {ITEM_SUBCLASS_WEAPON_STAFF};
                r_weapons = {ITEM_SUBCLASS_ARMOR_IDOL};
            }
            break;
        }
    }

    if (slot_mh == EQUIPMENT_SLOT_MAINHAND)
    {
        return mh_weapons.find(proto->SubClass) != mh_weapons.end();
    }

    if (slot_oh == EQUIPMENT_SLOT_OFFHAND)
    {
        return oh_weapons.find(proto->SubClass) != oh_weapons.end();
    }

    if (slot_rh == EQUIPMENT_SLOT_RANGED)
    {
        return r_weapons.find(proto->SubClass) != r_weapons.end();
    }

    return false;
}

bool RandomItemMgr::CanEquipArmor(uint8 clazz, uint32 level, ItemTemplate const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if ((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN || clazz == CLASS_SHAMAN) &&
        proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        return true;

    if ((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN) && level >= 40)
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_PLATE && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN) && level < 40) ||
        ((clazz == CLASS_HUNTER || clazz == CLASS_SHAMAN) && level >= 40))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_MAIL && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (((clazz == CLASS_HUNTER || clazz == CLASS_SHAMAN) && level < 40) ||
        (clazz == CLASS_DRUID || clazz == CLASS_ROGUE))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_LEATHER && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (proto->Quality <= ITEM_QUALITY_NORMAL)
        return true;

    uint8 sp = 0, ap = 0, tank = 0;
    for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        // for ItemStatValue != 0
        if (!proto->ItemStat[j].ItemStatValue)
            continue;

        AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    }

    return CheckItemStats(clazz, sp, ap, tank);
}

bool RandomItemMgr::CanEquipWeapon(uint8 clazz, ItemTemplate const* proto)
{
    switch (clazz)
    {
        case CLASS_PRIEST:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER)
                return false;
            break;
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_WARRIOR:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                return false;
            break;
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_SHAMAN:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                return false;
            break;
        case CLASS_DRUID:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM)
                return false;
            break;
        case CLASS_HUNTER:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM)
                return false;
            break;
        case CLASS_ROGUE:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE)
                return false;
            break;
    }

    return true;
}

void RandomItemMgr::BuildItemInfoCache()
{
    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    // load weightscales
    LOG_INFO("playerbots", "Loading weightscales info");

    uint32 counter = 1;
    uint32 totalcount = 0;
    uint32 statcount = 0;
    uint32 curClass = CLASS_WARRIOR;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_WEIGHTSCALES);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].Get<uint32>();
            std::string name = fields[1].Get<std::string>();
            uint8 clazz = fields[2].Get<uint8>();

            if (clazz != curClass)
            {
                counter = 1;
                curClass = clazz;
            }

            WeightScale scale;
            scale.info.id = id;
            scale.info.name = name;
            m_weightScales[clazz][counter] = std::move(scale);
            counter++;
            totalcount++;

        } while (result->NextRow());

        LOG_INFO("playerbots", "Loaded {} weightscale class specs", totalcount);
    }

    stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_WEIGHTSCALE_DATA);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].Get<uint32>();
            std::string field = fields[1].Get<std::string>();
            uint32 weight = fields[2].Get<uint32>();

            WeightScaleStat stat;
            stat.stat = field;
            stat.weight = weight;

            for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
            {
                for (uint32 spec = 1; spec < 5; ++spec)
                {
                    if (m_weightScales[cls][spec].info.id == id)
                        m_weightScales[cls][spec].stats.push_back(std::move(stat));
                }
            }
            statcount++;

        } while (result->NextRow());

        LOG_INFO("playerbots", "Loaded {} weightscale stat weights", statcount);
    }

    if (m_weightScales[1].empty())
    {
        LOG_ERROR("playerbots", "Error loading item weight scales");
        return;
    }

    // vendor items
    LOG_INFO("playerbots", "Loading vendor item list...");

    std::set<uint32> vendorItems;
    vendorItems.clear();
    if (QueryResult result = WorldDatabase.Query("SELECT item FROM npc_vendor"))
    {
        do
        {
            Field* fields = result->Fetch();
            int32 entry = fields[0].Get<int32>();
            if (entry <= 0)
                continue;

            vendorItems.insert(entry);
        } while (result->NextRow());
    }

    LOG_INFO("playerbots", "Loaded {} vendor items...", vendorItems.size());

    // calculate drop source
    LOG_INFO("playerbots", "Loading loot templates...");
    DropMap* dropMap = new DropMap;

    if (CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates())
    {
        for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            uint32 sEntry = itr->first;
            if (LootTemplateAccess const* lTemplateA =
                    DropMapValue::GetLootTemplate(ObjectGuid::Create<HighGuid::Unit>(sEntry, uint32(1)), LOOT_CORPSE))
                for (auto const& lItem : lTemplateA->Entries)
                    dropMap->insert(std::make_pair(lItem->itemid, sEntry));
        }
    }

    if (GameObjectTemplateContainer const* gameobjects = sObjectMgr->GetGameObjectTemplates())
    {
        for (auto const& itr : *gameobjects)
        {
            uint32 sEntry = itr.first;
            if (LootTemplateAccess const* lTemplateA = DropMapValue::GetLootTemplate(
                    ObjectGuid::Create<HighGuid::GameObject>(sEntry, uint32(1)), LOOT_CORPSE))
                for (auto const& lItem : lTemplateA->Entries)
                    dropMap->insert(std::make_pair(lItem->itemid, sEntry));
        }
    }

    LOG_INFO("playerbots", "Loaded {} loot templates...", dropMap->size());

    ItemTemplateContainer const* itemTemplate = sObjectMgr->GetItemTemplateStore();
    LOG_INFO("playerbots", "Calculating stat weights for {} items...", itemTemplate->size());

    PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();

    for (auto const& itr : *itemTemplate)
    {
        ItemTemplate const* proto = &itr.second;
        if (!proto)
            continue;

        // skip test items
        if (strstr(proto->Name1.c_str(), "(Test)") || strstr(proto->Name1.c_str(), "(TEST)") ||
            strstr(proto->Name1.c_str(), "(test)") || strstr(proto->Name1.c_str(), "(JEFFTEST)") ||
            strstr(proto->Name1.c_str(), "Test ") || strstr(proto->Name1.c_str(), "Test") ||
            strstr(proto->Name1.c_str(), "TEST") || strstr(proto->Name1.c_str(), "TEST ") ||
            strstr(proto->Name1.c_str(), " TEST") || strstr(proto->Name1.c_str(), "2200 ") ||
            strstr(proto->Name1.c_str(), "Deprecated ") || strstr(proto->Name1.c_str(), "Unused ") ||
            strstr(proto->Name1.c_str(), "Monster ") || strstr(proto->Name1.c_str(), "[PH]") ||
            strstr(proto->Name1.c_str(), "(OLD)") || strstr(proto->Name1.c_str(), "QR") ||
            strstr(proto->Name1.c_str(), "zzOLD"))
        {
            itemForTest.insert(proto->ItemId);
            continue;
        }

        if (proto->Flags & ITEM_FLAG_DEPRECATED)
        {
            itemForTest.insert(proto->ItemId);
            continue;
        }
        // skip items with rank/rep requirements
        /*if (proto->RequiredHonorRank > 0 ||
            proto->RequiredSkillRank > 0 ||
            proto->RequiredCityRank > 0 ||
            proto->RequiredReputationRank > 0)
            continue;*/

        // if (proto->RequiredHonorRank > 0 || proto->RequiredSkillRank > 0 || proto->RequiredCityRank > 0)
        //     continue;

        // // skip random enchant items
        // if (proto->RandomProperty)
        //     continue;

        // // skip heirloom items
        // if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
        //     continue;

        // // check possible equip slots
        // EquipmentSlots slot = EQUIPMENT_SLOT_START;
        // for (std::map<EquipmentSlots, std::set<InventoryType> >::iterator i = viableSlots.begin();
        //      i != viableSlots.end(); ++i)
        // {
        //     std::set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
        //     if (slots.find((InventoryType)proto->InventoryType) != slots.end())
        //         slot = i->first;
        // }

        // if (slot == EQUIPMENT_SLOT_START)
        //     continue;

        // Init Item cache
        // ItemInfoEntry cacheInfo;

        // for (uint8 clazz = CLASS_WARRIOR; clazz < MAX_CLASSES; ++clazz)
        // {
        //     // skip nonexistent classes
        //     if (!((1 << (clazz - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(clazz))
        //         continue;

        //     // skip wrong classes
        //     if ((proto->AllowableClass & (1 << (clazz - 1))) == 0)
        //         continue;

        //     for (uint32 spec = 1; spec < 5; ++spec)
        //     {
        //         if (!m_weightScales[clazz][spec].info.id)
        //             continue;

        //         // check possible armor for spec
        //         if (m_weightScales)
        //         if (proto->Class == ITEM_CLASS_ARMOR && (
        //             slot == EQUIPMENT_SLOT_HEAD ||
        //             slot == EQUIPMENT_SLOT_SHOULDERS ||
        //             slot == EQUIPMENT_SLOT_CHEST ||
        //             slot == EQUIPMENT_SLOT_WAIST ||
        //             slot == EQUIPMENT_SLOT_LEGS ||
        //             slot == EQUIPMENT_SLOT_FEET ||
        //             slot == EQUIPMENT_SLOT_WRISTS ||
        //             slot == EQUIPMENT_SLOT_HANDS) &&
        //             !ShouldEquipArmorForSpec(clazz, spec, proto))
        //             continue;

        //         // check possible weapon for spec
        //         if ((proto->Class == ITEM_CLASS_WEAPON || (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD ||
        //         (proto->SubClass == ITEM_SUBCLASS_ARMOR_MISC && proto->InventoryType == INVTYPE_HOLDABLE))) &&
        //             !ShouldEquipWeaponForSpec(clazz, spec, proto))
        //             continue;

        //         StatWeight statWeight;
        //         statWeight.id = m_weightScales[clazz][spec].info.id;
        //         uint32 statW = CalculateStatWeight(clazz, spec, proto);
        //         // set stat weight = 1 for items that can be equipped but have no proper stats
        //         statWeight.weight = statW ? statW : 1;
        //         //statWeight.weight = statW;
        //         // save item statWeight into ItemCache
        //         cacheInfo.weights[statWeight.id] = statWeight.weight;
        //         LOG_DEBUG("playerbots", "Item: {}, weight: {}, class: {}, spec: {}", proto->ItemId,
        //         statWeight.weight, clazz, m_weightScales[clazz][spec].info.name);
        //     }
        // }

        // cacheInfo.team = TEAM_NEUTRAL;

        // // check faction
        // if (proto->Flags2 & ITEM_FLAG2_FACTION_HORDE)
        //     cacheInfo.team = TEAM_HORDE;

        // if (proto->Flags2 & ITEM_FLAG2_FACTION_ALLIANCE)
        //     cacheInfo.team = TEAM_ALLIANCE;

        // if (cacheInfo.team == TEAM_NEUTRAL && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        // {
        //     if (FactionEntry const* faction = sFactionStore.LookupEntry(HORDE))
        //         if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
        //             cacheInfo.team = TEAM_HORDE;

        //     if (FactionEntry const* faction = sFactionStore.LookupEntry(ALLIANCE))
        //         if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
        //             cacheInfo.team = TEAM_ALLIANCE;
        // }

        // if (cacheInfo.team < TEAM_NEUTRAL)
        //     LOG_DEBUG("playerbots", "Item: {}, team (item): {}", proto->ItemId, cacheInfo.team == TEAM_ALLIANCE ?
        //     "Alliance" : "Horde");

        // // check min level
        // if (proto->RequiredLevel)
        //     cacheInfo.minLevel = proto->RequiredLevel;

        // // check item source

        // if (proto->Flags & ITEM_FLAG_NO_DISENCHANT)
        // {
        //     cacheInfo.source = ITEM_SOURCE_PVP;
        //     LOG_DEBUG("playerbots", "Item: {}, source: PvP Reward", proto->ItemId);
        // }

        // // check quests
        // if (cacheInfo.source == ITEM_SOURCE_NONE)
        // {
        //     std::vector<uint32> questIds = GetQuestIdsForItem(proto->ItemId);
        //     if (questIds.size())
        //     {
        //         bool isAlly = false;
        //         bool isHorde = false;
        //         for (std::vector<uint32>::iterator i = questIds.begin(); i != questIds.end(); ++i)
        //         {
        //             Quest const* quest = sObjectMgr->GetQuestTemplate(*i);
        //             if (quest)
        //             {
        //                 cacheInfo.source = ITEM_SOURCE_QUEST;
        //                 cacheInfo.sourceId = *i;
        //                 if (!cacheInfo.minLevel)
        //                     cacheInfo.minLevel = quest->GetMinLevel();

        //                 // check quest team
        //                 if (cacheInfo.team == TEAM_NEUTRAL)
        //                 {
        //                     uint32 reqRace = quest->GetAllowableRaces();
        //                     if (reqRace)
        //                     {
        //                         if ((reqRace & RACEMASK_ALLIANCE) != 0)
        //                             isAlly = true;
        //                         else if ((reqRace & RACEMASK_HORDE) != 0)
        //                             isHorde = true;
        //                     }
        //                 }
        //             }
        //         }

        //         if (isAlly && isHorde)
        //             cacheInfo.team = TEAM_NEUTRAL;
        //         else if (isAlly)
        //             cacheInfo.team = TEAM_ALLIANCE;
        //         else if (isHorde)
        //             cacheInfo.team = TEAM_HORDE;

        //         LOG_DEBUG("playerbots", "Item: {}, team (quest): {}", proto->ItemId, cacheInfo.team == TEAM_ALLIANCE
        //         ? "Alliance" : cacheInfo.team == TEAM_HORDE ? "Horde" : "Both"); LOG_DEBUG("playerbots", "Item: {},
        //         source: quest {}, minlevel: {}", proto->ItemId, cacheInfo.sourceId, cacheInfo.minLevel);
        //     }
        // }

        // if (cacheInfo.minLevel)
        //     LOG_DEBUG("playerbots", "Item: {}, minlevel: {}", proto->ItemId, cacheInfo.minLevel);

        // // check vendors
        // if (cacheInfo.source == ITEM_SOURCE_NONE)
        // {
        //     for (std::set<uint32>::iterator i = vendorItems.begin(); i != vendorItems.end(); ++i)
        //     {
        //         if (proto->ItemId == *i)
        //         {
        //             cacheInfo.source = ITEM_SOURCE_VENDOR;
        //             LOG_DEBUG("playerbots", "Item: {} source: vendor", proto->ItemId);
        //             break;
        //         }
        //     }
        // }

        // // check drops
        // std::vector<int32> creatures;
        // std::vector<int32> gameobjects;
        // auto range = dropMap->equal_range(itr.first);

        // for (auto iter = range.first; iter != range.second; ++iter)
        // {
        //     if (iter->second > 0)
        //         creatures.push_back(iter->second);
        //     else
        //         gameobjects.push_back(abs(iter->second));
        // }

        // // check creature drop
        // if (cacheInfo.source == ITEM_SOURCE_NONE)
        // {
        //     if (creatures.size())
        //     {
        //         if (creatures.size() == 1)
        //         {
        //             cacheInfo.source = ITEM_SOURCE_DROP;
        //             cacheInfo.sourceId = creatures.front();
        //             LOG_DEBUG("playerbots", "Item: {}, source: creature drop, ID: {}", proto->ItemId,
        //             creatures.front());
        //         }
        //         else
        //         {
        //             cacheInfo.source = ITEM_SOURCE_DROP;
        //             LOG_DEBUG("playerbots", "Item: {}, source: creatures drop, number: {}", proto->ItemId,
        //             creatures.size());
        //         }
        //     }
        // }

        // // check gameobject drop
        // if (cacheInfo.source == ITEM_SOURCE_NONE || (cacheInfo.source == ITEM_SOURCE_DROP && !cacheInfo.sourceId))
        // {
        //     if (gameobjects.size())
        //     {
        //         if (gameobjects.size() == 1)
        //         {
        //             cacheInfo.source = ITEM_SOURCE_DROP;
        //             cacheInfo.sourceId = gameobjects.front();
        //             LOG_INFO("playerbots", "Item: {}, source: gameobject, ID: {}", proto->ItemId,
        //             gameobjects.front());
        //         }
        //         else
        //         {
        //             cacheInfo.source = ITEM_SOURCE_DROP;
        //             LOG_INFO("playerbots", "Item: {}, source: gameobjects, number: {}", proto->ItemId,
        //             gameobjects.size());
        //         }
        //     }
        // }

        // // check faction
        // if (proto->RequiredReputationFaction > 0 && proto->RequiredReputationFaction != 35 &&
        // proto->RequiredReputationRank < 15)
        // {
        //     cacheInfo.repFaction = proto->RequiredReputationFaction;
        //     cacheInfo.repRank = proto->RequiredReputationRank;
        // }

        // cacheInfo.quality = proto->Quality;
        // cacheInfo.itemId = proto->ItemId;
        // cacheInfo.slot = slot;

        // // save cache
        // PlayerbotsDatabasePreparedStatement* stmt =
        // PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_EQUIP_CACHE_NEW); stmt->SetData(0, proto->ItemId);
        // trans->Append(stmt);

        // stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_EQUIP_CACHE_NEW);
        // stmt->SetData(0, cacheInfo.itemId);
        // stmt->SetData(1, cacheInfo.quality);
        // stmt->SetData(2, cacheInfo.slot);
        // stmt->SetData(3, cacheInfo.source);
        // stmt->SetData(4, cacheInfo.sourceId);
        // stmt->SetData(5, cacheInfo.team);
        // stmt->SetData(6, cacheInfo.repFaction);
        // stmt->SetData(7, cacheInfo.repRank);
        // stmt->SetData(8, cacheInfo.minLevel);

        // for (uint8 i = 1; i <= MAX_STAT_SCALES; ++i)
        // {
        //     if (cacheInfo.weights[i])
        //         stmt->SetData(8 + i, cacheInfo.weights[i]);
        //     else
        //         stmt->SetData(8 + i, 0);
        // }

        // trans->Append(stmt);

        // itemInfoCache[cacheInfo.itemId] = std::move(cacheInfo);
    }

    PlayerbotsDatabase.CommitTransaction(trans);
}

uint32 RandomItemMgr::CalculateStatWeight(uint8 playerclass, uint8 spec, ItemTemplate const* proto)
{
    uint32 statWeight = 0;
    bool isCasterItem = false;
    bool isAttackItem = false;
    bool noCaster = (Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE ||
                    (Classes)playerclass == CLASS_DEATH_KNIGHT || (Classes)playerclass == CLASS_HUNTER;
    uint32 spellPower = 0;
    uint32 spellHeal = 0;
    uint32 attackPower = 0;
    bool hasInt = false;
    bool hasMana = !((Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE ||
                     (Classes)playerclass == CLASS_DEATH_KNIGHT);

    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_LIBRAM || proto->SubClass == ITEM_SUBCLASS_ARMOR_IDOL ||
        proto->SubClass == ITEM_SUBCLASS_ARMOR_TOTEM || proto->SubClass == ITEM_SUBCLASS_ARMOR_SIGIL)
        return (uint32)(proto->Quality + proto->ItemLevel);

    // check basic item stats
    int32 basicStatsWeight = 0;
    for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        uint32 statType;
        int32 val;
        std::string weightName;

        // if (j >= proto->StatsCount)
        //     continue;

        statType = proto->ItemStat[j].ItemStatType;
        val = proto->ItemStat[j].ItemStatValue;

        if (val == 0)
            continue;

        for (std::map<std::string, uint32>::iterator i = weightStatLink.begin(); i != weightStatLink.end(); ++i)
        {
            uint32 modd = i->second;
            if (modd == statType)
            {
                weightName = i->first;
                break;
            }
        }

        if (weightName.empty())
            continue;

        uint32 singleStat = CalculateSingleStatWeight(playerclass, spec, weightName, val);
        basicStatsWeight += singleStat;

        if (val)
        {
            if (weightName == "int" && !noCaster)
                isCasterItem = true;

            if (weightName == "int")
                hasInt = true;

            if (weightName == "splpwr")
                isCasterItem = true;

            if (weightName == "str")
                isAttackItem = true;

            if (weightName == "agi")
                isAttackItem = true;

            if (weightName == "atkpwr")
                isAttackItem = true;
        }
    }

    // check armor & block
    statWeight += CalculateSingleStatWeight(playerclass, spec, "armor", proto->Armor);
    statWeight += CalculateSingleStatWeight(playerclass, spec, "block", proto->Block);

    // check weapon dps
    if (proto->IsWeaponVellum())
    {
        WeaponAttackType attType = BASE_ATTACK;

        uint32 dps = 0;
        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
        {
            if (proto->Damage[i].DamageMax == 0)
                break;

            dps = (proto->Damage[i].DamageMin + proto->Damage[i].DamageMax) / (float)(proto->Delay / 1000.0f) / 2;
            if (dps)
            {
                if (proto->IsRangedWeapon())
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "rgddps", dps);
                else
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "mledps", dps);
            }
        }
    }

    // check item spells
    for (const auto& spellData : proto->Spells)
    {
        // no spell
        if (!spellData.SpellId)
            continue;

        // apply only at-equip spells
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_EQUIP)
            continue;

        // check if it is valid spell
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellData.SpellId);
        if (!spellInfo)
            continue;

        uint32 spellDamage = 0;
        uint32 spellHealing = 0;
        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; j++)
        {
            if (spellInfo->Effects[j].Effect == SPELL_EFFECT_APPLY_AURA && spellInfo->Effects[j].BasePoints)
            {
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    spellDamage = spellInfo->Effects[j].BasePoints + 1;
                }

                // spell healing
                // SPELL_AURA_MOD_HEALING_DONE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_HEALING_DONE)
                {
                    spellHealing = spellInfo->Effects[j].BasePoints + 1;
                }

                // check spell power
                if (spellDamage && spellDamage == spellHealing)
                {
                    isCasterItem = true;
                    spellPower += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                }

                // spell hit rating (pre tbc)
                // SPELL_AURA_MOD_SPELL_HIT_CHANCE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_SPELL_HIT_CHANCE)
                {
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "spellhitrtng",
                                                            spellInfo->Effects[j].BasePoints + 1);
                }

                // spell crit rating (pre tbc)
                // SPELL_AURA_MOD_SPELL_CRIT_CHANCE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_SPELL_CRIT_CHANCE)
                {
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "spellcritstrkrtng",
                                                            spellInfo->Effects[j].BasePoints + 1);
                }

                // spell penetration
                // SPELL_AURA_MOD_TARGET_RESISTANCE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_TARGET_RESISTANCE)
                {
                    // check if magic type
                    if (spellInfo->Effects[j].MiscValue == SPELL_SCHOOL_MASK_SPELL)
                        statWeight += CalculateSingleStatWeight(playerclass, spec, "spellpenrtng",
                                                                abs(spellInfo->Effects[j].BasePoints + 1));
                }

                // check attack power
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_ATTACK_POWER)
                {
                    isAttackItem = true;
                    attackPower +=
                        CalculateSingleStatWeight(playerclass, spec, "atkpwr", spellInfo->Effects[j].BasePoints + 1);
                }

                // check ranged ap
                // SPELL_AURA_MOD_RANGED_ATTACK_POWER
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_RANGED_ATTACK_POWER)
                {
                    isAttackItem = true;
                    attackPower +=
                        CalculateSingleStatWeight(playerclass, spec, "atkpwr", spellInfo->Effects[j].BasePoints + 1);
                }

                // check block
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_SHIELD_BLOCKVALUE)
                {
                    statWeight +=
                        CalculateSingleStatWeight(playerclass, spec, "block", spellInfo->Effects[j].BasePoints + 1);
                }

                // block chance
                // SPELL_AURA_MOD_BLOCK_PERCENT
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_BLOCK_PERCENT)
                {
                    statWeight +=
                        CalculateSingleStatWeight(playerclass, spec, "blockrtng", spellInfo->Effects[j].BasePoints + 1);
                }

                // armor penetration
                // SPELL_AURA_MOD_TARGET_RESISTANCE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_TARGET_RESISTANCE)
                {
                    // check if physical type
                    if (spellInfo->Effects[j].MiscValue == SPELL_SCHOOL_MASK_NORMAL)
                        statWeight += CalculateSingleStatWeight(playerclass, spec, "armorpenrtng",
                                                                abs(spellInfo->Effects[j].BasePoints + 1));
                }

                // hit rating (pre tbc)
                // SPELL_AURA_MOD_HIT_CHANCE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_HIT_CHANCE)
                {
                    statWeight +=
                        CalculateSingleStatWeight(playerclass, spec, "hitrtng", spellInfo->Effects[j].BasePoints + 1);
                }

                // crit rating (pre tbc)
                // SPELL_AURA_MOD_HIT_CHANCE
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_CRIT_PCT)
                {
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "critstrkrtng",
                                                            spellInfo->Effects[j].BasePoints + 1);
                }

                // check defense SPELL_AURA_MOD_SKILL
                // check block
                // if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_SKILL)
                //{
                //    statWeight += CalculateSingleStatWeight(playerclass, spec, "block",
                //    spellInfo->Effects[j].BasePoints + 1);
                //}

                // ratings
                // enum CombatRating
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_RATING)
                {
                    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
                    {
                        if (spellInfo->Effects[j].MiscValue & (1 << rating))
                        {
                            int32 val = spellInfo->Effects[j].BasePoints + 1;
                            std::string weightName;

                            for (std::map<std::string, uint32>::iterator i = weightRatingLink.begin();
                                 i != weightRatingLink.end(); ++i)
                            {
                                uint32 modd = i->second;
                                if (modd == rating)
                                {
                                    weightName = i->first;
                                    break;
                                }
                            }

                            if (weightName.empty())
                                continue;

                            statWeight += CalculateSingleStatWeight(playerclass, spec, weightName, val);
                        }
                    }
                }

                // mana regen
                // SPELL_AURA_MOD_POWER_REGEN
                if (spellInfo->Effects[j].ApplyAuraName == SPELL_AURA_MOD_POWER_REGEN)
                {
                    statWeight +=
                        CalculateSingleStatWeight(playerclass, spec, "manargn", spellInfo->Effects[j].BasePoints + 1);
                }
            }
        }
    }

    // check for caster item
    if (isCasterItem || hasInt)
    {
        if ((!hasMana || noCaster) && spellPower)
            return 0;

        if (!hasMana && hasInt)
            return 0;

        if ((!hasMana && noCaster && playerclass != CLASS_PALADIN) && spellPower > attackPower)
            return 0;

        bool playerCaster = false;
        for (std::vector<WeightScaleStat>::iterator i = m_weightScales[playerclass][spec].stats.begin();
             i != m_weightScales[playerclass][spec].stats.end(); ++i)
        {
            if (i->stat == "splpwr" || i->stat == "int" || i->stat == "manargn")
            {
                playerCaster = true;
            }
        }

        if (!playerCaster)
            return 0;
    }

    // check for caster item
    if (isAttackItem)
    {
        if (hasMana && !noCaster && !(hasInt || spellPower))
            return 0;
        // if (!noCaster && attackPower)
        //     return 0;

        bool playerAttacker = false;
        for (std::vector<WeightScaleStat>::iterator i = m_weightScales[playerclass][spec].stats.begin();
             i != m_weightScales[playerclass][spec].stats.end(); ++i)
        {
            if (i->stat == "str" || i->stat == "agi" || i->stat == "atkpwr" || i->stat == "mledps" ||
                i->stat == "rgddps")
            {
                playerAttacker = true;
            }
        }

        if (!playerAttacker)
            return 0;
    }

    statWeight += spellPower;
    statWeight += spellHeal;
    statWeight += attackPower;

    // handle negative stats
    if (basicStatsWeight < 0 && (abs(basicStatsWeight) >= statWeight))
        statWeight = 0;
    else
        statWeight += basicStatsWeight;

    return statWeight;
}

uint32 RandomItemMgr::CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value)
{
    uint32 statWeight = 0;
    for (std::vector<WeightScaleStat>::iterator i = m_weightScales[playerclass][spec].stats.begin();
         i != m_weightScales[playerclass][spec].stats.end(); ++i)
    {
        if (stat == i->stat)
        {
            statWeight = i->weight * value;
            // if (statWeight)
            //     LOG_INFO("playerbots", "stat: {}, val: {}, weight: {}, total: {}, class: {}, spec: {}",
            //         stat, value, i->weight, statWeight, playerclass, m_weightScales[playerclass][spec].info.name);
            return statWeight;
        }
    }

    return statWeight;
}

uint32 RandomItemMgr::GetQuestIdForItem(uint32 itemId)
{
    bool isQuest = false;
    uint32 questId = 0;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewardItemId[i])
                continue;

            if (quest->RewardItemId[i] == itemId)
            {
                isQuest = true;
                questId = quest->GetQuestId();
                break;
            }
        }

        uint32 rewChocieItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChocieItemCount; ++i)
        {
            if (!quest->RewardChoiceItemId[i])
                continue;

            if (quest->RewardChoiceItemId[i] == itemId)
            {
                isQuest = true;
                questId = quest->GetQuestId();
                break;
            }
        }
        if (isQuest)
            break;
    }
    return questId;
}

std::vector<uint32> RandomItemMgr::GetQuestIdsForItem(uint32 itemId)
{
    std::vector<uint32> questIds;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewardItemId[i])
                continue;

            if (quest->RewardItemId[i] == itemId)
            {
                questIds.push_back(quest->GetQuestId());
                break;
            }
        }

        uint32 rewChocieItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChocieItemCount; ++i)
        {
            if (!quest->RewardChoiceItemId[i])
                continue;

            if (quest->RewardChoiceItemId[i] == itemId)
            {
                questIds.push_back(quest->GetQuestId());
                break;
            }
        }
    }

    return std::move(questIds);
}

uint32 RandomItemMgr::GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId)
{
    if (!player)
        return 0;

    // get old item statWeight
    uint32 oldStatWeight = 0;
    uint32 specId = 0;
    uint32 closestUpgrade = 0;
    uint32 closestUpgradeWeight = 0;
    std::vector<uint32> classspecs;

    for (uint32 specNum = 1; specNum < 5; ++specNum)
    {
        if (!m_weightScales[player->getClass()][specNum].info.id)
            continue;

        classspecs.push_back(m_weightScales[player->getClass()][specNum].info.id);

        if (m_weightScales[player->getClass()][specNum].info.name == spec)
            specId = m_weightScales[player->getClass()][specNum].info.id;
    }
    if (!specId)
        return 0;

    if (itemId && itemInfoCache.find(itemId) != itemInfoCache.end())
    {
        oldStatWeight = itemInfoCache[itemId].weights[specId];

        if (oldStatWeight)
            LOG_INFO("playerbots", "Old Item: {}, weight: {}", itemId, oldStatWeight);
        else
            LOG_INFO("playerbots", "Old item has no stat weight");
    }

    for (std::map<uint32, ItemInfoEntry>::iterator i = itemInfoCache.begin(); i != itemInfoCache.end(); ++i)
    {
        ItemInfoEntry& info = i->second;

        // skip useless items
        if (info.weights[specId] == 0)
            continue;

        // skip higher lvl
        if (info.minLevel > player->GetLevel())
            continue;

        // skip too low level
        if (info.minLevel < (player->GetLevel() - 10))
            continue;

        // skip wrong team
        if (info.team != TEAM_NEUTRAL && info.team != player->GetTeamId())
            continue;

        // skip wrong slot
        if ((EquipmentSlots)info.slot != (EquipmentSlots)slot)
            continue;

        // skip higher quality
        if (quality && info.quality != quality)
            continue;

        // skip worse items
        if (info.weights[specId] <= oldStatWeight)
            continue;

        // skip items that only fit in slot, but not stats
        if (!itemId && info.weights[specId] == 1 && player->GetLevel() > 40)
            continue;

        // skip quest items
        if (info.source == ITEM_SOURCE_QUEST)
        {
            if (player->GetQuestRewardStatus(info.sourceId) != QUEST_STATUS_COMPLETE)
                continue;
        }

        // skip no stats trinkets
        if (info.weights[specId] == 1 && info.slot == EQUIPMENT_SLOT_NECK || info.slot == EQUIPMENT_SLOT_TRINKET1 ||
            info.slot == EQUIPMENT_SLOT_TRINKET2 || info.slot == EQUIPMENT_SLOT_FINGER1 ||
            info.slot == EQUIPMENT_SLOT_FINGER2)
            continue;

        // check if item stat score is the best among class specs
        uint32 bestSpecId = 0;
        uint32 bestSpecScore = 0;
        for (std::vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
        {
            if (info.weights[*i] > bestSpecScore)
            {
                bestSpecId = *i;
                bestSpecScore = info.weights[specId];
            }
        }

        if (bestSpecId && bestSpecId != specId && player->GetLevel() > 40)
            return 0;

        if (!closestUpgrade)
        {
            closestUpgrade = info.itemId;
            closestUpgradeWeight = info.weights[specId];
        }

        // pick closest upgrade
        if (info.weights[specId] < closestUpgradeWeight)
        {
            closestUpgrade = info.itemId;
            closestUpgradeWeight = info.weights[specId];
        }
    }

    if (closestUpgrade)
        LOG_INFO("playerbots", "New Item: {}, weight: {}", closestUpgrade, closestUpgradeWeight);

    return closestUpgrade;
}

std::vector<uint32> RandomItemMgr::GetUpgradeList(Player* player, std::string spec, uint8 slot, uint32 quality,
                                                  uint32 itemId, uint32 amount)
{
    std::vector<uint32> listItems;
    if (!player)
        return std::move(listItems);

    // get old item statWeight
    uint32 oldStatWeight = 0;
    uint32 specId = 0;
    uint32 closestUpgrade = 0;
    uint32 closestUpgradeWeight = 0;
    std::vector<uint32> classspecs;

    for (uint32 specNum = 1; specNum < 5; ++specNum)
    {
        if (!m_weightScales[player->getClass()][specNum].info.id)
            continue;

        classspecs.push_back(m_weightScales[player->getClass()][specNum].info.id);

        if (m_weightScales[player->getClass()][specNum].info.name == spec)
            specId = m_weightScales[player->getClass()][specNum].info.id;
    }

    if (!specId)
        return std::move(listItems);

    if (itemId && itemInfoCache.find(itemId) != itemInfoCache.end())
    {
        oldStatWeight = itemInfoCache[itemId].weights[specId];

        if (oldStatWeight)
            LOG_INFO("playerbots", "Old Item: {}, weight: {}", itemId, oldStatWeight);
        else
            LOG_INFO("playerbots", "Old item has no stat weight");
    }

    for (std::map<uint32, ItemInfoEntry>::iterator i = itemInfoCache.begin(); i != itemInfoCache.end(); ++i)
    {
        ItemInfoEntry& info = i->second;

        // skip useless items
        if (info.weights[specId] == 0)
            continue;

        // skip higher lvl
        if (info.minLevel > player->GetLevel())
            continue;

        // skip too low level
        if ((int32)info.minLevel < (int32)(player->GetLevel() - 20))
            continue;

        // skip wrong team
        if (info.team != TEAM_NEUTRAL && info.team != player->GetTeamId())
            continue;

        // skip wrong slot
        if ((EquipmentSlots)info.slot != (EquipmentSlots)slot)
            continue;

        // skip higher quality
        if (quality && info.quality != quality)
            continue;

        // skip worse items
        if (info.weights[specId] <= oldStatWeight)
            continue;

        // skip items that only fit in slot, but not stats
        if (!itemId && info.weights[specId] == 1 && player->GetLevel() > 40)
            continue;

        // skip quest items
        if (info.source == ITEM_SOURCE_QUEST)
        {
            if (player->GetQuestRewardStatus(info.sourceId) != QUEST_STATUS_COMPLETE)
                continue;
        }

        // skip no stats trinkets
        if (info.weights[specId] < 2 && (info.slot == EQUIPMENT_SLOT_NECK || info.slot == EQUIPMENT_SLOT_TRINKET1 ||
                                         info.slot == EQUIPMENT_SLOT_TRINKET2 || info.slot == EQUIPMENT_SLOT_FINGER1 ||
                                         info.slot == EQUIPMENT_SLOT_FINGER2))
            continue;

        // if (player->GetLevel() >= 40)
        //{
        //     // check if item stat score is the best among class specs
        //     uint32 bestSpecId = 0;
        //     uint32 bestSpecScore = 0;
        //     for (vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
        //     {
        //         if (info->weights[*i] > bestSpecScore)
        //         {
        //             bestSpecId = *i;
        //             bestSpecScore = info->weights[specId];
        //         }
        //     }

        //    if (bestSpecId && bestSpecId != specId)
        //        continue;
        //}

        listItems.push_back(info.itemId);
        // continue;

        // pick closest upgrade
        if (info.weights[specId] > closestUpgradeWeight)
        {
            closestUpgrade = info.itemId;
            closestUpgradeWeight = info.weights[specId];
        }
    }

    if (listItems.size())
        LOG_INFO("playerbots", "New Items: {}, Old item:%d, New items max: {}", listItems.size(), oldStatWeight,
                 closestUpgradeWeight);

    return std::move(listItems);
}

bool RandomItemMgr::HasStatWeight(uint32 itemId)
{
    auto itr = itemInfoCache.find(itemId);
    return itr != itemInfoCache.end();
}

uint32 RandomItemMgr::GetMinLevelFromCache(uint32 itemId)
{
    auto itr = itemInfoCache.find(itemId);
    if (itr == itemInfoCache.end())
        return 0;

    return itr->second.minLevel;
}

uint32 RandomItemMgr::GetStatWeight(Player* player, uint32 itemId)
{
    if (!player || !itemId)
        return 0;

    auto itr = itemInfoCache.find(itemId);
    if (itr == itemInfoCache.end())
        return 0;

    uint32 statWeight = 0;
    uint32 specId = 0;
    std::vector<uint32> classspecs;

    std::string const& specName = AiFactory::GetPlayerSpecName(player);
    if (specName.empty())
        return 0;

    for (uint32 specNum = 1; specNum < 5; ++specNum)
    {
        if (!m_weightScales[player->getClass()][specNum].info.id)
            continue;

        classspecs.push_back(m_weightScales[player->getClass()][specNum].info.id);

        if (m_weightScales[player->getClass()][specNum].info.name == specName)
            specId = m_weightScales[player->getClass()][specNum].info.id;

        if (m_weightScales[player->getClass()][specNum].info.name == specName)
        {
            specId = specNum;
            break;
        }
    }

    if (!specId)
        return 0;

    statWeight = itr->second.weights[specId];

    return statWeight;
}

uint32 RandomItemMgr::GetLiveStatWeight(Player* player, uint32 itemId)
{
    if (!player || !itemId)
        return 0;

    auto itr = itemInfoCache.find(itemId);
    if (itr == itemInfoCache.end())
        return 0;

    uint32 statWeight = 0;
    uint32 specId = 0;
    std::vector<uint32> classspecs;

    std::string const& specName = AiFactory::GetPlayerSpecName(player);
    if (specName.empty())
        return 0;

    for (uint32 specNum = 1; specNum < 5; ++specNum)
    {
        if (!m_weightScales[player->getClass()][specNum].info.id)
            continue;

        // for bestSpec check
        // classspecs.push_back(m_weightScales[player->getClass()][specNum].info.id);

        if (m_weightScales[player->getClass()][specNum].info.name == specName)
            specId = m_weightScales[player->getClass()][specNum].info.id;
    }
    if (!specId)
        return 0;

    statWeight = itr->second.weights[specId];

    // skip higher lvl
    if (itr->second.minLevel > player->GetLevel())
        return 0;

    // skip too low level
    // if ((int32)info->minLevel < (int32)(player->GetLevel() - 20))
    //    return 0;

    // skip wrong team
    if (itr->second.team != TEAM_NEUTRAL && itr->second.team != player->GetTeamId())
        return 0;

    // skip quest items
    if (itr->second.source == ITEM_SOURCE_QUEST && itr->second.sourceId)
    {
        if (player->GetQuestRewardStatus(itr->second.sourceId) != QUEST_STATUS_COMPLETE)
            return 0;
    }

    // skip pvp items
    if (itr->second.source == ITEM_SOURCE_PVP)
    {
        if (!player->GetHonorPoints() && !player->GetArenaPoints())
            return 0;
    }

    // skip no stats trinkets
    if (itr->second.weights[specId] == 1 &&
        (itr->second.slot == EQUIPMENT_SLOT_NECK || itr->second.slot == EQUIPMENT_SLOT_TRINKET1 ||
         itr->second.slot == EQUIPMENT_SLOT_TRINKET2 || itr->second.slot == EQUIPMENT_SLOT_FINGER1 ||
         itr->second.slot == EQUIPMENT_SLOT_FINGER2))
        return 0;

    // skip items that only fit in slot, but not stats
    if (!itemId && itr->second.weights[specId] == 1 && player->GetLevel() > 20)
        return 0;

    // check if item stat score is the best among class specs
    /*uint32 bestSpecId = 0;
    uint32 bestSpecScore = 0;
    for (vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
    {
        if (itemCache[itemId]->weights[*i] > bestSpecScore && itemCache[itemId]->weights[*i] > 1)
        {
            bestSpecId = *i;
            bestSpecScore = itemCache[itemId]->weights[specId];
        }
    }

    if (bestSpecId && bestSpecId != specId && player->GetLevel() >= 60)
        return 0;*/

    return statWeight;
}

void RandomItemMgr::BuildEquipCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_EQUIP_CACHE);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        LOG_INFO("server.loading",
                 "Loading equipment cache for {} classes, {} levels, {} slots, {} quality from {} items", MAX_CLASSES,
                 maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, itemTemplates->size());

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint8 clazz = fields[0].Get<uint8>();
            uint32 level = fields[1].Get<uint32>();
            uint8 slot = fields[2].Get<uint8>();
            uint32 quality = fields[3].Get<uint32>();
            uint32 itemId = fields[4].Get<uint32>();

            BotEquipKey key(level, clazz, slot, quality);
            equipCache[key].push_back(itemId);
            ++count;
        } while (result->NextRow());

        LOG_INFO("playerbots", "Equipment cache loaded from {} records", count);
    }
    else
    {
        uint64 total = MAX_CLASSES * maxLevel * EQUIPMENT_SLOT_END * ITEM_QUALITY_ARTIFACT;
        LOG_INFO("server.loading",
                 "Building equipment cache for {} classes, {} levels, {} slots, {} quality from {} items ({} total)",
                 MAX_CLASSES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, itemTemplates->size(), total);

        for (uint8 class_ = CLASS_WARRIOR; class_ < MAX_CLASSES; ++class_)
        {
            // skip nonexistent classes
            if (!((1 << (class_ - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(class_))
                continue;

            for (uint32 level = 1; level <= maxLevel; ++level)
            {
                for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    for (uint32 quality = ITEM_QUALITY_POOR; quality <= ITEM_QUALITY_ARTIFACT; ++quality)
                    {
                        BotEquipKey key(level, class_, slot, quality);

                        RandomItemList items;
                        for (auto const& itr : *itemTemplates)
                        {
                            ItemTemplate const* proto = &itr.second;
                            if (!proto)
                                continue;

                            if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR &&
                                proto->Class != ITEM_CLASS_CONTAINER && proto->Class != ITEM_CLASS_PROJECTILE)
                                continue;

                            if (!CanEquipItem(key, proto))
                                continue;

                            if (proto->Class == ITEM_CLASS_ARMOR &&
                                (slot == EQUIPMENT_SLOT_HEAD || slot == EQUIPMENT_SLOT_SHOULDERS ||
                                 slot == EQUIPMENT_SLOT_CHEST || slot == EQUIPMENT_SLOT_WAIST ||
                                 slot == EQUIPMENT_SLOT_LEGS || slot == EQUIPMENT_SLOT_FEET ||
                                 slot == EQUIPMENT_SLOT_WRISTS || slot == EQUIPMENT_SLOT_HANDS) &&
                                !CanEquipArmor(key.clazz, key.level, proto))
                                continue;

                            if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(key.clazz, proto))
                                continue;

                            if (slot == EQUIPMENT_SLOT_OFFHAND && key.clazz == CLASS_ROGUE &&
                                proto->Class != ITEM_CLASS_WEAPON)
                                continue;

                            items.push_back(itr.first);

                            PlayerbotsDatabasePreparedStatement* stmt =
                                PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_EQUIP_CACHE);
                            stmt->SetData(0, class_);
                            stmt->SetData(1, level);
                            stmt->SetData(2, slot);
                            stmt->SetData(3, quality);
                            stmt->SetData(4, proto->ItemId);
                            PlayerbotsDatabase.Execute(stmt);
                        }

                        equipCache[key] = items;

                        LOG_DEBUG("playerbots",
                                  "Equipment cache for class: {}, level {}, slot {}, quality {}: {} items", class_,
                                  level, slot, quality, items.size());
                    }
                }
            }
        }

        LOG_INFO("server.loading", "Equipment cache saved to DB");
    }
}

void RandomItemMgr::BuildEquipCacheNew()
{
    LOG_INFO("playerbots", "Loading equipments cache...");

    std::unordered_set<uint32> questItemIds;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 questId = i->first;
        Quest const* quest = i->second;

        if (quest->IsRepeatable())
            continue;

        if (quest->GetQuestLevel() <= 0)
            continue;

        if (quest->GetRequiredClasses())
            continue;

        for (int j = 0; j < quest->GetRewChoiceItemsCount(); j++)
            if (uint32 itemId = quest->RewardChoiceItemId[j])
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR)
                    continue;
                int requiredLevel = std::max((int)proto->RequiredLevel, quest->GetQuestLevel());
                equipCacheNew[requiredLevel][proto->InventoryType].push_back(itemId);
                questItemIds.insert(itemId);
            }

        for (int j = 0; j < quest->GetRewItemsCount(); j++)
            if (uint32 itemId = quest->RewardItemId[j])
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR)
                    continue;
                int requiredLevel = std::max((int)proto->RequiredLevel, quest->GetQuestLevel());
                equipCacheNew[requiredLevel][proto->InventoryType].push_back(itemId);
                questItemIds.insert(itemId);
            }
    }

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (auto const& itr : *itemTemplates)
    {
        ItemTemplate const* proto = &itr.second;
        if (!proto)
            continue;
        uint32 itemId = proto->ItemId;

        if (questItemIds.find(itemId) != questItemIds.end())
            continue;

        if (IsTestItem(itemId))
        {
            continue;
        }
        if (itemId == 22784)
        {  // Sunwell Orb
            continue;
        }
        equipCacheNew[proto->RequiredLevel][proto->InventoryType].push_back(itemId);
    }
}

RandomItemList RandomItemMgr::Query(uint32 level, uint8 clazz, uint8 slot, uint32 quality)
{
    // return equipCache[key];
    BotEquipKey key(level, clazz, slot, quality);
    RandomItemList items;
    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (auto const& itr : *itemTemplates)
    {
        ItemTemplate const* proto = &itr.second;
        if (!proto)
            continue;

        if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR &&
            proto->Class != ITEM_CLASS_CONTAINER && proto->Class != ITEM_CLASS_PROJECTILE)
            continue;

        if (!CanEquipItem(key, proto))
            continue;

        if (proto->Class == ITEM_CLASS_ARMOR &&
            (slot == EQUIPMENT_SLOT_HEAD || slot == EQUIPMENT_SLOT_SHOULDERS || slot == EQUIPMENT_SLOT_CHEST ||
             slot == EQUIPMENT_SLOT_WAIST || slot == EQUIPMENT_SLOT_LEGS || slot == EQUIPMENT_SLOT_FEET ||
             slot == EQUIPMENT_SLOT_WRISTS || slot == EQUIPMENT_SLOT_HANDS) &&
            !CanEquipArmor(key.clazz, key.level, proto))
            continue;

        if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(key.clazz, proto))
            continue;

        if (slot == EQUIPMENT_SLOT_OFFHAND && key.clazz == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
            continue;

        items.push_back(itr.first);
    }
    return items;
}

void RandomItemMgr::BuildAmmoCache()
{
    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("server.loading", "Building ammo cache for {} levels", maxLevel);

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel; level += 1)
    {
        for (uint32 subClass = ITEM_SUBCLASS_ARROW; subClass <= ITEM_SUBCLASS_BULLET; subClass++)
        {
            QueryResult results = WorldDatabase.Query(
                "SELECT entry, Flags FROM item_template WHERE class = {} AND subclass = {} AND RequiredLevel <= {} and duration = 0 "
                "ORDER BY stackable DESC, RequiredLevel DESC",
                ITEM_CLASS_PROJECTILE, subClass, level);
            if (!results)
                continue;
            do
            {
                Field* fields = results->Fetch();
                uint32 entry = fields[0].Get<uint32>();
                uint32 flags = fields[1].Get<uint32>();
                if (flags & ITEM_FLAG_DEPRECATED)
                {
                    continue;
                }
                ammoCache[level][subClass] = entry;
                ++counter;
                break;
            } while (results->NextRow());
        }
    }

    LOG_INFO("server.loading", "Cached {} types of ammo", counter);  // TEST
}

uint32 RandomItemMgr::GetAmmo(uint32 level, uint32 subClass) { return ammoCache[level][subClass]; }

void RandomItemMgr::BuildPotionCache()
{
    uint32 maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    // if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    //     maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("server.loading", "Building potion cache for {} levels", maxLevel);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 effects[] = {SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE};
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];

            for (auto const& itr : *itemTemplates)
            {
                ItemTemplate const* proto = &itr.second;
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                    (proto->SubClass != ITEM_SUBCLASS_POTION && proto->SubClass != ITEM_SUBCLASS_FLASK) ||
                    proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
                    if (!spellInfo)
                        continue;

                    for (uint8 i = 0; i < 3; i++)
                    {
                        if (spellInfo->Effects[i].Effect == effect)
                        {
                            potionCache[level / 10][effect].push_back(itr.first);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 effects[] = {SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE};
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];
            uint32 size = potionCache[level / 10][effect].size();
            ++counter;

            LOG_DEBUG("server.loading", "Potion cache for level={}, effect={}: {} items", level, effect, size);
        }
    }

    LOG_INFO("server.loading", "Cached {} types of potions", counter);  // TEST
}

void RandomItemMgr::BuildFoodCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("server.loading", "Building food cache for {} levels", maxLevel);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 categories[] = {11, 59};
        for (int i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];

            for (auto const& itr : *itemTemplates)
            {
                ItemTemplate const* proto = &itr.second;
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                    (proto->SubClass != ITEM_SUBCLASS_FOOD && proto->SubClass != ITEM_SUBCLASS_CONSUMABLE) ||
                    (proto->Spells[0].SpellCategory != category) || proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                foodCache[level / 10][category].push_back(itr.first);
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 categories[] = {11, 59};
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];
            uint32 size = foodCache[level / 10][category].size();
            ++counter;
            LOG_DEBUG("server.loading", "Food cache for level={}, category={}: {} items", level, category, size);
        }
    }

    LOG_INFO("server.loading", "Cached {} types of food", counter);
}

uint32 RandomItemMgr::GetRandomPotion(uint32 level, uint32 effect)
{
    std::vector<uint32> potions = potionCache[(level - 1) / 10][effect];
    if (potions.empty())
        return 0;

    return potions[urand(0, potions.size() - 1)];
}

uint32 RandomItemMgr::GetFood(uint32 level, uint32 category)
{
    std::initializer_list<uint32> items;
    std::vector<uint32> food;
    if (category == 11)
    {
        if (level < 5)
            items = {787, 117, 4540, 2680};
        else if (level < 15)
            items = {2287, 4592, 4541, 21072};
        else if (level < 25)
            items = {3770, 16170, 4542, 20074};
        else if (level < 35)
            items = {4594, 3771, 1707, 4457};
        else if (level < 45)
            items = {4599, 4601, 21552, 17222 /*21030, 16168 */};
        else if (level < 55)
            items = {8950, 8952, 8957, 21023 /*21033, 21031 */};
        else if (level < 65)
            items = {29292, 27859, 30458, 27662};
        else if (level < 75)
            items = {29450, 29451, 29452};
        else
            items = {35947};
    }

    if (category == 59)
    {
        if (level < 5)
            items = {159, 117};
        else if (level < 15)
            items = {1179, 21072};
        else if (level < 25)
            items = {1205};
        else if (level < 35)
            items = {1708};
        else if (level < 45)
            items = {1645};
        else if (level < 55)
            items = {8766};
        else if (level < 65)
            items = {28399};
        else if (level < 75)
            items = {27860};
        else
            items = {33445};
    }

    food.insert(food.end(), items);
    if (food.empty())
        return 0;
    return food[urand(0, food.size() - 1)];
}

uint32 RandomItemMgr::GetRandomFood(uint32 level, uint32 category)
{
    std::vector<uint32> food = foodCache[(level - 1) / 10][category];
    if (food.empty())
        return 0;

    return food[urand(0, food.size() - 1)];
}

void RandomItemMgr::BuildTradeCache()
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    LOG_INFO("server.loading", "Building trade cache for {} levels", maxLevel);

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();

    uint32 counter = 0;
    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_TRADE_GOODS || proto->Bonding != NO_BIND)
                continue;

            if (proto->ItemLevel < level)
                continue;

            if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                continue;

            if (proto->RequiredSkill)
                continue;

            tradeCache[level / 10].push_back(itr.first);
        }
    }

    for (uint32 level = 1; level <= maxLevel + 1; level += 10)
    {
        uint32 size = tradeCache[level / 10].size();
        LOG_DEBUG("server.loading", "Trade cache for level={}: {} items", level, size);
        ++counter;
    }

    LOG_INFO("server.loading", "Cached {} trade items", counter);  // TEST
}

uint32 RandomItemMgr::GetRandomTrade(uint32 level)
{
    std::vector<uint32> trade = tradeCache[(level - 1) / 10];
    if (trade.empty())
        return 0;

    return trade[urand(0, trade.size() - 1)];
}

void RandomItemMgr::BuildRarityCache()
{
    if (PreparedQueryResult result =
            PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RARITY_CACHE)))
    {
        LOG_INFO("playerbots", "Loading item rarity cache");

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 itemId = fields[0].Get<uint32>();
            float rarity = fields[1].Get<float>();

            rarityCache[itemId] = rarity;
            ++count;

        } while (result->NextRow());

        LOG_INFO("playerbots", "Item rarity cache loaded from {} records", count);
    }
    else
    {
        ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
        LOG_INFO("playerbots", "Building item rarity cache from {} items", itemTemplates->size());

        for (auto const& itr : *itemTemplates)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (proto->Quality == ITEM_QUALITY_POOR)
                continue;

            if (strstri(proto->Name1.c_str(), "qa") || strstri(proto->Name1.c_str(), "test") ||
                strstri(proto->Name1.c_str(), "deprecated"))
                continue;

            if (!proto->ItemLevel)
                continue;

            QueryResult results = WorldDatabase.Query(
                "SELECT MAX(q.chance) FROM ( "
                // "-- Creature "
                "SELECT  "
                "AVG ( "
                "   CASE  "
                "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                "    ELSE   "
                "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM creature_loot_template lt1 WHERE lt1.groupid = "
                "lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                "    / (SELECT COUNT(*) FROM creature_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = "
                "lt.entry AND lt1.ChanceOrQuestChance = 0) "
                "    END "
                ") chance, 'creature' type "
                "FROM creature_loot_template lt "
                "JOIN creature_template ct ON ct.LootId = lt.entry "
                "JOIN creature c ON c.id1 = ct.entry "
                "WHERE lt.item = {} "
                "union all "
                // "-- Gameobject "
                "SELECT  "
                "AVG ( "
                "   CASE  "
                "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                "    ELSE   "
                "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM gameobject_loot_template lt1 WHERE lt1.groupid "
                "= lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                "    / (SELECT COUNT(*) FROM gameobject_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry "
                "= lt.entry AND lt1.ChanceOrQuestChance = 0) "
                "    END "
                ") chance, 'gameobject' type "
                "FROM gameobject_loot_template lt "
                "JOIN gameobject_template ct ON ct.data1 = lt.entry "
                "JOIN gameobject c ON c.id1 = ct.entry "
                "WHERE lt.item = {} "
                "union all "
                // "-- Disenchant "
                "SELECT  "
                "AVG ( "
                "   CASE  "
                "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                "    ELSE   "
                "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM disenchant_loot_template lt1 WHERE lt1.groupid "
                "= lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                "    / (SELECT COUNT(*) FROM disenchant_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry "
                "= lt.entry AND lt1.ChanceOrQuestChance = 0) "
                "    END "
                ") chance, 'disenchant' type "
                "FROM disenchant_loot_template lt "
                "JOIN item_template ct ON ct.DisenchantID = lt.entry "
                "WHERE lt.item = {} "
                "union all "
                // "-- Fishing "
                "SELECT  "
                "AVG ( "
                "   CASE  "
                "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance "
                "    ELSE   "
                "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM fishing_loot_template lt1 WHERE lt1.groupid = "
                "lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                "    / (SELECT COUNT(*) FROM fishing_loot_template lt1 WHERE lt1.groupid = lt.groupid AND lt1.entry = "
                "lt.entry AND lt1.ChanceOrQuestChance = 0) "
                "    END "
                ") chance, 'fishing' type "
                "FROM fishing_loot_template lt "
                "WHERE lt.item = {} "
                "union all "
                // "-- Skinning "
                "SELECT  "
                "AVG ( "
                "   CASE  "
                "    WHEN lt.groupid = 0 THEN lt.ChanceOrQuestChance  "
                "    WHEN lt.ChanceOrQuestChance > 0 THEN lt.ChanceOrQuestChance  "
                "    ELSE   "
                "    IFNULL(100 - (SELECT SUM(ChanceOrQuestChance) FROM skinning_loot_template lt1 WHERE lt1.groupid = "
                "lt.groupid AND lt1.entry = lt.entry AND lt1.ChanceOrQuestChance > 0), 100) "
                "    * IFNULL((SELECT 1/COUNT(*) FROM skinning_loot_template lt1 WHERE lt1.groupid = lt.groupid AND "
                "lt1.entry = lt.entry AND lt1.ChanceOrQuestChance = 0), 1) "
                "    END "
                ") chance, 'skinning' type "
                "FROM skinning_loot_template lt "
                "JOIN creature_template ct ON ct.SkinningLootId = lt.entry "
                "JOIN creature c ON c.id1 = ct.entry "
                "WHERE lt.item = {}) q; ",
                itr.first, itr.first, itr.first, itr.first, itr.first);

            if (results)
            {
                Field* fields = results->Fetch();
                float rarity = fields[0].Get<float>();
                if (rarity > 0.01)
                {
                    rarityCache[itr.first] = rarity;

                    PlayerbotsDatabasePreparedStatement* stmt =
                        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_RARITY_CACHE);
                    stmt->SetData(0, itr.first);
                    stmt->SetData(1, rarity);
                    PlayerbotsDatabase.Execute(stmt);
                }
            }
        }

        LOG_INFO("playerbots", "Item rarity cache built from {} items", itemTemplates->size());
    }
}

float RandomItemMgr::GetItemRarity(uint32 itemId) { return rarityCache[itemId]; }

inline bool IsCraftedBySpellInfo(ItemTemplate const* proto, SpellInfo const* spellInfo)
{
    for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
    {
        if (spellInfo->Reagent[x] <= 0)
        {
            continue;
        }

        if (proto->ItemId == spellInfo->Reagent[x])
        {
            return true;
        }
    }

    for (uint8 i = 0; i < 3; ++i)
    {
        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM)
        {
            if (spellInfo->Effects[i].ItemType == proto->ItemId)
            {
                return true;
            }
        }
    }

    return false;
}

inline bool IsCraftedBySpell(ItemTemplate const* proto, uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    return IsCraftedBySpellInfo(proto, spellInfo);
}

inline bool IsCraftedBy(ItemTemplate const* proto, uint32 spellId)
{
    if (IsCraftedBySpell(proto, spellId))
        return true;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    for (uint32 effect = 0; effect < 3; ++effect)
    {
        uint32 craftId = spellInfo->Effects[effect].TriggerSpell;
        SpellInfo const* craftSpellInfo = sSpellMgr->GetSpellInfo(craftId);
        if (!craftSpellInfo)
            continue;

        if (IsCraftedBySpellInfo(proto, craftSpellInfo))
            return true;
    }

    return false;
}

inline bool ContainsInternal(ItemTemplate const* proto, uint32 skillId)
{
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine || skillLine->ID != skillId)
            continue;

        if (IsCraftedBy(proto, skillLine->Spell))
            return true;
    }

    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        if (itr->second.trainer_type != TRAINER_TYPE_TRADESKILLS)
            continue;

        uint32 trainerId = itr->second.Entry;
        TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);
        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator iter = trainer_spells->spellList.begin();
             iter != trainer_spells->spellList.end(); ++iter)
        {
            TrainerSpell const* tSpell = &iter->second;
            if (!tSpell || tSpell->reqSkill != skillId)
                continue;

            if (IsCraftedBy(proto, tSpell->spell))
                return true;
        }
    }

    std::vector<ItemTemplate*> const* itemTemplates = sObjectMgr->GetItemTemplateStoreFast();
    for (ItemTemplate const* recipe : *itemTemplates)
    {
        if (!recipe)
            continue;

        if (recipe->Class == ITEM_CLASS_RECIPE &&
            ((recipe->SubClass == ITEM_SUBCLASS_LEATHERWORKING_PATTERN && skillId == SKILL_LEATHERWORKING) ||
             (recipe->SubClass == ITEM_SUBCLASS_TAILORING_PATTERN && skillId == SKILL_TAILORING) ||
             (recipe->SubClass == ITEM_SUBCLASS_ENGINEERING_SCHEMATIC && skillId == SKILL_ENGINEERING) ||
             (recipe->SubClass == ITEM_SUBCLASS_BLACKSMITHING && skillId == SKILL_BLACKSMITHING) ||
             (recipe->SubClass == ITEM_SUBCLASS_COOKING_RECIPE && skillId == SKILL_COOKING) ||
             (recipe->SubClass == ITEM_SUBCLASS_ALCHEMY_RECIPE && skillId == SKILL_ALCHEMY) ||
             (recipe->SubClass == ITEM_SUBCLASS_FIRST_AID_MANUAL && skillId == SKILL_FIRST_AID) ||
             (recipe->SubClass == ITEM_SUBCLASS_ENCHANTING_FORMULA && skillId == SKILL_ENCHANTING) ||
             (recipe->SubClass == ITEM_SUBCLASS_JEWELCRAFTING_RECIPE && skillId == SKILL_JEWELCRAFTING) ||
             (recipe->SubClass == ITEM_SUBCLASS_FISHING_MANUAL && skillId == SKILL_FISHING)))
        {
            for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                if (IsCraftedBy(proto, recipe->Spells[i].SpellId))
                    return true;
            }
        }
    }

    return false;
}

bool RandomItemMgr::IsUsedBySkill(ItemTemplate const* proto, uint32 skillId)
{
    if (itemCache.find(proto->ItemId) != itemCache.end())
        return true;

    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
            break;
        default:
            return false;
    }

    bool contains = ContainsInternal(proto, skillId);
    if (contains)
        itemCache.insert(proto->ItemId);

    return contains;
}
