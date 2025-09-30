/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDCATACTIONS_H
#define _PLAYERBOT_DRUIDCATACTIONS_H

#include "GenericSpellActions.h"
#include "ReachTargetActions.h"

class PlayerbotAI;

class CastFeralChargeCatAction : public CastReachTargetSpellAction
{
public:
    CastFeralChargeCatAction(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, "feral charge - cat", 1.5f) {}
};

class CastCowerAction : public CastBuffSpellAction
{
public:
    CastCowerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cower") {}
};

class CastBerserkAction : public CastBuffSpellAction
{
public:
    CastBerserkAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "berserk") {}
};

class CastTigersFuryAction : public CastBuffSpellAction
{
public:
    CastTigersFuryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "tiger's fury") {}
};

class CastSavageRoarAction : public CastBuffSpellAction
{
public:
    CastSavageRoarAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "savage roar") {}
    std::string const GetTargetName() override { return "current target"; }
};

class CastRakeAction : public CastDebuffSpellAction
{
public:
    CastRakeAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "rake", true, 6.0f) {}
};

class CastRakeOnMeleeAttackersAction : public CastDebuffSpellOnMeleeAttackerAction
{
public:
    CastRakeOnMeleeAttackersAction(PlayerbotAI* botAI) : CastDebuffSpellOnMeleeAttackerAction(botAI, "rake", true, 6.0f) {}
};

class CastClawAction : public CastMeleeSpellAction
{
public:
    CastClawAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "claw") {}
};

class CastMangleCatAction : public CastMeleeSpellAction
{
public:
    CastMangleCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mangle (cat)") {}
};

class CastSwipeCatAction : public CastMeleeSpellAction
{
public:
    CastSwipeCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "swipe (cat)") {}
};

class CastFerociousBiteAction : public CastMeleeSpellAction
{
public:
    CastFerociousBiteAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "ferocious bite") {}
};

class CastRipAction : public CastMeleeDebuffSpellAction
{
public:
    CastRipAction(PlayerbotAI* botAI) : CastMeleeDebuffSpellAction(botAI, "rip", true, 12.0f) {}
};

class CastShredAction : public CastMeleeSpellAction
{
public:
    CastShredAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "shred") {}
};

class CastProwlAction : public CastBuffSpellAction
{
public:
    CastProwlAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "prowl") {}
};

class CastDashAction : public CastBuffSpellAction
{
public:
    CastDashAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "dash") {}
};

class CastRavageAction : public CastMeleeSpellAction
{
public:
    CastRavageAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "ravage") {}
};

class CastPounceAction : public CastMeleeSpellAction
{
public:
    CastPounceAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "pounce") {}
};

#endif
