/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZGMultipliers.h"

#include <string>
#include <unordered_set>

#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "TotemsShamanStrategy.h"
#include "ZGHelpers.h"

using namespace ZulGurubHelpers;

float HakkarKeepPoisonousBloodMultiplier::GetValue(Action* action)
{
    // Blood Siphon drains the players: on one with Poisonous Blood it damages Hakkar, on anyone else it heals him.
    // The poison is dispellable, so curing it hands Hakkar the heal.
    auto* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    std::string const spell = spellAction->getSpell();

    // The totem pulses on the whole party. Totems go down at the pull, before Hakkar is on the bot's threat list,
    // and Call of the Elements drops whatever the water slot of the totem bar holds.
    if (spell == "cleansing totem" || spell == "call of the elements")
    {
        if (!bot->FindNearestCreature(NPC_HAKKAR, 100.0f))
            return 1.0f;
        if (spell == "cleansing totem")
            return 0.0f;
        ActionButton const* water = bot->GetActionButton(TOTEM_BAR_SLOT_WATER);
        return water && water->GetType() == ACTION_BUTTON_SPELL && water->GetAction() == CLEANSING_TOTEM[0] ? 0.0f
                                                                                                             : 1.0f;
    }

    // Every other bot spell that removes a poison; Stoneform makes the dwarf immune to poison, which purges it.
    static std::unordered_set<std::string> const poisonRemovers = {"cure poison",    "abolish poison", "cleanse",
                                                                   "purify",         "cleanse spirit", "cure toxins",
                                                                   "stoneform"};
    if (!poisonRemovers.count(spell) || !AI_VALUE2(Unit*, "find target", "hakkar"))
        return 1.0f;

    // Stoneform's action targets the current target, not the caster.
    Unit* target = spell == "stoneform" ? bot : action->GetTarget();
    return target && target->HasAura(SPELL_POISONOUS_BLOOD) ? 0.0f : 1.0f;
}
