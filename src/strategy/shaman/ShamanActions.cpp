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
    uint32 totemSpell = 0;
    for (int i = (int)totemSpellIdsCount - 1; i >= 0; --i)
    {
        if (bot->HasSpell(totemSpellIds[i]))
        {
            totemSpell = totemSpellIds[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(actionButtonId, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetTotemAction::isUseful()
{
    Player* bot = botAI->GetBot();
    ActionButton const* button = bot->GetActionButton(actionButtonId);
    if (!button || button->GetType() != ACTION_BUTTON_SPELL || button->GetAction() == 0)
        return true; // No totem assigned

    // Find the highest rank the bot knows
    for (int i = (int)totemSpellIdsCount - 1; i >= 0; --i)
    {
        if (bot->HasSpell(totemSpellIds[i]))
            return button->GetAction() != totemSpellIds[i];
    }
    // Bot doesn't know any valid rank
    return false;
}
