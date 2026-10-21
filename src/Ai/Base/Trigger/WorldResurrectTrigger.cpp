/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WorldResurrectTrigger.h"

#include "WorldResurrectTargetValue.h"

bool WorldResurrectTrigger::IsActive() { return WorldResurrect::CanHelp(botAI) && WorldResurrect::GetTarget(botAI); }
