/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "EquipGlyphsAction.h"

#include "Playerbots.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "DBCStores.h"
#include "AiObjectContext.h"
#include "Log.h"

#include <unordered_map>
#include <sstream>
#include <unordered_set>

namespace
{
    // itemId -> GlyphInfo
    std::unordered_map<uint32, EquipGlyphsAction::GlyphInfo> s_GlyphCache;
}

void EquipGlyphsAction::BuildGlyphCache()
{
    if (!s_GlyphCache.empty())
        return;

    ItemTemplateContainer const* store = sObjectMgr->GetItemTemplateStore();

    for (auto const& kv : *store)
    {
        uint32 itemId = kv.first;
        ItemTemplate const* proto = &kv.second;
        if (!proto || proto->Class != ITEM_CLASS_GLYPH)
            continue;

        // inspect item spell
        for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId) continue;

            SpellInfo const* si = sSpellMgr->GetSpellInfo(spellId);
            if (!si)         continue;

            for (uint8 eff = 0; eff <= EFFECT_2; ++eff)
            {
                if (si->Effects[eff].Effect != SPELL_EFFECT_APPLY_GLYPH)
                    continue;

                uint32 glyphId = si->Effects[eff].MiscValue;
                if (!glyphId) continue;

                if (auto const* gp = sGlyphPropertiesStore.LookupEntry(glyphId))
                    s_GlyphCache[itemId] = {gp, proto};
            }
        }
    }
}

EquipGlyphsAction::GlyphInfo const* EquipGlyphsAction::GetGlyphInfo(uint32 itemId)
{
    BuildGlyphCache();
    auto it = s_GlyphCache.find(itemId);
    return (it == s_GlyphCache.end()) ? nullptr : &it->second;
}

/// -----------------------------------------------------------------
///  Validation and collect
/// -----------------------------------------------------------------
bool EquipGlyphsAction::CollectGlyphs(std::vector<uint32> const& itemIds,
                                      std::vector<GlyphInfo const*>& out) const
{
    std::unordered_set<uint32> seen;

    for (uint32 itemId : itemIds)
    {
        if (!seen.insert(itemId).second)
            return false;                                 // double

        auto const* info = GetGlyphInfo(itemId);
        if (!info)                                        // no good glyph
            return false;

        // check class by AllowableClass
        if ((info->proto->AllowableClass & bot->getClassMask()) == 0)
            return false;

        out.push_back(info);
    }
    return out.size() <= 6 && !out.empty();
}

/// -----------------------------------------------------------------
///  Action
/// -----------------------------------------------------------------
bool EquipGlyphsAction::Execute(Event event)
{
    // 1) parse IDs
    std::vector<uint32> itemIds;
    std::istringstream iss(event.getParam());
    for (uint32 id; iss >> id; ) itemIds.push_back(id);

    std::vector<GlyphInfo const*> glyphs;
    if (!CollectGlyphs(itemIds, glyphs))
    {
        botAI->TellMaster("Usage: glyph equip <6 glyph item IDs> (3 major, 3 minor).");
        return false;
    }

    // 2) prepare a empty slots table ?
    bool used[6] = {false,false,false,false,false,false};

    // 3) for each glyph, find the first available and compatible socket
    for (auto const* g : glyphs)
    {
        bool placed = false;

        for (uint8 i = 0; i < MAX_GLYPH_SLOT_INDEX; ++i)
        {
            if (used[i]) continue;

            uint32 slotId   = bot->GetGlyphSlot(i);
            auto const* gs  = sGlyphSlotStore.LookupEntry(slotId);
            if (!gs || gs->TypeFlags != g->prop->TypeFlags)
                continue;                                   // major/minor don't match

            // Remove aura if exist
            uint32 cur = bot->GetGlyph(i);
            if (cur)
                if (auto* old = sGlyphPropertiesStore.LookupEntry(cur))
                    bot->RemoveAurasDueToSpell(old->SpellId);

            // Apply new one
            bot->CastSpell(bot, g->prop->SpellId, true);
            bot->SetGlyph(i, g->prop->Id, true);

            used[i] = true;
            placed  = true;
            break;
        }

        if (!placed)
        {
            botAI->TellMaster("Not enought empty sockets for all glyphs.");
            return false;
        }
    }

    botAI->TellMaster("Glyphs updated.");

    // Flag for custom glyphs
    botAI->GetAiObjectContext()->GetValue<bool>("custom_glyphs")->Set(true);
    LOG_INFO("playerbots", "Custom Glyph Flag set to ON");

    return true;
}
