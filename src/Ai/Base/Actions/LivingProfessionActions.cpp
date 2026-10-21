/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingProfessionActions.h"

#include "ItemUsageValue.h"
#include "LivingProfessionPolicy.h"
#include "LivingWorldMgr.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "Trainer.h"

namespace LivingProfessions
{
bool CanWork(PlayerbotAI* ai)
{
    Player* player = ai->GetBot();
    return LivingWorldMgr::instance().IsEnabled(player) && player->IsAlive() && !player->IsInCombat() &&
           !player->GetGroup() && !ai->GetMaster() && !player->InBattleground() && !player->GetMap()->IsDungeon() &&
           !player->IsInFlight() && !player->IsMounted() && !player->IsNonMeleeSpellCast(false) && !player->GetTrader();
}

bool WithinEra(Trainer::Spell const& lesson)
{
    if (!LivingWorld::RecipeWithinEra(lesson.ReqSkillRank, sPlayerbotAIConfig.RandomBotMaxLevel))
        return false;
    uint32 const spellId = lesson.SpellId;
    uint32 const cap = LivingWorld::ProfessionCap(sPlayerbotAIConfig.RandomBotMaxLevel);
    if (auto const* skill = sSpellMgr->GetSpellLearnSkill(spellId))
        if (skill->maxvalue > cap)
            return false;
    SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);
    if (!spell)
        return false;
    for (auto const& effect : spell->GetEffects())
        if (effect.IsEffect(SPELL_EFFECT_LEARN_SPELL))
            if (auto const* skill = sSpellMgr->GetSpellLearnSkill(effect.TriggerSpell))
                if (skill->maxvalue > cap)
                    return false;
    return true;
}

bool CanTrain(Player* player, Creature* npc, Trainer::Trainer* trainer, Trainer::Spell const& lesson)
{
    if (!npc || !npc->IsAlive() || npc->IsHostileTo(player) || !trainer ||
        trainer->GetTrainerType() != Trainer::Type::Tradeskill || !trainer->IsTrainerValidForPlayer(player) ||
        !trainer->CanTeachSpell(player, &lesson) || !WithinEra(lesson))
        return false;
    uint32 const era = sPlayerbotAIConfig.RandomBotMaxLevel;
    uint32 const skill = PlayerbotFactory::GetTrainerSpellTradeSkill(&lesson);
    std::array<bool, 14> known{};
    for (size_t i = 0; i < known.size(); ++i)
        known[i] = player->HasSkill(LivingWorld::Professions[i].skill);
    if (!LivingWorld::WantsProfession(player->GetGUID().GetCounter(), skill, era, known))
        return false;
    uint32 const cost = static_cast<uint32>(lesson.MoneyCost * player->GetReputationPriceDiscount(npc));
    return LivingWorld::CanAffordTraining(player->GetMoney(), cost, player->GetLevel());
}

bool HasReagents(Player* player, SpellInfo const* spell)
{
    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
        if (spell->Reagent[i] > 0 && spell->ReagentCount[i] &&
            !player->HasItemCount(spell->Reagent[i], spell->ReagentCount[i], false))
            return false;
    return true;
}

bool IsProfessionRecipe(Player* player, SpellInfo const* spell)
{
    auto const bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spell->Id);
    for (auto it = bounds.first; it != bounds.second; ++it)
        if (LivingWorld::FindProfession(it->second->SkillLine, sPlayerbotAIConfig.RandomBotMaxLevel) &&
            player->HasSkill(it->second->SkillLine))
            return true;
    return false;
}

bool NeedsProduct(Player* player, SpellInfo const* spell)
{
    bool creates = false;
    uint64 const stacks =
        LivingWorldMgr::instance().GetProfile(player).motivation == LivingWorld::Motivation::Trader ? 4 : 1;
    for (auto const& effect : spell->GetEffects())
    {
        if (!effect.IsEffect(SPELL_EFFECT_CREATE_ITEM) && !effect.IsEffect(SPELL_EFFECT_CREATE_ITEM_2))
            continue;
        auto const* product = sObjectMgr->GetItemTemplate(effect.ItemType);
        if (!product)
            return false;
        creates = true;
        // Count the bank too: a slow crafter must not endlessly stockpile the same recipe.
        if (player->GetItemCount(product->ItemId, true) >= uint64(product->GetMaxStackSize()) * stacks)
            return false;
    }
    return creates;
}

Item* EnchantTarget(Player* player, SpellInfo const* spell)
{
    for (auto const& effect : spell->GetEffects())
    {
        if (!effect.IsEffect(SPELL_EFFECT_ENCHANT_ITEM))
            continue;
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!item || !item->IsFitToSpellRequirements(spell))
                continue;
            // Preserve existing enchants until spec-aware upgrade scoring is available.
            if (!item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT))
                return item;
        }
    }
    return nullptr;
}
}  // namespace LivingProfessions

using namespace LivingProfessions;

LivingTrainingPlan LivingTrainingValue::Calculate()
{
    if (!CanWork(botAI))
        return {};
    LivingTrainingPlan result;
    uint32 cheapest = UINT32_MAX;
    for (ObjectGuid const guid : AI_VALUE(GuidVector, "nearest npcs"))
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* npc = unit ? unit->ToCreature() : nullptr;
        if (!npc || !npc->IsTrainer() || !bot->IsWithinDistInMap(npc, 40.0f))
            continue;
        auto* trainer = sObjectMgr->GetTrainer(npc->GetEntry());
        if (!trainer || trainer->GetTrainerType() != Trainer::Type::Tradeskill)
            continue;
        for (auto const& lesson : trainer->GetSpells())
            if (lesson.MoneyCost < cheapest && CanTrain(bot, npc, trainer, lesson))
            {
                cheapest = lesson.MoneyCost;
                result = {guid, lesson.SpellId};
            }
    }
    return result;
}

LivingCraftPlan LivingCraftValue::Calculate()
{
    if (!CanWork(botAI))
        return {};
    LivingCraftPlan result;
    uint32 best = 0;
    for (auto const& [id, learned] : bot->GetSpellMap())
    {
        if (!learned || learned->State == PLAYERSPELL_REMOVED || !learned->Active)
            continue;
        auto const* spell = sSpellMgr->GetSpellInfo(id);
        if (!spell || !IsProfessionRecipe(bot, spell) || !HasReagents(bot, spell))
            continue;
        Item* target = nullptr;
        if (!NeedsProduct(bot, spell))
        {
            target = EnchantTarget(bot, spell);
            if (!target)
                continue;
        }
        if (!botAI->CanCastSpell(id, bot, true, target))
            continue;
        uint32 const score = (ItemUsageValue::SpellGivesSkillUp(id, bot) ? 100000U : 0U) +
                             LivingWorld::Mix(bot->GetGUID().GetCounter() ^ id) % 10000U + 1;
        if (score > best)
        {
            best = score;
            result = {id, target ? target->GetGUID() : ObjectGuid::Empty};
        }
    }
    return result;
}

bool LivingTrainAction::isUseful() { return CanWork(botAI); }
bool LivingTrainAction::isPossible() { return AI_VALUE(LivingTrainingPlan, "living training").spell != 0; }

bool LivingTrainAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;
    auto* value = context->GetValue<LivingTrainingPlan>("living training");
    auto const plan = value->Get();
    Unit* unit = botAI->GetUnit(plan.trainer);
    Creature* npc = unit ? unit->ToCreature() : nullptr;
    auto* trainer = npc ? sObjectMgr->GetTrainer(npc->GetEntry()) : nullptr;
    auto const* lesson = trainer ? trainer->GetSpell(plan.spell) : nullptr;
    if (!lesson || !CanTrain(bot, npc, trainer, *lesson) || !bot->IsWithinDistInMap(npc, 40.0f))
    {
        value->Set({});
        return false;
    }
    if (!bot->GetNPCIfCanInteractWith(plan.trainer, UNIT_NPC_FLAG_TRAINER))
    {
        if (MoveNear(npc, INTERACTION_DISTANCE - 1.0f))
            return true;
        value->Set({});
        return false;
    }
    value->Set({});
    trainer->TeachSpell(npc, bot, plan.spell);
    return true;
}

bool LivingCraftAction::isUseful() { return CanWork(botAI) && !bot->isMoving(); }
bool LivingCraftAction::isPossible() { return AI_VALUE(LivingCraftPlan, "living craft").spell != 0; }

bool LivingCraftAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;
    auto* value = context->GetValue<LivingCraftPlan>("living craft");
    auto const plan = value->Get();
    value->Set({});
    auto const* spell = sSpellMgr->GetSpellInfo(plan.spell);
    Item* target = plan.item.IsEmpty() ? nullptr : bot->GetItemByGuid(plan.item);
    if (!spell || !HasReagents(bot, spell) || (!plan.item.IsEmpty() && !target) ||
        (target ? EnchantTarget(bot, spell) != target : !NeedsProduct(bot, spell)))
        return false;
    return botAI->CastSpell(plan.spell, bot, target);
}

void LivingProfessionStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "often", {NextAction("living train", ACTION_NORMAL + 2.0f), NextAction("living craft", ACTION_NORMAL + 1.0f)}));
    triggers.push_back(new TriggerNode(
        "often",
        {NextAction("living supplies", ACTION_NORMAL + 1.5f), NextAction("living processing", ACTION_NORMAL + 0.5f),
         NextAction("living auction", ACTION_NORMAL + 1.0f), NextAction("living mail", ACTION_NORMAL + 2.5f)}));
}
