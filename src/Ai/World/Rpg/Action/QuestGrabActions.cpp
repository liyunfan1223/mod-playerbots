/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestGrabActions.h"

#include "Creature.h"
#include "GameObject.h"
#include "Item.h"
#include "LootObjectStack.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "UseItemAction.h"

#include <vector>

namespace
{
    std::vector<Item*> GetQuestUseItems(PlayerbotAI* botAI)
    {
        std::vector<Item*> items =
            botAI->GetAiObjectContext()->GetValue<std::vector<Item*>>("inventory items", "quest")->Get();

        // Quest starter items are handled by the "use random quest item" maintenance action.
        std::erase_if(items, [](Item* item) { return item->GetTemplate()->StartQuest; });
        return items;
    }

    // Creature (> 0) or gameobject (< 0) entries that incomplete quest objectives still need.
    std::vector<int32> GetNeededNpcOrGoEntries(Player* bot)
    {
        std::vector<int32> entries;
        for (auto const& [questId, status] : bot->getQuestStatusMap())
        {
            Quest const* quest =
                status.Status == QUEST_STATUS_INCOMPLETE ? sObjectMgr->GetQuestTemplate(questId) : nullptr;
            if (!quest)
                continue;

            for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            {
                if (quest->RequiredNpcOrGo[i] && status.CreatureOrGOCount[i] < quest->RequiredNpcOrGoCount[i])
                    entries.push_back(quest->RequiredNpcOrGo[i]);
            }
        }
        return entries;
    }

    bool IsItemNeededForQuest(Player* bot, uint32 itemId)
    {
        for (auto const& [questId, status] : bot->getQuestStatusMap())
        {
            Quest const* quest =
                status.Status == QUEST_STATUS_INCOMPLETE ? sObjectMgr->GetQuestTemplate(questId) : nullptr;
            if (!quest)
                continue;

            for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
            {
                if (quest->RequiredItemId[i] == itemId && status.ItemCount[i] < quest->RequiredItemCount[i])
                    return true;
            }
        }
        return false;
    }

    // Whether one of the item's spells creates an item a quest still needs.
    bool CreatesNeededQuestItem(Player* bot, ItemTemplate const* proto)
    {
        for (auto const& itemSpell : proto->Spells)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itemSpell.SpellId);
            if (!spellInfo)
                continue;

            for (SpellEffectInfo const& effect : spellInfo->Effects)
            {
                if (effect.Effect == SPELL_EFFECT_CREATE_ITEM && IsItemNeededForQuest(bot, effect.ItemType))
                    return true;
            }
        }
        return false;
    }
}

bool QuestGrabGiverAction::Execute(Event /*event*/)
{
    // No OrganizeQuestLog() here: it is written for random bots and would drop quests of a
    // player's bot whose log is nearly full.
    return SearchQuestGiverAndAcceptOrReward(sPlayerbotAIConfig.QuestGrabDistance, false);
}

bool QuestGrabUseItemAction::isUseful() { return !GetQuestUseItems(botAI).empty(); }

bool QuestGrabUseItemAction::Execute(Event /*event*/)
{
    float const range = sPlayerbotAIConfig.QuestGrabDistance;
    std::vector<WorldObject*> targets;
    for (int32 entry : GetNeededNpcOrGoEntries(bot))
    {
        WorldObject* target = entry < 0 ? static_cast<WorldObject*>(bot->FindNearestGameObject(-entry, range, true))
                                        : bot->FindNearestCreature(entry, range);
        if (target)
            targets.push_back(target);
    }

    // Quest data names the target but not the item that credits it, so try each quest item and
    // let the server's own spell target checks decide. Targets in reach are tried before walking
    // anywhere, so a rejected pair can't pull the bot away from one that works.
    WorldObject* moveTarget = nullptr;
    for (WorldObject* target : targets)
    {
        if (!IsWithinInteractionDist(target))
        {
            if (!moveTarget || bot->GetExactDist(target) < bot->GetExactDist(moveTarget))
                moveTarget = target;
            continue;
        }

        for (Item* item : GetQuestUseItems(botAI))
        {
            UseItemAction useItemAction(botAI);
            Creature* creature = target->ToCreature();
            bool const used = creature ? useItemAction.UseItemOnUnit(item, creature)
                                       : useItemAction.UseItemOnGameObject(item, target->GetGUID());

            LOG_DEBUG("playerbots", "[Quest Grab] {} {} using {} on {}", bot->GetName(),
                      used ? "succeeded" : "failed", item->GetTemplate()->Name1, target->GetName());

            if (used)
                return true;
        }
    }

    return moveTarget && MoveWorldObjectTo(moveTarget->GetGUID());
}

bool QuestGrabCreateItemAction::isUseful() { return !GetQuestUseItems(botAI).empty(); }

bool QuestGrabCreateItemAction::Execute(Event /*event*/)
{
    for (Item* item : GetQuestUseItems(botAI))
    {
        if (!CreatesNeededQuestItem(bot, item->GetTemplate()))
            continue;

        // No target to aim at: the spell's own range/spell focus check decides, a bot in the
        // wrong place just gets a failed cast.
        UseItemAction useItemAction(botAI);
        bool const used = useItemAction.UseItemAuto(item);

        LOG_DEBUG("playerbots", "[Quest Grab] {} {} using {}", bot->GetName(), used ? "succeeded" : "failed",
                  item->GetTemplate()->Name1);

        if (used)
            return true;
    }
    return false;
}

bool QuestGrabObjectAction::Execute(Event /*event*/)
{
    GuidVector candidates = AI_VALUE(GuidVector, "possible quest grab targets");
    if (candidates.empty())
        return false;

    ObjectGuid const guid = candidates.front();
    GameObject* go = botAI->GetGameObject(guid);
    if (!go)
        return false;

    if (!IsWithinInteractionDist(go))
        return MoveWorldObjectTo(guid);

    // Chests go through the loot pipeline, which casts the opening/gathering spell or uses the
    // key their lock needs. The other types grant credit through GameObject::Use(), like a click.
    if (go->GetGoType() == GAMEOBJECT_TYPE_CHEST)
    {
        context->GetValue<LootObject>("loot target")->Set(LootObject(bot, guid));
        return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
    }

    LOG_DEBUG("playerbots", "[Quest Grab] {} using quest object {}", bot->GetName(), go->GetGOInfo()->name);
    go->Use(bot);
    return true;
}
