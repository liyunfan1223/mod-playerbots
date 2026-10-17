/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RpgSubActions.h"
#include "BudgetValues.h"
#include "ChooseRpgTargetAction.h"
#include "EmoteAction.h"
#include "Formations.h"
#include "GossipDef.h"
#include "GuildCreateActions.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PossibleRpgTargetsValue.h"
#include "SocialMgr.h"

void RpgHelper::OnExecute(std::string nextAction)
{
    if (botAI->HasGameClientMaster() && nextAction == "rpg")
        nextAction = "rpg cancel";

    SET_AI_VALUE(std::string, "next rpg action", nextAction);
}

void RpgHelper::BeforeExecute()
{
    OnExecute();

    bot->SetTarget(guidP());

    setFacingTo(guidP());
}

void RpgHelper::AfterExecute(bool doDelay, bool waitForGroup)
{
    OnExecute();

    bot->SetTarget(guidP());

    setFacingTo(guidP());

    if (doDelay)
        setDelay(waitForGroup);

    setFacing(guidP());
}

GuidPosition RpgHelper::guidP() { return AI_VALUE(GuidPosition, "rpg target"); }

ObjectGuid RpgHelper::guid() { return (ObjectGuid)guidP(); }

 bool RpgHelper::InRange()
 {
     GuidPosition targetGuid = guidP();
     if (!targetGuid)
         return false;

    return bot->GetExactDist2dSq(targetGuid.GetPositionX(), targetGuid.GetPositionY()) <
        INTERACTION_DISTANCE * INTERACTION_DISTANCE;
 }

void RpgHelper::setFacingTo(GuidPosition guidPosition)
{
    bot->SetFacingTo(guidPosition.getAngleTo(bot) + static_cast<float>(M_PI));
}

void RpgHelper::setFacing(GuidPosition guidPosition)
{
    if (!guidPosition.IsUnit())
        return;

    if (guidPosition.IsPlayer())
        return;

    //    Unit* unit = guidPosition.GetUnit();

    //    unit->SetFacingTo(unit->GetAngle(bot));
}

void RpgHelper::setDelay(bool waitForGroup)
{
    if (!botAI->HasGameClientMaster() || (waitForGroup && botAI->GetGroupLeader() == bot && bot->GetGroup()))
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.RpgDelay);
    else
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.RpgDelay / 5);
}

bool RpgSubAction::isPossible() { return rpg->guidP() && rpg->guidP().GetWorldObject(); }

bool RpgSubAction::isUseful() { return rpg->InRange(); }

bool RpgSubAction::Execute(Event event)
{
    bool doAction = botAI->DoSpecificAction(ActionName(), ActionEvent(event), true);
    rpg->AfterExecute(doAction, true);
    return doAction;
}

std::string const RpgSubAction::ActionName() { return "none"; }

Event RpgSubAction::ActionEvent(Event event) { return event; }

bool RpgStayAction::isUseful() { return rpg->InRange() && !botAI->HasGameClientMaster(); }

bool RpgStayAction::Execute(Event /*event*/)
{
    bot->PlayerTalkClass->SendCloseGossip();

    rpg->AfterExecute();
    return true;
}

bool RpgWorkAction::isUseful() { return rpg->InRange() && !botAI->HasGameClientMaster(); }

bool RpgWorkAction::Execute(Event /*event*/)
{
    bot->HandleEmoteCommand(EMOTE_STATE_USE_STANDING);
    rpg->AfterExecute();
    return true;
}

bool RpgEmoteAction::isUseful() { return rpg->InRange() && !botAI->HasGameClientMaster(); }

bool RpgEmoteAction::Execute(Event /*event*/)
{
    uint32 type = TalkAction::GetRandomEmote(rpg->guidP().GetUnit());

    WorldPacket p1;
    p1 << rpg->guid();
    bot->GetSession()->HandleGossipHelloOpcode(p1);

    bot->HandleEmoteCommand(type);

    rpg->AfterExecute();

    return true;
}

bool RpgCancelAction::Execute(Event /*event*/)
{
    RESET_AI_VALUE(GuidPosition, "rpg target");
    rpg->OnExecute("");
    return true;
}

bool RpgTaxiAction::isUseful() { return rpg->InRange() && !botAI->HasGameClientMaster(); }

bool RpgTaxiAction::Execute(Event /*event*/)
{
    GuidPosition guidP = rpg->guidP();

    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.GetPositionX(), guidP.GetPositionY(), guidP.GetPositionZ(), guidP.GetMapId(), bot->GetTeamId());

    std::vector<uint32> nodes;
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i);
        if (entry && entry->from == node && (bot->m_taxi.IsTaximaskNodeKnown(entry->to) || bot->isTaxiCheater()))
        {
            nodes.push_back(i);
        }
    }

    if (nodes.empty())
    {
        LOG_ERROR("playerbots", "Bot {} - No flight paths available", bot->GetName());
        return false;
    }

    uint32 path = nodes[urand(0, nodes.size() - 1)];
    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 100000);

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
    if (!entry)
        return false;

    TaxiNodesEntry const* nodeFrom = sTaxiNodesStore.LookupEntry(entry->from);
    TaxiNodesEntry const* nodeTo = sTaxiNodesStore.LookupEntry(entry->to);

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
    {
        LOG_ERROR("playerbots", "Bot {} cannot talk to flightmaster ({} location available)", bot->GetName(),
                  nodes.size());
        return false;
    }

    if (!bot->ActivateTaxiPathTo({entry->from, entry->to}, flightMaster, 0))
    {
        LOG_ERROR("playerbots", "Bot {} cannot fly {} ({} location available)", bot->GetName(), path, nodes.size());
        return false;
    }

    LOG_INFO("playerbots", "Bot {} <{}> is flying from {} to {} ({} location available)",
             bot->GetGUID().ToString().c_str(), bot->GetName(), nodeFrom->name[0], nodeTo->name[0], nodes.size());

    bot->SetMoney(money);

    rpg->AfterExecute();

    return true;
}

bool RpgDiscoverAction::Execute(Event /*event*/)
{
    GuidPosition guidP = rpg->guidP();

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.GetPositionX(), guidP.GetPositionY(), guidP.GetPositionZ(), guidP.GetMapId(), bot->GetTeamId());

    if (!node)
        return false;

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
        return false;

    return bot->GetSession()->SendLearnNewTaxiNode(flightMaster);
}

std::string const RpgStartQuestAction::ActionName() { return "accept all quests"; }

Event RpgStartQuestAction::ActionEvent(Event /*event*/)
{
    WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
    p << rpg->guid();
    p.rpos(0);
    return Event("rpg action", p);
}

std::string const RpgEndQuestAction::ActionName() { return "talk to quest giver"; }

Event RpgEndQuestAction::ActionEvent(Event /*event*/)
{
    WorldPacket p(CMSG_QUESTGIVER_COMPLETE_QUEST);
    p << rpg->guid();
    p.rpos(0);
    return Event("rpg action", p);
}

std::string const RpgBuyAction::ActionName() { return "buy"; }

Event RpgBuyAction::ActionEvent(Event /*event*/) { return Event("rpg action", "vendor"); }

std::string const RpgSellAction::ActionName() { return "sell"; }

Event RpgSellAction::ActionEvent(Event /*event*/) { return Event("rpg action", "vendor"); }

std::string const RpgRepairAction::ActionName() { return "repair"; }

bool RpgTrainAction::isUseful()
{
    if (!rpg->InRange())
        return false;

    Creature* creature = rpg->guidP().GetCreature();
    if (!creature)
        return false;

    if (!creature->IsInWorld() || creature->IsDuringRemoveFromWorld() || !creature->IsAlive())
        return false;

    return true;
}

bool RpgTrainAction::isPossible()
{
    GuidPosition gp = rpg->guidP();

    CreatureTemplate const* cinfo = gp.GetCreatureTemplate();
    if (!cinfo)
        return false;

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(cinfo->Entry);
    if (!trainer)
        return false;

    if (!trainer->IsTrainerValidForPlayer(bot))
        return false;

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cinfo->faction);
    float reputationDiscount = bot->GetReputationPriceDiscount(factionTemplate);
    uint32 currentGold = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells);

    for (auto& spell : trainer->GetSpells())
    {
        Trainer::Spell const* trainerSpell = trainer->GetSpell(spell.SpellId);
        if (!trainerSpell)
            continue;

        if (!trainer->CanTeachSpell(bot, trainerSpell))
            continue;

        if (currentGold < static_cast<uint32>(floor(trainerSpell->MoneyCost * reputationDiscount)))
            continue;

        // we only check if at least one spell can be learned from the trainer;
        // otherwise, the train action should not be allowed
        return true;
    }

    return false;
}

std::string const RpgTrainAction::ActionName() { return "trainer"; }

bool RpgHealAction::Execute(Event /*event*/)
{
    bool retVal = false;

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            retVal = botAI->DoSpecificAction("lesser heal on party", Event(), true);
            break;
        case CLASS_DRUID:
            retVal = botAI->DoSpecificAction("healing touch on party", Event(), true);
            break;
        case CLASS_PALADIN:
            retVal = botAI->DoSpecificAction("holy light on party", Event(), true);
            break;
        case CLASS_SHAMAN:
            retVal = botAI->DoSpecificAction("healing wave on party", Event(), true);
            break;
    }

    return retVal;
}

std::string const RpgHomeBindAction::ActionName() { return "home"; }

std::string const RpgQueueBgAction::ActionName()
{
    SET_AI_VALUE(uint32, "bg type", (uint32)AI_VALUE(BattlegroundTypeId, "rpg bg type"));
    return "free bg join";
}

std::string const RpgBuyPetitionAction::ActionName() { return "buy petition"; }

std::string const RpgUseAction::ActionName() { return "use"; }

Event RpgUseAction::ActionEvent(Event /*event*/)
{
    return Event("rpg action", chat->FormatWorldobject(rpg->guidP().GetWorldObject()));
}

std::string const RpgSpellAction::ActionName() { return "cast random spell"; }

Event RpgSpellAction::ActionEvent(Event /*event*/)
{
    return Event("rpg action", chat->FormatWorldobject(rpg->guidP().GetWorldObject()));
}

std::string const RpgCraftAction::ActionName() { return "craft random item"; }

Event RpgCraftAction::ActionEvent(Event /*event*/)
{
    return Event("rpg action", chat->FormatWorldobject(rpg->guidP().GetWorldObject()));
}

std::vector<Item*> RpgTradeUsefulAction::CanGiveItems(GuidPosition guidPosition)
{
    Player* player = guidPosition.GetPlayer();

    std::vector<Item*> giveItems;

    if (IsRealPlayer(botAI->GetMaster()) || !GET_PLAYERBOT_AI(player))
        return giveItems;

    std::vector<ItemUsage> myUsages = {ITEM_USAGE_NONE, ITEM_USAGE_VENDOR, ITEM_USAGE_AH, ITEM_USAGE_DISENCHANT};

    for (auto& myUsage : myUsages)
    {
        std::vector<Item*> myItems =
            AI_VALUE2(std::vector<Item*>, "inventory items", "usage " + std::to_string(myUsage));
        std::reverse(myItems.begin(), myItems.end());

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
                continue;

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", item->GetEntry());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}

bool RpgTradeUsefulAction::Execute(Event /*event*/)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    std::vector<Item*> items = CanGiveItems(guidP);

    if (items.empty())
        return false;

    Item* item = items.front();

    std::ostringstream param;

    param << chat->FormatWorldobject(player);
    param << " ";
    param << chat->FormatItem(item->GetTemplate());

    bool hasTraded = botAI->DoSpecificAction("trade", Event("rpg action", param.str().c_str()), true);

    if (hasTraded || bot->GetTradeData())
    {
        if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
        {
            if (bot->GetGroup() && bot->GetGroup()->IsMember(guidP) && botAI->HasGameClientMaster())
                botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "rpg_item_better_for_player",
                    "You can use this %item better than me, %player.",
                    {{"%item", chat->FormatItem(item->GetTemplate())}, {"%player", guidP.GetPlayer()->GetName()}}));
            else
                bot->Say(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                             "rpg_item_better_for_player",
                             "You can use this %item better than me, %player.",
                             {{"%item", chat->FormatItem(item->GetTemplate())}, {"%player", player->GetName()}}),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

            if (!urand(0, 4) || items.size() < 2)
            {
                // bot->Say("End trade with" + chat->FormatWorldobject(player), (bot->GetTeamId() == TEAM_ALLIANCE ?
                // LANG_COMMON : LANG_ORCISH));
                WorldPacket p;
                uint32 status = TRADE_STATUS_TRADE_ACCEPT;
                p << status;
                bot->GetSession()->HandleAcceptTradeOpcode(p);
            }
        }
        else
            bot->Say(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "rpg_start_trade_with_player",
                         "Start trade with %player",
                         {{"%player", chat->FormatWorldobject(player)}}),
                     (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        botAI->SetNextCheckDelay(sPlayerbotAIConfig.RpgDelay);
        return true;
    }

    return false;
}

bool RpgDuelAction::isUseful()
{
    // do not offer duel in non pvp areas
    if (sPlayerbotAIConfig.IsInPvpProhibitedZone(bot->GetZoneId()))
        return false;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    AreaTableEntry const* casterAreaEntry = sAreaTableStore.LookupEntry(bot->GetAreaId());
    if (casterAreaEntry && !(casterAreaEntry->flags & AREA_FLAG_ALLOW_DUELS))
    {
        // Dueling isn't allowed here
        return false;
    }

    return true;
}

bool RpgDuelAction::Execute(Event /*event*/)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    return botAI->DoSpecificAction("cast custom spell", Event("rpg action", chat->FormatWorldobject(player) + " 7266"),
                                   true);
}

bool RpgMountAnimAction::isUseful()
{
    return AI_VALUE2(bool, "mounted", "self target") && !AI_VALUE2(bool, "moving", "self target");
}

bool RpgMountAnimAction::Execute(Event /*event*/)
{
    WorldPacket p;
    bot->GetSession()->HandleMountSpecialAnimOpcode(p);

    return true;
}
