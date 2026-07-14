/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#ifndef PLAYERBOTS_PALADINBLESSINGPLAN_H
#define PLAYERBOTS_PALADINBLESSINGPLAN_H

#include "ObjectGuid.h"
#include "PaladinBlessingTypes.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

// The pure (game-state-free) blessing decision logic: who casts which class-wide greater, and
// which single-target exception a paladin owes a member.  Everything here is a function of the
// paladin roster + priority-weighted demand only -> no Player*/aura access.
namespace ai::blessing
{
    // Reagent message the cast action should send for the chosen blessing.
    enum class ReagentNoticeKind { None, Clear, Announce };

    // One bot's next blessing cast for this tick (spellId 0 = nothing to cast).
    struct PendingBlessing
    {
        uint32               spellId  = 0;
        ObjectGuid           target;
        BaseBlessingCategory category = BASE_NONE;
        ReagentNoticeKind    notice   = ReagentNoticeKind::None;
    };

    struct PaladinFlags
    {
        ObjectGuid guid;
        bool  isHuman      = false;
        bool  hasKings     = false;
        bool  hasSanctuary = false;
        int32 mightStrength  = 0;
        int32 wisdomStrength = 0;
    };

    struct BlessingConstraints
    {
        std::vector<PaladinFlags> paladins;
        std::vector<std::pair<ObjectGuid, BaseBlessingCategory>> forced;
    };

    // Priority weight of `cat` for `role`: rank 0 -> highest … last rank -> 1; 0 if not wanted.
    inline int RoleWeight(RoleProfile role, BaseBlessingCategory cat)
    {
        auto const& prio = BASE_BLESSING_PRIORITIES[role].priorities;
        int const n = static_cast<int>(BLESSING_PRIORITY_SLOTS);
        for (int i = 0; i < n; ++i)
            if (prio[i] == cat)
                return n - i;
        return 0;
    }

    inline bool PaladinCanCast(PaladinFlags const& p, BaseBlessingCategory cat)
    {
        switch (cat)
        {
            case BASE_MIGHT:     return p.mightStrength > 0;
            case BASE_WISDOM:    return p.wisdomStrength > 0;
            case BASE_KINGS:     return p.hasKings;
            case BASE_SANCTUARY: return p.hasSanctuary;
            default:             return false;
        }
    }

    inline bool SomeBotCanCast(std::vector<PaladinFlags> const& bots, BaseBlessingCategory cat)
    {
        for (auto const& b : bots)
            if (PaladinCanCast(b, cat))
                return true;
        return false;
    }

    inline int32 CategoryStrength(PaladinFlags const& p, BaseBlessingCategory cat)
    {
        switch (cat)
        {
            case BASE_MIGHT:  return p.mightStrength;
            case BASE_WISDOM: return p.wisdomStrength;
            default:          return 0;
        }
    }

    inline bool HasStrengthGap(std::vector<PaladinFlags> const& bots, BaseBlessingCategory cat)
    {
        if (cat != BASE_MIGHT && cat != BASE_WISDOM)
            return false;
        bool seen = false;
        int32 first = 0;
        for (auto const& b : bots)
        {
            int32 const s = CategoryStrength(b, cat);
            if (s <= 0)
                continue;                       // can't cast -> not a provider
            if (!seen)
            {
                seen  = true;
                first = s;
            }
            else if (s != first)
                return true;                    // two providers, differing strength
        }
        return false;
    }

    // Assigns each bot paladin (sorted by GUID) a DISTINCT class-wide greater category for one
    // class, by priority-weighted demand.  Forced strategy casts pin a bot to its category;
    // Might/Wisdom go to the strongest free caster, Sanctuary to a talented one; the rest take the
    // highest-demand unclaimed category they can cast.  Deterministic: every paladin computes the
    // same result.
    inline std::vector<BaseBlessingCategory> AssignClassGreaters(
        std::vector<PaladinFlags> const& bots,
        std::array<int, BLESSING_CATEGORY_SLOTS> const& demand,
        std::array<bool, BLESSING_CATEGORY_SLOTS> const& honoredHuman,
        std::vector<std::pair<ObjectGuid, BaseBlessingCategory>> const& forced,
        bool suppressSanctuary)
    {
        std::vector<BaseBlessingCategory> g(bots.size(), BASE_NONE);
        std::array<bool, BLESSING_CATEGORY_SLOTS> claimed{};

        // Honored human claims cover their category class-wide; bots don't greater it.
        for (BaseBlessingCategory c : { BASE_MIGHT, BASE_WISDOM, BASE_KINGS, BASE_SANCTUARY })
            if (honoredHuman[c])
                claimed[c] = true;

        // 1. Forced strategy casts: a forced bot greaters its forced category.
        for (auto const& f : forced)
        {
            if (suppressSanctuary && f.second == BASE_SANCTUARY)
                continue;
            for (size_t i = 0; i < bots.size(); ++i)
                if (bots[i].guid == f.first && g[i] == BASE_NONE && PaladinCanCast(bots[i], f.second))
                {
                    g[i] = f.second;
                    claimed[f.second] = true;
                }
        }

        // 2. Choose which categories the free (non-forced) bots will greater: the highest-demand
        //    castable, unclaimed categories, capped to the number of free bots.
        size_t freeCount = 0;
        for (size_t i = 0; i < bots.size(); ++i)
            if (g[i] == BASE_NONE)
                ++freeCount;

        std::vector<BaseBlessingCategory> ranked;
        for (BaseBlessingCategory c : { BASE_MIGHT, BASE_WISDOM, BASE_KINGS, BASE_SANCTUARY })
            if (demand[c] > 0 && !claimed[c] && SomeBotCanCast(bots, c))
                ranked.push_back(c);

        auto const byDemand = [&](BaseBlessingCategory a, BaseBlessingCategory b)
        {
            if (demand[a] != demand[b])
                return demand[a] > demand[b];
            return static_cast<uint8>(a) < static_cast<uint8>(b);  // tie: lower id, deterministic
        };

        if (ranked.size() > freeCount)
        {
            // Undersupplied: not every wanted category can be greatered, so some drop to the
            // single-target exception path.
            std::array<bool, BLESSING_CATEGORY_SLOTS> strengthPreferred{};
            for (BaseBlessingCategory c : ranked)
                strengthPreferred[c] = HasStrengthGap(bots, c);
            std::sort(ranked.begin(), ranked.end(),
                [&](BaseBlessingCategory a, BaseBlessingCategory b)
                {
                    if (strengthPreferred[a] != strengthPreferred[b])
                        return strengthPreferred[a];
                    return byDemand(a, b);
                });
            ranked.resize(freeCount);                  // top-N cap: as many categories as free paladins
        }
        else
        {
            std::sort(ranked.begin(), ranked.end(), byDemand);
        }

        std::array<bool, BLESSING_CATEGORY_SLOTS> chosen{};
        for (BaseBlessingCategory c : ranked)
            chosen[c] = true;

        // 3. Among the CHOSEN categories, pin specialists.  Sanctuary goes to a capable (talented)
        //    paladin; Might/Wisdom go to the STRONGEST free caster (highest rank × Improved talent),
        //    so a higher-rank paladin without the talent correctly out-buffs a lower-level one who
        //    has it.  Ties resolve to the lowest GUID (bots is GUID-sorted), so all bots agree.
        if (chosen[BASE_SANCTUARY] && !claimed[BASE_SANCTUARY])
            for (size_t i = 0; i < bots.size(); ++i)
                if (g[i] == BASE_NONE && bots[i].hasSanctuary)
                {
                    g[i] = BASE_SANCTUARY;
                    claimed[BASE_SANCTUARY] = true;
                    break;
                }

        auto claimStrongest = [&](BaseBlessingCategory cat, auto strengthOf)
        {
            if (!chosen[cat] || claimed[cat])
                return;
            std::optional<std::size_t> best;
            int32 bestStrength = -1;
            for (std::size_t i = 0; i < bots.size(); ++i)
                if (g[i] == BASE_NONE && PaladinCanCast(bots[i], cat) && strengthOf(bots[i]) > bestStrength)
                {
                    bestStrength = strengthOf(bots[i]);
                    best = i;
                }
            if (best)
            {
                g[*best] = cat;
                claimed[cat] = true;
            }
        };
        claimStrongest(BASE_MIGHT,  [](PaladinFlags const& p) { return p.mightStrength; });
        claimStrongest(BASE_WISDOM, [](PaladinFlags const& p) { return p.wisdomStrength; });

        // 4. Remaining free bots take the remaining chosen categories (demand order, GUID order).
        for (size_t i = 0; i < bots.size(); ++i)
        {
            if (g[i] != BASE_NONE)
                continue;
            for (BaseBlessingCategory c : ranked)
                if (chosen[c] && !claimed[c] && PaladinCanCast(bots[i], c))
                {
                    g[i] = c;
                    claimed[c] = true;
                    break;
                }
        }

        return g;
    }

    // When my class greater is useless to a member, the single-target category I should give it
    // instead. Paladins whose greater is useless to the member, lowest-GUID first,
    // take the uncovered wants in order, so the choice is collision-free across paladins.  Returns
    // BASE_NONE if I am not the designated provider for any uncovered want.
    inline BaseBlessingCategory ResolveExceptionCategory(
        RoleProfile role,
        std::array<bool, BLESSING_CATEGORY_SLOTS> const& memberWanted,
        std::array<bool, BLESSING_CATEGORY_SLOTS> const& coveredByGreaters,
        std::vector<BaseBlessingCategory> const& greaters,
        std::vector<PaladinFlags> const& bots,
        std::size_t selfIdx)
    {
        std::vector<BaseBlessingCategory> uncovered;
        for (BaseBlessingCategory c : BASE_BLESSING_PRIORITIES[role].priorities)
            if (c != BASE_NONE && memberWanted[c] && !coveredByGreaters[c])
                uncovered.push_back(c);

        std::vector<std::size_t> uselessPals;
        for (std::size_t i = 0; i < bots.size(); ++i)
            if (greaters[i] != BASE_NONE && !memberWanted[greaters[i]])
                uselessPals.push_back(i);

        // Match each uncovered want to a DISTINCT useless paladin that can cast it (lowest index
        // first), so a want isn't dropped when the same-index paladin can't cast that category.
        std::vector<bool> taken(uselessPals.size(), false);
        for (BaseBlessingCategory want : uncovered)
            for (std::size_t j = 0; j < uselessPals.size(); ++j)
            {
                if (taken[j] || !PaladinCanCast(bots[uselessPals[j]], want))
                    continue;
                taken[j] = true;
                if (uselessPals[j] == selfIdx)
                    return want;
                break;                          // this want is covered by another paladin
            }

        return BASE_NONE;
    }
}

#endif
