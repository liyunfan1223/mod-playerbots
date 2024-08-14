/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGACTION_H
#define _PLAYERBOT_RPGACTION_H

#include "MovementActions.h"

class ObjectGuid;
class PlayerbotAI;
class Unit;

class RpgAction : public MovementAction
{
public:
    RpgAction(PlayerbotAI* botAI, std::string const name = "rpg") : MovementAction(botAI, name) { }
    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    virtual bool SetNextRpgAction();
    typedef void (RpgAction::*RpgElement)(ObjectGuid guid);
    bool AddIgnore(ObjectGuid guid);
    bool RemIgnore(ObjectGuid guid);
    bool HasIgnore(ObjectGuid guid);
};

class CRpgAction : public RpgAction
{
public:
    CRpgAction(PlayerbotAI* botAI) : RpgAction(botAI, "crpg") { }
    bool isUseful() override;
};

#endif
