#/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UnholyDKStrategy.h"
#include "Playerbots.h"

class UnholyDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
	    UnholyDKStrategyActionNodeFactory()
	    {
		    // Unholy
		    //creators["bone shield"] = &bone_shield;
		    //creators["plague strike"] = &plague_strike;
		    //creators["death grip"] = &death_grip;
		    //creators["death coil"] = &death_coil;
		    creators["death strike"] = &death_strike;
		    //creators["unholy blight"] = &unholy_blight;
		    creators["scourge strike"] = &scourge_strike;
		    //creators["death and decay"] = &death_and_decay;
		    //creators["unholy pressence"] = &unholy_pressence;
		    //creators["raise dead"] = &raise_dead;
		    //creators["army of the dead"] = &army of the dead;
		    //creators["summon gargoyle"] = &army of the dead;
		    //creators["anti magic shell"] = &anti_magic_shell;
		    //creators["anti magic zone"] = &anti_magic_zone;
		    //creators["ghoul frenzy"] = &ghoul_frenzy;
		    creators["corpse explosion"] = &corpse_explosion;
			creators["icy touch"] = &icy_touch;
	    }

    private:
	    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
	    {
		    return new ActionNode("death strike",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
	    }

	    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
	    {
		    return new ActionNode("corpse explosion",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
	    }

	    static ActionNode* scourge_strike([[maybe_unused]] PlayerbotAI* botAI)
	    {
		    return new ActionNode("scourge strike",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
	    }
		static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
		{
		    return new ActionNode("icy touch",
			    /*P*/ NextAction::array(0, new NextAction("blood presence"), nullptr),
			    /*A*/ nullptr,
			    /*C*/ nullptr);
		}
};

UnholyDKStrategy::UnholyDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new UnholyDKStrategyActionNodeFactory());
}


NextAction** UnholyDKStrategy::getDefaultActions()
{
    return NextAction::array(0, 
		new NextAction("scourge strike", ACTION_DEFAULT + 0.8f), 
		new NextAction("blood strike", ACTION_DEFAULT + 0.7f), 
		new NextAction("ghoul frenzy", ACTION_DEFAULT + 0.6f), 
		new NextAction("summon gargoyle", ACTION_DEFAULT + 0.5f),
		new NextAction("death coil", ACTION_DEFAULT + 0.4f),
		new NextAction("plague strike", ACTION_DEFAULT + 0.3f), 
		new NextAction("icy touch", ACTION_DEFAULT + 0.2f),
		new NextAction("horn of winter", ACTION_DEFAULT + 0.1f), 
		new NextAction("melee", ACTION_DEFAULT), 
		nullptr);
}

void UnholyDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

	// triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction(, ACTION_NORMAL + 2), nullptr)));
	triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("death pact", ACTION_EMERGENCY + 1), nullptr)));
}

void UnholyDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode("loot available", NextAction::array(0, new NextAction("corpse explosion", ACTION_NORMAL + 1), nullptr)));
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0, new NextAction("death and decay", ACTION_NORMAL + 3),
        new NextAction("corpse explosion", ACTION_NORMAL + 3), nullptr)));
}
