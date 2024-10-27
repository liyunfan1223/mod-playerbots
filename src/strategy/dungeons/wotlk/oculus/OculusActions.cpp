#include "Playerbots.h"
#include "OculusActions.h"
#include "OculusStrategy.h"
#include "LastSpellCastValue.h"

bool AvoidUnstableSphereAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "drakos the interrogator");
    if (!boss) { return false; }

    float radius = 12.0f;
    float extraDistance = 1.0f;
    Unit* closestSphere = nullptr;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_UNSTABLE_SPHERE && !unit->isMoving())
        {
            if (!closestSphere || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestSphere))
            {
                closestSphere = unit;
            }
        }
    }

    if (closestSphere && bot->GetExactDist2d(closestSphere) < radius + extraDistance)
    {
        return MoveAway(closestSphere, fmin(3.0f, bot->GetExactDist2d(closestSphere) - radius + extraDistance));
    }

    return false;
}

bool MountDrakeAction::isPossible() { return bot->GetMapId() == OCULUS_MAP_ID; }
bool MountDrakeAction::Execute(Event event)
{
    std::map<int32, int32> drakeAssignments;
    // Composition can be adjusted - both 3/1/1 and 2/2/1 are good default comps
    // {Amber, Emerald, Ruby}
    std::vector<uint8> composition = {2, 2, 1};
    // std::vector<uint8> composition = {3, 1, 1};
    int32 myIndex = botAI->GetGroupSlotIndex(bot);

    Player* master = botAI->GetMaster();
    if (!master) { return false; }
    Unit* vehicle = master->GetVehicleBase();
    if (!vehicle) { return false; }

    // Subtract the player's chosen mount type from the composition so player can play whichever they prefer
    switch (vehicle->GetEntry())
    {
        case NPC_AMBER_DRAKE:
            composition[0]--;
            break;
        case NPC_EMERALD_DRAKE:
            composition[1]--;
            break;
        case NPC_RUBY_DRAKE:
            composition[2]--;
            break;
    }

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Player* player = botAI->GetPlayer(member);
        if (!player) { continue; }

        for (int i = 0; i < composition.size(); i++)
        {
            if (composition[i] > 0)
            {
                drakeAssignments[botAI->GetGroupSlotIndex(player)] = DRAKE_ITEMS[i];
                composition[i]--;
                break;
            }
        }
    }

    // Correct/update the drake items in inventories incase assignments have changed
    for (uint32 itemId : DRAKE_ITEMS)
    {
        Item* item = bot->GetItemByEntry(itemId);
        if (!item) { continue; }

        if (itemId == drakeAssignments[myIndex])
        {
            // Use our assigned drake
            return UseItemAuto(item);
        }
        // Else assigned drake is different, destroy old drake
        uint32 count = 1;
        bot->DestroyItemCount(item, count, true);
        break;
    }

    // Bot does not have the correct drake item
    bot->AddItem(drakeAssignments[myIndex], 1);
    return false;
}

bool DismountDrakeAction::Execute(Event event)
{
    if (bot->GetVehicle())
    {
        bot->ExitVehicle();
        return true;
    }
    return false;
}

bool FlyDrakeAction::Execute(Event event)
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }
    Unit* masterVehicle = master->GetVehicleBase();
    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase || !masterVehicle) { return false; }

    MotionMaster* mm = vehicleBase->GetMotionMaster();
    Unit* boss = AI_VALUE2(Unit*, "find target", "ley-guardian eregos");
    if (boss && !boss->HasAura(SPELL_PLANAR_SHIFT))
    {
        // Handle as boss encounter instead of formation flight
        mm->Clear(false);
        float distance = vehicleBase->GetExactDist(boss);
        float range = 55.0f;    // Drake range is 60yd
        if (distance > range)
        {
            mm->MoveForwards(boss, range - distance);
            vehicleBase->SendMovementFlagUpdate();
            return true;
        }

        vehicleBase->SetFacingToObject(boss);
        mm->MoveIdle();
        vehicleBase->SendMovementFlagUpdate();
        return false;
    }

    if (vehicleBase->GetExactDist(masterVehicle) > 20.0f)
    {
        // 3/4 of a circle, with frontal cone 90 deg unobstructed
        float angle = botAI->GetGroupSlotIndex(bot) * (2*M_PI - M_PI_2)/5 + M_PI_2;
        vehicleBase->SetCanFly(true);
        mm->MoveFollow(masterVehicle, 15.0f, angle);
        vehicleBase->SendMovementFlagUpdate();
        return true;
    }
    return false;
}

bool DrakeAttackAction::Execute(Event event)
{
    vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase) { return false; }

    Unit* target = AI_VALUE(Unit*, "current target");

    if (!target)
    {
        GuidVector attackers = AI_VALUE(GuidVector, "attackers");
        for (auto& attacker : attackers)
        {
            Unit* unit = botAI->GetUnit(attacker);
            if (!unit) { continue; }

            SET_AI_VALUE(Unit*, "current target", unit);
            target = unit;
            break;
        }
    }

    if (!target) { return false; }

    switch (vehicleBase->GetEntry())
    {
        case NPC_AMBER_DRAKE:
            return AmberDrakeAction(target);
        case NPC_EMERALD_DRAKE:
            return EmeraldDrakeAction(target);
        case NPC_RUBY_DRAKE:
            return RubyDrakeAction(target);
        default:
            break;
    }
    return false;
}

bool DrakeAttackAction::CastDrakeSpellAction(Unit* target, uint32 spellId, uint32 cooldown)
{
    if (botAI->CanCastVehicleSpell(spellId, target))
        if (botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase->AddSpellCooldown(spellId, 0, cooldown);
            return true;
        }
    return false;
}

bool DrakeAttackAction::AmberDrakeAction(Unit* target)
{
    Aura* shockCharges = target->GetAura(SPELL_SHOCK_CHARGE, vehicleBase->GetGUID());
    if (shockCharges && shockCharges->GetStackAmount() > 8)
    {
        // At 9 charges, better to detonate and re-channel rather than stacking the last charge due to gcd
        // If stacking Amber drakes, may need to drop this even lower as the charges stack so fast
        return CastDrakeSpellAction(target, SPELL_SHOCK_LANCE, 0);
    }

    // Deal with enrage after shock charges, as Stop Time adds 5 charges and they may get wasted
    if (target->HasAura(SPELL_ENRAGED_ASSAULT) &&
        !target->HasAura(SPELL_STOP_TIME) &&
        !vehicleBase->HasSpellCooldown(SPELL_STOP_TIME))
    {
        return CastDrakeSpellAction(target, SPELL_STOP_TIME, 60000);
    }

    if (!vehicleBase->FindCurrentSpellBySpellId(SPELL_TEMPORAL_RIFT))
    {
        return CastDrakeSpellAction(target, SPELL_TEMPORAL_RIFT, 0);
    }

    return false;
}

bool DrakeAttackAction::EmeraldDrakeAction(Unit* target)
{
    Aura* poisonStacks = target->GetAura(SPELL_LEECHING_POISON, vehicleBase->GetGUID());
    if (!poisonStacks || (poisonStacks->GetStackAmount() < 3 ||
                         poisonStacks->GetDuration() < 4000))
    {
        return CastDrakeSpellAction(target, SPELL_LEECHING_POISON, 0);
    }

    if (!vehicleBase->HasSpellCooldown(SPELL_TOUCH_THE_NIGHTMARE) &&
        (!target->HasAura(SPELL_TOUCH_THE_NIGHTMARE) || vehicleBase->HealthAbovePct(90)))
    {
        return CastDrakeSpellAction(target, SPELL_TOUCH_THE_NIGHTMARE, 10000);
    }

    Unit* healingTarget = nullptr;
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
        {
            continue;
        }

        Unit* drake = unit->GetVehicleBase();
        if (!drake || drake->IsFullHealth()) { continue; }

        if (!healingTarget || drake->GetHealthPct() < healingTarget->GetHealthPct() - 15.0f)
        {
            healingTarget = drake;
        }
    }

    Spell* currentSpell = vehicleBase->FindCurrentSpellBySpellId(SPELL_DREAM_FUNNEL);
    if (healingTarget)
    {
        if (!currentSpell || currentSpell->m_targets.GetUnitTarget() != healingTarget)
        {
            float distance = vehicleBase->GetExactDist(healingTarget);
            float range = 55.0f;
            if (distance > range)
            {
                MotionMaster* mm = vehicleBase->GetMotionMaster();
                mm->Clear(false);
                mm->MoveForwards(healingTarget, distance - range - 10.0f);
                vehicleBase->SendMovementFlagUpdate();
                return false;
            }
            return CastDrakeSpellAction(healingTarget, SPELL_DREAM_FUNNEL, 0);
        }
    }
    // Fill GCDs with Leeching Poison to refresh timer, rather than idling
    if (!currentSpell)
    {
        return CastDrakeSpellAction(target, SPELL_LEECHING_POISON, 0);
    }

    return false;
}

bool DrakeAttackAction::RubyDrakeAction(Unit* target)
{
    Aura* evasiveCharges = vehicleBase->GetAura(SPELL_EVASIVE_CHARGES);
    Aura* evasiveManeuvers = vehicleBase->GetAura(SPELL_EVASIVE_MANEUVERS);

    if (evasiveCharges)
    {
        if (evasiveManeuvers &&
            !vehicleBase->HasSpellCooldown(SPELL_MARTYR) &&
            evasiveManeuvers->GetDuration() > 10000 &&
            evasiveCharges->GetStackAmount() >= 5)
        {
            return CastDrakeSpellAction(vehicleBase, SPELL_MARTYR, 10000);
        }

        if (!vehicleBase->HasSpellCooldown(SPELL_EVASIVE_MANEUVERS) &&
            evasiveCharges->GetStackAmount() >= 10)
        {
            return CastDrakeSpellAction(vehicleBase, SPELL_EVASIVE_MANEUVERS, 5000);
        }
    }

    return CastDrakeSpellAction(target, SPELL_SEARING_WRATH, 0);
}

bool AvoidArcaneExplosionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "mage-lord urom");
    if (!boss) { return false; }

    const Position* closestPos = nullptr;

    for (auto& position : uromSafePositions)
    {
        if (!closestPos || bot->GetExactDist(position) < bot->GetExactDist(closestPos))
            {
                closestPos = &position;
            }
    }

    if (!closestPos) { return false; }
    
    return MoveNear(bot->GetMapId(), closestPos->GetPositionX(), closestPos->GetPositionY(), closestPos->GetPositionZ(), 2.0f, MovementPriority::MOVEMENT_COMBAT);
}

bool TimeBombSpreadAction::Execute(Event event)
{
    float radius = 10.0f;
    float distanceExtra = 2.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }

        Unit* unit = botAI->GetUnit(member);
        if (unit && bot->GetExactDist2d(unit) < radius)
        {
            return MoveAway(unit, radius + distanceExtra - bot->GetExactDist2d(unit));
        }
    }
    return false;
}
