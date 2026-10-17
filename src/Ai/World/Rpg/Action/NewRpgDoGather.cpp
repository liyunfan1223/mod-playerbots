/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NewRpgDoGather.h"

#include "GameObject.h"
#include "GatherNodeMgr.h"
#include "LootObjectStack.h"
#include "NewRpgInfo.h"
#include "ObjectDefines.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"

// Base-rank gather spells, the same ids OpenLootAction::DoLoot casts.
enum GatherSpells
{
    HERB_GATHERING = 2366,
    MINING = 2575,
};

void NewRpgDoGatherAction::ClearLootTargetForNode(ObjectGuid::LowType spawnId)
{
    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    if (!lootTarget.guid.IsGameObject())
        return;

    // GO instance guids are map-generated and do NOT equal spawn ids -
    // resolve the object and compare its spawn id. An unresolvable target
    // (despawned) is dead weight either way: release it too.
    GameObject* go = botAI->GetGameObject(lootTarget.guid);
    if (go && go->GetSpawnId() != spawnId)
        return;

    context->GetValue<LootObject>("loot target")->Set(LootObject());
}

void NewRpgDoGatherAction::AbandonNode(NewRpgInfo::DoGather& data, bool markVisited)
{
    ClearLootTargetForNode(data.nodeSpawnId);
    if (markVisited)
        data.visited.insert(data.nodeSpawnId);
    data.nodeSpawnId = 0;
    data.lastReach = 0;
}

bool NewRpgDoGatherAction::Execute(Event /*event*/)
{
    NewRpgInfo& info = botAI->rpgInfo;
    auto* dataPtr = std::get_if<NewRpgInfo::DoGather>(&info.data);
    if (!dataPtr)
        return false;
    auto& data = *dataPtr;

    // A dead bot can't interact with nodes: standing on them just burns the
    // timeout and wrongly writes live nodes into `visited`. Leave the state
    // untouched and resume after revive.
    if (!bot->IsAlive())
        return false;

    // (Nearly) full bags: the loot pipeline refuses to store new items
    // above this fill level (see StoreLootAction), so every harvest would
    // open the node, store nothing and stall until the timeout. Stop
    // farming and let the maintenance logic (sell/destroy) catch up.
    if (AI_VALUE(uint8, "bag space") > 80)
    {
        info.ChangeToIdle();
        return true;
    }

    // Teleported/summoned away mid-route: drop the target and reselect.
    // Checked before anything that uses data.nodePos, which holds
    // coordinates from the old map.
    if (data.nodeSpawnId && bot->GetMapId() != data.nodePos.GetMapId())
    {
        AbandonNode(data);
        return true;
    }

    bool runPeriodicChecks =
        !data.lastPassiveCheck || GetMSTimeDiffToNow(data.lastPassiveCheck) > passiveCheckInterval;
    if (runPeriodicChecks)
        data.lastPassiveCheck = getMSTime();

    // Periodically drop a target that is verifiably empty (grid loaded, no
    // live object) instead of walking all the way to it. Deliberately NOT
    // added to `visited`: nodes respawn, and a permanently poisoned spawn
    // point would make the bot run past the herb standing on it later.
    if (runPeriodicChecks && data.nodeSpawnId &&
        GatherNodeMgr::IsVerifiablyDown(bot->GetMap(), data.nodePos, data.nodeSpawnId))
    {
        AbandonNode(data);
        return true;
    }

    // Opportunistic switch (same cadence as the passive check): take a
    // verifiably live node over the current, possibly unverified target
    // when it is either meaningfully closer (switchMinGain) or right next
    // to the path (nodePickupDistance) - without the second clause the
    // bot runs past herbs whenever its target happens to be at a
    // comparable distance. Both clauses require the live node to be
    // strictly closer than the current target. Closeness alone does not
    // prevent ping-ponging (bot movement flips which node is closer), so
    // the node last switched away from may not steal the target back
    // until the current target resolves.
    if (runPeriodicChecks && data.nodeSpawnId)
    {
        float distToTarget = bot->GetExactDist(data.nodePos);
        GatherNodeSpawn const* live = sGatherNodeMgr.GetNearestLiveNode(bot, data.visited, nodeSwitchDistance);
        if (live && live->spawnId != data.nodeSpawnId && live->spawnId != data.lastSwitchedFrom)
        {
            float distToLive = bot->GetExactDist(live->pos);
            bool muchCloser = distToLive + switchMinGain < distToTarget;
            bool passingBy = distToLive < nodePickupDistance && distToLive < distToTarget;
            if (muchCloser || passingBy)
            {
                ClearLootTargetForNode(data.nodeSpawnId);
                data.lastSwitchedFrom = data.nodeSpawnId;
                data.nodeSpawnId = live->spawnId;
                data.nodePos = live->pos;
                data.lastReach = 0;
                return true;
            }
        }
    }

    if (!data.nodeSpawnId)
    {
        // The previous target was resolved (harvested or written off), so
        // the node we once switched away from becomes fair game again.
        data.lastSwitchedFrom = 0;

        // Prefer a node we can already see to be up over an unverified pick.
        GatherNodeSpawn const* node = sGatherNodeMgr.GetNearestLiveNode(bot, data.visited, nodeSwitchDistance);
        if (!node)
            node = sGatherNodeMgr.GetNextNode(bot, data.visited);
        if (!node)
        {
            // nothing (left) to gather in this zone
            info.ChangeToIdle();
            return true;
        }
        data.nodeSpawnId = node->spawnId;
        data.nodePos = node->pos;
        data.lastReach = 0;
        return true;
    }

    // 10yd keeps the bot inside lootDistance (default 15) so the loot
    // pipeline's "loot available" trigger can fire on the node.
    if (bot->GetExactDist(data.nodePos) > 10.0f)
        return MoveFarTo(data.nodePos);

    if (!data.lastReach)
        data.lastReach = getMSTime();

    GameObject* go = GatherNodeMgr::FindLiveNode(bot->GetMap(), data.nodeSpawnId);

    if (!go)
    {
        // harvested (by us or someone else) or despawned - move on. NOT
        // added to `visited`: the spawn point becomes a valid target again
        // once it respawns.
        AbandonNode(data);
        return true;
    }

    // A harvest cast is already underway - do nothing that could clobber
    // it (movement cancels the cast).
    if (bot->GetCurrentSpell(CURRENT_GENERIC_SPELL) || bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    // Write-off cases that do go into `visited` (this bot can't harvest
    // the node, now or later this session):
    // - IsEmpty: no access to the loot at all (quest-gated chests and
    //   anything else the index filter misses). Deliberately only the
    //   content check - positional/transient failures like the loot
    //   pipeline's Z-distance check on a slope are left to the timeout,
    //   so the bot doesn't bounce off harvestable nodes.
    // - nodeStayTime timeout: the loot pipeline failed to harvest a live
    //   node in time; without the write-off the bot would re-select it
    //   forever.
    LootObject lootObj(bot, go->GetGUID());
    bool timedOut = GetMSTimeDiffToNow(data.lastReach) > nodeStayTime;
    if (lootObj.IsEmpty() || timedOut)
    {
        AbandonNode(data, /*markVisited*/ true);
        return true;
    }

    // Keep the loot pipeline pointed at our node, not a stale previous one:
    // otherwise its move-to-loot (higher relevance) pulls the bot toward the
    // old target while our approach pulls toward this one, and they fight.
    // Don't override a valid target the pipeline is already busy with in loot
    // range (e.g. a corpse right next to us).
    LootObject currentTarget = AI_VALUE(LootObject, "loot target");
    bool keepCurrent = !currentTarget.IsEmpty() && currentTarget.IsLootPossible(bot) &&
                       AI_VALUE2(float, "distance", "loot target") <= sPlayerbotAIConfig.LootDistance;
    if (currentTarget.guid != go->GetGUID() && !keepCurrent)
        context->GetValue<LootObject>("loot target")->Set(LootObject(bot, go->GetGUID()));

    // Drive the close approach ourselves when out of interaction range. The
    // loot pipeline's own move-to-loot can't: it is gated on IsLootPossible
    // (whose Z check fails for nodes well above/below the bot - ledges,
    // underwater kelp), and it approaches via LOS-sampled offset points,
    // which find nothing when the node sits behind an obstacle (los=false
    // around a corner). A validated path (exact_waypoint false) climbs,
    // descends and routes around geometry to the node. This only runs when
    // the pipeline's higher-relevance approach already failed to move this
    // tick; a node with no reachable path is retired by the timeout above.
    if (!lootObj.IsLootPossible(bot) || bot->GetDistance(go) > INTERACTION_DISTANCE - 2.0f)
    {
        if (bot->isMoving())
            return false;
        return MoveTo(bot->GetMapId(), data.nodePos.GetPositionX(), data.nodePos.GetPositionY(),
                      data.nodePos.GetPositionZ(), false, false, false, false);
    }

    // The loot pipeline only dismounts, it never leaves shapeshift form. Drop
    // the form only when it actually blocks the gather cast (per the spell's
    // own stance rules: Herb Gathering is explicitly allowed in flight forms,
    // while e.g. Travel Form blocks it). We're in interaction range here (the
    // approach branch above returned otherwise), so the bot keeps its form
    // speed while approaching. No-op for non-druids (FORM_NONE).
    if (bot->GetShapeshiftForm() != FORM_NONE)
    {
        uint32 gatherSpellId = lootObj.skillId == SKILL_MINING ? MINING : HERB_GATHERING;
        SpellInfo const* gatherSpell = sSpellMgr->GetSpellInfo(gatherSpellId);
        if (gatherSpell && gatherSpell->CheckShapeshift(bot->GetShapeshiftForm()) != SPELL_CAST_OK)
        {
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
            return true;
        }
    }

    // In range with the pipeline pointed at our node: it casts the gather
    // spell and loots. While it works, Add() returns false and this idles.
    return AI_VALUE(LootObjectStack*, "available loot")->Add(go->GetGUID());
}
