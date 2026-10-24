/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPVALUES_H
#define PLAYERBOTS_PVPVALUES_H

#include "NamedObjectContext.h"
#include "SharedDefines.h"
#include "Value.h"
#include <map>
#include <vector>

class Player;
class PlayerbotAI;
class Unit;

bool IsUnavailableCombatTarget(Player const* player, Unit const* target);
bool IsValidWsgCarrier(Player const* player, Unit const* carrier, bool friendly);
bool CanRespondToWsgCarrier(PlayerbotAI* botAI, Unit* carrier);
bool IsWsgEscortTargetAllowed(PlayerbotAI* botAI, Unit const* target);
bool ShouldRegroupWsgEscort(PlayerbotAI* botAI);
bool IsWsgSupportTarget(PlayerbotAI* botAI, Unit const* target);

class BgTypeValue : public ManualSetValue<uint32>
{
public:
    BgTypeValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "bg type") {}
};

class ArenaTypeValue : public ManualSetValue<uint32>
{
public:
    ArenaTypeValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "arena type") {}
};

struct WsgTeamAssignment
{
    uint32 Role = 0;
    uint8 Defenders = 0;
    uint8 Escorts = 0;
    uint8 EscortIndex = 0;
    bool Escort = false;
    bool BaseDefender = false;
    bool Defender = false;
    bool Attacker = false;
    bool Returner = false;
    bool CommittedAttack = false;
    bool Valid = false;
};

class WsgTeamAssignmentValue : public CalculatedValue<WsgTeamAssignment>
{
public:
    WsgTeamAssignmentValue(PlayerbotAI* botAI)
        : CalculatedValue<WsgTeamAssignment>(botAI, "wsg team assignment", 2 * IN_MILLISECONDS)
    {
    }

    WsgTeamAssignment Calculate() override;

private:
    std::vector<ObjectGuid> _escortGuids;
};

class WsgSupportTargetValue : public CalculatedValue<ObjectGuid>
{
public:
    WsgSupportTargetValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "wsg support target", 250) {}
    ObjectGuid Calculate() override;
};

class BgRoleValue : public ManualSetValue<uint32>
{
public:
    BgRoleValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "bg role") {}
};

class BgMastersValue : public SingleCalculatedValue<std::vector<CreatureData const*>>, public Qualified
{
public:
    BgMastersValue(PlayerbotAI* botAI) : SingleCalculatedValue<std::vector<CreatureData const*>>(botAI, "bg masters") {}

    std::vector<CreatureData const*> Calculate() override;
};

class BgMasterValue : public CDPairCalculatedValue, public Qualified
{
public:
    BgMasterValue(PlayerbotAI* botAI) : CDPairCalculatedValue(botAI, "bg master", 60) {}

    CreatureData const* Calculate() override;
    CreatureData const* NearestBm(bool allowDead = true);
};

class RpgBgTypeValue : public CalculatedValue<BattlegroundTypeId>
{
public:
    RpgBgTypeValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "rpg bg type") {}

    BattlegroundTypeId Calculate() override;
};

class FlagCarrierValue : public UnitCalculatedValue
{
public:
    FlagCarrierValue(PlayerbotAI* botAI, bool sameTeam = false, bool ignoreRange = false)
        : UnitCalculatedValue(botAI), sameTeam(sameTeam), ignoreRange(ignoreRange)
    {
    }

    Unit* Calculate() override;

private:
    bool sameTeam;
    bool ignoreRange;
};

#endif
