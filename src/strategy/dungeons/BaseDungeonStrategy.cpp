#include "BaseDungeonStrategy.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "GenericTriggers.h"

void BaseDungeonStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Base dungeon triggers - can be extended by specific dungeons
    
    // Automatically enable soulstone strategy for warlocks in dungeons and raids
    if (botAI->GetBot()->getClass() == CLASS_WARLOCK)
    {
        // Enable soulstone dungeon strategy automatically when in dungeon or raid
        triggers.push_back(new TriggerNode("in dungeon", 
            NextAction::array(0, new NextAction("enable soulstone dungeon", 1.0f), nullptr)));
    }
}

void BaseDungeonStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    // Base dungeon multipliers - can be extended by specific dungeons
}
