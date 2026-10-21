/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingProcessingActions.h"

#include <unordered_set>

#include "ItemUsageValue.h"
#include "LivingEconomyActions.h"
#include "LivingEconomyPolicy.h"
#include "LivingFieldActions.h"
#include "LivingProfessionActions.h"
#include "LivingWorldMgr.h"
#include "LootObjectStack.h"
#include "Playerbots.h"

namespace
{
struct ProcessingSpell
{
    uint32 spell;
    uint32 skill;
    uint32 batch;
};

constexpr std::array<ProcessingSpell, 3> ProcessingSpells{{
    {13262, SKILL_ENCHANTING, 1},
    {31252, SKILL_JEWELCRAFTING, 5},
    {51005, SKILL_INSCRIPTION, 5},
}};

bool CanProcess(PlayerbotAI* ai)
{
    auto* context = ai->GetAiObjectContext();
    Player* player = ai->GetBot();
    return LivingProfessions::CanWork(ai) && !player->isMoving() && !player->GetLootGUID() &&
           !context->GetValue<LootObjectStack*>("available loot")->Get()->IsLootPending() &&
           context->GetValue<uint8>("bag space")->Get() < 80;
}

std::unordered_set<uint32> QuestItems(Player* player)
{
    std::unordered_set<uint32> items;
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        if (auto const* quest = sObjectMgr->GetQuestTemplate(player->GetQuestSlotQuestId(slot)))
        {
            items.insert(quest->GetSrcItemId());
            for (uint32 item : quest->RequiredItemId)
                items.insert(item);
            for (uint32 item : quest->ItemDrop)
                items.insert(item);
        }
    return items;
}

bool CanLearnedProcess(Player* player, ProcessingSpell const& process)
{
    auto const learned = player->GetSpellMap().find(process.spell);
    return LivingWorld::FindProfession(process.skill, sPlayerbotAIConfig.RandomBotMaxLevel) &&
           player->HasSkill(process.skill) && learned != player->GetSpellMap().end() && learned->second &&
           learned->second->State != PLAYERSPELL_REMOVED && learned->second->Active;
}

bool EligibleItem(PlayerbotAI* ai, Item* item, ProcessingSpell const& process, LivingProfessions::Stock const& stock,
                  std::unordered_set<uint32> const& quests)
{
    Player* player = ai->GetBot();
    if (!item || LivingProfessions::FieldReservedItem(ai, item->GetGUID()) ||
        item->GetOwnerGUID() != player->GetGUID() || item->IsEquipped() || item->IsInTrade() || item->IsWrapped() ||
        item->IsRefundable() || item->IsBOPTradable() || item->m_lootGenerated ||
        item->GetUInt32Value(ITEM_FIELD_DURATION))
        return false;
    bool const backpack = item->GetBagSlot() == INVENTORY_SLOT_BAG_0 && item->GetSlot() >= INVENTORY_SLOT_ITEM_START &&
                          item->GetSlot() < INVENTORY_SLOT_ITEM_END;
    bool const bag = item->GetBagSlot() >= INVENTORY_SLOT_BAG_START && item->GetBagSlot() < INVENTORY_SLOT_BAG_END;
    if (!backpack && !bag)
        return false;
    auto const* proto = item->GetTemplate();
    if (quests.contains(item->GetEntry()) || proto->Class == ITEM_CLASS_QUEST || proto->StartQuest ||
        proto->Bonding == BIND_QUEST_ITEM || proto->HasFlag(ITEM_FLAG_CONJURED) || proto->TotemCategory ||
        (proto->Class == ITEM_CLASS_WEAPON &&
         (proto->SubClass == ITEM_SUBCLASS_WEAPON_MISC || proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)))
        return false;
    if (process.skill == SKILL_JEWELCRAFTING && !proto->HasFlag(ITEM_FLAG_IS_PROSPECTABLE))
        return false;
    if (process.skill == SKILL_INSCRIPTION && !proto->HasFlag(ITEM_FLAG_IS_MILLABLE))
        return false;
    if (process.skill == SKILL_ENCHANTING)
    {
        if (!proto->DisenchantID || proto->RequiredLevel > player->GetLevel() ||
            (proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON))
            return false;
        // The shared evaluator does not compare random-property variants of the same equipped entry.
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
            if (Item* equipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                if (equipped->GetEntry() == item->GetEntry())
                    return false;
        // Keep invested-in gear; valuation of enchants, sockets and alternate sets is separate work.
        for (uint8 slot = PERM_ENCHANTMENT_SLOT; slot <= SOCK_ENCHANTMENT_SLOT_3; ++slot)
            if (item->GetEnchantmentId(EnchantmentSlot(slot)))
                return false;
    }
    auto const it = stock.reserve.find(item->GetEntry());
    uint32 const reserve = it == stock.reserve.end() ? 0 : it->second;
    if (!LivingWorld::CanProcessBatch(player->GetItemCount(item->GetEntry(), false), reserve, item->GetCount(),
                                      process.batch))
        return false;
    // Recalculate immediately before destructive processing: equipment/quest state can change after planning.
    ItemUsageValue usage(ai);
    usage.Qualify(std::to_string(item->GetEntry()) + "," + std::to_string(item->GetItemRandomPropertyId()));
    ItemUsage const purpose = usage.Calculate();
    if (process.skill == SKILL_ENCHANTING)
    {
        if (purpose != ITEM_USAGE_DISENCHANT && purpose != ITEM_USAGE_VENDOR && purpose != ITEM_USAGE_AH)
            return false;
    }
    else if (purpose != ITEM_USAGE_SKILL && purpose != ITEM_USAGE_KEEP && purpose != ITEM_USAGE_AH &&
             purpose != ITEM_USAGE_VENDOR && purpose != ITEM_USAGE_NONE)
        return false;
    return ai->CanCastSpell(process.spell, player, true, item);
}
}  // namespace

bool LivingProfessions::IsProcessingLoot(Player* player, ObjectGuid guid)
{
    if (!LivingWorldMgr::instance().IsEnabled(player) || !guid.IsItem())
        return false;
    Item* item = player->GetItemByGuid(guid);
    return item && item->m_lootGenerated &&
           (item->loot.loot_type == LOOT_DISENCHANTING || item->loot.loot_type == LOOT_PROSPECTING ||
            item->loot.loot_type == LOOT_MILLING);
}

LivingProcessingPlan LivingProcessingValue::Calculate()
{
    if (!CanProcess(botAI))
        return {};
    auto const stock = LivingProfessions::ProfessionStock(bot);
    auto const quests = QuestItems(bot);
    auto const items = LivingProfessions::BagItems(bot);
    for (auto const& process : ProcessingSpells)
    {
        if (!CanLearnedProcess(bot, process))
            continue;
        for (Item* item : items)
            if (EligibleItem(botAI, item, process, stock, quests))
                return {process.spell, item->GetGUID()};
    }
    return {};
}

bool LivingProcessingAction::isUseful() { return CanProcess(botAI); }
bool LivingProcessingAction::isPossible() { return AI_VALUE(LivingProcessingPlan, "living processing").spell != 0; }

bool LivingProcessingAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;
    auto* value = context->GetValue<LivingProcessingPlan>("living processing");
    auto const plan = value->Get();
    value->Set({});
    auto const process = std::find_if(ProcessingSpells.begin(), ProcessingSpells.end(),
                                      [&plan](auto const& candidate) { return candidate.spell == plan.spell; });
    if (process == ProcessingSpells.end() || !CanLearnedProcess(bot, *process))
        return false;
    Item* item = bot->GetItemByGuid(plan.item);
    if (!EligibleItem(botAI, item, *process, LivingProfessions::ProfessionStock(bot), QuestItems(bot)))
        return false;
    return botAI->CastSpell(plan.spell, bot, item);
}
