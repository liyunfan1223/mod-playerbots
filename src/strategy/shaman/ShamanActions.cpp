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

bool SetStrengthOfEarthTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = STRENGTH_OF_EARTH_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(STRENGTH_OF_EARTH_TOTEM[i]))
        {
            totemSpell = STRENGTH_OF_EARTH_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_EARTH, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetStoneskinTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = STONESKIN_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(STONESKIN_TOTEM[i]))
        {
            totemSpell = STONESKIN_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_EARTH, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetTremorTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = TREMOR_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(TREMOR_TOTEM[i]))
        {
            totemSpell = TREMOR_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_EARTH, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetEarthbindTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = EARTHBIND_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(EARTHBIND_TOTEM[i]))
        {
            totemSpell = EARTHBIND_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_EARTH, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetSearingTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = SEARING_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(SEARING_TOTEM[i]))
        {
            totemSpell = SEARING_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_FIRE, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetMagmaTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = MAGMA_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(MAGMA_TOTEM[i]))
        {
            totemSpell = MAGMA_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_FIRE, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetFlametongueTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = FLAMETONGUE_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(FLAMETONGUE_TOTEM[i]))
        {
            totemSpell = FLAMETONGUE_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_FIRE, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetTotemOfWrathAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = TOTEM_OF_WRATH_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(TOTEM_OF_WRATH[i]))
        {
            totemSpell = TOTEM_OF_WRATH[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_FIRE, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetFrostResistanceTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = FROST_RESISTANCE_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(FROST_RESISTANCE_TOTEM[i]))
        {
            totemSpell = FROST_RESISTANCE_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_FIRE, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetHealingStreamTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = HEALING_STREAM_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(HEALING_STREAM_TOTEM[i]))
        {
            totemSpell = HEALING_STREAM_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_WATER, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetManaSpringTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = MANA_SPRING_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(MANA_SPRING_TOTEM[i]))
        {
            totemSpell = MANA_SPRING_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_WATER, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetCleansingTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = CLEANSING_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(CLEANSING_TOTEM[i]))
        {
            totemSpell = CLEANSING_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_WATER, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetFireResistanceTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = FIRE_RESISTANCE_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(FIRE_RESISTANCE_TOTEM[i]))
        {
            totemSpell = FIRE_RESISTANCE_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_WATER, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetWrathOfAirTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = WRATH_OF_AIR_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(WRATH_OF_AIR_TOTEM[i]))
        {
            totemSpell = WRATH_OF_AIR_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_AIR, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetWindfuryTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = WINDFURY_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(WINDFURY_TOTEM[i]))
        {
            totemSpell = WINDFURY_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_AIR, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetNatureResistanceTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = NATURE_RESISTANCE_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(NATURE_RESISTANCE_TOTEM[i]))
        {
            totemSpell = NATURE_RESISTANCE_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_AIR, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}

bool SetGroundingTotemAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    uint32 totemSpell = 0;
    for (int i = GROUNDING_TOTEM_COUNT - 1; i >= 0; --i)
    {
        if (bot->HasSpell(GROUNDING_TOTEM[i]))
        {
            totemSpell = GROUNDING_TOTEM[i];
            break;
        }
    }
    if (!totemSpell)
        return false;
    bot->addActionButton(TOTEM_BAR_SLOT_AIR, totemSpell, ACTION_BUTTON_SPELL);
    return true;
}
