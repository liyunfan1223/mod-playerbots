/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WaitForAttackStrategy.h"
#include "Action.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Strategy.h"

void WaitForAttackStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wait for attack safe distance",
        {
            NextAction("wait for attack keep safe distance", ACTION_RAID)
        }
    ));
}

void WaitForAttackStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new WaitForAttackMultiplier(botAI));
}

bool WaitForAttackStrategy::ShouldWait(PlayerbotAI* botAI)
{
    if (botAI->HasStrategy("wait for attack", BOT_STATE_COMBAT))
    {
        Player* bot = botAI->GetBot();
        if (bot->GetGroup() && botAI->HasGameClientMaster())
        {
            // Don't wait if the current target is an enemy player
            Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            if (target && target->IsPlayer())
                return false;

            AiObjectContext* context = botAI->GetAiObjectContext();
            time_t combatStartTime = context->GetValue<time_t>("combat start time")->Get();

            if (botAI->GetState() == BOT_STATE_COMBAT)
            {
                if (combatStartTime == 0)
                {
                    combatStartTime = time(nullptr);
                    context->GetValue<time_t>("combat start time")->Set(combatStartTime);
                }

                return time(nullptr) - combatStartTime < GetWaitTime(botAI);
            }
        }
    }

    return false;
}

uint8 WaitForAttackStrategy::GetWaitTime(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<uint8>("wait for attack time")->Get();
}

float WaitForAttackStrategy::GetSafeDistance()
{
    return sPlayerbotAIConfig.SpellDistance;
}

float WaitForAttackMultiplier::GetValue(Action* action)
{
    std::string const& actionName = action->getName();

    if (actionName != "wait for attack keep safe distance" &&
        actionName != "dps assist" &&
        actionName != "set facing" &&
        actionName != "pull my target" &&
        actionName != "pull rti target" &&
        actionName != "reach pull" &&
        actionName != "pull start" &&
        actionName != "pull action" &&
        actionName != "pull end")
    {
        return WaitForAttackStrategy::ShouldWait(botAI) ? 0.0f : 1.0f;
    }

    return 1.0f;
}
