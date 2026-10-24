/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EOETRIGGERCONTEXT_H
#define PLAYERBOTS_EOETRIGGERCONTEXT_H

#include "EoEDefinitions.h"
#include "NamedObjectContext.h"

class RaidEoETriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidEoETriggerContext() { EoEMalygosDefinition().RegisterTriggers(creators); }
};

#endif
