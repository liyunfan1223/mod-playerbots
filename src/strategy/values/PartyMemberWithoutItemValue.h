/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PARTYMEMBERWITHOUTITEMVALUE_H
#define _PLAYERBOT_PARTYMEMBERWITHOUTITEMVALUE_H

#include "NamedObjectContext.h"
#include "PartyMemberValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;
class Unit;

class PartyMemberWithoutItemValue : public PartyMemberValue, public Qualified
{
public:
    PartyMemberWithoutItemValue(PlayerbotAI* botAI, std::string const name = "party member without item",
                                float range = sPlayerbotAIConfig->farDistance)
        : PartyMemberValue(botAI, name)
    {
    }

protected:
    Unit* Calculate() override;
    virtual FindPlayerPredicate* CreatePredicate();
};

class PartyMemberWithoutFoodValue : public PartyMemberWithoutItemValue
{
public:
    PartyMemberWithoutFoodValue(PlayerbotAI* botAI, std::string const name = "party member without food")
        : PartyMemberWithoutItemValue(botAI, name)
    {
    }

protected:
    FindPlayerPredicate* CreatePredicate() override;
};

class PartyMemberWithoutWaterValue : public PartyMemberWithoutItemValue
{
public:
    PartyMemberWithoutWaterValue(PlayerbotAI* botAI, std::string const name = "party member without water")
        : PartyMemberWithoutItemValue(botAI, name)
    {
    }

protected:
    FindPlayerPredicate* CreatePredicate() override;
};

#endif
