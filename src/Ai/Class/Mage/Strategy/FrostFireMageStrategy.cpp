/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FrostFireMageStrategy.h"
#include "Playerbots.h"

FrostFireMageStrategy::FrostFireMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

// ===== Default Actions =====
std::vector<NextAction> FrostFireMageStrategy::getDefaultActions()
{
    return {
        NextAction("frostfire bolt", 5.2f),
        NextAction("fire blast", 5.1f),  // cast during movement
        NextAction("shoot", 5.0f)
    };
}

// ===== Trigger Initialization =====
void FrostFireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
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
