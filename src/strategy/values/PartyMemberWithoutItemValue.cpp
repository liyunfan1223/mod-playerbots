/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PartyMemberWithoutItemValue.h"

#include "Playerbots.h"

class PlayerWithoutItemPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutItemPredicate(PlayerbotAI* botAI, std::string const item)
        : PlayerbotAIAware(botAI), FindPlayerPredicate(), item(item)
    {
    }

    bool Check(Unit* unit) override
    {
        if (!unit->IsAlive())
            return false;

        Player* member = unit->ToPlayer();
        if (!member)
            return false;

        if (!(member->IsInSameGroupWith(botAI->GetBot()) || member->IsInSameRaidWith(botAI->GetBot())))
            return false;

        PlayerbotAI* memberbotAI = GET_PLAYERBOT_AI(member);
        if (!memberbotAI)
            return false;

        return !memberbotAI->GetAiObjectContext()->GetValue<uint32>("item count", item)->Get();
    }

private:
    std::string const item;
};

Unit* PartyMemberWithoutItemValue::Calculate()
{
    FindPlayerPredicate* predicate = CreatePredicate();
    Unit* unit = FindPartyMember(*predicate);
    delete predicate;
    return unit;
}

FindPlayerPredicate* PartyMemberWithoutItemValue::CreatePredicate()
{
    return new PlayerWithoutItemPredicate(botAI, qualifier);
}

class PlayerWithoutFoodPredicate : public PlayerWithoutItemPredicate
{
public:
    PlayerWithoutFoodPredicate(PlayerbotAI* botAI) : PlayerWithoutItemPredicate(botAI, "conjured food") {}

    bool Check(Unit* unit) override
    {
        if (!PlayerWithoutItemPredicate::Check(unit))
            return false;

        Player* member = unit->ToPlayer();
        if (!member)
            return false;

        return member->getClass() != CLASS_MAGE;
    }
};

class PlayerWithoutWaterPredicate : public PlayerWithoutItemPredicate
{
public:
    PlayerWithoutWaterPredicate(PlayerbotAI* botAI) : PlayerWithoutItemPredicate(botAI, "conjured water") {}

    bool Check(Unit* unit) override
    {
        if (!PlayerWithoutItemPredicate::Check(unit))
            return false;

        Player* member = dynamic_cast<Player*>(unit);
        if (!member)
            return false;

        uint8 cls = member->getClass();
        return cls == CLASS_DRUID || cls == CLASS_HUNTER || cls == CLASS_PALADIN || cls == CLASS_PRIEST ||
               cls == CLASS_SHAMAN || cls == CLASS_WARLOCK;
    }
};

FindPlayerPredicate* PartyMemberWithoutFoodValue::CreatePredicate() { return new PlayerWithoutFoodPredicate(botAI); }

FindPlayerPredicate* PartyMemberWithoutWaterValue::CreatePredicate() { return new PlayerWithoutWaterPredicate(botAI); }
