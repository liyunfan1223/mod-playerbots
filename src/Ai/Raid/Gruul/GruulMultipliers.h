/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULMULTIPLIERS_H
#define PLAYERBOTS_GRUULMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Multiplier.h"
#include <string>

// General

class GruulsLairEncounterMultiplier : public Multiplier
{
public:
    GruulsLairEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, GruulHelpers::GRUUL_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class GruulsLairDelayDpsCooldownsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulsLairDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "gruul's lair delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// High King Maulgar <Lord of the Ogres>

class HighKingMaulgarControlTankActionsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarControlTankActionsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "high king maulgar control tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarRestrictTauntingMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarRestrictTauntingMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "high king maulgar restrict taunting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarDisableDpsAssistMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarDisableDpsAssistMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "high king maulgar disable dps assist") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarAvoidWhirlwindMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "high king maulgar avoid whirlwind") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarControlHunterActionsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarControlHunterActionsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "high king maulgar control hunter actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighKingMaulgarControlMageTankActionsMultiplier : public GruulsLairEncounterMultiplier
{
public:
    HighKingMaulgarControlMageTankActionsMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "high king maulgar control mage tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Gruul the Dragonkiller

class GruulTheDragonkillerControlTankMovementMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerControlTankMovementMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "gruul the dragonkiller control tank movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GruulTheDragonkillerStaySpreadForShatterMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerStaySpreadForShatterMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "gruul the dragonkiller stay spread for shatter") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GruulTheDragonkillerControlAvoidanceMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerControlAvoidanceMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "gruul the dragonkiller control avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GruulTheDragonkillerHoldWhileSnaredMultiplier : public GruulsLairEncounterMultiplier
{
public:
    GruulTheDragonkillerHoldWhileSnaredMultiplier(PlayerbotAI* botAI)
        : GruulsLairEncounterMultiplier(botAI, "gruul the dragonkiller hold while snared") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
