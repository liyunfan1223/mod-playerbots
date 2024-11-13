#include "Playerbots.h"
#include "ForgeOfSoulsActions.h"
#include "ForgeOfSoulsStrategy.h"

bool MoveFromBronjahmAction::Execute(Event event)
{    
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    
    float distance = bot->GetExactDist2d(boss->GetPosition());
    float targetDis = 20.0f;
    if (distance >= targetDis)
        return false;
    return MoveAway(boss, targetDis - distance);
}


bool AttackCorruptedSoulFragmentAction::Execute(Event event)
{
    Unit* fragment = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto &target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            fragment = unit;
            break;
        }
    }

    if (fragment && AI_VALUE(Unit*, "current target") != fragment)
        return Attack(fragment);

    return false;
    
}


bool BronjahmGroupPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (botAI->IsTank(bot))
    {
        bot->SetTarget(boss->GetGUID());
        if (AI_VALUE2(bool, "has aggro", "current target"))
            if (bot->GetExactDist2d(BRONJAHM_TANK_POSITION) > 5.0f)
                return MoveTo(bot->GetMapId(), BRONJAHM_TANK_POSITION.GetPositionX(),
                              BRONJAHM_TANK_POSITION.GetPositionY(), BRONJAHM_TANK_POSITION.GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_NORMAL);
            else
                return Attack(boss);
        else
        {
            return Attack(boss);
        }
    }
    else
    {
        float maxMovement = 10.0f;

        if (bot->GetExactDist2d(boss) > maxMovement)
        {
            if (bot->getClass() == CLASS_HUNTER)
            {
                return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 6.5f, maxMovement));
            }
            else
            {
                return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 2.0f, maxMovement));
            }
        }
        else
            return false;
    }
}

bool BronjahmGroupPositionAction::isUseful() { return true; }


