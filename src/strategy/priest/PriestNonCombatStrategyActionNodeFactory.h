/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
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
        creators["holy nova"] = &holy_nova;
        creators["power word: shield"] = &power_word_shield;
        creators["power word: shield on party"] = &power_word_shield_on_party;
        creators["renew"] = &renew;
        creators["renew on party"] = &renew_on_party;
        creators["greater heal"] = &greater_heal;
        creators["greater heal on party"] = &greater_heal_on_party;
        creators["heal"] = &heal;
        creators["heal on party"] = &heal_on_party;
        creators["lesser heal"] = &lesser_heal;
        creators["lesser heal on party"] = &lesser_heal_on_party;
        creators["flash heal"] = &flash_heal;
        creators["flash heal on party"] = &flash_heal_on_party;
        creators["circle of healing on party"] = &circle_of_healing;
        creators["prayer of fortitude on party"] = &prayer_of_fortitude_on_party;
        creators["prayer of spirit on party"] = &prayer_of_spirit_on_party;
    }

private:
    static ActionNode* holy_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("holy nova",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* power_word_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("power word: shield",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("renew", 50.0f), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* power_word_shield_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("power word: shield on party",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("renew on party", 50.0f), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* renew([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("renew",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* renew_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("renew on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* greater_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("greater heal",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NextAction::array(0, new NextAction("heal"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* greater_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("greater heal on party",
                              /*P*/ NULL,
                              /*A*/ NextAction::array(0, new NextAction("heal on party"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heal",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NextAction::array(0, new NextAction("lesser heal"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heal on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NextAction::array(0, new NextAction("lesser heal on party"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* lesser_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser heal",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* lesser_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser heal on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* flash_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flash heal",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* flash_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flash heal on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* circle_of_healing([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("circle of healing on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              // /*A*/ NextAction::array(0, new NextAction("flash heal on party"), NULL),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* prayer_of_fortitude_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("prayer of fortitude on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NextAction::array(0, new NextAction("power word: fortitude on party"), NULL),
                              /*C*/ NULL);
    }
    static ActionNode* prayer_of_spirit_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("prayer of spirit on party",
                              /*P*/ NextAction::array(0, new NextAction("remove shadowform"), NULL),
                              /*A*/ NextAction::array(0, new NextAction("divine spirit on party"), NULL),
                              /*C*/ NULL);
    }
};

#endif
