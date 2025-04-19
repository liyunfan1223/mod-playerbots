// RaidOnyxiaActions.h
#ifndef _PLAYERBOT_RAIDONYXIAACTIONS_H_
#define _PLAYERBOT_RAIDONYXIAACTIONS_H_

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

class PlayerbotAI;

class RaidOnyxiaMoveToSideAction : public MovementAction
{
public:
    RaidOnyxiaMoveToSideAction(PlayerbotAI* botAI, std::string const name = "ony move to side")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

class RaidOnyxiaSpreadOutAction : public MovementAction
{
public:
    RaidOnyxiaSpreadOutAction(PlayerbotAI* botAI, std::string const name = "ony spread out")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

struct SafeZone
{
    Position pos;
    float radius;
};

class RaidOnyxiaMoveToSafeZoneAction : public MovementAction
{
public:
    RaidOnyxiaMoveToSafeZoneAction(PlayerbotAI* botAI, std::string const name = "ony move to safe zone")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;

private:
    std::vector<SafeZone> GetSafeZonesForBreath(uint32 spellId)
    {
        // Define your safe zone coordinates based on the map
        // Example assumes Onyxia's lair map coordinates
        float z = bot->GetPositionZ();  // Stay at current height

        switch (spellId)
        {
            case 17086:  // N to S
            case 18351:  // S to N
                return {SafeZone{Position(-10.0f, -180.0f, z), 5.0f},
                        SafeZone{Position(-20.0f, -250.0f, z), 5.0f}};  // Bottom Safe Zone

            case 18576:  // E to W
            case 18609:  // W to E
                return {
                    SafeZone{Position(20.0f, -210.0f, z), 5.0f},
                    SafeZone{Position(-75.0f, -210.0f, z), 5.0f},
                };  // Left Safe Zone

            case 18564:  // SE to NW
            case 18584:  // NW to SE
                return {
                    SafeZone{Position(-60.0f, -195.0f, z), 5.0f},
                    SafeZone{Position(10.0f, -240.0f, z), 5.0f},
                };  // NW Safe Zone

            case 18596:  // SW to NE
            case 18617:  // NE to SW
                return {
                    SafeZone{Position(7.0f, -185.0f, z), 5.0f},
                    SafeZone{Position(-60.0f, -240.0f, z), 5.0f},
                };  // NE Safe Zone

            default:
                return {SafeZone{Position(0.0f, 0.0f, z), 5.0f}};  // Fallback center - shouldn't ever happen
        }
    }
};

class RaidOnyxiaKillWhelpsAction : public AttackAction
{
public:
    RaidOnyxiaKillWhelpsAction(PlayerbotAI* botAI, std::string const name = "ony kill whelps")
        : AttackAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
};

class OnyxiaAvoidEggsAction : public MovementAction
{
public:
    OnyxiaAvoidEggsAction(PlayerbotAI* botAI) : MovementAction(botAI, "ony avoid eggs move") {}

    bool Execute(Event event) override;
};

#endif
