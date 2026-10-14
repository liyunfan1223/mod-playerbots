/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTFACTORY_H
#define PLAYERBOTS_PLAYERBOTFACTORY_H

#include "InventoryAction.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include <string>
#include <utility>

class Item;

namespace Trainer
{
    class Trainer;
    struct Spell;
}

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
    uint32 InitTalentsTree(bool incremental = false, bool use_template = true, bool reset = false);
    static void InitTalentsBySpecNo(Player* bot, int specNo, bool reset);
    static void InitTalentsByParsedSpecLink(Player* bot, std::vector<std::vector<uint32>> parsedSpecLink, bool reset);
    void InitAvailableSpells();
    static bool IsTrainerSpellAllowedForBot(Player* bot, Trainer::Trainer const* trainer,
                                             Trainer::Spell const* trainerSpell);
    void InitClassSpells();
    void InitSpecialSpells();
    void InitEquipment(bool incremental, bool second_chance = false);
    void InitPet();
    void InitAmmo();
    static uint32 CalcMixedGearScore(uint32 gs, uint32 quality);
    static void DestroyEquippedGear(Player* bot);
    static void AutoGear(Player* bot, uint32 itemQuality, uint32 ilvl, bool incremental, bool secondChance = false,
                        bool applyFinishers = true);
    void InitPetTalents();
    void CleanupConsumables();
    void InitReagents();
    void InitConsumables();
    void InitPotions();
    void InitGlyphs(bool increment = false);
    void InitFood();
    void InitMounts();
    void InitBags(bool destroyOld = true);
    void ApplyEnchantAndGemsNew(bool destroyOld = true);
    void InitInstanceQuests();
    void UnbindInstance();
    void InitKeyring();
    void InitReputation();
    void InitAttunementQuests();
    void InitGuild();

private:
    enum class ProfessionSpecializationSpell : uint32
    {
        Weapon = 9787,
        Armor = 9788,
        Hammer = 17040,
        Axe = 17041,
        Sword = 17039,

        LearnWeapon = 9789,
        LearnArmor = 9790,
        LearnHammer = 39099,
        LearnAxe = 39098,
        LearnSword = 39097,

        Dragon = 10656,
        Elemental = 10658,
        Tribal = 10660,

        LearnDragon = 10657,
        LearnElemental = 10659,
        LearnTribal = 10661,

        Spellfire = 26797,
        Mooncloth = 26798,
        Shadoweave = 26801,

        Goblin = 20222,
        Gnomish = 20219,

        LearnGoblin = 20221,
        LearnGnomish = 20220,

        LearnSpellfire = 26796,
        LearnMooncloth = 26799,
        LearnShadoweave = 26800,

        Transmute = 28672,
        Elixir = 28677,
        Potion = 28675,

        LearnTransmute = 28674,
        LearnElixir = 28678,
        LearnPotion = 28676
    };

    enum class ProfessionRollType : uint32
    {
        Random = 1,
        Class = 2
    };

    struct WeightedProfessionPair
    {
        uint16 firstSkill;
        uint16 secondSkill;
        uint32 weight;
    };

    void Prepare();
    // void InitSecondEquipmentSet();
    // void InitEquipmentNew(bool incremental);
    bool CanEquipItem(ItemTemplate const* proto);
    bool CanEquipUnseenItem(uint8 slot, uint16& dest, uint32 item);
    static bool IsPrimaryTradeSkill(uint16 skillId);
    static bool IsSecondaryTradeSkill(uint16 skillId);
    static uint16 GetTrainerSpellTradeSkill(Trainer::Spell const* trainerSpell);
    static bool IsGatheringTradeSkill(uint16 skillId);
    static bool IsCraftingTradeSkill(uint16 skillId);
    static uint32 GetProfessionStarterSpell(uint16 skillId);
    static std::vector<WeightedProfessionPair> GetClassProfessionPairs(Player* bot);
    static std::vector<WeightedProfessionPair> GetRandomProfessionPairs();
    static std::pair<uint16, uint16> ChooseProfessionPair(std::vector<WeightedProfessionPair> const& professionPairs);
    static uint16 ChooseComplementaryProfession(
        std::vector<WeightedProfessionPair> const& professionPairs, uint16 existingSkill);
    static uint16 ChooseSingleProfession(std::vector<WeightedProfessionPair> const& professionPairs);
    static uint32 GetStoredOrRandomValue(Player* bot, std::string const& key, uint32 minValue, uint32 maxValue);
    static bool HasAnySpell(Player* bot, std::vector<uint32> const& spells);
    static bool LearnProfessionSpecialization(Player* bot,
                                             ProfessionSpecializationSpell knownSpell,
                                             ProfessionSpecializationSpell learnSpell);
    void InitTradeSkills();
    void InitTradeSpecializations();
    bool InitAlchemySpecialization();
    bool InitEngineeringSpecialization();
    bool InitLeatherworkingSpecialization();
    bool InitTailoringSpecialization();
    bool InitBlacksmithingSpecialization();
    void UpdateTradeSkills();
    void SetRandomSkill(uint16 id);
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
    static void BuildCcBreakTrinketCache();
    uint8 GetPreferredArmorType(uint8 cls);
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
    static std::vector<uint32> ccBreakTrinketCache;

protected:
    EnchantContainer m_EnchantContainer;
    Player* bot;
    PlayerbotAI* botAI;
};

#endif
