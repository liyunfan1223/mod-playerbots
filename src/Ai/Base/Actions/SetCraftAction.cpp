/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SetCraftAction.h"

#include "ChatHelper.h"
#include "CraftOrder.h"
#include "CraftValue.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotSpellRepository.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"

#include <array>

// Mining is here for its Smelting window.
static constexpr std::array<uint32, 11> CRAFTING_SKILLS = {
    SKILL_ALCHEMY,     SKILL_BLACKSMITHING, SKILL_COOKING,        SKILL_ENCHANTING, SKILL_ENGINEERING, SKILL_FIRST_AID,
    SKILL_INSCRIPTION, SKILL_JEWELCRAFTING, SKILL_LEATHERWORKING, SKILL_MINING,     SKILL_TAILORING};

bool SetCraftAction::Execute(Event event)
{
    Player* master = GetMaster();
    Player* requester = event.getOwner() ? event.getOwner() : master;
    if (!requester)
        return false;

    std::string const param = event.getParam();
    if (param.empty())
    {
        TellProfessions(requester);
        return true;
    }

    if (!master || requester != master)
    {
        CraftOrder::Tell(botAI, requester,
                         PlayerbotTextMgr::instance().GetBotTextOrDefault(
                             "craft_master_only", "I only take crafting orders from the player I'm with", {}));
        return false;
    }

    if (param == "reset")
        CraftOrder::Refund(botAI);
    else if (param == "?")
        CraftOrder::Touch(botAI->GetCraftOrder());
    else
        return PlaceOrder(param);

    CraftOrder::TellStatus(botAI);
    return true;
}

void SetCraftAction::TellProfessions(Player* requester)
{
    bool any = false;
    for (uint32 skill : CRAFTING_SKILLS)
    {
        std::string const link = ChatHelper::FormatProfessionLink(bot, skill);
        if (link.empty())
            continue;

        std::ostringstream out;
        out << link << " " << bot->GetSkillValue(skill) << "/" << bot->GetMaxSkillValue(skill);
        CraftOrder::Tell(botAI, requester, out.str());
        any = true;
    }

    if (!any)
    {
        CraftOrder::Tell(botAI, requester,
                         PlayerbotTextMgr::instance().GetBotTextOrDefault("craft_no_professions",
                                                                          "I don't know any crafting professions", {}));
    }
}

bool SetCraftAction::PlaceOrder(std::string const& param)
{
    auto const refuse = [&](std::string const& key, std::string const& text,
                            std::map<std::string, std::string> const& placeholders = {})
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(key, text, placeholders));
        return false;
    };

    ItemIds const itemIds = chat->parseItems(param);
    if (itemIds.empty())
        return refuse("craft_usage", "Usage: 'craft', 'craft [item link] [count]', 'craft ?' or 'craft reset'");

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(*itemIds.begin());
    if (!proto)
        return false;

    CraftData& data = botAI->GetCraftOrder();
    if (!data.IsEmpty())
    {
        ItemTemplate const* current = sObjectMgr->GetItemTemplate(data.itemId);
        return refuse("craft_busy", "I'm already working on an order for %item. Say 'craft reset' to cancel it first",
                      {{"%item", current ? ChatHelper::FormatItem(current) : ""}});
    }

    std::string const itemLink = ChatHelper::FormatItem(proto);
    if (proto->Bonding == BIND_WHEN_PICKED_UP)
    {
        return refuse("craft_bind_on_pickup",
                      "I can't craft %item for you: it is bind on pickup, so I couldn't trade it to you",
                      {{"%item", itemLink}});
    }

    uint32 const spellId = FindCraftSpell(proto->ItemId);
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!spellInfo)
        return refuse("craft_cannot_craft", "I cannot craft this");

    CraftData order;
    for (uint32 i = 0; i < MAX_SPELL_REAGENTS; ++i)
    {
        if (spellInfo->Reagent[i] > 0 && spellInfo->ReagentCount[i])
            order.reagents[uint32(spellInfo->Reagent[i])].perCraft += spellInfo->ReagentCount[i];
    }

    if (order.reagents.empty())
        return refuse("craft_cannot_craft", "I cannot craft this");

    std::vector<CraftRules::StackNeed> needs = {
        {CraftOrder::GetMaxCreated(spellInfo, proto), proto->GetMaxStackSize()}};
    for (auto const& [item, reagent] : order.reagents)
    {
        ItemTemplate const* reagentProto = sObjectMgr->GetItemTemplate(item);
        if (reagentProto && (reagentProto->Bonding == BIND_WHEN_PICKED_UP || reagentProto->Bonding == BIND_QUEST_ITEM))
        {
            return refuse("craft_reagent_bind_on_pickup",
                          "I can't take an order for %item: %reagent is bind on pickup, so you couldn't trade it to me",
                          {{"%item", itemLink}, {"%reagent", ChatHelper::FormatItem(reagentProto)}});
        }

        needs.push_back({reagent.perCraft, reagentProto ? reagentProto->GetMaxStackSize() : 1});
    }

    for (uint32 i = 0; i < spellInfo->Totem.size(); ++i)
    {
        if ((spellInfo->Totem[i] && !bot->HasItemCount(spellInfo->Totem[i])) ||
            (spellInfo->TotemCategory[i] && !bot->HasItemTotemCategory(spellInfo->TotemCategory[i])))
            return refuse("craft_missing_tool", "I don't have the tool needed to craft %item", {{"%item", itemLink}});
    }

    if (bot->HasSpellCooldown(spellId))
    {
        return refuse("craft_on_cooldown", "I can't craft %item again until its cooldown ends", {{"%item", itemLink}});
    }

    uint32 const ownProducts = bot->GetItemCount(proto->ItemId);
    if (proto->MaxCount > 0 && ownProducts >= uint32(proto->MaxCount))
        return refuse("craft_unique_held", "I already have as many %item as I can hold", {{"%item", itemLink}});

    uint32 const requested = CraftRules::ParseOrderCount(param);
    uint32 count = CraftRules::CraftsThatFit(needs, requested, CraftOrder::FreeBagSlots(bot));
    if (!count)
        return refuse("craft_no_bag_space", "I don't have enough bag space to craft %item", {{"%item", itemLink}});

    if (count < requested)
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_bag_space_limit", "I only have bag space to craft %count %item",
            {{"%count", std::to_string(count)}, {"%item", itemLink}}));
    }

    // A cooldown or a unique product stops the next craft until the customer has collected the first.
    if (count > 1 && (spellInfo->RecoveryTime || spellInfo->CategoryRecoveryTime || proto->MaxCount > 0))
    {
        count = 1;
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_limit_one", "I can only craft one %item at a time", {{"%item", itemLink}}));
    }

    order.owner = GetMaster()->GetGUID();
    order.itemId = proto->ItemId;
    order.spellId = spellId;
    order.count = count;
    order.ownProducts = ownProducts;
    for (auto& [item, reagent] : order.reagents)
        reagent.own = bot->GetItemCount(item);

    // The trade that collects the order carries the fee, which these trading modes refuse.
    bool const randomBot = sRandomPlayerbotMgr.IsRandomBot(bot) || sRandomPlayerbotMgr.IsAddclassBot(bot);
    if (randomBot && (sPlayerbotAIConfig.EnableRandomBotTrading == 0 ||
                      (sPlayerbotAIConfig.EnableRandomBotTrading == 2 && CraftOrder::GetFee(bot, order, count))))
        return refuse("trade_disabled", "Trading is disabled");

    data = std::move(order);
    CraftOrder::Touch(data);

    CraftOrder::TellStatus(botAI);

    if (spellInfo->RequiresSpellFocus)
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "craft_needs_station",
            "This recipe needs me to stand next to a crafting station, such as an anvil, forge or cooking fire", {}));
    }

    return true;
}

uint32 SetCraftAction::FindCraftSpell(uint32 itemId)
{
    for (auto const& [spellId, playerSpell] : bot->GetSpellMap())
    {
        if (playerSpell->State == PLAYERSPELL_REMOVED || !playerSpell->Active)
            continue;

        if (!PlayerbotSpellRepository::Instance().GetSkillLine(spellId))
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        for (SpellEffectInfo const& effect : spellInfo->GetEffects())
        {
            if (effect.Effect == SPELL_EFFECT_CREATE_ITEM && effect.ItemType == itemId)
                return spellId;
        }
    }

    return 0;
}
