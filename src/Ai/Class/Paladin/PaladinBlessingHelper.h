/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#ifndef PLAYERBOTS_PALADINBLESSINGHELPER_H
#define PLAYERBOTS_PALADINBLESSINGHELPER_H

#include "ObjectAccessor.h"
#include "PaladinBlessingRegistry.h"
#include "PaladinBlessingTypes.h"
#include "Playerbots.h"
#include "SpellAuraEffects.h"
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <vector>

// Shared blessing utilities: blessing-category mapping, aura presence/strength
// comparison (Improved Might/Wisdom aware), and role classification.
namespace ai::blessing
{
    // Discipline priest Renewed Hope talent (ranks 1/2); its 3% raid damage
    // reduction doesn't stack with Blessing of Sanctuary.
    static constexpr uint32 SPELL_RENEWED_HOPE_R1 = 57470;
    static constexpr uint32 SPELL_RENEWED_HOPE_R2 = 57472;

    inline bool IsHumanPaladin(ObjectGuid guid)
    {
        Player* p = ObjectAccessor::FindPlayer(guid);
        return p && p->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(p) == nullptr;
    }

    // Maps a blessing category to the AuraType that carries its primary stat effect.
    inline AuraType statAuraTypeFor(BaseBlessingCategory cat)
    {
        switch (cat)
        {
            case BASE_MIGHT:     return SPELL_AURA_MOD_ATTACK_POWER;
            case BASE_WISDOM:    return SPELL_AURA_MOD_POWER_REGEN;
            case BASE_KINGS:     return SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE;
            case BASE_SANCTUARY: return SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
            default:             return SPELL_AURA_NONE;
        }
    }

    inline int32 GetAuraStrength(Aura const* aura, AuraType auraType)
    {
        if (!aura)
            return 0;

        int32 amount = 0;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            AuraEffect* auraEffect = aura->GetEffect(effect);
            if (!auraEffect || auraEffect->GetAuraType() != auraType)
                continue;

            amount = std::max(amount, auraEffect->GetAmount());
        }

        return amount;
    }

    inline int32 GetBlessingCastStrength(Player* caster, BlessingType type, uint32 spellId)
    {
        if (!caster || !spellId)
            return 0;

        BaseBlessingCategory category = BaseBlessingOf(type);
        if (category != BASE_MIGHT && category != BASE_WISDOM)
            return 0;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return 0;

        AuraType auraType =
            category == BASE_MIGHT ? SPELL_AURA_MOD_ATTACK_POWER : SPELL_AURA_MOD_POWER_REGEN;
        int32 amount = 0;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            if (spellInfo->Effects[effect].ApplyAuraName != auraType)
                continue;

            amount = std::max(amount, spellInfo->Effects[effect].BasePoints + 1);
        }

        if (amount <= 0)
            return 0;

        switch (category)
        {
            case BASE_MIGHT:
                if (caster->HasAura(SPELL_IMPROVED_MIGHT_R2))
                    return amount * 125 / 100;
                if (caster->HasAura(SPELL_IMPROVED_MIGHT_R1))
                    return amount * 112 / 100;
                break;
            case BASE_WISDOM:
                if (caster->HasAura(SPELL_IMPROVED_WISDOM_R2))
                    return amount * 120 / 100;
                if (caster->HasAura(SPELL_IMPROVED_WISDOM_R1))
                    return amount * 110 / 100;
                break;
            default:
                break;
        }

        return amount;
    }

    // Strength of the strongest Blessing of Might / Wisdom `p` can currently cast (highest known
    // rank × Improved talent).  Lets the assignment pick the strongest provider rather than just a
    // talented one — a higher-rank paladin without the talent can out-buff a lower-level one who
    // has it.  Returns 0 for Kings/Sanctuary (no rank/strength variance) or if untrained.
    inline int32 HighestKnownBlessingStrength(Player* p, BaseBlessingCategory category)
    {
        if (category != BASE_MIGHT && category != BASE_WISDOM)
            return 0;
        std::optional<uint32> const id = HighestKnownBlessingRank(p, category, false);
        if (!id)
            return 0;
        BlessingType const type = category == BASE_MIGHT ? BLESSING_MIGHT_SINGLE : BLESSING_WISDOM_SINGLE;
        return GetBlessingCastStrength(p, type, *id);
    }

    // -------------------------------------------------------------------------
    // Single-pass aura scanner
    // -------------------------------------------------------------------------

    // A blessing found on a member during the single aura-walk pass.
    struct MemberBlessing
    {
        BaseBlessingCategory category;
        ObjectGuid           caster;
        bool                 isGreater;             // true = class-wide greater variant; false = single-target
        bool                 isHumanPaladinCaster;  // resolved during the scan so later checks stay map/vector reads
        int32                duration;              // remaining ms (-1 = permanent);
        int32                strength;
    };

    // Walk member's applied-aura list ONCE and return every blessing-category aura with its
    // caster, greater/single flag, remaining duration, and (Might/Wisdom) stat strength.
    inline std::vector<MemberBlessing> scanBlessingAuras(
        Unit* member, std::unordered_map<ObjectGuid, bool>& humanCasters)
    {
        std::vector<MemberBlessing> result;
        if (!member)
            return result;

        for (auto const& pair : member->GetAppliedAuras())
        {
            AuraApplication const* app = pair.second;
            if (!app)
                continue;

            Aura* aura = app->GetBase();
            if (!aura)
                continue;

            BaseBlessingCategory cat = CategoryOfSpell(aura->GetId());
            if (cat == BASE_NONE)
                continue;

            int32 const strength = (cat == BASE_MIGHT || cat == BASE_WISDOM)
                ? GetAuraStrength(aura, statAuraTypeFor(cat)) : 0;

            ObjectGuid const caster = aura->GetCasterGUID();
            auto cached = humanCasters.find(caster);
            bool const isHuman = cached != humanCasters.end()
                ? cached->second
                : (humanCasters[caster] = IsHumanPaladin(caster));

            result.push_back({ cat, caster, IsGreaterBlessingSpell(aura->GetId()),
                               isHuman, aura->GetDuration(), strength });
        }

        return result;
    }

    // -------------------------------------------------------------------------
    // Coverage queries over the scanned MemberBlessings
    // -------------------------------------------------------------------------

    using CoverageMap = std::unordered_map<ObjectGuid, std::vector<MemberBlessing>>;

    // Does `botGuid` already provide `cat` to `member`, fresh (not near expiry)?
    inline bool IsCoveredByMe(ObjectGuid botGuid, Unit* member, BaseBlessingCategory cat,
                              CoverageMap const& coverage)
    {
        auto it = coverage.find(member->GetGUID());
        if (it == coverage.end())
            return false;

        int64 const refreshMs =
            static_cast<int64>(sPlayerbotAIConfig.BlessingRefreshThresholdSeconds) * IN_MILLISECONDS;
        for (auto const& mb : it->second)
            if (mb.category == cat && mb.caster == botGuid)
                return mb.duration < 0 || mb.duration > refreshMs;

        return false;
    }

    // Does `member` carry a `cat` blessing from a HUMAN paladin?
    inline bool HasHumanBlessing(Unit* member, BaseBlessingCategory cat, CoverageMap const& coverage)
    {
        auto it = coverage.find(member->GetGUID());
        if (it == coverage.end())
            return false;

        for (auto const& mb : it->second)
            if (mb.category == cat && mb.isHumanPaladinCaster)
                return true;

        return false;
    }

    inline bool HasStrongerBlessing(Unit* member, BaseBlessingCategory cat, int32 myStrength,
                                    CoverageMap const& coverage)
    {
        auto it = coverage.find(member->GetGUID());
        if (it == coverage.end())
            return false;

        for (auto const& mb : it->second)
            if (mb.category == cat && mb.strength > myStrength)
                return true;

        return false;
    }
}

#endif
