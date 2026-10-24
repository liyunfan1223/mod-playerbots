/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOEACTIONCONTEXT_H
#define PLAYERBOTS_EOEACTIONCONTEXT_H

#include "Action.h"
#include "EoEDefinitions.h"
#include "NamedObjectContext.h"

class RaidEoEActionContext : public NamedObjectContext<Action>
{
public:
    RaidEoEActionContext() { EoEMalygosDefinition().RegisterActions(creators); }
};

#endif
