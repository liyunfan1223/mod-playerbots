// RaidOnyxiaActions.cpp
#include "RaidOnyxiaActions.h"

#include "GenericSpellActions.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PositionAction.h"
#include "SharedDefines.h"

bool RaidOnyxiaPositionTankAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss || !botAI->IsTank(bot))
        return false;

    // Back wall
    const float safeX = 31.0f;
    const float safeY = -209.0f;

    const float safeZ = boss->GetPositionZ();

    float dist = bot->GetExactDist2d(safeX, safeY);
    if (dist > 5.0f)
    {
        bot->Yell("Moving to Tank Position!", LANG_UNIVERSAL);
        return MoveTo(boss->GetMapId(), safeX, safeY, safeZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool RaidOnyxiaMoveToSideAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss)
        return false;

    float angleToBot = boss->GetAngle(bot);
    float bossFacing = boss->GetOrientation();
    float diff = fabs(angleToBot - bossFacing);
    if (diff > M_PI)
        diff = 2 * M_PI - diff;

    if (diff < M_PI / 4 || diff > 3 * M_PI / 4)
    {
        // We're in front or behind, need to move to side
        float offsetAngle = bossFacing + M_PI_2;  // 90° to the right
        float offsetDist = 15.0f;

        float sideX = boss->GetPositionX() + offsetDist * cos(offsetAngle);
        float sideY = boss->GetPositionY() + offsetDist * sin(offsetAngle);

        bot->Yell("Moving to Side of Boss!", LANG_UNIVERSAL);
        return MoveTo(boss->GetMapId(), sideX, sideY, boss->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool RaidOnyxiaSpreadOutAction::Execute(Event event)
{
    bot->Yell("Spreading Out!", LANG_UNIVERSAL);
    return MoveFromGroup(9.0f);
}

bool RaidOnyxiaAvoidLavaAction::Execute(Event event)
{
    if (bot->HasAuraType(SPELL_AURA_PERIODIC_DAMAGE))
    {
        return MoveFromGroup(10.0f);
    }
    return false;
}

bool RaidOnyxiaMoveToSafeZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss)
        return false;

    Position bossPos = boss->GetPosition();
    float angle = boss->GetOrientation();  // Facing direction in radians
    float offset = 25.0f;

    // Pick one side (+90 degrees), perpendicular to breath - urand for a bit of
    // randomness
    float sideAngle = angle + (urand(0, 1) ? M_PI_2 : -M_PI_2);
    float safeX = bossPos.GetPositionX() + offset * cos(sideAngle);
    float safeY = bossPos.GetPositionY() + offset * sin(sideAngle);

    bot->Yell("Running To Safe Zone for Deep Breath!", LANG_UNIVERSAL);
    return MoveTo(boss->GetMapId(), safeX, safeY, bossPos.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}
