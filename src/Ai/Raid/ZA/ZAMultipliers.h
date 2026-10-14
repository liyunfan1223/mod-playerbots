/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZAMULTIPLIERS_H
#define PLAYERBOTS_ZAMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "ZAHelpers.h"
#include <string>

class ZulAmanEncounterMultiplier : public Multiplier
{
public:
    ZulAmanEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, ZaHelpers::ZA_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

// General

class ZulAmanDelayDpsCooldownsMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanDisableTankActionsMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanControlMisdirectionMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanDisableCombatFormationMoveMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman disable combat formation move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZulAmanAvoidWhirlwindMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZulAmanAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'aman avoid whirlwind") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Akil'zon <Eagle Avatar>

class AkilzonStayInEyeOfTheStormMultiplier : public ZulAmanEncounterMultiplier
{
public:
    AkilzonStayInEyeOfTheStormMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "akil'zon stay in eye of the storm") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Nalorakk <Bear Avatar>

// Jan'alai <Dragonhawk Avatar>

class JanalaiStayAwayFromFireBombsMultiplier : public ZulAmanEncounterMultiplier
{
public:
    JanalaiStayAwayFromFireBombsMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "jan'alai stay away from fire bombs") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class JanalaiDoNotCrowdControlHatchersMultiplier : public ZulAmanEncounterMultiplier
{
public:
    JanalaiDoNotCrowdControlHatchersMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "jan'alai do not crowd control hatchers") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Halazzi <Lynx Avatar>

class HalazziDisableAutoDpsTargetingMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HalazziDisableAutoDpsTargetingMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "halazzi disable auto dps targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Hex Lord Malacrass

class HexLordMalacrassUnstableAfflictionMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HexLordMalacrassUnstableAfflictionMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "hex lord malacrass unstable affliction") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HexLordMalacrassSpellReflectionMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HexLordMalacrassSpellReflectionMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "hex lord malacrass spell reflection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HexLordMalacrassStayAwayFromFreezingTrapMultiplier : public ZulAmanEncounterMultiplier
{
public:
    HexLordMalacrassStayAwayFromFreezingTrapMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "hex lord malacrass stay away from freezing trap") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Zul'jin

class ZuljinStopAttackingDuringPhaseChangeMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZuljinStopAttackingDuringPhaseChangeMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'jin stop attacking during phase change") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ZuljinEagleDisableAvoidAoeMultiplier : public ZulAmanEncounterMultiplier
{
public:
    ZuljinEagleDisableAvoidAoeMultiplier(PlayerbotAI* botAI)
        : ZulAmanEncounterMultiplier(botAI, "zul'jin eagle disable avoid aoe") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
