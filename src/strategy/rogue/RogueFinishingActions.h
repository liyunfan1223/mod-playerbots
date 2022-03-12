/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_ROGUEFINISHINGACTIONS_H
#define _PLAYERBOT_ROGUEFINISHINGACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastEviscerateAction : public CastMeleeSpellAction
{
	public:
		CastEviscerateAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "eviscerate") { }
};

class CastSliceAndDiceAction : public CastMeleeSpellAction
{
	public:
		CastSliceAndDiceAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "slice and dice") { }
};

class CastExposeArmorAction : public CastMeleeSpellAction
{
	public:
		CastExposeArmorAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "expose armor") { }
};

class CastRuptureAction : public CastMeleeSpellAction
{
	public:
		CastRuptureAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "rupture") { }
};

class CastKidneyShotAction : public CastMeleeSpellAction
{
	public:
		CastKidneyShotAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "kidney shot") { }
};

#endif
