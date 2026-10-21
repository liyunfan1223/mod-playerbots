/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LivingFieldActions.h"

#include "FishValues.h"
#include "FishingAction.h"
#include "GameTime.h"
#include "GatherNodeMgr.h"
#include "ItemPackets.h"
#include "LivingEconomyActions.h"
#include "LivingFieldPolicy.h"
#include "LivingProfessionActions.h"
#include "LivingProfessionPolicy.h"
#include "LivingWorldMgr.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "Spell.h"

namespace
{
constexpr float MaxFieldDistance = 400.0f;
constexpr uint32 FieldTimeout = 180000;

bool IsPole(Item* item)
{
    return item && item->GetTemplate()->Class == ITEM_CLASS_WEAPON &&
           item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
}

bool FieldWindow(Player* player)
{
    return LivingWorld::FieldWorkWindow(player->GetGUID().GetCounter(), GameTime::GetGameTime().count(),
                                        LivingWorldMgr::instance().GetProfile(player).motivation);
}

bool FreeToForage(PlayerbotAI* ai)
{
    Player* player = ai->GetBot();
    return LivingWorldMgr::instance().IsEnabled(player) && player->IsAlive() && !player->IsInCombat() &&
           !player->GetGroup() && !ai->GetMaster() && !player->InBattleground() && !player->GetMap()->IsDungeon() &&
           !player->IsInFlight() && !player->GetTrader() && FieldWindow(player);
}

bool HasPole(Player* player)
{
    if (IsPole(player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND)))
        return true;
    for (Item* item : LivingProfessions::BagItems(player))
        if (IsPole(item) && player->CanUseItem(item) == EQUIP_ERR_OK)
            return true;
    return false;
}

bool Equip(Player* player, ObjectGuid guid, uint8 slot)
{
    Item* item = player->GetItemByGuid(guid);
    if (!item)
        return false;
    if (item == player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        return true;
    WorldPacket packet(CMSG_AUTOEQUIP_ITEM_SLOT);
    packet << guid << slot;
    WorldPackets::Item::AutoEquipItemSlot request(std::move(packet));
    request.Read();
    player->GetSession()->HandleAutoEquipItemSlotOpcode(request);
    Item* equipped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    return equipped && equipped->GetGUID() == guid;
}

std::set<uint32> Outputs(LootStore const& store, uint32 entry)
{
    std::set<uint32> items;
    if (auto const* loot = store.GetLootFor(entry))
        loot->CollectNonQuestItems(items);
    return items;
}

bool WantedDirect(Player* player, std::set<uint32> const& outputs, LivingProfessions::Stock const& stock);

LivingFieldPlan Wanted(Player* player, std::set<uint32> const& outputs, LivingProfessions::Stock const& stock,
                       bool practice)
{
    // Recipe inputs are shared by every profession. Include bank stock to prevent repeated stockpiling.
    for (uint32 entry : outputs)
    {
        auto const it = stock.reserve.find(entry);
        if (it != stock.reserve.end() && LivingWorld::MaterialDeficit(player->GetItemCount(entry, true), it->second))
        {
            LivingFieldPlan plan;
            plan.item = entry;
            plan.target = it->second;
            return plan;
        }
    }
    for (uint32 entry : outputs)
    {
        auto const* item = sObjectMgr->GetItemTemplate(entry);
        if (!item)
            continue;
        LootStore const* store = nullptr;
        if (item->HasFlag(ITEM_FLAG_IS_PROSPECTABLE) && player->HasSpell(31252) &&
            LivingWorld::FindProfession(SKILL_JEWELCRAFTING, sPlayerbotAIConfig.RandomBotMaxLevel) &&
            player->GetSkillValue(SKILL_JEWELCRAFTING) >= item->RequiredSkillRank)
            store = &LootTemplates_Prospecting;
        else if (item->HasFlag(ITEM_FLAG_IS_MILLABLE) && player->HasSpell(51005) &&
                 LivingWorld::FindProfession(SKILL_INSCRIPTION, sPlayerbotAIConfig.RandomBotMaxLevel) &&
                 player->GetSkillValue(SKILL_INSCRIPTION) >= item->RequiredSkillRank)
            store = &LootTemplates_Milling;
        if (!store || !WantedDirect(player, Outputs(*store, entry), stock))
            continue;
        auto const reserved = stock.reserve.find(entry);
        uint32 const target = (reserved == stock.reserve.end() ? 0 : reserved->second) + 5;
        if (player->GetItemCount(entry, true) < target)
        {
            LivingFieldPlan plan;
            plan.item = entry;
            plan.target = target;
            return plan;
        }
    }
    if (practice)
        for (uint32 entry : outputs)
        {
            auto const* item = sObjectMgr->GetItemTemplate(entry);
            if (item && item->Quality == ITEM_QUALITY_NORMAL && item->GetMaxStackSize() > 1 &&
                player->GetItemCount(entry, true) < item->GetMaxStackSize())
            {
                LivingFieldPlan plan;
                plan.item = entry;
                plan.target = item->GetMaxStackSize();
                return plan;
            }
        }
    return {};
}

bool WantedDirect(Player* player, std::set<uint32> const& outputs, LivingProfessions::Stock const& stock)
{
    for (uint32 entry : outputs)
        if (auto const it = stock.reserve.find(entry); it != stock.reserve.end())
            if (player->GetItemCount(entry, true) < it->second)
                return true;
    return false;
}

bool Finished(PlayerbotAI* ai, LivingFieldState const& state)
{
    Player* player = ai->GetBot();
    return !FreeToForage(ai) || player->GetMapId() != state.plan.position.GetMapId() ||
           GetMSTimeDiffToNow(state.started) >= FieldTimeout ||
           player->GetItemCount(state.plan.item, true) >= state.plan.target;
}

void EndTrip(PlayerbotAI* ai, LivingFieldState& state)
{
    Player* player = ai->GetBot();
    if (Spell* spell = player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        if (spell->m_spellInfo->Id == FISHING_SPELL)
            player->InterruptSpell(CURRENT_CHANNELED_SPELL);
    if (state.plan.item && player->IsAlive() && !player->IsInCombat() && !player->IsInFlight() && !ai->GetMaster())
        player->StopMoving();
    auto* context = ai->GetAiObjectContext();
    LootObject const target = context->GetValue<LootObject>("loot target")->Get();
    if (!state.plan.fishing && target.guid.IsGameObject() && !player->GetLootGUID())
        if (GameObject* node = ai->GetGameObject(target.guid))
            if (node->GetSpawnId() == state.plan.spawn)
            {
                context->GetValue<LootObject>("loot target")->Set({});
                context->GetValue<LootObjectStack*>("available loot")->Get()->Remove(target.guid);
            }
    state.plan = {};
    ai->GetAiObjectContext()->GetValue<LivingFieldPlan>("living field plan")->Set({});
}
}  // namespace

LivingFieldPlan LivingFieldPlanValue::Calculate()
{
    if (!FreeToForage(botAI) || bot->IsNonMeleeSpellCast(false) || bot->GetLootGUID() ||
        AI_VALUE(uint8, "bag space") >= 80 ||
        bot->GetMap()->GetEntry()->Expansion() > LivingWorld::Expansion(sPlayerbotAIConfig.RandomBotMaxLevel))
        return {};
    auto const stock = LivingProfessions::ProfessionStock(bot);
    LivingFieldPlan best;
    float bestDistance = MaxFieldDistance;
    std::unordered_map<uint32, std::set<uint32>> loot;
    if (auto const* nodes = sGatherNodeMgr.GetZoneNodes(bot->GetMapId(), bot->GetZoneId()))
        for (auto const& node : *nodes)
        {
            float const distance = bot->GetExactDist(node.pos);
            if (distance >= bestDistance || !bot->HasSkill(node.skillId) ||
                node.reqSkillValue > LivingWorld::ProfessionCap(sPlayerbotAIConfig.RandomBotMaxLevel) ||
                bot->GetSkillValue(node.skillId) < node.reqSkillValue ||
                GatherNodeMgr::IsVerifiablyDown(bot->GetMap(), node.pos, node.spawnId))
                continue;
            auto [it, inserted] = loot.try_emplace(node.lootId);
            if (inserted)
                it->second = Outputs(LootTemplates_Gameobject, node.lootId);
            bool const practice = bot->GetPureSkillValue(node.skillId) < bot->GetPureMaxSkillValue(node.skillId) &&
                                  bot->GetSkillValue(node.skillId) <= node.reqSkillValue + 100;
            auto plan = Wanted(bot, it->second, stock, practice);
            if (!plan.item)
                continue;
            plan.spawn = node.spawnId;
            plan.position = node.pos;
            plan.skill = node.skillId;
            plan.requiredSkill = node.reqSkillValue;
            best = plan;
            bestDistance = distance;
        }
    if (!bot->HasSkill(SKILL_FISHING) || !bot->HasSpell(FISHING_SPELL) || !HasPole(bot))
        return best;
    int32 required = sObjectMgr->GetFishingBaseSkillLevel(bot->GetAreaId());
    if (!required)
        required = sObjectMgr->GetFishingBaseSkillLevel(bot->GetZoneId());
    if (required <= 0 || bot->GetSkillValue(SKILL_FISHING) < uint32(required))
        return best;
    uint32 lootArea = LootTemplates_Fishing.HaveLootFor(bot->GetAreaId()) ? bot->GetAreaId() : bot->GetZoneId();
    GameObject* pool = nullptr;
    float poolDistance = sPlayerbotAIConfig.FishingDistance + 20.0f;
    for (ObjectGuid guid : AI_VALUE(GuidVector, "nearest game objects no los"))
        if (GameObject* object = botAI->GetGameObject(guid))
            if (object->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE && object->isSpawned() && object->InSamePhase(bot) &&
                bot->GetDistance2d(object) < poolDistance)
            {
                pool = object;
                poolDistance = bot->GetDistance2d(object);
            }
    auto const outputs = pool ? Outputs(LootTemplates_Gameobject, pool->GetGOInfo()->GetLootId())
                              : Outputs(LootTemplates_Fishing, lootArea);
    auto fish =
        Wanted(bot, outputs, stock, bot->GetPureSkillValue(SKILL_FISHING) < bot->GetPureMaxSkillValue(SKILL_FISHING));
    if (!fish.item)
        return best;
    context->GetValue<WorldPosition>("fishing spot")->Set({});
    MoveNearWaterAction findWater(botAI);
    findWater.isPossible();
    WorldPosition spot = AI_VALUE(WorldPosition, "fishing spot");
    if (!spot.IsValid() || spot.GetMapId() != bot->GetMapId() || bot->GetExactDist(spot) >= bestDistance)
        return best;
    fish.position = spot;
    fish.skill = SKILL_FISHING;
    fish.requiredSkill = required;
    fish.fishing = true;
    fish.pool = pool ? pool->GetGUID() : ObjectGuid::Empty;
    return fish;
}

bool LivingFieldAction::isUseful() { return FreeToForage(botAI) && AI_VALUE(uint8, "bag space") < 80; }

bool LivingFieldAction::isPossible()
{
    auto const state = AI_VALUE(LivingFieldState, "living field state");
    return state.plan.item || (state.pole.IsEmpty() && AI_VALUE(LivingFieldPlan, "living field plan").item);
}

bool LivingFieldAction::Execute(Event /*event*/)
{
    auto* stateValue = context->GetValue<LivingFieldState>("living field state");
    auto state = stateValue->Get();
    if (!isUseful())
        return false;
    if (!state.plan.item)
    {
        state.plan = AI_VALUE(LivingFieldPlan, "living field plan");
        state.started = getMSTime();
        if (!state.plan.item)
            return false;
        stateValue->Set(state);
    }
    if (Finished(botAI, state) || !bot->HasSkill(state.plan.skill) ||
        bot->GetSkillValue(state.plan.skill) < state.plan.requiredSkill)
    {
        EndTrip(botAI, state);
        stateValue->Set(state);
        return true;
    }
    if (bot->GetLootGUID() || AI_VALUE(LootObjectStack*, "available loot")->IsLootPending())
        return false;
    if (state.plan.fishing)
    {
        if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            if (spell->m_spellInfo->Id != FISHING_SPELL)
                return false;
            UseBobberAction reel(botAI);
            if (reel.isUseful())
                reel.Execute(Event());
            // The legacy bobber value sleeps until a bite; keep autonomous fishers responsive to combat.
            botAI->SetNextCheckDelay(500);
            return true;
        }
    }
    if (bot->IsNonMeleeSpellCast(false))
        return false;
    float const distance = bot->GetExactDist(state.plan.position);
    if (distance > (state.plan.fishing ? 1.0f : 10.0f))
    {
        if (bot->isMoving())
            return true;
        bool const moved = MoveTo(bot->GetMapId(), state.plan.position.GetPositionX(),
                                  state.plan.position.GetPositionY(), state.plan.position.GetPositionZ());
        if (!moved)
        {
            EndTrip(botAI, state);
            stateValue->Set(state);
        }
        return moved;
    }
    if (bot->isMoving())
        bot->StopMoving();
    if (bot->IsMounted())
    {
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        return true;
    }
    if (!state.plan.fishing)
    {
        GameObject* node = GatherNodeMgr::FindLiveNode(bot->GetMap(), state.plan.spawn);
        if (!node || !node->InSamePhase(bot))
        {
            EndTrip(botAI, state);
            stateValue->Set(state);
            return true;
        }
        LootObject target(bot, node->GetGUID());
        if (target.IsEmpty())
        {
            EndTrip(botAI, state);
            stateValue->Set(state);
            return false;
        }
        context->GetValue<LootObject>("loot target")->Set(target);
        AI_VALUE(LootObjectStack*, "available loot")->Add(node->GetGUID());
        return false;  // Let the normal, higher-priority harvest/loot actions run.
    }
    if (state.pole.IsEmpty())
    {
        Item* main = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* off = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        state.mainHand = main ? main->GetGUID() : ObjectGuid::Empty;
        state.offHand = off ? off->GetGUID() : ObjectGuid::Empty;
        if (IsPole(main))
            state.pole = main->GetGUID();
        else
            for (Item* item : LivingProfessions::BagItems(bot))
                if (IsPole(item) && bot->CanUseItem(item) == EQUIP_ERR_OK &&
                    Equip(bot, item->GetGUID(), EQUIPMENT_SLOT_MAINHAND))
                {
                    state.pole = item->GetGUID();
                    break;
                }
        if (state.pole.IsEmpty())
            EndTrip(botAI, state);
        stateValue->Set(state);
        return true;
    }
    WorldPosition water;
    if (!state.plan.pool.IsEmpty())
    {
        GameObject* pool = botAI->GetGameObject(state.plan.pool);
        if (pool && pool->isSpawned() && pool->InSamePhase(bot) && bot->GetDistance2d(pool) >= 10.0f &&
            bot->GetDistance2d(pool) <= 20.0f && bot->IsWithinLOSInMap(pool))
            water = WorldPosition(pool);
    }
    else
        water = FindWaterRadial(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMap(),
                                bot->GetPhaseMask(), 10.0f, 20.0f, 2.5f, true, 16);
    if (!water.IsValid() || !Equip(bot, state.pole, EQUIPMENT_SLOT_MAINHAND))
    {
        EndTrip(botAI, state);
        stateValue->Set(state);
        return false;
    }
    bot->SetFacingTo(bot->GetAngle(water.GetPositionX(), water.GetPositionY()));
    if (botAI->CastSpell(FISHING_SPELL, bot))
        return true;
    EndTrip(botAI, state);
    stateValue->Set(state);
    return false;
}

bool LivingRestoreWeaponsAction::isUseful()
{
    auto const state = AI_VALUE(LivingFieldState, "living field state");
    return bot->IsAlive() && (state.plan.item || !state.pole.IsEmpty()) &&
           (!state.plan.item || Finished(botAI, state) || AI_VALUE(uint8, "bag space") >= 80);
}

bool LivingRestoreWeaponsAction::Execute(Event /*event*/)
{
    if (!isUseful())
        return false;
    auto* value = context->GetValue<LivingFieldState>("living field state");
    auto state = value->Get();
    EndTrip(botAI, state);
    if (!state.mainHand.IsEmpty() && bot->GetItemByGuid(state.mainHand) &&
        !Equip(bot, state.mainHand, EQUIPMENT_SLOT_MAINHAND))
    {
        value->Set(state);
        return false;
    }
    if (!state.offHand.IsEmpty() && bot->GetItemByGuid(state.offHand) &&
        !Equip(bot, state.offHand, EQUIPMENT_SLOT_OFFHAND))
    {
        value->Set(state);
        return false;
    }
    value->Set({});
    return true;
}

void LivingFieldStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("very often", {NextAction("living fieldwork", ACTION_DEFAULT + 0.5f),
                                                      NextAction("living restore weapons", ACTION_HIGH + 1.0f)}));
}

bool LivingProfessions::FieldReservedItem(PlayerbotAI* ai, ObjectGuid item)
{
    if (!ai || !LivingWorldMgr::instance().IsEnabled(ai->GetBot()) || item.IsEmpty())
        return false;
    auto const state = ai->GetAiObjectContext()->GetValue<LivingFieldState>("living field state")->Get();
    return !state.pole.IsEmpty() && (item == state.mainHand || item == state.offHand || item == state.pole);
}
