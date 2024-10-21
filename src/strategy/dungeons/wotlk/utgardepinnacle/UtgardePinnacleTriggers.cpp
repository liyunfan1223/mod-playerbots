#include "Playerbots.h"
#include "UtgardePinnacleTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool SkadiFreezingCloudTrigger::IsActive()
{
    Unit* bossMount = AI_VALUE2(Unit*, "find target", "grauf");
    if (!bossMount) { return false; }

    // Need to check two conditions here - the persistent ground effect doesn't
    // seem to be detectable until 3-5 secs in, despite it dealing damage.
    // The initial breath triggers straight away but once it's over, the bots will run back on
    // to the freezing cloud and take damage.
    // Therefore check both conditions and trigger on either.

    // Check this one first, if true then we don't need to iterate over any objects
    if (bossMount->HasAura(SPELL_FREEZING_CLOUD_BREATH))
    {
        return true;
    }

    // Otherwise, check for persistent ground objects emitting the freezing cloud
    GuidVector objects = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto i = objects.begin(); i != objects.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_BREATH_TRIGGER)
        {
            Unit::AuraApplicationMap const& Auras = unit->GetAppliedAuras();
            for (Unit::AuraApplicationMap::const_iterator itr = Auras.begin(); itr != Auras.end(); ++itr)
            {
                Aura* aura = itr->second->GetBase();
                if (aura && aura->GetId() == SPELL_FREEZING_CLOUD)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool SkadiWhirlwindTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "skadi the ruthless");
    return boss && boss->HasAura(SPELL_SKADI_WHIRLWIND);
}
