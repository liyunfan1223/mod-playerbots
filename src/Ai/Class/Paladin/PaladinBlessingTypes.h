/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#ifndef PLAYERBOTS_PALADINBLESSINGTYPES_H
#define PLAYERBOTS_PALADINBLESSINGTYPES_H

#include "Define.h"
#include <string>

class Group;
class Player;

namespace ai::blessing
{
    static constexpr uint32 SPELL_IMPROVED_MIGHT_R1  = 20042;
    static constexpr uint32 SPELL_IMPROVED_MIGHT_R2  = 20045;
    static constexpr uint32 SPELL_IMPROVED_WISDOM_R1 = 20244;
    static constexpr uint32 SPELL_IMPROVED_WISDOM_R2 = 20245;

    // Blessing first-rank spell ids
    static constexpr uint32 SPELL_BLESSING_OF_MIGHT             = 19740;
    static constexpr uint32 SPELL_GREATER_BLESSING_OF_MIGHT     = 25782;
    static constexpr uint32 SPELL_BLESSING_OF_WISDOM            = 19742;
    static constexpr uint32 SPELL_GREATER_BLESSING_OF_WISDOM    = 25894;
    static constexpr uint32 SPELL_BLESSING_OF_KINGS             = 20217;
    static constexpr uint32 SPELL_GREATER_BLESSING_OF_KINGS     = 25898;
    static constexpr uint32 SPELL_BLESSING_OF_SANCTUARY         = 20911;
    static constexpr uint32 SPELL_GREATER_BLESSING_OF_SANCTUARY = 25899;

    enum RoleProfile : uint8
    {
        ROLE_CASTER          = 0,
        ROLE_PHYSICAL_DPS    = 1,
        ROLE_HYBRID_DPS      = 2,
        ROLE_DRUID_TANK      = 3,
        ROLE_WARRIOR_DK_TANK = 4,
        ROLE_PALADIN_TANK    = 5,
        ROLE_PROFILE_COUNT   = 6
    };

    enum BlessingType : uint8
    {
        BLESSING_NONE              = 0,
        BLESSING_MIGHT_SINGLE      = 1,
        BLESSING_MIGHT_GREATER     = 2,
        BLESSING_WISDOM_SINGLE     = 3,
        BLESSING_WISDOM_GREATER    = 4,
        BLESSING_KINGS_SINGLE      = 5,
        BLESSING_KINGS_GREATER     = 6,
        BLESSING_SANCTUARY_SINGLE  = 7,
        BLESSING_SANCTUARY_GREATER = 8
    };

    enum BaseBlessingCategory : uint8
    {
        BASE_NONE      = 0,
        BASE_MIGHT     = 1,
        BASE_WISDOM    = 2,
        BASE_KINGS     = 3,
        BASE_SANCTUARY = 4
    };

    static constexpr size_t BLESSING_CATEGORY_SLOTS = BASE_SANCTUARY + 1;   // = 5
    static constexpr size_t BLESSING_PRIORITY_SLOTS = 4;                    // categories per role
    static_assert(BASE_MIGHT == 1 && BASE_WISDOM == 2 && BASE_KINGS == 3 && BASE_SANCTUARY == 4,
                  "blessing category enum values double as array indices");

    struct BaseBlessingPriorityEntry
    {
        BaseBlessingCategory priorities[BLESSING_PRIORITY_SLOTS];
    };

    inline constexpr BaseBlessingPriorityEntry BASE_BLESSING_PRIORITIES[] =
    {
        // All casters
        {{ BASE_KINGS,     BASE_WISDOM, BASE_SANCTUARY, BASE_MIGHT     }},
        // Physical DPS (no mana)
        {{ BASE_MIGHT,     BASE_KINGS,  BASE_SANCTUARY, BASE_NONE      }},
        // Hybrid DPS
        {{ BASE_MIGHT,     BASE_KINGS,  BASE_WISDOM,    BASE_SANCTUARY }},
        // Druid tanks
        {{ BASE_KINGS,     BASE_MIGHT,  BASE_SANCTUARY, BASE_WISDOM,   }},
        // Warrior and DK tanks
        {{ BASE_KINGS,     BASE_MIGHT,  BASE_SANCTUARY, BASE_NONE      }},
        // Paladin tanks
        {{ BASE_SANCTUARY, BASE_MIGHT,  BASE_WISDOM,    BASE_KINGS     }},
    };
    static_assert(std::size(BASE_BLESSING_PRIORITIES) == ROLE_PROFILE_COUNT,
                  "exactly one BASE_BLESSING_PRIORITIES row per RoleProfile");

    // Blessing vocabulary shared by the planner and helpers.
    inline constexpr BaseBlessingCategory BaseBlessingOf(BlessingType type)
    {
        switch (type)
        {
            case BLESSING_MIGHT_SINGLE:
            case BLESSING_MIGHT_GREATER:      return BASE_MIGHT;
            case BLESSING_WISDOM_SINGLE:
            case BLESSING_WISDOM_GREATER:     return BASE_WISDOM;
            case BLESSING_KINGS_SINGLE:
            case BLESSING_KINGS_GREATER:      return BASE_KINGS;
            case BLESSING_SANCTUARY_SINGLE:
            case BLESSING_SANCTUARY_GREATER:  return BASE_SANCTUARY;
            default:                          return BASE_NONE;
        }
    }

    inline constexpr BlessingType ToSingleVariant(BaseBlessingCategory category)
    {
        switch (category)
        {
            case BASE_MIGHT:     return BLESSING_MIGHT_SINGLE;
            case BASE_WISDOM:    return BLESSING_WISDOM_SINGLE;
            case BASE_KINGS:     return BLESSING_KINGS_SINGLE;
            case BASE_SANCTUARY: return BLESSING_SANCTUARY_SINGLE;
            default:             return BLESSING_NONE;
        }
    }

    inline constexpr BlessingType ToSingleVariant(BlessingType type)
    {
        return ToSingleVariant(BaseBlessingOf(type));
    }

    inline constexpr BlessingType ToGreaterVariant(BaseBlessingCategory category)
    {
        switch (category)
        {
            case BASE_MIGHT:     return BLESSING_MIGHT_GREATER;
            case BASE_WISDOM:    return BLESSING_WISDOM_GREATER;
            case BASE_KINGS:     return BLESSING_KINGS_GREATER;
            case BASE_SANCTUARY: return BLESSING_SANCTUARY_GREATER;
            default:             return BLESSING_NONE;
        }
    }

    inline constexpr BlessingType ToGreaterVariant(BlessingType type)
    {
        return ToGreaterVariant(BaseBlessingOf(type));
    }

    inline std::string BlessingSpellName(BlessingType type)
    {
        switch (type)
        {
            case BLESSING_MIGHT_SINGLE:      return "blessing of might";
            case BLESSING_MIGHT_GREATER:     return "greater blessing of might";
            case BLESSING_WISDOM_SINGLE:     return "blessing of wisdom";
            case BLESSING_WISDOM_GREATER:    return "greater blessing of wisdom";
            case BLESSING_KINGS_SINGLE:      return "blessing of kings";
            case BLESSING_KINGS_GREATER:     return "greater blessing of kings";
            case BLESSING_SANCTUARY_SINGLE:  return "blessing of sanctuary";
            case BLESSING_SANCTUARY_GREATER: return "greater blessing of sanctuary";
            default:                         return "";
        }
    }

    RoleProfile ResolveRoleProfile(Player* player);
    bool IsEligibleGroupForAutoBlessings(Group const* group);
}

#endif
