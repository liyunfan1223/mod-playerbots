/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PRIESTNONCOMBATSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_PRIESTNONCOMBATSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class PriestNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        PriestNonCombatStrategyActionNodeFactory()
        {
            creators["power word: shield"] = &power_word_shield;
            creators["power word: shield on party"] = &power_word_shield_on_party;
            creators["circle of healing"] = &circle_of_healing;
            creators["prayer of healing"] = &prayer_of_healing;
            creators["greater heal"] = &greater_heal;
            creators["heal"] = &heal;
            creators["greater heal on party"] = &greater_heal_on_party;
            creators["heal on party"] = &heal_on_party;
            creators["lightwell"] = &lightwell;
        }

    private:
        ACTION_NODE_A(greater_heal, "greater heal", "heal");
        ACTION_NODE_A(heal, "heal", "lesser heal");
        ACTION_NODE_A(greater_heal_on_party, "greater heal on party", "heal on party");
        ACTION_NODE_A(heal_on_party, "heal on party", "lesser heal on party");
        ACTION_NODE_A(lightwell, "lightwell", "circle of healing");
        ACTION_NODE_A(circle_of_healing, "circle of healing", "prayer of healing");
        ACTION_NODE_A(prayer_of_healing, "prayer of healing", "renew on party");
        ACTION_NODE_A(power_word_shield, "power word: shield", "renew");
        ACTION_NODE_A(power_word_shield_on_party, "power word: shield on party", "renew on party");
};

#endif
