/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotFactory.h"
#include "PlayerbotsDatabase.h"
#include "AccountMgr.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GuildMgr.h"
#include "InventoryAction.h"
#include "Item.h"
#include "ItemPackets.h"
#include "ItemTemplate.h"
#include "ItemVisitors.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectMgr.h"
#include "PerfMonitor.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotGuildMgr.h"
#include "PlayerbotRepository.h"
#include "PlayerbotSpellRepository.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "ReputationMgr.h"
#include "SharedDefines.h"
#include "StatsWeightCalculator.h"
#include "SpellMgr.h"
#include "Trainer.h"
#include "World.h"
#include <array>
#include <unordered_set>
#include <utility>

#include <array>
#include <utility>

namespace
{
    // WotLK Eternal Belt Buckle mechanics
    constexpr uint32 ITEM_ETERNAL_BELT_BUCKLE = 41611;  // Eternal Belt Buckle (consumable)
    constexpr uint32 ENCHANT_SOCKET_BELT = 3729;        // "Socket Belt" -> adds 1 prismatic socket
    constexpr uint8 ETERNAL_BELT_BUCKLE_LEVEL = 70;     // required level of the buckle
}

// A non-zero prismatic enchantment is the socket marker, so the belt already has its extra socket.
static bool HasBeltBuckleSocket(Item* waist)
{
    return waist && waist->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT) != 0;
}

// Add the prismatic socket to a belt, granted free like the enchants and gems around it.
static bool TryApplyBeltBuckle(Player* bot, Item* waist)
{
    if (!bot || !waist)
        return false;
    if (HasBeltBuckleSocket(waist))
        return false;
    if (bot->GetLevel() < ETERNAL_BELT_BUCKLE_LEVEL)
        return false;

    // The buckle is a WotLK item; leave it alone when the bot is held back to an earlier expansion.
    if (sPlayerbotAIConfig.LimitEnchantExpansion && bot->GetLevel() <= 70)
        return false;

    bot->ApplyEnchantment(waist, PRISMATIC_ENCHANTMENT_SLOT, false);
    waist->SetEnchantment(PRISMATIC_ENCHANTMENT_SLOT, ENCHANT_SOCKET_BELT, 0, 0, bot->GetGUID());
    bot->ApplyEnchantment(waist, PRISMATIC_ENCHANTMENT_SLOT, true);

    if (bot->HasItemCount(ITEM_ETERNAL_BELT_BUCKLE, 1))
        bot->DestroyItemCount(ITEM_ETERNAL_BELT_BUCKLE, 1, true);

    return true;
}

const uint64 diveMask = (1LL << 7) | (1LL << 44) | (1LL << 37) | (1LL << 38) | (1LL << 26) | (1LL << 30) | (1LL << 27) |
                        (1LL << 33) | (1LL << 24) | (1LL << 34);

static std::vector<uint32> initSlotsOrder = {EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2, EQUIPMENT_SLOT_MAINHAND,
    EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED, EQUIPMENT_SLOT_HEAD, EQUIPMENT_SLOT_SHOULDERS, EQUIPMENT_SLOT_CHEST,
    EQUIPMENT_SLOT_LEGS, EQUIPMENT_SLOT_HANDS, EQUIPMENT_SLOT_NECK, EQUIPMENT_SLOT_BODY, EQUIPMENT_SLOT_WAIST,
    EQUIPMENT_SLOT_FEET, EQUIPMENT_SLOT_WRISTS, EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2, EQUIPMENT_SLOT_BACK};

uint32 PlayerbotFactory::tradeSkills[] = {SKILL_ALCHEMY,         SKILL_ENCHANTING,   SKILL_SKINNING,
                                          SKILL_TAILORING,       SKILL_LEATHERWORKING, SKILL_ENGINEERING,
                                          SKILL_HERBALISM,       SKILL_INSCRIPTION,  SKILL_MINING,
                                          SKILL_BLACKSMITHING,   SKILL_COOKING,      SKILL_FIRST_AID,
                                          SKILL_FISHING,         SKILL_JEWELCRAFTING};

std::list<uint32> PlayerbotFactory::classQuestIds;
std::list<uint32> PlayerbotFactory::specialQuestIds;
std::vector<uint32> PlayerbotFactory::enchantSpellIdCache;
std::vector<uint32> PlayerbotFactory::enchantGemIdCache;
std::unordered_map<uint32, std::vector<uint32>> PlayerbotFactory::trainerIdCache;
std::vector<uint32> PlayerbotFactory::ccBreakTrinketCache;

namespace
{
constexpr uint32 SPELL_DRUID_THICK_HIDE = 16931;
constexpr uint32 SPELL_OWLKIN_FRENZY = 48393;
constexpr uint32 SPELL_PRIMAL_TENACITY = 33957;
constexpr uint32 SPELL_IMPROVED_BARKSKIN = 63411;

constexpr uint32 SPELL_SECOND_WIND = 29838;
constexpr uint32 SPELL_BLOOD_CRAZE = 16492;
constexpr uint32 SPELL_GAG_ORDER = 12958;

constexpr uint32 SPELL_SACRED_CLEANSING = 53553;
constexpr uint32 SPELL_RECKONING = 20179;
constexpr uint32 SPELL_DIVINE_PURPOSE = 31872;

constexpr uint32 SPELL_HUNTER_THICK_HIDE = 19612;
constexpr uint32 SPELL_CONCUSSIVE_BARRAGE = 35102;
constexpr uint32 SPELL_ENTRAPMENT = 19388;

constexpr uint32 SPELL_DEADLY_BREW = 51626;
constexpr uint32 SPELL_THROWING_SPECIALIZATION = 51679;
constexpr uint32 SPELL_WAYLAY = 51696;

constexpr uint32 SPELL_IMPROVED_MANA_BURN = 14772;
constexpr uint32 SPELL_BODY_AND_SOUL = 64129;
constexpr uint32 SPELL_IMPROVED_VAMPIRIC_EMBRACE = 27840;

constexpr uint32 SPELL_ABOMINATIONS_MIGHT = 53138;
constexpr uint32 SPELL_IMPROVED_ICY_TALONS = 55610;
constexpr uint32 SPELL_SUDDEN_DOOM = 49529;
constexpr uint32 SPELL_ACCLIMATION = 50152;
constexpr uint32 SPELL_MAGIC_SUPPRESSION = 49611;

constexpr uint32 SPELL_SHAMAN_DUAL_WIELD = 30798;
constexpr uint32 SPELL_ASTRAL_SHIFT = 51479;
constexpr uint32 SPELL_EARTHEN_POWER = 51524;
constexpr uint32 SPELL_FOCUSED_MIND = 30866;

constexpr uint32 SPELL_BURNOUT = 44472;
constexpr uint32 SPELL_ICE_SHARDS = 15047;
constexpr uint32 SPELL_IMPROVED_BLINK = 31570;
constexpr uint32 SPELL_FIERY_PAYBACK = 64357;
constexpr uint32 SPELL_SHATTERED_BARRIER = 54787;

constexpr uint32 SPELL_IMPROVED_HOWL_OF_TERROR = 30057;
constexpr uint32 SPELL_NEMESIS = 63123;
constexpr uint32 SPELL_INTENSITY = 18136;
constexpr uint32 SPELL_NETHER_PROTECTION = 30302;

// Some creature_template rows are Blizzard developer leftovers or placeholders that carry the tameable
// flag but have no spawn in the world - e.g. 5596 "Twain Test Prop", a wolf family beast wearing a
// dragon whelp model. No player can ever tame those, so bots should not end up with them either.
bool HasCreatureSpawnRow(uint32 entry)
{
    static std::unordered_set<uint32> const spawnedEntries = []  // built once, at first use
    {
        std::unordered_set<uint32> entries;
        for (auto const& itr : sObjectMgr->GetAllCreatureData())
        {
            CreatureData const& data = itr.second;
            entries.insert(data.id);
            if (data.id2)
                entries.insert(data.id2);
            if (data.id3)
                entries.insert(data.id3);
        }
        return entries;
    }();

    return spawnedEntries.find(entry) != spawnedEntries.end();
}
}

bool PlayerbotFactory::IsPrimaryTradeSkill(uint16 skillId)
{
    SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skillId);
    return skillLine && skillLine->categoryId == SKILL_CATEGORY_PROFESSION;
}

bool PlayerbotFactory::IsSecondaryTradeSkill(uint16 skillId)
{
    switch (skillId)
    {
        case SKILL_COOKING:
        case SKILL_FIRST_AID:
        case SKILL_FISHING:
            return true;
        default:
            return false;
    }
}

uint16 PlayerbotFactory::GetTrainerSpellTradeSkill(Trainer::Spell const* trainerSpell)
{
    if (!trainerSpell)
        return 0;

    auto getSpellTradeSkill = [](uint32 spellId) -> uint16
    {
        if (SpellLearnSkillNode const* learnSkill = sSpellMgr->GetSpellLearnSkill(spellId))
        {
            if (IsPrimaryTradeSkill(learnSkill->skill) || IsSecondaryTradeSkill(learnSkill->skill))
                return learnSkill->skill;
        }

        SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
        {
            uint16 const skillId = itr->second->SkillLine;
            if (IsPrimaryTradeSkill(skillId) || IsSecondaryTradeSkill(skillId))
                return skillId;
        }

        return 0;
    };

    uint16 const requiredSkill = static_cast<uint16>(trainerSpell->ReqSkillLine);
    if (IsPrimaryTradeSkill(requiredSkill) || IsSecondaryTradeSkill(requiredSkill))
        return requiredSkill;

    if (uint16 const skillId = getSpellTradeSkill(trainerSpell->SpellId))
        return skillId;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(trainerSpell->SpellId);
    if (!spellInfo)
        return 0;

    for (uint8 effectIndex = 0; effectIndex < MAX_SPELL_EFFECTS; ++effectIndex)
    {
        if (spellInfo->Effects[effectIndex].Effect != SPELL_EFFECT_LEARN_SPELL)
            continue;

        uint32 const learnedSpellId = spellInfo->Effects[effectIndex].TriggerSpell;
        if (!learnedSpellId)
            continue;

        if (uint16 const skillId = getSpellTradeSkill(learnedSpellId))
            return skillId;
    }

    return 0;
}

bool PlayerbotFactory::IsTrainerSpellAllowedForBot(Player* bot, Trainer::Trainer const* trainer,
                                                     Trainer::Spell const* trainerSpell)
{
    if (!bot || !trainer || !trainerSpell)
        return false;

    if (trainer->GetTrainerType() != Trainer::Type::Tradeskill || !sRandomPlayerbotMgr.IsRandomBot(bot))
        return true;

    uint16 const skillId = GetTrainerSpellTradeSkill(trainerSpell);
    if (!skillId)
        return false;

    if (IsSecondaryTradeSkill(skillId))
        return true;

    if (!IsPrimaryTradeSkill(skillId))
        return false;

    uint16 const firstSkill = sRandomPlayerbotMgr.GetValue(bot, "firstSkill");
    uint16 const secondSkill = sRandomPlayerbotMgr.GetValue(bot, "secondSkill");

    if ((IsPrimaryTradeSkill(firstSkill) && skillId == firstSkill) ||
        (IsPrimaryTradeSkill(secondSkill) && skillId == secondSkill))
        return true;

    if (IsPrimaryTradeSkill(firstSkill) || IsPrimaryTradeSkill(secondSkill))
        return false;

    uint32 knownPrimarySkills = 0;
    for (uint32 tradeSkill : tradeSkills)
    {
        if (IsPrimaryTradeSkill(tradeSkill) && bot->HasSkill(tradeSkill))
            ++knownPrimarySkills;
    }

    uint32 const maxPrimaryTradeSkills =
        std::min<uint32>(2, sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL));
    return knownPrimarySkills <= maxPrimaryTradeSkills && bot->HasSkill(skillId);
}

bool PlayerbotFactory::IsGatheringTradeSkill(uint16 skillId)
{
    switch (skillId)
    {
        case SKILL_HERBALISM:
        case SKILL_MINING:
        case SKILL_SKINNING:
            return true;
        default:
            return false;
    }
}

bool PlayerbotFactory::IsCraftingTradeSkill(uint16 skillId)
{
    return IsPrimaryTradeSkill(skillId) && !IsGatheringTradeSkill(skillId);
}

uint32 PlayerbotFactory::GetProfessionStarterSpell(uint16 skillId)
{
    static constexpr std::array<std::pair<uint16, uint32>, 14> ProfessionStarterSpells = {{
        {SKILL_ALCHEMY, 2259},
        {SKILL_BLACKSMITHING, 2018},
        {SKILL_COOKING, 2550},
        {SKILL_ENCHANTING, 7411},
        {SKILL_ENGINEERING, 4036},
        {SKILL_FIRST_AID, 3273},
        {SKILL_FISHING, 7620},
        {SKILL_HERBALISM, 2366},
        {SKILL_INSCRIPTION, 45357},
        {SKILL_JEWELCRAFTING, 25229},
        {SKILL_LEATHERWORKING, 2108},
        {SKILL_MINING, 2575},
        {SKILL_SKINNING, 8613},
        {SKILL_TAILORING, 3908}
    }};

    for (auto const& [professionSkill, starterSpell] : ProfessionStarterSpells)
    {
        if (professionSkill == skillId)
            return starterSpell;
    }

    return 0;
}

std::vector<PlayerbotFactory::WeightedProfessionPair> PlayerbotFactory::GetClassProfessionPairs(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_WARRIOR:
            return {{SKILL_MINING, SKILL_BLACKSMITHING, 45},
                    {SKILL_MINING, SKILL_ENGINEERING, 30},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 15},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 10}};
        case CLASS_PALADIN:
            return {{SKILL_MINING, SKILL_BLACKSMITHING, 45},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 30},
                    {SKILL_MINING, SKILL_ENGINEERING, 15},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 10}};
        case CLASS_DEATH_KNIGHT:
            return {{SKILL_MINING, SKILL_BLACKSMITHING, 45},
                    {SKILL_MINING, SKILL_ENGINEERING, 35},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 20}};
        case CLASS_HUNTER:
            return {{SKILL_SKINNING, SKILL_LEATHERWORKING, 45},
                    {SKILL_MINING, SKILL_ENGINEERING, 35},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 10},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 10}};
        case CLASS_ROGUE:
            return {{SKILL_SKINNING, SKILL_LEATHERWORKING, 35},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25},
                    {SKILL_MINING, SKILL_ENGINEERING, 25},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 10},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 5}};
        case CLASS_DRUID:
            return {{SKILL_SKINNING, SKILL_LEATHERWORKING, 35},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 35},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 20},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 10}};
        case CLASS_SHAMAN:
            return {{SKILL_HERBALISM, SKILL_ALCHEMY, 35},
                    {SKILL_SKINNING, SKILL_LEATHERWORKING, 25},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 25},
                    {SKILL_MINING, SKILL_JEWELCRAFTING, 15}};
        case CLASS_PRIEST:
            return {{SKILL_TAILORING, SKILL_ENCHANTING, 45},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 30},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25}};
        case CLASS_MAGE:
            return {{SKILL_TAILORING, SKILL_ENCHANTING, 50},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 25}};
        case CLASS_WARLOCK:
        default:
            return {{SKILL_TAILORING, SKILL_ENCHANTING, 50},
                    {SKILL_HERBALISM, SKILL_ALCHEMY, 25},
                    {SKILL_HERBALISM, SKILL_INSCRIPTION, 25}};
    }
}

std::vector<PlayerbotFactory::WeightedProfessionPair> PlayerbotFactory::GetRandomProfessionPairs()
{
    return {{SKILL_MINING, SKILL_BLACKSMITHING, 20},
            {SKILL_MINING, SKILL_ENGINEERING, 18},
            {SKILL_MINING, SKILL_JEWELCRAFTING, 16},
            {SKILL_SKINNING, SKILL_LEATHERWORKING, 18},
            {SKILL_HERBALISM, SKILL_ALCHEMY, 18},
            {SKILL_HERBALISM, SKILL_INSCRIPTION, 14},
            {SKILL_TAILORING, SKILL_ENCHANTING, 10},
            {SKILL_HERBALISM, SKILL_MINING, 6},
            {SKILL_HERBALISM, SKILL_SKINNING, 5},
            {SKILL_MINING, SKILL_SKINNING, 5}};
}

std::pair<uint16, uint16> PlayerbotFactory::ChooseProfessionPair(
    std::vector<WeightedProfessionPair> const& professionPairs)
{
    uint32 totalWeight = 0;
    for (WeightedProfessionPair const& pair : professionPairs)
        totalWeight += pair.weight;

    if (!totalWeight)
        return {SKILL_HERBALISM, SKILL_ALCHEMY};

    uint32 roll = urand(1, totalWeight);
    for (WeightedProfessionPair const& pair : professionPairs)
    {
        if (roll <= pair.weight)
            return {pair.firstSkill, pair.secondSkill};

        roll -= pair.weight;
    }

    WeightedProfessionPair const& fallback = professionPairs.back();
    return {fallback.firstSkill, fallback.secondSkill};
}

uint16 PlayerbotFactory::ChooseSingleProfession(std::vector<WeightedProfessionPair> const& professionPairs)
{
    std::vector<std::pair<uint16, uint32>> gatheringSkills;
    std::vector<std::pair<uint16, uint32>> craftingSkills;

    auto addWeightedSkill = [](std::vector<std::pair<uint16, uint32>>& skills, uint16 skillId, uint32 weight)
    {
        for (std::pair<uint16, uint32>& skill : skills)
        {
            if (skill.first == skillId)
            {
                skill.second += weight;
                return;
            }
        }

        skills.push_back({skillId, weight});
    };

    for (WeightedProfessionPair const& pair : professionPairs)
    {
        if (IsGatheringTradeSkill(pair.firstSkill))
            addWeightedSkill(gatheringSkills, pair.firstSkill, pair.weight);
        if (IsCraftingTradeSkill(pair.firstSkill))
            addWeightedSkill(craftingSkills, pair.firstSkill, pair.weight);

        if (IsGatheringTradeSkill(pair.secondSkill))
            addWeightedSkill(gatheringSkills, pair.secondSkill, pair.weight);
        if (IsCraftingTradeSkill(pair.secondSkill))
            addWeightedSkill(craftingSkills, pair.secondSkill, pair.weight);
    }

    std::vector<std::pair<uint16, uint32>>* selectedPool = nullptr;
    if (!gatheringSkills.empty() && !craftingSkills.empty())
        selectedPool = urand(0, 1) == 0 ? &gatheringSkills : &craftingSkills;
    else if (!gatheringSkills.empty())
        selectedPool = &gatheringSkills;
    else if (!craftingSkills.empty())
        selectedPool = &craftingSkills;

    if (!selectedPool || selectedPool->empty())
        return SKILL_HERBALISM;

    uint32 totalWeight = 0;
    for (std::pair<uint16, uint32> const& skill : *selectedPool)
        totalWeight += skill.second;

    if (!totalWeight)
        return selectedPool->front().first;

    uint32 roll = urand(1, totalWeight);
    for (std::pair<uint16, uint32> const& skill : *selectedPool)
    {
        if (roll <= skill.second)
            return skill.first;

        roll -= skill.second;
    }

    return selectedPool->back().first;
}

uint16 PlayerbotFactory::ChooseComplementaryProfession(
    std::vector<WeightedProfessionPair> const& professionPairs, uint16 existingSkill)
{
    std::vector<std::pair<uint16, uint32>> candidates;
    uint32 totalWeight = 0;

    for (WeightedProfessionPair const& pair : professionPairs)
    {
        uint16 candidate = 0;
        if (pair.firstSkill == existingSkill)
            candidate = pair.secondSkill;
        else if (pair.secondSkill == existingSkill)
            candidate = pair.firstSkill;

        if (!candidate || candidate == existingSkill)
            continue;

        bool merged = false;
        for (std::pair<uint16, uint32>& existingCandidate : candidates)
        {
            if (existingCandidate.first != candidate)
                continue;

            existingCandidate.second += pair.weight;
            merged = true;
            break;
        }

        if (!merged)
            candidates.push_back({candidate, pair.weight});

        totalWeight += pair.weight;
    }

    if (candidates.empty() || !totalWeight)
    {
        std::pair<uint16, uint16> const fallback = ChooseProfessionPair(professionPairs);
        return fallback.first != existingSkill ? fallback.first : fallback.second;
    }

    uint32 roll = urand(1, totalWeight);
    for (std::pair<uint16, uint32> const& candidate : candidates)
    {
        if (roll <= candidate.second)
            return candidate.first;

        roll -= candidate.second;
    }

    return candidates.back().first;
}

uint32 PlayerbotFactory::GetStoredOrRandomValue(Player* bot,
                                                std::string const& key,
                                                uint32 minValue,
                                                uint32 maxValue)
{
    uint32 value = sRandomPlayerbotMgr.GetValue(bot, key);
    if (value < minValue || value > maxValue)
    {
        value = urand(minValue, maxValue);
        sRandomPlayerbotMgr.SetValue(bot, key, value);
    }

    return value;
}

bool PlayerbotFactory::HasAnySpell(Player* bot, std::vector<uint32> const& spells)
{
    for (uint32 spellId : spells)
    {
        if (bot->HasSpell(spellId))
            return true;
    }

    return false;
}

bool PlayerbotFactory::LearnProfessionSpecialization(Player* bot,
                                                     ProfessionSpecializationSpell knownSpell,
                                                     ProfessionSpecializationSpell learnSpell)
{
    uint32 const knownSpellId = static_cast<uint32>(knownSpell);
    uint32 const learnSpellId = static_cast<uint32>(learnSpell);

    if (bot->HasSpell(knownSpellId) || !sSpellMgr->GetSpellInfo(learnSpellId))
        return false;

    bot->CastSpell(bot, learnSpellId, true);
    return bot->HasSpell(knownSpellId);
}

PlayerbotFactory::PlayerbotFactory(Player* bot, uint32 level, uint32 itemQuality, uint32 gearScoreLimit)
    : level(level), itemQuality(itemQuality), gearScoreLimit(gearScoreLimit), bot(bot)
{
    botAI = GET_PLAYERBOT_AI(bot);
    if (!this->itemQuality)
    {
        uint32 gs = sPlayerbotAIConfig.RandomGearScoreLimit == 0
                        ? 0
                        : PlayerbotFactory::CalcMixedGearScore(sPlayerbotAIConfig.RandomGearScoreLimit,
                                                               sPlayerbotAIConfig.RandomGearQualityLimit);
        this->itemQuality = sPlayerbotAIConfig.RandomGearQualityLimit;
        this->gearScoreLimit = gs;
    }
}

void PlayerbotFactory::Init()
{
    if (sPlayerbotAIConfig.RandomBotPreQuests)
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

    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig.RandomBotQuestIds.begin();
         i != sPlayerbotAIConfig.RandomBotQuestIds.end(); ++i)
    {
        uint32 questId = *i;
        AddPrevQuests(questId, specialQuestIds);
        specialQuestIds.remove(questId);
        specialQuestIds.push_back(questId);
    }
    uint32 maxStoreSize = sSpellMgr->GetSpellInfoStoreSize();
    for (uint32 id = 1; id < maxStoreSize; ++id)
    {
        if (id == 7218 || id == 19927 || id == 44119 || id == 47147 || id == 47181 ||
            id == 47242 || id == 50358 || id == 52639) // Test Enchants
            continue;

        if (id == 35791 || id == 39405) // Grandfathered TBC Enchants
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

        if (proto->HasFlag(ITEM_FLAG_UNIQUE_EQUIPPABLE))
        {
            continue;
        }

        if (sRandomItemMgr.IsInternalItem(proto))
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

    BuildCcBreakTrinketCache();
}

void PlayerbotFactory::BuildCcBreakTrinketCache()
{
    ccBreakTrinketCache.clear();
    // Spell 42292: removes all movement-impairing and loss-of-control effects — the PvP trinket spell.
    QueryResult result = WorldDatabase.Query(
        "SELECT entry, ItemLevel FROM item_template "
        "WHERE Quality >= 2 AND InventoryType = 12 "
        "AND (FlagsExtra & 8192) = 0 "
        "AND (spellid_1 = 42292 OR spellid_2 = 42292 OR spellid_3 = 42292 "
        "  OR spellid_4 = 42292 OR spellid_5 = 42292)");

    if (!result)
    {
        LOG_INFO("playerbots", "CC-break trinket cache: no items found.");
        return;
    }

    struct CcItem { uint32 itemId; uint16 itemLevel; };
    std::vector<CcItem> tmp;
    do
    {
        Field* f = result->Fetch();
        tmp.push_back({f[0].Get<uint32>(), f[1].Get<uint16>()});
    } while (result->NextRow());

    std::sort(tmp.begin(), tmp.end(), [](CcItem const& a, CcItem const& b) {
        return a.itemLevel > b.itemLevel;
    });
    for (auto& c : tmp)
        ccBreakTrinketCache.push_back(c.itemId);

    LOG_INFO("playerbots", "CC-break trinket cache: {} items.", ccBreakTrinketCache.size());
}

uint8 PlayerbotFactory::GetPreferredArmorType(uint8 cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_DEATH_KNIGHT:
            return ITEM_SUBCLASS_ARMOR_PLATE;

        case CLASS_HUNTER:
        case CLASS_SHAMAN:
            return ITEM_SUBCLASS_ARMOR_MAIL;

        case CLASS_ROGUE:
        case CLASS_DRUID:
            return ITEM_SUBCLASS_ARMOR_LEATHER;

        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            return ITEM_SUBCLASS_ARMOR_CLOTH;

        default:
            return 0;
    }
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

    if (sPlayerbotAIConfig.RandomBotShowHelmet == ShowHideCosmetic::ALWAYS_SHOW ||
        (sPlayerbotAIConfig.RandomBotShowHelmet == ShowHideCosmetic::RANDOMIZE && urand(0, 4)))
    {
        bot->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
    }
    else
    {
        bot->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
    }

    if (sPlayerbotAIConfig.RandomBotShowCloak == ShowHideCosmetic::ALWAYS_SHOW ||
        (sPlayerbotAIConfig.RandomBotShowCloak == ShowHideCosmetic::RANDOMIZE && urand(0, 4)))
    {
        bot->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
    }
    else
    {
        bot->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
    }
}

void PlayerbotFactory::Randomize(bool incremental)
{
    // if (sPlayerbotAIConfig.DisableRandomLevels)
    //     return;

    LOG_DEBUG("playerbots", "{} randomizing {} (level {} class = {})...", (incremental ? "Incremental" : "Full"),
             bot->GetName().c_str(), level, bot->getClass());
    // LOG_DEBUG("playerbots", "Preparing to {} randomize...", (incremental ? "incremental" : "full"));
    uint32 oldLevel = bot->GetLevel();
    Prepare();
    LOG_DEBUG("playerbots", "Resetting player...");
    PerfMonitorOperation* pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Reset");

    if (!sPlayerbotAIConfig.EquipAndSpecPersistence ||
        level < uint32(sPlayerbotAIConfig.EquipAndSpecPersistenceLevel))
    {
        bot->resetTalents(true);
    }

    if (!incremental)
    {
        ClearSkills();
        ClearSpells();
        ResetQuests();

        if (!sPlayerbotAIConfig.EquipAndSpecPersistence ||
            level < uint32(sPlayerbotAIConfig.EquipAndSpecPersistenceLevel) || level < oldLevel)
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

    // pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Immersive");
    // LOG_INFO("playerbots", "Initializing immersive...");
    // InitImmersive();
    // if (pmo)
    //     pmo->finish();

    if (sPlayerbotAIConfig.RandomBotPreQuests)
    {
        pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Quests");
        InitInstanceQuests();
        InitAttunementQuests();
        if (pmo)
            pmo->finish();
    }
    else
    {
        pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Quests");
        InitAttunementQuests();
        if (pmo)
            pmo->finish();
    }

    LOG_DEBUG("playerbots", "Initializing skills (step 1)...");
    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills1");
    bot->LearnDefaultSkills();
    InitSkills();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells1");
    LOG_DEBUG("playerbots", "Initializing spells (step 1)...");
    InitClassSpells();
    InitAvailableSpells();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Talents");
    LOG_DEBUG("playerbots", "Initializing talents...");
    if (!incremental || !sPlayerbotAIConfig.EquipAndSpecPersistence ||
        bot->GetLevel() < sPlayerbotAIConfig.EquipAndSpecPersistenceLevel)
    {
        uint32 specIndex = InitTalentsTree();
        sRandomPlayerbotMgr.SetValue(bot->GetGUID().GetCounter(), "specNo", specIndex + 1);
    }
    if (botAI)
    {
        PlayerbotRepository::instance().Reset(botAI);
        // botAI->DoSpecificAction("auto talents");
        botAI->ResetStrategies(false);  // fix wrong stored strategy
    }
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells2");
    LOG_DEBUG("playerbots", "Initializing spells (step 2)...");
    InitAvailableSpells();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Reputation");
    LOG_DEBUG("playerbots", "Initializing reputation...");
    InitReputation();
    if (pmo)
        pmo->finish();

    LOG_DEBUG("playerbots", "Initializing special spells...");
    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Spells3");
    InitSpecialSpells();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Mounts");
    LOG_DEBUG("playerbots", "Initializing mounts...");
    InitMounts();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    // pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Skills2");
    // LOG_INFO("playerbots", "Initializing skills (step 2)...");
    // UpdateTradeSkills();
    // if (pmo)
    //     pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Equip");
    LOG_DEBUG("playerbots", "Initializing equipmemt...");
    if (!incremental || !sPlayerbotAIConfig.EquipAndSpecPersistence ||
        bot->GetLevel() < sPlayerbotAIConfig.EquipAndSpecPersistenceLevel)
    {
        InitEquipment(incremental, incremental ? false : sPlayerbotAIConfig.TwoRoundsGearInit);
    }
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    // if (bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
    // {
    //     pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Enchant");
    //     LOG_INFO("playerbots", "Initializing enchant templates...");
    //     LoadEnchantContainer();
    //     if (pmo)
    //         pmo->finish();
    // }

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Bags");
    LOG_DEBUG("playerbots", "Initializing bags...");
    InitBags();
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Ammo");
    LOG_DEBUG("playerbots", "Initializing ammo...");
    InitAmmo();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Food");
    LOG_DEBUG("playerbots", "Initializing food...");
    InitFood();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Potions");
    LOG_DEBUG("playerbots", "Initializing potions...");
    InitPotions();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Reagents");
    LOG_DEBUG("playerbots", "Initializing reagents...");
    InitReagents();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Keys");
    LOG_DEBUG("playerbots", "Initializing keys...");
    InitKeyring();
    if (pmo)
        pmo->finish();

    // pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_EqSets");
    // LOG_DEBUG("playerbots", "Initializing second equipment set...");
    //    InitSecondEquipmentSet();
    // if (pmo)
    //     pmo->finish();

    if (bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
    {
        ApplyEnchantAndGemsNew();
    }
    // {
    // pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_EnchantTemplate");
    // LOG_INFO("playerbots", "Initializing enchant templates...");
    // ApplyEnchantTemplate();
    // if (pmo)
    //     pmo->finish();
    // }

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Inventory");
    LOG_DEBUG("playerbots", "Initializing inventory...");
    // InitInventory();
    if (pmo)
        pmo->finish();

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Consumable");
    LOG_DEBUG("playerbots", "Initializing consumables...");
    InitConsumables();
    if (pmo)
        pmo->finish();

    LOG_DEBUG("playerbots", "Initializing glyphs...");
    InitGlyphs();
    // bot->SaveToDB(false, false);

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Guilds");
    // bot->SaveToDB(false, false);
    if (sPlayerbotAIConfig.RandomBotGuildCount > 0)
    {
        LOG_DEBUG("playerbots", "Initializing guilds...");
        InitGuild();
    }
    // bot->SaveToDB(false, false);
    if (pmo)
        pmo->finish();

    if (bot->GetLevel() >= 70)
    {
        pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Arenas");
        RandomPlayerbotFactory::AssignBotToArenaTeam(bot);
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
        pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Pet");
        LOG_DEBUG("playerbots", "Initializing pet...");
        InitPet();
        // bot->SaveToDB(false, false);
        InitPetTalents();
        if (pmo)
            pmo->finish();
    }

    pmo = sPerfMonitor.start(PERF_MON_RNDBOT, "PlayerbotFactory_Save");
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
    // if (!sPlayerbotAIConfig.EquipAndSpecPersistence ||
    //     bot->GetLevel() < sPlayerbotAIConfig.EquipAndSpecPersistenceLevel)
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
    InitSkills();
    InitClassSpells();
    InitAvailableSpells();
    InitReputation();
    InitSpecialSpells();
    InitMounts();
    InitKeyring();
    if (!sPlayerbotAIConfig.EquipAndSpecPersistence ||
        bot->GetLevel() < sPlayerbotAIConfig.EquipAndSpecPersistenceLevel)
    {
        InitTalentsTree(true, true, true);
    }
    if (bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
        ApplyEnchantAndGemsNew();
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
    uint8 specTab = AiFactory::GetPlayerSpecTab(bot);
    std::vector<std::pair<uint32, uint32>> items;

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
        {
            if (specTab == PRIEST_TAB_SHADOW)
            {
                std::vector<uint32> wizard_oils = {
                    BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL };
                for (uint32 itemId : wizard_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 2});
                    break;
                }
            }
            else
            {
                std::vector<uint32> mana_oils = {
                    BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL };
                for (uint32 itemId : mana_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 2});
                    break;
                }
            }
            break;
        }
        case CLASS_MAGE:
        {
            std::vector<uint32> wizard_oils = {
                BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL };
            for (uint32 itemId : wizard_oils)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (proto->RequiredLevel > level || level > 75)
                    continue;
                items.push_back({itemId, 2});
                break;
            }
            break;
        }
        case CLASS_DRUID:
        {
            if (specTab == DRUID_TAB_BALANCE)
            {
                std::vector<uint32> wizard_oils = {
                    BRILLIANT_WIZARD_OIL, SUPERIOR_WIZARD_OIL, WIZARD_OIL, LESSER_WIZARD_OIL, MINOR_WIZARD_OIL };
                for (uint32 itemId : wizard_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 2});
                    break;
                }
            }
            else if (specTab == DRUID_TAB_FERAL)
            {
                std::vector<uint32> sharpening_stones = {
                    ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE,
                    HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE };
                std::vector<uint32> weightstones = {
                    ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
                    HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE };
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
            else
            {
                std::vector<uint32> mana_oils = {
                    BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL };
                for (uint32 itemId : mana_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 2});
                    break;
                }
            }
            break;
        }
        case CLASS_PALADIN:
        {
            if (specTab == PALADIN_TAB_HOLY)
            {
                std::vector<uint32> mana_oils = {
                    BRILLIANT_MANA_OIL, SUPERIOR_MANA_OIL, LESSER_MANA_OIL, MINOR_MANA_OIL };
                for (uint32 itemId : mana_oils)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto->RequiredLevel > level || level > 75)
                        continue;
                    items.push_back({itemId, 2});
                    break;
                }
            }
            else
            {
                std::vector<uint32> sharpening_stones = {
                    ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE,
                    HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE };
                std::vector<uint32> weightstones = {
                    ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
                    HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE };
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
        case CLASS_DEATH_KNIGHT:
        {
            std::vector<uint32> sharpening_stones = {
                ADAMANTITE_SHARPENING_STONE, FEL_SHARPENING_STONE, DENSE_SHARPENING_STONE, SOLID_SHARPENING_STONE,
                HEAVY_SHARPENING_STONE, COARSE_SHARPENING_STONE, ROUGH_SHARPENING_STONE };
            std::vector<uint32> weightstones = {
                ADAMANTITE_WEIGHTSTONE, FEL_WEIGHTSTONE, DENSE_WEIGHTSTONE, SOLID_WEIGHTSTONE,
                HEAVY_WEIGHTSTONE, COARSE_WEIGHTSTONE, ROUGH_WEIGHTSTONE };
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
            std::vector<uint32> instant_poisons = {
                INSTANT_POISON_IX, INSTANT_POISON_VIII, INSTANT_POISON_VII, INSTANT_POISON_VI, INSTANT_POISON_V,
                INSTANT_POISON_IV, INSTANT_POISON_III, INSTANT_POISON_II, INSTANT_POISON };
            std::vector<uint32> deadly_poisons = {
                DEADLY_POISON_IX, DEADLY_POISON_VIII, DEADLY_POISON_VII, DEADLY_POISON_VI, DEADLY_POISON_V,
                DEADLY_POISON_IV, DEADLY_POISON_III, DEADLY_POISON_II, DEADLY_POISON };
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
    if (bot->GetLevel() <= 70 && sPlayerbotAIConfig.LimitTalentsExpansion)
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
    std::map<uint32, std::vector<TalentEntry const*>> spells;
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
        sPlayerbotAIConfig.ParsedHunterPetLinkOrder[pet_family->petTalentType][20];
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
                for (uint32 rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, (uint32)pet->GetFreeTalentPoints()); ++rank)
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
               sPlayerbotAIConfig.ParsedHunterPetLinkOrder[spec][startPoints].size() == 0)
        {
            startPoints--;
        }

        for (uint32 points = startPoints; points <= 20; points++)
        {
            if (sPlayerbotAIConfig.ParsedHunterPetLinkOrder[spec][points].size() == 0)
                continue;
            for (std::vector<uint32>& p : sPlayerbotAIConfig.ParsedHunterPetLinkOrder[spec][points])
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

            if (!HasCreatureSpawnRow(itr->first))
                continue;

            if (itr->second.minlevel > bot->GetLevel())
                continue;

            bool onlyWolf = sPlayerbotAIConfig.HunterWolfPet == 2 ||
                            (sPlayerbotAIConfig.HunterWolfPet == 1 &&
                             bot->GetLevel() >= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
            // Wolf only (for higher dps)
            if (onlyWolf && itr->second.family != CREATURE_FAMILY_WOLF)
                continue;

            // Exclude configured pet families
            if (std::find(sPlayerbotAIConfig.ExcludedHunterPetFamilies.begin(),
                          sPlayerbotAIConfig.ExcludedHunterPetFamilies.end(),
                          itr->second.family) != sPlayerbotAIConfig.ExcludedHunterPetFamilies.end())
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

uint32 PlayerbotFactory::InitTalentsTree(bool increment /*false*/, bool use_template /*true*/, bool reset /*false*/)
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
            bool isCat = !bot->HasAura(SPELL_DRUID_THICK_HIDE);
            if (!isCat && bot->GetLevel() == 20)
            {
                uint32 bearP = sPlayerbotAIConfig.RandomClassSpecProb[cls][1];
                uint32 catP = sPlayerbotAIConfig.RandomClassSpecProb[cls][3];
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
            pointSum += sPlayerbotAIConfig.RandomClassSpecProb[cls][i];
        }
        uint32 point = urand(1, pointSum);
        uint32 currentP = 0;
        int i;
        for (i = 0; i < MAX_SPECNO; i++)
        {
            currentP += sPlayerbotAIConfig.RandomClassSpecProb[cls][i];
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

    if (bot->getClass() == CLASS_SHAMAN && bot->HasSpell(SPELL_SHAMAN_DUAL_WIELD))
    {
        bot->SetSkill(SKILL_DUAL_WIELD, 0, 1, 1);
        bot->SetCanDualWield(true);
    }

    bot->SendTalentsInfoData(false);
    return sPlayerbotAIConfig.RandomClassSpecIndex[cls][specTab];
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
           sPlayerbotAIConfig.ParsedSpecLinkOrder[cls][specNo][startLevel].size() == 0)
    {
        startLevel--;
    }
    for (int level = startLevel; level <= 80; level++)
    {
        if (sPlayerbotAIConfig.ParsedSpecLinkOrder[cls][specNo][level].size() == 0)
        {
            continue;
        }
        for (std::vector<uint32>& p : sPlayerbotAIConfig.ParsedSpecLinkOrder[cls][specNo][level])
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

    if (bot->getClass() == CLASS_SHAMAN && bot->HasSpell(SPELL_SHAMAN_DUAL_WIELD))
    {
        bot->SetSkill(SKILL_DUAL_WIELD, 0, 1, 1);
        bot->SetCanDualWield(true);
    }

    bot->SendTalentsInfoData(false);
    sRandomPlayerbotMgr.SetValue(bot->GetGUID().GetCounter(), "specNo", (uint32)specNo + 1);
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

        if (sPlayerbotAIConfig.IsInRandomQuestItemList(id))
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
    //     if (!proto->ItemStat[j].ItemStatValue)
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
//         if (urand(0, 100) < 100 * sPlayerbotAIConfig.RandomGearLoweringChance && quality > ITEM_QUALITY_NORMAL)
//         {
//             quality--;
//         }
//         // current item;
//         Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
//         if (oldItem)
//             isUpgrade = true;

//         uint32 itemInSlot = isUpgrade ? oldItem->GetTemplate()->ItemId : 0;

//         uint32 maxLevel = sPlayerbotAIConfig.RandomBotMaxLevel;
//         if (maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
//             maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

//         uint32 minLevel = sPlayerbotAIConfig.RandomBotMinLevel;
//         if (minLevel < sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL))
//             minLevel = sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL);

//         // test
//         do
//         {
//             if (isUpgrade)
//             {
//                 std::vector<uint32> ids = sRandomItemMgr.GetUpgradeList(bot, specName, slot, 0, itemInSlot);
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
//                 std::vector<uint32> ids = sRandomItemMgr.GetUpgradeList(bot, specName, slot, quality, itemInSlot);
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
    if (level < 5)
    {
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

                if (bot->HasItemCount(itemId, count))
                {
                    continue;
                }

                bot->StoreNewItemInBestSlots(itemId, count);
            }
        }
        return;
    }

    std::unordered_map<uint8, std::vector<std::pair<uint32, int32>>> items;
    // int tab = AiFactory::GetPlayerSpecTab(bot);

    uint32 blevel = bot->GetLevel();
    int32 delta = std::min(blevel, 10u);

    bool isPvp = sRandomPlayerbotMgr.IsSpecPvp(bot->GetGUID().GetCounter(), bot->getClass());

    StatsWeightCalculator calculator(bot);
    if (isPvp)
        calculator.SetPvpSpec(true);

    // Pre-select CC-break trinket for PvP specs: best available by item level
    // that the bot meets the level requirement for.
    // Humans (Every Man for Himself) and Undead (Will of the Forsaken) have a
    // racial that shares the PvP trinket cooldown, so they don't need one.
    bool racialHasCcBreak = (bot->getRace() == RACE_HUMAN || bot->getRace() == RACE_UNDEAD_PLAYER);
    uint32 pvpTrinket1 = 0;
    if (isPvp && level >= 50 && !racialHasCcBreak)
    {
        for (uint32 itemId : ccBreakTrinketCache)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto) continue;
            // Respect gear quality limit: trinket must not exceed itemQuality setting
            if (static_cast<int32>(proto->Quality) > static_cast<int32>(itemQuality)) continue;
            if (proto->RequiredLevel > level) continue;
            if (!CanEquipItem(proto)) continue;
            uint16 dest;
            if (!CanEquipUnseenItem(EQUIPMENT_SLOT_TRINKET1, dest, itemId)) continue;
            pvpTrinket1 = itemId;
            break;
        }
    }

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

        // Exclude resilience weighting for trinkets
        bool isTrinketSlot = (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2);
        calculator.SetExcludeResilience(isTrinketSlot);

        Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (second_chance && oldItem)
        {
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }

        oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        // PvP specs: force TRINKET1 to the best available CC-break trinket.
        if (slot == EQUIPMENT_SLOT_TRINKET1 && pvpTrinket1 != 0)
        {
            if (oldItem)
            {
                uint8 bagIndex = oldItem->GetBagSlot();
                uint8 oldSlot  = oldItem->GetSlot();
                uint8 dstBag   = NULL_BAG;
                WorldPacket packet(CMSG_AUTOSTORE_BAG_ITEM, 3);
                packet << bagIndex << oldSlot << dstBag;
                WorldPackets::Item::AutoStoreBagItem nicePacket(std::move(packet));
                nicePacket.Read();
                bot->GetSession()->HandleAutoStoreBagItemOpcode(nicePacket);
                oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (oldItem) continue;
            }
            uint16 dest;
            if (CanEquipUnseenItem(slot, dest, pvpTrinket1))
                bot->EquipNewItem(dest, pvpTrinket1, true);
            continue;
        }

        int32 desiredQuality = itemQuality;
        if (urand(0, 100) < 100 * sPlayerbotAIConfig.RandomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL)
            desiredQuality--;

        do
        {
            for (uint32 requiredLevel = bot->GetLevel(); requiredLevel > uint32(std::max((int32)bot->GetLevel() - delta, 0));
                 requiredLevel--)
            {
                for (InventoryType inventoryType : GetPossibleInventoryTypeListBySlot((EquipmentSlots)slot))
                {
                    for (uint32 itemId : sRandomItemMgr.GetEquipmentNew(requiredLevel, inventoryType))
                    {
                        uint32 skipProb = 25;
                        if (urand(1, 100) <= skipProb)
                            continue;

                        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                        // disable next expansion gear
                        if (sPlayerbotAIConfig.LimitGearExpansion && bot->GetLevel() <= 60 && itemId >= 23728)
                            continue;

                        if (sPlayerbotAIConfig.LimitGearExpansion && bot->GetLevel() <= 70 && itemId >= 35570 &&
                            itemId != 36737 && itemId != 37739 &&
                            itemId != 37740)  // transition point from TBC -> WOTLK isn't as clear, and there are other
                                              // wearable TBC items above 35570 but nothing of significance
                            continue;

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

                        if (proto->Quality != uint32(desiredQuality))
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

                        int32 bestRandomProp = 0;
                        if (proto->RandomProperty || proto->RandomSuffix)
                            bestRandomProp = calculator.PickBestRandomPropertyId(itemId);
                        items[slot].push_back({itemId, bestRandomProp});
                    }
                }
            }
        } while (items[slot].size() < 25 && desiredQuality-- > ITEM_QUALITY_POOR);

        std::vector<std::pair<uint32, int32>>& ids = items[slot];
        if (ids.empty())
        {
            continue;
        }

        float bestScoreForSlot = -1;
        uint32 bestItemForSlot = 0;
        int32 bestRandomPropForSlot = 0;
        for (size_t index = 0; index < ids.size(); index++)
        {
            uint32 newItemId = ids[index].first;
            int32 newItemProp = ids[index].second;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(newItemId);

            float cur_score = calculator.CalculateItem(newItemId, newItemProp, slot);

            if (cur_score > 0.0f && proto && proto->Class == ITEM_CLASS_ARMOR && sPlayerbotAIConfig.PreferClassArmorType)
            {
                uint8 preferredArmorType = GetPreferredArmorType(bot->getClass());
                if (preferredArmorType != 0 && proto->SubClass == preferredArmorType)
                    cur_score *= 3.0f;  // 3x multiplier for preferred armor type
            }

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
                bestRandomPropForSlot = newItemProp;
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
            float old_score = calculator.CalculateItem(oldItem->GetEntry(), oldItem->GetItemRandomPropertyId(), slot);
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

        if (Item* equipped = bot->EquipNewItem(dest, bestItemForSlot, true))
        {
            if (bestRandomPropForSlot != 0)
            {
                uint8 equipSlot = equipped->GetSlot();
                bot->_ApplyItemMods(equipped, equipSlot, false);
                equipped->SetItemRandomProperties(bestRandomPropForSlot);
                bot->_ApplyItemMods(equipped, equipSlot, true);
            }
        }
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

            // CC-break trinket was force-equipped in the main pass; leave it alone.
            if (slot == EQUIPMENT_SLOT_TRINKET1 && pvpTrinket1 != 0)
                continue;

            bool isTrinketSlot = (slot == EQUIPMENT_SLOT_TRINKET1 || slot == EQUIPMENT_SLOT_TRINKET2);
            calculator.SetExcludeResilience(isTrinketSlot);

            if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

            std::vector<std::pair<uint32, int32>>& ids = items[slot];
            if (ids.empty())
                continue;

            float bestScoreForSlot = -1;
            uint32 bestItemForSlot = 0;
            int32 bestRandomPropForSlot = 0;
            for (size_t index = 0; index < ids.size(); index++)
            {
                uint32 newItemId = ids[index].first;
                int32 newItemProp = ids[index].second;

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(newItemId);

                float cur_score = calculator.CalculateItem(newItemId, newItemProp, slot);

                if (cur_score > 0.0f && proto && proto->Class == ITEM_CLASS_ARMOR && sPlayerbotAIConfig.PreferClassArmorType)
                {
                    uint8 preferredArmorType = GetPreferredArmorType(bot->getClass());
                    if (preferredArmorType != 0 && proto->SubClass == preferredArmorType)
                        cur_score *= 3.0f;  // 3x multiplier for preferred armor type
                }

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
                    bestRandomPropForSlot = newItemProp;
                }
            }

            if (bestItemForSlot == 0)
                continue;

            uint16 dest;
            if (!CanEquipUnseenItem(slot, dest, bestItemForSlot))
                continue;

            if (Item* equipped = bot->EquipNewItem(dest, bestItemForSlot, true))
            {
                if (bestRandomPropForSlot != 0)
                {
                    uint8 equipSlot = equipped->GetSlot();
                    bot->_ApplyItemMods(equipped, equipSlot, false);
                    equipped->SetItemRandomProperties(bestRandomPropForSlot);
                    bot->_ApplyItemMods(equipped, equipSlot, true);
                }
            }
            bot->AutoUnequipOffhandIfNeed();
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
        return true;

    uint32 delta = 1 + (80 - bot->GetLevel()) / 10;
    return proto->Quality < ITEM_QUALITY_RARE || (bot->GetLevel() - requiredLevel) > delta;
}

inline Item* StoreNewItemInInventorySlot(Player* player, uint32 newItemId, uint32 count)
{
    ItemPosCountVec vDest;
    InventoryResult msg = player->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, vDest, newItemId, count);
    if (msg == EQUIP_ERR_OK)
    {
        if (Item* newItem = player->StoreNewItem(vDest, newItemId, true, Item::GenerateItemRandomPropertyId(newItemId)))
            return newItem;
    }

    return nullptr;
}

// void PlayerbotFactory::InitSecondEquipmentSet()
// {
//     if (bot->getClass() == CLASS_MAGE || bot->getClass() == CLASS_WARLOCK || bot->getClass() == CLASS_PRIEST)
//         return;

//     std::map<uint32, std::vector<uint32>> items;

//     uint32 desiredQuality = itemQuality;
//     while (urand(0, 100) < 100 * sPlayerbotAIConfig.RandomGearLoweringChance && desiredQuality >
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

//                 if (sRandomItemMgr.HasStatWeight(proto->ItemId))
//                 {
//                     if (!sRandomItemMgr.GetLiveStatWeight(bot, proto->ItemId))
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

//                 if (sRandomItemMgr.HasStatWeight(proto->ItemId))
//                 {
//                     if (!sRandomItemMgr.GetLiveStatWeight(bot, proto->ItemId))
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
            continue;

        uint16 dest;
        if (!CanEquipUnseenItem(slot, dest, newItemId))
            continue;

        if (old_bag && destroyOld)
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);

        if (old_bag)
            continue;

        bot->EquipNewItem(dest, newItemId, true);
        // if (newItem)
        // {
        //     newItem->AddToWorld();
        //     newItem->AddToUpdateQueueOf(bot);
        // }
    }
}

void PlayerbotFactory::EnchantItem(Item* item)
{
    if (bot->GetLevel() < sPlayerbotAIConfig.MinEnchantingBotLevel)
        return;

    if (urand(0, 100) < 100 * sPlayerbotAIConfig.RandomGearLoweringChance)
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
            for (uint8 i = 0; i < MAX_SPELL_ITEM_ENCHANTMENT_EFFECTS; ++i)
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

    if (Item* pItem = Item::CreateItem(item, 1, bot))
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
    if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        return;

    uint32 const maxPrimaryTradeSkills =
        std::min<uint32>(2, sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL));

    uint16 firstSkill = sRandomPlayerbotMgr.GetValue(bot, "firstSkill");
    uint16 secondSkill = sRandomPlayerbotMgr.GetValue(bot, "secondSkill");
    ProfessionRollType professionRollType =
        static_cast<ProfessionRollType>(sRandomPlayerbotMgr.GetValue(bot, "professionRollType"));

    if (professionRollType != ProfessionRollType::Class && professionRollType != ProfessionRollType::Random)
    {
        professionRollType = urand(1, 100) <= sPlayerbotAIConfig.ClassMatchingProfessionChance
                                 ? ProfessionRollType::Class
                                 : ProfessionRollType::Random;
        sRandomPlayerbotMgr.SetValue(bot, "professionRollType", static_cast<uint32>(professionRollType));
    }

    std::vector<WeightedProfessionPair> professionPairs = professionRollType == ProfessionRollType::Class
                                                              ? GetClassProfessionPairs(bot)
                                                              : GetRandomProfessionPairs();

    std::vector<uint16> knownPrimarySkills;
    for (uint32 tradeSkill : tradeSkills)
    {
        if (IsPrimaryTradeSkill(tradeSkill) && bot->HasSkill(tradeSkill))
            knownPrimarySkills.push_back(static_cast<uint16>(tradeSkill));
    }

    std::vector<uint16> primarySkills;
    auto addPrimarySkill = [&primarySkills, maxPrimaryTradeSkills](uint16 skillId)
    {
        if (!skillId || !IsPrimaryTradeSkill(skillId) || primarySkills.size() >= maxPrimaryTradeSkills)
            return;

        if (std::find(primarySkills.begin(), primarySkills.end(), skillId) == primarySkills.end())
            primarySkills.push_back(skillId);
    };

    auto isKnownPrimarySkill = [&knownPrimarySkills](uint16 skillId)
    {
        return std::find(knownPrimarySkills.begin(), knownPrimarySkills.end(), skillId) != knownPrimarySkills.end();
    };

    if (knownPrimarySkills.empty())
    {
        // A full randomization clears skills before this method. Keep a valid stored assignment stable and relearn it.
        addPrimarySkill(firstSkill);
        addPrimarySkill(secondSkill);
    }
    else if (knownPrimarySkills.size() == 1)
    {
        // The real skill is authoritative. Reuse a stored complement only when the stored pair contains that skill.
        uint16 const knownSkill = knownPrimarySkills.front();
        addPrimarySkill(knownSkill);

        if (firstSkill == knownSkill && secondSkill != knownSkill)
            addPrimarySkill(secondSkill);
        else if (secondSkill == knownSkill && firstSkill != knownSkill)
            addPrimarySkill(firstSkill);
    }
    else
    {
        // For contaminated bots, stored values are trusted only when the corresponding skill is actually present.
        if (isKnownPrimarySkill(firstSkill))
            addPrimarySkill(firstSkill);
        if (isKnownPrimarySkill(secondSkill))
            addPrimarySkill(secondSkill);

        for (uint16 skillId : knownPrimarySkills)
            addPrimarySkill(skillId);
    }

    if (primarySkills.empty() && maxPrimaryTradeSkills == 1)
        addPrimarySkill(ChooseSingleProfession(professionPairs));
    else if (primarySkills.empty() && maxPrimaryTradeSkills >= 2)
    {
        std::pair<uint16, uint16> const professionPair = ChooseProfessionPair(professionPairs);
        addPrimarySkill(professionPair.first);
        addPrimarySkill(professionPair.second);
    }
    else if (primarySkills.size() == 1 && maxPrimaryTradeSkills >= 2)
    {
        addPrimarySkill(ChooseComplementaryProfession(professionPairs, primarySkills.front()));

        // Defensive fallback for an empty or malformed pair table.
        if (primarySkills.size() == 1)
        {
            for (uint32 tradeSkill : tradeSkills)
            {
                if (IsPrimaryTradeSkill(tradeSkill) && tradeSkill != primarySkills.front())
                {
                    addPrimarySkill(static_cast<uint16>(tradeSkill));
                    break;
                }
            }
        }
    }

    firstSkill = primarySkills.empty() ? 0 : primarySkills[0];
    secondSkill = primarySkills.size() > 1 ? primarySkills[1] : 0;
    sRandomPlayerbotMgr.SetValue(bot, "firstSkill", firstSkill);
    sRandomPlayerbotMgr.SetValue(bot, "secondSkill", secondSkill);

    for (uint16 skillId : knownPrimarySkills)
    {
        if (std::find(primarySkills.begin(), primarySkills.end(), skillId) == primarySkills.end())
            bot->SetSkill(skillId, 0, 0, 0);
    }

    SetRandomSkill(SKILL_FIRST_AID);
    SetRandomSkill(SKILL_FISHING);
    SetRandomSkill(SKILL_COOKING);

    std::vector<uint16> skillsToLearn = {SKILL_FIRST_AID, SKILL_FISHING, SKILL_COOKING};
    skillsToLearn.insert(skillsToLearn.end(), primarySkills.begin(), primarySkills.end());

    uint32 selectedPrimaryStarterSpells = 0;
    for (uint16 skillId : primarySkills)
    {
        uint32 const starterSpellId = GetProfessionStarterSpell(skillId);
        if (starterSpellId && bot->HasSpell(starterSpellId))
            ++selectedPrimaryStarterSpells;
    }

    bot->SetFreePrimaryProfessions(
        static_cast<uint16>(maxPrimaryTradeSkills > selectedPrimaryStarterSpells
                                ? maxPrimaryTradeSkills - selectedPrimaryStarterSpells
                                : 0));

    for (uint16 skillId : skillsToLearn)
    {
        uint32 const spellId = GetProfessionStarterSpell(skillId);
        if (!spellId || bot->HasSpell(spellId))
            continue;

        if (IsPrimaryTradeSkill(skillId) && !bot->GetFreePrimaryProfessionPoints())
            continue;

        bot->learnSpell(spellId, false);
    }

    for (uint16 skillId : primarySkills)
        SetRandomSkill(skillId);

    bot->SetFreePrimaryProfessions(
        static_cast<uint16>(maxPrimaryTradeSkills > primarySkills.size()
                                ? maxPrimaryTradeSkills - primarySkills.size()
                                : 0));

    InitTradeSpecializations();
}

void PlayerbotFactory::InitTradeSpecializations()
{
    InitAlchemySpecialization();
    InitEngineeringSpecialization();
    InitLeatherworkingSpecialization();
    InitTailoringSpecialization();
    InitBlacksmithingSpecialization();
}

bool PlayerbotFactory::InitAlchemySpecialization()
{
    if (!bot->HasSkill(SKILL_ALCHEMY) ||
        bot->GetBaseSkillValue(SKILL_ALCHEMY) < 325 ||
        bot->GetLevel() <= 67)
        return false;

    if (HasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Transmute),
                          static_cast<uint32>(ProfessionSpecializationSpell::Elixir),
                          static_cast<uint32>(ProfessionSpecializationSpell::Potion)}))
        return false;

    switch (GetStoredOrRandomValue(bot, "alchemySpecialization", 1, 3))
    {
        case 1:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Transmute,
                                                 ProfessionSpecializationSpell::LearnTransmute);
        case 2:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Elixir,
                                                 ProfessionSpecializationSpell::LearnElixir);
        case 3:
        default:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Potion,
                                                 ProfessionSpecializationSpell::LearnPotion);
    }
}

bool PlayerbotFactory::InitEngineeringSpecialization()
{
    if (!bot->HasSkill(SKILL_ENGINEERING) ||
        bot->GetBaseSkillValue(SKILL_ENGINEERING) < 200 ||
        bot->GetLevel() < 30)
        return false;

    if (HasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Goblin),
                          static_cast<uint32>(ProfessionSpecializationSpell::Gnomish)}))
        return false;

    switch (GetStoredOrRandomValue(bot, "engineeringSpecialization", 1, 2))
    {
        case 1:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Goblin,
                                                 ProfessionSpecializationSpell::LearnGoblin);
        case 2:
        default:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Gnomish,
                                                 ProfessionSpecializationSpell::LearnGnomish);
    }
}

bool PlayerbotFactory::InitLeatherworkingSpecialization()
{
    if (!bot->HasSkill(SKILL_LEATHERWORKING) ||
        bot->GetBaseSkillValue(SKILL_LEATHERWORKING) < 225 ||
        bot->GetLevel() <= 40)
        return false;

    if (HasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Dragon),
                          static_cast<uint32>(ProfessionSpecializationSpell::Elemental),
                          static_cast<uint32>(ProfessionSpecializationSpell::Tribal)}))
        return false;

    switch (GetStoredOrRandomValue(bot, "leatherSpecialization", 1, 3))
    {
        case 1:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Dragon,
                                                 ProfessionSpecializationSpell::LearnDragon);
        case 2:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Elemental,
                                                 ProfessionSpecializationSpell::LearnElemental);
        case 3:
        default:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Tribal,
                                                 ProfessionSpecializationSpell::LearnTribal);
    }
}

bool PlayerbotFactory::InitTailoringSpecialization()
{
    if (!bot->HasSkill(SKILL_TAILORING) ||
        bot->GetBaseSkillValue(SKILL_TAILORING) < 350 ||
        bot->GetLevel() <= 59)
        return false;

    if (HasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Spellfire),
                          static_cast<uint32>(ProfessionSpecializationSpell::Mooncloth),
                          static_cast<uint32>(ProfessionSpecializationSpell::Shadoweave)}))
        return false;

    switch (GetStoredOrRandomValue(bot, "tailorSpecialization", 1, 3))
    {
        case 1:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Spellfire,
                                                 ProfessionSpecializationSpell::LearnSpellfire);
        case 2:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Mooncloth,
                                                 ProfessionSpecializationSpell::LearnMooncloth);
        case 3:
        default:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Shadoweave,
                                                 ProfessionSpecializationSpell::LearnShadoweave);
    }
}

bool PlayerbotFactory::InitBlacksmithingSpecialization()
{
    bool learnedSpecialization = false;

    if (!bot->HasSkill(SKILL_BLACKSMITHING) ||
        bot->GetBaseSkillValue(SKILL_BLACKSMITHING) < 225)
        return false;

    if (!bot->HasSpell(static_cast<uint32>(ProfessionSpecializationSpell::Armor)) &&
        !bot->HasSpell(static_cast<uint32>(ProfessionSpecializationSpell::Weapon)))
    {
        switch (GetStoredOrRandomValue(bot, "blacksmithSpecialization", 1, 2))
        {
            case 1:
                learnedSpecialization = LearnProfessionSpecialization(bot,
                                                                      ProfessionSpecializationSpell::Armor,
                                                                      ProfessionSpecializationSpell::LearnArmor);
                break;
            case 2:
            default:
                learnedSpecialization = LearnProfessionSpecialization(bot,
                                                                      ProfessionSpecializationSpell::Weapon,
                                                                      ProfessionSpecializationSpell::LearnWeapon);
                break;
        }
    }

    if (!bot->HasSpell(static_cast<uint32>(ProfessionSpecializationSpell::Weapon)) ||
        bot->GetBaseSkillValue(SKILL_BLACKSMITHING) < 250 ||
        bot->GetLevel() <= 49 ||
        HasAnySpell(bot, {static_cast<uint32>(ProfessionSpecializationSpell::Hammer),
                          static_cast<uint32>(ProfessionSpecializationSpell::Axe),
                          static_cast<uint32>(ProfessionSpecializationSpell::Sword)}))
        return learnedSpecialization;

    switch (GetStoredOrRandomValue(bot, "blacksmithWeaponSpecialization", 1, 3))
    {
        case 1:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Hammer,
                                                 ProfessionSpecializationSpell::LearnHammer);
        case 2:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Axe,
                                                 ProfessionSpecializationSpell::LearnAxe);
        case 3:
        default:
            return LearnProfessionSpecialization(bot,
                                                 ProfessionSpecializationSpell::Sword,
                                                 ProfessionSpecializationSpell::LearnSword);
    }
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
    if (bot->GetLevel() >= sPlayerbotAIConfig.UseGroundMountAtMinLevel)
        bot->learnSpell(33388);
    if (bot->GetLevel() >= sPlayerbotAIConfig.UseFastGroundMountAtMinLevel)
        bot->learnSpell(33391);
    if (bot->GetLevel() >= sPlayerbotAIConfig.UseFlyMountAtMinLevel)
        bot->learnSpell(34090);
    if (bot->GetLevel() >= sPlayerbotAIConfig.UseFastFlyMountAtMinLevel)
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

    InitTradeSkills();
    InitInventorySkill();

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
            Trainer::Trainer* trainer = sObjectMgr->GetTrainer(i->first);

            if (!trainer)
                continue;

            if (trainer->GetTrainerType() != Trainer::Type::Tradeskill &&
                trainer->GetTrainerType() != Trainer::Type::Class)
                continue;

            if (trainer->GetTrainerType() == Trainer::Type::Class &&
                !trainer->IsTrainerValidForPlayer(bot))
                continue;

            trainerIdCache[bot->getClass()].push_back(i->first);
        }
    }
    for (uint32 trainerId : trainerIdCache[bot->getClass()])
    {
        Trainer::Trainer* trainer = sObjectMgr->GetTrainer(trainerId);

        for (auto& spell : trainer->GetSpells())
        {
            // simplified version of Trainer::TeachSpell method

            Trainer::Spell const* trainerSpell = trainer->GetSpell(spell.SpellId);
            if (!trainerSpell)
                continue;

            if (!IsTrainerSpellAllowedForBot(bot, trainer, trainerSpell))
                continue;

            if (!trainer->CanTeachSpell(bot, trainerSpell))
                continue;

            if (trainerSpell->IsCastable())
                bot->CastSpell(bot, trainerSpell->SpellId, true);
            else
                bot->learnSpell(trainerSpell->SpellId, false);
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
                bot->learnSpell(2458, false);  // Berserker Stance
            break;
        case CLASS_PALADIN:
            bot->learnSpell(21084, true);
            bot->learnSpell(635, true);
            if (level >= 12)
                bot->learnSpell(7328, false);  // Redemption
            if (level >= 20)
                bot->learnSpell(5502, false); // Sense Undead
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
                bot->learnSpell(697, false);  // summon voidwalker
            if (level >= 20)
                bot->learnSpell(712, false);  // summon succubus
            if (level >= 30)
                bot->learnSpell(691, false);  // summon felhunter
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
                bot->learnSpell(8071, false);  // stoneskin totem
            if (level >= 10)
                bot->learnSpell(3599, false);  // searing totem
            if (level >= 20)
                bot->learnSpell(5394, false);  // healing stream totem
            break;
        default:
            break;
    }
}

void PlayerbotFactory::InitSpecialSpells()
{
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig.RandomBotSpellIds.begin();
         i != sPlayerbotAIConfig.RandomBotSpellIds.end(); ++i)
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
    std::map<uint32, std::vector<TalentEntry const*>> spells;
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
            for (uint32 rank = 0; rank < std::min((uint32)MAX_TALENT_RANK, bot->GetFreeTalentPoints()); ++rank)
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
    // if (sPlayerbotAIConfig.ParsedSpecLinkOrder[bot->getClass()][specNo][80].size() == 0)
    // {
    //     return;
    // }
    uint32 cls = bot->getClass();
    int startLevel = bot->GetLevel();
    uint32 specIndex = sPlayerbotAIConfig.RandomClassSpecIndex[cls][specTab];
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
           sPlayerbotAIConfig.ParsedSpecLinkOrder[cls][specIndex][startLevel].size() == 0)
    {
        startLevel--;
    }
    for (int level = startLevel; level <= 80; level++)
    {
        if (sPlayerbotAIConfig.ParsedSpecLinkOrder[cls][specIndex][level].size() == 0)
        {
            continue;
        }
        for (std::vector<uint32>& p : sPlayerbotAIConfig.ParsedSpecLinkOrder[cls][specIndex][level])
        {
            uint32 tab = p[0], row = p[1], col = p[2], lvl = p[3];
            if (sPlayerbotAIConfig.LimitTalentsExpansion && bot->GetLevel() <= 60 && (row > 6 || (row == 6 && col != 1)))
                continue;

            if (sPlayerbotAIConfig.LimitTalentsExpansion && bot->GetLevel() <= 70 && (row > 8 || (row == 8 && col != 1)))
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
    for (std::vector<uint32>::iterator i = sPlayerbotAIConfig.RandomBotAccounts.begin();
         i != sPlayerbotAIConfig.RandomBotAccounts.end(); i++)
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
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_HUNTER && botClass != CLASS_ROGUE && botClass != CLASS_WARRIOR)
        return;

    Item const* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!item)
        return;

    ItemTemplate const* proto = item->GetTemplate();
    if (!proto)
        return;

    uint32 subClass = 0;
    switch (proto->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_GUN:
            subClass = ITEM_SUBCLASS_BULLET;
            break;
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            subClass = ITEM_SUBCLASS_ARROW;
            break;
        default: // invalid weapon type, nothing to do
            return;
    }

    uint32 entry = sRandomItemMgr.GetAmmo(level, subClass);
    if (!entry)
        return;

    uint32 count = bot->GetItemCount(entry);
    uint32 maxCount = botClass == CLASS_HUNTER ? 6000 : 1000;

    if (count < maxCount)
    {
        if (Item* newItem = StoreNewItemInInventorySlot(bot, entry, maxCount - count))
            newItem->AddToUpdateQueueOf(bot);
    }

    bot->SetAmmo(entry);
}

uint32 PlayerbotFactory::CalcMixedGearScore(uint32 gs, uint32 quality)
{
    return gs * PlayerbotAI::GetItemScoreMultiplier(ItemQualities(quality));
}

void PlayerbotFactory::DestroyEquippedGear(Player* bot)
{
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;
        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
    }
}

void PlayerbotFactory::AutoGear(Player* bot, uint32 itemQuality, uint32 ilvl, bool incremental, bool secondChance,
                                bool applyFinishers)
{
    uint32 gs = ilvl == 0 ? 0 : CalcMixedGearScore(ilvl, itemQuality);
    PlayerbotFactory factory(bot, bot->GetLevel(), itemQuality, gs);
    factory.InitEquipment(incremental, secondChance);

    if (!applyFinishers)
        return;

    factory.InitAmmo();
    if (bot->GetLevel() >= sPlayerbotAIConfig.MinEnchantingBotLevel)
        factory.ApplyEnchantAndGemsNew();
    bot->DurabilityRepairAll(false, 1.0f, false);
}

void PlayerbotFactory::InitMounts()
{
    uint32 firstmount = sPlayerbotAIConfig.UseGroundMountAtMinLevel;
    uint32 secondmount = sPlayerbotAIConfig.UseFastGroundMountAtMinLevel;
    uint32 thirdmount = sPlayerbotAIConfig.UseFlyMountAtMinLevel;
    uint32 fourthmount = sPlayerbotAIConfig.UseFastFlyMountAtMinLevel;

    if (bot->GetLevel() < firstmount)
        return;

    uint32 const ridingSkill = bot->GetPureSkillValue(SKILL_RIDING);
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
        default:
            if (bot->GetTeamId() == TEAM_HORDE)
            { // Orc mounts
                slow = {6654, 6653, 580};
                fast = {23250, 23252, 23251};
            }
            else // Human mounts
            {
                slow = {470, 6648, 458, 472};
                fast = {23228, 23227, 23229};
            }
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
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if (spellInfo && PlayerbotSpellRepository::Instance().GetRequiredRidingSkill(spellInfo) > ridingSkill)
            continue;

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

        uint32 itemId = sRandomItemMgr.GetRandomPotion(level, effect);
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

        if (proto->RequiredLevel > bot->GetLevel() ||
            static_cast<int32>(proto->RequiredLevel) < static_cast<int32>(bot->GetLevel()) - 9)
            continue;

        if (proto->RequiredSkill && !bot->HasSkill(proto->RequiredSkill))
            continue;

        if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
            continue;

        items[proto->Spells[0].SpellCategory].push_back(itemId);
    }

    uint32 categories[] = {11, 59};
    for (size_t i = 0; i < sizeof(categories) / sizeof(uint32); ++i)
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
                items.push_back({21177, 100});   // Symbol of Kings
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
        {
            HasRelicBySubclassVisitor relicVisitor(ITEM_SUBCLASS_ARMOR_TOTEM);
            IterateItems(&relicVisitor, (IterateItemsMask)(ITERATE_ITEMS_IN_BAGS | ITERATE_ITEMS_IN_EQUIP));
            bool hasRelic = relicVisitor.found;

            if (!hasRelic)
            {
                if (level >= 4)
                    items.push_back({5175, 1});  // Earth Totem
                if (level >= 10)
                    items.push_back({5176, 1});  // Flame Totem
                if (level >= 20)
                    items.push_back({5177, 1});  // Water Totem
            }
            else
            {
                ItemIds totemIds = {5175, 5176, 5177, 5178};
                FindItemByIdsVisitor totemVisitor(totemIds);
                IterateItems(&totemVisitor, (IterateItemsMask)(ITERATE_ITEMS_IN_BAGS | ITERATE_ITEMS_IN_EQUIP | ITERATE_ITEMS_IN_BANK));
                for (Item* item : totemVisitor.GetResult())
                    bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
            }
            if (level >= 30)
            {
                if (!hasRelic)
                    items.push_back({5178, 1});  // Air Totem
                items.push_back({17030, 20});  // Ankh
            }
            break;
        }
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

    if (sPlayerbotAIConfig.LimitTalentsExpansion && bot->GetLevel() <= 70)
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
        if (bot->HasAura(SPELL_SECOND_WIND))
            tab = 3;
        // Fury PvP (spec index 4): If the bot has the Blood Craze talent
        else if (bot->HasAura(SPELL_BLOOD_CRAZE))
            tab = 4;
        // Protection PvP (spec index 5): If the bot has the Gag Order talent
        else if (bot->HasAura(SPELL_GAG_ORDER))
            tab = 5;
    }

    // Paladin PvP exceptions
    if (bot->getClass() == CLASS_PALADIN)
    {
        // Holy PvP (spec index 3): If the bot has the Sacred Cleansing talent
        if (bot->HasAura(SPELL_SACRED_CLEANSING))
            tab = 3;
        // Protection PvP (spec index 4): If the bot has the Reckoning talent
        else if (bot->HasAura(SPELL_RECKONING))
            tab = 4;
        // Retribution PvP (spec index 5): If the bot has the Divine Purpose talent
        else if (bot->HasAura(SPELL_DIVINE_PURPOSE))
            tab = 5;
    }

    // Hunter PvP exceptions
    if (bot->getClass() == CLASS_HUNTER)
    {
        // Beast Mastery PvP (spec index 3): If the bot has the Thick Hide talent
        if (bot->HasAura(SPELL_HUNTER_THICK_HIDE))
            tab = 3;
        // Marksmanship PvP (spec index 4): If the bot has the Concussive Barrage talent
        else if (bot->HasAura(SPELL_CONCUSSIVE_BARRAGE))
            tab = 4;
        // Survival PvP (spec index 5): If the bot has the Entrapment talent and does NOT have the Concussive Barrage talent
        else if (bot->HasAura(SPELL_ENTRAPMENT) && !bot->HasAura(SPELL_CONCUSSIVE_BARRAGE))
            tab = 5;
    }

    // Rogue PvP exceptions
    if (bot->getClass() == CLASS_ROGUE)
    {
        // Assassination PvP (spec index 3): If the bot has the Deadly Brew talent
        if (bot->HasAura(SPELL_DEADLY_BREW))
            tab = 3;
        // Combat PvP (spec index 4): If the bot has the Throwing Specialization talent
        else if (bot->HasAura(SPELL_THROWING_SPECIALIZATION))
            tab = 4;
        // Subtlety PvP (spec index 5): If the bot has the Waylay talent
        else if (bot->HasAura(SPELL_WAYLAY))
            tab = 5;
    }

    // Priest PvP exceptions
    if (bot->getClass() == CLASS_PRIEST)
    {
        // Discipline PvP (spec index 3): If the bot has the Improved Mana Burn talent
        if (bot->HasAura(SPELL_IMPROVED_MANA_BURN))
            tab = 3;
        // Holy PvP (spec index 4): If the bot has the Body and Soul talent
        else if (bot->HasAura(SPELL_BODY_AND_SOUL))
            tab = 4;
        // Shadow PvP (spec index 5): If the bot has the Improved Vampiric Embrace talent
        else if (bot->HasAura(SPELL_IMPROVED_VAMPIRIC_EMBRACE))
            tab = 5;
    }

    // Death Knight PvE/PvP exceptions
    if (bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        // Double Aura Blood PvE (spec index 3): If the bot has both the Abomination's Might and Improved Icy Talons
        // talents
        if (bot->HasAura(SPELL_ABOMINATIONS_MIGHT) && bot->HasAura(SPELL_IMPROVED_ICY_TALONS))
            tab = 3;
        // Blood PvP (spec index 4): If the bot has the Sudden Doom talent
        else if (bot->HasAura(SPELL_SUDDEN_DOOM))
            tab = 4;
        // Frost PvP (spec index 5): If the bot has the Acclimation talent
        else if (bot->HasAura(SPELL_ACCLIMATION))
            tab = 5;
        // Unholy PvP (spec index 6): If the bot has the Magic Suppression talent
        else if (bot->HasAura(SPELL_MAGIC_SUPPRESSION))
            tab = 6;
    }

    // Shaman PvP exceptions
    if (bot->getClass() == CLASS_SHAMAN)
    {
        // Elemental PvP (spec index 3): If the bot has the Astral Shift talent
        if (bot->HasAura(SPELL_ASTRAL_SHIFT))
            tab = 3;
        // Enhancement PvP (spec index 4): If the bot has the Earthen Power talent
        else if (bot->HasAura(SPELL_EARTHEN_POWER))
            tab = 4;
        // Restoration PvP (spec index 5): If the bot has the Focused Mind talent
        else if (bot->HasAura(SPELL_FOCUSED_MIND))
            tab = 5;
    }

    // Mage PvE/PvP exceptions
    if (bot->getClass() == CLASS_MAGE)
    {
        // Frostfire PvE (spec index 3): If the bot has both the Burnout talent and the Ice Shards talent
        if (bot->HasAura(SPELL_BURNOUT) && bot->HasAura(SPELL_ICE_SHARDS))
            tab = 3;
        // Arcane PvP (spec index 4): If the bot has the Improved Blink talent
        else if (bot->HasAura(SPELL_IMPROVED_BLINK))
            tab = 4;
        // Fire PvP (spec index 5): If the bot has the Fiery Payback talent
        else if (bot->HasAura(SPELL_FIERY_PAYBACK))
            tab = 5;
        // Frost PvP (spec index 6): If the bot has the Shattered Barrier talent
        else if (bot->HasAura(SPELL_SHATTERED_BARRIER))
            tab = 6;
    }

    // Warlock PvP exceptions
    if (bot->getClass() == CLASS_WARLOCK)
    {
        // Affliction PvP (spec index 3): If the bot has the Improved Howl of Terror talent
        if (bot->HasAura(SPELL_IMPROVED_HOWL_OF_TERROR))
            tab = 3;
        // Demonology PvP (spec index 4): If the bot has both the Nemesis talent and the Intensity talent
        else if (bot->HasAura(SPELL_NEMESIS) && bot->HasAura(SPELL_INTENSITY))
            tab = 4;
        // Destruction PvP (spec index 5): If the bot has the Nether Protection talent
        else if (bot->HasAura(SPELL_NETHER_PROTECTION))
            tab = 5;
    }

    // Druid PvE/PvP exceptions
    if (bot->getClass() == CLASS_DRUID)
    {
        // Cat PvE (spec index 3): If the bot is Feral spec, level 20 or higher, and does NOT have the Thick Hide talent
        if (tab == DRUID_TAB_FERAL && bot->GetLevel() >= 20 && !bot->HasAura(SPELL_DRUID_THICK_HIDE))
            tab = 3;
        // Balance PvP (spec index 4): If the bot has the Owlkin Frenzy talent
        else if (bot->HasAura(SPELL_OWLKIN_FRENZY))
            tab = 4;
        // Feral PvP (spec index 5): If the bot has the Primal Tenacity talent
        else if (bot->HasAura(SPELL_PRIMAL_TENACITY))
            tab = 5;
        // Resto PvP (spec index 6): If the bot has the Improved Barkskin talent
        else if (bot->HasAura(SPELL_IMPROVED_BARKSKIN))
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
        if (sPlayerbotAIConfig.ParsedSpecGlyph[cls][tab].size() > slotIndex &&
            sPlayerbotAIConfig.ParsedSpecGlyph[cls][tab][slotIndex] != 0)
        {
            uint32 itemId = sPlayerbotAIConfig.ParsedSpecGlyph[cls][tab][slotIndex];
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
    if (bot->HasSkill(SKILL_MINING) && !bot->HasItemCount(2901, 1, true))
        StoreItem(2901, 1);  // Mining Pick

    if ((bot->HasSkill(SKILL_BLACKSMITHING) || bot->HasSkill(SKILL_ENGINEERING)) &&
        !bot->HasItemCount(5956, 1, true))
        StoreItem(5956, 1);  // Blacksmith Hammer

    if (bot->HasSkill(SKILL_ENGINEERING) && !bot->HasItemCount(6219, 1, true))
        StoreItem(6219, 1);  // Arclight Spanner

    if (bot->HasSkill(SKILL_ENCHANTING) && !bot->HasItemCount(16207, 1, true))
        StoreItem(16207, 1);  // Runed Arcanite Rod

    if (bot->HasSkill(SKILL_SKINNING) && !bot->HasItemCount(7005, 1, true))
        StoreItem(7005, 1);  // Skinning Knife
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
    uint32 itemId = sRandomItemMgr.GetRandomTrade(level);
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
    if (urand(0, 100) < 100 * sPlayerbotAIConfig.RandomGearLoweringChance && desiredQuality > ITEM_QUALITY_NORMAL)
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
    {
        if (!bot->HasItemCount(5976, 1) && bot->GetLevel() > 9)
            StoreItem(5976, 1);
        return;
    }

    if (sPlayerbotAIConfig.DeleteRandomBotGuilds)
        return;

    std::string guildName = PlayerbotGuildMgr::instance().AssignToGuild(bot);
    if (guildName.empty())
        return;

    Guild* guild = sGuildMgr->GetGuildByName(guildName);
    if (!guild)
    {
        if (!PlayerbotGuildMgr::instance().CreateGuild(bot, guildName))
            LOG_ERROR("playerbots","Failed to create guild {} for bot {}", guildName, bot->GetName());
        return;
    }
    else
    {
        if (guild->AddMember(bot->GetGUID(),urand(GR_OFFICER, GR_INITIATE)))
            PlayerbotGuildMgr::instance().OnGuildUpdate(guild);
        else
            LOG_ERROR("playerbots","Bot {} failed to join guild {}.", bot->GetName(), guildName);
    }
    // add guild tabard
    if (bot->GetGuildId() && bot->GetLevel() > 9 && urand(0, 4) && !bot->HasItemCount(5976, 1))
        StoreItem(5976, 1);
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

        uint32 value = sRandomPlayerbotMgr.GetValue(owner, name.str());
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
            if (from != to && static_cast<int32>(percentMap[to]) + delta <= 100 &&
                static_cast<int32>(percentMap[from]) - delta >= 0)
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
            sRandomPlayerbotMgr.SetValue(owner, name.str(), percentMap[type]);
        }
    }
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

void PlayerbotFactory::ApplyEnchantAndGemsNew(bool /*destroyOld*/)
{
    std::vector<uint32> curCount = GetCurrentGemsCount();
    uint8 jewelersCount = 0;
    int requiredActive = 2;
    std::vector<uint32> availableGems;
    for (uint32 const& enchantGem : enchantGemIdCache)
    {
        ItemTemplate const* gemTemplate = sObjectMgr->GetItemTemplate(enchantGem);
        if (!gemTemplate)
            continue;

        GemPropertiesEntry const* gemProperties = sGemPropertiesStore.LookupEntry(gemTemplate->GemProperties);
        if (!gemProperties)
            continue;

        if (sPlayerbotAIConfig.LimitEnchantExpansion && bot->GetLevel() <= 70 && enchantGem >= 39900)
            continue;

        uint32 requiredLevel = gemTemplate->ItemLevel;

        if (requiredLevel > bot->GetLevel())
            continue;

        uint32 enchant_id = gemProperties->spellitemenchantement;
        if (!enchant_id)
            continue;

        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!enchant || (enchant->slot != PERM_ENCHANTMENT_SLOT && enchant->slot != TEMP_ENCHANTMENT_SLOT))
            continue;

        if (enchant->requiredSkill && bot->GetSkillValue(enchant->requiredSkill) < enchant->requiredSkillValue)
            continue;

        if (enchant->requiredLevel > bot->GetLevel())
            continue;

        availableGems.push_back(enchantGem);
    }
    StatsWeightCalculator calculator(bot);
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
            continue;
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item || !item->GetOwner())
            continue;

        if (item->GetTemplate() && item->GetTemplate()->Quality < ITEM_QUALITY_UNCOMMON)
            continue;
        int32 bestEnchantId = -1;
        float bestScore = 0;
        for (uint32 const& enchantSpell : enchantSpellIdCache)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(enchantSpell);
            if (!spellInfo)
                continue;

            if (!item->IsFitToSpellRequirements(spellInfo))
                continue;

            uint32 requiredLevel = spellInfo->BaseLevel;
            if (requiredLevel > bot->GetLevel())
                continue;

            // disable next expansion enchantments
            if (sPlayerbotAIConfig.LimitEnchantExpansion && bot->GetLevel() <= 60 && enchantSpell >= 27899)
                continue;

            if (sPlayerbotAIConfig.LimitEnchantExpansion && bot->GetLevel() <= 70 && enchantSpell >= 44483)
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
                    continue;

                if (enchant->requiredSkill &&
                    (!bot->HasSkill(enchant->requiredSkill) ||
                     (bot->GetSkillValue(enchant->requiredSkill) < enchant->requiredSkillValue)))
                {
                    continue;
                }
                if (enchant->requiredLevel > bot->GetLevel())
                    continue;

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
        // Before gemming: on the belt, make sure the buckle socket is there.
        if (slot == EQUIPMENT_SLOT_WAIST)
            TryApplyBeltBuckle(bot, item);

        // HasSocket() covers both the template sockets and the one a buckle adds.
        if (!item->HasSocket())
            continue;

        // A buckle's gem goes into the first colourless template socket - see WorldSession::HandleSocketOpcode.
        uint8 firstPrismatic = 0;
        while (firstPrismatic < MAX_GEM_SOCKETS && item->GetTemplate()->Socket[firstPrismatic].Color)
            ++firstPrismatic;

        bool const hasPrismaticSocket = item->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT) != 0;

        for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS;
             ++enchant_slot)
        {
            uint32 socketIndex = enchant_slot - SOCK_ENCHANTMENT_SLOT;
            uint8 socketColor = item->GetTemplate()->Socket[socketIndex].Color;

            // The buckle socket takes any gem the meta check below lets through.
            bool const isPrismatic = !socketColor && hasPrismaticSocket && socketIndex == firstPrismatic;
            if (!socketColor && !isPrismatic)
                continue;

            int32 enchantIdChosen = -1;
            bool jewelersGemChosen;
            float bestGemScore = -1;
            for (uint32& enchantGem : availableGems)
            {
                ItemTemplate const* gemTemplate = sObjectMgr->GetItemTemplate(enchantGem);
                if (!gemTemplate)
                    continue;

                if (gemTemplate->Quality > item->GetTemplate()->Quality)
                    continue;

                // Limit jewelers (JC) epic gems to 3
                bool isJewelersGem = gemTemplate->ItemLimitCategory == 2;
                if (isJewelersGem && jewelersCount >= 3)
                    continue;

                GemPropertiesEntry const* gemProperties = sGemPropertiesStore.LookupEntry(gemTemplate->GemProperties);
                if (!gemProperties)
                    continue;

                if ((socketColor & gemProperties->color) == 0 && gemProperties->color == 1)  // meta socket
                    continue;

                uint32 enchant_id = gemProperties->spellitemenchantement;
                if (!enchant_id)
                    continue;

                StatsWeightCalculator calculator(bot);
                float score = calculator.CalculateEnchant(enchant_id);
                if (curCount[0] != 0)
                {
                    // Ensure meta gem activation
                    for (size_t i = 1; i < curCount.size(); i++)
                    {
                        if (curCount[i] < (uint32)requiredActive && (gemProperties->color & (1 << i)))
                        {
                            score *= 2;
                            break;
                        }
                    }
                }
                if (socketColor & gemProperties->color)
                    score *= 1.2f;
                if (score > bestGemScore)
                {
                    enchantIdChosen = enchant_id;
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

    // Complete all level-appropriate attunement quests for the bot
    if (level >= 60)
    {
        std::list<uint32> questsToComplete;

        // Check each quest status before adding to the completion list
        for (uint32 questId : sPlayerbotAIConfig.AttunementQuests)
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
