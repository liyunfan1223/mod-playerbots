#include "UtgardePinnacleMultipliers.h"
#include "UtgardePinnacleActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "UtgardePinnacleTriggers.h"
#include "Action.h"

float SkadiMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "skadi the ruthless");
    if (!boss) { return 1.0f; }

    Unit* bossMount = AI_VALUE2(Unit*, "find target", "grauf");

    if (!bossMount)
    // Actual bossfight (dismounted)
    {
        if (boss->HasAura(SPELL_SKADI_WHIRLWIND))
        {
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidSkadiWhirlwindAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    else
    {
        // Bots tend to get stuck trying to attack the boss in the sky, not the adds on the ground
        if (dynamic_cast<AttackAction*>(action)
            && (action->GetTarget() == boss || action->GetTarget() == bossMount))
        {
            return 0.0f;
        }

        // TODO: BELOW IS EXPERIMENTAL
        // Meant to stop the jittery movement when dodging the breath.
        // Currently causes issues with making the bots unresponsive and often getting the healer killed.
        // Semi-glitchy movement is better than semi-afk bots, so this is commented out until it gets improved

        // bool cloudActive = false;
        // // Need to check two conditions here - the persistent ground effect doesn't
        // // seem to be detectable until 3-5 secs in, despite it dealing damage.
        // // The initial breath triggers straight away but once it's over, the bots will run back on
        // // to the frezzing cloud and take damage.
        // // Therefore check both conditions and trigger on either.

        // // Check this one early, if true then we don't need to iterate over any objects
        // if (bossMount->HasAura(SPELL_FREEZING_CLOUD_BREATH))
        // {
        //     cloudActive = true;
        // }

        // // Otherwise, check for persistent ground objects emitting the freezing cloud
        // GuidVector objects = AI_VALUE(GuidVector, "nearest hostile npcs");
        // for (auto i = objects.begin(); i != objects.end(); ++i)
        // {
        //     Unit* unit = botAI->GetUnit(*i);
        //     if (unit && unit->GetEntry() == NPC_BREATH_TRIGGER)
        //     {
        //         Unit::AuraApplicationMap const& Auras = unit->GetAppliedAuras();
        //         for (Unit::AuraApplicationMap::const_iterator itr = Auras.begin(); itr != Auras.end(); ++itr)
        //         {
        //             Aura* aura = itr->second->GetBase();
        //             if (aura && aura->GetId() == SPELL_FREEZING_CLOUD)
        //             {
        //                 cloudActive = true;
        //                 break;
        //             }
        //         }
        //     }
        // }

        // if (cloudActive)
        // {
        //     if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidFreezingCloudAction*>(action))
        //     {
        //         return 0.0f;
        //     }
        // }
    }

    return 1.0f;
}
