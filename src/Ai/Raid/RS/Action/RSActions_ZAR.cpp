/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NearestNpcsValue.h"
#include "Playerbots.h"
#include "RSActions.h"
#include <limits>
#include <vector>

bool RsZarithrianAddsAction::Execute(Event )
{
    context->GetValue<std::string>("rti")->Set("skull");

    if (!RsIsDesignatedMarker(bot))
        return false;

    RsUpdateSkullMarker(bot, FindPriorityAdd());
    return false;
}

Unit* RsZarithrianAddsAction::FindPriorityAdd()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* boss = AI_VALUE2(Unit*, "find target", "general zarithrian");

    Unit* current = botAI->GetUnit(group->GetTargetIcon(RtiTargetValue::skullIndex));
    if (current && current->IsAlive() && current->GetEntry() == NPC_ONYX_FLAMECALLER)
        return current;

    constexpr float markRange = 20.0f;
    Unit* best = nullptr;
    GuidVector const targets = AI_VALUE(GuidVector, "possible targets no los");
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_ONYX_FLAMECALLER)
            continue;

        if (boss && unit->GetExactDist2d(boss) > markRange)
            continue;

        if (!best || unit->GetGUID() < best->GetGUID())
            best = unit;
    }

    return best ? best : boss;
}

uint32 RsZarithrianTankAction::GetCleaveStacks(Unit* unit)
{
    if (!unit)
        return 0;

    Aura* aura = botAI->GetAura("cleave armor", unit, false, true);
    return aura ? aura->GetStackAmount() : 0;
}

Unit* RsZarithrianTankAction::FindBoss()
{
    return RsFindTarget(botAI, [](Unit* unit) { return unit->GetEntry() == NPC_GENERAL_ZARITHRIAN; });
}

void RsZarithrianTankAction::CollectAdds(std::vector<Unit*>& adds)
{
    RsCollectTargets(botAI, adds, [](Unit* unit) { return unit->GetEntry() == NPC_ONYX_FLAMECALLER; });
}

bool RsZarithrianTankAction::HoldBoss(Unit* boss)
{
    bot->SetTarget(boss->GetGUID());

    if (boss->GetVictim() != bot)
        RsCastClassTaunt(botAI, bot, boss);

    bool const onSpot = bot->GetExactDist2d(RS_ZARITHRIAN_TANK_POSITION.GetPositionX(),
                                            RS_ZARITHRIAN_TANK_POSITION.GetPositionY()) <= 3.0f;

    if (!bot->IsWithinMeleeRange(boss) && onSpot)
        return MoveTo(bot->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

    if (!onSpot)
        return MoveTo(bot->GetMapId(), RS_ZARITHRIAN_TANK_POSITION.GetPositionX(),
                      RS_ZARITHRIAN_TANK_POSITION.GetPositionY(), RS_ZARITHRIAN_TANK_POSITION.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool RsZarithrianTankAction::RunAddsToBoss(Unit* boss, std::vector<Unit*> const& adds)
{
    for (Unit* add : adds)
    {
        if (!add || !add->IsAlive())
            continue;

        if (add->GetVictim() != bot)
            RsCastClassTaunt(botAI, bot, add);
    }

    Unit* nearest = nullptr;
    float best = std::numeric_limits<float>::max();
    for (Unit* add : adds)
    {
        if (!add || !add->IsAlive())
            continue;
        float const d = bot->GetExactDist2d(add);
        if (d < best)
        {
            best = d;
            nearest = add;
        }
    }
    Unit* meleeTarget = nearest ? nearest : boss;
    bot->SetTarget(meleeTarget->GetGUID());

    bool const onSpot = bot->GetExactDist2d(RS_ZARITHRIAN_TANK_POSITION.GetPositionX(),
                                            RS_ZARITHRIAN_TANK_POSITION.GetPositionY()) <= 3.0f;

    if (!bot->IsWithinMeleeRange(meleeTarget) && onSpot)
        return MoveTo(bot->GetMapId(), meleeTarget->GetPositionX(), meleeTarget->GetPositionY(),
                      meleeTarget->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT);

    if (!onSpot)
        return MoveTo(bot->GetMapId(), RS_ZARITHRIAN_TANK_POSITION.GetPositionX(),
                      RS_ZARITHRIAN_TANK_POSITION.GetPositionY(), RS_ZARITHRIAN_TANK_POSITION.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool RsZarithrianTankAction::Execute(Event )
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = FindBoss();
    if (!boss)
        return false;

    if (RsReleaseIfFollowing(bot))
        return false;

    Player* mainTankPlayer = nullptr;
    Player* assistTankPlayer = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (!mainTankPlayer && PlayerbotAI::IsMainTank(member))
                mainTankPlayer = member;

            if (!assistTankPlayer && PlayerbotAI::IsAssistTank(member))
                assistTankPlayer = member;

            if (mainTankPlayer && assistTankPlayer)
                break;
        }
    }

    std::vector<Unit*> adds;
    CollectAdds(adds);

    if (!mainTankPlayer || !assistTankPlayer || mainTankPlayer == assistTankPlayer)
    {
        for (Unit* add : adds)
        {
            if (add && add->IsAlive() && add->GetVictim() != bot)
                RsCastClassTaunt(botAI, bot, add);
        }
        return HoldBoss(boss);
    }

    if (GetCleaveStacks(bot) >= RS_ZARITHRIAN_CLEAVE_SWAP_STACKS)
    {

        if (boss->GetVictim() == bot)
            bot->AttackStop();

        return RunAddsToBoss(boss, adds);
    }

    Player* bossVictimTank = nullptr;
    if (Unit* victim = boss->GetVictim())
    {
        if (victim == mainTankPlayer || victim == assistTankPlayer)
            bossVictimTank = victim->ToPlayer();
    }

    if (bossVictimTank == bot)
        return HoldBoss(boss);

    bool const holderShredded =
        bossVictimTank && GetCleaveStacks(bossVictimTank) >= RS_ZARITHRIAN_CLEAVE_SWAP_STACKS;
    if (!bossVictimTank || holderShredded)
    {
        Player* takeOverTank = mainTankPlayer;
        if (holderShredded)
            takeOverTank = (bossVictimTank == mainTankPlayer) ? assistTankPlayer : mainTankPlayer;

        if (bot == takeOverTank)
            return HoldBoss(boss);
    }

    return RunAddsToBoss(boss, adds);
}
