/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MageActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "UseItemAction.h"
#include <cmath>

namespace
{
bool IsKnownConjuredWaterItem(Item const* item, std::vector<Item*> const& waterItems)
{
    if (!item)
        return false;

    for (Item* knownWater : waterItems)
    {
        if (knownWater == item)
            return true;
    }

    return false;
}

bool PlaceConjuredWaterInTrade(PlayerbotAI* botAI, std::vector<Item*> const& waterItems)
{
    Player* bot = botAI->GetBot();
    TradeData* tradeData = bot->GetTradeData();
    if (!tradeData)
        return false;

    // Already trading conjured water.
    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        Item* traded = tradeData->GetItem(TradeSlots(i));
        if (IsKnownConjuredWaterItem(traded, waterItems))
            return true;
    }

    int8 freeSlot = -1;
    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; ++i)
    {
        if (!tradeData->GetItem(TradeSlots(i)))
        {
            freeSlot = i;
            break;
        }
    }

    if (freeSlot < 0)
        return false;

    for (Item* item : waterItems)
    {
        if (!item || item->IsInTrade() || !item->CanBeTraded())
            continue;

        WorldPacket packet(CMSG_SET_TRADE_ITEM, 3);
        packet << (uint8)freeSlot;
        packet << (uint8)item->GetBagSlot();
        packet << (uint8)item->GetSlot();
        bot->GetSession()->HandleSetTradeItemOpcode(packet);
        return true;
    }

    return false;
}
}

std::vector<NextAction> CastMoltenArmorAction::getAlternatives()
{
    if (!botAI->HasSpell("molten armor"))
        return NextAction::merge({ NextAction("mage armor") }, CastBuffSpellAction::getAlternatives());

    return CastBuffSpellAction::getAlternatives();
}

std::vector<NextAction> CastMageArmorAction::getAlternatives()
{
    if (!botAI->HasSpell("mage armor"))
        return NextAction::merge({ NextAction("ice armor") }, CastBuffSpellAction::getAlternatives());

    return CastBuffSpellAction::getAlternatives();
}

bool UseManaSapphireAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(33312, false) > 0;  // Mana Sapphire
}

bool UseManaEmeraldAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(22044, false) > 0;  // Mana Emerald
}

bool UseManaRubyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(8008, false) > 0;  // Mana Ruby
}

bool UseManaCitrineAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(8007, false) > 0;  // Mana Citrine
}

bool UseManaJadeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(5513, false) > 0;  // Mana Jade
}

bool UseManaAgateAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(5514, false) > 0;  // Mana Agate
}

bool CastFrostNovaAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsInWorld() || target->isFrozen() ||
        (target->ToCreature() &&
         target->ToCreature()->HasMechanicTemplateImmunity(1 << (MECHANIC_FREEZE - 1))))
    {
        return false;
    }

    return ServerFacade::instance().IsDistanceLessOrEqualThan(
        AI_VALUE2(float, "distance", GetTargetName()), 10.f);
}

bool CastConeOfColdAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = ServerFacade::instance().IsDistanceLessOrEqualThan(
        AI_VALUE2(float, "distance", GetTargetName()), 10.f);

    return facingTarget && targetClose;
}

bool CastDragonsBreathAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return facingTarget && targetClose;
}

bool CastBlastWaveAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return targetClose;
}

Unit* CastFocusMagicOnPartyAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* casterDps = nullptr;
    Unit* healer = nullptr;
    Unit* target = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
            bot->GetDistance(member) > sPlayerbotAIConfig.SpellDistance || member->HasAura(54646))  // Focus Magic
        {
            continue;
        }

        if (member->getClass() == CLASS_MAGE)
            return member;

        if (!casterDps && PlayerbotAI::IsCaster(member) && PlayerbotAI::IsDps(member))
            casterDps = member;

        if (!healer && PlayerbotAI::IsHeal(member))
            healer = member;

        if (!target)
            target = member;
    }

    if (casterDps)
        return casterDps;

    if (healer)
        return healer;

    return target;
}

bool CastBlinkBackAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    bot->SetOrientation(bot->GetAngle(target) + M_PI);
    return CastSpellAction::Execute(event);
}

Unit* MageGiveWaterAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    if (Player* trader = bot->GetTrader())
    {
        if (trader->IsAlive() && trader->GetMap() == bot->GetMap())
            return trader;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMap() != bot->GetMap())
            continue;

        uint8 cls = member->getClass();
        if (cls != CLASS_DRUID && cls != CLASS_HUNTER && cls != CLASS_PALADIN && cls != CLASS_PRIEST &&
            cls != CLASS_SHAMAN && cls != CLASS_WARLOCK)
        {
            continue;
        }

        if (member->GetTrader() && member->GetTrader() != bot)
            continue;

        PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
        if (!memberAi)
            continue;

        if (!memberAi->HasStrategy("food", memberAi->GetState()))
            continue;

        if (memberAi->GetAiObjectContext()->GetValue<uint32>("item count", "conjured water")->Get())
            continue;

        return member;
    }

    return nullptr;
}

bool MageGiveWaterAction::isUseful()
{
    Unit* target = GetTarget();
    Player* receiver = target ? target->ToPlayer() : nullptr;
    if (!receiver && bot->GetTrader())
        receiver = bot->GetTrader()->ToPlayer();

    if (!receiver)
        return false;

    if (bot->GetTrader() && bot->GetTrader() != receiver)
        return false;

    if (!botAI->HasSpell("conjure water"))
        return false;

    if (botAI->HasSpell("ritual of refreshment"))
        return false;

    return true;
}

bool MageGiveWaterAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    Player* receiver = target ? target->ToPlayer() : nullptr;
    if (!receiver && bot->GetTrader())
        receiver = bot->GetTrader()->ToPlayer();

    if (!receiver)
        return false;

    if (!receiver->IsAlive() || receiver->GetMap() != bot->GetMap())
        return false;

    if (bot->GetDistance(receiver) > sPlayerbotAIConfig.spellDistance * 2)
        return false;

    if (!bot->IsWithinLOS(receiver->GetPositionX(), receiver->GetPositionY(), receiver->GetPositionZ()))
        return false;

    PlayerbotAI* receiverAi = GET_PLAYERBOT_AI(receiver);
    if (!receiverAi)
        return false;

    if (receiverAi->GetAiObjectContext()->GetValue<uint32>("item count", "conjured water")->Get())
        return false;

    // if someone needs water and mage has none, conjure first and do not open trade yet.
    std::vector<Item*> waterItems =
        botAI->GetAiObjectContext()->GetValue<std::vector<Item*>>("inventory items", "conjured water")->Get();
    if (waterItems.empty())
    {
        botAI->DoSpecificAction("conjure water", Event(), true);
        return true;
    }

    if (bot->GetTrader())
    {

        if (bot->GetTrader() != receiver)
            return false;

        if (!bot->GetTradeData() || !receiver->GetTradeData())
            return false;

        if (!PlaceConjuredWaterInTrade(botAI, waterItems))
            return false;

        WorldPacket p;
        uint32 status = 0;
        p << status;
        bot->GetSession()->HandleAcceptTradeOpcode(p);

        return true;
    }

    if (receiver->GetTrader() && receiver->GetTrader() != bot)
        return false;

    WorldPacket packet(CMSG_INITIATE_TRADE);
    packet << receiver->GetGUID();
    bot->GetSession()->HandleInitiateTradeOpcode(packet);
    return true;
}
