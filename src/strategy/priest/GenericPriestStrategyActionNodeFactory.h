/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class GenericPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        GenericPriestStrategyActionNodeFactory()
        {
            creators["power word: shield"] = &power_word_shield;
            creators["power word: shield on party"] = &power_word_shield_on_party;
            creators["greater heal"] = &greater_heal;
            creators["greater heal on party"] = &greater_heal_on_party;
            creators["heal"] = &heal;
            creators["heal on party"] = &heal_on_party;
            creators["flash heal"] = &flash_heal;
            creators["flash heal on party"] = &flash_heal_on_party;
            creators["psychic scream"] = &psychic_scream;
            creators["lightwell"] = &lightwell;
            creators["circle of healing"] = &circle_of_healing;
        }

    private:
        ACTION_NODE_A(power_word_shield, "power word: shield", "renew");
        ACTION_NODE_A(power_word_shield_on_party, "power word: shield on party", "renew on party");
        ACTION_NODE_A(greater_heal, "greater heal", "heal");
        ACTION_NODE_A(greater_heal_on_party, "greater heal on party", "heal on party");
        ACTION_NODE_A(heal, "heal", "lesser heal");
        ACTION_NODE_A(heal_on_party, "heal on party", "lesser heal on party");
        ACTION_NODE_A(flash_heal, "flash heal", "greater heal");
        ACTION_NODE_A(flash_heal_on_party, "flash heal on party", "greater heal on party");
        ACTION_NODE_A(psychic_scream, "psychic scream", "fade");
        ACTION_NODE_A(lightwell, "lightwell", "circle of healing");
        ACTION_NODE_A(circle_of_healing, "circle of healing", "prayer of healing");
};

class CurePriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        CurePriestStrategyActionNodeFactory()
        {
            creators["abolish disease"] = &abolish_disease;
            creators["abolish disease on party"] = &abolish_disease_on_party;
        }

    private:
        ACTION_NODE_A(abolish_disease, "abolish disease", "cure disease");
        ACTION_NODE_A(abolish_disease_on_party, "abolish disease on party", "cure disease on party");
};

#endif
