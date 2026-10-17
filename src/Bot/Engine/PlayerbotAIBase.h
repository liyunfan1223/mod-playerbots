/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTAIBASE_H
#define PLAYERBOTS_PLAYERBOTAIBASE_H

#include "Define.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAIBase
{
public:
    PlayerbotAIBase(bool isBotAI);

    bool CanUpdateAI();
    void SetNextCheckDelay(uint32 const delay);
    void IncreaseNextCheckDelay(uint32 delay);
    void YieldThread(Player* bot, uint32 delay = sPlayerbotAIConfig.ReactDelay);
    virtual void UpdateAI(uint32 elapsed, bool minimal = false);
    virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false) = 0;
    bool IsActive();
    bool IsBotAI() const;

    void SetActionDuration(uint32 duration);
    void ResetActionDuration();
    bool IsActionDurationActive() const { return nextAICheckDelay > 0; }

protected:
    uint32 nextAICheckDelay;
    class PerfMonitorOperation* totalPmo = nullptr;

private:
    bool _isBotAI;
};

#endif
