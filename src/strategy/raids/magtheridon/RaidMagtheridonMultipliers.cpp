#include <unordered_map>
#include <ctime>

#include "RaidMagtheridonMultipliers.h"
#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "WarlockActions.h"

using namespace MagtheridonHelpers;
static std::unordered_map<uint32, time_t> magtheridonAggroWaitTimer;
static std::unordered_map<uint32, bool> lastAggroShadowCageState;

float MagtheridonMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (magtheridon && !magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)))
    {
        if (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
            magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagtheridonSpells::BLAST_NOVA)))
        {
            auto it = botToCubeAssignment.find(bot->GetGUID());
            if (it != botToCubeAssignment.end())
            {
                if (dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
                {
                    return 1.0f;
                }

                return 0.0f;
            }
        }
    }

    if (channeler && channeler->IsAlive() && 
       (dynamic_cast<CastCurseOfAgonyAction*>(action) ||
        dynamic_cast<CastCurseOfAgonyOnAttackerAction*>(action) ||
        dynamic_cast<CastCurseOfDoomAction*>(action) ||
        dynamic_cast<CastCurseOfWeaknessAction*>(action) ||
        dynamic_cast<CastCurseOfTheElementsAction*>(action) ||
        dynamic_cast<CastCurseOfExhaustionAction*>(action)))
    {
        return 0.0f;
    }

    if ((botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1)) && 
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    if (magtheridon)
    {
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)),
                              lastAggroShadowCageState, magtheridonAggroWaitTimer);

        const int aggroWaitSeconds = 8;
        auto it = magtheridonAggroWaitTimer.find(bot->GetMapId());
        if (it != magtheridonAggroWaitTimer.end())
        {
            time_t since = time(nullptr) - it->second;
            if (since < aggroWaitSeconds)
            {
                if (!botAI->IsMainTank(bot) && dynamic_cast<AttackAction*>(action))
                {
                    return 0.0f;
                }
            }
        }

        const TankSpot& tankSpot = MagtheridonTankSpots::Magtheridon;
        const float positionThreshold = 3.0f;
        const float orientationLeeway = 30.0f * M_PI / 180.0f;

        float distanceToTankSpot = bot->GetExactDist2d(tankSpot.x, tankSpot.y);
        float desiredOrientation = atan2(magtheridon->GetPositionY() - bot->GetPositionY(), magtheridon->GetPositionX() - bot->GetPositionX());
        float currentOrientation = bot->GetOrientation();
        float delta = desiredOrientation - currentOrientation;
        while (delta > M_PI) delta -= 2 * M_PI;
        while (delta < -M_PI) delta += 2 * M_PI;
        float orientationDifference = fabs(delta);

        if (botAI->IsMainTank(bot) && magtheridon && magtheridon->GetVictim() == bot &&
            distanceToTankSpot < positionThreshold && orientationDifference < orientationLeeway)
        {
            if (dynamic_cast<MovementAction*>(action))
            {
                return 0.0f;
            }
        }
    }

    return 1.0f;
}
