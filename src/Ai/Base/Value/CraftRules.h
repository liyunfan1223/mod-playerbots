/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTRULES_H
#define PLAYERBOTS_CRAFTRULES_H

#include "Define.h"
#include <string>
#include <vector>

// Pure helpers for profession links. No core types, so they are unit-testable without a server.
namespace CraftRules
{
// The recipe field of a |Htrade: link: one bit per skill line entry, six per character, least significant bit first,
// in standard base64 without padding.
std::string EncodeRecipeBits(std::vector<bool> const& known);
}  // namespace CraftRules

#endif
