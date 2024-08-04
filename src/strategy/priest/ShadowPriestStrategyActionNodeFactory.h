/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHADOWPRIESTSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_SHADOWPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class ShadowPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShadowPriestStrategyActionNodeFactory()
    {
        creators["mind blast"] = &mind_blast;
        creators["dispersion"] = &dispersion;
        creators["mind flay"] = &mind_flay;
        creators["smite"] = &smite;
    }

private:
    static ActionNode* mind_blast(PlayerbotAI* botAI)
    {
        return new ActionNode("mind blast",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mind flay"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* mind_flay(PlayerbotAI* botAI)
    {
        return new ActionNode("mind flay",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("smite"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* smite(PlayerbotAI* botAI)
    {
        return new ActionNode("smite",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* dispersion(PlayerbotAI* botAI)
    {
        return new ActionNode("dispersion",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
                              /*C*/ nullptr);
    }
};

#endif
