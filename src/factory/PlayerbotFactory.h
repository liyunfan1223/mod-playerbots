/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTFACTORY_H
#define _PLAYERBOT_PLAYERBOTFACTORY_H

#include "InventoryAction.h"
#include "Player.h"
#include "PlayerbotAI.h"

class Item;

struct ItemTemplate;

struct EnchantTemplate
{
    uint8 ClassId;
    uint8 SpecId;
    uint32 SpellId;
    uint8 SlotId;
};

typedef std::vector<EnchantTemplate> EnchantContainer;

// TODO: more spec/role
/* classid+talenttree
enum spec : uint8
{
    WARRIOR ARMS = 10,
    WARRIOR FURY = 11,
    WARRIOR PROT = 12,
    ROLE_HEALER = 1,
    ROLE_MDPS = 2,
    ROLE_CDPS = 3,
};
*/

/*enum roles : uint8
{
    ROLE_TANK   = 0,
    ROLE_HEALER = 1,
    ROLE_MDPS   = 2,
    ROLE_CDPS   = 3
};*/

class PlayerbotFactory
{
public:
    PlayerbotFactory(Player* bot, uint32 level, uint32 itemQuality = 0, uint32 gearScoreLimit = 0);

    static ObjectGuid GetRandomBot();
    static void Init();
    void Refresh();
    void Randomize(bool incremental);
    static std::list<uint32> classQuestIds;
    void ClearEverything();
    void InitSkills();

    static uint32 tradeSkills[];
    static float CalculateEnchantScore(uint32 enchant_id, Player* bot);
    void InitTalentsTree(bool incremental = false, bool use_template = true, bool reset = false);
    static void InitTalentsBySpecNo(Player* bot, int specNo, bool reset);
    static void InitTalentsByParsedSpecLink(Player* bot, std::vector<std::vector<uint32>> parsedSpecLink, bool reset);
    void InitAvailableSpells();
    void InitClassSpells();
    void InitSpecialSpells();
    void InitEquipment(bool incremental, bool second_chance = false);
    void InitPet();
    void InitAmmo();
    static uint32 CalcMixedGearScore(uint32 gs, uint32 quality);
    void InitPetTalents();
    void CleanupConsumables();
    void InitReagents();
    void InitConsumables();
    void InitPotions();
    void InitGlyphs(bool increment = false);
    void InitFood();
    void InitMounts();
    void InitBags(bool destroyOld = true);
    void ApplyEnchantAndGemsNew(bool destoryOld = true);
    void InitInstanceQuests();
    void UnbindInstance();
    void InitKeyring();
    void InitReputation();
    void InitAttunementQuests();

private:
    void Prepare();
    // void InitSecondEquipmentSet();
    // void InitEquipmentNew(bool incremental);
    bool CanEquipItem(ItemTemplate const* proto);
    bool CanEquipUnseenItem(uint8 slot, uint16& dest, uint32 item);
    void InitTradeSkills();
    void UpdateTradeSkills();
    void SetRandomSkill(uint16 id);
    void InitSpells();
    void ClearSpells();
    void ClearSkills();
    void InitTalents(uint32 specNo);
    void InitTalentsByTemplate(uint32 specNo);
    void InitQuests(std::list<uint32>& questMap, bool withRewardItem = true);
    void ClearInventory();
    void ClearAllItems();
    void ResetQuests();

    std::vector<uint32> GetCurrentGemsCount();
    bool CanEquipArmor(ItemTemplate const* proto);
    bool CanEquipWeapon(ItemTemplate const* proto);
    void EnchantItem(Item* item);
    void AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank);
    bool CheckItemStats(uint8 sp, uint8 ap, uint8 tank);
    void CancelAuras();
    bool IsDesiredReplacement(Item* item);
    void InitInventory();
    void InitInventoryTrade();
    void InitInventoryEquip();
    void InitInventorySkill();
    Item* StoreItem(uint32 itemId, uint32 count);
    void InitGuild();
    void InitArenaTeam();
    void InitImmersive();
    static void AddPrevQuests(uint32 questId, std::list<uint32>& questIds);
    void LoadEnchantContainer();
    void ApplyEnchantTemplate();
    void ApplyEnchantTemplate(uint8 spec);
    std::vector<InventoryType> GetPossibleInventoryTypeListBySlot(EquipmentSlots slot);
    void IterateItems(IterateItemsVisitor* visitor, IterateItemsMask mask = ITERATE_ITEMS_IN_BAGS);
    void IterateItemsInBags(IterateItemsVisitor* visitor);
    void IterateItemsInEquip(IterateItemsVisitor* visitor);
    void IterateItemsInBank(IterateItemsVisitor* visitor);
    EnchantContainer::const_iterator GetEnchantContainerBegin() { return m_EnchantContainer.begin(); }
    EnchantContainer::const_iterator GetEnchantContainerEnd() { return m_EnchantContainer.end(); }
    uint32 level;
    uint32 itemQuality;
    uint32 gearScoreLimit;
    static std::list<uint32> specialQuestIds;
    static std::unordered_map<uint32, std::vector<uint32>> trainerIdCache;
    static std::vector<uint32> enchantSpellIdCache;
    static std::vector<uint32> enchantGemIdCache;

protected:
    EnchantContainer m_EnchantContainer;
    Player* bot;
    PlayerbotAI* botAI;
};

#endif
