/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_EQUIPGLYPHSACTION_H
#define PLAYERBOTS_EQUIPGLYPHSACTION_H

#include "Action.h"

// 1 = major, 2 = minor dans GlyphProperties.dbc
enum class GlyphKind : uint32 { MAJOR = 1, MINOR = 2 };

class EquipGlyphsAction : public Action
{
public:
    EquipGlyphsAction(PlayerbotAI* botAI) : Action(botAI, "glyph equip") {}
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
