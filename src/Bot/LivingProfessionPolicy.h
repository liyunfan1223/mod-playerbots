/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_PROFESSION_POLICY_H
#define PLAYERBOTS_LIVING_PROFESSION_POLICY_H

#include "LivingWorldPolicy.h"

namespace LivingWorld
{
struct Profession
{
    uint16_t skill;
    uint8_t expansion;
    bool secondary;
};

inline constexpr std::array<Profession, 14> Professions{{{129, 0, true},
                                                         {164, 0, false},
                                                         {165, 0, false},
                                                         {171, 0, false},
                                                         {182, 0, false},
                                                         {185, 0, true},
                                                         {186, 0, false},
                                                         {197, 0, false},
                                                         {202, 0, false},
                                                         {333, 0, false},
                                                         {356, 0, true},
                                                         {393, 0, false},
                                                         {755, 1, false},
                                                         {773, 2, false}}};

inline uint8_t Expansion(uint32_t levelCap) { return levelCap <= 60 ? 0 : levelCap <= 70 ? 1 : 2; }
inline uint32_t ProfessionCap(uint32_t levelCap) { return 300 + 75 * Expansion(levelCap); }

// Recipes carry no expansion tag, so use the skill they require: vanilla recipes need under 300
// (300 is the first Outland tier, e.g. Smelt Fel Iron), TBC tops out at Master (375).
// Wrath's 350-375 recipes overlap TBC, but their reagents only exist in Northrend.
inline bool RecipeWithinEra(uint32_t reqSkillRank, uint32_t levelCap)
{
    switch (Expansion(levelCap))
    {
        case 0: return reqSkillRank < 300;
        case 1: return reqSkillRank <= 375;
        default: return true;
    }
}

inline Profession const* FindProfession(uint32_t skill, uint32_t levelCap)
{
    for (auto const& profession : Professions)
        if (profession.skill == skill && profession.expansion <= Expansion(levelCap))
            return &profession;
    return nullptr;
}

inline bool CanAffordTraining(uint32_t money, uint32_t price, uint32_t level)
{
    if (!price)
        return true;
    // Keep repair/travel money, and spend at most one fifth of the current purse on a lesson.
    uint32_t const reserve = std::max<uint32_t>(100, level * 100);
    return price <= money / 5 && money - price >= reserve;
}

inline bool WantsProfession(uint32_t guid, uint32_t skill, uint32_t levelCap, std::array<bool, 14> const& known)
{
    auto const* candidate = FindProfession(skill, levelCap);
    if (!candidate)
        return false;
    if (candidate->secondary)
        return true;
    uint32_t existing = 0;
    uint32_t preferred = 0;
    uint32_t const score = Mix(guid ^ skill ^ 0x50524F46U);
    for (size_t i = 0; i < Professions.size(); ++i)
    {
        auto const& profession = Professions[i];
        if (profession.skill == skill && known[i])
            return true;
        if (profession.secondary)
            continue;
        if (known[i])
            ++existing;
        else if (profession.expansion <= Expansion(levelCap))
        {
            uint32_t const other = Mix(guid ^ profession.skill ^ 0x50524F46U);
            if (other < score || (other == score && profession.skill < skill))
                ++preferred;
        }
    }
    return existing < 2 && preferred < 2 - existing;
}
}  // namespace LivingWorld

#endif
