/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULTRIGGERS_H
#define PLAYERBOTS_GRUULTRIGGERS_H

#include "EncounterHelpers.h"
#include "GruulHelpers.h"
#include "Trigger.h"
#include <string>

// Shared

class GruulsLairEncounterTrigger : public Trigger
{
public:
    GruulsLairEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, GruulHelpers::GRUUL_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class GruulsLairNoEncounterInProgressTrigger : public Trigger
{
public:
    GruulsLairNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul's lair no encounter in progress", 1000) {}
    bool IsActive() override;
};

// High King Maulgar <Lord of the Ogres>

class HighKingMaulgarThreeOgresNeedMeleeTanksTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar three ogres need melee tanks") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarKroshNeedsMageTankTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarKroshNeedsMageTankTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar krosh needs mage tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarKigglerNeedsMoonkinTankTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarKigglerNeedsMoonkinTankTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar kiggler needs moonkin tank") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarShouldAssignDpsPriorityTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar should assign dps priority") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarChannelingWhirlwindTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar channeling whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarShouldStandBackFromKroshTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarShouldStandBackFromKroshTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar should stand back from krosh") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarWildFelStalkerSpawnedTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarWildFelStalkerSpawnedTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar wild fel stalker spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

class HighKingMaulgarHunterShouldMisdirectTrigger : public GruulsLairEncounterTrigger
{
public:
    HighKingMaulgarHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "high king maulgar hunter should misdirect") {}

protected:
    bool IsActiveInEncounter() override;
};

// Gruul the Dragonkiller

class GruulTheDragonkillerShouldBeTankedTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerRangedShouldSpreadTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerBotInCaveInTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerBotInCaveInTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller bot in cave in") {}

protected:
    bool IsActiveInEncounter() override;
};

class GruulTheDragonkillerIncomingShatterTrigger : public GruulsLairEncounterTrigger
{
public:
    GruulTheDragonkillerIncomingShatterTrigger(PlayerbotAI* botAI)
        : GruulsLairEncounterTrigger(botAI, "gruul the dragonkiller incoming shatter") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
