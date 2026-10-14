/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MGTMULTIPLIERS_H
#define PLAYERBOTS_MGTMULTIPLIERS_H

#include "Multiplier.h"

class MgTCrystalFocusMultiplier : public Multiplier
{
public:
    MgTCrystalFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt crystal focus") {}
    float GetValue(Action* action) override;
};

class MgTFocusOrderMultiplier : public Multiplier
{
public:
    MgTFocusOrderMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt focus order") {}
    float GetValue(Action* action) override;
};

class MgTFocusBurstMultiplier : public Multiplier
{
public:
    MgTFocusBurstMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt focus burst") {}
    float GetValue(Action* action) override;
};

class MgTRoomLeashMultiplier : public Multiplier
{
public:
    MgTRoomLeashMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt room leash") {}
    float GetValue(Action* action) override;
};

class MgTDampeningFieldMultiplier : public Multiplier
{
public:
    MgTDampeningFieldMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt dampening field") {}
    float GetValue(Action* action) override;
};

class MgTDelrissaTremorTotemMultiplier : public Multiplier
{
public:
    MgTDelrissaTremorTotemMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt delrissa tremor totem") {}
    float GetValue(Action* action) override;
};

class MgTFlameStrikeMultiplier : public Multiplier
{
public:
    MgTFlameStrikeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt flame strike") {}
    float GetValue(Action* action) override;
};

class MgTPhoenixBurnMultiplier : public Multiplier
{
public:
    MgTPhoenixBurnMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt phoenix burn") {}
    float GetValue(Action* action) override;
};

class MgTKaelUnattackableMultiplier : public Multiplier
{
public:
    MgTKaelUnattackableMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt kael unattackable") {}
    float GetValue(Action* action) override;
};

class MgTGravityLapseMultiplier : public Multiplier
{
public:
    MgTGravityLapseMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mgt gravity lapse") {}
    float GetValue(Action* action) override;
};

#endif
