/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#ifndef PLAYERBOTS_PALADINBLESSINGREGISTRY_H
#define PLAYERBOTS_PALADINBLESSINGREGISTRY_H

#include "PaladinBlessingTypes.h"
#include "Player.h"
#include "SpellMgr.h"
#include <array>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

// Blessing spell-id registry: every rank of every blessing keyed by id, plus per-(category,
// variant) ascending rank chains, built once.  Lets callers map an aura's spell id to its
// blessing category / variant, and resolve a bot's highest known rank id straight from the
// chain -- no name lookup, no full spell-map scan.
namespace ai::blessing
{
    struct BlessingSpellInfo
    {
        BaseBlessingCategory category;
        bool                 isGreater;
    };

    static constexpr std::size_t BLESSING_CHAIN_SLOTS = BLESSING_CATEGORY_SLOTS * 2;

    inline std::size_t BlessingChainIndex(BaseBlessingCategory category, bool greater)
    {
        return static_cast<std::size_t>(category) * 2 + (greater ? 1 : 0);
    }

    struct BlessingRegistry
    {
        std::unordered_map<uint32, BlessingSpellInfo> byId;
        // slots 0/1 (BASE_NONE) are intentionally unused
        std::array<std::vector<uint32>, BLESSING_CHAIN_SLOTS> chains;
    };

    inline BlessingRegistry const& Blessings()
    {
        static BlessingRegistry const reg = []
        {
            BlessingRegistry r;
            auto addChain = [&](uint32 firstRankId, BaseBlessingCategory cat, bool greater)
            {
                for (SpellInfo const* info = firstRankId ? sSpellMgr->GetSpellInfo(firstRankId) : nullptr;
                     info; info = info->GetNextRankSpell())
                {
                    r.byId[info->Id] = { cat, greater };
                    r.chains[BlessingChainIndex(cat, greater)].push_back(info->Id);  // GetNextRankSpell: ascending
                }
            };
            addChain(SPELL_BLESSING_OF_MIGHT,             BASE_MIGHT,     false);
            addChain(SPELL_GREATER_BLESSING_OF_MIGHT,     BASE_MIGHT,     true);
            addChain(SPELL_BLESSING_OF_WISDOM,            BASE_WISDOM,    false);
            addChain(SPELL_GREATER_BLESSING_OF_WISDOM,    BASE_WISDOM,    true);
            addChain(SPELL_BLESSING_OF_KINGS,             BASE_KINGS,     false);
            addChain(SPELL_GREATER_BLESSING_OF_KINGS,     BASE_KINGS,     true);
            addChain(SPELL_BLESSING_OF_SANCTUARY,         BASE_SANCTUARY, false);
            addChain(SPELL_GREATER_BLESSING_OF_SANCTUARY, BASE_SANCTUARY, true);
            return r;
        }();
        return reg;
    }

    // Category of any blessing spell rank, or BASE_NONE for an unrecognised spell.
    inline BaseBlessingCategory CategoryOfSpell(uint32 spellId)
    {
        auto const& m = Blessings().byId;
        auto it = m.find(spellId);
        return it == m.end() ? BASE_NONE : it->second.category;
    }

    // True if the spell id is a GREATER (class-wide) blessing variant.
    inline bool IsGreaterBlessingSpell(uint32 spellId)
    {
        auto const& m = Blessings().byId;
        auto it = m.find(spellId);
        return it != m.end() && it->second.isGreater;
    }

    // The bot's HIGHEST known rank id of (category, greater?), or nullopt if it knows none.
    // Resolves the castable id straight from the known rank chain -- no name lookup / full
    // spell-map scan (SpellIdValue), and no mana-save downgrade (we always want the strongest).
    inline std::optional<uint32> HighestKnownBlessingRank(Player* bot, BaseBlessingCategory category, bool greater)
    {
        if (!bot || category == BASE_NONE)
            return std::nullopt;
        std::optional<uint32> best;
        for (uint32 id : Blessings().chains[BlessingChainIndex(category, greater)])  // ascending
            if (bot->HasSpell(id))
                best = id;
        return best;
    }
}

#endif
