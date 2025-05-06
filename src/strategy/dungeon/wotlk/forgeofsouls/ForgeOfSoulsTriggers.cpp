#include "Playerbots.h"
#include "ForgeOfSoulsTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");

    if (boss && boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL) && bot->HasAura(SPELL_CORRUPT_SOUL))
        return true;

    return false;
}

bool SwitchToSoulFragment::IsActive()
{
    Unit* fragment = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            if (botAI->IsDps(bot))
                return true;
        }
    }

    return false;
    
}

bool BronjahmPositionTrigger::IsActive()
{

    return bool(AI_VALUE2(Unit*, "find target", "bronjahm"));
}
