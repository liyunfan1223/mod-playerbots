/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_HODIR_H
#define PLAYERBOTS_ULDTRIGGERS_HODIR_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

// Carrying Biting Cold with no Toasty Fire to shed it for free. Holds no state: the action owns the arm
// threshold and the shed-to-floor latch.
class HodirBitingColdTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir biting cold";

    HodirBitingColdTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A Snowpacked Icicle Target is up, and this bot is not inside the Safe Area it radiates. That NPC
// is the only Flash Freeze exemption in the fight, a Toasty Fire doesn't grant one.
class HodirNearSnowpackedIcicleTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir near snowpacked icicle";

    HodirNearSnowpackedIcicleTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Not the shared BossFrostResistanceTrigger: it takes the first paladin whatever its spec, so a tank can
// lose Devotion or a healer Concentration, and it raises the aura through a ChangeStrategy nothing removes.
class HodirFrostResistanceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir frost resistance trigger";

    HodirFrostResistanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// An icicle is about to land on or beside this bot. Small icicles always count; a drift icicle only
// counts while it is still falling, because once it lands it becomes the shelter everyone needs.
class HodirIcicleDodgeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir icicle dodge";

    HodirIcicleDodgeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The bot is off its anchor and nothing more urgent wants the tick.
class HodirRaidPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir raid position";

    HodirRaidPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Everyone but healers: trapped raiders first, then helper blocks, then the boss. Tanks only get the boss.
class HodirSetDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir set dps priority";

    HodirSetDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Frozen Blows is up and the wrong tank is holding him.
class HodirFrozenBlowsSwapTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir frozen blows swap";

    HodirFrozenBlowsSwapTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The bot carries Storm Cloud, so it holds its rally point until the charges are spent.
class HodirSpreadStormCloudTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir spread storm cloud";

    HodirSpreadStormCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Somebody nearby is carrying, this bot deals damage, and it has not been handed Storm Power yet.
class HodirCollectStormPowerTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir collect storm power";

    HodirCollectStormPowerTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// A hunter or rogue that could hand its threat to a tank. Screened here as well as in the action's
// isUseful so the other twenty bots never reach the node.
class HodirRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "hodir redirect threat";

    HodirRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
