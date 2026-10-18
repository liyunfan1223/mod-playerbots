/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDDEFINITIONS_H
#define PLAYERBOTS_ULDDEFINITIONS_H

#include <vector>

#include "RaidEncounter.h"

EncounterDefinition const& UldAuriayaDefinition();
EncounterDefinition const& UldKologarnDefinition();
EncounterDefinition const& UldRazorscaleDefinition();
EncounterDefinition const& UldXT002Definition();
EncounterDefinition const& UldFreyaDefinition();
EncounterDefinition const& UldAlgalonDefinition();
EncounterDefinition const& UldIgnisDefinition();
EncounterDefinition const& UldIronAssemblyDefinition();
EncounterDefinition const& UldFlameLeviathanDefinition();
EncounterDefinition const& UldHodirDefinition();
EncounterDefinition const& UldThorimDefinition();
EncounterDefinition const& UldYoggSaronDefinition();
EncounterDefinition const& UldMimironDefinition();
EncounterDefinition const& UldVezaxDefinition();

// Every Ulduar boss with a definition. The contexts and the strategy's tick walk this.
inline std::vector<EncounterDefinition const*> const& UldEncounterDefinitions()
{
    static std::vector<EncounterDefinition const*> const definitions = {
        &UldAuriayaDefinition(),
        &UldKologarnDefinition(),
        &UldRazorscaleDefinition(),
        &UldXT002Definition(),
        &UldFreyaDefinition(),
        &UldAlgalonDefinition(),
        &UldIgnisDefinition(),
        &UldIronAssemblyDefinition(),
        &UldFlameLeviathanDefinition(),
        &UldHodirDefinition(),
        &UldThorimDefinition(),
        &UldYoggSaronDefinition(),
        &UldMimironDefinition(),
        &UldVezaxDefinition(),
    };
    return definitions;
}

#endif
