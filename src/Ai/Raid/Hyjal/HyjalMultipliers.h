/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALMULTIPLIERS_H
#define PLAYERBOTS_HYJALMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Multiplier.h"
#include <string>

// General

class HyjalEncounterMultiplier : public Multiplier
{
public:
    HyjalEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, HyjalHelpers::HYJAL_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class HyjalDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    HyjalDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "hyjal delay dps cooldowns") {}
    float GetValue(Action* action) override;
};

class HyjalDisableDisperseAndTankFaceMultiplier : public HyjalEncounterMultiplier
{
public:
    HyjalDisableDisperseAndTankFaceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "hyjal disable disperse and tank face") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Rage Winterchill

class RageWinterchillMeleeControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    RageWinterchillMeleeControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "rage winterchill melee control avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class RageWinterchillRangedControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    RageWinterchillRangedControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "rage winterchill ranged control avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Anetheron

class AnetheronDisableAssistTargetingMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronDisableAssistTargetingMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "anetheron disable assist targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronAvoidAccidentalInfernalAggroMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronAvoidAccidentalInfernalAggroMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "anetheron avoid accidental infernal aggro") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronInfernalTargetRunToPositionMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronInfernalTargetRunToPositionMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "anetheron infernal target run to position") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronControlMovementMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronControlMovementMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "anetheron control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AnetheronControlMisdirectionMultiplier : public HyjalEncounterMultiplier
{
public:
    AnetheronControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "anetheron control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kaz'rogal

class KazrogalControlLowManaMovementMultiplier : public HyjalEncounterMultiplier
{
public:
    KazrogalControlLowManaMovementMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "kaz'rogal control low mana movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KazrogalKeepAspectOfTheViperActiveMultiplier : public HyjalEncounterMultiplier
{
public:
    KazrogalKeepAspectOfTheViperActiveMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "kaz'rogal keep aspect of the viper active") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Azgalor

class AzgalorDisableAutoTargetingAndPositioningMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorDisableAutoTargetingAndPositioningMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "azgalor disable auto targeting and positioning") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AzgalorDoomedBotPrioritizePositioningMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorDoomedBotPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "azgalor doomed bot prioritize positioning") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AzgalorMeleeDpsControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorMeleeDpsControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "azgalor melee dps control avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AzgalorRangedControlAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    AzgalorRangedControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "azgalor ranged control avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Archimonde

class ArchimondeSetTremorTotemMultiplier : public HyjalEncounterMultiplier
{
public:
    ArchimondeSetTremorTotemMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "archimonde set tremor totem") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ArchimondeControlDoomfireAvoidanceMultiplier : public HyjalEncounterMultiplier
{
public:
    ArchimondeControlDoomfireAvoidanceMultiplier(PlayerbotAI* botAI)
        : HyjalEncounterMultiplier(botAI, "archimonde control doomfire avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
