/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_XT002_H
#define PLAYERBOTS_ULDTRIGGERS_XT002_H

#include "GenericTriggers.h"
#include "Trigger.h"

// XT-002 Deconstructor. Every trigger is false without XT, so the shared Ulduar strategy idles elsewhere.

// Carries Searing Light, Gravity Bomb or both. One trigger since one action has to own the bot: two nodes at
// one relevance pull it between destinations.
class XT002DebuffCarrierTrigger : public Trigger
{
public:
    static constexpr char const* Name = "xt002 debuff carrier trigger";

    XT002DebuffCarrierTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Melee never stand next to a Boombot; ranged kill it from outside the blast. Void Zones only count for a
// bot not carrying a debuff, since the carrier action owns those.
class XT002AvoidHazardTrigger : public Trigger
{
public:
    static constexpr char const* Name = "xt002 avoid hazard trigger";

    XT002AvoidHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Anchors the main tank, ranged and healers once XT is in combat, or bots walk there before the pull. Stands
// down for any dodge, a tank without aggro, and healers with someone out of range.
class XT002RaidPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "xt002 raid position trigger";

    XT002RaidPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Every role's target comes from the encounter's priority order, tanks included: generic tank targeting
// drags XT into the add pile. No raid icons: they're group-global.
class XT002SetDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "xt002 set dps priority trigger";

    XT002SetDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class XT002PummellerTauntTrigger : public Trigger
{
public:
    static constexpr char const* Name = "xt002 pummeller taunt trigger";

    XT002PummellerTauntTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class XT002RedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "xt002 redirect threat trigger";

    XT002RedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
