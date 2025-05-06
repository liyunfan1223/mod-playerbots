// OnyxiaTriggers.h
#ifndef _PLAYERBOT_ONYXIATRIGGERS_H_
#define _PLAYERBOT_ONYXIATRIGGERS_H_

#include "PlayerbotAI.h"
#include "Trigger.h"

// Mechanics
class OnyxiaDeepBreathTrigger : public Trigger
{
public:
    OnyxiaDeepBreathTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class OnyxiaNearTailTrigger : public Trigger
{
public:
    OnyxiaNearTailTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class RaidOnyxiaFireballSplashTrigger : public Trigger
{
public:
    RaidOnyxiaFireballSplashTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class RaidOnyxiaWhelpsSpawnTrigger : public Trigger
{
public:
    RaidOnyxiaWhelpsSpawnTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class OnyxiaAvoidEggsTrigger : public Trigger
{
public:
    OnyxiaAvoidEggsTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

#endif
