/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ENEMYHEALERTARGETVALUE_H
#define _PLAYERBOT_ENEMYHEALERTARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class EnemyHealerTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    EnemyHealerTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "enemy healer target") {}

protected:
    Unit* Calculate() override;
};

#endif
