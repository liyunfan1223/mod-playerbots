/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#include "ForceRebuff.h"
#include "Common.h"
#include "GenericSpellActions.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Timer.h"
#include <algorithm>

static constexpr uint32 REBUFF_WINDOW_TIMEOUT_MS = 2 * MINUTE * IN_MILLISECONDS;

void ForceRebuffState::Begin(bool reply)
{
    replyToReadyCheck = reply || (IsPending() && replyToReadyCheck);
    pending = true;
    buffPendingThisCycle = true;
    beginMs = getMSTime();
}

bool ForceRebuffState::IsOnGlobalCooldown() const
{
    return bot && lastGcdSpell && bot->GetGlobalCooldownMgr().HasGlobalCooldown(lastGcdSpell);
}

bool ForceRebuffState::IsPending() const
{
    return pending && getMSTimeDiff(beginMs, getMSTime()) < REBUFF_WINDOW_TIMEOUT_MS;
}

void ForceRebuffState::NoteBuffWork()
{
    if (!bot || !IsPending() || bot->IsInCombat())
        return;

    buffPendingThisCycle = true;
}

void ForceRebuffState::NoteBuffProposed()
{
    if (!bot || !IsPending() || bot->IsInCombat())
        return;

    buffProposedThisCycle = true;
}

bool ForceRebuffState::BuffBelowRefreshTarget(int32 remaining, int32 maxDuration, uint32 baseBeforeDuration) const
{
    if (bot && IsPending() && !bot->IsInCombat())
    {
        if (remaining <= 0 || maxDuration <= 0)
            return false;

        // Anything refreshed after the rebuff began counts as topped off
        uint32 const marginMs = std::max(sPlayerbotAIConfig.ForceRebuffMarginSecs * IN_MILLISECONDS,
                                         getMSTimeDiff(beginMs, getMSTime()) + 5000);
        return uint32(remaining) + marginMs < uint32(maxDuration);
    }

    return baseBeforeDuration && uint32(remaining) < baseBeforeDuration;
}

// During a force-rebuff, healing yields to buffing: heals are suppressed while
// buff work is proposed or the last cast's GCD runs, so the pass is not stretched
// by interleaved heals.
class ForceRebuffBuffFirstMultiplier : public Multiplier
{
public:
    ForceRebuffBuffFirstMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "force rebuff buff first") {}

    float GetValue(Action* action) override
    {
        ForceRebuffState const& rebuff = botAI->forceRebuff;
        if (!action || !rebuff.IsPending() || botAI->GetBot()->IsInCombat())
            return 1.0f;

        if (!dynamic_cast<CastHealingSpellAction*>(action))
            return 1.0f;

        return rebuff.IsBuffProposedThisCycle() || rebuff.IsOnGlobalCooldown() ? 0.0f : 1.0f;
    }
};

void ForceRebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("force rebuff pending", { NextAction("ready reply", 6.0f) }));
}

void ForceRebuffStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ForceRebuffBuffFirstMultiplier(botAI));
}
