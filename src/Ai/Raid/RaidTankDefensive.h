/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTANKDEFENSIVE_H
#define PLAYERBOTS_RAIDTANKDEFENSIVE_H

#include "Define.h"

#include <string>
#include <vector>

class Player;
class PlayerbotAI;

// Weakest tank cooldown the bot can cast now, by cooldown length, or 0 when none is castable or one is
// already running. physicalOnly skips Anti-Magic Shell for a hit it would be wasted on.
uint32 NextTankDefensive(PlayerbotAI* botAI, Player* bot, bool physicalOnly = false);

// Same pick for an incoming magic hit, any role. needMs is how far off the hit is, so a button that
// runs out before it lands is skipped. 0 only asks whether anything is castable.
uint32 NextMagicDefensive(PlayerbotAI* botAI, Player* bot, uint32 needMs);

// Same pick for a physical hit, without Anti-Magic Shell and Cloak of Shadows
uint32 NextPhysicalDefensive(PlayerbotAI* botAI, Player* bot, uint32 needMs);

// Percent of a magic hit the bot can still take off on its own, the running button's if one is up. 0 when
// nothing is castable or the pick adds health instead of cutting damage.
uint8 BestMagicDefensiveCut(PlayerbotAI* botAI, Player* bot, uint32 needMs);

// Cast name of a tank button off cooldown within withinMs, or nullptr. For a hit still seconds away:
// NextTankDefensive also says 0 on the global cooldown, mid channel or with a button running.
char const* ReadyTankDefensive(Player* bot, bool physicalOnly, uint32 withinMs = 0);
// Same pick as a spell id, skipping a button that runs out within needMs. Works for a human tank too.
uint32 ReadyTankDefensiveSpell(Player* bot, bool physicalOnly, uint32 withinMs, uint32 needMs);
bool IsMagicOnlyTankDefensive(uint32 spellId);
// Cast name of a tank button already up on the bot for at least forMs more, or nullptr
char const* RunningTankDefensive(Player* bot, bool physicalOnly, uint32 forMs);

// For an encounter that holds the class nodes until the window it wants them in. The tank check only
// matches buttons NextTankDefensive offers, the magic list has every button.
bool IsHeldTankDefensive(std::string const& actionName);
std::vector<std::string> HeldMagicDefensiveNames();

#endif
