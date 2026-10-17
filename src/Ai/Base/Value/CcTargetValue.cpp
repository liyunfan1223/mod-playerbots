/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CcTargetValue.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "Group.h"
#include "PlayerbotAI.h"
#include "ServerFacade.h"

class FindTargetForCcStrategy : public FindTargetStrategy
{
public:
    FindTargetForCcStrategy(PlayerbotAI* botAI, std::string const spell)
        : FindTargetStrategy(botAI), spell(spell), maxDistance(0.f)
    {
    }

public:
    void CheckAttacker(Unit* creature, ThreatManager* /*threatMgr*/) override
    {
        Player* bot = botAI->GetBot();
        if (!botAI->CanCastSpell(spell, creature))
            return;

        if (*botAI->GetAiObjectContext()->GetValue<Unit*>("rti cc target") == creature)
        {
            result = creature;
            return;
        }

        if (*botAI->GetAiObjectContext()->GetValue<Unit*>("current target") == creature)
            return;

        uint8 health = static_cast<uint8>(creature->GetHealthPct());
        if (health < sPlayerbotAIConfig.MediumHealth)
            return;

        float minDistance = botAI->GetRange("spell");
        Group* group = bot->GetGroup();
        if (!group)
            return;

        if (*botAI->GetAiObjectContext()->GetValue<uint8>("aoe count") > 2)
        {
            WorldLocation aoe = *botAI->GetAiObjectContext()->GetValue<WorldLocation>("aoe position");
            if (ServerFacade::instance().IsDistanceLessOrEqualThan(
                    ServerFacade::instance().GetDistance2d(creature, aoe.GetPositionX(), aoe.GetPositionY()),
                    sPlayerbotAIConfig.AoeRadius))
                return;
        }

        uint32 tankCount = 0;
        uint32 dpsCount = 0;
        GetPlayerCount(creature, &tankCount, &dpsCount);
        if (!tankCount || !dpsCount)
        {
            result = creature;
            return;
        }

        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* member = ObjectAccessor::FindPlayer(itr->guid);
            if (!member || !member->IsAlive() || member == bot)
                continue;

            if (!PlayerbotAI::IsTank(member))
                continue;

            float distance = ServerFacade::instance().GetDistance2d(member, creature);
            if (distance < minDistance)
                minDistance = distance;
        }

        if (!result || minDistance > maxDistance)
        {
            result = creature;
            maxDistance = minDistance;
        }
    }

private:
    std::string const spell;
    float maxDistance;
};

Unit* CcTargetValue::Calculate()
{
    FindTargetForCcStrategy strategy(botAI, qualifier);
    return FindTarget(&strategy);
}
