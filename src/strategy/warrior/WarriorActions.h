/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARRIORACTIONS_H
#define _PLAYERBOT_WARRIORACTIONS_H

#include "AiObject.h"
#include "GenericSpellActions.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "ReachTargetActions.h"

// stances
BUFF_ACTION(CastBattleStanceAction, "battle stance");
BUFF_ACTION(CastDefensiveStanceAction, "defensive stance");
BUFF_ACTION(CastBerserkerStanceAction, "berserker stance");

// shouts
BUFF_ACTION(CastBattleShoutAction, "battle shout");
MELEE_ACTION_U(CastBattleShoutTauntAction, "battle shout", CastSpellAction::isUseful()); // useful to rebuff
DEBUFF_ACTION_R(CastDemoralizingShoutAction, "demoralizing shout", 8.0f);          // low range debuff
class CastDemoralizingShoutWithoutLifeTimeCheckAction : public CastDebuffSpellAction
{
    public:
        CastDemoralizingShoutWithoutLifeTimeCheckAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "demoralizing shout", false, 0.0f) \
        {
            range = 8.0f;
        }
};

MELEE_ACTION(CastChallengingShoutAction, "challenging shout");
DEBUFF_ACTION_R(CastIntimidatingShoutAction, "intimidating shout", 8.0f);
// shouts 2.4.3
BUFF_ACTION(CastCommandingShoutAction, "commanding shout");

// arms
MELEE_ACTION(CastHeroicStrikeAction, "heroic strike");
REACH_ACTION(CastChargeAction, "charge", 8.0f);
DEBUFF_CHECKISOWNER_ACTION(CastRendAction, "rend");
// DEBUFF_ENEMY_ACTION(CastRendOnAttackerAction, "rend");

class CastRendOnAttackerAction : public CastDebuffSpellOnMeleeAttackerAction
{
    public:
        CastRendOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnMeleeAttackerAction(botAI, "rend") {}
};

MELEE_ACTION(CastThunderClapAction, "thunder clap");
SNARE_ACTION(CastThunderClapSnareAction, "thunder clap");
SNARE_ACTION(CastHamstringAction, "hamstring");
MELEE_ACTION(CastOverpowerAction, "overpower");
MELEE_ACTION(CastMockingBlowAction, "mocking blow");
BUFF_ACTION(CastRetaliationAction, "retaliation");
// arms 3.3.5
SPELL_ACTION(CastHeroicThrowAction, "heroic throw");
SNARE_ACTION(CastHeroicThrowSnareAction, "heroic throw");
DEBUFF_ACTION(CastShatteringThrowAction, "shattering throw");

// arms talents
MELEE_ACTION(CastMortalStrikeAction, "mortal strike");
BUFF_ACTION(CastSweepingStrikesAction, "sweeping strikes");
// arms talents 3.3.5
BUFF_ACTION(CastBladestormAction, "bladestorm");

// fury
MELEE_ACTION(CastCleaveAction, "cleave");
MELEE_ACTION(CastExecuteAction, "execute");
REACH_ACTION(CastInterceptAction, "intercept", 8.0f);
ENEMY_HEALER_ACTION(CastInterceptOnEnemyHealerAction, "intercept");
SNARE_ACTION(CastInterceptOnSnareTargetAction, "intercept");
MELEE_ACTION(CastSlamAction, "slam");
BUFF_ACTION(CastBerserkerRageAction, "berserker rage");
MELEE_ACTION(CastWhirlwindAction, "whirlwind");
MELEE_ACTION(CastPummelAction, "pummel");
ENEMY_HEALER_ACTION(CastPummelOnEnemyHealerAction, "pummel");
BUFF_ACTION(CastRecklessnessAction, "recklessness");
// fury 2.4.3
MELEE_ACTION(CastVictoryRushAction, "victory rush");
// fury 3.3.5
BUFF_ACTION(CastEnragedRegenerationAction, "enraged regeneration");
BUFF_ACTION(CastHeroicFuryAction, "heroic fury");

// fury talents
BUFF_ACTION(CastDeathWishAction, "death wish");
MELEE_ACTION(CastBloodthirstAction, "bloodthirst");
DEBUFF_ACTION_R(CastPiercingHowlAction, "piercing howl", 8.0f);
// fury talents 2.4.3
BUFF_ACTION(CastRampageAction, "rampage");

// protection
MELEE_ACTION_U(CastTauntAction, "taunt", GetTarget() && GetTarget()->GetTarget() != bot->GetGUID());
SNARE_ACTION(CastTauntOnSnareTargetAction, "taunt");
BUFF_ACTION(CastBloodrageAction, "bloodrage");
MELEE_ACTION(CastShieldBashAction, "shield bash");
ENEMY_HEALER_ACTION(CastShieldBashOnEnemyHealerAction, "shield bash");
MELEE_ACTION(CastRevengeAction, "revenge");
BUFF_ACTION(CastShieldBlockAction, "shield block");
DEBUFF_ACTION_U(CastDisarmAction, "disarm", GetTarget() && GetTarget()->IsPlayer() ? !botAI->IsRanged((Player*)GetTarget()) : CastDebuffSpellAction::isUseful());
DEBUFF_ENEMY_ACTION(CastDisarmOnAttackerAction, "disarm");
BUFF_ACTION(CastShieldWallAction, "shield wall");
// protection 2.4.3
PROTECT_ACTION(CastInterveneAction, "intervene");
BUFF_ACTION(CastSpellReflectionAction, "spell reflection");

// protection talents
BUFF_ACTION(CastLastStandAction, "last stand");
MELEE_ACTION(CastShieldSlamAction, "shield slam");
MELEE_ACTION(CastConcussionBlowAction, "concussion blow");
// protection talents 2.4.3
MELEE_ACTION(CastDevastateAction, "devastate");
// protection talents 3.3.5
DEBUFF_ACTION_R(CastShockwaveAction, "shockwave", 8.0f);
SNARE_ACTION(CastShockwaveSnareAction, "shockwave");

class CastSunderArmorAction : public CastDebuffSpellAction
{
    public:
        CastSunderArmorAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "sunder armor")
        {
            range = ATTACK_DISTANCE;
        }

        bool isUseful() override;
};

#endif
