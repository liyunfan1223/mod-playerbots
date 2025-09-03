/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_EQUIPGLYPHSACTION_H
#define _PLAYERBOT_EQUIPGLYPHSACTION_H

#include "Action.h"

// 1 = major, 2 = minor dans GlyphProperties.dbc
enum class GlyphKind : uint32 { MAJOR = 1, MINOR = 2 };

class EquipGlyphsAction : public Action
{
public:
    EquipGlyphsAction(PlayerbotAI* ai) : Action(ai, "glyph equip") {}
    bool Execute(Event event) override;

    /// ---- Rendu public pour être utilisable par le cache global ----
    struct GlyphInfo
    {
        GlyphPropertiesEntry const* prop;   ///< entrée GlyphProperties.dbc
        ItemTemplate const*         proto;  ///< template de l’objet glyphe
    };

private:
    /// Construit la cache {itemId -> GlyphInfo}
    static void BuildGlyphCache();
    static GlyphInfo const* GetGlyphInfo(uint32 itemId);

    /// Parse & valide la liste d’items glyphes
    bool CollectGlyphs(std::vector<uint32> const& itemIds,
                       std::vector<GlyphInfo const*>& out) const;
};

#endif
