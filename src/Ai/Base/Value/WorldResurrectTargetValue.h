/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WORLDRESURRECTTARGETVALUE_H
#define PLAYERBOTS_WORLDRESURRECTTARGETVALUE_H

#include "Value.h"

class Player;
class PlayerbotAI;

namespace WorldResurrect
{
char const* GetSpellName(Player const* player);
bool CanHelp(PlayerbotAI* botAI);
Player* GetTarget(PlayerbotAI* botAI);
bool TryClaim(PlayerbotAI* botAI, ObjectGuid targetGuid);
void ReleaseClaim(ObjectGuid botGuid);
}  // namespace WorldResurrect

class WorldResurrectTargetValue : public CalculatedValue<ObjectGuid>
{
public:
    WorldResurrectTargetValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "world resurrect target", 5) {}

protected:
    ObjectGuid Calculate() override;
};

#endif
