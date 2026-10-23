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

#include <algorithm>

namespace
{
    std::vector<Item*> GetQuestUseItems(PlayerbotAI* botAI)
    {
        std::vector<Item*> items =
            botAI->GetAiObjectContext()->GetValue<std::vector<Item*>>("inventory items", "quest")->Get();

        // Quest starter items are handled by the "use random quest item" maintenance action.
        items.erase(std::remove_if(items.begin(), items.end(),
                                   [](Item* item) { return item->GetTemplate()->StartQuest; }),
                    items.end());
        return items;
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
    for (Item* item : GetQuestUseItems(botAI))
    {
        ItemTemplate const* proto = item->GetTemplate();
        for (auto const& [questId, status] : bot->getQuestStatusMap())
        {
            if (status.Status != QUEST_STATUS_INCOMPLETE)
                continue;

            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (!quest)
                continue;

            for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            {
                int32 reqEntry = quest->RequiredNpcOrGo[i];
                if (!reqEntry || status.CreatureOrGOCount[i] >= quest->RequiredNpcOrGoCount[i])
                    continue;

                // Quest data names the target but not the item that credits it, so try each
                // quest item and let the server's own spell target checks decide.
                float const range = sPlayerbotAIConfig.QuestGrabDistance;
                GameObject* go = nullptr;
                Creature* creature = nullptr;
                if (reqEntry < 0)
                    go = bot->FindNearestGameObject(uint32(-reqEntry), range, true);
                else
                    creature = bot->FindNearestCreature(uint32(reqEntry), range);

                WorldObject* target = go ? static_cast<WorldObject*>(go) : creature;
                if (!target)
                    continue;

                if (!IsWithinInteractionDist(target))
                    return MoveWorldObjectTo(target->GetGUID());

                UseItemAction useItemAction(botAI);
                bool const used = go ? useItemAction.UseItemOnGameObject(item, go->GetGUID())
                                     : useItemAction.UseItemOnUnit(item, creature);

                LOG_DEBUG("playerbots", "[Quest Grab] {} {} using {} on {} for quest {} objective {}", bot->GetName(),
                          used ? "succeeded" : "failed", proto->Name1, reqEntry, questId, i);

                if (used)
                    return true;
            }
        }
    }
    return false;
}

bool QuestGrabCreateItemAction::isUseful() { return !GetQuestUseItems(botAI).empty(); }

bool QuestGrabCreateItemAction::Execute(Event /*event*/)
{
    for (Item* item : GetQuestUseItems(botAI))
    {
        ItemTemplate const* proto = item->GetTemplate();
        for (uint8 spellIdx = 0; spellIdx < MAX_ITEM_PROTO_SPELLS; ++spellIdx)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[spellIdx].SpellId);
            if (!spellInfo)
                continue;

            for (uint8 effectIdx = 0; effectIdx < MAX_SPELL_EFFECTS; ++effectIdx)
            {
                if (spellInfo->Effects[effectIdx].Effect != SPELL_EFFECT_CREATE_ITEM)
                    continue;

                uint32 const createdItemId = spellInfo->Effects[effectIdx].ItemType;
                for (auto const& [questId, status] : bot->getQuestStatusMap())
                {
                    if (status.Status != QUEST_STATUS_INCOMPLETE)
                        continue;

                    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
                    if (!quest)
                        continue;

                    for (uint8 j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; ++j)
                    {
                        if (quest->RequiredItemId[j] != createdItemId ||
                            status.ItemCount[j] >= quest->RequiredItemCount[j])
                            continue;

                        // No target to aim at: the spell's own range/spell focus check decides,
                        // a bot in the wrong place just gets a failed cast.
                        UseItemAction useItemAction(botAI);
                        bool const used = useItemAction.UseItemAuto(item);

                        LOG_DEBUG("playerbots", "[Quest Grab] {} {} using {} (creates {}) for quest {}",
                                  bot->GetName(), used ? "succeeded" : "failed", proto->Name1, createdItemId, questId);

                        if (used)
                            return true;
                    }
                }
            }
        }
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
