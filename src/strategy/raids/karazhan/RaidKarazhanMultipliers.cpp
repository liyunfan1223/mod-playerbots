#include "RaidKarazhanMultipliers.h"
#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "AiObjectContext.h"
#include "AttackAction.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "RogueActions.h"
#include "WarriorActions.h"

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float KarazhanAttumenTheHuntsmanMultiplier::GetValue(Action* action)
{
    RaidKarazhanHelpers karazhanHelper(botAI);
    Unit* boss = karazhanHelper.GetFirstAliveUnitByEntry(NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (boss && !(botAI->IsTank(bot) && botAI->HasAggro(boss) && boss->GetVictim() == bot) &&
       (dynamic_cast<MovementAction*>(action) &&
       !dynamic_cast<KarazhanAttumenTheHuntsmanStackBehindAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KarazhanBigBadWolfMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!boss)
    {
        return 1.0f;
    }

    if (bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD))
    {
        if ((dynamic_cast<MovementAction*>(action) && !dynamic_cast<KarazhanBigBadWolfRunAwayAction*>(action)) || 
           (dynamic_cast<AttackAction*>(action)))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float KarazhanShadeOfAranMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!boss)
    {
        return 1.0f;
    }

    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION)) 
    {
        if (IsChargeAction(action))
        {
            return 0.0f;
        }

        if (dynamic_cast<MovementAction*>(action))
        {
            const float safeDistance = 20.0f;
            if (bot->GetDistance2d(boss) >= safeDistance)
            {
                return 0.0f;
            }
        }
    }

    bool flameWreathActive = boss->HasAura(SPELL_FLAME_WREATH);
    if (!flameWreathActive && bot->GetGroup())
    {
        for (GroupReference* itr = bot->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->HasAura(SPELL_AURA_FLAME_WREATH))
            {
                flameWreathActive = true;
                break;
            }
        }
    }
    if (flameWreathActive)
    {
        if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float KarazhanNetherspiteBlueAndGreenBeamMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<CastKillingSpreeAction*>(action))
    {
        return 0.0f;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker /*unused*/, greenBlocker, blueBlocker] = karazhanHelper.GetCurrentBeamBlockers();
    bool isBlocker = (bot == greenBlocker || bot == blueBlocker);
    if (isBlocker)
    {
        Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
        Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);
        bool inBeam = false;
        for (Unit* portal : {bluePortal, greenPortal}) 
        {
            if (!portal)
            {
                continue;
            }
            float bx = boss->GetPositionX(), by = boss->GetPositionY();
            float px = portal->GetPositionX(), py = portal->GetPositionY();
            float dx = px - bx, dy = py - by;
            float length = sqrt(dx*dx + dy*dy);
            if (length == 0.0f)
            {
                continue;
            }
            dx /= length; dy /= length;
            float botdx = bot->GetPositionX() - bx, botdy = bot->GetPositionY() - by;
            float t = (botdx * dx + botdy * dy);
            float beamX = bx + dx * t, beamY = by + dy * t;
            float distToBeam = sqrt(pow(bot->GetPositionX() - beamX, 2) + pow(bot->GetPositionY() - beamY, 2));
            if (distToBeam < 0.3f && t > 0.0f && t < length) 
            {
                inBeam = true;
                break;
            }
        }
        if (inBeam)
        {
            std::vector<Unit*> voidZones = karazhanHelper.GetAllVoidZones();
            bool inVoidZone = false;
            for (Unit* vz : voidZones) 
            {
                if (bot->GetExactDist2d(vz) < 4.0f)
                {
                    inVoidZone = true;
                    break;
                }
            }
            if (!inVoidZone)
            {
                if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
                {
                    return 0.0f;
                }
            }
        }
    }

    return 1.0f;
}

float KarazhanNetherspiteRedBeamMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    RaidKarazhanHelpers karazhanHelper(botAI);
    auto [redBlocker, greenBlocker /*unused*/, blueBlocker /*unused*/] = karazhanHelper.GetCurrentBeamBlockers();
    static std::map<ObjectGuid, uint32> beamMoveTimes;
    static std::map<ObjectGuid, bool> lastBeamMoveSideways;
    ObjectGuid botGuid = bot->GetGUID();
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    if (bot == redBlocker && boss && redPortal)
    {
        Position blockingPos = karazhanHelper.GetPositionOnBeam(boss, redPortal, 18.0f);
        float bx = boss->GetPositionX();
        float by = boss->GetPositionY();
        float px = redPortal->GetPositionX();
        float py = redPortal->GetPositionY();
        float dx = px - bx;
        float dy = py - by;
        float length = sqrt(dx*dx + dy*dy);
        if (length != 0.0f)
        {
            dx /= length;
            dy /= length;
            float perpDx = -dy;
            float perpDy = dx;
            Position sidewaysPos(blockingPos.GetPositionX() + perpDx * 3.0f,
                                 blockingPos.GetPositionY() + perpDy * 3.0f,
                                 blockingPos.GetPositionZ());

            uint32 intervalSecs = 5;
            if (beamMoveTimes[botGuid] == 0)
            {
                beamMoveTimes[botGuid] = time(nullptr);
                lastBeamMoveSideways[botGuid] = false;
            }
            if (time(nullptr) - beamMoveTimes[botGuid] >= intervalSecs)
            {
                lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
                beamMoveTimes[botGuid] = time(nullptr);
            }
            Position targetPos = lastBeamMoveSideways[botGuid] ? sidewaysPos : blockingPos;
            float distToTarget = bot->GetExactDist2d(targetPos.GetPositionX(), targetPos.GetPositionY());
            const float positionTolerance = 1.5f;
            if (distToTarget < positionTolerance)
            {
                if (dynamic_cast<MovementAction*>(action) || IsChargeAction(action))
                {
                    return 0.0f;
                }
            }
        }
    }
    
    return 1.0f;
}

float KarazhanPrinceMalchezaarMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!boss || !boss->IsAlive())
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsMelee(bot) && bot->HasAura(SPELL_ENFEEBLE) &&
        !dynamic_cast<KarazhanPrinceMalchezaarNonTankAvoidHazardAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsRanged(bot) && bot->HasAura(SPELL_ENFEEBLE) &&
        (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<KarazhanPrinceMalchezaarNonTankAvoidHazardAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}
