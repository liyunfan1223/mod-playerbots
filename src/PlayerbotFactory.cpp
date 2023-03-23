/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotFactory.h"
#include "AccountMgr.h"
#include "AiFactory.h"
#include "ArenaTeamMgr.h"
#include "GuildMgr.h"
#include "MapMgr.h"
#include "Playerbots.h"
#include "PerformanceMonitor.h"
#include "PlayerbotDbStore.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "ItemVisitors.h"
#include "InventoryAction.h"

#include <random>

#define PLAYER_SKILL_INDEX(x)       (PLAYER_SKILL_INFO_1_1 + ((x)*3))

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

PlayerbotFactory::PlayerbotFactory(Player* bot, uint32 level, uint32 itemQuality) : level(level), itemQuality(itemQuality), InventoryAction(GET_PLAYERBOT_AI(bot), "factory")
{
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
        if (level < 20)
            itemQuality = urand(ITEM_QUALITY_NORMAL, ITEM_QUALITY_UNCOMMON);
        else if (level < 40)
            itemQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_RARE);
        else if (level < 60)
            itemQuality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_EPIC);
        else if (level < 70)
            itemQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
        else if (level < 80)
            itemQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
        else
            itemQuality = urand(ITEM_QUALITY_RARE, ITEM_QUALITY_EPIC);
    }

    if (bot->isDead())
        bot->ResurrectPlayer(1.0f, false);

    bot->CombatStop(true);

    if (!sPlayerbotAIConfig->disableRandomLevels)
    {
        bot->SetLevel(level);
    }
    else if (bot->getLevel() < sPlayerbotAIConfig->randombotStartingLevel)
    {
        bot->SetLevel(sPlayerbotAIConfig->randombotStartingLevel);
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
    if (sPlayerbotAIConfig->disableRandomLevels)
    {
        return;
    }

    LOG_INFO("playerbots", "Preparing to {} randomize...", (incremental ? "incremental" : "full"));
    Prepare();

    LOG_INFO("playerbots", "Resetting player...");
    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Reset");
    bot->resetTalents(true);
    ClearSkills();
    ClearSpells();

    if (!incremental)
    {
        ClearInventory();
        ResetQuests();
    }

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

    if (sPlayerbotAIConfig->randomBotPreQuests)
    {
        LOG_INFO("playerbots", "Initializing quests...");
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Quests");
        InitQuests(classQuestIds);
        InitQuests(specialQuestIds);

        // quest rewards boost bot level, so reduce back
        if (!sPlayerbotAIConfig->disableRandomLevels)
        {
            bot->SetLevel(level);
        }
        else if (bot->getLevel() < sPlayerbotAIConfig->randombotStartingLevel)
        {
            bot->SetLevel(sPlayerbotAIConfig->randombotStartingLevel);
        }

        ClearInventory();
        bot->SetUInt32Value(PLAYER_XP, 0);
        CancelAuras();
        bot->SaveToDB(false, false);
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells1");
    LOG_INFO("playerbots", "Initializing spells (step 1)...");
    InitAvailableSpells();
    if (pmo)
        pmo->finish();

    LOG_INFO("playerbots", "Initializing skills (step 1)...");
    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills1");
    InitSkills();
    InitTradeSkills();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Talents");
    LOG_INFO("playerbots", "Initializing talents...");
    //InitTalentsTree(incremental);
    //sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", 0);
    botAI->DoSpecificAction("auto talents");
    sPlayerbotDbStore->Reset(botAI);
    botAI->ResetStrategies(false); // fix wrong stored strategy
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells2");
    LOG_INFO("playerbots", "Initializing spells (step 2)...");
    InitAvailableSpells();
    InitSpecialSpells();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Mounts");
    LOG_INFO("playerbots", "Initializing mounts...");
    InitMounts();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills2");
    LOG_INFO("playerbots", "Initializing skills (step 2)...");
    UpdateTradeSkills();
    bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Equip");
    LOG_INFO("playerbots", "Initializing equipmemt...");
    InitEquipment(incremental);
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
    bot->SaveToDB(false, false);
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
    InitSecondEquipmentSet();
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
    InitInventory();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Consumable");
    LOG_INFO("playerbots", "Initializing consumables...");
    AddConsumables();
    if (pmo)
        pmo->finish();

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Guilds");
    LOG_INFO("playerbots", "Initializing guilds...");
    bot->SaveToDB(false, false); //thesawolf - save save save (hopefully avoids dupes)
    InitGuild();
    if (pmo)
        pmo->finish();

    if (bot->getLevel() >= 70)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Arenas");
        LOG_INFO("playerbots", "Initializing arena teams...");
        InitArenaTeam();
        if (pmo)
            pmo->finish();
    }

    if (bot->getLevel() >= 10)
    {
        pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Pet");
        LOG_INFO("playerbots", "Initializing pet...");
        InitPet();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "PlayerbotFactory_Save");
    LOG_INFO("playerbots", "Saving to DB...");
    if (incremental)
    {
        uint32 money = bot->GetMoney();
        bot->SetMoney(money + 1000 * sqrt(urand(1, level * 5)));
    }
    else
    {
        bot->SetMoney(10000 * sqrt(urand(1, level * 5)));
    }
    bot->SaveToDB(false, false);
    LOG_INFO("playerbots", "Done.");
    if (pmo)
        pmo->finish();
}

void PlayerbotFactory::Refresh()
{
    Prepare();
    InitAmmo();
    InitFood();
    InitPotions();
    InitReagents();
    //bot->SaveToDB();
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

void PlayerbotFactory::InitPet()
{
    Pet* pet = bot->GetPet();
    if (!pet)
    {
        if (bot->getClass() != CLASS_HUNTER)
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

		for (uint32 i = 0; i < 100; i++)
		{
			uint32 index = urand(0, ids.size() - 1);
            CreatureTemplate const* co = sObjectMgr->GetCreatureTemplate(ids[index]);
            if (!co)
                continue;

            uint32 guid = map->GenerateLowGuid<HighGuid::Pet>();
            uint32 pet_number = sObjectMgr->GeneratePetNumber();
            pet = new Pet(bot, HUNTER_PET);
            if (!pet->Create(guid, bot->GetMap(), bot->GetPhaseMask(), co->Entry, pet_number))
            {
                delete pet;
                pet = nullptr;
                continue;
            }

            pet->Relocate(bot);
            pet->SetOwnerGUID(bot->GetGUID());
            pet->SetGuidValue(UNIT_FIELD_CREATEDBY, bot->GetGUID());
            pet->SetFaction(bot->GetFaction());
            pet->SetLevel(bot->getLevel());
            pet->InitStatsForLevel(bot->getLevel());
            pet->SetPower(POWER_HAPPINESS, HAPPINESS_LEVEL_SIZE * 2);
            pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
            pet->GetMap()->AddToMap(pet->ToCreature());
            pet->InitPetCreateSpells();
            pet->LearnPetPassives();
            pet->CastPetAuras(true);
            pet->UpdateAllStats();

            PetStable& petStable = bot->GetOrInitPetStable();
            pet->FillPetInfo(&petStable.CurrentPet.emplace());

            LOG_DEBUG("playerbots",   "Bot {}: assign pet {} ({} level)", bot->GetName().c_str(), co->Entry, bot->getLevel());
            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            bot->PetSpellInitialize();
            break;
        }
    }

    if (pet)
    {
        pet->InitStatsForLevel(bot->getLevel());
        pet->SetLevel(bot->getLevel());
        pet->SetPower(POWER_HAPPINESS, HAPPINESS_LEVEL_SIZE * 2);
        pet->SetHealth(pet->GetMaxHealth());
    }
    else
    {
        LOG_ERROR("playerbots", "Cannot create pet for bot {}", bot->GetName().c_str());
        return;
    }

    for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

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
}

void PlayerbotFactory::ClearSpells()
{
    bot->resetSpells();
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

void PlayerbotFactory::InitTalentsTree(bool incremental)
{
    uint32 specNo = sRandomPlayerbotMgr->GetValue(bot->GetGUID().GetCounter(), "specNo");
    if (incremental && specNo)
    {
        specNo -= 1;
    }
    else
    {
        uint32 point = urand(0, 100);
        uint8 cls = bot->getClass();
        uint32 p1 = sPlayerbotAIConfig->specProbability[cls][0];
        uint32 p2 = p1 + sPlayerbotAIConfig->specProbability[cls][1];

        specNo = (point < p1 ? 0 : (point < p2 ? 1 : 2));
        sRandomPlayerbotMgr->SetValue(bot, "specNo", specNo + 1);
    }

    InitTalents(specNo);

    if (bot->GetFreeTalentPoints())
    {
        InitTalents(2 - specNo);
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
    if (bot->HasSkill(SKILL_SHIELD) && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        return true;

    if (bot->HasSkill(SKILL_PLATE_MAIL))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_PLATE)
            return false;
    }
    else if (bot->HasSkill(SKILL_MAIL))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_MAIL)
            return false;
    }
    else if (bot->HasSkill(SKILL_LEATHER))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_LEATHER)
            return false;
    }

    if (proto->Quality <= ITEM_QUALITY_NORMAL)
        return true;

    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;

        if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
            continue;

        if (slot == EQUIPMENT_SLOT_OFFHAND && bot->getClass() == CLASS_PALADIN && proto->SubClass != ITEM_SUBCLASS_ARMOR_SHIELD)
            continue;
    }

    uint8 sp = 0;
    uint8 ap = 0;
    uint8 tank = 0;
    for (uint8 j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        // for ItemStatValue != 0
        if(!proto->ItemStat[j].ItemStatValue)
            continue;

        AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    }

    return CheckItemStats(sp, ap, tank);
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
    uint8 tab = AiFactory::GetPlayerSpecTab(bot);

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE)
                return false;
            break;
        case CLASS_MAGE:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND)
                return false;
            break;
        case CLASS_WARLOCK:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_WARRIOR:
            if (tab == 1) //fury
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    return false;
            }
            if ((tab == 0) && (bot->getLevel() > 10)) // arms
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&  proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    return false;
            }
            else // prot + lowlvl
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    return false;
            }
            break;
        case CLASS_PALADIN:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                return false;
            break;
        case CLASS_SHAMAN:
            if (tab == 1) //enh
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2)
                    return false;
            }
            else //ele,resto
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                    return false;
            }
            break;
        case CLASS_DRUID:
            if (tab == 1) // feral
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                    return false;
            }
            else // balance, resto
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
                    return false;
            }
            break;
        case CLASS_HUNTER:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW)
                return false;
            break;
        case CLASS_ROGUE:
            if (tab == 0) // assa
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    return false;
            }
            else
            {
                if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN && proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                    proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW && proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
                    return false;
            }
            break;
        case CLASS_DEATH_KNIGHT:
            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2)
                return false;
            break;
    }

    return true;
}

bool PlayerbotFactory::CanEquipItem(ItemTemplate const* proto, uint32 desiredQuality)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Quality != desiredQuality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
    {
        requiredLevel = sRandomItemMgr->GetMinLevelFromCache(proto->ItemId);
    }

    if (!requiredLevel)
        return false;

    uint32 level = bot->getLevel();
    uint32 delta = 2;
    if (level < 15)
        delta = urand(7, 15);
    else if (proto->Class == ITEM_CLASS_WEAPON || proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        delta = urand(2, 3);
    else if (!(level % 10) || (level % 10) == 9)
        delta = 2;
    else if (level < 40)
        delta = urand(5, 10);
    else if (level < 60)
        delta = urand(3, 7);
    else if (level < 70)
        delta = urand(2, 5);
    else if (level < 80)
        delta = urand(2, 4);

    if (desiredQuality > ITEM_QUALITY_NORMAL && (requiredLevel > level || requiredLevel < level - delta))
        return false;

    for (uint32 gap = 60; gap <= 80; gap += 10)
    {
        if (level > gap && requiredLevel <= gap)
            return false;
    }

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

void PlayerbotFactory::InitEquipmentNew(bool incremental)
{
    if (incremental)
    {
        DestroyItemsVisitor visitor(bot);
        IterateItems(&visitor, (IterateItemsMask)(ITERATE_ITEMS_IN_BAGS | ITERATE_ITEMS_IN_BANK));
    }
    else
    {
    DestroyItemsVisitor visitor(bot);
    IterateItems(&visitor, ITERATE_ALL_ITEMS);
    }

    std::string const& specName = AiFactory::GetPlayerSpecName(bot);
    if (specName.empty())
        return;

    // look for upgrades
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD && !bot->GetGuildId())
            continue;

        bool isUpgrade = false;
        bool found = false;
        bool noItem = false;
        uint32 quality = urand(ITEM_QUALITY_UNCOMMON, ITEM_QUALITY_EPIC);
        uint32 attempts = 10;
        if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && quality > ITEM_QUALITY_NORMAL) {
            quality--;
        }
        // current item;
        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (oldItem)
            isUpgrade = true;

        uint32 itemInSlot = isUpgrade ? oldItem->GetTemplate()->ItemId : 0;

        uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
        if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

        uint32 minLevel = sPlayerbotAIConfig->randomBotMinLevel;
        if (minLevel < sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL))
            minLevel = sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL);

        // test
        do
        {
            if (isUpgrade)
            {
                std::vector<uint32> ids = sRandomItemMgr->GetUpgradeList(bot, specName, slot, 0, itemInSlot);
                if (!ids.empty())
                    Shuffle(ids);

                for (uint32 index = 0; index < ids.size(); ++index)
                {
                    uint32 newItemId = ids[index];
                    if (incremental && !IsDesiredReplacement(oldItem))
                    {
                        continue;
                    }

                    uint16 dest;
                    if (!CanEquipUnseenItem(slot, dest, newItemId))
                        continue;

                    if (oldItem)
                    {
                        bot->RemoveItem(INVENTORY_SLOT_BAG_0, slot, true);
                        oldItem->DestroyForPlayer(bot);
                    }

                    Item* newItem = bot->EquipNewItem(dest, newItemId, true);
                    if (newItem)
                    {
                        newItem->AddToWorld();
                        newItem->AddToUpdateQueueOf(bot);
                        bot->AutoUnequipOffhandIfNeed();
                        newItem->SetOwnerGUID(bot->GetGUID());
                        EnchantItem(newItem);
                        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Equip: {}, slot: {}, Old item: {}",
                            bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->getLevel(), bot->GetName(), newItemId, slot, itemInSlot);
                        found = true;
                        break;
                    }
                }
            }
            else
            {
                std::vector<uint32> ids = sRandomItemMgr->GetUpgradeList(bot, specName, slot, quality, itemInSlot);
                if (!ids.empty())
                    Shuffle(ids);

                for (uint32 index = 0; index < ids.size(); ++index)
                {
                    uint32 newItemId = ids[index];
                    uint16 dest;
                    if (!CanEquipUnseenItem(slot, dest, newItemId))
                        continue;

                    Item* newItem = bot->EquipNewItem(dest, newItemId, true);
                    if (newItem)
                    {
                        bot->AutoUnequipOffhandIfNeed();
                        newItem->SetOwnerGUID(bot->GetGUID());
                        EnchantItem(newItem);
                        found = true;
                        LOG_INFO("playerbots", "Bot {} {}:{} <{}>: Equip: {}, slot: {}",
                            bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->getLevel(), bot->GetName(), newItemId, slot);
                        break;
                    }
                }
            }
            quality--;
        } while (!found && quality != ITEM_QUALITY_POOR);
        if (!found)
        {
            LOG_INFO("playerbots", "Bot {} {}:{} <{}>: no item for slot {}",
                bot->GetGUID().ToString().c_str(), IsAlliance(bot->getRace()) ? "A" : "H", bot->getLevel(), bot->GetName(), slot);
            continue;
        }
    }
}

void PlayerbotFactory::InitEquipment(bool incremental)
{
    //DestroyItemsVisitor visitor(bot);
    //IterateItems(&visitor, ITERATE_ALL_ITEMS);

    if (incremental)
    {
        //DestroyItemsVisitor visitor(bot);
        //IterateItems(&visitor, (IterateItemsMask)(ITERATE_ITEMS_IN_BAGS | ITERATE_ITEMS_IN_BANK));
    }
    else
    {
        DestroyItemsVisitor visitor(bot);
        IterateItems(&visitor, ITERATE_ITEMS_IN_EQUIP);
    }

    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD && !bot->GetGuildId())
            continue;

        bool found = false;
        uint32 quality = itemQuality;
        if (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && quality > ITEM_QUALITY_NORMAL)
        {
            quality--;
        }

        do
        {
            std::vector<uint32> ids = sRandomItemMgr->Query(level, bot->getClass(), slot, quality);
            if (!ids.empty())
            {
                Acore::Containers::RandomShuffle(ids);
                std::random_device rd;
            }

            for (uint32 index = 0; index < ids.size(); ++index)
            {
                uint32 newItemId = ids[index];
                Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

                if (incremental && !IsDesiredReplacement(oldItem))
                {
                    continue;
                }

                if (sRandomItemMgr->HasStatWeight(newItemId) || (oldItem && sRandomItemMgr->HasStatWeight(oldItem->GetTemplate()->ItemId)))
                {
                    if (!sRandomItemMgr->GetLiveStatWeight(bot, newItemId))
                        continue;

                    if (oldItem)
                    {
                        if (sRandomItemMgr->GetLiveStatWeight(bot, newItemId) < sRandomItemMgr->GetLiveStatWeight(bot, oldItem->GetTemplate()->ItemId))
                            continue;
                    }
                }

                uint16 dest;
                if (!CanEquipUnseenItem(slot, dest, newItemId))
                    continue;

                if (oldItem)
                {
                    bot->RemoveItem(INVENTORY_SLOT_BAG_0, slot, true);
                    oldItem->DestroyForPlayer(bot);
                }

                if (Item* newItem = bot->EquipNewItem(dest, newItemId, true))
                {
                    newItem->AddToWorld();
                    newItem->AddToUpdateQueueOf(bot);
                    bot->AutoUnequipOffhandIfNeed();
                    newItem->SetOwnerGUID(bot->GetGUID());
                    EnchantItem(newItem);
                    found = true;
                    break;
                }
            }

            quality--;
        } while (!found && quality != ITEM_QUALITY_POOR);

        if (!found)
        {
            LOG_INFO("playerbots",   "{}: no items to equip for slot {}", bot->GetName().c_str(), slot);
            continue;
        }
    }

    // Update stats here so the bots will benefit from the new equipped items' stats
    bot->InitStatsForLevel(true);
    bot->UpdateAllStats();
}

bool PlayerbotFactory::IsDesiredReplacement(Item* item)
{
    if (!item)
        return true;

    ItemTemplate const* proto = item->GetTemplate();
    uint32 requiredLevel = proto->RequiredLevel;
    if (!requiredLevel)
    {
        requiredLevel = sRandomItemMgr->GetMinLevelFromCache(proto->ItemId);
    }

    uint32 delta = 1 + (80 - bot->getLevel()) / 10;
    return int32(bot->getLevel() - requiredLevel) > delta;
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

void PlayerbotFactory::InitSecondEquipmentSet()
{
    if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_PRIEST)
        return;

    std::map<uint32, std::vector<uint32>> items;

    uint32 desiredQuality = itemQuality;
    while (urand(0, 100) < 100 * sPlayerbotAIConfig->randomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL)
    {
        desiredQuality--;
    }

    ItemTemplateContainer const* itemTemplate = sObjectMgr->GetItemTemplateStore();
    do
    {
        for (auto const& itr : *itemTemplate)
        {
            ItemTemplate const* proto = &itr.second;
            if (!proto)
                continue;

            if (!CanEquipItem(proto, desiredQuality))
                continue;

            if (proto->Class == ITEM_CLASS_WEAPON)
            {
                //if (!CanEquipWeapon(proto))
                //    continue;

                if (sRandomItemMgr->HasStatWeight(proto->ItemId))
                {
                    if (!sRandomItemMgr->GetLiveStatWeight(bot, proto->ItemId))
                        continue;
                }

                Item* existingItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (existingItem)
                {
                    switch (existingItem->GetTemplate()->SubClass)
                    {
                        case ITEM_SUBCLASS_WEAPON_AXE:
                        case ITEM_SUBCLASS_WEAPON_DAGGER:
                        case ITEM_SUBCLASS_WEAPON_FIST:
                        case ITEM_SUBCLASS_WEAPON_MACE:
                        case ITEM_SUBCLASS_WEAPON_SWORD:
                            if (proto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || proto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER || proto->SubClass == ITEM_SUBCLASS_WEAPON_FIST ||
                                proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || proto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD)
                                continue;
                            break;
                        default:
                            if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE && proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER && proto->SubClass != ITEM_SUBCLASS_WEAPON_FIST &&
                                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE && proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
                                continue;
                            break;
                    }
                }
            }
            else if (proto->Class == ITEM_CLASS_ARMOR && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
            {
                //if (!CanEquipArmor(proto))
                //    continue;

                if (sRandomItemMgr->HasStatWeight(proto->ItemId))
                {
                    if (!sRandomItemMgr->GetLiveStatWeight(bot, proto->ItemId))
                        continue;
                }

                if (Item* existingItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                    if (existingItem->GetTemplate()->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
                        continue;
            }
            else
                continue;

            items[proto->Class].push_back(itr.first);
        }
    } while (items[ITEM_CLASS_ARMOR].empty() && items[ITEM_CLASS_WEAPON].empty() && desiredQuality-- > ITEM_QUALITY_NORMAL);

    for (std::map<uint32, std::vector<uint32>>::iterator i = items.begin(); i != items.end(); ++i)
    {
        std::vector<uint32>& ids = i->second;
        if (ids.empty())
        {
            LOG_DEBUG("playerbots",   "{}: no items to make second equipment set for slot {}", bot->GetName().c_str(), i->first);
            continue;
        }

        for (uint32 attempts = 0; attempts < 15; attempts++)
        {
            uint32 index = urand(0, ids.size() - 1);
            uint32 newItemId = ids[index];

            if (Item* newItem = StoreNewItemInInventorySlot(bot, newItemId, 1))
            {
                newItem->SetOwnerGUID(bot->GetGUID());
                EnchantItem(newItem);
                newItem->AddToWorld();
                newItem->AddToUpdateQueueOf(bot);
                break;
            }
        }
    }
}

void PlayerbotFactory::InitBags()
{
    std::vector<uint32> ids;

    ItemTemplateContainer const* itemTemplates = sObjectMgr->GetItemTemplateStore();
    for (auto const& itr : *itemTemplates)
    {
        ItemTemplate const* proto = &itr.second;
        if (!proto || proto->Class != ITEM_CLASS_CONTAINER)
            continue;

        if (!CanEquipItem(proto, ITEM_QUALITY_NORMAL))
            continue;

        ids.push_back(itr.first);
    }

    if (ids.empty())
    {
        LOG_ERROR("playerbots", "{}: no bags found", bot->GetName().c_str());
        return;
    }

    for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        for (uint32 attempts = 0; attempts < 15; attempts++)
        {
            uint32 index = urand(0, ids.size() - 1);
            uint32 newItemId = ids[index];

            uint16 dest;
            if (!CanEquipUnseenItem(slot, dest, newItemId))
                continue;

            if (Item* newItem = bot->EquipNewItem(dest, newItemId, true))
            {
                newItem->AddToWorld();
                newItem->AddToUpdateQueueOf(bot);
                break;
            }
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

    if (Item* pItem = Item::CreateItem(item, 1, bot))
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
            break;
        case CLASS_PALADIN:
            bot->SetSkill(SKILL_PLATE_MAIL, 0, skillLevel, skillLevel);
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_2H_SWORDS);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_POLEARMS);
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
            break;
        case CLASS_DEATH_KNIGHT:
            SetRandomSkill(SKILL_SWORDS);
            SetRandomSkill(SKILL_AXES);
            SetRandomSkill(SKILL_MACES);
            SetRandomSkill(SKILL_2H_SWORDS);
            SetRandomSkill(SKILL_2H_MACES);
            SetRandomSkill(SKILL_2H_AXES);
            SetRandomSkill(SKILL_POLEARMS);
            break;
        default:
            break;
    }

    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
            bot->SetSkill(SKILL_PLATE_MAIL, 1, skillLevel, skillLevel);
            break;
        case CLASS_SHAMAN:
        case CLASS_HUNTER:
            bot->SetSkill(SKILL_MAIL, 1, skillLevel, skillLevel);
            break;
        default:
            break;
    }
}

void PlayerbotFactory::SetRandomSkill(uint16 id)
{
    uint32 maxValue = level * 5;

    // do not let skill go beyond limit even if maxlevel > blizzlike
    if (level > 60)
    {
        maxValue = (level + 10) * 5;
    }

    uint32 value = urand(maxValue - level, maxValue);
    uint32 curValue = bot->GetSkillValue(id);

    uint16 step = bot->GetSkillValue(id) ? bot->GetSkillStep(id) : 1;

    if (!bot->HasSkill(id) || value > curValue)
        bot->SetSkill(id, step, value, maxValue);
}

void PlayerbotFactory::InitAvailableSpells()
{
    bot->LearnDefaultSkills();
    bot->LearnCustomSpells();
    bot->learnQuestRewardedSpells();

    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        if (itr->second.trainer_type != TRAINER_TYPE_TRADESKILLS && itr->second.trainer_type != TRAINER_TYPE_CLASS)
            continue;

        if (itr->second.trainer_type == TRAINER_TYPE_CLASS && itr->second.trainer_class != bot->getClass())
            continue;

        TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(itr->second.Entry);
        if (!trainer_spells)
            continue;

		for (TrainerSpellMap::const_iterator iter = trainer_spells->spellList.begin(); iter != trainer_spells->spellList.end(); ++iter)
		{
			TrainerSpell const* tSpell = &iter->second;
			if (!tSpell)
				continue;

			TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
			if (state != TRAINER_SPELL_GREEN)
				continue;

		    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(tSpell->spell);
		    if (!spellInfo)
		        continue;

            if (itr->second.trainer_type == TRAINER_TYPE_TRADESKILLS)
            {
                std::string const SpellName = spellInfo->SpellName[0];
                if (spellInfo->Effects[EFFECT_1].Effect == SPELL_EFFECT_SKILL_STEP)
                {
                    uint32 skill = spellInfo->Effects[EFFECT_1].MiscValue;
                    if (skill && !bot->HasSkill(skill))
                    {
                        SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
                        if (pSkill)
                        {
                            if (SpellName.find("Apprentice") != std::string::npos && pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY)
                                continue;
                        }
                    }
                }
            }

            bool learned = false;
            for (uint8 j = 0; j < 3; ++j)
            {
                if (!tSpell->learnedSpell[j] && !bot->IsSpellFitByClassAndRace(tSpell->learnedSpell[j]))
                    continue;

                if (spellInfo->Effects[j].Effect == SPELL_EFFECT_LEARN_SPELL)
                {
                    uint32 learnedSpell = spellInfo->Effects[j].TriggerSpell;
                    bot->learnSpell(learnedSpell);
                    learned = true;
                }
            }

            if (!learned)
                bot->learnSpell(tSpell->spell);
		}
    }
    if (bot->IsSpellFitByClassAndRace(20271) && !bot->HasSpell(20271)) // judgement missing
        bot->learnSpell(20271, false);
}

void PlayerbotFactory::InitSpecialSpells()
{
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotSpellIds.begin(); i != sPlayerbotAIConfig->randomBotSpellIds.end(); ++i)
    {
        uint32 spellId = *i;
        bot->learnSpell(spellId);
    }
}

void PlayerbotFactory::InitTalents(uint32 specNo)
{
    uint32 classMask = bot->getClassMask();

    std::map<uint32, std::vector<TalentEntry const*> > spells;
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
    for (std::map<uint32, std::vector<TalentEntry const*>>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        std::vector<TalentEntry const*>& spells = i->second;
        if (spells.empty())
        {
            LOG_ERROR("playerbots", "{}: No spells for talent row {}", bot->GetName().c_str(), i->first);
            continue;
        }

        uint32 attemptCount = 0;
        while (!spells.empty() && (int)freePoints - (int)bot->GetFreeTalentPoints() < 5 && attemptCount++ < 3 && bot->GetFreeTalentPoints())
        {
            uint32 index = urand(0, spells.size() - 1);
            TalentEntry const* talentInfo = spells[index];
            for (uint32 rank = 0; rank < MAX_TALENT_RANK && bot->GetFreeTalentPoints(); ++rank)
            {
                uint32 spellId = talentInfo->RankID[rank];
                if (!spellId)
                    continue;

                bot->learnSpell(spellId);

                if (uint32 talentCost = GetTalentSpellCost(spellId))
                {
                    uint32 free_points = freePoints - talentCost;
                    bot->SetFreeTalentPoints(free_points > 0 ? free_points : 0);
                }
            }
            spells.erase(spells.begin() + index);
        }

        freePoints = bot->GetFreeTalentPoints();
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
    }

    if (!subClass)
        return;

    uint32 entry = bot->GetUInt32Value(PLAYER_AMMO_ID);
    uint32 count = bot->GetItemCount(entry) / 200;
    uint32 maxCount = 5 + level / 10;

    if (!entry || count <= 2)
    {
        entry = sRandomItemMgr->GetAmmo(level, subClass);
    }

    if (count < maxCount)
    {
        for (uint32 i = 0; i < maxCount - count; i++)
        {
            if (Item* newItem = StoreNewItemInInventorySlot(bot, entry, 200))
                newItem->AddToUpdateQueueOf(bot);
        }
    }

    bot->SetAmmo(entry);
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
            LOG_INFO("playerbots", "Bot {} ({}) learned {} mount {}", bot->GetGUID().ToString().c_str(), bot->getLevel(), type == 0 ? "slow" : (type == 1 ? "fast" : "flying"), spell);
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
    uint32 categories[] = { 11, 59 };
    for (uint8 i = 0; i < 2; ++i)
    {
        uint32 category = categories[i];

        FindFoodVisitor visitor(bot, category);
        IterateItems(&visitor);
        if (!visitor.GetResult().empty())
            continue;

        uint32 itemId = sRandomItemMgr->GetFood(level, category);
        if (!itemId)
        {
            LOG_INFO("playerbots", "No food (category {}) available for bot {} ({} level)", category, bot->GetName().c_str(), bot->getLevel());
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

void PlayerbotFactory::InitReagents()
{
    std::vector<uint32> items;
    uint32 regCount = 1;
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            regCount = 2;
            if (bot->getLevel() > 11)
                items = { 17056 };
            if (bot->getLevel() > 19)
                items = { 17056, 17031 };
            if (bot->getLevel() > 35)
                items = { 17056, 17031, 17032 };
            if (bot->getLevel() > 55)
                items = { 17056, 17031, 17032, 17020 };
            break;
        case CLASS_DRUID:
            regCount = 2;
            if (bot->getLevel() > 19)
                items = { 17034 };
            if (bot->getLevel() > 29)
                items = { 17035 };
            if (bot->getLevel() > 39)
                items = { 17036 };
            if (bot->getLevel() > 49)
                items = { 17037, 17021 };
            if (bot->getLevel() > 59)
                items = { 17038, 17026 };
            if (bot->getLevel() > 69)
                items = { 22147, 22148 };
            break;
        case CLASS_PALADIN:
            regCount = 3;
            if (bot->getLevel() > 50)
                items = { 21177 };
            break;
        case CLASS_SHAMAN:
            regCount = 1;
            if (bot->getLevel() > 22)
                items = { 17057 };
            if (bot->getLevel() > 28)
                items = { 17057, 17058 };
            if (bot->getLevel() > 29)
                items = { 17057, 17058, 17030 };
            break;
        case CLASS_WARLOCK:
            regCount = 10;
            if (bot->getLevel() > 9)
                items = { 6265 };
            break;
        case CLASS_PRIEST:
            regCount = 3;
            if (bot->getLevel() > 48)
                items = { 17028 };
            if (bot->getLevel() > 55)
                items = { 17028, 17029 };
            break;
        case CLASS_ROGUE:
            regCount = 1;
            if (bot->getLevel() > 21)
                items = { 5140 };
            if (bot->getLevel() > 33)
                items = { 5140, 5530 };
            break;
    }

    /*for (uint32 itemID : items)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemID);
        if (!proto)
        {
            LOG_ERROR("playerbots", "No reagent (ItemId {}) found for bot {} (Class:{})", i, bot->GetGUID().ToString().c_str(), bot->getClass());
            continue;
        }

        uint32 maxCount = proto->GetMaxStackSize();

        QueryItemCountVisitor visitor(itemID);
        IterateItems(&visitor);
        if (visitor.GetCount() > maxCount) continue;

        uint32 randCount = urand(maxCount / 2, maxCount * regCount);

        Item* newItem = bot->StoreNewItemInInventorySlot(*i, randCount);
        if (newItem)
            newItem->AddToUpdateQueueOf(bot);

        LOG_INFO("playerbots", "Bot {} got reagent {} x{}", bot->GetGUID().ToString().c_str(), proto->Name1.c_str(), randCount);
    }*/

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (itr->second->State == PLAYERSPELL_REMOVED || itr->second->Active || spellInfo->IsPassive())
            continue;

        if (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL)
            continue;

        for (const auto& reagent : spellInfo->Reagent)
        {
            if (reagent)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(reagent);
                if (!proto)
                {
                    LOG_ERROR("playerbots", "No reagent (ItemId {}) found for bot {} (Class:{})", reagent, bot->GetGUID().ToString().c_str(), bot->getClass());
                    continue;
                }

                uint32 maxCount = proto->GetMaxStackSize();

                QueryItemCountVisitor visitor(reagent);
                IterateItems(&visitor);
                if (visitor.GetCount() > maxCount) continue;

                uint32 randCount = urand(maxCount / 2, maxCount * regCount);

                Item* newItem = StoreNewItemInInventorySlot(bot, reagent, randCount);
                if (newItem)
                    newItem->AddToUpdateQueueOf(bot);

                LOG_INFO("playerbots", "Bot {} got reagent {} x{}", bot->GetGUID().ToString().c_str(), proto->Name1.c_str(), randCount);
            }
        }

        for (const auto& totem : spellInfo->Totem)
        {
            if (totem && !bot->HasItemCount(totem, 1))
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(totem);
                if (!proto)
                {
                    LOG_ERROR("playerbots", "No totem (ItemId {}) found for bot {} (Class:{})", totem, bot->GetGUID().ToString().c_str(), bot->getClass());
                    continue;
                }

                Item* newItem = StoreNewItemInInventorySlot(bot, totem, 1);
                if (newItem)
                    newItem->AddToUpdateQueueOf(bot);

                LOG_INFO("playerbots", "Bot {} got totem {} x{}", bot->GetGUID().ToString().c_str(), proto->Name1.c_str(), 1);
            }
        }
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
      if ((*itr).ClassId == bot->getClass() && (*itr).SpecId == spec)
         botAI->EnchantItemT((*itr).SpellId, (*itr).SlotId);
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
