/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotFactory.h"

#include <random>
#include <utility>

#include "AccountMgr.h"
#include "AiFactory.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GuildMgr.h"
#include "InventoryAction.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ItemVisitors.h"
#include "Log.h"
#include "LogCommon.h"
#include "LootMgr.h"
#include "MapMgr.h"
#include "ObjectMgr.h"
#include "PerformanceMonitor.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotDbStore.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "ReputationMgr.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "StatsWeightCalculator.h"
#include "World.h"
#include "AiObjectContext.h"
#include "ItemPackets.h"

const uint64 diveMask = (1LL << 7) | (1LL << 44) | (1LL << 37) | (1LL << 38) | (1LL << 26) | (1LL << 30) | (1LL << 27) |
                        (1LL << 33) | (1LL << 24) | (1LL << 34);

static std::vector<uint32> initSlotsOrder = {EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2, EQUIPMENT_SLOT_MAINHAND,
    EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED, EQUIPMENT_SLOT_HEAD, EQUIPMENT_SLOT_SHOULDERS, EQUIPMENT_SLOT_CHEST,
    EQUIPMENT_SLOT_LEGS, EQUIPMENT_SLOT_HANDS, EQUIPMENT_SLOT_NECK, EQUIPMENT_SLOT_BODY, EQUIPMENT_SLOT_WAIST,
    EQUIPMENT_SLOT_FEET, EQUIPMENT_SLOT_WRISTS, EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2, EQUIPMENT_SLOT_BACK};

uint32 PlayerbotFactory::tradeSkills[] = {SKILL_ALCHEMY,        SKILL_ENCHANTING,  SKILL_SKINNING,  SKILL_TAILORING,
                                          SKILL_LEATHERWORKING, SKILL_ENGINEERING, SKILL_HERBALISM, SKILL_MINING,
                                          SKILL_BLACKSMITHING,  SKILL_COOKING,     SKILL_FIRST_AID, SKILL_FISHING,
                                          SKILL_JEWELCRAFTING};

std::list<uint32> PlayerbotFactory::classQuestIds;
std::list<uint32> PlayerbotFactory::specialQuestIds;
std::vector<uint32> PlayerbotFactory::enchantSpellIdCache;
std::vector<uint32> PlayerbotFactory::enchantGemIdCache;
std::unordered_map<uint32, std::vector<uint32>> PlayerbotFactory::trainerIdCache;

PlayerbotFactory::PlayerbotFactory(Player* bot, uint32 level, uint32 itemQuality, uint32 gearScoreLimit)
    : level(level), itemQuality(itemQuality), gearScoreLimit(gearScoreLimit), bot(bot)
{
    botAI = GET_PLAYERBOT_AI(bot);
    if (!this->itemQuality)
    {
        uint32 gs = sPlayerbotAIConfig->randomGearScoreLimit == 0
                        ? 0
                        : PlayerbotFactory::CalcMixedGearScore(sPlayerbotAIConfig->randomGearScoreLimit,
                                                               sPlayerbotAIConfig->randomGearQualityLimit);
        this->itemQuality = sPlayerbotAIConfig->randomGearQualityLimit;
        this->gearScoreLimit = gs;
    }
}

void PlayerbotFactory::Init()
{
    if (sPlayerbotAIConfig->randomBotPreQuests)
    {
        ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
        for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
        {
            uint32 questId = i->first;
            Quest const* quest = i->second;

            if (!quest->GetRequiredClasses() || quest->IsRepeatable() || quest->GetMinLevel() < 10)
                continue;

            if (quest->GetRewSpellCast() > 0)
            {
                int32 spellId = quest->GetRewSpellCast();
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo)
                    continue;
            }
            else if (quest->GetRewSpell() > 0)
            {
                int32 spellId = quest->GetRewSpell();
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo)
                    continue;
            }

            AddPrevQuests(questId, classQuestIds);
            classQuestIds.remove(questId);
            classQuestIds.push_back(questId);
        }
    }

    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotQuestIds.begin();
         i != sPlayerbotAIConfig->randomBotQuestIds.end(); ++i)
    {
        uint32 questId = *i;
        AddPrevQuests(questId, specialQuestIds);
        specialQuestIds.remove(questId);
        specialQuestIds.push_back(questId);
    }
    uint32 maxStoreSize = sSpellMgr->GetSpellInfoStoreSize();
    for (uint32 id = 1; id < maxStoreSize; ++id)
    {
        if (id == 47181 || id == 50358 || id == 47242 || id == 52639 || id == 47147 || id == 7218)  // Test Enchant
            continue;

        if (id == 15463 || id == 15490) // Legendary Arcane Amalgamation
            continue;

        if (id == 29467 || id == 29475 || id == 29480 || id == 29483) // Naxx40 Sapphiron Shoulder Enchants
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if (!spellInfo)
            continue;

        //uint32 requiredLevel = spellInfo->BaseLevel; //not used, line marked for removal.

        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->Effects[j].Effect != SPELL_EFFECT_ENCHANT_ITEM)
                continue;

            uint32 enchant_id = spellInfo->Effects[j].MiscValue;
            if (!enchant_id)
                continue;

            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchant || (enchant->slot != PERM_ENCHANTMENT_SLOT && enchant->slot != TEMP_ENCHANTMENT_SLOT))
                continue;

            // SpellInfo const* enchantSpell = sSpellMgr->GetSpellInfo(enchant->spellid[0]);
            // if (!enchantSpell)
            //     continue;
            if (strstr(spellInfo->SpellName[0], "Test"))
                break;

            enchantSpellIdCache.push_back(id);
            break;
            // LOG_INFO("playerbots", "Add {} to enchantment spells", id);
        }
    }
    LOG_INFO("playerbots", "Loading {} enchantment spells", enchantSpellIdCache.size());
    for (auto iter = sSpellItemEnchantmentStore.begin(); iter != sSpellItemEnchantmentStore.end(); iter++)
    {
        uint32 gemId = iter->GemID;
        if (gemId == 0)
        {
            continue;
        }

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(gemId);
        if (!proto)
        {
            continue;
        }

        if (proto->ItemLevel < 60)
        {
            continue;
        }

        if (proto->Flags & ITEM_FLAG_UNIQUE_EQUIPPABLE)
        {
            continue;
        }

        if (sRandomItemMgr->IsTestItem(gemId))
        {
           continue;
        }

        if (!sGemPropertiesStore.LookupEntry(proto->GemProperties))
        {
            continue;
        }

        // LOG_INFO("playerbots", "Add {} to enchantment gems", gemId);
        enchantGemIdCache.push_back(gemId);
    }
    LOG_INFO("playerbots", "Loading {} enchantment gems", enchantGemIdCache.size());
}

void PlayerbotFactory::Prepare()
{
    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);

    bot->CombatStop(true);
    uint32 currentLevel = bot->GetLevel();
    bot->GiveLevel(level);
    if (level != currentLevel)
    {
        bot->SetUInt32Value(PLAYER_XP, 0);
    }
    if (!sPlayerbotAIConfig->randomBotShowHelmet || !urand(0, 4))
    {
        bot->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
    }

    if (!sPlayerbotAIConfig->randomBotShowCloak || !urand(0, 4))
    {
        bot->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
    }
}

void PlayerbotFactory::Randomize(bool incremental)
{
    // if (sPlayerbotAIConfig->disableRandomLevels)
    // {
    //     return;
    // }
    LOG_DEBUG("playerbots", "{} randomizing {} (level {} class = {})...", (incremental ? "Incremental" : "Full"),
             bot->GetName().c_str(), level, bot->getClass());
    // LOG_DEBUG("playerbots", "Preparing to {} randomize...", (incremental ? "incremental" : "full"));
    Prepare();
    LOG_DEBUG("playerbots", "Resetting player...");
    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Reset");
    if (!sPlayerbotAIConfig->equipmentPersistence || level < sPlayerbotAIConfig->equipmentPersistenceLevel)
    {
        bot->resetTalents(true);
    }
    if (!incremental)
    {
        ClearSkills();
        ClearSpells();
        ResetQuests();
        if (!sPlayerbotAIConfig->equipmentPersistence || level < sPlayerbotAIConfig->equipmentPersistenceLevel)
        {
            ClearAllItems();
        }
    }
    ClearInventory();
    bot->RemoveAllSpellCooldown();
    UnbindInstance();

    bot->GiveLevel(level);
    bot->InitStatsForLevel(true);
    CancelAuras();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    // pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Immersive");
    // LOG_INFO("playerbots", "Initializing immersive...");
    // InitImmersive();
    // if (pmo)
    //     pmo->finish();

    if (sPlayerbotAIConfig->randomBotPreQuests)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Quests");
        InitInstanceQuests();
        InitAttunementQuests();
        if (pmo)
            pmo->finish();
    }
    else
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Quests");
        InitAttunementQuests();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells1");
    LOG_DEBUG("playerbots", "Initializing spells (step 1)...");
    bot->LearnDefaultSkills();
    InitClassSpells();
    InitAvailableSpells();
    if (pmo)
        pmo->finish();

    LOG_DEBUG("playerbots", "Initializing skills (step 1)...");
    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills1");
    InitSkills();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Talents");
    LOG_DEBUG("playerbots", "Initializing talents...");
    if (!incremental || !sPlayerbotAIConfig->equipmentPersistence ||
        bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel)
    {
        InitTalentsTree();
    }
    sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", 0);
    if (botAI)
    {
        sPlayerbotDbStore->Reset(botAI);
        // botAI->DoSpecificAction("auto talents");
        botAI->ResetStrategies(false);  // fix wrong stored strategy
    }
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells2");
    LOG_DEBUG("playerbots", "Initializing spells (step 2)...");
    InitAvailableSpells();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Reputation");
    LOG_DEBUG("playerbots", "Initializing reputation...");
    InitReputation();
    if (pmo)
        pmo->finish();

    LOG_DEBUG("playerbots", "Initializing special spells...");
    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells3");
    InitSpecialSpells();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Mounts");
    LOG_DEBUG("playerbots", "Initializing mounts...");
    InitMounts();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    // pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills2");
    // LOG_INFO("playerbots", "Initializing skills (step 2)...");
    // UpdateTradeSkills();
    // if (pmo)
    //     pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Equip");
    LOG_DEBUG("playerbots", "Initializing equipmemt...");
    if (!incremental || !sPlayerbotAIConfig->equipmentPersistence ||
        bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel)
    {
        if (sPlayerbotAIConfig->incrementalGearInit || !incremental)
            InitEquipment(incremental, incremental ? false : sPlayerbotAIConfig->twoRoundsGearInit);
    }
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    // if (bot->GetLevel() >= sPlayerbotAIConfig->minEnchantingBotLevel)
    // {
    //     pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Enchant");
    //     LOG_INFO("playerbots", "Initializing enchant templates...");
    //     LoadEnchantContainer();
    //     if (pmo)
    //         pmo->finish();
    // }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Bags");
    LOG_DEBUG("playerbots", "Initializing bags...");
    InitBags();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Ammo");
    LOG_DEBUG("playerbots", "Initializing ammo...");
    InitAmmo();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Food");
    LOG_DEBUG("playerbots", "Initializing food...");
    InitFood();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Potions");
    LOG_DEBUG("playerbots", "Initializing potions...");
    InitPotions();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Reagents");
    LOG_DEBUG("playerbots", "Initializing reagents...");
    InitReagents();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Keys");
    LOG_DEBUG("playerbots", "Initializing keys...");
    InitKeyring();
    if (pmo)
        pmo->finish();

    // pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_EqSets");
    // LOG_DEBUG("playerbots", "Initializing second equipment set...");
    //    InitSecondEquipmentSet();
    // if (pmo)
    //     pmo->finish();

    if (bot->GetLevel() >= sPlayerbotAIConfig->minEnchantingBotLevel)
    {
        ApplyEnchantAndGemsNew();
    }
    // {
    // pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_EnchantTemplate");
    // LOG_INFO("playerbots", "Initializing enchant templates...");
    // ApplyEnchantTemplate();
    // if (pmo)
    //     pmo->finish();
    // }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Inventory");
    LOG_DEBUG("playerbots", "Initializing inventory...");
    // InitInventory();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Consumable");
    LOG_DEBUG("playerbots", "Initializing consumables...");
    InitConsumables();
    if (pmo)
        pmo->finish();

    LOG_DEBUG("playerbots", "Initializing glyphs...");
    InitGlyphs();
    // bot->SaveToDB(false, false);

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Guilds");
    // bot->SaveToDB(false, false);
    if (sPlayerbotAIConfig->randomBotGuildCount > 0)
    {
        LOG_DEBUG("playerbots", "Initializing guilds...");
        InitGuild();
    }
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    if (bot->GetLevel() >= 70)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Arenas");
        // LOG_INFO("playerbots", "Initializing arena teams...");
        InitArenaTeam();
        if (pmo)
            pmo->finish();
    }

    if (!incremental)
    {
        bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT, true);
        bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true);
        // bot->SaveToDB(false, false);
    }
    if (bot->GetLevel() >= 10)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Pet");
        LOG_DEBUG("playerbots", "Initializing pet...");
        InitPet();
        // bot->SaveToDB(false, false);
        InitPetTalents();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Save");
    LOG_DEBUG("playerbots", "Saving to DB...");
    bot->SetMoney(urand(level * 100000, level * 5 * 100000));
    bot->SetHealth(bot->GetMaxHealth());
    bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
    bot->SaveToDB(false, false);
    LOG_DEBUG("playerbots", "Initialization Done.");
    if (pmo)
        pmo->finish();
}

void PlayerbotFactory::Refresh()
{
    // Prepare();
    // if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel)
    // {
    //     InitEquipment(true);
    // }
    InitAttunementQuests();
    ClearInventory();
    InitAmmo();
    InitFood();
    InitReagents();
    InitConsumables();
    InitPotions();
    InitPet();
    InitPetTalents();
    InitClassSpells();
    InitAvailableSpells();
    InitSkills();
    InitReputation();
    InitSpecialSpells();
    InitMounts();
    InitKeyring();
    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel)
    {
        InitTalentsTree(true, true, true);
    }
    if (bot->GetLevel() >= sPlayerbotAIConfig->minEnchantingBotLevel)
    {
        ApplyEnchantAndGemsNew();
    }
    bot->DurabilityRepairAll(false, 1.0f, false);
    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);
    uint32 money = urand(level * 1000, level * 5 * 1000);
    if (bot->GetMoney() < money)
        bot->SetMoney(money);
    // bot->SaveToDB(false, false);
}

void PlayerbotFactory::InitConsumables()
{
    int specTab = AiFactory::GetPlayerSpecTab(bot);
    std::vector<std::pair<uint32, uint32>> items;

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
        {
            // Discipline or Holy: Mana Oil
            if (specTab == 0 || specTab == 1)
            {
                std::vector<uint32> mana_oils = {BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL};
                for (uint32 itemId : mana_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 4});
                    break;
                }
            }
            // Shadow: Wizard Oil
            if (specTab == 2)
            {
                std::vector<uint32> wizard_oils = {BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL};
                for (uint32 itemId : wizard_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 4});
                    break;
                }
            }
            break;
        }
        case CLASS_MAGE:
        {
            // Always Wizard Oil
            std::vector<uint32> wizard_oils = {BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL};
            for (uint32 itemId : wizard_oils)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > level || level > 75)
                    continue;
                items.push_back({itemId, 4});
                break;
            }
            break;
        }
        case CLASS_DRUID:
        {
            // Balance: Wizard Oil
            if (specTab == 0)
            {
                std::vector<uint32> wizard_oils = {BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL};
                for (uint32 itemId : wizard_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 4});
                    break;
                }
            }
            // Feral: Sharpening Stones & Weightstones
            else if (specTab == 1)
            {
                std::vector<uint32> sharpening_stones = {ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE, HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE};
                std::vector<uint32> weightstones = {ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE, HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE};
                for (uint32 itemId : sharpening_stones)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 20});
                    break;
                }
                for (uint32 itemId : weightstones)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 20});
                    break;
                }
            }
            // Restoration: Mana Oil
            else if (specTab == 2)
            {
                std::vector<uint32> mana_oils = {BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL};
                for (uint32 itemId : mana_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 4});
                    break;
                }
            }
            break;
        }
        case CLASS_PALADIN:
        {
            // Holy: Mana Oil
            if (specTab == 0)
            {
                std::vector<uint32> mana_oils = {BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL};
                for (uint32 itemId : mana_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 4});
                    break;
                }
            }
            // Protection: Wizard Oil (Protection prioritizes Superior over Brilliant)
            else if (specTab == 1)
            {
                std::vector<uint32> wizard_oils = {BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL};
                for (uint32 itemId : wizard_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 4});
                    break;
                }
            }
            // Retribution: Sharpening Stones & Weightstones
            else if (specTab == 2)
            {
                std::vector<uint32> sharpening_stones = {ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE, HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE};
                std::vector<uint32> weightstones = {ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE, HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE};
                for (uint32 itemId : sharpening_stones)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 20});
                    break;
                }
                for (uint32 itemId : weightstones)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 20});
                    break;
                }
            }
            break;
        }
        case CLASS_WARRIOR:
        case CLASS_HUNTER:
        {
            // Sharpening Stones & Weightstones
            std::vector<uint32> sharpening_stones = {ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE, HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE};
            std::vector<uint32> weightstones = {ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE, HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE};
            for (uint32 itemId : sharpening_stones)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > level || level > 75)
                    continue;
                items.push_back({itemId, 20});
                break;
            }
            for (uint32 itemId : weightstones)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > level || level > 75)
                    continue;
                items.push_back({itemId, 20});
                break;
            }
            break;
        }
        case CLASS_ROGUE:
        {
            // Poisons
            std::vector<uint32> instant_poisons = {INSTANT_POISON_IX, INSTANT_POISON_VIII, INSTANT_POISON_VII, INSTANT_POISON_VI, INSTANT_POISON_V, INSTANT_POISON_IV, INSTANT_POISON_III, INSTANT_POISON_II, INSTANT_POISON};
            std::vector<uint32> deadly_poisons = {DEADLY_POISON_IX, DEADLY_POISON_VIII, DEADLY_POISON_VII, DEADLY_POISON_VI, DEADLY_POISON_V, DEADLY_POISON_IV, DEADLY_POISON_III, DEADLY_POISON_II, DEADLY_POISON};
            for (uint32 itemId : deadly_poisons)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > level)
                    continue;
                items.push_back({itemId, 20});
                break;
            }
            for (uint32 itemId : instant_poisons)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > level)
                    continue;
                items.push_back({itemId, 20});
                break;
            }
            break;
        }
        default:
            break;
    }

    for (std::pair<uint32, uint32> item : items)
    {
        int count = (int)item.second - (int)bot->GetItemCount(item.first);
        if (count > 0)
            StoreItem(item.first, count);
    }
}

void PlayerbotFactory::InitPetTalents()
{
    if (bot->GetLevel() <= 70 && sPlayerbotAIConfig->limitTalentsExpansion)
        return;

    Pet* pet = bot->GetPet();
    if (!pet)
    {
        // LOG_INFO("playerbots", "{} init pet talents failed with no pet", bot->GetName().c_str());
        return;
    }
    CreatureTemplate const* ci = pet->GetCreatureTemplate();
    if (!ci)
    {
        // LOG_INFO("playerbots", "{} init pet talents failed with no creature template", bot->GetName().c_str());
        return;
    }
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(ci->family);
    if (!pet_family || pet_family->petTalentType < 0)
    {
        // LOG_INFO("playerbots", "{} init pet talents failed with petTalentType < 0({})", bot->GetName().c_str(),
        // pet_family->petTalentType);
        return;
    }
    std::unordered_map<uint32, std::vector<TalentEntry const*>> spells;
    bool diveTypePet = (1LL << ci->family) & diveMask;

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        // prevent learn talent for different family (cheating)
        if (!((1 << pet_family->petTalentType) & talentTabInfo->petTalentMask))
            continue;
        bool diveClass = talentInfo->TalentID == 2201 || talentInfo->TalentID == 2208 || talentInfo->TalentID == 2219 ||
                         talentInfo->TalentID == 2203;
        if (diveClass && !diveTypePet)
            continue;
        bool dashClass = talentInfo->TalentID == 2119 || talentInfo->TalentID == 2207 || talentInfo->TalentID == 2111 ||
                         talentInfo->TalentID == 2109;
        if (dashClass && diveTypePet)
            continue;
        spells[talentInfo->Row].push_back(talentInfo);
    }

    std::vector<std::vector<uint32>> order =
        sPlayerbotAIConfig->parsedHunterPetLinkOrder[pet_family->petTalentType][20];
    uint32 maxTalentPoints = pet->GetMaxTalentPointsForLevel(pet->GetLevel());

    if (order.empty())
    {
        int row = 0;
        for (auto i = spells.begin(); i != spells.end(); ++i, ++row)
        {
            std::vector<TalentEntry const*>& spells_row = i->second;
            if (spells_row.empty())
            {
                LOG_INFO("playerbots", "{}: No spells for talent row {}", bot->GetName().c_str(), i->first);
                continue;
            }
            int attemptCount = 0;
            // keep learning for the last row
            while (!spells_row.empty() &&
                   ((((int)maxTalentPoints - (int)pet->GetFreeTalentPoints()) < 3 * (row + 1)) || (row == 5)) &&
                   attemptCount++ < 10 && pet->GetFreeTalentPoints())
            {
                int index = urand(0, spells_row.size() - 1);
                TalentEntry const* talentInfo = spells_row[index];
                int maxRank = 0;
                for (int rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, (uint32)pet->GetFreeTalentPoints()); ++rank)
                {
                    uint32 spellId = talentInfo->RankID[rank];
                    if (!spellId)
                        continue;

                    maxRank = rank;
                }
                // LOG_INFO("playerbots", "{} learn pet talent {}({})", bot->GetName().c_str(), talentInfo->TalentID,
                // maxRank);
                if (talentInfo->DependsOn)
                {
                    bot->LearnPetTalent(pet->GetGUID(), talentInfo->DependsOn,
                                        std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));
                }
                bot->LearnPetTalent(pet->GetGUID(), talentInfo->TalentID, maxRank);
                spells_row.erase(spells_row.begin() + index);
            }
        }
    }
    else
    {
        uint32 spec = pet_family->petTalentType;
        uint32 startPoints = pet->GetMaxTalentPointsForLevel(pet->GetLevel());
        while (startPoints > 1 && startPoints < 20 &&
               sPlayerbotAIConfig->parsedHunterPetLinkOrder[spec][startPoints].size() == 0)
        {
            startPoints--;
        }

        for (uint32 points = startPoints; points <= 20; points++)
        {
            if (sPlayerbotAIConfig->parsedHunterPetLinkOrder[spec][points].size() == 0)
                continue;
            for (std::vector<uint32>& p : sPlayerbotAIConfig->parsedHunterPetLinkOrder[spec][points])
            {
                uint32 row = p[0], col = p[1], lvl = p[2];
                uint32 talentID = 0;
                uint32 learnLevel = 0;
                std::vector<TalentEntry const*>& spell = spells[row];
                for (TalentEntry const* talentInfo : spell)
                {
                    if (talentInfo->Col != col)
                    {
                        continue;
                    }
                    if (talentInfo->DependsOn)
                    {
                        bot->LearnPetTalent(pet->GetGUID(), talentInfo->DependsOn,
                                            std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));
                    }
                    talentID = talentInfo->TalentID;

                    uint32 currentTalentRank = 0;
                    for (uint8 rank = 0; rank < MAX_TALENT_RANK; ++rank)
                    {
                        if (talentInfo->RankID[rank] && pet->HasSpell(talentInfo->RankID[rank]))
                        {
                            currentTalentRank = rank + 1;
                            break;
                        }
                    }
                    learnLevel = std::min(lvl, pet->GetFreeTalentPoints() + currentTalentRank) - 1;
                }
                bot->LearnPetTalent(pet->GetGUID(), talentID, learnLevel);
                if (pet->GetFreeTalentPoints() == 0)
                {
                    break;
                }
            }
            if (pet->GetFreeTalentPoints() == 0)
            {
                break;
            }
        }
    }
    bot->SendTalentsInfoData(true);
}

void PlayerbotFactory::InitPet()
{
    Pet* pet = bot->GetPet();

    if (!pet && bot->GetPetStable() && bot->GetPetStable()->CurrentPet)
        return;

    if (!pet)
    {
        if (bot->getClass() != CLASS_HUNTER || bot->GetLevel() < 10)
            return;

        Map* map = bot->GetMap();
        if (!map)
            return;

        std::vector<uint32> ids;

        CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();

        for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
        {
            if (!itr->second.IsTameable(bot->CanTameExoticPets()))
                continue;

            if (itr->second.minlevel > bot->GetLevel())
                continue;

            bool onlyWolf = sPlayerbotAIConfig->hunterWolfPet == 2 ||
                            (sPlayerbotAIConfig->hunterWolfPet == 1 &&
                             bot->GetLevel() >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
            // Wolf only (for higher dps)
            if (onlyWolf && itr->second.family != CREATURE_FAMILY_WOLF)
                continue;

            // Exclude configured pet families
            if (std::find(sPlayerbotAIConfig->excludedHunterPetFamilies.begin(),
                          sPlayerbotAIConfig->excludedHunterPetFamilies.end(),
                          itr->second.family) != sPlayerbotAIConfig->excludedHunterPetFamilies.end())
                continue;

            ids.push_back(itr->first);
        }

        if (ids.empty())
        {
            LOG_ERROR("playerbots", "No pets available for bot {} ({} level)", bot->GetName().c_str(), bot->GetLevel());
            return;
        }

        for (uint32 i = 0; i < 10; i++)
        {
            uint32 index = urand(0, ids.size() - 1);
            CreatureTemplate const* co = sObjectMgr->GetCreatureTemplate(ids[index]);
            if (!co)
                continue;
            if (co->Name.size() > 21)
                continue;
            uint32 guid = map->GenerateLowGuid<HighGuid::Pet>();
            uint32 pet_number = sObjectMgr->GeneratePetNumber();
            if (bot->GetPetStable() && bot->GetPetStable()->CurrentPet)
            {
                auto petGuid = bot->GetPetStable()->CurrentPet.value(); // To correct the build warnin in VS
                // bot->GetPetStable()->CurrentPet.value();
                // bot->GetPetStable()->CurrentPet.reset();
                bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
                bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
            }
            if (bot->GetPetStable() && bot->GetPetStable()->GetUnslottedHunterPet())
            {
                bot->GetPetStable()->UnslottedPets.clear();
                bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
                bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
            }
            // }
            pet = bot->CreateTamedPetFrom(co->Entry, 0);
            if (!pet)
            {
                continue;
            }

            // prepare visual effect for levelup
            pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel() - 1);

            // add to world
            pet->GetMap()->AddToMap(pet->ToCreature());

            // visual effect for levelup
            pet->SetUInt32Value(UNIT_FIELD_LEVEL, bot->GetLevel());

            // caster have pet now
            bot->SetMinion(pet, true);

            pet->InitTalentForLevel();

            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            bot->PetSpellInitialize();
            break;
        }
    }

    if (pet)
    {
        pet->InitStatsForLevel(bot->GetLevel());
        pet->SetLevel(bot->GetLevel());
        pet->SetPower(POWER_HAPPINESS, pet->GetMaxPower(Powers(POWER_HAPPINESS)));
        pet->SetHealth(pet->GetMaxHealth());
    }
    else
    {
        LOG_ERROR("playerbots", "Cannot create pet for bot {}", bot->GetName().c_str());
        return;
    }

    // LOG_INFO("playerbots", "Start make spell auto cast for {} spells. {} already auto casted.", pet->m_spells.size(),
    // pet->GetPetAutoSpellSize());
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
        {
            continue;
        }
        pet->ToggleAutocast(spellInfo, true);
    }
}

void PlayerbotFactory::ClearSkills()
{
    for (uint32 i = 0; i < sizeof(tradeSkills) / sizeof(uint32); ++i)
    {
        bot->SetSkill(tradeSkills[i], 0, 0, 0);
    }
    bot->SetUInt32Value(PLAYER_SKILL_INDEX(0), 0);
    bot->SetUInt32Value(PLAYER_SKILL_INDEX(1), 0);

    // unlearn default race/class skills
    if (PlayerInfo const* info = sObjectMgr->GetPlayerInfo(bot->getRace(), bot->getClass()))
    {
        for (PlayerCreateInfoSkills::const_iterator itr = info->skills.begin(); itr != info->skills.end(); ++itr)
        {
            uint32 skillId = itr->SkillId;
            if (!bot->HasSkill(skillId))
                continue;
            bot->SetSkill(skillId, 0, 0, 0);
        }
    }
}

void PlayerbotFactory::ClearEverything()
{
    bot->GiveLevel(bot->getClass() == CLASS_DEATH_KNIGHT ? sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)
                                                         : sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL));
    bot->SetUInt32Value(PLAYER_XP, 0);
    LOG_INFO("playerbots", "Resetting player...");
    bot->resetTalents(true);
    ClearSkills();
    ClearSpells();
    ClearInventory();
    ResetQuests();
    // bot->SaveToDB(false, false);
}

void PlayerbotFactory::ClearSpells()
{
    std::list<uint32> spells;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
        //const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId); //not used, line marked for removal.
        if (itr->second->State == PLAYERSPELL_REMOVED)
        {
            continue;
        }

        spells.push_back(spellId);
    }

    for (std::list<uint32>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        bot->removeSpell(*i, SPEC_MASK_ALL, false);
    }
}

void PlayerbotFactory::ResetQuests()
{
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        bot->SetQuestSlot(slot, 0);
    }
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 entry = quest->GetQuestId();
        if (bot->GetQuestStatus(entry) == QUEST_STATUS_NONE)
            continue;

        bot->RemoveRewardedQuest(entry);
        bot->RemoveActiveQuest(entry, false);

    }
}

void PlayerbotFactory::InitSpells() { InitAvailableSpells(); }

void PlayerbotFactory::InitTalentsTree(bool increment /*false*/, bool use_template /*true*/, bool reset /*false*/)
{
    uint32 specTab;
    uint8 cls = bot->getClass();
    std::map<uint8, uint32> tabs = AiFactory::GetPlayerSpecTabs(bot);
    uint32 total_tabs = tabs[0] + tabs[1] + tabs[2];
    if (increment && total_tabs != 0)
    {
        /// @todo: match current talent with template
        specTab = AiFactory::GetPlayerSpecTab(bot);
        /// @todo: fix cat druid hardcode
        if (bot->getClass() == CLASS_DRUID && specTab == DRUID_TAB_FERAL && bot->GetLevel() >= 20)
        {
            bool isCat = !bot->HasAura(16931);
            if (!isCat && bot->GetLevel() == 20)
            {
                uint32 bearP = sPlayerbotAIConfig->randomClassSpecProb[cls][1];
                uint32 catP = sPlayerbotAIConfig->randomClassSpecProb[cls][3];
                if (urand(1, bearP + catP) <= catP)
                    isCat = true;
            }
            if (isCat)
            {
                specTab = 3;
            }
        }
    }
    else
    {
        uint32 pointSum = 0;
        for (int i = 0; i < MAX_SPECNO; i++)
        {
            pointSum += sPlayerbotAIConfig->randomClassSpecProb[cls][i];
        }
        uint32 point = urand(1, pointSum);
        uint32 currentP = 0;
        int i;
        for (i = 0; i < MAX_SPECNO; i++)
        {
            currentP += sPlayerbotAIConfig->randomClassSpecProb[cls][i];
            if (point <= currentP)
            {
                specTab = i;
                break;
            }
        }
        if (i == MAX_SPECNO)
        {
            specTab = 0;
            LOG_ERROR("playerbots", "Fail to select spec num for bot {}! Set to 0.", bot->GetName());
        }
    }
    if (reset)
    {
        bot->resetTalents(true);
    }
    // use template if can
    if (use_template)
    {
        InitTalentsByTemplate(specTab);
    }
    // if LimitTalentsExpansion = 1 there may be unused talent points
    if (bot->GetFreeTalentPoints())
        InitTalents((specTab + 1) % 3);

    if (bot->GetFreeTalentPoints())
        InitTalents((specTab + 2) % 3);

    bot->SendTalentsInfoData(false);
}

void PlayerbotFactory::InitTalentsBySpecNo(Player* bot, int specNo, bool reset)
{
    if (reset)
    {
        bot->resetTalents(true);
    }
    uint32 cls = bot->getClass();
    int startLevel = bot->GetLevel();
    uint32 classMask = bot->getClassMask();
    std::unordered_map<uint32, std::vector<TalentEntry const*>> spells_row;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells_row[talentInfo->Row].push_back(talentInfo);
    }
    while (startLevel > 1 && startLevel < 80 &&
           sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specNo][startLevel].size() == 0)
    {
        startLevel--;
    }
    for (int level = startLevel; level <= 80; level++)
    {
        if (sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specNo][level].size() == 0)
        {
            continue;
        }
        for (std::vector<uint32>& p : sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specNo][level])
        {
            uint32 tab = p[0], row = p[1], col = p[2], lvl = p[3];
            uint32 talentID = -1;

            std::vector<TalentEntry const*>& spells = spells_row[row];
            if (spells.size() <= 0)
            {
                return;
            }
            for (TalentEntry const* talentInfo : spells)
            {
                if (talentInfo->Col != col)
                {
                    continue;
                }
                TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
                if (talentTabInfo->tabpage != tab)
                {
                    continue;
                }
                if (talentInfo->DependsOn)
                {
                    bot->LearnTalent(talentInfo->DependsOn,
                                     std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));
                }
                talentID = talentInfo->TalentID;
            }
            bot->LearnTalent(talentID, std::min(lvl, bot->GetFreeTalentPoints()) - 1);
            if (bot->GetFreeTalentPoints() == 0)
            {
                break;
            }
        }
        if (bot->GetFreeTalentPoints() == 0)
        {
            break;
        }
    }
    bot->SendTalentsInfoData(false);
}

void PlayerbotFactory::InitTalentsByParsedSpecLink(Player* bot, std::vector<std::vector<uint32>> parsedSpecLink,
                                                   bool reset)
{
    if (reset)
    {
        bot->resetTalents(true);
    }
    uint32 classMask = bot->getClassMask();
    std::unordered_map<uint32, std::vector<TalentEntry const*>> spells_row;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells_row[talentInfo->Row].push_back(talentInfo);
    }
    for (std::vector<uint32>& p : parsedSpecLink)
    {
        uint32 tab = p[0], row = p[1], col = p[2], lvl = p[3];
        uint32 talentID = -1;

        std::vector<TalentEntry const*>& spells = spells_row[row];
        if (spells.size() <= 0)
        {
            return;
        }
        for (TalentEntry const* talentInfo : spells)
        {
            if (talentInfo->Col != col)
            {
                continue;
            }
            TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
            if (talentTabInfo->tabpage != tab)
            {
                continue;
            }
            if (talentInfo->DependsOn)
            {
                bot->LearnTalent(talentInfo->DependsOn,
                                 std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));
            }
            talentID = talentInfo->TalentID;
        }
        bot->LearnTalent(talentID, std::min(lvl, bot->GetFreeTalentPoints()) - 1);
        if (bot->GetFreeTalentPoints() == 0)
        {
            break;
        }
    }
    bot->SendTalentsInfoData(false);
}

class DestroyItemsVisitor : public IterateItemsVisitor
{
public:
    DestroyItemsVisitor(Player* bot) : IterateItemsVisitor(), bot(bot) {}

    bool Visit(Item* item) override
    {
        uint32 id = item->GetTemplate()->ItemId;
        if (CanKeep(id))
        {
            keep.insert(id);
            return true;
        }

        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        return true;
    }

private:
    bool CanKeep(uint32 id)
    {
        if (keep.find(id) != keep.end())
            return false;

        if (sPlayerbotAIConfig->IsInRandomQuestItemList(id))
            return true;

        return false;
    }

    Player* bot;
    std::set<uint32> keep;
};

bool PlayerbotFactory::CanEquipArmor(ItemTemplate const* proto)
{
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE && !bot->HasSkill(SKILL_PLATE_MAIL))
    {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && !bot->HasSkill(SKILL_MAIL))
    {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_LEATHER && !bot->HasSkill(SKILL_LEATHER))
    {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_CLOTH && !bot->HasSkill(SKILL_CLOTH))
    {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD && !bot->HasSkill(SKILL_SHIELD))
    {
        return false;
    }

    return true;
    // for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    // {
    //     if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
    //         continue;

    //     if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
    //         continue;

    //     if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_PALADIN && proto->SubClass !=
    //     ITEM_SUBCLASS_ARMOR_SHIELD)
    //         continue;
    // }

    // uint8 sp = 0;
    // uint8 ap = 0;
    // uint8 tank = 0;
    // for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    // {
    //     // for ItemStatValue != 0
    //     if(!proto->ItemStat[j].ItemStatValue)
    //         continue;

    //     AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    // }

    // return CheckItemStats(sp, ap, tank);
}

bool PlayerbotFactory::CheckItemStats(uint8 sp, uint8 ap, uint8 tank)
{
    switch (bot->getClass())
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
        case CLASS_DEATH_KNIGHT:
            if ((!ap && !tank) || sp > ap || sp > tank)
                return false;
            break;
    }

    return sp || ap || tank;
}

void PlayerbotFactory::AddItemStats(uint32 mod, uint8& sp, uint8& ap, uint8& tank)
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

bool PlayerbotFactory::CanEquipWeapon(ItemTemplate const* proto)
{
    switch (bot->getClass())
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

bool PlayerbotFactory::CanEquipItem(ItemTemplate const* proto)
{
    if (proto->Duration != 0)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM /*|| proto->Bonding == BIND_WHEN_USE*/)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    uint32 requiredLevel = proto->RequiredLevel;
    // disable since bad performance
    bool hasItem = bot->HasItemCount(proto->ItemId, 1, false);
    // bot->GetItemCount()
    // !requiredLevel -> it's a quest reward item
    if (!requiredLevel && hasItem)
        return false;

    uint32 level = bot->GetLevel();

    if (requiredLevel > level)
        return false;

    return true;
}

void Shuffle(std::vector<uint32>& items)
{
    uint32 count = items.size();
    for (uint32 i = 0; i < count * 5; i++)
    {
        int i1 = urand(0, count - 1);
        int i2 = urand(0, count - 1);

        uint32 item = items[i1];
        items[i1] = items[i2];
        items[i2] = item;
    }
}

// void PlayerbotFactory::InitEquipmentNew(bool incremental)
// {
//     if (incremental)
//     {
//         DestroyItemsVisitor visitor(bot);
//         IterateItems(&visitor, (IterateItemsMask)(ITERATE_ITEMS_IN_BAGS | ITERATE_ITEMS_IN_BANK));
//     }
//     else
//     {
//     DestroyItemsVisitor visitor(bot);
//     IterateItems(&visitor, ITERATE_ALL_ITEMS);
//     }

//     std::string const& specName = AiFactory::GetPlayerSpecName(bot);
//     if (specName.empty())
//         return;

//     // look for upgrades
//     for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
//     {
//         if (slot == EQUIPMENT_SLOT_TABARD && !bot->GetGuildId())
//             continue;

//         bool isUpgrade = false;
//         bool found = false;
//         bool noItem = false;
//         uint32 quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_EPIC);
//         uint32 attempts = 10;
//         if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && quality > ITEM_QUALITY_NORMAL) {
//             quality--;
//         }
//         // current item;
//         Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
//         if (oldItem)
//             isUpgrade = true;

//         uint32 itemInSlot = isUpgrade ? oldItem->GetTemplate()->ItemId : 0;

//         uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
//         if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
//             maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

//         uint32 minLevel = sPlayerbotAIConfig->randomBotMinLevel;
//         if (minLevel < sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL))
//             minLevel = sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL);

//         // test
//         do
//         {
//             if (isUpgrade)
//             {
//                 std::vector<uint32> ids = sRandomItemMgr->GetUpgradeList(bot, specName, slot, 0, itemInSlot);
//                 if (!ids.empty())
//                     Shuffle(ids);

//                 for (uint32 index = 0; index < ids.size(); ++index)
//                 {
//                     uint32 newItemId = ids[index];
//                     if (incremental && !IsDesiredReplacement(oldItem))
//                     {
//                         continue;
//                     }

//                     uint16 dest;
//                     if (!CanEquipUnseenItem(slot, dest, newItemId))
//                         continue;

//                     if (oldItem)
//                     {
//                         bot->RemoveItem(INVENTORY_SLOT_BAG_0, slot, true);
//                         oldItem->DestroyForPlayer(bot);
//                     }

//                     Item* newItem = bot->EquipNewItem(dest, newItemId, true);
//                     if (newItem)
//                     {
//                         newItem->AddToWorld();
//                         newItem->AddToUpdateQueueOf(bot);
//                         bot->AutoUnequipOffhandIfNeed();
//                         newItem->SetOwnerGUID(bot->GetGUID());
//                         EnchantItem(newItem);
//                         LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Equip: {}, slot: {}, Old item: {}",
//                             bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H",
//                             bot->GetLevel(), bot->GetName(), newItemId, slot, itemInSlot);
//                         found = true;
//                         break;
//                     }
//                 }
//             }
//             else
//             {
//                 std::vector<uint32> ids = sRandomItemMgr->GetUpgradeList(bot, specName, slot, quality, itemInSlot);
//                 if (!ids.empty())
//                     Shuffle(ids);

//                 for (uint32 index = 0; index < ids.size(); ++index)
//                 {
//                     uint32 newItemId = ids[index];
//                     uint16 dest;
//                     if (!CanEquipUnseenItem(slot, dest, newItemId))
//                         continue;

//                     Item* newItem = bot->EquipNewItem(dest, newItemId, true);
//                     if (newItem)
//                     {
//                         bot->AutoUnequipOffhandIfNeed();
//                         newItem->SetOwnerGUID(bot->GetGUID());
//                         EnchantItem(newItem);
//                         found = true;
//                         LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Equip: {}, slot: {}",
//                             bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H",
//                             bot->GetLevel(), bot->GetName(), newItemId, slot);
//                         break;
//                     }
//                 }
//             }
//             quality--;
//         } while (!found && quality != ITEM_QUALITY_POOR);
//         if (!found)
//         {
//             LOG_INFO("playerbots", "Bot {} {}:{} <{}>: no item for slot {}",
//                 bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->GetLevel(),
//                 bot->GetName(), slot);
//             continue;
//         }
//     }
// }

void PlayerbotFactory::InitEquipment(bool incremental, bool second_chance)
{
    if (incremental && !sPlayerbotAIConfig->incrementalGearInit)
        return;

    if (level < 5) {
        // original items
        if (CharStartOutfitEntry const* oEntry = GetCharStartOutfitEntry(bot->getRace(), bot->getClass(), bot->getGender()))
        {
            for (int j = 0; j < MAX_OUTFIT_ITEMS; ++j)
            {
                if (oEntry->ItemId[j] <= 0)
                    continue;

                uint32 itemId = oEntry->ItemId[j];

                // skip hearthstone
                if (itemId == 6948)
                    continue;

                // just skip, reported in ObjectMgr::LoadItemTemplates
                ItemTemplate const* iProto = sObjectMgr->GetItemTemplate(itemId);
                if (!iProto)
                    continue;

                // BuyCount by default
                uint32 count = iProto->BuyCount;

                // special amount for food/drink
                if (iProto->Class == ITEM_CLASS_CONSUMABLE && iProto->SubClass == ITEM_SUBCLASS_FOOD)
                {
                    continue;
                }

                if (bot->HasItemCount(itemId, count)) {
                    continue;
                }

                bot->StoreNewItemInBestSlots(itemId, count);
            }
        }
        return;
    }

    std::unordered_map<uint8, std::vector<uint32>> items;
    // int tab = AiFactory::GetPlayerSpecTab(bot);

    uint32 blevel = bot->GetLevel();
    int32 delta = std::min(blevel, 10u);

    StatsWeightCalculator calculator(bot);
    for (int32 slot : initSlotsOrder)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;

        if (level < 50 && (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2))
            continue;

        if (level < 30 && (slot == EQUIPMENT_SLOT_NECK || slot == EQUIPMENT_SLOT_HEAD))
            continue;

        if (level < 20 && (slot == EQUIPMENT_SLOT_FINGER1 || slot == EQUIPMENT_SLOT_FINGER2))
            continue;

        if (level < 5 && (slot != EQUIPMENT_SLOT_MAINHAND) && (slot != EQUIPMENT_SLOT_OFFHAND) &&
            (slot != EQUIPMENT_SLOT_FEET) && (slot != EQUIPMENT_SLOT_LEGS) && (slot != EQUIPMENT_SLOT_CHEST) &&
            (slot != EQUIPMENT_SLOT_RANGED))
            continue;

        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (second_chance && oldItem)
        {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }

        oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        int32 desiredQuality = itemQuality;
        if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL)
        {
            desiredQuality--;
        }
        do
        {
            for (uint32 requiredLevel = bot->GetLevel(); requiredLevel > std::max((int32)bot->GetLevel() - delta, 0);
                 requiredLevel--)
            {
                for (InventoryType inventoryType : GetPossibleInventoryTypeListBySlot((EquipmentSlots)slot))
                {
                    for (uint32 itemId : sRandomItemMgr->GetCachedEquipments(requiredLevel, inventoryType))
                    {
                        if (itemId == 46978)  // shaman earth ring totem
                        {
                            continue;
                        }
                        uint32 skipProb = 25;
                        if (urand(1, 100) <= skipProb)
                            continue;

                        // disable next expansion gear
                        if (sPlayerbotAIConfig->limitGearExpansion && bot->GetLevel() <= 60 && itemId >= 23728)
                            continue;

                        if (sPlayerbotAIConfig->limitGearExpansion && bot->GetLevel() <= 70 && itemId >= 35570 &&
                            itemId != 36737 && itemId != 37739 &&
                            itemId != 37740)  // transition point from TBC -> WOTLK isn't as clear, and there are other
                                              // wearable TBC items above 35570 but nothing of significance
                            continue;

                        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                        if (!proto)
                            continue;

                        bool shouldCheckGS = desiredQuality > ITEM_QUALITY_NORMAL;

                        if (shouldCheckGS && gearScoreLimit != 0 &&
                            CalcMixedGearScore(proto->ItemLevel, proto->Quality) > gearScoreLimit)
                        {
                            continue;
                        }
                        if (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR)
                            continue;

                        if (proto->Quality != desiredQuality)
                            continue;

                        if (proto->Class == ITEM_CLASS_ARMOR &&
                            (slot == EQUIPMENT_SLOT_HEAD || slot == EQUIPMENT_SLOT_SHOULDERS ||
                             slot == EQUIPMENT_SLOT_CHEST || slot == EQUIPMENT_SLOT_WAIST ||
                             slot == EQUIPMENT_SLOT_LEGS || slot == EQUIPMENT_SLOT_FEET ||
                             slot == EQUIPMENT_SLOT_WRISTS || slot == EQUIPMENT_SLOT_HANDS) &&
                            !CanEquipArmor(proto))
                            continue;

                        if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(proto))
                            continue;

                        if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_ROGUE &&
                            proto->Class != ITEM_CLASS_WEAPON)
                            continue;
                        items[slot].push_back(itemId);
                    }
                }
            }
        } while (items[slot].size() < 25 && desiredQuality-- > ITEM_QUALITY_POOR);

        std::vector<uint32>& ids = items[slot];
        if (ids.empty())
        {
            continue;
        }

        float bestScoreForSlot = -1;
        uint32 bestItemForSlot = 0;
        for (int index = 0; index < ids.size(); index++)
        {
            uint32 newItemId = ids[index];

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(newItemId);

            float cur_score = calculator.CalculateItem(newItemId);
            if (cur_score > bestScoreForSlot)
            {
                // delay heavy check to here
                if (!CanEquipItem(proto))
                    continue;
                uint16 dest;
                if (!CanEquipUnseenItem(slot, dest, newItemId))
                    continue;
                bestScoreForSlot = cur_score;
                bestItemForSlot = newItemId;
            }
        }

        if (bestItemForSlot == 0)
        {
            continue;
        }
        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, bestItemForSlot))
        {
            continue;
        }

        if (incremental && oldItem)
        {
            float old_score = calculator.CalculateItem(oldItem->GetEntry(), oldItem->GetItemRandomPropertyId());
            if (bestScoreForSlot < 1.2f * old_score)
                continue;
        }
        if (oldItem)
        {
            uint8 bagIndex = oldItem->GetBagSlot();
            uint8 slot = oldItem->GetSlot();
            uint8 dstBag = NULL_BAG;

            WorldPacket packet(CMSG_AUTOSTORE_BAG_ITEM, 3);
            packet << bagIndex << slot << dstBag;
            WorldPackets::Item::AutoStoreBagItem nicePacket(std::move(packet));
            nicePacket.Read();
            bot->GetSession()->HandleAutoStoreBagItemOpcode(nicePacket);
        }

        oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        // fail to store in bag
        if (oldItem)
            continue;

        Item* newItem = bot->EquipNewItem(dest, bestItemForSlot, true);
        bot->AutoUnequipOffhandIfNeed();
        // if (newItem)
        // {
        //     newItem->AddToWorld();
        // newItem->AddToUpdateQueueOf(bot);
        // }
    }
    // Secondary init for better equips
    /// @todo: clean up duplicate code
    if (second_chance)
    {
        for (int32 slot : initSlotsOrder)
        {
            if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
                continue;

            if (level < 50 && (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2))
                continue;

            if (level < 30 && (slot == EQUIPMENT_SLOT_NECK || slot == EQUIPMENT_SLOT_HEAD))
                continue;

            if (level < 20 && (slot == EQUIPMENT_SLOT_FINGER1 || slot == EQUIPMENT_SLOT_FINGER2))
                continue;

            if (level < 5 && (slot != EQUIPMENT_SLOT_MAINHAND) && (slot != EQUIPMENT_SLOT_OFFHAND) &&
                (slot != EQUIPMENT_SLOT_FEET) && (slot != EQUIPMENT_SLOT_LEGS) && (slot != EQUIPMENT_SLOT_CHEST) &&
                (slot != EQUIPMENT_SLOT_RANGED))
                continue;

            if (Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

            std::vector<uint32>& ids = items[slot];
            if (ids.empty())
                continue;

            float bestScoreForSlot = -1;
            uint32 bestItemForSlot = 0;
            for (int index = 0; index < ids.size(); index++)
            {
                uint32 newItemId = ids[index];

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(newItemId);

                float cur_score = calculator.CalculateItem(newItemId);
                if (cur_score > bestScoreForSlot)
                {
                    // delay heavy check to here
                    if (!CanEquipItem(proto))
                        continue;
                    uint16 dest;
                    if (!CanEquipUnseenItem(slot, dest, newItemId))
                        continue;
                    bestScoreForSlot = cur_score;
                    bestItemForSlot = newItemId;
                }
            }

            if (bestItemForSlot == 0)
            {
                continue;
            }
            uint16 dest;
            if (!CanEquipUnseenItem(slot, dest, bestItemForSlot))
            {
                continue;
            }
            Item* newItem = bot->EquipNewItem(dest, bestItemForSlot, true);
            bot->AutoUnequipOffhandIfNeed();
            // if (newItem)
            // {
            //     newItem->AddToWorld();
            //     newItem->AddToUpdateQueueOf(bot);
            // }
        }
    }
}

bool PlayerbotFactory::IsDesiredReplacement(Item* item)
{
    if (!item)
        return true;

    ItemTemplate const* proto = item->GetTemplate();
    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
    {
        return true;
    }
    // if (!requiredLevel)
    // {
    //     requiredLevel = sRandomItemMgr->GetMinLevelFromCache(proto->ItemId);
    // }

    uint32 delta = 1 + (80 - bot->GetLevel()) / 10;
    return proto->Quality < ITEM_QUALITY_RARE || int32(bot->GetLevel() - requiredLevel) > delta;
}

inline Item* StoreNewItemInInventorySlot(Player* player, uint32 newItemId, uint32 count)
{
    ItemPosCountVec vDest;
    InventoryResult msg = player->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, vDest, newItemId, count);
    if (msg == EQUIP_ERR_OK)
    {
        if (Item* newItem = player->StoreNewItem(vDest, newItemId, true, Item::GenerateItemRandomPropertyId(newItemId)))
        {
            return newItem;
        }
    }

    return nullptr;
}

// void PlayerbotFactory::InitSecondEquipmentSet()
// {
//     if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_PRIEST)
//         return;

//     std::map<uint32, std::vector<uint32>> items;

//     uint32 desiredQuality = itemQuality;
//     while (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && desiredQuality >
//     ITEM_QUALITY_NORMAL)
//     {
//         desiredQuality--;
//     }

//     ItemTemplateContainer const* itemTemplate = sObjectMgr->GetItemTemplateStore();
//     do
//     {
//         for (auto const& itr : *itemTemplate)
//         {
//             ItemTemplate const* proto = &itr.second;
//             if (!proto)
//                 continue;
//             if (!CanEquipItem(proto, desiredQuality))
//                 continue;

//             if (proto->Class == ITEM_CLASS_WEAPON)
//             {
//                 //if (!CanEquipWeapon(proto))
//                 //    continue;

//                 if (sRandomItemMgr->HasStatWeight(proto->ItemId))
//                 {
//                     if (!sRandomItemMgr->GetLiveStatWeight(bot, proto->ItemId))
//                         continue;
//                 }

//                 Item* existingItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
//                 if (existingItem)
//                 {
//                     switch (existingItem->GetTemplate()->SubClass)
//                     {
//                         case ITEM_SUBCLASS_WEAPON_AXE:
//                         case ITEM_SUBCLASS_WEAPON_DAGGER:
//                         case ITEM_SUBCLASS_WEAPON_FIST:
//                         case ITEM_SUBCLASS_WEAPON_MACE:
//                         case ITEM_SUBCLASS_WEAPON_SWORD:
//                             if (proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || proto->SubClass ==
//                             ITEM_SUBCLASS_WEAPON_DAGGER || proto->SubClass == ITEM_SUBCLASS_WEAPON_FIST ||
//                                 proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || proto->SubClass ==
//                                 ITEM_SUBCLASS_WEAPON_SWORD) continue;
//                             break;
//                         default:
//                             if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass !=
//                             ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
//                                 proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass !=
//                                 ITEM_SUBCLASS_WEAPON_SWORD) continue;
//                             break;
//                     }
//                 }
//             }
//             else if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
//             {
//                 //if (!CanEquipArmor(proto))
//                 //    continue;

//                 if (sRandomItemMgr->HasStatWeight(proto->ItemId))
//                 {
//                     if (!sRandomItemMgr->GetLiveStatWeight(bot, proto->ItemId))
//                         continue;
//                 }

//                 if (Item* existingItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
//                     if (existingItem->GetTemplate()->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
//                         continue;
//             }
//             else
//                 continue;

//             items[proto->Class].push_back(itr.first);
//         }
//     } while (items[ITEM_CLASS_ARMOR].empty() && items[ITEM_CLASS_WEAPON].empty() && desiredQuality-- >
//     ITEM_QUALITY_NORMAL);

//     for (std::map<uint32, std::vector<uint32>>::iterator i = items.begin(); i != items.end(); ++i)
//     {
//         std::vector<uint32>& ids = i->second;
//         if (ids.empty())
//         {
//             LOG_DEBUG("playerbots",   "{}: no items to make second equipment set for slot {}",
//             bot->GetName().c_str(), i->first); continue;
//         }

//         for (uint32 attempts = 0; attempts < 15; attempts++)
//         {
//             uint32 index = urand(0, ids.size() - 1);
//             uint32 newItemId = ids[index];

//             if (Item* newItem = StoreNewItemInInventorySlot(bot, newItemId, 1))
//             {
//                 newItem->SetOwnerGUID(bot->GetGUID());
//                 EnchantItem(newItem);
//                 newItem->AddToWorld();
//                 newItem->AddToUpdateQueueOf(bot);
//                 break;
//             }
//         }
//     }
// }

void PlayerbotFactory::InitBags(bool destroyOld)
{
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        uint32 newItemId = 51809;
        Item* old_bag = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (old_bag && old_bag->GetTemplate()->ItemId == newItemId)
        {
            continue;
        }
        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, newItemId))
            continue;

        if (old_bag && destroyOld)
        {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }
        if (old_bag)
        {
            continue;
        }
        Item* newItem = bot->EquipNewItem(dest, newItemId, true);
        // if (newItem)
        // {
        //     newItem->AddToWorld();
        //     newItem->AddToUpdateQueueOf(bot);
        // }
    }
}

void PlayerbotFactory::EnchantItem(Item* item)
{
    if (bot->GetLevel() < sPlayerbotAIConfig->minEnchantingBotLevel)
        return;

    if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance)
        return;

    ItemTemplate const* proto = item->GetTemplate();
    uint32 itemLevel = proto->ItemLevel;

    std::vector<uint32> ids;

    for (uint32 id = 1; id < sSpellMgr->GetSpellInfoStoreSize(); ++id)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(id);
        if (!spellInfo)
            continue;

        uint32 requiredLevel = spellInfo->BaseLevel;
        if (requiredLevel && (requiredLevel > itemLevel || requiredLevel < itemLevel - 35))
            continue;

        if (spellInfo->MaxLevel && level > spellInfo->MaxLevel)
            continue;

        uint32 spellLevel = spellInfo->SpellLevel;
        if (spellLevel && (spellLevel > level || spellLevel < level - 10))
            continue;

        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->Effects[j].Effect != SPELL_EFFECT_ENCHANT_ITEM)
                continue;

            uint32 enchant_id = spellInfo->Effects[j].MiscValue;
            if (!enchant_id)
                continue;

            SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchant || enchant->slot != PERM_ENCHANTMENT_SLOT)
                continue;

            SpellInfo const* enchantSpell = sSpellMgr->GetSpellInfo(enchant->spellid[0]);
            if (!enchantSpell || (enchantSpell->SpellLevel && enchantSpell->SpellLevel > level))
                continue;

            uint8 sp = 0;
            uint8 ap = 0;
            uint8 tank = 0;
            for (uint8 i = ITEM_MOD_MANA; i < MAX_ITEM_MOD; ++i)
            {
                if (enchant->type[i] != ITEM_ENCHANTMENT_TYPE_STAT)
                    continue;

                AddItemStats(enchant->spellid[i], sp, ap, tank);
            }

            if (!CheckItemStats(sp, ap, tank))
                continue;

            if (enchant->EnchantmentCondition && !bot->EnchantmentFitsRequirements(enchant->EnchantmentCondition, -1))
                continue;

            if (!item->IsFitToSpellRequirements(spellInfo))
                continue;

            ids.push_back(enchant_id);
        }
    }

    if (ids.empty())
    {
        LOG_DEBUG("playerbots", "{}: no enchantments found for item {}", bot->GetName().c_str(),
                  item->GetTemplate()->ItemId);
        return;
    }

    uint32 index = urand(0, ids.size() - 1);
    uint32 id = ids[index];

    SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(id);
    if (!enchant)
        return;

    bot->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
    item->SetEnchantment(PERM_ENCHANTMENT_SLOT, id, 0, 0, bot->GetGUID());
    bot->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
}

bool PlayerbotFactory::CanEquipUnseenItem(uint8 slot, uint16& dest, uint32 item)
{
    dest = 0;

    if (Item* pItem = Item::CreateItem(item, 1, bot, false, 0, true))
    {
        InventoryResult result = botAI ? botAI->CanEquipItem(slot, dest, pItem, true, true)
                                       : bot->CanEquipItem(slot, dest, pItem, true, true);
        pItem->RemoveFromUpdateQueueOf(bot);
        delete pItem;
        return result == EQUIP_ERR_OK;
    }

    return false;
}

void PlayerbotFactory::InitTradeSkills()
{
    uint16 firstSkill = sRandomPlayerbotMgr->GetValue(bot, "firstSkill");
    uint16 secondSkill = sRandomPlayerbotMgr->GetValue(bot, "secondSkill");
    if (!firstSkill || !secondSkill)
    {
        std::vector<uint32> firstSkills;
        std::vector<uint32> secondSkills;

        switch (bot->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_DEATH_KNIGHT:
                firstSkills.push_back(SKILL_MINING);
                secondSkills.push_back(SKILL_BLACKSMITHING);
                secondSkills.push_back(SKILL_ENGINEERING);
                secondSkills.push_back(SKILL_JEWELCRAFTING);
                break;
            case CLASS_SHAMAN:
            case CLASS_DRUID:
            case CLASS_HUNTER:
            case CLASS_ROGUE:
                firstSkills.push_back(SKILL_SKINNING);
                secondSkills.push_back(SKILL_LEATHERWORKING);
                break;
            default:
                firstSkills.push_back(SKILL_TAILORING);
                secondSkills.push_back(SKILL_ENCHANTING);
        }

        switch (urand(0, 6))
        {
            case 0:
                firstSkill = SKILL_HERBALISM;
                secondSkill = SKILL_ALCHEMY;
                break;
            case 1:
                firstSkill = SKILL_HERBALISM;
                secondSkill = SKILL_MINING;
                break;
            case 2:
                firstSkill = SKILL_MINING;
                secondSkill = SKILL_SKINNING;
                break;
            case 3:
                firstSkill = SKILL_HERBALISM;
                secondSkill = SKILL_SKINNING;
                break;
            default:
                firstSkill = firstSkills[urand(0, firstSkills.size() - 1)];
                secondSkill = secondSkills[urand(0, secondSkills.size() - 1)];
                break;
        }

        sRandomPlayerbotMgr->SetValue(bot, "firstSkill", firstSkill);
        sRandomPlayerbotMgr->SetValue(bot, "secondSkill", secondSkill);
    }

    SetRandomSkill(SKILL_FIRST_AID);
    SetRandomSkill(SKILL_FISHING);
    SetRandomSkill(SKILL_COOKING);

    SetRandomSkill(firstSkill);
    SetRandomSkill(secondSkill);
}

void PlayerbotFactory::UpdateTradeSkills()
{
    for (uint32 i = 0; i < sizeof(tradeSkills) / sizeof(uint32); ++i)
    {
        if (bot->GetSkillValue(tradeSkills[i]) == 1)
            bot->SetSkill(tradeSkills[i], 0, 0, 0);
    }
}

void PlayerbotFactory::InitSkills()
{
    //uint32 maxValue = level * 5; //not used, line marked for removal.
    bot->UpdateSkillsForLevel();

    bot->SetSkill(SKILL_RIDING, 0, 0, 0);
    if (bot->GetLevel() >= sPlayerbotAIConfig->useGroundMountAtMinLevel)
        bot->learnSpell(33388);
    if (bot->GetLevel() >= sPlayerbotAIConfig->useFastGroundMountAtMinLevel)
        bot->learnSpell(33391);
    if (bot->GetLevel() >= sPlayerbotAIConfig->useFlyMountAtMinLevel)
        bot->learnSpell(34090);
    if (bot->GetLevel() >= sPlayerbotAIConfig->useFastFlyMountAtMinLevel)
        bot->learnSpell(34091);

    uint32 skillLevel = bot->GetLevel() < 40 ? 0 : 1;
    uint32 dualWieldLevel = bot->GetLevel() < 20 ? 0 : 1;
    SetRandomSkill(SKILL_DEFENSE);
    SetRandomSkill(SKILL_UNARMED);
    switch (bot->getClass())
    {
        case CLASS_DRUID:
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_POLEARMS);
            SetRandomSkill(SKILL_FIST_WEAPONS);
            break;
        case CLASS_WARRIOR:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_BOWS);
            SetRandomSkill(SKILL_GUNS);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_2H_SWORDS);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_CROSSBOWS);
            SetRandomSkill(SKILL_POLEARMS);
            SetRandomSkill(SKILL_FIST_WEAPONS);
            SetRandomSkill(SKILL_THROWN);
            bot->SetSkill(SKILL_DUAL_WIELD, 0, dualWieldLevel, dualWieldLevel);
            bot->SetSkill(SKILL_PLATE_MAIL, 0, skillLevel, skillLevel);
            bot->SetCanDualWield(dualWieldLevel);
            break;
        case CLASS_PALADIN:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_2H_SWORDS);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_POLEARMS);
            bot->SetSkill(SKILL_PLATE_MAIL, 0, skillLevel, skillLevel);
            break;
        case CLASS_PRIEST:
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_WANDS);
            break;
        case CLASS_SHAMAN:
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_FIST_WEAPONS);
            bot->SetSkill(SKILL_MAIL, 0, skillLevel, skillLevel);
            break;
        case CLASS_MAGE:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_WANDS);
            break;
        case CLASS_WARLOCK:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_WANDS);
            break;
        case CLASS_HUNTER:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_BOWS);
            SetRandomSkill(SKILL_GUNS);
            SetRandomSkill(SKILL_2H_SWORDS);
            SetRandomSkill(SKILL_STAVES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_CROSSBOWS);
            SetRandomSkill(SKILL_POLEARMS);
            SetRandomSkill(SKILL_FIST_WEAPONS);
            SetRandomSkill(SKILL_THROWN);
            bot->SetSkill(SKILL_DUAL_WIELD, 0, dualWieldLevel, dualWieldLevel);
            bot->SetSkill(SKILL_MAIL, 0, skillLevel, skillLevel);
            bot->SetCanDualWield(dualWieldLevel);
            break;
        case CLASS_ROGUE:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_BOWS);
            SetRandomSkill(SKILL_GUNS);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_DAGGERS);
            SetRandomSkill(SKILL_CROSSBOWS);
            SetRandomSkill(SKILL_FIST_WEAPONS);
            SetRandomSkill(SKILL_THROWN);
            SetRandomSkill(SKILL_LOCKPICKING);
            bot->SetSkill(SKILL_DUAL_WIELD, 0, 1, 1);
            bot->SetCanDualWield(true);
            break;
        case CLASS_DEATH_KNIGHT:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_2H_SWORDS);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_POLEARMS);
            bot->SetSkill(SKILL_DUAL_WIELD, 0, dualWieldLevel, dualWieldLevel);
            bot->SetCanDualWield(dualWieldLevel);
            break;
        default:
            break;
    }

    // switch (bot->getClass())
    // {
    //     case CLASS_WARRIOR:
    //     case CLASS_PALADIN:
    //         bot->SetSkill(SKILL_PLATE_MAIL, 0, skillLevel, skillLevel);
    //         break;
    //     case CLASS_SHAMAN:
    //     case CLASS_HUNTER:
    //         bot->SetSkill(SKILL_MAIL, 0, skillLevel, skillLevel);
    //         break;
    //     default:
    //         break;
    // }
}

void PlayerbotFactory::SetRandomSkill(uint16 id)
{
    uint32 maxValue = level * 5;

    // do not let skill go beyond limit even if maxlevel > blizzlike
    // if (level > 60)
    // {
    //     maxValue = (level + 10) * 5;
    // }

    // uint32 value = urand(maxValue - level, maxValue);
    uint32 value = maxValue;
    //uint32 curValue = bot->GetSkillValue(id); //not used, line marked for removal.

    uint16 step = bot->GetSkillValue(id) ? bot->GetSkillStep(id) : 1;

    // if (!bot->HasSkill(id) || value > curValue)
    bot->SetSkill(id, step, value, maxValue);
}

void PlayerbotFactory::InitAvailableSpells()
{
    if (trainerIdCache[bot->getClass()].empty())
    {
        CreatureTemplateContainer const* creatureTemplateContainer = sObjectMgr->GetCreatureTemplates();
        for (CreatureTemplateContainer::const_iterator i = creatureTemplateContainer->begin();
             i != creatureTemplateContainer->end(); ++i)
        {
            CreatureTemplate const& co = i->second;
            if (co.trainer_type != TRAINER_TYPE_TRADESKILLS && co.trainer_type != TRAINER_TYPE_CLASS)
                continue;

            if (co.trainer_type == TRAINER_TYPE_CLASS && co.trainer_class != bot->getClass())
                continue;

            uint32 trainerId = co.Entry;
            trainerIdCache[bot->getClass()].push_back(trainerId);
        }
    }
    for (uint32 trainerId : trainerIdCache[bot->getClass()])
    {
        TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);

        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin();
             itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            if (tSpell->learnedSpell[0] && !bot->IsSpellFitByClassAndRace(tSpell->learnedSpell[0]))
                continue;

            TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
            if (state != TRAINER_SPELL_GREEN)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(tSpell->spell);
            bool learn = true;
            for (uint8 j = 0; j < 3; ++j)
            {
                if (!tSpell->learnedSpell[j] && !bot->IsSpellFitByClassAndRace(tSpell->learnedSpell[j]))
                    continue;

                if (spellInfo->Effects[j].Effect == SPELL_EFFECT_PROFICIENCY ||
                    (spellInfo->Effects[j].Effect == SPELL_EFFECT_SKILL_STEP &&
                     spellInfo->Effects[j].MiscValue != SKILL_RIDING) ||
                    spellInfo->Effects[j].Effect == SPELL_EFFECT_DUAL_WIELD)
                {
                    learn = false;
                    break;
                }
            }
            if (!learn)
            {
                continue;
            }

            if (tSpell->IsCastable())
                bot->CastSpell(bot, tSpell->spell, true);
            else
                bot->learnSpell(tSpell->learnedSpell[0], false);
        }
    }
}

void PlayerbotFactory::InitClassSpells()
{
    int32_t level = bot->GetLevel();
    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            bot->learnSpell(78, true);
            bot->learnSpell(2457, true);
            if (level >= 10)
            {
                bot->learnSpell(71, false);    // Defensive Stance
                bot->learnSpell(355, false);   // Taunt
                bot->learnSpell(7386, false);  // Sunder Armor
            }
            if (level >= 30)
            {
                bot->learnSpell(2458, false);  // Berserker Stance
            }
            break;
        case CLASS_PALADIN:
            bot->learnSpell(21084, true);
            bot->learnSpell(635, true);
            if (level >= 12)
            {
                bot->learnSpell(7328, false);  // Redemption
            }
            break;
        case CLASS_ROGUE:
            bot->learnSpell(1752, true);
            bot->learnSpell(2098, true);
            break;
        case CLASS_DEATH_KNIGHT:
            bot->learnSpell(45477, true);
            bot->learnSpell(47541, true);
            bot->learnSpell(45462, true);
            bot->learnSpell(45902, true);
            // to leave DK starting area
            bot->learnSpell(53428, false);
            bot->learnSpell(50977, false);
            bot->learnSpell(49142, false);
            bot->learnSpell(48778, false);
            break;
        case CLASS_HUNTER:
            bot->learnSpell(2973, true);
            bot->learnSpell(75, true);
            if (level >= 10)
            {
                bot->learnSpell(883, false);   // call pet
                bot->learnSpell(1515, false);  // tame pet
                bot->learnSpell(6991, false);  // feed pet
                bot->learnSpell(982, false);   // revive pet
                bot->learnSpell(2641, false);  // dismiss pet
            }
            break;
        case CLASS_PRIEST:
            bot->learnSpell(585, true);
            bot->learnSpell(2050, true);
            break;
        case CLASS_MAGE:
            bot->learnSpell(133, true);
            bot->learnSpell(168, true);
            break;
        case CLASS_WARLOCK:
            bot->learnSpell(687, true);
            bot->learnSpell(686, true);
            bot->learnSpell(688, false);  // summon imp
            if (level >= 10)
            {
                bot->learnSpell(697, false);  // summon voidwalker
            }
            if (level >= 20)
            {
                bot->learnSpell(712, false);  // summon succubus
            }
            if (level >= 30)
            {
                bot->learnSpell(691, false);  // summon felhunter
            }
            break;
        case CLASS_DRUID:
            bot->learnSpell(5176, true);
            bot->learnSpell(5185, true);
            if (level >= 10)
            {
                bot->learnSpell(5487, false);  // bear form
                bot->learnSpell(6795, false);  // Growl
                bot->learnSpell(6807, false);  // Maul
            }
            break;
        case CLASS_SHAMAN:
            bot->learnSpell(403, true);
            bot->learnSpell(331, true);
            // bot->learnSpell(66747, true); // Totem of the Earthen Ring
            if (level >= 4)
            {
                bot->learnSpell(8071, false);  // stoneskin totem
            }
            if (level >= 10)
            {
                bot->learnSpell(3599, false);  // searing totem
            }
            if (level >= 20)
            {
                bot->learnSpell(5394, false);  // healing stream totem
            }
            break;
        default:
            break;
    }
}

void PlayerbotFactory::InitSpecialSpells()
{
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotSpellIds.begin();
         i != sPlayerbotAIConfig->randomBotSpellIds.end(); ++i)
    {
        uint32 spellId = *i;
        bot->learnSpell(spellId);
    }
    // to leave DK starting area
    if (bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        bot->learnSpell(50977, false);
    }
}

void PlayerbotFactory::InitTalents(uint32 specNo)
{
    uint32 classMask = bot->getClassMask();
    std::unordered_map<uint32, std::vector<TalentEntry const*>> spells;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo || talentTabInfo->tabpage != specNo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells[talentInfo->Row].push_back(talentInfo);
    }

    uint32 freePoints = bot->GetFreeTalentPoints();
    for (auto i = spells.begin(); i != spells.end(); ++i)
    {
        std::vector<TalentEntry const*>& spells_row = i->second;
        if (spells_row.empty())
        {
            LOG_INFO("playerbots", "{}: No spells for talent row {}", bot->GetName().c_str(), i->first);
            continue;
        }
        int attemptCount = 0;
        while (!spells_row.empty() && (int)freePoints - (int)bot->GetFreeTalentPoints() < 5 && attemptCount++ < 3 &&
               bot->GetFreeTalentPoints())
        {
            int index = urand(0, spells_row.size() - 1);
            TalentEntry const* talentInfo = spells_row[index];
            int maxRank = 0;
            for (int rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, bot->GetFreeTalentPoints()); ++rank)
            {
                uint32 spellId = talentInfo->RankID[rank];
                if (!spellId)
                    continue;

                maxRank = rank;
            }
            if (talentInfo->DependsOn)
            {
                bot->LearnTalent(talentInfo->DependsOn,
                                 std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));
            }
            bot->LearnTalent(talentInfo->TalentID, maxRank);
            spells_row.erase(spells_row.begin() + index);
        }

        freePoints = bot->GetFreeTalentPoints();
    }
}

void PlayerbotFactory::InitTalentsByTemplate(uint32 specTab)
{
    // if (sPlayerbotAIConfig->parsedSpecLinkOrder[bot->getClass()][specNo][80].size() == 0) {
    //     return;
    // }
    uint32 cls = bot->getClass();
    int startLevel = bot->GetLevel();
    uint32 specIndex = sPlayerbotAIConfig->randomClassSpecIndex[cls][specTab];
    uint32 classMask = bot->getClassMask();
    std::unordered_map<uint32, std::vector<TalentEntry const*>> spells_row;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells_row[talentInfo->Row].push_back(talentInfo);
    }
    while (startLevel > 1 && startLevel < 80 &&
           sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specIndex][startLevel].size() == 0)
    {
        startLevel--;
    }
    for (int level = startLevel; level <= 80; level++)
    {
        if (sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specIndex][level].size() == 0)
        {
            continue;
        }
        for (std::vector<uint32>& p : sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specIndex][level])
        {
            uint32 tab = p[0], row = p[1], col = p[2], lvl = p[3];
            if (sPlayerbotAIConfig->limitTalentsExpansion && bot->GetLevel() <= 60 && (row > 6 || (row == 6 && col != 1)))
                continue;

            if (sPlayerbotAIConfig->limitTalentsExpansion && bot->GetLevel() <= 70 && (row > 8 || (row == 8 && col != 1)))
                continue;

            uint32 talentID = 0;
            uint32 learnLevel = 0;
            std::vector<TalentEntry const*>& spells = spells_row[row];
            if (spells.size() <= 0)
            {
                return;
            }
            for (TalentEntry const* talentInfo : spells)
            {
                if (talentInfo->Col != col)
                {
                    continue;
                }
                TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
                if (talentTabInfo->tabpage != tab)
                {
                    continue;
                }
                if (talentInfo->DependsOn)
                {
                    bot->LearnTalent(talentInfo->DependsOn,
                                     std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));
                }
                talentID = talentInfo->TalentID;

                uint32 currentTalentRank = 0;
                for (uint8 rank = 0; rank < MAX_TALENT_RANK; ++rank)
                {
                    if (talentInfo->RankID[rank] && bot->HasTalent(talentInfo->RankID[rank], bot->GetActiveSpec()))
                    {
                        currentTalentRank = rank + 1;
                        break;
                    }
                }
                learnLevel = std::min(lvl, bot->GetFreeTalentPoints() + currentTalentRank) - 1;
            }
            bot->LearnTalent(talentID, learnLevel);
            if (bot->GetFreeTalentPoints() == 0)
            {
                break;
            }
        }
        if (bot->GetFreeTalentPoints() == 0)
        {
            break;
        }
    }
}

ObjectGuid PlayerbotFactory::GetRandomBot()
{
    GuidVector guids;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin();
         i != sPlayerbotAIConfig->randomBotAccounts.end(); i++)
    {
        uint32 accountId = *i;
        if (!AccountMgr::GetCharactersCount(accountId))
            continue;

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID);
        stmt->SetData(0, accountId);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (!result)
            continue;

        do
        {
            Field* fields = result->Fetch();
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[0].Get<uint32>());
            if (!ObjectAccessor::FindPlayer(guid))
                guids.push_back(guid);
        } while (result->NextRow());
    }

    if (guids.empty())
        return ObjectGuid::Empty;

    uint32 index = urand(0, guids.size() - 1);
    return guids[index];
}

void PlayerbotFactory::AddPrevQuests(uint32 questId, std::list<uint32>& questIds)
{
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    for (Quest::PrevQuests::const_iterator iter = quest->prevQuests.begin(); iter != quest->prevQuests.end(); ++iter)
    {
        uint32 prevId = abs(*iter);
        AddPrevQuests(prevId, questIds);
        questIds.remove(prevId);
        questIds.push_back(prevId);
    }
}

void PlayerbotFactory::InitQuests(std::list<uint32>& questMap, bool withRewardItem)
{
    uint32 count = 0;
    for (std::list<uint32>::iterator i = questMap.begin(); i != questMap.end(); ++i)
    {
        uint32 questId = *i;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);

        if (!bot->SatisfyQuestClass(quest, false) || quest->GetMinLevel() > bot->GetLevel() ||
            !bot->SatisfyQuestRace(quest, false))
            continue;

        bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
        // set reward to 5 to skip majority quest reward
        uint32 reward = withRewardItem ? 0 : 5;
        bot->RewardQuest(quest, reward, bot, false);

        if (!withRewardItem)
        {
            // destroy the quest reward item
            if (uint32 itemId = quest->RewardChoiceItemId[reward])
            {
                bot->DestroyItemCount(itemId, quest->RewardChoiceItemCount[reward], true);
            }

            if (quest->GetRewItemsCount())
            {
                for (uint32 i = 0; i < quest->GetRewItemsCount(); ++i)
                {
                    if (uint32 itemId = quest->RewardItemId[i])
                    {
                        bot->DestroyItemCount(itemId, quest->RewardItemIdCount[i], true);
                    }
                }
            }
        }
    }
}

void PlayerbotFactory::InitInstanceQuests()
{
    // Yunfan: use configuration instead of hard code
    uint32 currentXP = bot->GetUInt32Value(PLAYER_XP);
    // LOG_INFO("playerbots", "Initializing quests...");
    InitQuests(classQuestIds, false);
    InitQuests(specialQuestIds, false);

    // quest rewards boost bot level, so reduce back
    bot->GiveLevel(level);

    ClearInventory();
    bot->SetUInt32Value(PLAYER_XP, currentXP);
}

void PlayerbotFactory::ClearInventory()
{
    DestroyItemsVisitor visitor(bot);
    IterateItems(&visitor);
}

void PlayerbotFactory::ClearAllItems()
{
    DestroyItemsVisitor visitor(bot);
    IterateItems(&visitor, ITERATE_ALL_ITEMS);
}

void PlayerbotFactory::InitAmmo()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE && bot->getClass() != CLASS_WARRIOR)
        return;

    Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!pItem)
        return;

    uint32 subClass = 0;
    switch (pItem->GetTemplate()->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_GUN:
            subClass = ITEM_SUBCLASS_BULLET;
            break;
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            subClass = ITEM_SUBCLASS_ARROW;
            break;
        default:
            break;
    }

    if (!subClass)
        return;

    std::vector<uint32> ammoEntryList = sRandomItemMgr->GetAmmo(level, subClass);
    uint32 entry = 0;
    for (uint32 tEntry : ammoEntryList)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(tEntry);
        if (!proto)
            continue;

        // disable next expansion ammo
        if (sPlayerbotAIConfig->limitGearExpansion && bot->GetLevel() <= 60 && tEntry >= 23728)
            continue;

        if (sPlayerbotAIConfig->limitGearExpansion && bot->GetLevel() <= 70 && tEntry >= 35570)
            continue;

        entry = tEntry;
        break;
    }

    if (!entry)
        return;

    uint32 count = bot->GetItemCount(entry);
    uint32 maxCount = bot->getClass() == CLASS_HUNTER ? 6000 : 1000;

    if (count < maxCount)
    {
        if (Item* newItem = StoreNewItemInInventorySlot(bot, entry, maxCount - count))
        {
            newItem->AddToUpdateQueueOf(bot);
        }
    }
    bot->SetAmmo(entry);
}

uint32 PlayerbotFactory::CalcMixedGearScore(uint32 gs, uint32 quality)
{
    return gs * PlayerbotAI::GetItemScoreMultiplier(ItemQualities(quality));
}

void PlayerbotFactory::InitMounts()
{
    uint32 firstmount = sPlayerbotAIConfig->useGroundMountAtMinLevel;
    uint32 secondmount = sPlayerbotAIConfig->useFastGroundMountAtMinLevel;
    uint32 thirdmount = sPlayerbotAIConfig->useFlyMountAtMinLevel;
    uint32 fourthmount = sPlayerbotAIConfig->useFastFlyMountAtMinLevel;

    if (bot->GetLevel() < firstmount)
        return;

    std::map<uint8, std::map<uint32, std::vector<uint32>>> mounts;
    std::vector<uint32> slow, fast, fslow, ffast;

    switch (bot->getRace())
    {
        case RACE_HUMAN:
            slow = {470, 6648, 458, 472};
            fast = {23228, 23227, 23229};
            break;
        case RACE_ORC:
            slow = {6654, 6653, 580};
            fast = {23250, 23252, 23251};
            break;
        case RACE_DWARF:
            slow = {6899, 6777, 6898};
            fast = {23238, 23239, 23240};
            break;
        case RACE_NIGHTELF:
            slow = {10789, 8394, 10793};
            fast = {23219, 23220, 63637};
            break;
        case RACE_UNDEAD_PLAYER:
            slow = {17463, 17464, 17462};
            fast = {17465, 23246, 66846};
            break;
        case RACE_TAUREN:
            slow = {18990, 18989, 64657};
            fast = {23249, 23248, 23247};
            break;
        case RACE_GNOME:
            slow = {10969, 17453, 10873, 17454};
            fast = {23225, 23223, 23222};
            break;
        case RACE_TROLL:
            slow = {10796, 10799, 8395};
            fast = {23241, 23242, 23243};
            break;
        case RACE_DRAENEI:
            slow = {34406, 35711, 35710};
            fast = {35713, 35712, 35714};
            break;
        case RACE_BLOODELF:
            slow = {33660, 35020, 35022, 35018};
            fast = {35025, 35025, 35027};
            break;
    }

    switch (bot->GetTeamId())
    {
        case TEAM_ALLIANCE:
            fslow = {32235, 32239, 32240};
            ffast = {32242, 32289, 32290, 32292};
            break;
        case TEAM_HORDE:
            fslow = {32244, 32245, 32243};
            ffast = {32295, 32297, 32246, 32296};
            break;
        default:
            break;
    }

    mounts[bot->getRace()][0] = slow;
    mounts[bot->getRace()][1] = fast;
    mounts[bot->getRace()][2] = fslow;
    mounts[bot->getRace()][3] = ffast;

    for (uint32 type = 0; type < 4; type++)
    {
        bool hasMount = false;
        for (uint32& spell : mounts[bot->getRace()][type])
        {
            if (bot->HasSpell(spell))
            {
                hasMount = true;
                break;
            }
        }
        if (hasMount)
            continue;

        if (bot->GetLevel() < secondmount && type == 1)
            continue;

        if (bot->GetLevel() < thirdmount && type == 2)
            continue;

        if (bot->GetLevel() < fourthmount && type == 3)
            continue;

        uint32 index = urand(0, mounts[bot->getRace()][type].size() - 1);
        uint32 spell = mounts[bot->getRace()][type][index];
        if (spell)
        {
            bot->learnSpell(spell);
            LOG_DEBUG("playerbots", "Bot {} ({}) learned {} mount {}", bot->GetGUID().ToString().c_str(),
                      bot->GetLevel(), type == 0 ? "slow" : (type == 1 ? "fast" : "flying"), spell);
        }
    }
}

void PlayerbotFactory::UnbindInstance()
{
    Player* p = bot;
    ObjectGuid guid = p->GetGUID();

    for (uint8 d = 0; d < MAX_DIFFICULTY; ++d)
    {
        std::vector<InstanceSave*> toUnbind;
        BoundInstancesMap const& m_boundInstances = sInstanceSaveMgr->PlayerGetBoundInstances(guid, Difficulty(d));
        for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end(); ++itr)
        {
            InstanceSave* instanceSave = itr->second.save;
            toUnbind.push_back(instanceSave);
        }
        for (std::vector<InstanceSave*>::const_iterator itr = toUnbind.begin(); itr != toUnbind.end(); ++itr)
            sInstanceSaveMgr->PlayerUnbindInstance(guid, (*itr)->GetMapId(), (*itr)->GetDifficulty(), true, p);
    }
}

void PlayerbotFactory::InitPotions()
{
    uint32 effects[] = {SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE};
    for (uint8 i = 0; i < 2; ++i)
    {
        uint32 effect = effects[i];

        if (effect == SPELL_EFFECT_ENERGIZE && !bot->GetPower(POWER_MANA))
            continue;

        FindPotionVisitor visitor(bot, effect);
        IterateItems(&visitor);
        if (!visitor.GetResult().empty())
            continue;

        uint32 itemId = sRandomItemMgr->GetRandomPotion(level, effect);
        if (!itemId)
        {
            // LOG_INFO("playerbots", "No potions (type {}) available for bot {} ({} level)", effect,
            // bot->GetName().c_str(), bot->GetLevel());
            continue;
        }

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        uint32 maxCount = proto->GetMaxStackSize();
        if (Item* newItem = StoreNewItemInInventorySlot(bot, itemId, urand(maxCount / 2, maxCount)))
            newItem->AddToUpdateQueueOf(bot);
    }
}

std::vector<uint32> PlayerbotFactory::GetCurrentGemsCount()
{
    std::vector<uint32> curcount(4);
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* pItem2 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem2 && !pItem2->IsBroken() && pItem2->HasSocket())
        {
            for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot <= PRISMATIC_ENCHANTMENT_SLOT;
                 ++enchant_slot)
            {
                if (enchant_slot == BONUS_ENCHANTMENT_SLOT)
                    continue;

                uint32 enchant_id = pItem2->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if (!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchantEntry)
                    continue;

                uint32 gemid = enchantEntry->GemID;
                if (!gemid)
                    continue;

                ItemTemplate const* gemProto = sObjectMgr->GetItemTemplate(gemid);
                if (!gemProto)
                    continue;

                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                if (!gemProperty)
                    continue;

                uint8 GemColor = gemProperty->color;

                for (uint8 b = 0, tmpcolormask = 1; b < 4; b++, tmpcolormask <<= 1)
                {
                    if (tmpcolormask & GemColor)
                        ++curcount[b];
                }
            }
        }
    }
    return curcount;
}

void PlayerbotFactory::InitFood()
{
    if (botAI && botAI->HasCheat(BotCheatMask::food))
    {
        return;
    }
    std::unordered_map<uint32, std::vector<uint32>> items;
    ItemTemplateContainer const* itemTemplateContainer = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator i = itemTemplateContainer->begin(); i != itemTemplateContainer->end();
         ++i)
    {
        uint32 itemId = i->first;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        if (proto->Class != ITEM_CLASS_CONSUMABLE || proto->SubClass != ITEM_SUBCLASS_FOOD ||
            (proto->Spells[0].SpellCategory != 11 && proto->Spells[0].SpellCategory != 59) || proto->Bonding != NO_BIND)
            continue;

        if (proto->RequiredLevel > bot->GetLevel() || proto->RequiredLevel < bot->GetLevel() - 9)
            continue;

        if (proto->RequiredSkill && !bot->HasSkill(proto->RequiredSkill))
            continue;

        if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
            continue;

        items[proto->Spells[0].SpellCategory].push_back(itemId);
    }

    uint32 categories[] = {11, 59};
    for (int i = 0; i < sizeof(categories) / sizeof(uint32); ++i)
    {
        uint32 category = categories[i];
        std::vector<uint32>& ids = items[category];
        int tries = 0;
        for (int j = 0; j < 2; j++)
        {
            uint32 index = urand(0, ids.size() - 1);
            if (index >= ids.size())
                continue;

            uint32 itemId = ids[index];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            // beer / wine ...
            if (proto->Spells[0].SpellId == 11007 || proto->Spells[0].SpellId == 11008 ||
                proto->Spells[0].SpellId == 11009 || proto->Spells[0].SpellId == 11629 ||
                proto->Spells[0].SpellId == 50986)
            {
                tries++;
                if (tries > 5)
                {
                    continue;
                }
                j--;
                continue;
            }
            StoreItem(itemId, proto->GetMaxStackSize());
        }
    }
}

void PlayerbotFactory::InitReagents()
{
    int specTab = AiFactory::GetPlayerSpecTab(bot);
    std::vector<std::pair<uint32, uint32>> items;
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        if (level >= 56)
                items.push_back({37201, 40});   // Corpse Dust
            break;
        case CLASS_DRUID:
            if (level >= 20 && level < 30)
                items.push_back({17034, 20});   // Maple Seed
            else if (level >= 30 && level < 40)
                items.push_back({17035, 20});   // Stranglethorn Seed
            else if (level >= 40 && level < 50)
                items.push_back({17036, 20});   // Ashwood Seed
            else if (level >= 50 && level < 60)
            {
                items.push_back({17037, 20});   // Hornbeam Seed
                items.push_back({17021, 20});   // Wild Berries
            }
            else if (level >= 60 && level < 69)
            {
                items.push_back({17038, 20});   // Ironwood Seed
                items.push_back({17026, 20});   // Wild Thornroot
            }
            else if (level == 69)
            {
                items.push_back({22147, 20});   // Flintweed Seed
                items.push_back({17026, 20});   // Wild Thornroot
            }
            else if (level >= 70 && level < 79)
            {
                items.push_back({22147, 20});   // Flintweed Seed
                items.push_back({22148, 20});   // Wild Quillvine
            }
            else if (level == 79)
            {
                items.push_back({44614, 20});   // Starleaf Seed
                items.push_back({22148, 20});   // Wild Quillvine
            }
            else if (level >= 80)
            {
                items.push_back({44614, 20});   // Starleaf Seed
                items.push_back({44605, 20});   // Wild Spineleaf
            }
            break;
        case CLASS_MAGE:
            if (level >= 20)
                items.push_back({17031, 20});  // Rune of Teleportation
            if (level >= 40)
                items.push_back({17032, 20});  // Rune of Portals
            if (level >= 56)
                items.push_back({17020, 20});  // Arcane Powder
            break;
        case CLASS_PALADIN:
            if (level >= 52)
                items.push_back({21177, 80});   // Symbol of Kings
            break;
        case CLASS_PRIEST:
            if (level >= 48 && level < 56)
                items.push_back({17028, 40});  // Holy Candle
            else if (level >= 56 && level < 60)
            {
                items.push_back({17028, 20});  // Holy Candle
                items.push_back({17029, 20});  // Sacred Candle
            }
            else if (level >= 60 && level < 77)
                items.push_back({17029, 40});  // Sacred Candle
            else if (level >= 77 && level < 80)
            {
                items.push_back({17029, 20});  // Sacred Candle
                items.push_back({44615, 20});  // Devout Candle
            }
            else if (level >= 80)
                items.push_back({44615, 40});  // Devout Candle
            break;
        case CLASS_SHAMAN:
            if (level >= 4)
                items.push_back({5175, 1});  // Earth Totem
            if (level >= 10)
                items.push_back({5176, 1});  // Flame Totem
            if (level >= 20)
                items.push_back({5177, 1});  // Water Totem
            if (level >= 30)
            {
                items.push_back({5178, 1});  // Air Totem
                items.push_back({17030, 20});  // Ankh
            }
            break;
        case CLASS_WARLOCK:
            items.push_back({6265, 5});  // Soul Shard
            break;
        default:
            break;
    }
    for (std::pair item : items)
    {
        int count = (int)item.second - (int)bot->GetItemCount(item.first);
        if (count > 0)
            StoreItem(item.first, count);
    }
}

void PlayerbotFactory::CleanupConsumables() // remove old consumables as part of randombot level-up maintenance
{
    std::vector<Item*> itemsToDelete;

    std::vector<Item*> items;
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            items.push_back(item);

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* bag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                if (Item* item = bag->GetItemByPos(j))
                    items.push_back(item);

    for (Item* item : items)
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (!proto) continue;

        // Remove ammo
        if (proto->Class == ITEM_CLASS_PROJECTILE)
            itemsToDelete.push_back(item);

        // Remove food/drink
        if (proto->Class == ITEM_CLASS_CONSUMABLE && proto->SubClass == ITEM_SUBCLASS_FOOD)
            itemsToDelete.push_back(item);

        // Remove potions
        if (proto->Class == ITEM_CLASS_CONSUMABLE && proto->SubClass == ITEM_SUBCLASS_POTION)
            itemsToDelete.push_back(item);

        // Remove reagents
        if (proto->Class == ITEM_CLASS_REAGENT || (proto->Class == ITEM_CLASS_MISC && proto->SubClass == ITEM_SUBCLASS_REAGENT))
            itemsToDelete.push_back(item);
    }

    std::set<uint32> idsToDelete = {
        BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL,
        BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL,
        ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE,
        HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE,
        ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
        HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE,
        INSTANT_POISON_IX, INSTANT_POISON_VIII, INSTANT_POISON_VII, INSTANT_POISON_VI, INSTANT_POISON_V,
        INSTANT_POISON_IV, INSTANT_POISON_III, INSTANT_POISON_II, INSTANT_POISON,
        DEADLY_POISON_IX, DEADLY_POISON_VIII, DEADLY_POISON_VII, DEADLY_POISON_VI, DEADLY_POISON_V,
        DEADLY_POISON_IV, DEADLY_POISON_III, DEADLY_POISON_II, DEADLY_POISON
    };

    for (Item* item : items)
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (!proto) continue;

        if (idsToDelete.find(proto->ItemId) != idsToDelete.end())
            itemsToDelete.push_back(item);
    }

    for (Item* item : itemsToDelete)
        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
}

void PlayerbotFactory::InitGlyphs(bool increment)
{
    bot->InitGlyphsForLevel();
    if (!increment && botAI &&
        botAI->GetAiObjectContext()->GetValue<bool>("custom_glyphs")->Get())
        return;   // // Added for custom Glyphs - custom glyphs flag test

    if (!increment)
    {
        for (uint32 slotIndex = 0; slotIndex < MAX_GLYPH_SLOT_INDEX; ++slotIndex)
        {
            uint32 glyph = bot->GetGlyph(slotIndex);
            if (GlyphPropertiesEntry const* glyphEntry = sGlyphPropertiesStore.LookupEntry(glyph))
            {
                bot->RemoveAurasDueToSpell(glyphEntry->SpellId);

                // Removed any triggered auras
                Unit::AuraMap& ownedAuras = bot->GetOwnedAuras();
                for (Unit::AuraMap::iterator iter = ownedAuras.begin(); iter != ownedAuras.end();)
                {
                    Aura* aura = iter->second;
                    if (SpellInfo const* triggeredByAuraSpellInfo = aura->GetTriggeredByAuraSpellInfo())
                    {
                        if (triggeredByAuraSpellInfo->Id == glyphEntry->SpellId)
                        {
                            bot->RemoveOwnedAura(iter);
                            continue;
                        }
                    }
                    ++iter;
                }

                bot->SetGlyph(slotIndex, 0, true);
            }
        }
    }

    if (sPlayerbotAIConfig->limitTalentsExpansion && bot->GetLevel() <= 70)
    {
        bot->SendTalentsInfoData(false);
        return;
    }

    uint32 level = bot->GetLevel();
    uint32 maxSlot = 0;
    if (level >= 15)
        maxSlot = 2;
    if (level >= 30)
        maxSlot = 3;
    if (level >= 50)
        maxSlot = 4;
    if (level >= 70)
        maxSlot = 5;
    if (level >= 80)
        maxSlot = 6;

    uint8 glyphOrder[6] = {0, 1, 3, 2, 4, 5};

    if (!maxSlot)
        return;

    uint8 cls = bot->getClass();
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    /// @todo: fix cat druid hardcode

    // Warrior PVP exceptions
    if (bot->getClass() == CLASS_WARRIOR)
    {
        // Arms PvP (spec index 3): If the bot has the Second Wind talent
        if (bot->HasAura(29838))
            tab = 3;
        // Fury PvP (spec index 4): If the bot has the Blood Craze talent
        else if (bot->HasAura(16492))
            tab = 4;
        // Protection PvP (spec index 5): If the bot has the Gag Order talent
        else if (bot->HasAura(12958))
            tab = 5;
    }

    // Paladin PvP exceptions
    if (bot->getClass() == CLASS_PALADIN)
    {
        // Holy PvP (spec index 3): If the bot has the Sacred Cleansing talent
        if (bot->HasAura(53553))
            tab = 3;
        // Protection PvP (spec index 4): If the bot has the Reckoning talent
        else if (bot->HasAura(20179))
            tab = 4;
        // Retribution PvP (spec index 5): If the bot has the Divine Purpose talent
        else if (bot->HasAura(31872))
            tab = 5;
    }

    // Hunter PvP exceptions
    if (bot->getClass() == CLASS_HUNTER)
    {
        // Beast Mastery PvP (spec index 3): If the bot has the Thick Hide talent
        if (bot->HasAura(19612))
            tab = 3;
        // Marksmanship PvP (spec index 4): If the bot has the Concussive Barrage talent
        else if (bot->HasAura(35102))
            tab = 4;
        // Survival PvP (spec index 5): If the bot has the Entrapment talent and does NOT have the Concussive Barrage talent
        else if (bot->HasAura(19388) && !bot->HasAura(35102))
            tab = 5;
    }

    // Rogue PvP exceptions
    if (bot->getClass() == CLASS_ROGUE)
    {
        // Assassination PvP (spec index 3): If the bot has the Deadly Brew talent
        if (bot->HasAura(51626))
            tab = 3;
        // Combat PvP (spec index 4): If the bot has the Throwing Specialization talent
        else if (bot->HasAura(51679))
            tab = 4;
        // Subtlety PvP (spec index 5): If the bot has the Waylay talent
        else if (bot->HasAura(51696))
            tab = 5;
    }

    // Priest PvP exceptions
    if (bot->getClass() == CLASS_PRIEST)
    {
        // Discipline PvP (spec index 3): If the bot has the Improved Mana Burn talent
        if (bot->HasAura(14772))
            tab = 3;
        // Holy PvP (spec index 4): If the bot has the Body and Soul talent
        else if (bot->HasAura(64129))
            tab = 4;
        // Shadow PvP (spec index 5): If the bot has the Improved Vampiric Embrace talent
        else if (bot->HasAura(27840))
            tab = 5;
    }

    // Death Knight PvE/PvP exceptions
    if (bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        // Double Aura Blood PvE (spec index 3): If the bot has both the Abomination's Might and Improved Icy Talons
        // talents
        if (bot->HasAura(53138) && bot->HasAura(55610))
            tab = 3;
        // Blood PvP (spec index 4): If the bot has the Sudden Doom talent
        else if (bot->HasAura(49529))
            tab = 4;
        // Frost PvP (spec index 5): If the bot has the Acclimation talent
        else if (bot->HasAura(50152))
            tab = 5;
        // Unholy PvP (spec index 6): If the bot has the Magic Suppression talent
        else if (bot->HasAura(49611))
            tab = 6;
    }

    // Shaman PvP exceptions
    if (bot->getClass() == CLASS_SHAMAN)
    {
        // Elemental PvP (spec index 3): If the bot has the Astral Shift talent
        if (bot->HasAura(51479))
            tab = 3;
        // Enhancement PvP (spec index 4): If the bot has the Earthen Power talent
        else if (bot->HasAura(51524))
            tab = 4;
        // Restoration PvP (spec index 5): If the bot has the Focused Mind talent
        else if (bot->HasAura(30866))
            tab = 5;
    }

    // Mage PvE/PvP exceptions
    if (bot->getClass() == CLASS_MAGE)
    {
        // Frostfire PvE (spec index 3): If the bot has both the Burnout talent and the Ice Shards talent
        if (bot->HasAura(44472) && bot->HasAura(15047))
            tab = 3;
        // Arcane PvP (spec index 4): If the bot has the Improved Blink talent
        else if (bot->HasAura(31570))
            tab = 4;
        // Fire PvP (spec index 5): If the bot has the Fiery Payback talent
        else if (bot->HasAura(64357))
            tab = 5;
        // Frost PvP (spec index 6): If the bot has the Shattered Barrier talent
        else if (bot->HasAura(54787))
            tab = 6;
    }

    // Warlock PvP exceptions
    if (bot->getClass() == CLASS_WARLOCK)
    {
        // Affliction PvP (spec index 3): If the bot has the Improved Howl of Terror talent
        if (bot->HasAura(30057))
            tab = 3;
        // Demonology PvP (spec index 4): If the bot has both the Nemesis talent and the Intensity talent
        else if (bot->HasAura(63123) && bot->HasAura(18136))
            tab = 4;
        // Destruction PvP (spec index 5): If the bot has the Nether Protection talent
        else if (bot->HasAura(30302))
            tab = 5;
    }

    // Druid PvE/PvP exceptions
    if (bot->getClass() == CLASS_DRUID)
    {
        // Cat PvE (spec index 3): If the bot is Feral spec, level 20 or higher, and does NOT have the Thick Hide talent
        if (tab == DRUID_TAB_FERAL && bot->GetLevel() >= 20 && !bot->HasAura(16931))
            tab = 3;
        // Balance PvP (spec index 4): If the bot has the Owlkin Frenzy talent
        else if (bot->HasAura(48393))
            tab = 4;
        // Feral PvP (spec index 5): If the bot has the Primal Tenacity talent
        else if (bot->HasAura(33957))
            tab = 5;
        // Resto PvP (spec index 6): If the bot has the Improved Barkskin talent
        else if (bot->HasAura(63411))
            tab = 6;
    }

    std::list<uint32> glyphs;
    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator i = itemTemplates->begin(); i != itemTemplates->end(); ++i)
    {
        //uint32 itemId = i->first; //not used, line marked for removal.
        ItemTemplate const* proto = &i->second;
        if (!proto)
            continue;

        if (proto->Class != ITEM_CLASS_GLYPH)
            continue;

        if ((proto->AllowableClass & bot->getClassMask()) == 0 || (proto->AllowableRace & bot->getRaceMask()) == 0)
            continue;

        for (uint32 spell = 0; spell < MAX_ITEM_PROTO_SPELLS; spell++)
        {
            uint32 spellId = proto->Spells[spell].SpellId;
            SpellInfo const* entry = sSpellMgr->GetSpellInfo(spellId);
            if (!entry)
                continue;

            for (uint32 effect = 0; effect <= EFFECT_2; ++effect)
            {
                if (entry->Effects[effect].Effect != SPELL_EFFECT_APPLY_GLYPH)
                    continue;

                uint32 glyph = entry->Effects[effect].MiscValue;
                glyphs.push_back(glyph);
            }
        }
    }

    std::unordered_set<uint32> chosen;
    for (uint32 slotIndex = 0; slotIndex < maxSlot; ++slotIndex)
    {
        uint8 realSlot = glyphOrder[slotIndex];
        if (bot->GetGlyph(realSlot))
        {
            continue;
        }
        // uint32 slot = bot->GetGlyphSlot(slotIndex);
        // GlyphSlotEntry const *gs = sGlyphSlotStore.LookupEntry(slot);
        // if (!gs)
        //     continue;
        if (sPlayerbotAIConfig->parsedSpecGlyph[cls][tab].size() > slotIndex &&
            sPlayerbotAIConfig->parsedSpecGlyph[cls][tab][slotIndex] != 0)
        {
            uint32 itemId = sPlayerbotAIConfig->parsedSpecGlyph[cls][tab][slotIndex];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (proto->Class != ITEM_CLASS_GLYPH)
                continue;

            if ((proto->AllowableClass & bot->getClassMask()) == 0 || (proto->AllowableRace & bot->getRaceMask()) == 0)
                continue;

            if (proto->RequiredLevel > bot->GetLevel())
                continue;

            uint32 glyph = 0;
            for (uint32 spell = 0; spell < MAX_ITEM_PROTO_SPELLS; spell++)
            {
                uint32 spellId = proto->Spells[spell].SpellId;
                SpellInfo const* entry = sSpellMgr->GetSpellInfo(spellId);
                if (!entry)
                    continue;

                for (uint32 effect = 0; effect <= EFFECT_2; ++effect)
                {
                    if (entry->Effects[effect].Effect != SPELL_EFFECT_APPLY_GLYPH)
                        continue;

                    glyph = entry->Effects[effect].MiscValue;
                }
            }
            if (!glyph)
            {
                continue;
            }
            GlyphPropertiesEntry const* glyphEntry = sGlyphPropertiesStore.LookupEntry(glyph);
            bot->CastSpell(bot, glyphEntry->SpellId,
                           TriggerCastFlags(TRIGGERED_FULL_MASK &
                                            ~(TRIGGERED_IGNORE_SHAPESHIFT | TRIGGERED_IGNORE_CASTER_AURASTATE)));
            bot->SetGlyph(realSlot, glyph, true);
            chosen.insert(glyph);
        }
        else
        {
            uint32 slot = bot->GetGlyphSlot(realSlot);
            GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(slot);
            if (!gs)
                continue;

            std::vector<uint32> ids;
            for (std::list<uint32>::iterator i = glyphs.begin(); i != glyphs.end(); ++i)
            {
                uint32 id = *i;
                GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(id);
                if (!gp || gp->TypeFlags != gs->TypeFlags)
                    continue;

                ids.push_back(id);
            }

            //int maxCount = urand(0, 3); //not used, line marked for removal.
            //int count = 0; //not used, line marked for removal.
            //bool found = false; //not used, line marked for removal.
            for (int attempts = 0; attempts < 15; ++attempts)
            {
                uint32 index = urand(0, ids.size() - 1);
                if (index >= ids.size())
                    continue;

                uint32 id = ids[index];
                if (chosen.find(id) != chosen.end())
                    continue;

                chosen.insert(id);
                GlyphPropertiesEntry const* glyphEntry = sGlyphPropertiesStore.LookupEntry(id);
                bot->CastSpell(bot, glyphEntry->SpellId,
                               TriggerCastFlags(TRIGGERED_FULL_MASK &
                                                ~(TRIGGERED_IGNORE_SHAPESHIFT | TRIGGERED_IGNORE_CASTER_AURASTATE)));

                bot->SetGlyph(realSlot, id, true);
                //found = true; //not used, line marked for removal.
                break;
            }
        }
    }
    bot->SendTalentsInfoData(false);
}

void PlayerbotFactory::CancelAuras() { bot->RemoveAllAuras(); }

void PlayerbotFactory::InitInventory()
{
    InitInventoryTrade();
    InitInventoryEquip();
    InitInventorySkill();
}

void PlayerbotFactory::InitInventorySkill()
{
    if (bot->HasSkill(SKILL_MINING))
    {
        StoreItem(2901, 1);  // Mining Pick
    }

    if (bot->HasSkill(SKILL_BLACKSMITHING) || bot->HasSkill(SKILL_ENGINEERING))
    {
        StoreItem(5956, 1);  // Blacksmith Hammer
    }

    if (bot->HasSkill(SKILL_ENGINEERING))
    {
        StoreItem(6219, 1);  // Arclight Spanner
    }

    if (bot->HasSkill(SKILL_ENCHANTING))
    {
        StoreItem(16207, 1);  // Runed Arcanite Rod
    }

    if (bot->HasSkill(SKILL_SKINNING))
    {
        StoreItem(7005, 1);  // Skinning Knife
    }
}

Item* PlayerbotFactory::StoreItem(uint32 itemId, uint32 count)
{
    //ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId); //not used, line marked for removal.
    ItemPosCountVec sDest;
    InventoryResult msg = bot->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, itemId, count);
    if (msg != EQUIP_ERR_OK)
        return nullptr;

    return bot->StoreNewItem(sDest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
}

void PlayerbotFactory::InitInventoryTrade()
{
    uint32 itemId = sRandomItemMgr->GetRandomTrade(level);
    if (!itemId)
    {
        LOG_ERROR("playerbots", "No trade items available for bot {} ({} level)", bot->GetName().c_str(),
                  bot->GetLevel());
        return;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return;

    uint32 count = 1, stacks = 1;
    switch (proto->Quality)
    {
        case ITEM_QUALITY_NORMAL:
            count = proto->GetMaxStackSize();
            stacks = urand(1, 3);
            break;
        case ITEM_QUALITY_UNCOMMON:
            stacks = 1;
            int maxStackSize = proto->GetMaxStackSize() / 2;
            uint32 max = std::max(1, maxStackSize);
            count = urand(1, max);
            break;
    }

    for (uint32 i = 0; i < stacks; i++)
        StoreItem(itemId, count);
}

void PlayerbotFactory::InitInventoryEquip()
{
    std::vector<uint32> ids;

    uint32 desiredQuality = itemQuality;
    if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL)
    {
        desiredQuality--;
    }

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (auto const& itr : *itemTemplates)
    {
        ItemTemplate const* proto = &itr.second;
        if (!proto)
            continue;

        if ((proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON) ||
            (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_WHEN_USE))
            continue;

        if (proto->Class == ITEM_CLASS_ARMOR && !CanEquipArmor(proto))
            continue;

        if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(proto))
            continue;

        if (proto->Quality != desiredQuality)
            continue;

        if (!CanEquipItem(proto))
            continue;

        ids.push_back(itr.first);
    }

    uint32 maxCount = urand(0, 3);
    uint32 count = 0;
    for (uint32 attempts = 0; attempts < 15; attempts++)
    {
        uint32 index = urand(0, ids.size() - 1);
        if (index >= ids.size())
            continue;

        uint32 itemId = ids[index];
        if (StoreItem(itemId, 1) && count++ >= maxCount)
            break;
    }
}

void PlayerbotFactory::InitGuild()
{
    if (bot->GetGuildId())
        return;

    // bot->SaveToDB(false, false);

    // add guild tabard
    if (bot->GetGuildId() && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);

    if (sPlayerbotAIConfig->randomBotGuilds.empty())
        RandomPlayerbotFactory::CreateRandomGuilds();

    std::vector<uint32> guilds;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotGuilds.begin();
         i != sPlayerbotAIConfig->randomBotGuilds.end(); ++i)
        guilds.push_back(*i);

    if (guilds.empty())
    {
        LOG_ERROR("playerbots", "No random guilds available");
        return;
    }

    int index = urand(0, guilds.size() - 1);
    uint32 guildId = guilds[index];
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
    {
        LOG_ERROR("playerbots", "Invalid guild {}", guildId);
        return;
    }

    if (guild->GetMemberSize() < urand(10, sPlayerbotAIConfig->randomBotGuildSizeMax))
        guild->AddMember(bot->GetGUID(), urand(GR_OFFICER, GR_INITIATE));

    // add guild tabard
    if (bot->GetGuildId() && bot->GetLevel() > 9 && urand(0, 4) && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);

    // bot->SaveToDB(false, false);
}

void PlayerbotFactory::InitImmersive()
{
    uint32 owner = bot->GetGUID().GetCounter();
    std::map<Stats, uint32> percentMap;

    bool initialized = false;
    for (uint32 i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        Stats type = (Stats)i;

        std::ostringstream name;
        name << "immersive_stat_" << i;

        uint32 value = sRandomPlayerbotMgr->GetValue(owner, name.str());
        if (value)
            initialized = true;

        percentMap[type] = value;
    }

    if (!initialized)
    {
        switch (bot->getClass())
        {
            case CLASS_DRUID:
            case CLASS_SHAMAN:
                percentMap[STAT_STRENGTH] = 10;
                percentMap[STAT_INTELLECT] = 10;
                percentMap[STAT_SPIRIT] = 20;
                percentMap[STAT_AGILITY] = 30;
                percentMap[STAT_STAMINA] = 30;
                break;
            case CLASS_PALADIN:
                percentMap[STAT_STRENGTH] = 10;
                percentMap[STAT_INTELLECT] = 10;
                percentMap[STAT_SPIRIT] = 20;
                percentMap[STAT_AGILITY] = 50;
                percentMap[STAT_STAMINA] = 10;
                break;
            case CLASS_WARRIOR:
                percentMap[STAT_STRENGTH] = 10;
                percentMap[STAT_SPIRIT] = 20;
                percentMap[STAT_AGILITY] = 50;
                percentMap[STAT_STAMINA] = 20;
                break;
            case CLASS_ROGUE:
            case CLASS_HUNTER:
                percentMap[STAT_SPIRIT] = 40;
                percentMap[STAT_AGILITY] = 50;
                percentMap[STAT_STAMINA] = 10;
                break;
            case CLASS_MAGE:
                percentMap[STAT_INTELLECT] = 50;
                percentMap[STAT_SPIRIT] = 40;
                percentMap[STAT_STAMINA] = 10;
                break;
            case CLASS_PRIEST:
                percentMap[STAT_INTELLECT] = 50;
                percentMap[STAT_SPIRIT] = 40;
                percentMap[STAT_STAMINA] = 10;
                break;
            case CLASS_WARLOCK:
                percentMap[STAT_INTELLECT] = 50;
                percentMap[STAT_SPIRIT] = 10;
                percentMap[STAT_STAMINA] = 40;
                break;
        }

        for (uint8 i = 0; i < 5; i++)
        {
            Stats from = (Stats)urand(STAT_STRENGTH, MAX_STATS - 1);
            Stats to = (Stats)urand(STAT_STRENGTH, MAX_STATS - 1);
            int32 delta = urand(0, 5 + bot->GetLevel() / 3);
            if (from != to && percentMap[to] + delta <= 100 && percentMap[from] - delta >= 0)
            {
                percentMap[to] += delta;
                percentMap[from] -= delta;
            }
        }

        for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        {
            Stats type = (Stats)i;

            std::ostringstream name;
            name << "immersive_stat_" << i;
            sRandomPlayerbotMgr->SetValue(owner, name.str(), percentMap[type]);
        }
    }
}

void PlayerbotFactory::InitArenaTeam()
{
    if (!sPlayerbotAIConfig->IsInRandomAccountList(bot->GetSession()->GetAccountId()))
        return;

    // Currently the teams are only remade after a server restart and if deleteRandomBotArenaTeams = 1
    // This is because randomBotArenaTeams is only empty on server restart.
    // A manual reinitalization (.playerbots rndbot init) is also required after the teams have been deleted.
    if (sPlayerbotAIConfig->randomBotArenaTeams.empty())
    {
        if (sPlayerbotAIConfig->deleteRandomBotArenaTeams)
        {
            LOG_INFO("playerbots", "Deleting random bot arena teams...");

            for (auto it = sArenaTeamMgr->GetArenaTeams().begin(); it != sArenaTeamMgr->GetArenaTeams().end(); ++it)
            {
                ArenaTeam* arenateam = it->second;
                if (arenateam->GetCaptain() && arenateam->GetCaptain().IsPlayer())
                {
                    Player* bot = ObjectAccessor::FindPlayer(arenateam->GetCaptain());
                    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
                    if (!botAI || botAI->IsRealPlayer())
                    {
                        continue;
                    }
                    else
                    {
                        arenateam->Disband(nullptr);
                    }
                }
            }

            LOG_INFO("playerbots", "Random bot arena teams deleted");
        }

        RandomPlayerbotFactory::CreateRandomArenaTeams(ARENA_TYPE_2v2, sPlayerbotAIConfig->randomBotArenaTeam2v2Count);
        RandomPlayerbotFactory::CreateRandomArenaTeams(ARENA_TYPE_3v3, sPlayerbotAIConfig->randomBotArenaTeam3v3Count);
        RandomPlayerbotFactory::CreateRandomArenaTeams(ARENA_TYPE_5v5, sPlayerbotAIConfig->randomBotArenaTeam5v5Count);
    }

    std::vector<uint32> arenateams;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotArenaTeams.begin();
         i != sPlayerbotAIConfig->randomBotArenaTeams.end(); ++i)
        arenateams.push_back(*i);

    if (arenateams.empty())
    {
        LOG_ERROR("playerbots", "No random arena team available");
        return;
    }

    while (!arenateams.empty())
    {
        int index = urand(0, arenateams.size() - 1);
        uint32 arenateamID = arenateams[index];
        ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamById(arenateamID);
        if (!arenateam)
        {
            LOG_ERROR("playerbots", "Invalid arena team {}", arenateamID);
            arenateams.erase(arenateams.begin() + index);
            continue;
        }

        if (arenateam->GetMembersSize() < ((uint32)arenateam->GetType()) && bot->GetLevel() >= 70)
        {
            ObjectGuid capt = arenateam->GetCaptain();
            Player* botcaptain = ObjectAccessor::FindPlayer(capt);

            // To avoid bots removing each other from groups when queueing, force them to only be in one team
            for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
            {
                uint32 arenaTeamId = bot->GetArenaTeamId(arena_slot);
                if (!arenaTeamId)
                    continue;

                ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(arenaTeamId);
                if (team)
                {
                    if (sCharacterCache->GetCharacterArenaTeamIdByGuid(bot->GetGUID(), team->GetSlot()) != 0)
                    {
                        return;
                    }
                    return;
                }
            }

            if (botcaptain && botcaptain->GetTeamId() == bot->GetTeamId())  // need?
            {
                arenateam->AddMember(bot->GetGUID());
                arenateam->SaveToDB();
            }
        }
        arenateams.erase(arenateams.begin() + index);
    }

    // bot->SaveToDB(false, false);
}

void PlayerbotFactory::ApplyEnchantTemplate()
{
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);

    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            if (tab == 2)
                ApplyEnchantTemplate(12);
            else if (tab == 1)
                ApplyEnchantTemplate(11);
            else
                ApplyEnchantTemplate(10);
            break;
        case CLASS_DRUID:
            if (tab == 2)
                ApplyEnchantTemplate(112);
            else if (tab == 0)
                ApplyEnchantTemplate(110);
            else
                ApplyEnchantTemplate(111);
            break;
        case CLASS_SHAMAN:
            if (tab == 0)
                ApplyEnchantTemplate(70);
            else if (tab == 2)
                ApplyEnchantTemplate(71);
            else
                ApplyEnchantTemplate(72);
            break;
        case CLASS_PALADIN:
            if (tab == 0)
                ApplyEnchantTemplate(20);
            else if (tab == 2)
                ApplyEnchantTemplate(22);
            else if (tab == 1)
                ApplyEnchantTemplate(21);
            break;
        case CLASS_HUNTER:
            ApplyEnchantTemplate(30);
            break;
        case CLASS_ROGUE:
            ApplyEnchantTemplate(40);
            break;
        case CLASS_MAGE:
            ApplyEnchantTemplate(80);
            break;
        case CLASS_WARLOCK:
            ApplyEnchantTemplate(90);
            break;
        case CLASS_PRIEST:
            ApplyEnchantTemplate(50);
            break;
    }
}

void PlayerbotFactory::ApplyEnchantTemplate(uint8 spec)
{
    for (EnchantContainer::const_iterator itr = GetEnchantContainerBegin(); itr != GetEnchantContainerEnd(); ++itr)
        if ((*itr).ClassId == bot->getClass() && (*itr).SpecId == spec)
        {
            uint32 spellid = (*itr).SpellId;
            uint32 slot = (*itr).SlotId;
            Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!pItem || !pItem->IsInWorld() || !pItem->GetOwner() || !pItem->GetOwner()->IsInWorld() ||
                !pItem->GetOwner()->GetSession())
                return;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if (!spellInfo)
                return;

            uint32 enchantid = spellInfo->Effects[0].MiscValue;
            if (!enchantid)
            {
                // LOG_ERROR("playerbots", "{}: Invalid enchantid ", enchantid, " report to devs",
                // bot->GetName().c_str());
                return;
            }

            if (!((1 << pItem->GetTemplate()->SubClass) & spellInfo->EquippedItemSubClassMask) &&
                !((1 << pItem->GetTemplate()->InventoryType) & spellInfo->EquippedItemInventoryTypeMask))
            {
                // LOG_ERROR("playerbots", "{}: items could not be enchanted, wrong item type equipped",
                // bot->GetName().c_str());
                return;
            }

            bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, false);
            pItem->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);
            bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, true);
        }
    // botAI->EnchantItemT((*itr).SpellId, (*itr).SlotId);
    // const SpellItemEnchantmentEntry* a = sSpellItemEnchantmentStore.LookupEntry(1);
}

void PlayerbotFactory::ApplyEnchantAndGemsNew(bool destoryOld)
{
    //int32 bestGemEnchantId[4] = {-1, -1, -1, -1};  // 1, 2, 4, 8 color //not used, line marked for removal.
    //float bestGemScore[4] = {0, 0, 0, 0}; //not used, line marked for removal.
    std::vector<uint32> curCount = GetCurrentGemsCount();
    uint8 jewelersCount = 0;
    int requiredActive = 2;
    std::vector<uint32> availableGems;
    for (const uint32& enchantGem : enchantGemIdCache)
    {
        ItemTemplate const* gemTemplate = sObjectMgr->GetItemTemplate(enchantGem);
        if (!gemTemplate)
            continue;

        const GemPropertiesEntry* gemProperties = sGemPropertiesStore.LookupEntry(gemTemplate->GemProperties);
        if (!gemProperties)
            continue;

        if (sPlayerbotAIConfig->limitEnchantExpansion && bot->GetLevel() <= 70 && enchantGem >= 39900)
            continue;

        uint32 requiredLevel = gemTemplate->ItemLevel;

        if (requiredLevel > bot->GetLevel())
        {
            continue;
        }

        uint32 enchant_id = gemProperties->spellitemenchantement;
        if (!enchant_id)
            continue;

        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!enchant || (enchant->slot != PERM_ENCHANTMENT_SLOT && enchant->slot != TEMP_ENCHANTMENT_SLOT))
        {
            continue;
        }
        if (enchant->requiredSkill && bot->GetSkillValue(enchant->requiredSkill) < enchant->requiredSkillValue)
        {
            continue;
        }

        if (enchant->requiredLevel > bot->GetLevel())
        {
            continue;
        }
        availableGems.push_back(enchantGem);
    }
    StatsWeightCalculator calculator(bot);
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item || !item->GetOwner())
        {
            continue;
        }

        if (item->GetTemplate() && item->GetTemplate()->Quality < ITEM_QUALITY_UNCOMMON)
            continue;
        int32 bestEnchantId = -1;
        float bestScore = 0;
        for (const uint32& enchantSpell : enchantSpellIdCache)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(enchantSpell);
            if (!spellInfo)
                continue;

            if (!item->IsFitToSpellRequirements(spellInfo))
            {
                continue;
            }

            uint32 requiredLevel = spellInfo->BaseLevel;
            if (requiredLevel > bot->GetLevel())
            {
                continue;
            }

            // disable next expansion enchantments
            if (sPlayerbotAIConfig->limitEnchantExpansion && bot->GetLevel() <= 60 && enchantSpell >= 27899)
                continue;

            if (sPlayerbotAIConfig->limitEnchantExpansion && bot->GetLevel() <= 70 && enchantSpell >= 44483)
                continue;

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (spellInfo->Effects[j].Effect != SPELL_EFFECT_ENCHANT_ITEM)
                    continue;

                uint32 enchant_id = spellInfo->Effects[j].MiscValue;
                if (!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchant || (enchant->slot != PERM_ENCHANTMENT_SLOT && enchant->slot != TEMP_ENCHANTMENT_SLOT))
                {
                    continue;
                }
                if (enchant->requiredSkill &&
                    (!bot->HasSkill(enchant->requiredSkill) ||
                     (bot->GetSkillValue(enchant->requiredSkill) < enchant->requiredSkillValue)))
                {
                    continue;
                }
                if (enchant->requiredLevel > bot->GetLevel())
                {
                    continue;
                }
                float score = calculator.CalculateEnchant(enchant_id);
                if (score >= bestScore)
                {
                    bestScore = score;
                    bestEnchantId = enchant_id;
                }
            }
        }
        // enchant item
        if (bestEnchantId != -1)
        {
            bot->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, false);
            item->SetEnchantment(PERM_ENCHANTMENT_SLOT, bestEnchantId, 0, 0, bot->GetGUID());
            bot->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
        }
        if (!item->HasSocket())
            continue;

        for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + 3; ++enchant_slot)
        {
            uint8 socketColor = item->GetTemplate()->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
            if (!socketColor)
            {
                continue;
            }
            int32 enchantIdChosen = -1;
            int32 colorChosen;
            bool jewelersGemChosen;
            float bestGemScore = -1;
            for (uint32& enchantGem : availableGems)
            {
                ItemTemplate const* gemTemplate = sObjectMgr->GetItemTemplate(enchantGem);
                if (!gemTemplate)
                    continue;

                // Limit jewelers (JC) epic gems to 3
                bool isJewelersGem = gemTemplate->ItemLimitCategory == 2;
                if (isJewelersGem && jewelersCount >= 3)
                    continue;

                const GemPropertiesEntry* gemProperties = sGemPropertiesStore.LookupEntry(gemTemplate->GemProperties);
                if (!gemProperties)
                    continue;

                if ((socketColor & gemProperties->color) == 0 && gemProperties->color == 1)  // meta socket
                    continue;

                uint32 enchant_id = gemProperties->spellitemenchantement;
                if (!enchant_id)
                    continue;

                //SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id); //not used, line marked for removal.
                StatsWeightCalculator calculator(bot);
                float score = calculator.CalculateEnchant(enchant_id);
                if (curCount[0] != 0)
                {
                    // Ensure meta gem activation
                    for (int i = 1; i < curCount.size(); i++)
                    {
                        if (curCount[i] < requiredActive && (gemProperties->color & (1 << i)))
                        {
                            score *= 2;
                            break;
                        }
                    }
                }
                if (socketColor & gemProperties->color)
                    score *= 1.2;
                if (score > bestGemScore)
                {
                    enchantIdChosen = enchant_id;
                    colorChosen = gemProperties->color;
                    bestGemScore = score;
                    jewelersGemChosen = isJewelersGem;
                }
            }
            if (enchantIdChosen == -1)
                continue;
            bot->ApplyEnchantment(item, EnchantmentSlot(enchant_slot), false);
            item->SetEnchantment(EnchantmentSlot(enchant_slot), enchantIdChosen, 0, 0, bot->GetGUID());
            bot->ApplyEnchantment(item, EnchantmentSlot(enchant_slot), true);
            curCount = GetCurrentGemsCount();
            if (jewelersGemChosen)
                ++jewelersCount;
        }
    }
}

std::vector<InventoryType> PlayerbotFactory::GetPossibleInventoryTypeListBySlot(EquipmentSlots slot)
{
    std::vector<InventoryType> ret;
    switch (slot)
    {
        case EQUIPMENT_SLOT_HEAD:
            ret.push_back(INVTYPE_HEAD);
            break;
        case EQUIPMENT_SLOT_NECK:
            ret.push_back(INVTYPE_NECK);
            break;
        case EQUIPMENT_SLOT_SHOULDERS:
            ret.push_back(INVTYPE_SHOULDERS);
            break;
        case EQUIPMENT_SLOT_BODY:
            ret.push_back(INVTYPE_BODY);
            break;
        case EQUIPMENT_SLOT_CHEST:
            ret.push_back(INVTYPE_CHEST);
            ret.push_back(INVTYPE_ROBE);
            break;
        case EQUIPMENT_SLOT_WAIST:
            ret.push_back(INVTYPE_WAIST);
            break;
        case EQUIPMENT_SLOT_LEGS:
            ret.push_back(INVTYPE_LEGS);
            break;
        case EQUIPMENT_SLOT_FEET:
            ret.push_back(INVTYPE_FEET);
            break;
        case EQUIPMENT_SLOT_WRISTS:
            ret.push_back(INVTYPE_WRISTS);
            break;
        case EQUIPMENT_SLOT_HANDS:
            ret.push_back(INVTYPE_HANDS);
            break;
        case EQUIPMENT_SLOT_FINGER1:
        case EQUIPMENT_SLOT_FINGER2:
            ret.push_back(INVTYPE_FINGER);
            break;
        case EQUIPMENT_SLOT_TRINKET1:
        case EQUIPMENT_SLOT_TRINKET2:
            ret.push_back(INVTYPE_TRINKET);
            break;
        case EQUIPMENT_SLOT_BACK:
            ret.push_back(INVTYPE_CLOAK);
            break;
        case EQUIPMENT_SLOT_MAINHAND:
            ret.push_back(INVTYPE_WEAPON);
            ret.push_back(INVTYPE_2HWEAPON);
            ret.push_back(INVTYPE_WEAPONMAINHAND);
            break;
        case EQUIPMENT_SLOT_OFFHAND:
            ret.push_back(INVTYPE_WEAPON);
            ret.push_back(INVTYPE_2HWEAPON);
            ret.push_back(INVTYPE_WEAPONOFFHAND);
            ret.push_back(INVTYPE_SHIELD);
            ret.push_back(INVTYPE_HOLDABLE);
            break;
        case EQUIPMENT_SLOT_RANGED:
            ret.push_back(INVTYPE_RANGED);
            ret.push_back(INVTYPE_RANGEDRIGHT);
            ret.push_back(INVTYPE_RELIC);
            break;
        default:
            break;
    }
    return ret;
}

void PlayerbotFactory::LoadEnchantContainer()
{
    m_EnchantContainer.clear();

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_ENCHANTS);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();

            EnchantTemplate pEnchant;
            pEnchant.ClassId = fields[0].Get<uint8>();
            pEnchant.SpecId = fields[1].Get<uint8>();
            pEnchant.SpellId = fields[2].Get<uint32>();
            pEnchant.SlotId = fields[3].Get<uint8>();

            m_EnchantContainer.push_back(std::move(pEnchant));
        } while (result->NextRow());
    }
}

void PlayerbotFactory::IterateItems(IterateItemsVisitor* visitor, IterateItemsMask mask)
{
    if (mask & ITERATE_ITEMS_IN_BAGS)
        IterateItemsInBags(visitor);

    if (mask & ITERATE_ITEMS_IN_EQUIP)
        IterateItemsInEquip(visitor);

    if (mask == ITERATE_ITEMS_IN_BANK)
        IterateItemsInBank(visitor);
}

void PlayerbotFactory::IterateItemsInBags(IterateItemsVisitor* visitor)
{
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    for (uint32 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (!visitor->Visit(pItem))
                        return;
}

void PlayerbotFactory::IterateItemsInEquip(IterateItemsVisitor* visitor)
{
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!pItem)
            continue;

        if (!visitor->Visit(pItem))
            return;
    }
}

void PlayerbotFactory::IterateItemsInBank(IterateItemsVisitor* visitor)
{
    for (uint8 slot = BANK_SLOT_ITEM_START; slot < BANK_SLOT_ITEM_END; slot++)
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!pItem)
            continue;

        if (!visitor->Visit(pItem))
            return;
    }

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pBag)
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* pItem = pBag->GetItemByPos(j))
                    {
                        if (!pItem)
                            continue;

                        if (!visitor->Visit(pItem))
                            return;
                    }
                }
            }
        }
    }
}

void PlayerbotFactory::InitKeyring()
{
    if (!bot)
        return;

    if (bot->GetLevel() < 70)
        return;

    ReputationMgr& repMgr = bot->GetReputationMgr(); // Reference, use . instead of ->

    std::vector<std::pair<uint32, uint32>> keysToCheck;

    // Reputation-based Keys (Honored requirement)
    if (repMgr.GetRank(sFactionStore.LookupEntry(1011)) >= REP_HONORED && !bot->HasItemCount(30633, 1))
        keysToCheck.emplace_back(1011, 30633); // Lower City - Auchenai Key
    if (repMgr.GetRank(sFactionStore.LookupEntry(942)) >= REP_HONORED && !bot->HasItemCount(30623, 1))
        keysToCheck.emplace_back(942, 30623); // Cenarion Expedition - Reservoir Key
    if (repMgr.GetRank(sFactionStore.LookupEntry(989)) >= REP_HONORED && !bot->HasItemCount(30635, 1))
        keysToCheck.emplace_back(989, 30635); // Keepers of Time - Key of Time
    if (repMgr.GetRank(sFactionStore.LookupEntry(935)) >= REP_HONORED && !bot->HasItemCount(30634, 1))
        keysToCheck.emplace_back(935, 30634); // The Sha'tar - Warpforged Key

    // Faction-specific Keys (Honored requirement)
    if (bot->GetTeamId() == TEAM_ALLIANCE && repMgr.GetRank(sFactionStore.LookupEntry(946)) >= REP_HONORED && !bot->HasItemCount(30622, 1))
        keysToCheck.emplace_back(946, 30622); // Honor Hold - Flamewrought Key (Alliance)
    if (bot->GetTeamId() == TEAM_HORDE && repMgr.GetRank(sFactionStore.LookupEntry(947)) >= REP_HONORED && !bot->HasItemCount(30637, 1))
        keysToCheck.emplace_back(947, 30637); // Thrallmar - Flamewrought Key (Horde)

    // Keys that do not require Rep or Faction
    // Shattered Halls Key, Shadow Labyrinth Key, Key to the Arcatraz, Master's Key
    std::vector<uint32> nonRepKeys = {28395, 27991, 31084, 24490};
    for (uint32 keyId : nonRepKeys)
    {
        if (!bot->HasItemCount(keyId, 1))
            keysToCheck.emplace_back(0, keyId);
    }

    // Assign keys
    for (auto const& keyPair : keysToCheck)
    {
        uint32 keyId = keyPair.second;
        if (keyId > 0)
        {
            if (Item* newItem = StoreNewItemInInventorySlot(bot,keyId, 1))
            {
                newItem->AddToUpdateQueueOf(bot);
            }
        }
    }
}
void PlayerbotFactory::InitReputation()
{
    if (!bot)
        return;

    if (bot->GetLevel() < 70)
        return; // Only apply for level 70+ bots

    ReputationMgr& repMgr = bot->GetReputationMgr();

    // List of factions that require Honored reputation for heroic keys
    std::vector<uint32> factions = {
        1011, // Lower City
        942,  // Cenarion Expedition
        989,  // Keepers of Time
        935   // The Sha'tar
    };

    // Add faction-specific reputation
    if (bot->GetTeamId() == TEAM_ALLIANCE)
        factions.push_back(946); // Honor Hold (Alliance)
    else if (bot->GetTeamId() == TEAM_HORDE)
        factions.push_back(947); // Thrallmar (Horde)

    // Set reputation to Honored for each required faction
    for (uint32 factionId : factions)
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionId);
        if (!factionEntry)
            continue;

        // Bottom of Honored rank
        int32 honoredRep = ReputationMgr::ReputationRankToStanding(static_cast<ReputationRank>(REP_HONORED - 1)) + 1;

        // Get bot's current reputation with this faction
        int32 currentRep = repMgr.GetReputation(factionEntry);

        // Only set reputation if it's lower than the required Honored value
        if (currentRep < honoredRep)
        {
            repMgr.SetReputation(factionEntry, honoredRep);
        }
    }
}

void PlayerbotFactory::InitAttunementQuests()
{
    uint32 level = bot->GetLevel();
    if (level < 55)
        return; // Only apply for level 55+ bots

    uint32 currentXP = bot->GetUInt32Value(PLAYER_XP);

    // List of attunement quest IDs
    std::list<uint32> attunementQuestsTBC = {
        // Caverns of Time - Part 1
        10279, // To The Master's Lair
        10277, // The Caverns of Time

        // Caverns of Time - Part 2 (Escape from Durnholde Keep)
        10282, // Old Hillsbrad
        10283, // Taretha's Diversion
        10284, // Escape from Durnholde
        10285, // Return to Andormu

        // Caverns of Time - Part 2 (The Black Morass)
        10296, // The Black Morass
        10297, // The Opening of the Dark Portal
        10298, // Hero of the Brood

        // Magister's Terrace Attunement
        11481, // Crisis at the Sunwell
        11482, // Duty Calls
        11488, // Magisters' Terrace
        11490, // The Scryer's Scryer
        11492  // Hard to Kill
    };

    // Complete all level-appropriate attunement quests for the bot
    if (level >= 60)
    {
        std::list<uint32> questsToComplete;

        // Check each quest status before adding to the completion list
        for (uint32 questId : attunementQuestsTBC)
        {
            QuestStatus questStatus = bot->GetQuestStatus(questId);

            if (questStatus == QUEST_STATUS_NONE) // Quest not yet taken/completed
            {
                questsToComplete.push_back(questId);
            }
        }

        // Only complete quests that haven't been finished yet
        if (!questsToComplete.empty())
        {
            InitQuests(questsToComplete, false);
        }
    }

    // Reset XP so bot's level remains unchanged
    bot->GiveLevel(level);
    bot->SetUInt32Value(PLAYER_XP, currentXP);
}
