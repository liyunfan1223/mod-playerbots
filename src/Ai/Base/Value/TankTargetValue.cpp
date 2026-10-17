/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TankTargetValue.h"
#include "AiObjectContext.h"
#include "AttackersValue.h"
#include "Group.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Strategy.h"

class FindTargetForTankStrategy : public FindNonCcTargetStrategy
{
public:
    FindTargetForTankStrategy(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI), minThreat(0) {}

    void CheckAttacker(Unit* creature, ThreatManager* threatMgr) override
    {
        if (!creature || !creature->IsAlive())
            return;

        Player* bot = botAI->GetBot();
        float threat = threatMgr->GetThreat(bot);
        if (!result)
        {
            minThreat = threat;
            result = creature;
        }
        // neglect if victim is main tank, or no victim (for untauntable target)
        if (Unit* victim = threatMgr->GetCurrentVictim())
        {
            if (victim->ToPlayer() && PlayerbotAI::IsMainTank(victim->ToPlayer()))
                return;
        }
        if (minThreat >= threat)
        {
            minThreat = threat;
            result = creature;
        }
    }

protected:
    float minThreat;
};

class FindTankTargetSmartStrategy : public FindTargetStrategy
{
public:
    FindTankTargetSmartStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) {}

    void CheckAttacker(Unit* attacker, ThreatManager* /*threatMgr*/) override
    {
        if (Group* group = botAI->GetBot()->GetGroup())
        {
            ObjectGuid guid = group->GetTargetIcon(4);
            if (guid && attacker->GetGUID() == guid)
                return;
        }

        if (!attacker->IsAlive())
            return;

        if (!result || IsBetter(attacker, result))
            result = attacker;
    }
    bool IsBetter(Unit* new_unit, Unit* old_unit)
    {
        Player* bot = botAI->GetBot();
        // if group has multiple tanks, explicit main tank just focus on the current target
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (currentTarget && PlayerbotAI::IsExplicitMainTank(bot) && PlayerbotAI::GetGroupTankNum(bot) > 1)
        {
            if (old_unit == currentTarget)
                return false;

            if (new_unit == currentTarget)
                return true;
        }
        float new_threat = new_unit->GetThreatMgr().GetThreat(bot);
        float old_threat = old_unit->GetThreatMgr().GetThreat(bot);
        float new_dis = bot->GetDistance(new_unit);
        float old_dis = bot->GetDistance(old_unit);
        // hasAggro? -> withinMelee? -> threat
        if (GetIntervalLevel(new_unit) != GetIntervalLevel(old_unit))
            return GetIntervalLevel(new_unit) > GetIntervalLevel(old_unit);

        int32_t interval = GetIntervalLevel(new_unit);
        if (interval == 2)
            return new_dis < old_dis;

        return new_threat < old_threat;
    }
    int32_t GetIntervalLevel(Unit* unit)
    {
        if (!botAI->HasAggro(unit))
            return 2;

        if (botAI->GetBot()->IsWithinMeleeRange(unit))
            return 1;

        return 0;
    }
};

Unit* TankTargetValue::Calculate()
{
    GuidSet const exclusions = GatherStrategyTargetExclusions(botAI, TargetValueExclusionType::TankTarget);
    std::string const rti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    Unit* rtiTarget = RtiTargetValue::Calculate();
    if (rtiTarget && exclusions.find(rtiTarget->GetGUID()) == exclusions.end())
    {
        Unit* victim = rtiTarget->GetVictim();

        if (victim && victim != bot)
        {
            if (Player* victimPlayer = victim->ToPlayer())
            {
                // rti target is attacking a non-tank player
                if (!PlayerbotAI::IsTank(victimPlayer))
                    return rtiTarget;
                // rti target is attacking a tank player, check if the tank is a bot and has the same rti setting
                PlayerbotAI* victimBotAI = GET_PLAYERBOT_AI(victimPlayer);
                if (!victimBotAI || victimBotAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rti)
                    return rtiTarget;
            }
        }
    }

    // FindTargetForTankStrategy strategy(botAI);
    FindTankTargetSmartStrategy strategy(botAI);
    return FindTarget(&strategy, exclusions);
}
