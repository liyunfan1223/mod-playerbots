/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTMGR_H
#define PLAYERBOTS_PVPLOADOUTMGR_H

#include "PvpLoadoutRules.h"
#include <optional>

class Player;
class PlayerbotAI;

// Applies the PvP loadout to a bot and restores its PvE state. Runs on the bot's map thread, out of combat.
class PvpLoadoutMgr
{
public:
    // The arena the bot is in, or 0 when it is not in one.
    static uint32 GetArenaInstanceId(Player* bot);

    // What the swap should do now, given the bot's stored loadout.
    static PvpLoadout::Transition GetTransition(Player* bot, std::optional<PvpLoadout::Snapshot> const& loadout);

    // Fills the talent link and glyphs of the snapshot from the bot's current (PvE) state.
    static void CaptureTalents(Player* bot, PvpLoadout::Snapshot& snapshot);

    // Respecs to the PvP premade for the bot's talent tree and applies that premade's glyphs; no premade, no change.
    static void ApplyPvpTalents(Player* bot);

    // Puts the snapshot's talents and glyphs back, then spends any talent points gained since it was taken.
    static void RestoreTalents(Player* bot, PvpLoadout::Snapshot const& snapshot);

    // The opponents' rating for the bot's arena; not ready while a skirmish waits for its opponents.
    static PvpLoadout::MatchRating ResolveMatchRating(Player* bot);

    // Plans the match gear and copies the PvE items it replaces, without touching the bot; save before applying. Call
    // after ApplyPvpTalents: the PvP talents decide rankings, dual wield and Titan's Grip.
    static void PlanMatchGear(Player* bot, uint32 rating, PvpLoadout::Snapshot& snapshot);

    // Once the copies are saved: destroys the copied PvE items, creates the match gear in their slots and gems it. A
    // slot whose item is not the copied one is left alone.
    static void ApplyPlannedGear(Player* bot, PvpLoadout::Snapshot& snapshot);

    // In a skirmish, tells the real players on the bot's team its PvP spec, average item level and the rating its gear
    // was scaled to (Playerbots.PvpLoadoutAnnounce).
    static void Announce(PlayerbotAI* botAI, uint32 rating);

    // Destroys the match items and recreates the snapshot's PvE items in their slots, re-equipping any original the bot
    // still has; safe to repeat.
    static void RestoreGear(Player* bot, PvpLoadout::Snapshot& snapshot);

    // Restores and forgets the bot's stored loadout, if it holds one. Returns whether it did.
    static bool Restore(PlayerbotAI* botAI);
};

#endif
