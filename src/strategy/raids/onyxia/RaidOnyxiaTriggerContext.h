#ifndef _PLAYERBOT_RAIDONYXIATRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDONYXIATRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidOnyxiaTriggers.h"

class RaidOnyxiaTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidOnyxiaTriggerContext()
    {
        creators["ony phase1 start"] = &RaidOnyxiaTriggerContext::phase1_start;
        creators["ony near tail"] = &RaidOnyxiaTriggerContext::near_tail;
        creators["ony deep breath warning"] = &RaidOnyxiaTriggerContext::deep_breath;
        creators["ony fireball splash incoming"] = &RaidOnyxiaTriggerContext::fireball_splash;
        creators["ony phase3 start"] = &RaidOnyxiaTriggerContext::phase3_start;
        creators["ony lava pulse"] = &RaidOnyxiaTriggerContext::lava_pulse;
    }

private:
    static Trigger* phase1_start(PlayerbotAI* ai) { return new OnyxiaPhase1StartTrigger(ai); }
    static Trigger* near_tail(PlayerbotAI* ai) { return new OnyxiaNearTailTrigger(ai); }
    static Trigger* deep_breath(PlayerbotAI* ai) { return new OnyxiaDeepBreathTrigger(ai); }
    static Trigger* fireball_splash(PlayerbotAI* ai) { return new RaidOnyxiaFireballSplashTrigger(ai); }
    static Trigger* phase3_start(PlayerbotAI* ai) { return new OnyxiaPhase3StartTrigger(ai); }
    static Trigger* lava_pulse(PlayerbotAI* ai) { return new OnyxiaLavaPulseTrigger(ai); }
};

#endif
