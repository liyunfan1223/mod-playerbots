/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotSpellRepository.h"

#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "QueryResult.h"  // Required due to a poor implementation by AC
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <algorithm>
#include <array>
#include <utility>

static constexpr uint32 SPELL_APPRENTICE_RIDING = 33388;
static constexpr uint32 SPELL_JOURNEYMAN_RIDING = 33391;
static constexpr uint32 SPELL_EXPERT_RIDING = 34090;
static constexpr uint32 SPELL_ARTISAN_RIDING = 34091;
static constexpr uint32 RIDING_RANK_APPRENTICE = 75;
static constexpr uint32 RIDING_RANK_JOURNEYMAN = 150;
static constexpr uint32 RIDING_RANK_EXPERT = 225;
static constexpr uint32 RIDING_RANK_ARTISAN = 300;
// Speed auras store the bonus minus one: 99 is 100%, 279 is 280%.
static constexpr int32 FAST_GROUND_MOUNT_SPEED = 99;
static constexpr int32 FAST_FLYING_MOUNT_SPEED = 279;

// Highest first, so a mount that lists several Riding spells gets the highest rank.
static constexpr std::array<std::pair<uint32, uint32>, 4> RIDING_SPELL_RANKS = {{
    {SPELL_ARTISAN_RIDING, RIDING_RANK_ARTISAN},
    {SPELL_EXPERT_RIDING, RIDING_RANK_EXPERT},
    {SPELL_JOURNEYMAN_RIDING, RIDING_RANK_JOURNEYMAN},
    {SPELL_APPRENTICE_RIDING, RIDING_RANK_APPRENTICE},
}};

//  caches the result set
void PlayerbotSpellRepository::Initialize()
{
    LOG_INFO("playerbots", "Playerbots: ListSpellsAction caches initialized");

    // Mount spells carry no Riding requirement; the item that teaches them does.
    for (auto const& [itemId, itemTemplate] : *sObjectMgr->GetItemTemplateStore())
    {
        if (itemTemplate.RequiredSkill != SKILL_RIDING)
            continue;

        for (_Spell const& itemSpell : itemTemplate.Spells)
        {
            if (itemSpell.SpellId <= 0 || itemSpell.SpellTrigger != ITEM_SPELLTRIGGER_LEARN_SPELL_ID)
                continue;

            auto [itr, inserted] =
                _ridingSkillByMount.try_emplace(uint32(itemSpell.SpellId), itemTemplate.RequiredSkillRank);
            if (!inserted)
                itr->second = std::min(itr->second, itemTemplate.RequiredSkillRank);
        }
    }

    // Fast class mounts are taught by no item; spell_required lists their Riding spell.
    for (auto const& [ridingSpell, rank] : RIDING_SPELL_RANKS)
    {
        SpellsRequiringSpellMapBounds const bounds = sSpellMgr->GetSpellsRequiringSpellBounds(ridingSpell);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
            _ridingSkillByMount.try_emplace(itr->second, rank);
    }

    std::unordered_map<uint32, std::vector<SkillLineAbilityEntry const*>> abilitiesBySkill;
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine)
            continue;

        skillSpells[skillLine->Spell] = skillLine;

        // Only professions have a profession link.
        SkillLineEntry const* skill = sSkillLineStore.LookupEntry(skillLine->SkillLine);
        if (skill && (skill->categoryId == SKILL_CATEGORY_PROFESSION || skill->categoryId == SKILL_CATEGORY_SECONDARY))
            abilitiesBySkill[skillLine->SkillLine].push_back(skillLine);
    }

    InitTradeSkillLinks(abilitiesBySkill);

    for (uint32 spellId = 0; spellId < sSpellMgr->GetSpellInfoStoreSize(); ++spellId)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        for (SpellEffectInfo const& spellEffectInfo : spellInfo->GetEffects())
        {
            if ((spellEffectInfo.Effect == SPELL_EFFECT_OPEN_LOCK || spellEffectInfo.Effect == SPELL_EFFECT_SKINNING) &&
                spellEffectInfo.MiscValue)
                _openingSpells[uint32(spellEffectInfo.MiscValue)].push_back(spellId);
        }
    }

    // Fill the vendorItems cache once from the world database.
    QueryResult results = WorldDatabase.Query("SELECT item FROM npc_vendor WHERE maxcount = 0");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            int32 entry = fields[0].Get<int32>();
            if (entry <= 0)
                continue;

            vendorItems.insert(static_cast<uint32>(entry));
        } while (results->NextRow());
    }

    LOG_DEBUG("playerbots", "ListSpellsAction: initialized caches (skillSpells={}, vendorItems={}).",
              skillSpells.size(), vendorItems.size());
}

// The client numbers a profession's link bits over all of its SkillLineAbility rows, sorted by
// MinSkillLineRank and then by row ID; the link carries no list, so this order must match the client's DBC.
void PlayerbotSpellRepository::InitTradeSkillLinks(
    std::unordered_map<uint32, std::vector<SkillLineAbilityEntry const*>>& abilitiesBySkill)
{
    for (auto& [skillId, abilities] : abilitiesBySkill)
    {
        std::sort(abilities.begin(), abilities.end(),
                  [](SkillLineAbilityEntry const* lhs, SkillLineAbilityEntry const* rhs)
                  {
                      if (lhs->MinSkillLineRank != rhs->MinSkillLineRank)
                          return lhs->MinSkillLineRank < rhs->MinSkillLineRank;
                      return lhs->ID < rhs->ID;
                  });

        TradeSkillLinkData data{0, {}};
        data.entries.reserve(abilities.size());
        for (SkillLineAbilityEntry const* ability : abilities)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ability->Spell);
            if (!data.tradeSpellId && spellInfo && spellInfo->Effects[EFFECT_0].Effect == SPELL_EFFECT_TRADE_SKILL)
                data.tradeSpellId = ability->Spell;

            bool bindOnPickup = false;
            if (spellInfo)
            {
                for (SpellEffectInfo const& effect : spellInfo->GetEffects())
                {
                    if (effect.Effect != SPELL_EFFECT_CREATE_ITEM || !effect.ItemType)
                        continue;

                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(effect.ItemType);
                    if (proto && proto->Bonding == BIND_WHEN_PICKED_UP)
                        bindOnPickup = true;
                }
            }

            data.entries.push_back({ability->Spell, bindOnPickup});
        }

        if (data.tradeSpellId)
            _tradeSkillLinks.emplace(skillId, std::move(data));
    }
}

TradeSkillLinkData const* PlayerbotSpellRepository::GetTradeSkillLinkData(uint32 skillId) const
{
    auto const itr = _tradeSkillLinks.find(skillId);
    return itr != _tradeSkillLinks.end() ? &itr->second : nullptr;
}

SkillLineAbilityEntry const* PlayerbotSpellRepository::GetSkillLine(uint32 spellId) const
{
    auto itr = skillSpells.find(spellId);
    if (itr != skillSpells.end())
        return itr->second;
    return nullptr;
}

bool PlayerbotSpellRepository::IsItemBuyable(uint32 itemId) const
{
    return vendorItems.find(itemId) != vendorItems.end();
}

std::vector<uint32> const& PlayerbotSpellRepository::GetOpeningSpells(uint32 lockType) const
{
    static std::vector<uint32> const empty;
    auto const itr = _openingSpells.find(lockType);
    return itr != _openingSpells.end() ? itr->second : empty;
}

uint32 PlayerbotSpellRepository::GetRequiredRidingSkill(SpellInfo const* mountSpell) const
{
    auto const itr = _ridingSkillByMount.find(mountSpell->Id);
    if (itr != _ridingSkillByMount.end())
        return itr->second;

    // No item or spell_required row (Acherus Deathcharger, slow class and quest mounts): rate by speed.
    int32 speed = 0;
    bool flying = false;
    for (SpellEffectInfo const& effect : mountSpell->GetEffects())
    {
        switch (effect.ApplyAuraName)
        {
            case SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED:
            case SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS:
                flying = true;
                speed = std::max(speed, effect.BasePoints);
                break;
            case SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED:
            case SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS:
            case SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK:
                speed = std::max(speed, effect.BasePoints);
                break;
            default:
                break;
        }
    }

    if (speed <= 0)
        return 0;

    if (flying)
        return speed >= FAST_FLYING_MOUNT_SPEED ? RIDING_RANK_ARTISAN : RIDING_RANK_EXPERT;

    return speed >= FAST_GROUND_MOUNT_SPEED ? RIDING_RANK_JOURNEYMAN : RIDING_RANK_APPRENTICE;
}
