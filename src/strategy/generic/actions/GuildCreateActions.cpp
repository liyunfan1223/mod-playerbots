/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildCreateActions.h"

#include "ArenaTeam.h"
#include "BudgetValues.h"
#include "Event.h"
#include "GuildMgr.h"
#include "Playerbots.h"
#include "RandomPlayerbotFactory.h"
#include "ServerFacade.h"

bool BuyPetitionAction::Execute(Event event)
{
    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    bool vendored = false, result = false;
    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_PETITIONER);
        if (!pCreature)
            continue;

        std::string const guildName = RandomPlayerbotFactory::CreateRandomGuildName();
        if (guildName.empty())
            continue;

        WorldPacket data(CMSG_PETITION_BUY);

        data << pCreature->GetGUID();
        data << uint32(0);
        data << uint64(0);
        data << guildName.c_str();
        data << std::string("");
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint16(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);

        for (uint8 i = 0; i < 10; ++i)
            data << std::string("");

        data << uint32(0);  // index
        data << uint32(0);

        bot->GetSession()->HandlePetitionBuyOpcode(data);

        return true;
    }

    return false;
}

bool BuyPetitionAction::isUseful() { return canBuyPetition(bot); };

bool BuyPetitionAction::canBuyPetition(Player* bot)
{
    if (bot->GetGuildId())
        return false;

    if (bot->GetGuildIdInvited())
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (AI_VALUE2(uint32, "item count", "Hitem:5863:"))
        return false;

    if (botAI->GetGuilderType() == GuilderType::SOLO)
        return false;

    if (botAI->GetGrouperType() == GrouperType::SOLO)
        return false;

    if (!botAI->HasStrategy("guild", BOT_STATE_NON_COMBAT))
        return false;

    uint32 cost = 1000;  // GUILD_CHARTER_COST;

    if (AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) < cost)
        return false;

    return true;
}

bool PetitionOfferAction::Execute(Event event)
{
    uint32 petitionEntry = 5863;  // GUILD_CHARTER
    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatQItem(5863));

    if (petitions.empty())
        return false;

    ObjectGuid guid = event.getObject();

    Player* master = GetMaster();
    if (!master)
    {
        if (!guid)
            guid = bot->GetTarget();
    }
    else
    {
        if (!guid)
            guid = master->GetTarget();
    }

    if (!guid)
        return false;

    Player* player = ObjectAccessor::FindPlayer(guid);

    if (!player)
        return false;

    WorldPacket data(CMSG_OFFER_PETITION);

    data << uint32(0);
    data << petitions.front()->GetGUID();
    data << guid;

    QueryResult result =
        CharacterDatabase.Query("SELECT playerguid FROM petition_sign WHERE player_account = {} AND petitionguid = {}",
                                player->GetSession()->GetAccountId(), petitions.front()->GetGUID().GetCounter());
    if (result)
    {
        return false;
    }

    bot->GetSession()->HandleOfferPetitionOpcode(data);

    result = CharacterDatabase.Query("SELECT playerguid FROM petition_sign WHERE petitionguid = {}",
                                     petitions.front()->GetGUID().GetCounter());
    uint8 signs = result ? (uint8)result->GetRowCount() : 0;

    context->GetValue<uint8>("petition signs")->Set(signs);

    return true;
}

bool PetitionOfferAction::isUseful() { return !bot->GetGuildId(); }

bool PetitionOfferNearbyAction::Execute(Event event)
{
    uint32 found = 0;

    GuidVector nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (auto& i : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(i);

        if (!player)
            continue;

        if (player->GetGuildId())
            continue;

        if (player->GetGuildIdInvited())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);

        if (botAI)
        {
            /*
            if (botAI->GetGrouperType() == SOLO && !botAI->HasRealPlayerMaster()) //Do not invite solo players.
                continue;

            */
            if (botAI->HasActivePlayerMaster())  // Do not invite alts of active players.
                continue;
        }
        else
        {
            if (!sPlayerbotAIConfig->randomBotGroupNearby)
                return false;
        }

        if (sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        // Parse rpg target to quest action.
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        p << i;
        p.rpos(0);

        if (PetitionOfferAction::Execute(Event("petition offer nearby", p)))
            found++;
    }

    return found > 0;
}

bool PetitionOfferNearbyAction::isUseful()
{
    return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->FormatQItem(5863)) &&
           AI_VALUE(uint8, "petition signs") < sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS);
}

bool PetitionTurnInAction::Execute(Event event)
{
    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    bool vendored = false, result = false;
    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", chat->FormatQItem(5863));

    if (petitions.empty())
        return false;

    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_PETITIONER);
        if (!pCreature)
            continue;

        WorldPacket data(CMSG_TURN_IN_PETITION, 8);

        Item* petition = petitions.front();

        if (!petition)
            return false;

        data << petition->GetGUID();

        bot->GetSession()->HandleTurnInPetitionOpcode(data);

        if (bot->GetGuildId())
        {
            Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

            uint32 st, cl, br, bc, bg;
            bg = urand(0, 51);
            bc = urand(0, 17);
            cl = urand(0, 17);
            br = urand(0, 7);
            st = urand(0, 180);
            EmblemInfo emblemInfo(st, cl, br, bc, bg);

            guild->HandleSetEmblem(emblemInfo);

            // LANG_GUILD_VETERAN -> can invite
            guild->HandleSetRankInfo(2, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK | GR_RIGHT_INVITE);
        }

        return true;
    }

    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    // Select a new target to travel to.
    TravelTarget newTarget = TravelTarget(botAI);

    bool foundTarget = SetNpcFlagTarget(&newTarget, {UNIT_NPC_FLAG_PETITIONER});

    if (!foundTarget || !newTarget.isActive())
        return false;

    newTarget.setRadius(INTERACTION_DISTANCE);

    setNewTarget(&newTarget, oldTarget);

    return true;
}

bool PetitionTurnInAction::isUseful()
{
    bool inCity = false;
    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
    {
        if (zone->flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    return inCity && !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->FormatQItem(5863)) &&
           AI_VALUE(uint8, "petition signs") >= sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS) &&
           !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}

bool BuyTabardAction::Execute(Event event)
{
    bool canBuy = botAI->DoSpecificAction("buy", Event("buy tabard", "Hitem:5976:"));
    if (canBuy && AI_VALUE2(uint32, "item count", chat->FormatQItem(5976)))
        return true;

    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    // Select a new target to travel to.
    TravelTarget newTarget = TravelTarget(botAI);

    bool foundTarget = SetNpcFlagTarget(&newTarget, {UNIT_NPC_FLAG_TABARDDESIGNER}, "Tabard Vendor", {5976});

    if (!foundTarget || !newTarget.isActive())
        return false;

    newTarget.setRadius(INTERACTION_DISTANCE);

    setNewTarget(&newTarget, oldTarget);

    return true;
};

bool BuyTabardAction::isUseful()
{
    bool inCity = false;
    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
    {
        if (zone->flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    return inCity && bot->GetGuildId() && !AI_VALUE2(uint32, "item count", chat->FormatQItem(5976)) &&
           AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) >= 10000 &&
           !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}
