/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICPALADINSTRATEGYACTIONNODEFACTORY_H
#define PLAYERBOTS_GENERICPALADINSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class GenericPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericPaladinStrategyActionNodeFactory()
    {
        creators["cleanse poison"] = &cleanse_poison;
        creators["cleanse disease"] = &cleanse_disease;
        creators["cleanse poison on party"] = &cleanse_poison_on_party;
        creators["cleanse disease on party"] = &cleanse_disease_on_party;
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["seal of wisdom"] = &seal_of_wisdom;
        creators["seal of justice"] = &seal_of_justice;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["judgement of wisdom"] = &judgement_of_wisdom;
        creators["divine shield"] = &divine_shield;
        creators["flash of light"] = &flash_of_light;
        creators["flash of light on party"] = &flash_of_light_on_party;
        creators["retribution aura"] = &retribution_aura;
        creators["taunt spell"] = &hand_of_reckoning;
        creators["righteous defense"] = &righteous_defense;
        creators["avenger's shield"] = &avengers_shield;
        creators["divine sacrifice"] = &divine_sacrifice;
    }

private:
    static ActionNode* retribution_aura(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("retribution aura",
                              /*P*/ {},
                              /*A*/ { NextAction("devotion aura") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_poison(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("cleanse poison",
                              /*P*/ {},
                              /*A*/ { NextAction("purify poison") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_disease(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("cleanse disease",
                              /*P*/ {},
                              /*A*/ { NextAction("purify disease") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_poison_on_party(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("cleanse poison on party",
                              /*P*/ {},
                              /*A*/ { NextAction("purify poison on party") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_disease_on_party(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("cleanse disease on party",
                              /*P*/ {},
                              /*A*/ { NextAction("purify disease on party") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_corruption(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("seal of corruption",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of vengeance") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_vengeance(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("seal of vengeance",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of command") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_command(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("seal of command",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of righteousness") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_wisdom(PlayerbotAI* /* botAI */)
    {
        return new ActionNode ("seal of wisdom",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of corruption") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_justice(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("seal of justice",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of corruption") },
                              /*C*/ {});
    }
    static ActionNode* hand_of_reckoning(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("hand of reckoning",
                              /*P*/ {},
                              /*A*/ { NextAction("righteous defense") },
                              /*C*/ {});
    }
    static ActionNode* righteous_defense(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("righteous defense",
                              /*P*/ {},
                              /*A*/ { NextAction("avenger's shield") },
                              /*C*/ {});
    }
    static ActionNode* avengers_shield(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("avenger's shield",
                              /*P*/ {},
                              /*A*/ { NextAction("judgement of wisdom") },
                              /*C*/ {});
    }
    static ActionNode* divine_sacrifice(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("divine sacrifice",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ { NextAction("cancel divine sacrifice") });
    }
    static ActionNode* judgement_of_wisdom(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("judgement of wisdom",
                              /*P*/ {},
                              /*A*/ { NextAction("judgement of light") },
                              /*C*/ {});
    }
    static ActionNode* divine_shield(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("divine shield",
                              /*P*/ {},
                              /*A*/ { NextAction("divine protection") },
                              /*C*/ {});
    }
    static ActionNode* flash_of_light(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("flash of light",
                              /*P*/ {},
                              /*A*/ { NextAction("holy light") },
                              /*C*/ {});
    }
    static ActionNode* flash_of_light_on_party(PlayerbotAI* /* botAI */)
    {
        return new ActionNode("flash of light on party",
                              /*P*/ {},
                              /*A*/ { NextAction("holy light on party") },
                              /*C*/ {});
    }
};

#endif
