/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DRUIDCATACTIONS_H
#define PLAYERBOTS_DRUIDCATACTIONS_H

#include "GenericSpellActions.h"
#include "ReachTargetActions.h"

constexpr uint32 SPELL_POUNCE_RANK_1 = 9005;
constexpr uint32 SPELL_RAVAGE_RANK_1 = 6785;

class PlayerbotAI;

class CastFeralChargeCatAction : public CastReachTargetSpellAction
{
public:
    CastFeralChargeCatAction(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, "feral charge - cat", 1.5f) {}

    bool isUseful() override
    {
        if (botAI->HasAura("prowl", bot))
            return false;

        return CastReachTargetSpellAction::isUseful();
    }
};

class CastCowerAction : public CastBuffSpellAction
{
public:
    CastCowerAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cower") {}
    bool isUseful() override;
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

class CastClawAction : public CastMeleeSpellAction
{
public:
    CastClawAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "claw") {}

    bool isUseful() override
    {
        // Block Claw once Pounce is learned; Claw remains available as the stealth opener before then.
        if (botAI->HasAura("prowl", bot) && bot->HasSpell(SPELL_POUNCE_RANK_1))
            return false;

        return CastMeleeSpellAction::isUseful();
    }
};

class CastMangleCatAction : public CastMeleeSpellAction
{
public:
    CastMangleCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "mangle (cat)") {}

    bool isUseful() override
    {
        if (botAI->HasAura("prowl", bot))
            return false;

        return CastMeleeSpellAction::isUseful();
    }
};

class CastSwipeCatAction : public CastMeleeSpellAction
{
public:
    CastSwipeCatAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "swipe (cat)") {}

    bool isUseful() override
    {
        if (botAI->HasAura("prowl", bot))
            return false;

        return CastMeleeSpellAction::isUseful();
    }
};

class CastFerociousBiteAction : public CastMeleeSpellAction
{
public:
    CastFerociousBiteAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "ferocious bite") {}
};

class CastMaimAction : public CastMeleeSpellAction
{
public:
    CastMaimAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "maim") {}

    bool isUseful() override
    {
        Unit* target = GetTarget();
        if (!target || !target->ToPlayer())
            return false;

        return CastMeleeSpellAction::isUseful();
    }
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

    bool isUseful() override
    {
        if (botAI->HasAura("prowl", bot) && bot->HasSpell(SPELL_RAVAGE_RANK_1))
            return false;

        return CastMeleeSpellAction::isUseful();
    }
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

    bool isUseful() override
    {
        if (!botAI->HasAura("prowl", bot))
            return false;

        return CastMeleeSpellAction::isUseful();
    }
};

class CastPounceAction : public CastMeleeSpellAction
{
public:
    CastPounceAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "pounce") {}

    bool isUseful() override
    {
        if (!botAI->HasAura("prowl", bot))
            return false;

        return CastMeleeSpellAction::isUseful();
    }
};

#endif
