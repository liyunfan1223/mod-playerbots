/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotFactory.h"
#include "AccountMgr.h"
#include "AiFactory.h"
#include "ArenaTeamMgr.h"
#include "DBCStores.h"
#include "GuildMgr.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "LogCommon.h"
#include "LootMgr.h"
#include "MapMgr.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PerformanceMonitor.h"
#include "PlayerbotDbStore.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "ItemVisitors.h"
#include "InventoryAction.h"
#include "SharedDefines.h"
#include <random>
#include <utility>

#define PLAYER_SKILL_INDEX(x)       (PLAYER_SKILL_INFO_1_1 + ((x)*3))
#define ITEM_SUBCLASS_MASK_SINGLE_HAND (\
    (1 << ITEM_SUBCLASS_WEAPON_AXE) | (1 << ITEM_SUBCLASS_WEAPON_MACE) |\
    (1 << ITEM_SUBCLASS_WEAPON_SWORD) | (1 << ITEM_SUBCLASS_WEAPON_DAGGER) | (1 << ITEM_SUBCLASS_WEAPON_FIST))

uint32 PlayerbotFactory::tradeSkills[] =
{
    SKILL_ALCHEMY,
    SKILL_ENCHANTING,
    SKILL_SKINNING,
    SKILL_TAILORING,
    SKILL_LEATHERWORKING,
    SKILL_ENGINEERING,
    SKILL_HERBALISM,
    SKILL_MINING,
    SKILL_BLACKSMITHING,
    SKILL_COOKING,
    SKILL_FIRST_AID,
    SKILL_FISHING,
    SKILL_JEWELCRAFTING
};

std::list<uint32> PlayerbotFactory::classQuestIds;
std::list<uint32> PlayerbotFactory::specialQuestIds;

PlayerbotFactory::PlayerbotFactory(Player* bot, uint32 level, uint32 itemQuality, uint32 gearScoreLimit) : level(level), itemQuality(itemQuality), gearScoreLimit(gearScoreLimit), bot(bot)
{
    botAI = GET_PLAYERBOT_AI(bot);
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

            AddPrevQuests(questId, classQuestIds);
            classQuestIds.remove(questId);
            classQuestIds.push_back(questId);
        }
    }

    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotQuestIds.begin(); i != sPlayerbotAIConfig->randomBotQuestIds.end(); ++i)
    {
        uint32 questId = *i;
        AddPrevQuests(questId, specialQuestIds);
        specialQuestIds.remove(questId);
        specialQuestIds.push_back(questId);
    }
}

void PlayerbotFactory::Prepare()
{
    if (!itemQuality)
    {
        itemQuality = ITEM_QUALITY_RARE;
    }

    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);

    bot->CombatStop(true);

    bot->GiveLevel(level);
    bot->SetUInt32Value(PLAYER_XP, 0);
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

    LOG_INFO("playerbots", "Preparing to {} randomize...", (incremental ? "incremental" : "full"));
    Prepare();
    LOG_INFO("playerbots", "Resetting player...");
    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Reset");
    bot->resetTalents(true);
    // bot->SaveToDB(false, false);
    ClearSkills();
    // bot->SaveToDB(false, false);
    ClearSpells();
    bot->SaveToDB(false, false);
    if (!incremental)
    {
        ResetQuests();
    }
    if (!sPlayerbotAIConfig->equipmentPersistence || level < sPlayerbotAIConfig->equipmentPersistenceLevel) {
        ClearAllItems();
    }
    bot->SaveToDB(false, false);

    bot->GiveLevel(level);
    bot->InitStatsForLevel();
    CancelAuras();
    bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    /*
    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Immersive");
    LOG_INFO("playerbots", "Initializing immersive...");
    InitImmersive();
    if (pmo)
        pmo->finish();
    */
    InitInstanceQuests();
    // clear quest reward inventory
    ClearInventory();
    bot->GiveLevel(level);

    if (sPlayerbotAIConfig->randomBotPreQuests)
    {
        LOG_INFO("playerbots", "Initializing quests...");
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Quests");
        InitQuests(classQuestIds);
        InitQuests(specialQuestIds);

        // quest rewards boost bot level, so reduce back
        
        bot->GiveLevel(level);
        

        ClearInventory();
        bot->SetUInt32Value(PLAYER_XP, 0);
        CancelAuras();
        bot->SaveToDB(false, false);
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells1");
    LOG_INFO("playerbots", "Initializing spells (step 1)...");
    // bot->LearnDefaultSkills();
    InitClassSpells();
    // bot->SaveToDB(false, false);
    InitAvailableSpells();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    LOG_INFO("playerbots", "Initializing skills (step 1)...");
    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills1");
    InitSkills();
    // bot->SaveToDB(false, false);
    InitSpecialSpells();
    // bot->SaveToDB(false, false);
    
    // InitTradeSkills();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Talents");
    LOG_INFO("playerbots", "Initializing talents...");
    InitTalentsTree();
    // bot->SaveToDB(false, false);
    sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", 0);
    if (botAI) {
        sPlayerbotDbStore->Reset(botAI);
        // botAI->DoSpecificAction("auto talents");
        botAI->ResetStrategies(false); // fix wrong stored strategy
    }
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells2");
    LOG_INFO("playerbots", "Initializing spells (step 2)...");
    InitAvailableSpells();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Mounts");
    LOG_INFO("playerbots", "Initializing mounts...");
    InitMounts();
    bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills2");
    // LOG_INFO("playerbots", "Initializing skills (step 2)...");
    // UpdateTradeSkills();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Equip");
    LOG_INFO("playerbots", "Initializing equipmemt...");
    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel) {
        InitEquipment(incremental);
    }
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    if (bot->getLevel() >= sPlayerbotAIConfig->minEnchantingBotLevel)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Enchant");
        LOG_INFO("playerbots", "Initializing enchant templates...");
        LoadEnchantContainer();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Bags");
    LOG_INFO("playerbots", "Initializing bags...");
    InitBags();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Ammo");
    LOG_INFO("playerbots", "Initializing ammo...");
    InitAmmo();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Food");
    LOG_INFO("playerbots", "Initializing food...");
    InitFood();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Potions");
    LOG_INFO("playerbots", "Initializing potions...");
    InitPotions();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Reagents");
    LOG_INFO("playerbots", "Initializing reagents...");
    InitReagents();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_EqSets");
    LOG_INFO("playerbots", "Initializing second equipment set...");
    // InitSecondEquipmentSet();
    if (pmo)
        pmo->finish();

    if (bot->getLevel() >= sPlayerbotAIConfig->minEnchantingBotLevel)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_EnchantTemplate");
        LOG_INFO("playerbots", "Initializing enchant templates...");
        ApplyEnchantTemplate();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Inventory");
    LOG_INFO("playerbots", "Initializing inventory...");
    // InitInventory();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Consumable");
    LOG_INFO("playerbots", "Initializing consumables...");
    AddConsumables();
    if (pmo)
        pmo->finish();
    
    LOG_INFO("playerbots", "Initializing glyphs...");
    bot->SaveToDB(false, false);
    InitGlyphs();
    
    // pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Guilds");
    // LOG_INFO("playerbots", "Initializing guilds...");
    // bot->SaveToDB(false, false);
    // InitGuild();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    // if (bot->getLevel() >= 70)
    // {
    //     pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Arenas");
    //     LOG_INFO("playerbots", "Initializing arena teams...");
    //     InitArenaTeam();
    //     if (pmo)
    //         pmo->finish();
    // }

    if (!incremental) {
        bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT, true);
        bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true);
    }
    if (bot->getLevel() >= 10)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Pet");
        LOG_INFO("playerbots", "Initializing pet...");
        InitPet();
        bot->SaveToDB(false, false);
        InitPetTalents();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Save");
    LOG_INFO("playerbots", "Saving to DB...");
    bot->SetMoney(urand(level * 100000, level * 5 * 100000));
    bot->SetHealth(bot->GetMaxHealth());
    bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
    bot->SaveToDB(false, false);
    LOG_INFO("playerbots", "Done.");
    if (pmo)
        pmo->finish();
}

void PlayerbotFactory::Refresh()
{
    // Prepare();
    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel) {
        InitEquipment(true);
    }
    ClearInventory();
    InitAmmo();
    InitFood();
    InitReagents();
    // InitPotions();
    InitTalentsTree(true, true, true);
    InitPet();
    InitPetTalents();
    InitClassSpells();
    InitAvailableSpells();
    InitSkills();
    bot->DurabilityRepairAll(false, 1.0f, false);
    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);
    uint32 money = urand(level * 1000, level * 5 * 1000);
    if (bot->GetMoney() < money)
        bot->SetMoney(money);
    bot->SaveToDB(false, false);
}

void PlayerbotFactory::AddConsumables()
{
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        {
            if (level >= 5 && level < 20)
            {
                StoreItem(CONSUM_ID_MINOR_WIZARD_OIL, 5);
            }

            if (level >= 20 && level < 40)
            {
                StoreItem(CONSUM_ID_MINOR_MANA_OIL, 5);
                StoreItem(CONSUM_ID_MINOR_WIZARD_OIL, 5);
            }

            if (level >= 40 && level < 45)
            {
                StoreItem(CONSUM_ID_MINOR_MANA_OIL, 5);
                StoreItem(CONSUM_ID_WIZARD_OIL, 5);
            }

            if (level >= 45 && level < 52)
            {
                StoreItem(CONSUM_ID_BRILLIANT_MANA_OIL, 5);
                StoreItem(CONSUM_ID_BRILLIANT_WIZARD_OIL, 5);
            }
            if (level >= 52 && level < 58) {
                StoreItem(CONSUM_ID_SUPERIOR_MANA_OIL, 5);
                StoreItem(CONSUM_ID_BRILLIANT_WIZARD_OIL, 5);
            }

            if (level >= 58)
            {
                StoreItem(CONSUM_ID_SUPERIOR_MANA_OIL, 5);
                StoreItem(CONSUM_ID_SUPERIOR_WIZARD_OIL, 5);
            }
            break;
        }
        case CLASS_PALADIN:
        case CLASS_WARRIOR:
        case CLASS_HUNTER:
        {
            if (level >= 1 && level < 5)
            {
                StoreItem(CONSUM_ID_ROUGH_SHARPENING_STONE, 5);
                StoreItem(CONSUM_ID_ROUGH_WEIGHTSTONE, 5);
            }

            if (level >= 5 && level < 15)
            {
                StoreItem(CONSUM_ID_COARSE_WEIGHTSTONE, 5);
                StoreItem(CONSUM_ID_COARSE_SHARPENING_STONE, 5);
            }

            if (level >= 15 && level < 25)
            {
                StoreItem(CONSUM_ID_HEAVY_WEIGHTSTONE, 5);
                StoreItem(CONSUM_ID_HEAVY_SHARPENING_STONE, 5);
            }

            if (level >= 25 && level < 35)
            {
                StoreItem(CONSUM_ID_SOL_SHARPENING_STONE, 5);
                StoreItem(CONSUM_ID_SOLID_WEIGHTSTONE, 5);
            }

            if (level >= 35 && level < 50)
            {
                StoreItem(CONSUM_ID_DENSE_WEIGHTSTONE, 5);
                StoreItem(CONSUM_ID_DENSE_SHARPENING_STONE, 5);
            }

            if (level >= 50 && level < 60)
            {
                StoreItem(CONSUM_ID_FEL_SHARPENING_STONE, 5);
                StoreItem(CONSUM_ID_FEL_WEIGHTSTONE, 5);
            }

            if (level >= 60)
            {
                StoreItem(CONSUM_ID_ADAMANTITE_WEIGHTSTONE, 5);
                StoreItem(CONSUM_ID_ADAMANTITE_SHARPENING_STONE, 5);
            }
            break;
        }
        case CLASS_ROGUE:
        {
            if (level >= 20 && level < 28)
            {
                StoreItem(CONSUM_ID_INSTANT_POISON, 5);
            }

            if (level >= 28 && level < 30)
            {
                StoreItem(CONSUM_ID_INSTANT_POISON_II, 5);
            }

            if (level >= 30 && level < 36)
            {
                StoreItem(CONSUM_ID_DEADLY_POISON, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_II, 5);
            }

            if (level >= 36 && level < 44)
            {
                StoreItem(CONSUM_ID_DEADLY_POISON_II, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_III, 5);
            }

            if (level >= 44 && level < 52)
            {
                StoreItem(CONSUM_ID_DEADLY_POISON_III, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_IV, 5);
            }
            if (level >= 52 && level < 60) {
                StoreItem(CONSUM_ID_DEADLY_POISON_IV, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_V, 5);
            }

            if (level >= 60 && level < 62)
            {
                StoreItem(CONSUM_ID_DEADLY_POISON_V, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_VI, 5);
            }

            if (level >= 62 && level < 68)
            {
                StoreItem(CONSUM_ID_DEADLY_POISON_VI, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_VI, 5);
            }

            if (level >= 68)
            {
                StoreItem(CONSUM_ID_DEADLY_POISON_VII, 5);
                StoreItem(CONSUM_ID_INSTANT_POISON_VII, 5);
            }
            break;
        }
    }
}

void PlayerbotFactory::InitPetTalents()
{
    Pet* pet = bot->GetPet();
    if (!pet) {
        // LOG_INFO("playerbots", "{} init pet talents failed with no pet", bot->GetName().c_str());
        return;
    }
    CreatureTemplate const* ci = pet->GetCreatureTemplate();
    if (!ci) {
        // LOG_INFO("playerbots", "{} init pet talents failed with no creature template", bot->GetName().c_str());
        return;
    }
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(ci->family);
    if (pet_family->petTalentType < 0) {
        // LOG_INFO("playerbots", "{} init pet talents failed with petTalentType < 0({})", bot->GetName().c_str(), pet_family->petTalentType);
        return;
    }
    pet->resetTalents();
    std::map<uint32, std::vector<TalentEntry const*> > spells;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if(!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );

         // prevent learn talent for different family (cheating)
        if (!((1 << pet_family->petTalentType) & talentTabInfo->petTalentMask))
            continue;

        spells[talentInfo->Row].push_back(talentInfo);
    }

    uint32 curTalentPoints = pet->GetFreeTalentPoints();
    uint32 maxTalentPoints = pet->GetMaxTalentPointsForLevel(pet->GetLevel());
    int row = 0;
    // LOG_INFO("playerbots", "{} learning, max talent points: {}, cur: {}", bot->GetName().c_str(), maxTalentPoints, curTalentPoints);
    for (std::map<uint32, std::vector<TalentEntry const*> >::iterator i = spells.begin(); i != spells.end(); ++i, ++row)
    {
        std::vector<TalentEntry const*> &spells_row = i->second;
        if (spells_row.empty())
        {
            LOG_INFO("playerbots", "{}: No spells for talent row {}", bot->GetName().c_str(), i->first);
            continue;
        }
        int attemptCount = 0;
        // keep learning for the last row
        while (!spells_row.empty() && ((((int)maxTalentPoints - (int)pet->GetFreeTalentPoints()) < 3 * (row + 1)) || (row == 5)) 
            && attemptCount++ < 10 && pet->GetFreeTalentPoints())
        {
            int index = urand(0, spells_row.size() - 1);
            TalentEntry const *talentInfo = spells_row[index];
            int maxRank = 0;
            for (int rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, (uint32)pet->GetFreeTalentPoints()); ++rank)
            {
                uint32 spellId = talentInfo->RankID[rank];
                if (!spellId)
                    continue;

                maxRank = rank;
            }
            // LOG_INFO("playerbots", "{} learn pet talent {}({})", bot->GetName().c_str(), talentInfo->TalentID, maxRank);
            if (talentInfo->DependsOn) {
                bot->LearnPetTalent(pet->GetGUID(), talentInfo->DependsOn, std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));            
            }
            bot->LearnPetTalent(pet->GetGUID(), talentInfo->TalentID, maxRank);
			spells_row.erase(spells_row.begin() + index);
        }
    }
}

void PlayerbotFactory::InitPet()
{
    Pet* pet = bot->GetPet();
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

            if (itr->second.minlevel > bot->getLevel())
                continue;
            
			ids.push_back(itr->first);
		}

        if (ids.empty())
        {
            LOG_ERROR("playerbots", "No pets available for bot {} ({} level)", bot->GetName().c_str(), bot->getLevel());
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
            if (bot->GetPetStable() && bot->GetPetStable()->CurrentPet) {
                bot->GetPetStable()->CurrentPet.value();
                // bot->GetPetStable()->CurrentPet.reset();
                bot->RemovePet(nullptr, PET_SAVE_AS_CURRENT);
                bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
            }
            if (bot->GetPetStable() && bot->GetPetStable()->GetUnslottedHunterPet()) {
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
        pet->InitStatsForLevel(bot->getLevel());
        pet->SetLevel(bot->getLevel());
        pet->SetPower(POWER_HAPPINESS, pet->GetMaxPower(Powers(POWER_HAPPINESS)));
        pet->SetHealth(pet->GetMaxHealth());
    }
    else
    {
        LOG_ERROR("playerbots", "Cannot create pet for bot {}", bot->GetName().c_str());
        return;
}

    // LOG_INFO("playerbots", "Start make spell auto cast for {} spells. {} already auto casted.", pet->m_spells.size(), pet->GetPetAutoSpellSize());
    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive()) {
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
    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(bot->getRace(), bot->getClass());
    for (PlayerCreateInfoSkills::const_iterator itr = info->skills.begin(); itr != info->skills.end(); ++itr)
    {
        uint32 skillId = itr->SkillId;
        if (!bot->HasSkill(skillId))
            continue;
        bot->SetSkill(skillId, 0, 0, 0);
    }
}

void PlayerbotFactory::ClearEverything()
{
    bot->SaveToDB(false, false);
    bot->GiveLevel(bot->getClass() == CLASS_DEATH_KNIGHT ? sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL) : sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL));
    bot->SetUInt32Value(PLAYER_XP, 0);
    LOG_INFO("playerbots", "Resetting player...");
    bot->resetTalents(true);
    bot->SaveToDB(false, false);
    ClearSkills();
    // bot->SaveToDB(false, false);
    ClearSpells();
    bot->SaveToDB(false, false);
    ClearInventory();
    ResetQuests();
    bot->SaveToDB(false, false);
}

void PlayerbotFactory::ClearSpells()
{
    std::list<uint32> spells;
    for(PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if(itr->second->State == PLAYERSPELL_REMOVED) {
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
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 entry = quest->GetQuestId();

        // remove all quest entries for 'entry' from quest log
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 quest = bot->GetQuestSlotQuestId(slot);
            if (quest == entry)
            {
                bot->SetQuestSlot(slot, 0);
            }
        }

        // reset rewarded for restart repeatable quest
        bot->getQuestStatusMap().erase(entry);
        //bot->getQuestStatusMap()[entry].m_rewarded = false;
        //bot->getQuestStatusMap()[entry].m_status = QUEST_STATUS_NONE;
    }
    //bot->UpdateForQuestWorldObjects();
    CharacterDatabase.Execute("DELETE FROM character_queststatus WHERE guid = {}", bot->GetGUID().GetCounter());
}

void PlayerbotFactory::InitSpells()
{
    InitAvailableSpells();
}

void PlayerbotFactory::InitTalentsTree(bool increment/*false*/, bool use_template/*true*/, bool reset/*false*/)
{
    uint32 specNo;
    uint8 cls = bot->getClass();
    std::map<uint8, uint32> tabs = AiFactory::GetPlayerSpecTabs(bot);
    uint32 total_tabs = tabs[0] + tabs[1] + tabs[2];
    if (increment && bot->GetFreeTalentPoints() <= 2 && total_tabs != 0) {
        specNo = AiFactory::GetPlayerSpecTab(bot);
    } else {
        uint32 point = urand(0, 100);
        uint32 p1 = sPlayerbotAIConfig->specProbability[cls][0];
        uint32 p2 = p1 + sPlayerbotAIConfig->specProbability[cls][1];
        specNo = (point < p1 ? 0 : (point < p2 ? 1 : 2));
    }
    if (reset) {
        bot->resetTalents(true);
    }
    // use template if can
    if (use_template && sPlayerbotAIConfig->defaultTalentsOrder[cls][specNo].size() > 0) {
        InitTalentsByTemplate(specNo);
    } else {
        InitTalents(specNo);
        if (bot->GetFreeTalentPoints())
            InitTalents((specNo + 1) % 3);
    }
}

class DestroyItemsVisitor : public IterateItemsVisitor
{
    public:
        DestroyItemsVisitor(Player* bot) : IterateItemsVisitor(), bot(bot) { }

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
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_PLATE && !bot->HasSkill(SKILL_PLATE_MAIL)) {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && !bot->HasSkill(SKILL_MAIL)) {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_LEATHER && !bot->HasSkill(SKILL_LEATHER)) {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_CLOTH && !bot->HasSkill(SKILL_CLOTH)) {
        return false;
    }
    if (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD && !bot->HasSkill(SKILL_SHIELD)) {
        return false;
    }
    
    return true;
    // for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    // {
    //     if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
    //         continue;

    //     if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
    //         continue;

    //     if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_PALADIN && proto->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
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

void PlayerbotFactory::AddItemStats(uint32 mod, uint8 &sp, uint8 &ap, uint8 &tank)
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
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE)
            return false;
        break;
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
            return false;
        break;
    case CLASS_WARRIOR:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
			proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
			proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
        break;
    case CLASS_PALADIN:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
            return false;
        break;
	case CLASS_DEATH_KNIGHT:
		if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
			proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
			proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
			proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
			proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
			return false;
		break;
    case CLASS_SHAMAN:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
        break;
    case CLASS_DRUID:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
        break;
    case CLASS_HUNTER:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW)
            return false;
        break;
    case CLASS_ROGUE:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
				proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
        break;
    }

    return true;
}

bool PlayerbotFactory::CanEquipItem(ItemTemplate const* proto, uint32 desiredQuality)
{
    if (proto->Duration != 0)
        return false;

    if (proto->Quality != desiredQuality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM /*|| proto->Bonding == BIND_WHEN_USE*/)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
        return false;

    uint32 level = bot->getLevel();
    uint32 delta = 2;
    if (level < 15)
        delta = std::min(level, 15u); // urand(7, 15);
    // else if (proto->Class == ITEM_CLASS_WEAPON || proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
    //     delta = urand(2, 3);
    // else if (!(level % 10) || (level % 10) == 9)
    //     delta = 2;
    else if (level < 40)
        delta = 10; //urand(5, 10);
    else if (level < 60)
        delta = 6; // urand(3, 7);
    else if (level < 70)
        delta = 9; // urand(2, 5);
    else if (level < 80)
        delta = 9; // urand(2, 4);
    else if (level == 80)
        delta = 9; // urand(2, 4);

    if (desiredQuality > ITEM_QUALITY_NORMAL &&
            (requiredLevel > level || requiredLevel < level - delta))
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
//                             bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->getLevel(), bot->GetName(), newItemId, slot, itemInSlot);
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
//                             bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->getLevel(), bot->GetName(), newItemId, slot);
//                         break;
//                     }
//                 }
//             }
//             quality--;
//         } while (!found && quality != ITEM_QUALITY_POOR);
//         if (!found)
//         {
//             LOG_INFO("playerbots", "Bot {} {}:{} <{}>: no item for slot {}",
//                 bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->getLevel(), bot->GetName(), slot);
//             continue;
//         }
//     }
// }

void PlayerbotFactory::InitEquipment(bool incremental)
{
    std::unordered_map<uint8, std::vector<uint32> > items;
    int tab = AiFactory::GetPlayerSpecTab(bot);
    
    uint32 blevel = bot->getLevel();
    int32 delta = 2;
    if (blevel < 15)
        delta = std::min(blevel, 15u);
    else if (blevel < 40)
        delta = 10;
    else if (blevel < 60)
        delta = 6;
    else if (blevel < 70)
        delta = 9;
    else if (blevel < 80)
        delta = 9;
    else if (blevel == 80)
        delta = 9;

    for(uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;

        uint32 desiredQuality = itemQuality;
        if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL) {
            desiredQuality--;
        }
        do
        {
            ItemTemplateContainer const* itemTemplate = sObjectMgr->GetItemTemplateStore();
            for (uint32 requiredLevel = bot->GetLevel(); requiredLevel > std::max((int32)bot->GetLevel() - delta, 0); requiredLevel--) {
                for (InventoryType inventoryType : GetPossibleInventoryTypeListBySlot((EquipmentSlots)slot)) {
                    for (uint32 itemId : sRandomItemMgr->GetCachedEquipments(requiredLevel, inventoryType)) {
                        if (itemId == 46978) { // shaman earth ring totem
                            continue;
                        }
                        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                        if (!proto)
                            continue;
                        
                        if (gearScoreLimit != 0 && CalcMixedGearScore(proto->ItemLevel, proto->Quality) > gearScoreLimit) {
                            continue;
                        }
                        if (proto->Class != ITEM_CLASS_WEAPON &&
                            proto->Class != ITEM_CLASS_ARMOR)
                            continue;

                        if (!CanEquipItem(proto, desiredQuality))
                            continue;

                        if (proto->Class == ITEM_CLASS_ARMOR && (
                            slot == EQUIPMENT_SLOT_HEAD ||
                            slot == EQUIPMENT_SLOT_SHOULDERS ||
                            slot == EQUIPMENT_SLOT_CHEST ||
                            slot == EQUIPMENT_SLOT_WAIST ||
                            slot == EQUIPMENT_SLOT_LEGS ||
                            slot == EQUIPMENT_SLOT_FEET ||
                            slot == EQUIPMENT_SLOT_WRISTS ||
                            slot == EQUIPMENT_SLOT_HANDS) && !CanEquipArmor(proto))
                                continue;

                        if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(proto))
                            continue;

                        if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
                            continue;
                        
                        uint16 dest = 0;
                        if (CanEquipUnseenItem(slot, dest, itemId))
                            items[slot].push_back(itemId);
                    }
                }
                if (items[slot].size() >= 25) break;
            }
        } while (items[slot].size() < 25 && desiredQuality-- > ITEM_QUALITY_NORMAL);
    }

    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;

        std::vector<uint32>& ids = items[slot];
        if (ids.empty())
        {
            continue;
        }
        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (incremental && !IsDesiredReplacement(oldItem)) {
            continue;
        }

        float bestScoreForSlot = -1;
        uint32 bestItemForSlot = 0;
        for (int attempts = 0; attempts < std::max((int)(ids.size() * 0.75), 1); attempts++)
        {
            uint32 index = urand(0, ids.size() - 1);
            uint32 newItemId = ids[index];
            
            uint16 dest;
            if (!CanEquipUnseenItem(slot, dest, newItemId))
                continue;
            
            float cur_score = CalculateItemScore(newItemId, bot);
            if (cur_score > bestScoreForSlot) {
                bestScoreForSlot = cur_score;
                bestItemForSlot = newItemId;
            }
        }
        if (bestItemForSlot == 0) {
            continue;
        }
        if (oldItem)
        {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }
        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, bestItemForSlot)) {
            continue;
        }
        Item* newItem = bot->EquipNewItem(dest, bestItemForSlot, true);
        if (newItem)
        {
            newItem->AddToWorld();
            newItem->AddToUpdateQueueOf(bot);
            // bot->AutoUnequipOffhandIfNeed();
            // EnchantItem(newItem);
        }
    }
}

bool PlayerbotFactory::IsDesiredReplacement(Item* item)
{
    if (!item)
        return true;

    ItemTemplate const* proto = item->GetTemplate();
    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel) {
        return true;
    }
    // if (!requiredLevel)
    // {
    //     requiredLevel = sRandomItemMgr->GetMinLevelFromCache(proto->ItemId);
    // }
    
    uint32 delta = 1 + (80 - bot->getLevel()) / 10;
    return proto->Quality < ITEM_QUALITY_RARE || int32(bot->getLevel() - requiredLevel) > delta;
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
//     while (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL)
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
//                             if (proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || proto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER || proto->SubClass == ITEM_SUBCLASS_WEAPON_FIST ||
//                                 proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || proto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD)
//                                 continue;
//                             break;
//                         default:
//                             if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
//                                 proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
//                                 continue;
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
//     } while (items[ITEM_CLASS_ARMOR].empty() && items[ITEM_CLASS_WEAPON].empty() && desiredQuality-- > ITEM_QUALITY_NORMAL);

//     for (std::map<uint32, std::vector<uint32>>::iterator i = items.begin(); i != items.end(); ++i)
//     {
//         std::vector<uint32>& ids = i->second;
//         if (ids.empty())
//         {
//             LOG_DEBUG("playerbots",   "{}: no items to make second equipment set for slot {}", bot->GetName().c_str(), i->first);
//             continue;
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

void PlayerbotFactory::InitBags()
{
    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        uint32 newItemId = 51809;
        Item *old_bag = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (old_bag && old_bag->GetTemplate()->ItemId == newItemId) {
            continue;
        }
        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, newItemId))
            continue;
        
        if (old_bag) {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }
        Item* newItem = bot->EquipNewItem(dest, newItemId, true);
        if (newItem)
        {
            newItem->AddToWorld();
            newItem->AddToUpdateQueueOf(bot);
        }
    }
}

void PlayerbotFactory::EnchantItem(Item* item)
{
    if (bot->getLevel() < sPlayerbotAIConfig->minEnchantingBotLevel)
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
        LOG_DEBUG("playerbots",   "{}: no enchantments found for item {}", bot->GetName().c_str(), item->GetTemplate()->ItemId);
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

bool PlayerbotFactory::CanEquipUnseenItem(uint8 slot, uint16 &dest, uint32 item)
{
    dest = 0;

    if (Item* pItem = Item::CreateItem(item, 1, bot, false, 0, true))
    {
        InventoryResult result = bot->CanEquipItem(slot, dest, pItem, true, false);
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
    uint32 maxValue = level * 5;
    bot->UpdateSkillsForLevel();

    uint16 step = bot->GetSkillValue(SKILL_RIDING) ? bot->GetSkillStep(SKILL_RIDING) : 1;

    if (bot->getLevel() >= 70)
        bot->SetSkill(SKILL_RIDING, step, 300, 300);
    else if (bot->getLevel() >= 60)
        bot->SetSkill(SKILL_RIDING, step, 225, 225);
    else if (bot->getLevel() >= 40)
        bot->SetSkill(SKILL_RIDING, step, 150, 150);
    else if (bot->getLevel() >= 20)
        bot->SetSkill(SKILL_RIDING, step, 75, 75);
    else
        bot->SetSkill(SKILL_RIDING, 0, 0, 0);

    uint32 skillLevel = bot->getLevel() < 40 ? 0 : 1;
    uint32 dualWieldLevel = bot->getLevel() < 20 ? 0 : 1;
    SetRandomSkill(SKILL_DEFENSE);
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
            bot->SetSkill(SKILL_MAIL, 0, skillLevel, skillLevel);
            bot->SetSkill(SKILL_DUAL_WIELD, 0, dualWieldLevel, dualWieldLevel);
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
            bot->SetSkill(SKILL_DUAL_WIELD, 0, 1, 1);
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
    uint32 curValue = bot->GetSkillValue(id);

    uint16 step = bot->GetSkillValue(id) ? bot->GetSkillStep(id) : 1;

    if (!bot->HasSkill(id) || value > curValue)
        bot->SetSkill(id, step, value, maxValue);
}

void PlayerbotFactory::InitAvailableSpells()
{
    bot->LearnDefaultSkills();

    if (trainerIdCache.empty()) {
        CreatureTemplateContainer const* creatureTemplateContainer = sObjectMgr->GetCreatureTemplates();
        for (CreatureTemplateContainer::const_iterator i = creatureTemplateContainer->begin(); i != creatureTemplateContainer->end(); ++i)
        {
            CreatureTemplate const& co = i->second;
            if (co.trainer_type != TRAINER_TYPE_TRADESKILLS && co.trainer_type != TRAINER_TYPE_CLASS)
                continue;

            if (co.trainer_type == TRAINER_TYPE_CLASS && co.trainer_class != bot->getClass())
                continue;

            uint32 trainerId = co.Entry;
            trainerIdCache.push_back(trainerId);
        }
    }
    for (uint32 trainerId : trainerIdCache) {
		TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);

        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr !=  trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            if (!tSpell->learnedSpell[0] && !bot->IsSpellFitByClassAndRace(tSpell->learnedSpell[0]))
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
                    spellInfo->Effects[j].Effect == SPELL_EFFECT_SKILL_STEP ||
                    spellInfo->Effects[j].Effect == SPELL_EFFECT_DUAL_WIELD)
                {
                    learn = false;
                    break;
                }
            }
            if (!learn) {
                continue;
            }

            if (tSpell->learnedSpell[0]) {
                bot->learnSpell(tSpell->learnedSpell[0], false);
            }
            else {
                botAI->CastSpell(tSpell->spell, bot);
            }
        }
    }
}

void PlayerbotFactory::InitClassSpells()
{
    int32_t level = bot->getLevel();
    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            bot->learnSpell(78, true);
            bot->learnSpell(2457, true);
            if (level >= 10) {
                bot->learnSpell(71, false); // Defensive Stance
                bot->learnSpell(355, false); // Taunt
                bot->learnSpell(7386, false); // Sunder Armor
            }
            if (level >= 30) {
                bot->learnSpell(2458, false); // Berserker Stance
            }
            break;
        case CLASS_PALADIN:
            bot->learnSpell(21084, true);
            bot->learnSpell(635, true);
            if (level >= 12) {
                bot->learnSpell(7328, false); // Redemption
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
            //to leave DK starting area 
            bot->learnSpell(50977, false);
            break;
        case CLASS_HUNTER:
            bot->learnSpell(2973, true);
            bot->learnSpell(75, true);
            if (level >= 10) {
                bot->learnSpell(883, false); // call pet
                bot->learnSpell(1515, false); // tame pet
                bot->learnSpell(6991, false); // feed pet
                bot->learnSpell(982, false); // revive pet
                bot->learnSpell(2641, false); // dismiss pet
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
            bot->learnSpell(688, true); // summon imp
            if (level >= 10) {
                bot->learnSpell(697, false); // summon voidwalker
            }
            if (level >= 20) {
                bot->learnSpell(712, false); // summon succubus
            }
            if (level >= 30) {
                bot->learnSpell(691, false); // summon felhunter
            }
            break;
        case CLASS_DRUID:
            bot->learnSpell(5176, true);
            bot->learnSpell(5185, true);
            if (level >= 10) {
                bot->learnSpell(5487, false); // bear form
                bot->learnSpell(6795, false); // Growl
                bot->learnSpell(6807, false); // Maul
            }
            break;
        case CLASS_SHAMAN:
            bot->learnSpell(403, true);
            bot->learnSpell(331, true);
            // bot->learnSpell(66747, true); // Totem of the Earthen Ring
            if (level >= 4) {
                bot->learnSpell(8071, false); // stoneskin totem
            }
            if (level >= 10) {
                bot->learnSpell(3599, false); // searing totem
            }
            if (level >= 20) {
                bot->learnSpell(5394, false); // healing stream totem
            }
            break;
        default:
            break;
    }
}

void PlayerbotFactory::InitSpecialSpells()
{
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotSpellIds.begin(); i != sPlayerbotAIConfig->randomBotSpellIds.end(); ++i)
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
    std::map<uint32, std::vector<TalentEntry const*> > spells;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if(!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
        if(!talentTabInfo || talentTabInfo->tabpage != specNo)
            continue;

        if( (classMask & talentTabInfo->ClassMask) == 0 )
            continue;

        spells[talentInfo->Row].push_back(talentInfo);
    }

    uint32 freePoints = bot->GetFreeTalentPoints();
    for (std::map<uint32, std::vector<TalentEntry const*> >::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        std::vector<TalentEntry const*> &spells_row = i->second;
        if (spells_row.empty())
        {
            LOG_INFO("playerbots", "{}: No spells for talent row {}", bot->GetName().c_str(), i->first);
            continue;
        }
        int attemptCount = 0;
        while (!spells_row.empty() && (int)freePoints - (int)bot->GetFreeTalentPoints() < 5 && attemptCount++ < 3 && bot->GetFreeTalentPoints())
        {
            int index = urand(0, spells_row.size() - 1);
            TalentEntry const *talentInfo = spells_row[index];
            int maxRank = 0;
            for (int rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, bot->GetFreeTalentPoints()); ++rank)
            {
                uint32 spellId = talentInfo->RankID[rank];
                if (!spellId)
                    continue;

                maxRank = rank;
            }
            if (talentInfo->DependsOn) {
                bot->LearnTalent(talentInfo->DependsOn, std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));            
            }
            bot->LearnTalent(talentInfo->TalentID, maxRank);
			spells_row.erase(spells_row.begin() + index);
        }

        freePoints = bot->GetFreeTalentPoints();
    }
}

void PlayerbotFactory::InitTalentsByTemplate(uint32 specNo)
{
    if (sPlayerbotAIConfig->defaultTalentsOrder[bot->getClass()][specNo].size() == 0) {
        return;
    }
    uint32 classMask = bot->getClassMask();
    std::map<uint32, std::vector<TalentEntry const*> > spells_row;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if(!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
        if(!talentTabInfo)
            continue;

        if( (classMask & talentTabInfo->ClassMask) == 0 )
            continue;

        spells_row[talentInfo->Row].push_back(talentInfo);
    }

    // bot->SaveToDB();
    if (bot->GetLevel() < 80 && sPlayerbotAIConfig->defaultTalentsOrder[bot->getClass()][specNo].size() != 0) {
        for (std::vector<uint32> &p : sPlayerbotAIConfig->defaultTalentsOrderLowLevel[bot->getClass()][specNo]) {
            uint32 tab = p[0], row = p[1], col = p[2], lvl = p[3];
            uint32 talentID = -1;

            std::vector<TalentEntry const*> &spells = spells_row[row];
            if (spells.size() <= 0) {
                return;
            }
            // assert(spells.size() > 0);
            for (TalentEntry const* talentInfo : spells) {
                if (talentInfo->Col != col) {
                    continue;
                }
                TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
                if (talentTabInfo->tabpage != tab) {
                    continue;
                }
                if (talentInfo->DependsOn) {
                    bot->LearnTalent(talentInfo->DependsOn, std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));            
                }
                talentID = talentInfo->TalentID;
            }
            assert(talentID != -1);
            bot->LearnTalent(talentID, std::min(lvl, bot->GetFreeTalentPoints()) - 1);
            if (bot->GetFreeTalentPoints() == 0) {
                break;
            }
        }
    }

    for (std::vector<uint32> &p : sPlayerbotAIConfig->defaultTalentsOrder[bot->getClass()][specNo]) {
        uint32 tab = p[0], row = p[1], col = p[2], lvl = p[3];
        uint32 talentID = -1;

        std::vector<TalentEntry const*> &spells = spells_row[row];
        if (spells.size() <= 0) {
            return;
        }
        // assert(spells.size() > 0);
        for (TalentEntry const* talentInfo : spells) {
            if (talentInfo->Col != col) {
                continue;
            }
            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
            if (talentTabInfo->tabpage != tab) {
                continue;
            }
            if (talentInfo->DependsOn) {
                bot->LearnTalent(talentInfo->DependsOn, std::min(talentInfo->DependsOnRank, bot->GetFreeTalentPoints() - 1));            
            }
            talentID = talentInfo->TalentID;
        }
        assert(talentID != -1);
        bot->LearnTalent(talentID, std::min(lvl, bot->GetFreeTalentPoints()) - 1);
        if (bot->GetFreeTalentPoints() == 0) {
            break;
        }
    }
}

ObjectGuid PlayerbotFactory::GetRandomBot()
{
    GuidVector guids;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin(); i != sPlayerbotAIConfig->randomBotAccounts.end(); i++)
    {
        uint32 accountId = *i;
        if (!AccountMgr::GetCharactersCount(accountId))
            continue;

        CharacterDatabasePreparedStatement * stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID);
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
    Quest const *quest = sObjectMgr->GetQuestTemplate(questId);
    for (Quest::PrevQuests::const_iterator iter = quest->prevQuests.begin(); iter != quest->prevQuests.end(); ++iter)
    {
        uint32 prevId = abs(*iter);
        AddPrevQuests(prevId, questIds);
        questIds.remove(prevId);
        questIds.push_back(prevId);
    }
}

void PlayerbotFactory::InitQuests(std::list<uint32>& questMap)
{
    uint32 count = 0;
    for (std::list<uint32>::iterator i = questMap.begin(); i != questMap.end(); ++i)
    {
        uint32 questId = *i;
        Quest const *quest = sObjectMgr->GetQuestTemplate(questId);

        if (!bot->SatisfyQuestClass(quest, false) || quest->GetMinLevel() > bot->getLevel() || !bot->SatisfyQuestRace(quest, false))
            continue;

        bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
        bot->RewardQuest(quest, 0, bot, false);

        LOG_INFO("playerbots", "Bot {} ({} level) rewarded quest {}: {} (MinLevel={}, QuestLevel={})",
            bot->GetName().c_str(), bot->getLevel(), questId, quest->GetTitle().c_str(), quest->GetMinLevel(), quest->GetQuestLevel());

        if (!(count++ % 10))
            ClearInventory();
    }

    ClearInventory();
}

void PlayerbotFactory::InitInstanceQuests()
{
    // Yunfan: use configuration instead of hard code

    // The Caverns of Time
    if (bot->GetLevel() >= 64) {
        uint32 questId = 10277;
        Quest const *quest = sObjectMgr->GetQuestTemplate(questId);
        bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
        bot->RewardQuest(quest, 5, bot, false);
    }
    // Return to Andormu
    if (bot->GetLevel() >= 66) {
        uint32 questId = 10285;
        Quest const *quest = sObjectMgr->GetQuestTemplate(questId);
        bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
        bot->RewardQuest(quest, 5, bot, false);
    }
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
        case ITEM_SUBCLASS_WEAPON_THROWN:
            subClass = ITEM_SUBCLASS_THROWN;
            break;
    }

    if (!subClass)
        return;

    uint32 entry = sRandomItemMgr->GetAmmo(level, subClass);
    uint32 count = bot->GetItemCount(entry);
    uint32 maxCount = 10000;

    if (count < maxCount / 2)
    {
        if (Item* newItem = StoreNewItemInInventorySlot(bot, entry, maxCount / 2)) {
            newItem->AddToUpdateQueueOf(bot);
        }
    }
    bot->SetAmmo(entry);
}

uint32 PlayerbotFactory::CalcMixedGearScore(uint32 gs, uint32 quality)
{
    return gs * (quality + 1);
}

void PlayerbotFactory::InitMounts()
{
    uint32 firstmount = 20;
    uint32 secondmount = 40;
    uint32 thirdmount = 60;
    uint32 fourthmount = 70;

    if (bot->getLevel() < firstmount)
        return;

    std::map<uint8, std::map<uint32, std::vector<uint32>>> mounts;
    std::vector<uint32> slow, fast, fslow, ffast;

    switch (bot->getRace())
    {
        case RACE_HUMAN:
            slow = { 470, 6648, 458, 472 };
            fast = { 23228, 23227, 23229 };
            break;
        case RACE_ORC:
            slow = { 6654, 6653, 580 };
            fast = { 23250, 23252, 23251 };
            break;
        case RACE_DWARF:
            slow = { 6899, 6777, 6898 };
            fast = { 23238, 23239, 23240 };
            break;
        case RACE_NIGHTELF:
            slow = { 10789, 8394, 10793 };
            fast = { 24252, 63637, 22723 };
            break;
        case RACE_UNDEAD_PLAYER:
            slow = { 17463, 17464, 17462 };
            fast = { 17465, 23246, 66846 };
            break;
        case RACE_TAUREN:
            slow = { 18990, 18989, 64657 };
            fast = { 23249, 23248, 23247 };
            break;
        case RACE_GNOME:
            slow = { 10969, 17453, 10873, 17454 };
            fast = { 23225, 23223, 23222 };
            break;
        case RACE_TROLL:
            slow = { 10796, 10799, 8395, 472 };
            fast = { 23241, 23242, 23243 };
            break;
        case RACE_DRAENEI:
            slow = { 34406, 35711, 35710 };
            fast = { 35713, 35712, 35714 };
            break;
        case RACE_BLOODELF:
            slow = { 33660, 35020, 35022, 35018 };
            fast = { 35025, 35025, 35027 };
            break;
    }

    switch (bot->GetTeamId())
    {
        case TEAM_ALLIANCE:
            fslow = { 32235, 32239, 32240 };
            ffast = { 32242, 32289, 32290, 32292 };
            break;
        case TEAM_HORDE:
            fslow = { 32244, 32245, 32243 };
            ffast = { 32295, 32297, 32246, 32296 };
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
        if (bot->getLevel() < secondmount && type == 1)
            continue;

        if (bot->getLevel() < thirdmount && type == 2)
            continue;

        if (bot->getLevel() < fourthmount && type == 3)
            continue;

        uint32 index = urand(0, mounts[bot->getRace()][type].size() - 1);
        uint32 spell = mounts[bot->getRace()][type][index];
        if (spell)
        {
            bot->learnSpell(spell);
            LOG_DEBUG("playerbots", "Bot {} ({}) learned {} mount {}", bot->GetGUID().ToString().c_str(), bot->getLevel(), type == 0 ? "slow" : (type == 1 ? "fast" : "flying"), spell);
        }
    }
}

void PlayerbotFactory::InitPotions()
{
    uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
    for (uint8 i = 0; i < 2; ++i)
    {
        uint32 effect = effects[i];
        FindPotionVisitor visitor(bot, effect);
        IterateItems(&visitor);
        if (!visitor.GetResult().empty())
            continue;

        uint32 itemId = sRandomItemMgr->GetRandomPotion(level, effect);
        if (!itemId)
        {
            LOG_INFO("playerbots", "No potions (type {}) available for bot {} ({} level)", effect, bot->GetName().c_str(), bot->getLevel());
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

void PlayerbotFactory::InitFood()
{
    if (sPlayerbotAIConfig->freeFood) {
        return;
    }
    std::map<uint32, std::vector<uint32> > items;
    ItemTemplateContainer const* itemTemplateContainer = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator i = itemTemplateContainer->begin(); i != itemTemplateContainer->end(); ++i)
    {
        uint32 itemId = i->first;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        if (proto->Class != ITEM_CLASS_CONSUMABLE ||
            proto->SubClass != ITEM_SUBCLASS_FOOD ||
            (proto->Spells[0].SpellCategory != 11 && proto->Spells[0].SpellCategory != 59) ||
            proto->Bonding != NO_BIND)
            continue;

        if (proto->RequiredLevel > bot->getLevel() || proto->RequiredLevel < bot->getLevel() - 9)
            continue;

        if (proto->RequiredSkill && !bot->HasSkill(proto->RequiredSkill))
            continue;

        if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
            continue;

        items[proto->Spells[0].SpellCategory].push_back(itemId);
    }

    uint32 categories[] = { 11, 59 };
    for (int i = 0; i < sizeof(categories) / sizeof(uint32); ++i)
    {
        uint32 category = categories[i];
        std::vector<uint32>& ids = items[category];
        int tries = 0;
        for (int j = 0; j < 2; j++) {
            uint32 index = urand(0, ids.size() - 1);
            if (index >= ids.size())
                continue;

            uint32 itemId = ids[index];
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            // beer / wine ... 
            if (proto->Spells[0].SpellId == 11007 || proto->Spells[0].SpellId == 11008 || proto->Spells[0].SpellId == 11009 ||
                proto->Spells[0].SpellId == 11629 || proto->Spells[0].SpellId == 50986)  
            {
                tries++;
                if (tries > 5) {
                    continue;
                }
                j--;
                continue;
            }
            // bot->StoreNewItemInBestSlots(itemId, urand(1, proto->GetMaxStackSize()));
            bot->StoreNewItemInBestSlots(itemId, proto->GetMaxStackSize());
        }
   }
}

void PlayerbotFactory::InitReagents()
{
    int level = bot->getLevel();
    std::vector<std::pair<uint32, uint32>> items;
    switch (bot->getClass()) 
    {
        case CLASS_ROGUE:
        {
            std::vector<int> instant_poison_ids = {43231, 43230, 21927, 8928, 8927, 8926, 6950, 6949, 6947};
            std::vector<int> deadly_poison_ids = {43233, 43232, 22054, 22053, 20844, 8985, 8984, 2893, 2892};
            for (int& itemId: deadly_poison_ids) {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > bot->getLevel())
                    continue;
                items.push_back({itemId, 20}); // deadly poison
                break;    
            }
            for (int& itemId: instant_poison_ids) {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > bot->getLevel())
                    continue;
                items.push_back({itemId, 20}); // instant poison
                break;    
            }
        }
            break;
        case CLASS_SHAMAN:
            // items.push_back({46978, 1}); // Totem
            items.push_back({5175, 1}); // Earth Totem
            items.push_back({5176, 1}); // Flame Totem
            items.push_back({5177, 1}); // Water Totem
            items.push_back({5178, 1}); // Air Totem
            if (bot->getLevel() >= 30)
                items.push_back({17030, 40}); // Ankh
            break;
        case CLASS_WARLOCK:
            items.push_back({6265, 10}); // shard
            break;
        case CLASS_PRIEST:
            if (level >= 48 && level < 60) {
                items.push_back({17028, 40});
                // bot->StoreNewItemInBestSlots(17028, 40); // Wild Berries
            } else if (level >= 60 && level < 80) {
                items.push_back({17029, 40});
                // bot->StoreNewItemInBestSlots(17029, 40); // Wild Berries
            } else if (level >= 80) {
                items.push_back({44615, 40});
                // bot->StoreNewItemInBestSlots(44615, 40); // Wild Berries
            }
            break;
        case CLASS_MAGE:
            items.push_back({17020, 40});
            items.push_back({17031, 40}); // portal
            items.push_back({17032, 40}); // portal
            // bot->StoreNewItemInBestSlots(17020, 40); // Arcane Powder
            break;
        case CLASS_DRUID:
            if (level >= 20 && level < 30) {
                items.push_back({17034, 40});
            }
            if (level >= 30 && level < 40) {
                items.push_back({17035, 40});
            }
            if (level >= 40 && level < 50) {
                items.push_back({17036, 40});
            }
            if (level >= 50 && level < 60) {
                items.push_back({17037, 40});
                items.push_back({17021, 40});
            }
            if (level >= 60 && level < 70) {
                items.push_back({17038, 40});
                items.push_back({17026, 40});
            } 
            if (level >= 70 && level < 80) {
                items.push_back({22147, 40});
                items.push_back({22148, 40});
            } 
            if (level >= 80) {
                items.push_back({44614, 40});
                items.push_back({44605, 40});
            }
            break;
        case CLASS_PALADIN:
            items.push_back({21177, 100});
            break;
        case CLASS_DEATH_KNIGHT:
            items.push_back({21177, 40});
            break;
        default:
            break;
    }
    for (std::pair item : items) {
        bot->StoreNewItemInBestSlots(item.first, item.second);
    }
}

void PlayerbotFactory::InitGlyphs()
{
    bot->InitGlyphsForLevel();

    for (uint32 slotIndex = 0; slotIndex < MAX_GLYPH_SLOT_INDEX; ++slotIndex)
    {
        bot->SetGlyph(slotIndex, 0, true);
    }

    uint32 level = bot->getLevel();
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

    if (!maxSlot)
        return;

    std::list<uint32> glyphs;
    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (ItemTemplateContainer::const_iterator i = itemTemplates->begin(); i != itemTemplates->end(); ++i)
    {
        uint32 itemId = i->first;
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
            SpellInfo const *entry = sSpellMgr->GetSpellInfo(spellId);
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

    if (glyphs.empty())
    {
        LOG_INFO("playerbots", "No glyphs found for bot {}", bot->GetName().c_str());
        return;
    }

    std::unordered_set<uint32> chosen;
    for (uint32 slotIndex = 0; slotIndex < maxSlot; ++slotIndex)
    {
        uint32 slot = bot->GetGlyphSlot(slotIndex);
        GlyphSlotEntry const *gs = sGlyphSlotStore.LookupEntry(slot);
        if (!gs)
            continue;

        std::vector<uint32> ids;
        for (std::list<uint32>::iterator i = glyphs.begin(); i != glyphs.end(); ++i)
        {
            uint32 id = *i;
            GlyphPropertiesEntry const *gp = sGlyphPropertiesStore.LookupEntry(id);
            if (!gp || gp->TypeFlags != gs->TypeFlags)
                continue;

            ids.push_back(id);
        }

        int maxCount = urand(0, 3);
        int count = 0;
        bool found = false;
        for (int attempts = 0; attempts < 15; ++attempts)
        {
            uint32 index = urand(0, ids.size() - 1);
            if (index >= ids.size())
                continue;

            uint32 id = ids[index];
            if (chosen.find(id) != chosen.end())
                continue;

            chosen.insert(id);

            bot->SetGlyph(slotIndex, id, true);
            found = true;
            break;
        }
        if (!found)
            LOG_INFO("playerbots", "No glyphs found for bot {} index {} slot {}", bot->GetName().c_str(), slotIndex, slot);
    }
}

void PlayerbotFactory::CancelAuras()
{
    bot->RemoveAllAuras();
}

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
        StoreItem(2901, 1); // Mining Pick
    }

    if (bot->HasSkill(SKILL_BLACKSMITHING) || bot->HasSkill(SKILL_ENGINEERING))
    {
        StoreItem(5956, 1); // Blacksmith Hammer
    }

    if (bot->HasSkill(SKILL_ENGINEERING))
    {
        StoreItem(6219, 1); // Arclight Spanner
    }

    if (bot->HasSkill(SKILL_ENCHANTING))
    {
        StoreItem(16207, 1); // Runed Arcanite Rod
    }

    if (bot->HasSkill(SKILL_SKINNING))
    {
        StoreItem(7005, 1); // Skinning Knife
    }
}

Item* PlayerbotFactory::StoreItem(uint32 itemId, uint32 count)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
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
        LOG_ERROR("playerbots", "No trade items available for bot {} ({} level)", bot->GetName().c_str(), bot->getLevel());
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
            int maxStackSize = proto->GetMaxStackSize()/2;
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

        if ((proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON) || (proto->Bonding == BIND_WHEN_PICKED_UP || proto->Bonding == BIND_WHEN_USE))
            continue;

        if (proto->Class == ITEM_CLASS_ARMOR && !CanEquipArmor(proto))
            continue;

        if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(proto))
            continue;

        if (!CanEquipItem(proto, desiredQuality))
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

    bot->SaveToDB(false, false);

    // add guild tabard
    if (bot->GetGuildId() && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);

    if (sPlayerbotAIConfig->randomBotGuilds.empty())
        RandomPlayerbotFactory::CreateRandomGuilds();

    std::vector<uint32> guilds;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotGuilds.begin(); i != sPlayerbotAIConfig->randomBotGuilds.end(); ++i)
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

    if (guild->GetMemberSize() < urand(10, 15))
        guild->AddMember(bot->GetGUID(), urand(GR_OFFICER, GR_INITIATE));

    // add guild tabard
    if (bot->GetGuildId() && bot->getLevel() > 9 && urand(0, 4) && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);

    bot->SaveToDB(false, false);
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
            int32 delta = urand(0, 5 + bot->getLevel() / 3);
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

    if (sPlayerbotAIConfig->randomBotArenaTeams.empty())
        RandomPlayerbotFactory::CreateRandomArenaTeams();

    std::vector<uint32> arenateams;
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotArenaTeams.begin(); i != sPlayerbotAIConfig->randomBotArenaTeams.end(); ++i)
        arenateams.push_back(*i);

    if (arenateams.empty())
    {
        LOG_ERROR("playerbots", "No random arena team available");
        return;
    }

    int index = urand(0, arenateams.size() - 1);
    uint32 arenateamID = arenateams[index];
    ArenaTeam* arenateam = sArenaTeamMgr->GetArenaTeamById(arenateamID);
    if (!arenateam)
    {
        LOG_ERROR("playerbots", "Invalid arena team {}", arenateamID);
        return;
    }

    if (arenateam->GetMembersSize() < ((uint32)arenateam->GetType() * 2) && bot->getLevel() >= 70)
    {
        ObjectGuid capt = arenateam->GetCaptain();
        Player* botcaptain = ObjectAccessor::FindPlayer(capt);

        if ((bot && bot->GetArenaTeamId(arenateam->GetSlot())) || sCharacterCache->GetCharacterArenaTeamIdByGuid(bot->GetGUID(), arenateam->GetSlot()) != 0)
            return;

        if (botcaptain && botcaptain->GetTeamId() == bot->GetTeamId()) //need?
        {
            arenateam->AddMember(bot->GetGUID());
            arenateam->SaveToDB();
        }
    }

    bot->SaveToDB(false, false);
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
      if ((*itr).ClassId == bot->getClass() && (*itr).SpecId == spec) {
        uint32 spellid = (*itr).SpellId;
        uint32 slot =  (*itr).SlotId;
        Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!pItem || !pItem->IsInWorld() || !pItem->GetOwner() || !pItem->GetOwner()->IsInWorld() || !pItem->GetOwner()->GetSession())
            return;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
        if (!spellInfo)
            return;

        uint32 enchantid = spellInfo->Effects[0].MiscValue;
        if (!enchantid)
        {
            // LOG_ERROR("playerbots", "{}: Invalid enchantid ", enchantid, " report to devs", bot->GetName().c_str());
            return;
        }

        if (!((1 << pItem->GetTemplate()->SubClass) & spellInfo->EquippedItemSubClassMask) && !((1 << pItem->GetTemplate()->InventoryType) & spellInfo->EquippedItemInventoryTypeMask))
        {
            // LOG_ERROR("playerbots", "{}: items could not be enchanted, wrong item type equipped", bot->GetName().c_str());
            return;
        }

        bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, false);
        pItem->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);
        bot->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, true);
    }
    // botAI->EnchantItemT((*itr).SpellId, (*itr).SlotId);
}

std::vector<InventoryType> PlayerbotFactory::GetPossibleInventoryTypeListBySlot(EquipmentSlots slot) {
    std::vector<InventoryType> ret;
    switch (slot) {
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
      }
      while (result->NextRow());
   }
}

float PlayerbotFactory::CalculateItemScore(uint32 item_id, Player* bot)
{
    float score = 0;
    int tab = AiFactory::GetPlayerSpecTab(bot);
    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    ItemTemplate const* proto = &itemTemplates->at(item_id);
    uint8 cls = bot->getClass();
    int agility = 0, strength = 0, intellect = 0, spirit = 0;
    int stamina = 0, defense = 0, dodge = 0, parry = 0, block = 0, resilience = 0;
    int hit = 0, crit = 0, haste = 0, expertise = 0, attack_power = 0;
    int mana_regeneration = 0, spell_power = 0, armor_penetration = 0, spell_penetration = 0;
    int armor = 0;
    int itemLevel = proto->ItemLevel;
    int quality = proto->Quality;
    int dps = (proto->Damage[0].DamageMin + proto->Damage[0].DamageMax) / 2 * proto->Delay / 1000;
    armor += proto->Armor;
    block += proto->Block;
    for (int i = 0; i < proto->StatsCount; i++) {
        const _ItemStat &stat = proto->ItemStat[i];
        const int32 &value = stat.ItemStatValue;
        switch (stat.ItemStatType) {
            case ITEM_MOD_AGILITY:
                agility += value;
                break;
            case ITEM_MOD_STRENGTH:
                strength += value;
                break;
            case ITEM_MOD_INTELLECT:
                intellect += value;
                break;
            case ITEM_MOD_SPIRIT:
                spirit += value;
                break;
            case ITEM_MOD_STAMINA:
                stamina += value;
                break;
            case ITEM_MOD_DEFENSE_SKILL_RATING:
                defense += value;
                break;
            case ITEM_MOD_PARRY_RATING:
                parry += value;
                break;
            case ITEM_MOD_BLOCK_RATING:
            case ITEM_MOD_BLOCK_VALUE:
                block += value;
                break;
            case ITEM_MOD_RESILIENCE_RATING:
                resilience += value;
                break;
            case ITEM_MOD_HIT_RATING:
                hit += value;
                break;
            case ITEM_MOD_CRIT_RATING:
                crit += value;
                break;
            case ITEM_MOD_HASTE_RATING:
                haste += value;
                break;
            case ITEM_MOD_EXPERTISE_RATING:
                expertise += value;
                break;
            case ITEM_MOD_ATTACK_POWER:
                attack_power += value;
                break;
            case ITEM_MOD_SPELL_POWER:
                spell_power += value;
                break;
            case ITEM_MOD_MANA_REGENERATION:
                mana_regeneration += value;
                break;
            default:
                break;
        }
    }
    if (cls == CLASS_HUNTER) {
        // AGILITY only
        score = agility * 2.5 + attack_power + armor_penetration * 2 + dps * 5 + hit * 3 + crit * 2 + haste * 2.5 + intellect;
    } else if (cls == CLASS_WARLOCK || 
               cls == CLASS_MAGE || 
               (cls == CLASS_PRIEST && tab == 2) || // shadow
               (cls == CLASS_SHAMAN && tab == 0) || // element
               (cls == CLASS_DRUID && tab == 0) // balance
              ) {
        // SPELL DPS
        score = intellect * 0.5 + spirit * 0.5 + spell_power + spell_penetration 
            + hit * 1.5 + crit * 0.7 + haste * 1;       
    } else if ((cls == CLASS_PALADIN && tab == 0) || // holy
               (cls == CLASS_PRIEST && tab != 2) || // discipline / holy
               (cls == CLASS_SHAMAN && tab == 2) || // heal
               (cls == CLASS_DRUID && tab == 2)
              ) {
        // HEALER
        score = intellect * 0.5 + spirit * 0.5 + spell_power + mana_regeneration * 0.5 + crit * 0.5 + haste * 1;       
    } else if (cls == CLASS_ROGUE) {
        // AGILITY mainly (STRENGTH also)
        score = agility * 2 + strength + attack_power + armor_penetration * 1 + dps * 5 + hit * 2 + crit * 1.5 + haste * 1.5 + expertise * 2.5;
    } else if  ((cls == CLASS_PALADIN && tab == 2) || // retribution
                (cls == CLASS_WARRIOR && tab != 2) || // arm / fury
                (cls == CLASS_DEATH_KNIGHT && tab != 0) // ice / unholy
               ) {
        // STRENGTH mainly (AGILITY also)
        score = strength * 2 + agility + attack_power + armor_penetration + dps * 5 + hit * 1.5 + crit * 1.5 + haste * 1.5 + expertise * 2;
    } else if ((cls == CLASS_SHAMAN && tab == 1)) { // enhancement
        // STRENGTH mainly (AGILITY, INTELLECT also)
        score = strength * 1 + agility * 1.5 + intellect * 1.5 + attack_power + spell_power * 1.5 + armor_penetration * 0.5 + dps * 5
            + hit * 2 + crit * 1.5 + haste * 1.5 + expertise * 2;
    } else if ((cls == CLASS_WARRIOR && tab == 2) || 
               (cls == CLASS_PALADIN && tab == 1)) {
        // TANK WITH SHIELD
        score = strength * 1 + agility * 2 + attack_power * 0.2
            + defense * 2.5 + parry * 2 + dodge * 2 + resilience * 2 + block * 2 + armor * 0.3 + stamina * 3
            + hit * 1 + crit * 0.2 + haste * 0.5 + expertise * 3;
    } else if (cls == CLASS_DEATH_KNIGHT && tab == 0){
        // BLOOD DK TANK
        score = strength * 1 + agility * 2 + attack_power * 0.2
            + defense * 3.5 + parry * 2 + dodge * 2 + resilience * 2 + armor * 0.3 + stamina * 2.5 
            + hit * 2 + crit * 0.5 + haste * 0.5 + expertise * 3.5;
    } else {
        // BEAR DRUID TANK (AND FERAL DRUID...?)
        score = agility * 1.5 + strength * 1 + attack_power * 0.5 + armor_penetration * 0.5 + dps * 2
            + defense * 0.25 + dodge * 0.25 + armor * 0.3 + stamina * 1.5
            + hit * 1 + crit * 1 + haste * 0.5 + expertise * 3;
    }
    // penalty for different type armor
    if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass >= ITEM_SUBCLASS_ARMOR_CLOTH &&
        proto->SubClass <= ITEM_SUBCLASS_ARMOR_PLATE && NotSameArmorType(proto->SubClass, bot))
    {
        score *= 0.8;
    }
    // double hand
    if (proto->Class == ITEM_CLASS_WEAPON) {
        bool isDoubleHand = proto->Class == ITEM_CLASS_WEAPON && 
            !(ITEM_SUBCLASS_MASK_SINGLE_HAND & (1 << proto->SubClass)) && 
            !(ITEM_SUBCLASS_MASK_WEAPON_RANGED & (1 << proto->SubClass));

        if (isDoubleHand) {
            score *= 0.5;
        }
        // spec without double hand
        // enhancement, rogue, ice dk, shield tank, fury warrior without titan's grip but with duel wield
        if (isDoubleHand && 
            ((cls == CLASS_SHAMAN && tab == 1 && bot->HasSpell(674)) ||
            (cls == CLASS_ROGUE) ||
            (cls == CLASS_DEATH_KNIGHT && tab == 1) ||
            (cls == CLASS_WARRIOR && tab == 1 && !bot->HasAura(49152) && bot->HasSpell(674)) ||
            IsShieldTank(bot))) {
                score *= 0.1;
        }
        // spec with double hand
        // fury with titan's grip, fury without duel wield, arms, bear, retribution, blood dk
        if (isDoubleHand && 
            ((cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && bot->HasAura(49152)) ||
            (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_FURY && !bot->HasSpell(674)) ||
            (cls == CLASS_WARRIOR && tab == WARRIOR_TAB_ARMS) ||
            (cls == CLASS_DRUID && tab == 1) ||
            (cls == CLASS_PALADIN && tab == 2) ||
            (cls == CLASS_DEATH_KNIGHT && tab == 0) ||
            (cls == CLASS_SHAMAN && tab == 1 && !bot->HasSpell(674)))) {
            score *= 10;
        }
    }
    if (proto->Class == ITEM_CLASS_WEAPON) {
        if (cls == CLASS_HUNTER && proto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN) {
            score *= 0.1;
        }
        if (cls == CLASS_ROGUE && tab == ROGUE_TAB_ASSASSINATION && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER) {
            score *= 0.1;
        }
    }
    return (0.0001 + score) * itemLevel * (quality + 1);
}

bool PlayerbotFactory::IsShieldTank(Player* bot) 
{
    int tab = AiFactory::GetPlayerSpecTab(bot);
    return (bot->getClass() == CLASS_WARRIOR && tab == 2) || (bot->getClass() == CLASS_PALADIN && tab == 1); 
}

bool PlayerbotFactory::NotSameArmorType(uint32 item_subclass_armor, Player* bot) 
{
    if (bot->HasSkill(SKILL_PLATE_MAIL)) {
        return item_subclass_armor != ITEM_SUBCLASS_ARMOR_PLATE;
    }
    if (bot->HasSkill(SKILL_MAIL)) {
        return item_subclass_armor != ITEM_SUBCLASS_ARMOR_MAIL;
    }
    if (bot->HasSkill(SKILL_LEATHER)) {
        return item_subclass_armor != ITEM_SUBCLASS_ARMOR_LEATHER;
    }
    return false;
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
        if (Item *pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    for (uint32 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        if (Item *pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag *pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for(uint32 j = 0; j < pBag->GetBagSize(); ++j)
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
        if(!pItem)
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
                        if(!pItem)
                            continue;

                        if (!visitor->Visit(pItem))
                            return;
                    }
                }
            }
        }
    }
}