/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FireMageStrategy.h"
#include "Playerbots.h"
#include "Strategy.h"

FireMageStrategy::FireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

// ===== Default Actions =====
std::vector<NextAction> FireMageStrategy::getDefaultActions()
{
    return {
        NextAction("fireball", 5.3f),
        NextAction("frostbolt", 5.2f),   // fire immune target
        NextAction("fire blast", 5.1f),  // cast during movement
        NextAction("shoot", 5.0f)
    };
}

// ===== Trigger Initialization =====
void FireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // Debuff Triggers
    triggers.push_back(
        new TriggerNode(
            "improved scorch",
            {
                NextAction("scorch", 19.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "living bomb",
            {
                NextAction("living bomb", 18.5f)
            }
        )
    );

    // Proc Trigger
    triggers.push_back(
        new TriggerNode(
            "hot streak",
            {
                NextAction("pyroblast", 25.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "enemy too close for spell",
            {
                NextAction("dragon's breath", ACTION_INTERRUPT + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy is close",
            {
                NextAction("blast wave", ACTION_INTERRUPT)
            }
        )
    );
}

// Combat strategy to run to melee for Dragon's Breath and Blast Wave
// Disabled by default for the Fire/Frostfire spec
// To enable, type "co +firestarter"
// To disable, type "co -firestarter"
FirestarterStrategy::FirestarterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void FirestarterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "blast wave off cd and medium aoe",
            {
                NextAction("reach melee", 25.5f)
            }
        )
    );
}
