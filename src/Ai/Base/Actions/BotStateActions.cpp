/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BotStateActions.h"

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ThreatManager.h"

namespace
{
    // The bot's own attacker, read without the 1s "attackers" cache.
    Unit* FindOwnAttacker(Player* bot)
    {
        if (Unit* victim = bot->GetVictim())
            if (victim->IsAlive() && victim->IsInWorld())
                return victim;

        for (Unit* attacker : bot->getAttackers())
            if (attacker && attacker->IsAlive() && attacker->IsInWorld())
                return attacker;

        // Being attacked does not populate getAttackers() (only Unit::Attack() does): a passive
        // victim keeps its attackers as threat references on their lists, mirrored here.
        for (auto const& entry : bot->GetThreatMgr().GetThreatenedByMeList())
        {
            Unit* attacker = entry.second ? entry.second->GetOwner() : nullptr;
            if (attacker && attacker->IsAlive() && attacker->IsInWorld() && bot->IsValidAttackTarget(attacker) &&
                bot->IsWithinDistInMap(attacker, sPlayerbotAIConfig.SightDistance))
                return attacker;
        }

        return nullptr;
    }
}  // namespace

bool WakeOnCombatStartAction::Execute(Event /*event*/)
{
    botAI->ResetActionDuration();

    // Engage the bot's own attacker: the combat engine needs a current target and "dps assist"
    // waits for the 1s cached "attackers" vector. PlayerbotAI::DoNextAction makes the switch.
    if (botAI->GetState() == BOT_STATE_COMBAT || !bot->IsInCombat() ||
        context->GetValue<Unit*>("current target")->Get())
        return true;

    Unit* attacker = FindOwnAttacker(bot);
    if (!attacker)
        return true;

    context->GetValue<Unit*>("current target")->Set(attacker);
    return true;
}

bool WakeOnCombatStartAction::isUseful()
{
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return false;

    if (botAI->IsActionDurationActive())
        return true;

    return bot->IsInCombat() && !context->GetValue<Unit*>("current target")->Get() && FindOwnAttacker(bot);
}
