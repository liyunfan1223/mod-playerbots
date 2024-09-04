/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
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

enum PriorizedConsumables
{
    CONSUM_ID_ROUGH_WEIGHTSTONE = 3239,
    CONSUM_ID_COARSE_WEIGHTSTONE = 3239,
    CONSUM_ID_HEAVY_WEIGHTSTONE = 3241,
    CONSUM_ID_SOLID_WEIGHTSTONE = 7965,
    CONSUM_ID_DENSE_WEIGHTSTONE = 12643,
    CONSUM_ID_FEL_WEIGHTSTONE = 28420,
    CONSUM_ID_ADAMANTITE_WEIGHTSTONE = 28421,
    CONSUM_ID_ROUGH_SHARPENING_STONE = 2862,
    CONSUM_ID_COARSE_SHARPENING_STONE = 2863,
    CONSUM_ID_HEAVY_SHARPENING_STONE = 2871,
    CONSUM_ID_SOL_SHARPENING_STONE = 7964,
    CONSUM_ID_DENSE_SHARPENING_STONE = 12404,
    CONSUM_ID_ELEMENTAL_SHARPENING_STONE = 18262,
    CONSUM_ID_CONSECRATED_SHARPENING_STONE = 23122,
    CONSUM_ID_FEL_SHARPENING_STONE = 23528,
    CONSUM_ID_ADAMANTITE_SHARPENING_STONE = 23529,
    CONSUM_ID_LINEN_BANDAGE = 1251,
    CONSUM_ID_HEAVY_LINEN_BANDAGE = 2581,
    CONSUM_ID_WOOL_BANDAGE = 3530,
    CONSUM_ID_HEAVY_WOOL_BANDAGE = 3531,
    CONSUM_ID_SILK_BANDAGE = 6450,
    CONSUM_ID_HEAVY_SILK_BANDAGE = 6451,
    CONSUM_ID_MAGEWEAVE_BANDAGE = 8544,
    CONSUM_ID_HEAVY_MAGEWEAVE_BANDAGE = 8545,
    CONSUM_ID_RUNECLOTH_BANDAGE = 14529,
    CONSUM_ID_HEAVY_RUNECLOTH_BANDAGE = 14530,
    CONSUM_ID_NETHERWEAVE_BANDAGE = 21990,
    CONSUM_ID_HEAVY_NETHERWEAVE_BANDAGE = 21991,
    CONSUM_ID_BRILLIANT_MANA_OIL = 20748,
    CONSUM_ID_MINOR_MANA_OIL = 20745,
    CONSUM_ID_SUPERIOR_MANA_OIL = 22521,
    CONSUM_ID_LESSER_MANA_OIL = 20747,
    CONSUM_ID_BRILLIANT_WIZARD_OIL = 20749,
    CONSUM_ID_MINOR_WIZARD_OIL = 20744,
    CONSUM_ID_SUPERIOR_WIZARD_OIL = 22522,
    CONSUM_ID_WIZARD_OIL = 20750,
    CONSUM_ID_LESSER_WIZARD_OIL = 20746,
    CONSUM_ID_INSTANT_POISON = 6947,
    CONSUM_ID_INSTANT_POISON_II = 6949,
    CONSUM_ID_INSTANT_POISON_III = 6950,
    CONSUM_ID_INSTANT_POISON_IV = 8926,
    CONSUM_ID_INSTANT_POISON_V = 8927,
    CONSUM_ID_INSTANT_POISON_VI = 8928,
    CONSUM_ID_INSTANT_POISON_VII = 21927,
    CONSUM_ID_DEADLY_POISON = 2892,
    CONSUM_ID_DEADLY_POISON_II = 2893,
    CONSUM_ID_DEADLY_POISON_III = 8984,
    CONSUM_ID_DEADLY_POISON_IV = 8985,
    CONSUM_ID_DEADLY_POISON_V = 20844,
    CONSUM_ID_DEADLY_POISON_VI = 22053,
    CONSUM_ID_DEADLY_POISON_VII = 22054
};

#define MAX_CONSUM_ID 28

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
    void InitEquipment(bool incremental, bool second_chance = false);
    void InitPet();
    void InitAmmo();
    static uint32 CalcMixedGearScore(uint32 gs, uint32 quality);
    void InitPetTalents();

    void InitReagents();
    void InitGlyphs(bool increment = false);
    void InitFood();
    void InitMounts();
    void InitBags(bool destroyOld = true);
    void ApplyEnchantAndGemsNew(bool destoryOld = true);
    void InitInstanceQuests();
    void UnbindInstance();

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
    void InitSpecialSpells();
    void InitTalents(uint32 specNo);
    void InitTalentsByTemplate(uint32 specNo);
    void InitQuests(std::list<uint32>& questMap);
    void ClearInventory();
    void ClearAllItems();
    void ResetQuests();
    void InitPotions();

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
    void AddConsumables();
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
    std::vector<uint32> trainerIdCache;
    static std::vector<uint32> enchantSpellIdCache;
    static std::vector<uint32> enchantGemIdCache;

protected:
    EnchantContainer m_EnchantContainer;
    Player* bot;
    PlayerbotAI* botAI;
};

#endif
