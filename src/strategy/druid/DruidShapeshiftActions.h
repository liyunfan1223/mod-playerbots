/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDSHAPESHIFTACTIONS_H
#define _PLAYERBOT_DRUIDSHAPESHIFTACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastBearFormAction : public CastBuffSpellAction
{
public:
    CastBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bear form") {}

    bool isPossible() override;
    bool isUseful() override;
};

class CastDireBearFormAction : public CastBuffSpellAction
{
public:
    CastDireBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "dire bear form") {}

    NextAction** getAlternatives() override;
};

class CastCatFormAction : public CastBuffSpellAction
{
public:
    CastCatFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cat form") {}
};

class CastTreeFormAction : public CastBuffSpellAction
{
public:
    CastTreeFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "tree of life") {}
    bool isUseful() override;
};

class CastMoonkinFormAction : public CastBuffSpellAction
{
public:
    CastMoonkinFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "moonkin form") {}
};

class CastAquaticFormAction : public CastBuffSpellAction
{
public:
    CastAquaticFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aquatic form") {}
};

class CastTravelFormAction : public CastBuffSpellAction
{
public:
    CastTravelFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "travel form") {}

    bool isUseful() override;
};

class CastCasterFormAction : public CastBuffSpellAction
{
public:
    CastCasterFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "caster form") {}

    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Execute(Event event) override;
};

class CastCancelTreeFormAction : public CastBuffSpellAction
{
public:
    CastCancelTreeFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cancel tree form") {}

    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Execute(Event event) override;
};

#endif
