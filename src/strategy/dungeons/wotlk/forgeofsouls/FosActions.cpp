#include "Playerbots.h"
#include "FosActions.h"
#include "FosStrategy.h"

bool MoveFromBronjahmAction::Execute(Event event)
{
    Unit* boss = nullptr;
    boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    
    float distance = bot->GetExactDist2d(boss->GetPosition());
    float targetDis = 20.0f;
    if (distance >= targetDis)
        return false;
    return MoveAway(boss, targetDis - distance);
}

bool MoveFromBronjahmAction::isUseful() { return bot->HasAura(SPELL_CORRUPT_SOUL); }

bool AttackCorruptedSoulFragmentAction::Execute(Event event)
{
    Unit* fragment = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            fragment = unit;
            break;
        }
    }

    if (!fragment)
        return false;

    if (botAI->IsDps(bot))
    {
        if (AI_VALUE(Unit*, "current target") == fragment)
            return false;

        return Attack(fragment);
    }    
    else
        return false;
    
}

bool AttackCorruptedSoulFragmentAction::isUseful() { return botAI->IsDps(bot); }

bool BronjahmTankPositionAction::Execute(Event event)
{
    return MoveTo(bot->GetMapId(), BRONJAHM_TANK_POSITION.GetPositionX(), BRONJAHM_TANK_POSITION.GetPositionY(),
                  BRONJAHM_TANK_POSITION.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool BronjahmTankPositionAction::isUseful() { return bot->GetExactDist2d(BRONJAHM_TANK_POSITION) > 5.0f; }

bool BronjahmTankTargetAction::Execute(Event event)
{
    if (botAI->IsTank(bot))
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
        if (AI_VALUE(Unit*, "current target") == boss)
            return false;

        return Attack(boss);
    }
    else
        return false;
}

bool BronjahmDpsPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->getClass() == CLASS_HUNTER)
    {
        return Move(bot->GetAngle(boss), 8.0f);
    }
    else
    {
        return Move(bot->GetAngle(boss), 5.0f);
    }
}

bool BronjahmDpsPositionAction::isUseful()
{
    if (bot->GetExactDist2d(BRONJAHM_TANK_POSITION) <= 10.0f)
        return false;

    return botAI->IsDps(bot) || botAI->IsHeal(bot);
}
