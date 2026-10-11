/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NAXXACTIONS_H
#define PLAYERBOTS_NAXXACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NaxxBossHelper.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class GrobbulusGoBehindAction : public MovementAction
{
public:
    GrobbulusGoBehindAction(PlayerbotAI* botAI, float distance = 24.0f, float delta_angle = M_PI / 8)
        : MovementAction(botAI, "grobbulus go behind")
    {
        this->distance = distance;
        this->delta_angle = delta_angle;
    }
    virtual bool Execute(Event event);

protected:
    float distance, delta_angle;
};

class GrobbulusRotateAction : public RotateAroundTheCenterPointAction
{
public:
    GrobbulusRotateAction(PlayerbotAI* botAI)
        : RotateAroundTheCenterPointAction(botAI, "rotate grobbulus", 3281.23f, -3310.38f, 35.0f, 8, true, M_PI) {}
    virtual bool isUseful() override
    {
        return RotateAroundTheCenterPointAction::isUseful() && PlayerbotAI::IsMainTank(bot) &&
               AI_VALUE2(bool, "has aggro", "boss target");
    }
    uint32 GetCurrWaypoint() override;
};

class GrobbulusMoveCenterAction : public MoveInsideAction
{
public:
    GrobbulusMoveCenterAction(PlayerbotAI* botAI) : MoveInsideAction(botAI, 3281.23f, -3310.38f, 5.0f) {}
};

class GrobbulusMoveAwayAction : public MovementAction
{
public:
    GrobbulusMoveAwayAction(PlayerbotAI* botAI, float distance = 18.0f)
        : MovementAction(botAI, "grobbulus move away"), distance(distance)
    {
    }
    bool Execute(Event event) override;

private:
    float distance;
};

// One action for both roles: melee (and tanks) dance every phase, ranged/healers wait on the platform during the
// slow dance and only dance the fast one.
class HeiganDanceAction : public MovementAction
{
public:
    HeiganDanceAction(PlayerbotAI* botAI, bool ranged)
        : MovementAction(botAI, ranged ? "heigan dance ranged" : "heigan dance melee"), helper(botAI), ranged(ranged)
    {
    }
    bool Execute(Event event) override;

private:
    // Move to (near) the given dance waypoint. Returns true while a move had to be issued.
    bool MoveToWaypoint(uint32 index, float distance);
    bool MoveToPlatform(float distance);

    HeiganBossHelper helper;
    bool ranged;
    int32 lastWaypoint = -1;
};

class ThaddiusAttackNearestPetAction : public AttackAction
{
public:
    ThaddiusAttackNearestPetAction(PlayerbotAI* botAI) : AttackAction(botAI, "thaddius attack nearest pet"), helper(botAI) {}
    virtual bool Execute(Event event);
    virtual bool isUseful();

private:
    ThaddiusBossHelper helper;
};

// class ThaddiusMeleeToPlaceAction : public MovementAction
// {
// public:
//     ThaddiusMeleeToPlaceAction(PlayerbotAI* botAI) : MovementAction(botAI, "thaddius melee to place") {}
//     virtual bool Execute(Event event);
//     virtual bool isUseful();
// };

// class ThaddiusRangedToPlaceAction : public MovementAction
// {
// public:
//     ThaddiusRangedToPlaceAction(PlayerbotAI* botAI) : MovementAction(botAI, "thaddius ranged to place") {}
//     virtual bool Execute(Event event);
//     virtual bool isUseful();
// };

class ThaddiusMoveToPlatformAction : public MovementAction
{
public:
    ThaddiusMoveToPlatformAction(PlayerbotAI* botAI) : MovementAction(botAI, "thaddius move to platform") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class ThaddiusMovePolarityAction : public MovementAction
{
public:
    ThaddiusMovePolarityAction(PlayerbotAI* botAI) : MovementAction(botAI, "thaddius move polarity") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class RazuviousUseObedienceCrystalAction : public MovementAction
{
public:
    RazuviousUseObedienceCrystalAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "razuvious use obedience crystal"), helper(botAI)
    {
    }
    bool Execute(Event event) override;

private:
    RazuviousBossHelper helper;
};

class RazuviousTargetAction : public AttackAction
{
public:
    RazuviousTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "razuvious target"), helper(botAI) {}
    bool Execute(Event event) override;

private:
    RazuviousBossHelper helper;
};

class FourHorsemenAttractAlternativelyAction : public AttackAction
{
public:
    FourHorsemenAttractAlternativelyAction(PlayerbotAI* botAI) : AttackAction(botAI, "four horsemen attract alternatively"), helper(botAI)
    {
    }
    bool Execute(Event event) override;

protected:
    FourHorsemenBossHelper helper;
};

class FourHorsemenAttackInOrderAction : public AttackAction
{
public:
    FourHorsemenAttackInOrderAction(PlayerbotAI* botAI) : AttackAction(botAI, "four horsemen attack in order"), helper(botAI) {}
    bool Execute(Event event) override;

protected:
    FourHorsemenBossHelper helper;
};

// class SapphironGroundMainTankPositionAction : public MovementAction
// {
// public:
//     SapphironGroundMainTankPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "sapphiron ground main tank
//     position") {} virtual bool Execute(Event event);
// };

class SapphironGroundPositionAction : public MovementAction
{
public:
    SapphironGroundPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "sapphiron ground position"), helper(botAI) {}
    bool Execute(Event event) override;

protected:
    SapphironBossHelper helper;
};

class SapphironFlightPositionAction : public MovementAction
{
public:
    SapphironFlightPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "sapphiron flight position"), helper(botAI) {}
    bool Execute(Event event) override;

protected:
    SapphironBossHelper helper;
    bool MoveToNearestIcebolt();
};

// class SapphironAvoidChillAction : public MovementAction
// {
// public:
//     SapphironAvoidChillAction(PlayerbotAI* botAI) : MovementAction(botAI, "sapphiron avoid chill") {}
//     virtual bool Execute(Event event);
// };

class KelthuzadChooseTargetAction : public AttackAction
{
public:
    KelthuzadChooseTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "kel'thuzad choose target"), helper(botAI) {}
    virtual bool Execute(Event event);

private:
    KelthuzadBossHelper helper;
};

class KelthuzadPositionAction : public MovementAction
{
public:
    KelthuzadPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "kel'thuzad position"), helper(botAI) {}
    virtual bool Execute(Event event);

private:
    KelthuzadBossHelper helper;
};

class AnubrekhanChooseTargetAction : public AttackAction
{
public:
    AnubrekhanChooseTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "anub'rekhan choose target") {}
    bool Execute(Event event) override;
};

class AnubrekhanPositionAction : public RotateAroundTheCenterPointAction
{
public:
    AnubrekhanPositionAction(PlayerbotAI* botAI)
        : RotateAroundTheCenterPointAction(botAI, "anub'rekhan position", 3272.49f, -3476.27f, 45.0f, 16) {}
    bool Execute(Event event) override;
};

class GluthChooseTargetAction : public AttackAction
{
public:
    GluthChooseTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "gluth choose target"), helper(botAI) {}
    bool Execute(Event event) override;

private:
    GluthBossHelper helper;
};

class GluthPositionAction : public RotateAroundTheCenterPointAction
{
public:
    GluthPositionAction(PlayerbotAI* botAI)
        : RotateAroundTheCenterPointAction(botAI, "gluth position", 3293.61f, -3149.01f, 12.0f, 12), helper(botAI) {}
    bool Execute(Event event) override;

private:
    GluthBossHelper helper;
};

class GluthSlowdownAction : public Action
{
public:
    GluthSlowdownAction(PlayerbotAI* botAI) : Action(botAI, "gluth slowdown"), helper(botAI) {}
    bool Execute(Event event) override;

private:
    GluthBossHelper helper;
};

class LoathebPositionAction : public MovementAction
{
public:
    LoathebPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "loatheb position"), helper(botAI) {}
    virtual bool Execute(Event event);

private:
    LoathebBossHelper helper;
};

class LoathebChooseTargetAction : public AttackAction
{
public:
    LoathebChooseTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "loatheb choose target"), helper(botAI) {}
    virtual bool Execute(Event event);

private:
    LoathebBossHelper helper;
};

//class PatchwerkRangedPositionAction : public MovementAction
//{
//public:
//    PatchwerkRangedPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "patchwerk ranged position") {}
//    bool Execute(Event event) override;
//};

#endif
