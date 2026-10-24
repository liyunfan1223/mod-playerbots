/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZGMultipliers.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "TotemsShamanStrategy.h"
#include "ZGHelpers.h"
#include <string>
#include <unordered_set>

using namespace ZulGurubHelpers;

float HakkarKeepPoisonousBloodMultiplier::GetValueInEncounter(Action* action)
{
    // Blood Siphon drains the players: on one with Poisonous Blood it damages Hakkar, on anyone else it heals him.
    // The poison is dispellable, so curing it hands Hakkar the heal.
    auto* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    // The bots' poison cures. Abolish Poison and Cleansing Totem keep removing it after the cast.
    static std::unordered_set<std::string> const poisonRemovers = {
        "cure poison", "abolish poison", "cleanse", "purify", "cleanse spirit", "cure toxins", "stoneform",
        "cleansing totem", "call of the elements"};
    std::string const spell = spellAction->getSpell();
    if (!poisonRemovers.count(spell) || !AI_VALUE2(Unit*, "find target", "hakkar"))
        return 1.0f;

    if (spell == "abolish poison" || spell == "cleansing totem")
        return 0.0f;

    // Call of the Elements drops whatever the water slot of the totem bar holds.
    if (spell == "call of the elements")
    {
        ActionButton const* water = bot->GetActionButton(TOTEM_BAR_SLOT_WATER);
        bool const cleansingInSlot =
            water && water->GetType() == ACTION_BUTTON_SPELL && water->GetAction() == CLEANSING_TOTEM[0];
        return cleansingInSlot ? 0.0f : 1.0f;
    }

    Unit* target = action->GetTarget();
    return target && target->HasAura(static_cast<uint32>(ZulGurubSpells::SPELL_POISONOUS_BLOOD)) ? 0.0f : 1.0f;
}
