/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MGTTRIGGERS_H
#define PLAYERBOTS_MGTTRIGGERS_H

#include "MgTShared.h"
#include "Trigger.h"

class MgTCrystalActiveTrigger : public Trigger
{
public:
    MgTCrystalActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt crystal active") {}
    bool IsActive() override;
};

class MgTOutOfRoomTrigger : public Trigger
{
public:
    MgTOutOfRoomTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt out of room") {}
    bool IsActive() override;
};

class MgTInDampeningFieldTrigger : public Trigger
{
public:
    MgTInDampeningFieldTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt in dampening field") {}
    bool IsActive() override;
};

class MgTMageGuardAtRangeTrigger : public Trigger
{
public:
    MgTMageGuardAtRangeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt mage guard at range") {}
    bool IsActive() override;
};

class MgTArcaneNovaRangeTrigger : public Trigger
{
public:
    MgTArcaneNovaRangeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt arcane nova range") {}
    bool IsActive() override;
};

class MgTPriorityInterruptTrigger : public Trigger
{
public:
    MgTPriorityInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt priority interrupt") {}
    bool IsActive() override;
};

class MgTFocusTargetTrigger : public Trigger
{
public:
    MgTFocusTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt focus target") {}
    bool IsActive() override;
};

class MgTEnragedWretchedTrigger : public Trigger
{
public:
    MgTEnragedWretchedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt enraged wretched") {}
    bool IsActive() override;
};

class MgTDelrissaInterruptTrigger : public Trigger
{
public:
    MgTDelrissaInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt delrissa interrupt") {}
    bool IsActive() override;
};

class MgTDelrissaFocusTargetTrigger : public Trigger
{
public:
    MgTDelrissaFocusTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt delrissa focus target") {}
    bool IsActive() override;
};

class MgTDelrissaTremorTotemTrigger : public Trigger
{
public:
    MgTDelrissaTremorTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt delrissa tremor totem") {}
    bool IsActive() override;
};

class MgTFlameStrikeTrigger : public Trigger
{
public:
    MgTFlameStrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt flame strike") {}
    bool IsActive() override;
};

class MgTPhoenixBurnTrigger : public Trigger
{
public:
    MgTPhoenixBurnTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt phoenix burn") {}
    bool IsActive() override;
};

class MgTKaelFocusTargetTrigger : public Trigger
{
public:
    MgTKaelFocusTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt kael focus target") {}
    bool IsActive() override;
};

class MgTKaelInterruptTrigger : public Trigger
{
public:
    MgTKaelInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt kael interrupt") {}
    bool IsActive() override;
};

class MgTGravityLapseTrigger : public Trigger
{
public:
    MgTGravityLapseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mgt gravity lapse") {}
    bool IsActive() override;
};

#endif
