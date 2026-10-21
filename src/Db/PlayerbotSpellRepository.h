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

// One SkillLineAbility row of a profession, in the order of the bits in a profession link.
struct TradeSkillLinkEntry
{
    uint32_t spellId;
    bool bindOnPickup;  // the recipe creates a bind-on-pickup item
};

struct TradeSkillLinkData
{
    uint32_t tradeSpellId;  // profession spell the link opens
    std::vector<TradeSkillLinkEntry> entries;
};

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
    TradeSkillLinkData const* GetTradeSkillLinkData(uint32_t skillId) const;

private:
    PlayerbotSpellRepository() = default;
    ~PlayerbotSpellRepository() = default;

    void InitTradeSkillLinks(std::unordered_map<uint32_t, std::vector<SkillLineAbilityEntry const*>>& abilitiesBySkill);

    PlayerbotSpellRepository(PlayerbotSpellRepository const&) = delete;
    PlayerbotSpellRepository& operator=(PlayerbotSpellRepository const&) = delete;

    PlayerbotSpellRepository(PlayerbotSpellRepository&&) = delete;
    PlayerbotSpellRepository& operator=(PlayerbotSpellRepository&&) = delete;

    std::map<uint32_t, SkillLineAbilityEntry const*> skillSpells;
    std::set<uint32_t> vendorItems;
    std::map<uint32_t, std::vector<uint32_t>> _openingSpells;
    std::unordered_map<uint32_t, uint32_t> _ridingSkillByMount;
    std::unordered_map<uint32_t, TradeSkillLinkData> _tradeSkillLinks;
};

#endif
