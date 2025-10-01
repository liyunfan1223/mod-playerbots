/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellGlyphsAction.h"

#include "Event.h"
#include "Playerbots.h"

#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "World.h"

#include <unordered_map>
#include <sstream>

namespace
{
    // -----------------------------------------------------------------
    // Cache : GlyphID (MiscValue) -> ItemTemplate*
    // -----------------------------------------------------------------
    std::unordered_map<uint32, ItemTemplate const*> s_GlyphItemCache;

    void BuildGlyphItemCache()
    {
        if (!s_GlyphItemCache.empty())
            return;

        ItemTemplateContainer const* store = sObjectMgr->GetItemTemplateStore();

        for (auto const& kv : *store)                       // C++17 : range-for sur map
        {
            ItemTemplate const* proto = &kv.second;

            if (!proto || proto->Class != ITEM_CLASS_GLYPH)
                continue;

            for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                uint32 spellId = proto->Spells[i].SpellId;
                if (!spellId)
                    continue;

                SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellId);
                if (!spell)
                    continue;

                for (uint32 eff = 0; eff <= EFFECT_2; ++eff)
                {
                    if (spell->Effects[eff].Effect != SPELL_EFFECT_APPLY_GLYPH)
                        continue;

                    uint32 glyphId = spell->Effects[eff].MiscValue;
                    if (glyphId)
                        s_GlyphItemCache[glyphId] = proto;
                }
            }
        }
    }
} // namespace

// -----------------------------------------------------------------
// Action
// -----------------------------------------------------------------
bool TellGlyphsAction::Execute(Event event)
{
    //-----------------------------------------------------------------
    // 1. who sended the wisp ?  (source of event)
    //-----------------------------------------------------------------
    Player* sender = event.getOwner();          // API Event
    if (!sender)
        return false;

    //-----------------------------------------------------------------
    // 2. Generate glyphId cache -> item
    //-----------------------------------------------------------------
    BuildGlyphItemCache();

    //-----------------------------------------------------------------
    // 3. Look at the 6 glyphs sockets
    //-----------------------------------------------------------------
    std::ostringstream list;
    bool first = true;

    for (uint8 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)
    {
        uint32 glyphId = bot->GetGlyph(slot);
        if (!glyphId)
            continue;

        auto it = s_GlyphItemCache.find(glyphId);
        if (it == s_GlyphItemCache.end())
            continue;                                // No glyph found (rare)

        if (!first)
            list << ", ";

        // chat->FormatItem
        list << chat->FormatItem(it->second);
        first = false;
    }

    //-----------------------------------------------------------------
    // 4. Send chat messages
    //-----------------------------------------------------------------
    if (first)                                       // no glyphs
        botAI->TellMaster("No glyphs equipped");
    else
        botAI->TellMaster(std::string("Glyphs: ") + list.str());

    return true;
}
