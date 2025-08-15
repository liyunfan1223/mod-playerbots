/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
 
#pragma once

#include <string>
#include <functional>
#include "Common.h"

class Player;
class PlayerbotAI;

namespace ai::buff
{
    /**
     * Construit un qualifier d’aura "mono + greater" pour éviter le double-buff.
     * Exemple: "blessing of might" -> "blessing of might,greater blessing of might"
     */
    std::string MakeAuraQualifierForBuff(std::string const& name);

    /**
     * Retourne le nom du sort de groupe pour un buff mono donné.
     * Exemple: "blessing of wisdom" -> "greater blessing of wisdom"
     * Si aucun équivalent de groupe n'existe, retourne "".
     */
    std::string GroupVariantFor(std::string const& name);

    /**
     * Vérifie si le bot possède les composants requis pour caster un spell (via son spellId).
     * Retourne false si le spellId est invalide.
     */
    bool HasRequiredReagents(Player* bot, uint32 spellId);

    /**
     * Applique la politique "passer en buff de groupe" si:
     *  - le bot est dans un groupe de 3+,
     *  - la variante de groupe est connue/utile,
     *  - et les composants sont présents.
     *
     * Sinon, retourne baseName.
     *
     * Si announceOnMissing == true et que les composants manquent,
     * on appelle le callback 'announce' (si fourni) pour prévenir le groupe/party.
     */
    std::string UpgradeToGroupIfAppropriate(
        Player* bot,
        PlayerbotAI* botAI,
        std::string const& baseName,
        bool announceOnMissing = false,
        std::function<void(std::string const&)> announce = {}
    );
}
