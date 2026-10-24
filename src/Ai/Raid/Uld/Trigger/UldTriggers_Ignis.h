/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_IGNIS_H
#define PLAYERBOTS_ULDTRIGGERS_IGNIS_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

// Ignis the Furnace Master
class IgnisScorchedGroundTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis scorched ground trigger";

    IgnisScorchedGroundTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisMainTankPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis main tank position trigger";

    IgnisMainTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisConstructTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis construct tank trigger";

    IgnisConstructTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisAttackBrittleConstructTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis attack brittle construct trigger";

    IgnisAttackBrittleConstructTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisAttackBossTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis attack boss trigger";

    IgnisAttackBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisMoltenConstructAvoidTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis molten construct avoid trigger";

    IgnisMoltenConstructAvoidTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisFlameJetsTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis flame jets trigger";

    IgnisFlameJetsTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IgnisSlagPotHealTrigger : public Trigger
{
public:
    static constexpr char const* Name = "ignis slag pot heal trigger";

    IgnisSlagPotHealTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
