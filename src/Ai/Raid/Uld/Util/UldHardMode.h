/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDHARDMODE_H
#define PLAYERBOTS_ULDHARDMODE_H

#include "Define.h"

#include <vector>

class Player;
class PlayerbotAI;
class Unit;

// Ulduar hard modes are a raid choice, not the 10/25 heroic flag. Each is its boss's config switch alone,
// and the per-boss triggers still check their own mechanic, so the option must match what the raid does

// General Vezax: the raid keeps six Saronite Vapors alive, which spawns the Saronite Animus (33524)
// and gives Vezax the invulnerable Saronite Barrier until the Animus dies.
bool IsVezaxHardModeActive(PlayerbotAI* botAI);

// Assembly of Iron: the hard mode is a kill-order choice - leave Steelbreaker (32867) for last so
// he reaches his empowered phase 3.
bool IsIronAssemblyHardModeActive(PlayerbotAI* botAI);

// A phase check, not gated on the hard-mode option: alive with Molgeim and Brundir dead means phase 3,
// and a raid that gets there without the option set still needs the tank swap
bool IsSteelbreakerEmpowered(PlayerbotAI* botAI);
// Overload for callers that already resolved the three members, so a per-tick path does not repeat
// three grid lookups.
bool IsSteelbreakerEmpowered(PlayerbotAI* botAI, Unit* steelbreaker, Unit* molgeim, Unit* brundir);

// The council member the raid should kill next. Hard mode saves Steelbreaker for last; the normal
// order kills him first. Returns nullptr if none are alive.
Unit* GetIronAssemblyNextKillTarget(PlayerbotAI* botAI);

// Flame Leviathan: FL_TOWER_ALL when the option is on, else 0. Claiming all four is free, hazards are
// found by NPC entry so a destroyed tower adds none
uint32 FlameLeviathanActiveTowerMask(PlayerbotAI* botAI);

// Thorim: hard mode = the arena gauntlet is cleared fast enough that Sif (33196) interrupts her
// channel and joins the fight instead of despawning.
bool IsThorimHardModeActive(PlayerbotAI* botAI);

// Freya: an Elder left alive at the pull empowers her for the whole fight, even after it dies. The
// triggers check the hazard itself (rooted bot, Unstable Sun Beam), which only exists in hard mode
bool IsFreyaHardModeActive(PlayerbotAI* botAI);

// Mimiron "Firefighter": the Big Red Button pressed before the pull adds the spreading fire and Frost
// Bomb. GetMimironKeptFireBots picks the P3 Emergency Fire Bots kept alive to put the fire out
bool IsMimironHardModeActive(PlayerbotAI* botAI);

// XT-002: the raid kills the exposed Heart. No server signal exists before that kill, and this decides
// whether bots burn the Heart or stop at ULDUAR_XT002_HEART_SAFE_HP_PCT
bool IsXT002HardModeActive(PlayerbotAI* botAI);

// XT already has Heartbreak, so no more Heart phases. Config-independent on purpose, so a Heartbreak
// the humans triggered counts too
bool IsXT002HeartbreakActive(Player* bot, Unit* xt002);

// Thorim freed, off the watchers mask since Keepers are picked in-instance. Without his Titanic Storm,
// Weakened Immortal Guardians can't die, so P3 keeps them low in a pile
bool YoggThorimKeeperActive(PlayerbotAI* botAI);

// How many Keepers the raid freed, off the same mask. Under 4 in 25-man, Yogg casts Deafening Roar.
uint32 YoggKeepersFreedCount(PlayerbotAI* botAI);

#endif
