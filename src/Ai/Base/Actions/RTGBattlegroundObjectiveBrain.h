/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 * Ported/adapted from VincentVanclef/mod-playerbots, revision d7cc409c2e145bc4fa5b64bfc9539dab4cf11a43.
 */

#ifndef PLAYERBOTS_RTG_BATTLEGROUND_OBJECTIVE_BRAIN_H
#define PLAYERBOTS_RTG_BATTLEGROUND_OBJECTIVE_BRAIN_H

#include "ObjectGuid.h"
#include "PositionValue.h"
#include "SharedDefines.h"

class Battleground;
class BattlegroundEY;
class BattlegroundWS;
class Player;
class PlayerbotAI;

enum class BgObjectiveRole : uint8
{
    None,
    CaptureFlag,
    ReturnFlag,
    EscortCarrier,
    AttackCarrier,
    DefendFlag,
    CaptureTower,
    DefendTower,
    CarrierTurnIn,
    CarrierHold,
    Pressure
};

struct BgObjectiveAssignment
{
    BgObjectiveRole role = BgObjectiveRole::None;
    PositionInfo destination;
    ObjectGuid targetGuid;
    uint32 node = 0;
    uint32 assignedAt = 0;
    bool emergency = false;
};

// Owned by each bot action; no instance/player state survives the owning AI.
class RTGBattlegroundObjectiveBrain
{
public:
    static bool IsEnabled(Player* bot);
    bool Update(PlayerbotAI* botAI, PositionInfo& destination);
    bool ShouldHold(Player* bot) const;
    bool IsEscort() const { return _assignment.role == BgObjectiveRole::EscortCarrier; }
    void Reset();

private:
    BgObjectiveAssignment SelectWsg(PlayerbotAI* botAI, BattlegroundWS* bg, TeamId team);
    BgObjectiveAssignment SelectEots(PlayerbotAI* botAI, BattlegroundEY* bg, TeamId team);
    bool IsValid(PlayerbotAI* botAI, Battleground* bg, TeamId team) const;
    void SampleTeam(Player* bot, Battleground* bg, TeamId team);

    BgObjectiveAssignment _assignment;
    uint32 _instanceId = 0;
    uint32 _mapId = 0;
    TeamId _team = TEAM_NEUTRAL;
    uint32 _lastCheck = 0;
    uint32 _state = 0;
    uint32 _teamSize = 1;
    uint32 _teamSlot = 0;
};

#endif
