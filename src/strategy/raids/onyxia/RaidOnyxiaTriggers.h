// OnyxiaTriggers.h
#ifndef _PLAYERBOT_ONYXIATRIGGERS_H_
#define _PLAYERBOT_ONYXIATRIGGERS_H_

#include "PlayerbotAI.h"
#include "Trigger.h"

// Phase transition triggers
class OnyxiaPhase1StartTrigger : public Trigger
{
public:
    OnyxiaPhase1StartTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class OnyxiaPhase2StartTrigger : public Trigger
{
public:
    OnyxiaPhase2StartTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class OnyxiaPhase3StartTrigger : public Trigger
{
public:
    OnyxiaPhase3StartTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

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

class OnyxiaFearedTrigger : public Trigger
{
public:
    OnyxiaFearedTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class OnyxiaLavaPulseTrigger : public Trigger
{
public:
    OnyxiaLavaPulseTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

class RaidOnyxiaFireballSplashTrigger : public Trigger
{
public:
    RaidOnyxiaFireballSplashTrigger(PlayerbotAI* botAI);
    bool IsActive() override;
};

#endif
