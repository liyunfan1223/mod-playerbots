// RaidOnyxiaActions.cpp
#include "RaidOnyxiaActions.h"

#include "GenericSpellActions.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PositionAction.h"

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

    float distance = bot->GetDistance(boss);

    // Too close (30 yards) and either in front or behind
    if (distance <= 30.0f && (diff < M_PI / 4 || diff > 3 * M_PI / 4))
    {
        float offsetAngle = bossFacing + M_PI_2;  // 90° to the right
        float offsetDist = 15.0f;

        float sideX = boss->GetPositionX() + offsetDist * cos(offsetAngle);
        float sideY = boss->GetPositionY() + offsetDist * sin(offsetAngle);

        bot->Yell("Too close to front or tail — moving to side of Onyxia!", LANG_UNIVERSAL);
        return MoveTo(boss->GetMapId(), sideX, sideY, boss->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool RaidOnyxiaSpreadOutAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");

    if (!boss)
        return false;

    Player* target = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_targets.GetUnitTarget()->ToPlayer();
    if (target != bot)
        return false;

    bot->Yell("Spreading out — I'm the Fireball target!", LANG_UNIVERSAL);
    return MoveFromGroup(9.0f);  // move 9 yards
}

bool RaidOnyxiaMoveToSafeZoneAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss)
        return false;

    Position bossPos = boss->GetPosition();
    float angle = boss->GetOrientation();  // Facing direction in radians
    float offset = 30.0f;

    // Pick one side (+90 degrees), perpendicular to breath - urand for a bit of
    // randomness
    float sideAngle = angle + (urand(0, 1) ? M_PI_2 : -M_PI_2);
    float safeX = bossPos.GetPositionX() + offset * cos(sideAngle);
    float safeY = bossPos.GetPositionY() + offset * sin(sideAngle);

    bot->Yell("Running To Safe Zone for Deep Breath!", LANG_UNIVERSAL);
    return MoveTo(boss->GetMapId(), safeX, safeY, bossPos.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}
