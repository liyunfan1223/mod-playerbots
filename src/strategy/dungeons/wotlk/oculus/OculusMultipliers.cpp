#include "OculusMultipliers.h"
#include "OculusActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "OculusTriggers.h"
#include "FollowActions.h"
#include "ReachTargetActions.h"

float MountingDrakeMultiplier::GetValue(Action* action)
{
    // P.I.T.A bug where the bots will somehow interrupt their item spell use,
    // even though the 0.5 sec cast goes off, it puts the drake essence on 15 sec cd
    // and no drake comes down.
    // It seems like this is due to moving/other actions being processed during the 0.5 secs.
    // If we suppress everything, they seem to mount properly. A bit of a ham-fisted solution but it works
    Player* master = botAI->GetMaster();
    if (bot->GetMapId() != OCULUS_MAP_ID || !master->GetVehicleBase() || bot->GetVehicleBase()) { return 1.0f; }

    if (!dynamic_cast<MountDrakeAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float FlyingMultiplier::GetValue(Action* action)
{
    if (bot->GetMapId() != OCULUS_MAP_ID || !bot->GetVehicleBase()) { return 1.0f; }

    // Suppresses FollowAction as well as some attack-based movements
    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<FlyDrakeAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

float UromMultiplier::GetValue(Action* action)
{
    if(GetPhaseByCurrentPosition(bot) < 3)
    {
        Unit* target = action->GetTarget();
        if (target && target->GetEntry() == NPC_MAGE_LORD_UROM)
        {
            return 0.0f;
        }
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "mage-lord urom");
    if (!boss) { return 1.0f; }

    // REAL BOSS FIGHT
    if (boss->HasUnitState(UNIT_STATE_CASTING) &&
        boss->FindCurrentSpellBySpellId(SPELL_EMPOWERED_ARCANE_EXPLOSION))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AvoidArcaneExplosionAction*>(action))
        {
            return 0.0f;
        }
    }

    // Don't bother avoiding Frostbomb for melee
    if (botAI->IsMelee(bot))
    {
        if (dynamic_cast<AvoidAoeAction*>(action))
        {
            return 0.0f;
        }
    }

    if (bot->HasAura(SPELL_TIME_BOMB))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<TimeBombSpreadAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

uint8 UromMultiplier::GetPhaseByCurrentPosition(Unit* unit)
{
    // Distance to return a positive match for spawn platforms, tweak slightly if needed/
    // Make sure this doesn't get too large and reach the central ring as well
    float distance = 60.0f;

    for (uint8 i = 0; i < 3; ++i)
    {
        if (unit->GetDistance(uromCoords[i][0], uromCoords[i][1], uromCoords[i][2]) < distance)
        {
            return i;
        }
    }
    return 3;
}

float EregosMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ley-guardian eregos");
    if (!boss) { return 1.0f; }

    if (boss->HasAura(SPELL_PLANAR_SHIFT && dynamic_cast<DrakeAttackAction*>(action)))
    {
        return 0.0f;
    }
    return 1.0f;
}
