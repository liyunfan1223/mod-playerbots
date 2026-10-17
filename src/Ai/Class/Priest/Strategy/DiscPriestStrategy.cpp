/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DiscPriestStrategy.h"
#include "Playerbots.h"

DiscPriestStrategy::DiscPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    // No custom ActionNodeFactory needed
}

std::vector<NextAction> DiscPriestStrategy::getDefaultActions()
{
    return {
        NextAction("shoot", ACTION_DEFAULT)
    };
}

void DiscPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "group heal setting",
            {
                NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 8),
                NextAction("power word: shield on not full", ACTION_MEDIUM_HEAL + 7)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "medium group heal setting",
            {
                NextAction("divine hymn", ACTION_CRITICAL_HEAL + 7),
                NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 6),
                NextAction("power word: shield on not full", ACTION_CRITICAL_HEAL + 5),
                NextAction("prayer of healing on party", ACTION_CRITICAL_HEAL + 4)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member critical health",
            {
                NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 5),
                NextAction("penance on party", ACTION_CRITICAL_HEAL + 4),
                NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 3),
                NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 2)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member low health",
            {
                NextAction("power word: shield on party", ACTION_MEDIUM_HEAL + 4),
                NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 3),
                NextAction("penance on party", ACTION_MEDIUM_HEAL + 2),
                NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 0)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member medium health",
            {
                NextAction("power word: shield on party", ACTION_LIGHT_HEAL + 9),
                NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 7),
                NextAction("penance on party", ACTION_LIGHT_HEAL + 6),
                NextAction("flash heal on party", ACTION_LIGHT_HEAL + 5)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member almost full health",
            {
                NextAction("power word: shield on party", ACTION_LIGHT_HEAL + 3),
                NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 2),
                NextAction("renew on party", ACTION_LIGHT_HEAL + 1)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "party member to heal out of spell range",
            {
                NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "critical health", {
                NextAction("pain suppression", ACTION_EMERGENCY + 1)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "protect party member",
            {
                NextAction("pain suppression on party", ACTION_EMERGENCY)
            }
        )
    );
}
