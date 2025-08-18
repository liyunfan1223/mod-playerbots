/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ROGUECOMBOACTIONS_H
#define _PLAYERBOT_ROGUECOMBOACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastComboAction : public CastMeleeSpellAction
{
public:
    CastComboAction(PlayerbotAI* botAI, std::string const name) : CastMeleeSpellAction(botAI, name) {}

    bool isUseful() override;
};

class CastSinisterStrikeAction : public CastSpellAction
{
public:
    CastSinisterStrikeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "sinister strike") {}
};

class CastMutilateAction : public CastSpellAction
{
public:
    CastMutilateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "mutilate") {}
};

class CastRiposteAction : public CastSpellAction
{
public:
    CastRiposteAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "riposte") {}
};

class CastGougeAction : public CastSpellAction
{
public:
    CastGougeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "gouge") {}
};

class CastBackstabAction : public CastSpellAction
{
public:
    CastBackstabAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "backstab") {}
};

#endif
