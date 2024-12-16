#include "RaidIccActions.h"
#include "RaidIccStrategy.h"
#include "Playerbots.h"
#include "Timer.h"
#include "Vehicle.h"

enum CreatureIds {
    NPC_KOR_KRON_BATTLE_MAGE                    = 37117,
    NPC_KOR_KRON_AXETHROWER                     = 36968,
    NPC_KOR_KRON_ROCKETEER                      = 36982,

    NPC_SKYBREAKER_SORCERER                     = 37116,
    NPC_SKYBREAKER_RIFLEMAN                     = 36969,
    NPC_SKYBREAKER_MORTAR_SOLDIER               = 36978,

    NPC_IGB_HIGH_OVERLORD_SAURFANG              = 36939,
    NPC_IGB_MURADIN_BRONZEBEARD                 = 36948,
};

const std::vector<uint32> availableTargets = {
    NPC_KOR_KRON_AXETHROWER,        NPC_KOR_KRON_ROCKETEER,     NPC_KOR_KRON_BATTLE_MAGE,
    NPC_IGB_HIGH_OVERLORD_SAURFANG, NPC_SKYBREAKER_RIFLEMAN,    NPC_SKYBREAKER_MORTAR_SOLDIER,
    NPC_SKYBREAKER_SORCERER,        NPC_IGB_MURADIN_BRONZEBEARD
};

//Lord Marrowgwar
bool IccLmTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
    {
        if (bot->GetExactDist2d(ICC_LM_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_LM_TANK_POSITION.GetPositionX(),
                          ICC_LM_TANK_POSITION.GetPositionY(), ICC_LM_TANK_POSITION.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_NORMAL);
        else
            return Attack(boss);
    }
    return Attack(boss);
}

bool IccSpikeAction::Execute(Event event)
{
    // If we're impaled, we can't do anything
    if (bot->HasAura(69065) || // Impaled (10N)
        bot->HasAura(72669) || // Impaled (25N)
        bot->HasAura(72670) || // Impaled (10H)
        bot->HasAura(72671))   // Impaled (25H)
    {
        return false;
    }
    // Find the bos
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss) { return false; }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    Unit* spikeTarget = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && (unit->GetEntry() == 36619 ||
                    unit->GetEntry() == 38711 || 
                    unit->GetEntry() == 38712))
        {
            spikeTarget = unit;
            break;
        }
    }

    // Prioritize attacking a bone spike if found
    if (spikeTarget)
    {
        if (currentTarget != spikeTarget)
        {
            if (Attack(spikeTarget))
            {
                return Attack(spikeTarget);
            }
        }
        return Attack(spikeTarget); // Already attacking a spike
    }

    // No bone spikes found, attack boss if not already targeting
    if (currentTarget != boss)
    {
        if (Attack(boss))
        {
            return Attack(boss);
        }
    }

    return false;
}

//Lady
bool IccDarkReckoningAction::Execute(Event event)
{
    if (bot->HasAura(69483) && bot->GetExactDist2d(ICC_DARK_RECKONING_SAFE_POSITION) > 2.0f) //dark reckoning spell id
    {
        botAI->Yell("Dark Reckoning - moving to safe position");
        return MoveTo(bot->GetMapId(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionX(),
                      ICC_DARK_RECKONING_SAFE_POSITION.GetPositionY(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    return false;
}

bool IccAddsLadyDeathwhisperAction::Execute(Event event)
{
    if (botAI->IsMainTank(bot) || botAI->IsHeal(bot))
    {
        return false;
    }
    // Find the boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss) { return false; }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    Unit* add = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && (unit->GetEntry() == 37949 || //cult adherent
                    unit->GetEntry() == 38394 || 
                    unit->GetEntry() == 38625 ||
                    unit->GetEntry() == 38626 ||
                    unit->GetEntry() == 38010 ||
                    unit->GetEntry() == 38397 || 
                    unit->GetEntry() == 39000 || 
                    unit->GetEntry() == 39001 || 
                    unit->GetEntry() == 38136 ||
                    unit->GetEntry() == 38396 ||
                    unit->GetEntry() == 38632 ||
                    unit->GetEntry() == 38633 ||
                    unit->GetEntry() == 37890 || //cult fanatic
                    unit->GetEntry() == 38393 ||
                    unit->GetEntry() == 38628 ||
                    unit->GetEntry() == 38629 ||
                    unit->GetEntry() == 38135 ||
                    unit->GetEntry() == 38395 ||
                    unit->GetEntry() == 38634 ||
                    unit->GetEntry() == 38009 ||
                    unit->GetEntry() == 38398 ||
                    unit->GetEntry() == 38630 ||
                    unit->GetEntry() == 38631))
        {
            add = unit;
            break;
        }
    }

    // Prioritize attacking an add if found
    if (add)
    {
        if (currentTarget != add)
        {
            if (Attack(add))
            {
                return Attack(add);
            }
        }
        return Attack(add); // Already attacking an add
    }

    // No adds found, attack boss if not already targeting
    if (currentTarget != boss)
    {
        if (Attack(boss))
        {
            return Attack(boss);
        }
    }

    return false;
}

bool IccShadeLadyDeathwhisperAction::Execute(Event event)
{
    const float radius = 12.0f;

    // Get the nearest hostile NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == 38222) //vengeful sahde ID
        {
            float currentDistance = bot->GetDistance2d(unit);
            // Move away from the Vengeful Shade if the bot is too close
            if (currentDistance < radius)
            {
                return MoveAway(unit, radius - currentDistance);
            }
        }
    }
    return false;
}

bool IccRottingFrostGiantTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotting frost giant");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
    {
        if (bot->GetExactDist2d(ICC_ROTTING_FROST_GIANT_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionX(),
                          ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionY(), ICC_ROTTING_FROST_GIANT_TANK_POSITION.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_NORMAL);
        else
            return Attack(boss);
    }

    float radius = 5.0f;
    float distanceExtra = 2.0f;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }

        Unit* unit = botAI->GetUnit(member);
        if ((botAI->IsHeal(bot) || botAI->IsDps(bot)) && bot->GetExactDist2d(unit) < radius)
        {
            return MoveAway(unit, radius + distanceExtra - bot->GetExactDist2d(unit));
        }
    }
    return Attack(boss);
}

//Gunship
bool IccCannonFireAction::Execute(Event event)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    GuidVector attackers = AI_VALUE(GuidVector, "possible targets no los");

    Unit* target = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;
        for (uint32 entry : availableTargets)
        {
            if (unit->GetEntry() == entry) {
                target = unit;
                break;
            }
        }
        if (target)
            break;
    }
    if (!target)
        return false;

    if  (vehicleBase->GetPower(POWER_ENERGY) >= 90) {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "incinerating blast");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target)) {
            vehicleBase->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    }
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "cannon blast");
    if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target)) {
        vehicleBase->AddSpellCooldown(spellId, 0, 1000);
        return true;
    }
    return false;
}

bool IccGunshipEnterCannonAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    Unit* vehicleToEnter = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;
        
        if (!vehicleBase->IsFriendlyTo(bot))
            continue;
        
        if (!vehicleBase->GetVehicleKit() || !vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        uint32 entry = vehicleBase->GetEntry();
        if (entry != 36838 && entry != 36839)
            continue;
        
        if (vehicleBase->HasAura(69704) || vehicleBase->HasAura(69705))
            continue;
        
        if (!vehicleToEnter || bot->GetExactDist(vehicleToEnter) > bot->GetExactDist(vehicleBase))
            vehicleToEnter = vehicleBase;
    }

    if (!vehicleToEnter)
        return false;
    
    if (EnterVehicle(vehicleToEnter, true))
        return true;

    return false;
}

bool IccGunshipEnterCannonAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = bot->GetDistance(vehicleBase);
    
    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);


    botAI->RemoveShapeshift();

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool IccGunshipTeleportAllyAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kor'kron battle-mage");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());
    // Check if the bot is targeting a valid boss before teleporting
    if (bot->GetTarget() != boss->GetGUID())
        return false;

    if (bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_ALLY) > 15.0f)
        return bot->TeleportTo(bot->GetMapId(), ICC_GUNSHIP_TELEPORT_ALLY.GetPositionX(),
                      ICC_GUNSHIP_TELEPORT_ALLY.GetPositionY(), ICC_GUNSHIP_TELEPORT_ALLY.GetPositionZ(), bot->GetOrientation());
    else
        return Attack(boss);
}

bool IccGunshipTeleportHordeAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "skybreaker sorcerer");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());
    // Check if the bot is targeting a valid boss before teleporting
    if (bot->GetTarget() != boss->GetGUID())
        return false;

    if (bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_HORDE) > 15.0f)
        return bot->TeleportTo(bot->GetMapId(), ICC_GUNSHIP_TELEPORT_HORDE.GetPositionX(),
                      ICC_GUNSHIP_TELEPORT_HORDE.GetPositionY(), ICC_GUNSHIP_TELEPORT_HORDE.GetPositionZ(), bot->GetOrientation());
    else
        return Attack(boss);
}

//DBS
bool IccDbsTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
    {
        if (bot->GetExactDist2d(ICC_DBS_TANK_POSITION) > 5.0f)
            return MoveTo(bot->GetMapId(), ICC_DBS_TANK_POSITION.GetPositionX(),
                          ICC_DBS_TANK_POSITION.GetPositionY(), ICC_DBS_TANK_POSITION.GetPositionZ(), false,
                          false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
    float radius = 10.0f;
    Unit* closestMember = nullptr;
    GuidVector members = AI_VALUE(GuidVector, "group members");
        
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
                continue;

        if (botAI->IsAssistTank(bot))
            continue;

        if (!closestMember || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestMember))
            closestMember = unit;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < radius)
        return MoveAway(closestMember, 2.0f);
    }

    return false;
}

bool IccAddsDbsAction::Execute(Event event)
{
    if (botAI->IsHeal(bot))
    {
        return false;
    }
    // Find the boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss) { return false; }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    Unit* add = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && (unit->GetEntry() == 38508 || //blood beast
                    unit->GetEntry() == 38596 || 
                    unit->GetEntry() == 38597 || 
                    unit->GetEntry() == 38598))
        {
            add = unit;
            break;
        }
    }

    // Prioritize attacking an add if found
    if (add && !botAI->IsHeal(bot))
    {
        if (currentTarget != add)
        {
            if (Attack(add))
            {
                return Attack(add);
            }
        }
        return Attack(add); // Already attacking an add
    }

    // No adds found, attack boss if not already targeting
    if (currentTarget != boss)
    {
        if (Attack(boss))
        {
            return Attack(boss);
        }
    }

    return false;
}
 //FESTERGUT
bool IccFestergutTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
    return false;

    bot->SetTarget(boss->GetGUID());
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
    {
        if (bot->GetExactDist2d(ICC_FESTERGUT_TANK_POSITION) > 5.0f)
        return MoveTo(bot->GetMapId(), ICC_FESTERGUT_TANK_POSITION.GetPositionX(),
                      ICC_FESTERGUT_TANK_POSITION.GetPositionY(), ICC_FESTERGUT_TANK_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    float radius = 10.0f;
    Unit* closestMember = nullptr;
    GuidVector members = AI_VALUE(GuidVector, "group members");
    
    // First check if any group members have spores
    bool sporesPresent = false;
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(69279)) // gas spore
        {
            sporesPresent = true;
            break;
        }
    }

    // Only spread out if no spores are active
    if (!sporesPresent && (botAI->IsRanged(bot) || botAI->IsHeal(bot)))
    {
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || bot->GetGUID() == member)
            {
                continue;
            }
            if (!closestMember || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestMember))
            {
                closestMember = unit;
            }
        }

        if (closestMember && bot->GetExactDist2d(closestMember) < radius)
        {
            return MoveAway(closestMember, 2.0f);
        }
        return false; // Already at good distance
    }
    return false;
}

bool IccFestergutSporeAction::Execute(Event event)
{
    bool hasSpore = bot->HasAura(69279); // gas spore
    bool anySporePresent = false;
    bool meleeSporePresent = false;
    const float POSITION_TOLERANCE = 4.0f;
    const float SPREAD_RADIUS = 2.0f;  // How far apart ranged should spread

    // If bot has spore, stop attacking
    if (hasSpore)
    {
        bot->AttackStop();
    }

    // First check if we're already correctly positioned
    bool atMeleeSpot = bot->GetExactDist2d(ICC_FESTERGUT_MELEE_SPORE) < POSITION_TOLERANCE;
    
    // Calculate a unique spread position for ranged
    float angle = (bot->GetGUID().GetCounter() % 8) * (M_PI / 4); // Divide circle into 8 positions
    Position spreadRangedPos = ICC_FESTERGUT_RANGED_SPORE;
    spreadRangedPos.m_positionX += cos(angle) * SPREAD_RADIUS;
    spreadRangedPos.m_positionY += sin(angle) * SPREAD_RADIUS;
    
    bool atRangedSpot = bot->GetExactDist2d(spreadRangedPos) < POSITION_TOLERANCE;

    // Check if any spores exist and if melee spot is taken
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || unit == bot)
            continue;

        if (unit->HasAura(69279))
        {
            anySporePresent = true;
            if (unit->GetExactDist2d(ICC_FESTERGUT_MELEE_SPORE) < POSITION_TOLERANCE)
            {
                meleeSporePresent = true;
            }
        }
    }

    if (!anySporePresent)
        return false;

    // If we already have spore and are in position, stay there
    if (hasSpore && ((meleeSporePresent && atRangedSpot) || (!meleeSporePresent && atMeleeSpot)))
        return true;

    Position targetPos;
    if (hasSpore)
    {
        // If bot has spore, go to melee if spot is free, otherwise ranged
        targetPos = meleeSporePresent ? spreadRangedPos : ICC_FESTERGUT_MELEE_SPORE;
    }
    else
    {
        // If bot doesn't have spore, go to position based on role
        targetPos = botAI->IsMelee(bot) ? ICC_FESTERGUT_MELEE_SPORE : spreadRangedPos;
    }

    // Only move if we're not already at the target position
    if (bot->GetExactDist2d(targetPos) > POSITION_TOLERANCE)
    {
        return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return hasSpore;
}

//ROTFACE
bool IccRotfaceTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
    return false;

    // Tank positioning logic
    if (botAI->IsMainTank(bot))
    {
        if (bot->GetExactDist2d(ICC_ROTFACE_TANK_POSITION) > 7.0f)
        return MoveTo(bot->GetMapId(), ICC_ROTFACE_TANK_POSITION.GetPositionX(),
                      ICC_ROTFACE_TANK_POSITION.GetPositionY(), ICC_ROTFACE_TANK_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // Assist tank positioning for big ooze and small ooze management
    if (botAI->IsAssistTank(bot))
    {
        // If we have the ooze flood aura, don't try to tank anything
        if (bot->HasAura(71215))
        {
            return MoveTo(boss->GetMapId(), boss->GetPositionX() + 10.0f * cos(bot->GetAngle(boss)),
                         boss->GetPositionY() + 10.0f * sin(bot->GetAngle(boss)), bot->GetPositionZ(), false,
                         false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        Unit* bigOoze = AI_VALUE2(Unit*, "find target", "big ooze");
        Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");

        // Only care about big ooze
        if (bigOoze)
        {
            // Taunt the big ooze if it's not targeting us
            if (bigOoze->GetVictim() != bot)
            {
                if (botAI->CastSpell("taunt", bigOoze))
                    return true;
                return Attack(bigOoze);  // If taunt fails or is on cooldown, keep attacking
            }

            // If we have aggro on big ooze
            if (bigOoze->GetVictim() == bot)
            {
                // If there's a small ooze, move to it but keep attacking big ooze
                if (smallOoze)
                {
                    if (bot->GetExactDist2d(smallOoze) > 5.0f)
                    {
                        MoveTo(smallOoze->GetMapId(), smallOoze->GetPositionX(),
                              smallOoze->GetPositionY(), bot->GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                    return Attack(bigOoze);  // Keep attacking big ooze while moving
                }
                // Position handling for big ooze when no small ooze exists
                else
                {
                    float angle = boss->GetAngle(bigOoze);
                    float targetX = boss->GetPositionX() + (30.0f * cos(angle));
                    float targetY = boss->GetPositionY() + (30.0f * sin(angle));

                    if (abs(boss->GetExactDist2d(bot) - 30.0f) > 5.0f)
                    {
                        MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                    return Attack(bigOoze);
                }
            }
            
            // If big ooze exists but we don't have aggro, keep trying to get it
            return Attack(bigOoze);
        }

        // If no big ooze exists, stop all ooze-related actions and return to normal tanking behavior
        bot->AttackStop();
        return false;
    }

    return false;
}

// New action for group positioning
bool IccRotfaceGroupPositionAction::Execute(Event event)
{
    // Find Rotface
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    if (!bot->HasAura(71215) && !bot->HasAura(69789))  // ooze flood id
    {
        float radius = 10.0f;
        Unit* closestMember = nullptr;
        GuidVector members = AI_VALUE(GuidVector, "group members");
        
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || bot->GetGUID() == member)
                continue;

            if (botAI->IsAssistTank(bot))
                continue;

            if (!closestMember || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestMember))
                closestMember = unit;
        }

        if (closestMember && bot->GetExactDist2d(closestMember) < radius)
            return MoveAway(closestMember, 2.0f);
            
        return false;
    }
    
    // Ooze flood movement logic
    if (bot->HasAura(71215) || bot->HasAura(69789))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);
        
        float currentDistance = bot->GetExactDist2d(boss);
        const float DESIRED_DISTANCE = 4.0f;
        
        // Only move if we're not at the desired distance
        if (abs(currentDistance - DESIRED_DISTANCE) > 2.0f)  // 2.0f tolerance
        {
            float dx = boss->GetPositionX() - bot->GetPositionX();
            float dy = boss->GetPositionY() - bot->GetPositionY();
            float angle = atan2(dy, dx);

            // Calculate position that maintains 10 yards from boss
            float moveX = boss->GetPositionX() - (cos(angle) * DESIRED_DISTANCE);
            float moveY = boss->GetPositionY() - (sin(angle) * DESIRED_DISTANCE);
            
            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), 
                false, true, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        return true; // Stay in place if at correct distance
    }

    return false;
}

bool IccRotfaceMoveAwayFromExplosionAction::Execute(Event event)
{
    if (botAI->IsMainTank(bot) || bot->HasAura(71215))
    { return false; }

    // Stop current actions first
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    
    // Generate random angle between 0 and 2π
    float angle = frand(0, 2 * M_PI);
    
    // Calculate position 20 yards away in random direction
    float moveX = bot->GetPositionX() + 20.0f * cos(angle);
    float moveY = bot->GetPositionY() + 20.0f * sin(angle);
    float moveZ = bot->GetPositionZ();
    
    // Move to the position
    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ,
                 true,  // generatePath
                 true,  // forceDest 
                 true,  // straightLine
                 true,  // forceRecalculateSpeed
                 MovementPriority::MOVEMENT_FORCED);
}

//PP

bool IccPutricideGrowingOozePuddleAction::Execute(Event event)
{
   const float radius = 12.0f;

    // Get the nearest hostile NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == 37690) //growing ooze puddle ID
        {
            float currentDistance = bot->GetDistance2d(unit);
            // Move away from the Vengeful Shade if the bot is too close
            if (currentDistance < radius)
            {
                return MoveAway(unit, radius - currentDistance);
            }
        }
    }
    return false;
}

bool IccPutricideVolatileOozeAction::Execute(Event event)
{
    const float POSITION_TOLERANCE = 5.0f;
    const float STACK_DISTANCE = 8.0f;
    const float SAFE_DISTANCE = 20.0f;

    // 1. Main tank positioning
    if (botAI->IsMainTank(bot))
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_OOZE_POSITION.GetPositionX(),
                     ICC_PUTRICIDE_TANK_OOZE_POSITION.GetPositionY(),
                     ICC_PUTRICIDE_TANK_OOZE_POSITION.GetPositionZ(),
                     false, true, false, true, MovementPriority::MOVEMENT_NORMAL);

    // Find the ooze
    Unit* ooze = AI_VALUE2(Unit*, "find target", "volatile ooze");
    
    // If bot is melee (and not main tank), always attack ooze if it exists
    if (botAI->IsMelee(bot) && ooze)
    {
        bot->SetTarget(ooze->GetGUID());
        return Attack(ooze);
    }

    // Check for aura on any group member
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Unit* auraTarget = nullptr;
    bool anyoneHasAura = false;
    bool botHasAura = bot->HasAura(70447); // Check if this bot has the aura

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;
            
        if (member->HasAura(70447)) // Volatile Ooze Adhesive
        {
            anyoneHasAura = true;
            auraTarget = member;
            break;
        }
    }

    // For ranged and healers
    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
        // If bot has aura or someone else has aura, stack with aura target
        if (botHasAura || (anyoneHasAura && auraTarget))
        {
            Position targetPos;
            targetPos.m_positionX = auraTarget->GetPositionX();
            targetPos.m_positionY = auraTarget->GetPositionY();
            targetPos.m_positionZ = auraTarget->GetPositionZ();

            if (bot->GetExactDist2d(targetPos) > STACK_DISTANCE)
            {
                bot->AttackStop();
                return MoveTo(bot->GetMapId(), targetPos.GetPositionX(),
                            targetPos.GetPositionY(), targetPos.GetPositionZ(),
                            false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
        // If no one has aura and ooze exists, maintain safe distance
        else if (ooze)
        {
            float currentDist = bot->GetExactDist2d(ooze);
            if (abs(currentDist - SAFE_DISTANCE) > POSITION_TOLERANCE)
            {
                // Calculate position 20 yards from ooze
                float angle = ooze->GetAngle(bot);
                float moveX = ooze->GetPositionX() + (SAFE_DISTANCE * cos(angle));
                float moveY = ooze->GetPositionY() + (SAFE_DISTANCE * sin(angle));
                
                return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(),
                            false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }

        // If in position and ooze exists, attack it (except healers)
        if (ooze && !botAI->IsHeal(bot))
        {
            bot->SetTarget(ooze->GetGUID());
            return Attack(ooze);
        }
        else if (botAI->IsHeal(bot))
        {
            return false; // Allow healer to continue with normal healing actions
        }
    }

    return false;
}

bool IccPutricideGasCloudAction::Execute(Event event)
{

    if (botAI->IsMainTank(bot)) 
        return false;

    // Find the gas cloud
    Unit* gasCloud = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!gasCloud)
        return false;

    // Check if this bot has Gaseous Bloat
    bool botHasAura = bot->HasAura(70672);
    
    // Add this: Check if anyone in group has the aura
    bool someoneHasAura = false;
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member)
                continue;
                
            if (member->HasAura(70672)) // Gaseous Bloat
            {
                someoneHasAura = true;
                break;
            }
        }
    }

    // If bot has aura and gas cloud is within 30 yards, force stay down
    if (botHasAura && gasCloud->GetExactDist2d(bot) < 30.0f)
    {
        Position downPos = ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION;
        downPos.m_positionZ -= 3.0f;  // Move 3 yards down

        // If bot is not at the correct Z position (with some tolerance), force move down again
        if (abs(bot->GetPositionZ() - downPos.m_positionZ) > 0.5f)  // 0.5 yard tolerance
        {
            bot->GetMotionMaster()->Clear();  // Clear any existing movement
            return MoveTo(bot->GetMapId(), downPos.GetPositionX(),
                         downPos.GetPositionY(), downPos.GetPositionZ(),
                         false, true, true, true, MovementPriority::MOVEMENT_FORCED);
        }
        
        // If at correct Z position, just stay there
        return true;
    }
    
    // If bot has aura but gas cloud is not near, stay at tank position
    else if (botHasAura)
    {
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION.GetPositionX(),
                     ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION.GetPositionY(),
                     ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION.GetPositionZ(),
                     false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    // If someone has aura but not this bot, attack gas cloud (except healers)
    else if (someoneHasAura && !botAI->IsHeal(bot))
    {
        return Attack(gasCloud);
    }
    // If no one has aura yet, everyone stays at tank position
    else
    {
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION.GetPositionX(),
                     ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION.GetPositionY(),
                     ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION.GetPositionZ(),
                     false, true, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}
