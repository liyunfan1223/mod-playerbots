/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanActions.h"
#include "TotemsShamanStrategy.h"
#include "Playerbots.h"
#include "Totem.h"
#include "PlayerbotAI.h"
#include "Action.h"
#include <sstream>
#include "SpellMgr.h"
#include "WorldSession.h"

bool CastCallOfTheElementsAction::Execute(Event event)
{
    // Whisp the 4 totems slots de CoE if debug is activated
    if (botAI->HasStrategy("debug", BotState::BOT_STATE_COMBAT) ||
        botAI->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
    {
        Player* bot = botAI->GetBot();
        LocaleConstant loc = DEFAULT_LOCALE;
        if (bot->GetSession())
            loc = bot->GetSession()->GetSessionDbcLocale();

        auto spellName = [loc](uint32 id) -> std::string {
            if (!id) return "none";
            if (const SpellInfo* si = sSpellMgr->GetSpellInfo(id))
            {
                const char* n = si->SpellName[loc];
                return (n && *n) ? std::string(n) : std::string("spell ") + std::to_string(id);
            }
            return std::string("spell ") + std::to_string(id);
        };

        // Totem bar slots: 0=Earth, 1=Fire, 2=Water, 3=Air
        const int slots[4] = { TOTEM_BAR_SLOT_EARTH, TOTEM_BAR_SLOT_FIRE, TOTEM_BAR_SLOT_WATER, TOTEM_BAR_SLOT_AIR };
        const char* slotLbl[4] = { "earth", "fire", "water", "air" };

        std::ostringstream out;
        out << "[debug][totem] Call of the Elements -> ";
        for (int i = 0; i < 4; ++i)
        {
            const ActionButton* button = bot->GetActionButton(slots[i]);
            uint32 id = (button && button->GetType() == ACTION_BUTTON_SPELL) ? button->GetAction() : 0;
            out << slotLbl[i] << "=" << spellName(id) << " (id " << id << ")";
            if (i != 3) out << ", ";
        }
        botAI->TellMasterNoFacing(out.str());
    }
    return CastSpellAction::Execute(event);
}

bool CastTotemAction::isUseful()
{
    return CastBuffSpellAction::isUseful()
        && !AI_VALUE2(bool, "has totem", name)
        && !botAI->HasAura(buff, bot);
}

bool CastMagmaTotemAction::isUseful() {
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !bot->IsWithinMeleeRange(target))
        return false;

    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", name);
}

bool CastFireNovaAction::isUseful() {
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    Creature* fireTotem = bot->GetMap()->GetCreature(bot->m_SummonSlot[1]);
    if (!fireTotem)
        return false;

    if (target->GetDistance(fireTotem) > 8.0f)
        return false;

    return CastMeleeSpellAction::isUseful();
}

bool CastCleansingTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "mana tide totem");
}

// Will only cast Stoneclaw Totem if low on health and not in a group
bool CastStoneclawTotemAction::isUseful() { return !botAI->GetBot()->GetGroup(); }

// Will only cast Lava Burst if Flame Shock is on the target
bool CastLavaBurstAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    static const uint32 FLAME_SHOCK_IDS[] = {8050, 8052, 8053, 10447, 10448, 29228, 25457, 49232, 49233};

    ObjectGuid botGuid = botAI->GetBot()->GetGUID();
    for (uint32 spellId : FLAME_SHOCK_IDS)
    {
        if (target->HasAura(spellId, botGuid))
            return true;
    }
    return false;
}

// Logic for making a guardian (spirit wolf) use a spell (spirit walk)
// There is no existing code for guardians casting spells in the AC/Playerbots repo.
bool CastSpiritWalkAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    constexpr uint32 SPIRIT_WOLF = 29264;
    constexpr uint32 SPIRIT_WALK_SPELL = 58875;

    for (Unit* unit : bot->m_Controlled)
    {
        if (unit->GetEntry() == SPIRIT_WOLF)
        {
            if (unit->HasSpell(SPIRIT_WALK_SPELL))
            {
                unit->CastSpell(unit, SPIRIT_WALK_SPELL, false);
                return true;
            }
        }
    }
    return false;
}

// Set Strategy Assigned Totems (Actions) - First, it checks
// the highest-rank spell the bot knows for each totem type,
// then adds it to the Call of the Elements bar.

bool SetTotemAction::Execute(Event event)
{
    // Pick the highest-rank spell the bot knows (arrays are sorted from highest to lowest rank).
    uint32 totemSpell = 0;
    for (size_t i = 0; i < totemSpellIdsCount; ++i)
    {
        if (bot->HasSpell(totemSpellIds[i]))
        {
            totemSpell = totemSpellIds[i];
            break; // first known => highest known rank
        }
    }

    if (!totemSpell)
    {
        return false; // bot doesn't know any rank for this totem family
    }

    // Micro-safety: if the slot already holds this spell, do nothing.
    if (const ActionButton* button = bot->GetActionButton(actionButtonId))
    {
        if (button->GetType() == ACTION_BUTTON_SPELL && button->GetAction() == totemSpell)
        {
            return true; // already set correctly
        }
    }

    bot->addActionButton(actionButtonId, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}