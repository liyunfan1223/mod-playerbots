/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_Auriaya.h"

#include "UldEncounter_Auriaya.h"

// Base class checks shaman and totem action first, so this only runs for actions that could take the earth slot.
bool AuriayaAntiFearTotemGuardMultiplier::FearWindowActive() { return AuriayaFearWindowActive(botAI); }
