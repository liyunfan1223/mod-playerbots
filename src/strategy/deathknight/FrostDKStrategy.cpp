/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FrostDKStrategy.h"
#include "Playerbots.h"

class FrostDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
	    FrostDKStrategyActionNodeFactory()
	    {
		    //creators["icy touch"] = &icy_touch;
		    creators["obliterate"] = &obliterate;
		    creators["howling blast"] = &howling_blast;
		    creators["frost strike"] = &frost_strike;
		    //creators["chains of ice"] = &chains_of_ice;
		    creators["rune strike"] = &rune_strike;
		    //creators["icy clutch"] = &icy_clutch;
		    //creators["horn of winter"] = &horn_of_winter;
		    //creators["killing machine"] = &killing_machine;
		    //creators["frost presence"] = &frost_presence;
		    //creators["deathchill"] = &deathchill;
		    //creators["icebound fortitude"] = &icebound_fortitude;
		    //creators["mind freeze"] = &mind_freeze;
		    //creators["empower weapon"] = &empower_weapon;
		    //creators["hungering cold"] = &hungering_cold;
		    //creators["unbreakable armor"] = &unbreakable_armor;
		    //creators["improved icy talons"] = &improved_icy_talons;
	    }

	private:
		static ActionNode* obliterate([[maybe_unused]] PlayerbotAI* botAI)
		{
		    return new ActionNode("obliterate",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
		}

		static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
		{
		    return new ActionNode("rune strike",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
			    /*C*/ nullptr);
		}

		static ActionNode* frost_strike([[maybe_unused]] PlayerbotAI* botAI)
		{
		    return new ActionNode("frost strike",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
		}

		static ActionNode* howling_blast([[maybe_unused]] PlayerbotAI* botAI)
		{
		    return new ActionNode("howling blast",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
		}
};

FrostDKStrategy::FrostDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
	actionNodeFactories.Add(new FrostDKStrategyActionNodeFactory());
}

NextAction** FrostDKStrategy::getDefaultActions()
{
    return NextAction::array(0, 
		new NextAction("obliterate", ACTION_NORMAL + 5), 
		new NextAction("frost strike", ACTION_NORMAL + 4),
		// new NextAction("death strike", ACTION_NORMAL + 3),
		new NextAction("melee", ACTION_NORMAL), 
		NULL
	);
}

void FrostDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("empower weapon", NextAction::array(0, new NextAction("empower weapon", ACTION_NORMAL + 4), nullptr)));
	triggers.push_back(new TriggerNode("pestilence", NextAction::array(0, new NextAction("pestilence", ACTION_HIGH + 9), NULL)));
}

void FrostDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode("light aoe", NextAction::array(0, new NextAction("howling blast", ACTION_NORMAL + 4), nullptr)));
}
