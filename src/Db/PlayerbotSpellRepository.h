/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTSPELLREPOSITORY_H
#define PLAYERBOTS_PLAYERBOTSPELLREPOSITORY_H

#include "DBCStructure.h"

#include <cstdint>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

class SpellInfo;

class PlayerbotSpellRepository
{
public:
    static PlayerbotSpellRepository& Instance()
    {
        static PlayerbotSpellRepository instance;

        return instance;
    }

    void Initialize();

    SkillLineAbilityEntry const* GetSkillLine(uint32_t spellId) const;
    bool IsItemBuyable(uint32_t itemId) const;
    std::vector<uint32_t> const& GetOpeningSpells(uint32_t lockType) const;
    uint32_t GetRequiredRidingSkill(SpellInfo const* mountSpell) const;

private:
    PlayerbotSpellRepository() = default;
    ~PlayerbotSpellRepository() = default;

    PlayerbotSpellRepository(PlayerbotSpellRepository const&) = delete;
    PlayerbotSpellRepository& operator=(PlayerbotSpellRepository const&) = delete;

    PlayerbotSpellRepository(PlayerbotSpellRepository&&) = delete;
    PlayerbotSpellRepository& operator=(PlayerbotSpellRepository&&) = delete;

    std::map<uint32_t, SkillLineAbilityEntry const*> skillSpells;
    std::set<uint32_t> vendorItems;
    std::map<uint32_t, std::vector<uint32_t>> _openingSpells;
    std::unordered_map<uint32_t, uint32_t> _ridingSkillByMount;
};

#endif
