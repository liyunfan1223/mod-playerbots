#include "RaidGruulsLairMultipliers.h"
#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "WarriorActions.h"

using namespace GruulsLairHelpers;

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float HighKingMaulgarMultiplier::GetValue(Action* action)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (IsAnyOgreBossAlive(botAI) && dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }
    
    if (maulgar && maulgar->HasAura(static_cast<uint32>(GruulsLairSpells::WHIRLWIND)) &&
        (!kiggler || !kiggler->IsAlive()) &&
        (!krosh || !krosh->IsAlive()) &&
        (!olm || !olm->IsAlive()) &&
        (!blindeye || !blindeye->IsAlive()))
    {
        if (IsChargeAction(action) || (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<HighKingMaulgarWhirlwindRunAwayAction*>(action)))
        {
            return 0.0f;
        }
    }

    Unit* target = AI_VALUE(Unit*, "current target");
    if (krosh && target && target->GetGUID() == krosh->GetGUID() && dynamic_cast<CastArcaneShotAction*>(action))
    {
        return 0.0f;
    }

    if (IsKroshMageTank(botAI, bot) && 
        (dynamic_cast<CastFrostNovaAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
        dynamic_cast<CastConeOfColdAction*>(action) || dynamic_cast<CastFlamestrikeAction*>(action) ||
        dynamic_cast<CastDragonsBreathAction*>(action) || dynamic_cast<CastBlastWaveAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerMultiplier::GetValue(Action* action)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul) 
    {
        return 1.0f;
    }

    if (bot->HasAura(static_cast<uint32>(GruulsLairSpells::GROUND_SLAM_1)) || 
        bot->HasAura(static_cast<uint32>(GruulsLairSpells::GROUND_SLAM_2)))
    {
        if ((dynamic_cast<MovementAction*>(action) && !dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action)) ||
            IsChargeAction(action))
        {
            return 0.0f;
        }
    }

    const TankSpot& tankSpot = GruulsLairTankSpots::Gruul;
    const float positionThreshold = 3.0f;
    const float orientationLeeway = 30.0f * M_PI / 180.0f;

    float distanceToTankSpot = bot->GetExactDist2d(tankSpot.x, tankSpot.y);
    float desiredOrientation = atan2(gruul->GetPositionY() - bot->GetPositionY(), gruul->GetPositionX() - bot->GetPositionX());
    float currentOrientation = bot->GetOrientation();
    float delta = desiredOrientation - currentOrientation;
    while (delta > M_PI) delta -= 2 * M_PI;
    while (delta < -M_PI) delta += 2 * M_PI;
    float orientationDifference = fabs(delta);

    if (botAI->IsMainTank(bot) && gruul->GetVictim() == bot &&
        distanceToTankSpot < positionThreshold && orientationDifference < orientationLeeway)
    {
        if (dynamic_cast<MovementAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}
