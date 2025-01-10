#include "RaidIccActions.h"
#include "RaidIccStrategy.h"
#include "Playerbots.h"
#include "Timer.h"
#include "Vehicle.h"
#include "GenericSpellActions.h"
#include "GenericActions.h"
#include <fstream>

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

static std::vector<ObjectGuid> sporeOrder;

//Lord Marrowgwar
bool IccLmTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    bot->SetTarget(boss->GetGUID());
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
    {
        if (bot->GetExactDist2d(ICC_LM_TANK_POSITION) > 15.0f)
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
        return MoveTo(bot->GetMapId(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionX(),
                      ICC_DARK_RECKONING_SAFE_POSITION.GetPositionY(), ICC_DARK_RECKONING_SAFE_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    return false;
}

bool IccRangedPositionLadyDeathwhisperAction::Execute(Event event)
{
    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
    float radius = 5.0f;
    float moveIncrement = 3.0f;
    bool isRanged = botAI->IsRanged(bot);

    GuidVector members = AI_VALUE(GuidVector, "group members");
    if (isRanged)
    {
        // Ranged: spread from other members
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return MoveAway(unit, moveDistance);
            }
        }
    }

    return false;  // Everyone is in position
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
        if (unit && unit->GetEntry() == 38222) //vengeful shade ID
        {
            // Only run away if the shade is targeting us
            if (unit->GetVictim() == bot)
            {
                float currentDistance = bot->GetDistance2d(unit);
               
                // Move away from the Vengeful Shade if the bot is too close
                if (currentDistance < radius)
                {
                    return MoveAway(unit, radius - currentDistance);
                }
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
    }

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

    // Only target the mage that is channeling Below Zero
    if (!(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(69705)))
    {
        if (bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_ALLY2) > 45.0f)
            return bot->TeleportTo(bot->GetMapId(), ICC_GUNSHIP_TELEPORT_ALLY2.GetPositionX(),
                          ICC_GUNSHIP_TELEPORT_ALLY2.GetPositionY(), ICC_GUNSHIP_TELEPORT_ALLY2.GetPositionZ(), bot->GetOrientation());
    }

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

    // Only target the sorcerer that is channeling Below Zero
    if (!(boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(69705)))
    {
        if (bot->GetExactDist2d(ICC_GUNSHIP_TELEPORT_HORDE2) > 45.0f)
            return bot->TeleportTo(bot->GetMapId(), ICC_GUNSHIP_TELEPORT_HORDE2.GetPositionX(),
                          ICC_GUNSHIP_TELEPORT_HORDE2.GetPositionY(), ICC_GUNSHIP_TELEPORT_HORDE2.GetPositionZ(), bot->GetOrientation());
    }

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

    if (bot->HasAura(71042) || bot->HasAura(72408))
        return true;

    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
    float radius = 9.0f;
    float moveIncrement = 3.0f;
    bool isRanged = botAI->IsRanged(bot);

    GuidVector members = AI_VALUE(GuidVector, "group members");
    if (isRanged)
    {
        // Ranged: spread from other members
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot || botAI->IsTank(bot) || botAI->IsMelee(bot))
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return MoveAway(unit, moveDistance);
            }
        }
    }

    return false;  // Everyone is in position
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

    if (!(bot->HasAura(71042) || bot->HasAura(72408)) && botAI->IsMainTank(bot)) //rune of blood aura
        return false;

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
        // Find closest player (including melee)
        Unit* closestPlayer = nullptr;
        float minDist = radius;
        
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || unit == bot)
                continue;
                
            float dist = bot->GetExactDist2d(unit);
            if (dist < minDist)
            {
                minDist = dist;
                closestPlayer = unit;
            }
        }

        if (closestPlayer)
        {
            // Move away from closest player, but maintain roughly max range from boss
            float distToCenter = bot->GetExactDist2d(ICC_FESTERGUT_TANK_POSITION);
            float moveDistance = (distToCenter > 25.0f) ? 2.0f : 3.0f; // Move less if already far from center
            return MoveAway(closestPlayer, moveDistance);
        }
    }
    return false;
}

bool IccFestergutSporeAction::Execute(Event event)
{
    const float POSITION_TOLERANCE = 4.0f;
    const float SPREAD_RADIUS = 2.0f;  // How far apart ranged should spread

    bool hasSpore = bot->HasAura(69279); // gas spore
    
    // If bot has spore, stop attacking
    if (hasSpore)
    {
        bot->AttackStop();
    }

    // Calculate a unique spread position for ranged
    float angle = (bot->GetGUID().GetCounter() % 16) * (M_PI / 8); // Divide circle into 16 positions
    Position spreadRangedPos = ICC_FESTERGUT_RANGED_SPORE;
    spreadRangedPos.m_positionX += cos(angle) * SPREAD_RADIUS;
    spreadRangedPos.m_positionY += sin(angle) * SPREAD_RADIUS;

    // Find all spored players and the one with lowest GUID
    ObjectGuid lowestGuid;
    bool isFirst = true;
    std::vector<Unit*> sporedPlayers;
    
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit)
            continue;

        if (unit->HasAura(69279))
        {
            sporedPlayers.push_back(unit);
            if (isFirst || unit->GetGUID() < lowestGuid)
            {
                lowestGuid = unit->GetGUID();
                isFirst = false;
            }
        }
    }

    // If no spores present at all, return
    if (sporedPlayers.empty())
        return false;

    Position targetPos;
    if (hasSpore)
    {
        // If bot is tank, always go melee
        if (botAI->IsTank(bot))
        {
            targetPos = ICC_FESTERGUT_MELEE_SPORE;
        }
        // If this bot has the lowest GUID among spored players, it goes melee
        else if (bot->GetGUID() == lowestGuid)
        {
            targetPos = ICC_FESTERGUT_MELEE_SPORE;
        }
        // All other spored players go ranged
        else
        {
            targetPos = spreadRangedPos;
        }
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
                     true, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return hasSpore;
}

//ROTFACE
bool IccRotfaceTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
    return false;

    // Main tank positioning logic
    if (botAI->IsMainTank(bot))
    {
        if (bot->GetExactDist2d(ICC_ROTFACE_TANK_POSITION) > 7.0f)
        return MoveTo(bot->GetMapId(), ICC_ROTFACE_TANK_POSITION.GetPositionX(),
                      ICC_ROTFACE_TANK_POSITION.GetPositionY(), ICC_ROTFACE_TANK_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // Assist tank positioning for big ooze
    if (botAI->IsAssistTank(bot))
    {
        // If we have the ooze flood aura, move away
        if (bot->HasAura(71215))
        {
            return MoveTo(boss->GetMapId(), boss->GetPositionX() + 5.0f * cos(bot->GetAngle(boss)),
                         boss->GetPositionY() + 5.0f * sin(bot->GetAngle(boss)), bot->GetPositionZ(), false,
                         false, false, true, MovementPriority::MOVEMENT_FORCED);
        }

        Unit* bigOoze = AI_VALUE2(Unit*, "find target", "big ooze");
        if (bigOoze)
        {
            // Taunt if not targeting us
            if (bigOoze->GetVictim() != bot)
            {
                if (botAI->CastSpell("taunt", bigOoze))
                    return true;
                return Attack(bigOoze);
            }

            // Keep big ooze at designated position
            if (bigOoze->GetVictim() == bot)
            {
                if (bot->GetExactDist2d(ICC_ROTFACE_BIG_OOZE_POSITION) > 5.0f)
                {
                    return MoveTo(bot->GetMapId(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionX(),
                              ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionY(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionZ(),
                              false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
                return Attack(bigOoze);
            }

            return Attack(bigOoze);
        }
    }

    return false;
}

bool IccRotfaceGroupPositionAction::Execute(Event event)
{
    // Find Rotface
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;   

 

    // Check for puddles and move away if too close
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit)
        {
            if (unit->GetEntry() == 37013) // puddle
            {
                float puddleDistance = bot->GetExactDist2d(unit);
               
                
                if (puddleDistance < 30.0f && (bot->HasAura(71215) || bot->HasAura(69789)))
                {
                    float dx = boss->GetPositionX() - unit->GetPositionX();
                    float dy = boss->GetPositionY() - unit->GetPositionY();
                    float angle = atan2(dy, dx);
                    
                    // Move away from puddle in smaller increment
                    float moveDistance = std::min(35.0f - puddleDistance, 5.0f);
                    float moveX = boss->GetPositionX() + (moveDistance * cos(angle));
                    float moveY = boss->GetPositionY() + (moveDistance * sin(angle));
                    
                    return MoveTo(boss->GetMapId(), moveX, moveY, boss->GetPositionZ(), 
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    // Check if we're targeted by little ooze
    Unit* smallOoze = AI_VALUE2(Unit*, "find target", "little ooze");
    if (smallOoze && smallOoze->GetVictim() == bot)
    {
        if (bot->GetExactDist2d(ICC_ROTFACE_BIG_OOZE_POSITION) > 3.0f)
        {
            return MoveTo(bot->GetMapId(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionX(),
                        ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionY(), ICC_ROTFACE_BIG_OOZE_POSITION.GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        return true; // Stay at position
    }

    if(botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
        if (!bot->HasAura(71215) && !bot->HasAura(69789))  // ooze flood id
        {
            float radius = 10.0f;
            Unit* closestMember = nullptr;
            float minDist = radius;
            GuidVector members = AI_VALUE(GuidVector, "group members");
        
            for (auto& member : members)
            {
                Unit* unit = botAI->GetUnit(member);
                if (!unit || bot->GetGUID() == member)
                    continue;

                // Skip distance check if the other unit is an assist tank
                if (botAI->IsAssistTank(bot))
                    continue;

             float dist = bot->GetExactDist2d(unit);
                if (dist < minDist)
                {
                    minDist = dist;
                    closestMember = unit;
                }
            }

         if (closestMember)
            {
                float distToCenter = bot->GetExactDist2d(ICC_ROTFACE_TANK_POSITION);
                float moveDistance = (distToCenter > 25.0f) ? 2.0f : 3.0f;
                return MoveAway(closestMember, moveDistance);
            }
            
            return false;
        }
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
                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
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
            // Move away from the puddle if the bot is too close
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

uint8_t IccPutricideGasCloudAction::lastKnownPosition = 0;

bool IccPutricideGasCloudAction::Execute(Event event)
{
    if (botAI->IsMainTank(bot)) 
        return false;

    // Find the gas cloud
    Unit* gasCloud = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!gasCloud)
        return false;

    static ObjectGuid lastGasCloudGuid;
    // If this is a new gas cloud, reset to position 2
    if (lastGasCloudGuid != gasCloud->GetGUID())
    {
        lastGasCloudGuid = gasCloud->GetGUID();
        lastKnownPosition = 0;  // This will make it go to position 2
    }

    // Check if this bot has Gaseous Bloat
    bool botHasAura = botAI->HasAura("Gaseous Bloat", bot);
    
    // If bot has aura, handle movement between positions
    if (botHasAura)
    {
        // Get current position
        float dist1 = bot->GetExactDist2d(ICC_PUTRICIDE_GAS1_POSITION.GetPositionX(), ICC_PUTRICIDE_GAS1_POSITION.GetPositionY());
        float dist2 = bot->GetExactDist2d(ICC_PUTRICIDE_GAS2_POSITION.GetPositionX(), ICC_PUTRICIDE_GAS2_POSITION.GetPositionY());
        float dist3 = bot->GetExactDist2d(ICC_PUTRICIDE_GAS3_POSITION.GetPositionX(), ICC_PUTRICIDE_GAS3_POSITION.GetPositionY());
        float dist4 = bot->GetExactDist2d(ICC_PUTRICIDE_GAS4_POSITION.GetPositionX(), ICC_PUTRICIDE_GAS4_POSITION.GetPositionY());
            
        uint8_t currentPosition = 0;
        const Position* nextPos = nullptr;
        
        // Determine current position
        if (dist1 < 8.0f) currentPosition = 1;
        else if (dist2 < 8.0f) currentPosition = 2;
        else if (dist3 < 8.0f) currentPosition = 3;
        else if (dist4 < 8.0f) currentPosition = 4;

        // If we're at a new position, update last known position
        if (currentPosition != 0 && currentPosition != lastKnownPosition)
        {
            lastKnownPosition = currentPosition;
        }

        // Only prevent movement if we're already moving and haven't reached the target yet
        if (lastKnownPosition != 0 && lastKnownPosition != currentPosition && AI_VALUE(bool, "moving"))
        {
            return false;
        }

        // Determine next position to move to
        if (gasCloud->GetExactDist2d(bot) < 18.0f)
        {
            switch(currentPosition)
            {
                case 0: // Not at any position
                case 1:
                    nextPos = &ICC_PUTRICIDE_GAS2_POSITION;
                    lastKnownPosition = 2;
                    break;
                case 2:
                    nextPos = &ICC_PUTRICIDE_GAS3_POSITION;
                    lastKnownPosition = 3;
                    break;
                case 3:
                    nextPos = &ICC_PUTRICIDE_GAS4_POSITION;
                    lastKnownPosition = 4;
                    break;
                case 4:
                    nextPos = &ICC_PUTRICIDE_GAS1_POSITION;
                    lastKnownPosition = 1;
                    break;
            }

            if (nextPos)
            {
                // Use PathGenerator to find a safe path to the target
                PathGenerator path(bot);
                path.CalculatePath(nextPos->GetPositionX(), nextPos->GetPositionY(), nextPos->GetPositionZ(), false);
                
                if (path.GetPathType() == PATHFIND_NORMAL || path.GetPathType() == PATHFIND_INCOMPLETE)
                {
                    return MoveTo(bot->GetMapId(), nextPos->GetPositionX(), nextPos->GetPositionY(), nextPos->GetPositionZ(), 
                         false, true, false, false, MovementPriority::MOVEMENT_FORCED);
                }
                else
                {
                    // If no valid path found, try to move directly (old behavior)
                return MoveTo(bot->GetMapId(), nextPos->GetPositionX(), nextPos->GetPositionY(), nextPos->GetPositionZ(), 
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            }
            return false;
        }
        return false;
    }
    // If bot doesn't have aura, check if anyone else does
    else
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        bool someoneHasAura = false;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member)
                continue;
                
            if (botAI->HasAura("Gaseous Bloat", member))
            {
                someoneHasAura = true;
                break;
            }
        }

    // If someone has aura but not this bot, attack gas cloud (except healers)
        if (someoneHasAura && !botAI->IsHeal(bot))
    {
            lastKnownPosition = 0;
        return Attack(gasCloud);
    }
    // If no one has aura yet, everyone stays at position 2
        else if (!someoneHasAura)
    {
            lastKnownPosition = 0;
        return MoveTo(bot->GetMapId(), ICC_PUTRICIDE_GAS2_POSITION.GetPositionX(), 
                     ICC_PUTRICIDE_GAS2_POSITION.GetPositionY(),
                     ICC_PUTRICIDE_GAS2_POSITION.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
    }

    return false;
}

bool AvoidMalleableGooAction::Execute(Event event)
{
    if (botAI->IsRanged(bot) || botAI->IsHeal(bot))
    {
        float radius = 7.0f;
        float moveIncrement = 3.0f;
        bool isRanged = botAI->IsRanged(bot);

       GuidVector members = AI_VALUE(GuidVector, "group members");
        if (isRanged)
        {
            // Ranged: spread from other members
            for (auto& member : members)
            {
                Unit* unit = botAI->GetUnit(member);
                if (!unit || !unit->IsAlive() || unit == bot || botAI->IsTank(bot) || botAI->IsMelee(bot))
                    continue;

                float dist = bot->GetExactDist2d(unit);
                if (dist < radius)
                {   
                    float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                    return MoveAway(unit, moveDistance);
                }
            }
        }
        return false;
    }

    return false;
}

//BPC
bool IccBpcKelesethTankAction::Execute(Event event)
{
    if (!botAI->IsAssistTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss)
        return false;

    // Check if we're not the victim of Keleseth's attack
    if (!(boss->GetVictim() == bot))
        return Attack(boss);

    // First check for any nucleus that needs to be picked up
    bool isCollectingNuclei = false;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->IsAlive() && unit->GetEntry() == 38369) // Dark Nucleus entry
        {
            if (!unit->GetVictim() || unit->GetVictim() != bot)
            {
                isCollectingNuclei = true;
                return Attack(unit); // Pick up any nucleus that isn't targeting us
            }
        }
    }

    // If not collecting nuclei, move to OT position
    if (!isCollectingNuclei && bot->GetExactDist2d(ICC_BPC_OT_POSITION) > 20.0f)
        return MoveTo(bot->GetMapId(), ICC_BPC_OT_POSITION.GetPositionX(),
                    ICC_BPC_OT_POSITION.GetPositionY(), ICC_BPC_OT_POSITION.GetPositionZ(),
                    false, true, false, true, MovementPriority::MOVEMENT_COMBAT);

    return Attack(boss);
}

bool IccBpcNucleusAction::Execute(Event event)
{
    if (!botAI->IsAssistTank(bot))
        return false;

    // Actively look for any nucleus that isn't targeting us
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->IsAlive() && unit->GetEntry() == 38369) // Dark Nucleus entry
        {
            if (!unit->GetVictim() || unit->GetVictim() != bot)
                return Attack(unit); // Pick up any nucleus that isn't targeting us
        }
    }

    return false;
}

bool IccBpcMainTankAction::Execute(Event event)
{
    if (botAI->IsMainTank(bot))
    {
        // Move to MT position if we're not there
        if (bot->GetExactDist2d(ICC_BPC_MT_POSITION) > 20.0f)
            return MoveTo(bot->GetMapId(), ICC_BPC_MT_POSITION.GetPositionX(),
                        ICC_BPC_MT_POSITION.GetPositionY(), ICC_BPC_MT_POSITION.GetPositionZ(),
                        false, true, false, true, MovementPriority::MOVEMENT_COMBAT);

        Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
        Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
        Unit* currentTarget = AI_VALUE(Unit*, "current target");

        // Keep current prince if we have one
        if (currentTarget && (currentTarget == valanar || currentTarget == taldaram))
            return Attack(currentTarget);

        // Pick a new prince that isn't targeting us
        if (valanar && (!valanar->GetVictim() || valanar->GetVictim() != bot))
            return Attack(valanar);
        if (taldaram && (!taldaram->GetVictim() || taldaram->GetVictim() != bot))
            return Attack(taldaram);

        return false;
    }
    
    if (!botAI->IsTank(bot))
    {
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        // First check if skull-marked target is a valid empowered prince
        Unit* skullTarget = nullptr;
        if (Group* group = bot->GetGroup())
        {
            if (ObjectGuid skullGuid = group->GetTargetIcon(7)) // 7 = skull
            {
                skullTarget = botAI->GetUnit(skullGuid);
                if (skullTarget && skullTarget->IsAlive() && skullTarget->HasAura(71596) &&
                    (skullTarget->GetEntry() == 37972 ||    // Keleseth
                     skullTarget->GetEntry() == 37973 ||    // Taldaram
                     skullTarget->GetEntry() == 37970))     // Valanar
                {
                    return Attack(skullTarget);
                }
            }
        }

        // If no valid skull target, search for empowered prince
        Unit* empoweredPrince = nullptr;
        for (auto i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->HasAura(71596))
            {
                if (unit->GetEntry() == 37972 ||    // Keleseth
                 unit->GetEntry() == 37973 ||    // Taldaram
                    unit->GetEntry() == 37970)      // Valanar
                {
                empoweredPrince = unit;

                    // Mark empowered prince with skull if in group
                    if (Group* group = bot->GetGroup())
                    {
                        group->SetTargetIcon(7, bot->GetGUID(), unit->GetGUID()); // 7 = skull
                    }
                    break;
                }
            }
        }

        // Attack empowered prince if found and current target doesn't have aura
        if (empoweredPrince)
        {
            // Only switch if current target doesn't have the aura
            if (!currentTarget || !currentTarget->HasAura(71596))
            {
                return Attack(empoweredPrince);
            }
            else
            {
                return Attack(currentTarget);
            }
        }

        // Keep current prince target if no empowered prince found
        if (currentTarget && (currentTarget->GetEntry() == 37972 ||   // Keleseth
                             currentTarget->GetEntry() == 37973 ||   // Taldaram
                             currentTarget->GetEntry() == 37970))    // Valanar
        {
            return Attack(currentTarget);
        }

    }
    return false;
}

bool IccBpcEmpoweredVortexAction::Execute(Event event)
{
    // Double check that we're not a tank
    if (botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    float radius = 12.0f;
    float moveIncrement = 3.0f;
    bool isRanged = botAI->IsRanged(bot);

    GuidVector members = AI_VALUE(GuidVector, "group members");
    if (isRanged)
    {
        // Ranged: spread from other ranged
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot || 
                botAI->IsMainTank(bot) || botAI->IsAssistTank(bot) || !botAI->IsRanged(bot))
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return MoveAway(unit, moveDistance);
            }
        }
    }
    else
    {
        // Melee: move opposite to ranged group
        float avgX = 0, avgY = 0;
        int rangedCount = 0;

        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || !botAI->IsRanged(bot))
                continue;

            avgX += unit->GetPositionX();
            avgY += unit->GetPositionY();
            rangedCount++;
        }

        if (rangedCount > 0)
        {
            avgX /= rangedCount;
            avgY /= rangedCount;

            // Direction from ranged to Valanar
            float dx = valanar->GetPositionX() - avgX;
            float dy = valanar->GetPositionY() - avgY;
            float len = sqrt(dx*dx + dy*dy);
            
            if (len > 0)
            {
                dx /= len;
                dy /= len;
                float targetX = valanar->GetPositionX() + dx * 5.0f;
                float targetY = valanar->GetPositionY() + dy * 5.0f;
                float targetZ = valanar->GetPositionZ();
                bot->UpdateAllowedPositionZ(targetX, targetY, targetZ);

                return MoveTo(bot->GetMapId(), targetX, targetY, targetZ);
            }
        }
    }

    return false;
}

//BQL

bool IccBqlTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");

    // If tank is not at position, move there
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
    {
    if (bot->GetExactDist2d(ICC_BQL_TANK_POSITION) > 20.0f)
        return MoveTo(bot->GetMapId(), ICC_BQL_TANK_POSITION.GetPositionX(),
                    ICC_BQL_TANK_POSITION.GetPositionY(), ICC_BQL_TANK_POSITION.GetPositionZ(),
                    false, true, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    float radius = 8.0f;
    float moveIncrement = 3.0f;
    bool isRanged = botAI->IsRanged(bot);
    bool isMelee = botAI->IsMelee(bot);

    GuidVector members = AI_VALUE(GuidVector, "group members");
    if (isRanged && !(bot->HasAura(70877) || bot->HasAura(71474) && boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))) //frenzied bloodthrist
    {
        // Ranged: spread from other ranged
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot || unit->HasAura(70877) || unit->HasAura(71474))
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return MoveAway(unit, moveDistance);
            }
        }
    }
    if (isMelee && boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY)) // melee also spread
    {
        // Melee: spread from other melee
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot || unit->HasAura(70877) || unit->HasAura(71474))
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
            {
                float moveDistance = std::min(moveIncrement, radius - dist + 1.0f);
                return MoveAway(unit, moveDistance);
            }
        }
    }
    return false;  // Everyone is in position
}

bool IccBqlPactOfDarkfallenAction::Execute(Event event)
{
    // Check if bot has Pact of the Darkfallen
    if (!bot->HasAura(71340))
        return false;

    const float POSITION_TOLERANCE = 1.0f;  // Within 1 yards to break the link

    // Find other players with Pact of the Darkfallen
    std::vector<Player*> playersWithAura;
    Player* tankWithAura = nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Gather all players with the aura
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member->GetGUID() == bot->GetGUID())
            continue;

        if (member->HasAura(71340)) //pact of darkfallen
        {
            playersWithAura.push_back(member);
            // If this player is a tank, store them
            if (botAI->IsTank(member))
                tankWithAura = member;
        }
    }

    // If we found other players with the aura
    if (!playersWithAura.empty())
    {
        Position targetPos;
        
        if (playersWithAura.size() >= 2)  // 3 or more total (including this bot)
        {
            if (tankWithAura)
            {
                // Move to tank's position if we're not a tank
                if (!botAI->IsTank(bot))
                {
                    targetPos.Relocate(tankWithAura);
                }
                else
                {
                    // If we are the tank, stay put
                    return true;
                }
            }
            else
            {
                // Calculate center position of all affected players
                float sumX = bot->GetPositionX();
                float sumY = bot->GetPositionY();
                float sumZ = bot->GetPositionZ();
                int count = 1;  // Start with 1 for this bot

                for (Player* player : playersWithAura)
                {
                    sumX += player->GetPositionX();
                    sumY += player->GetPositionY();
                    sumZ += player->GetPositionZ();
                    count++;
                }

                targetPos.Relocate(sumX / count, sumY / count, sumZ / count);
            }
        }
        else  // Only one other player has aura
        {
            targetPos.Relocate(playersWithAura[0]);
        }

        // Move to target position if we're not already there
        if (bot->GetDistance(targetPos) > POSITION_TOLERANCE)
        {
            return MoveTo(bot->GetMapId(), targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
        return true;
    }

    // If no other players found with aura, move to center
    if (bot->GetDistance(ICC_BQL_CENTER_POSITION) > POSITION_TOLERANCE)
    {
        botAI->SetNextCheckDelay(500);
        return MoveTo(bot->GetMapId(), ICC_BQL_CENTER_POSITION.GetPositionX(), ICC_BQL_CENTER_POSITION.GetPositionY(), ICC_BQL_CENTER_POSITION.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool IccBqlVampiricBiteAction::Execute(Event event)
{
    // Only act when bot has Frenzied Bloodthirst
    if (!(bot->HasAura(70877) || bot->HasAura(71474)))
        return false;

    const float BITE_RANGE = 2.0f;
    Player* target = nullptr;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Create lists for potential targets
    std::vector<Player*> dpsTargets;
    std::vector<Player*> healTargets;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        // Skip if already has essence, frenzy, or is a tank, or uncontrollable frenzy
        if (member->HasAura(70867) || member->HasAura(70877) || member->HasAura(70879) || member->HasAura(71473) 
           || member->HasAura(71474) || member->HasAura(71525) || member->HasAura(71530) || member->HasAura(71531) 
           || member->HasAura(71532) || member->HasAura(71533) || member->HasAura(70923) || botAI->IsTank(member))
        {
            continue;
        }

        if (botAI->IsDps(member))
            dpsTargets.push_back(member);
        else if (botAI->IsHeal(member))
            healTargets.push_back(member);
    }

    // First try DPS targets
    if (!dpsTargets.empty())
    {
        target = dpsTargets[0];  // Take first available DPS
    }
    // If no DPS available, try healers
    else if (!healTargets.empty())
    {
        target = healTargets[0];  // Take first available healer
    }

    if (!target)
    {
        return false;
    }

    // Double check target is still alive
    if (!target->IsAlive())
    {
        return false;
    }

    // Check if we can reach the target
    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();

    if (bot->IsWithinLOS(x, y, z) && bot->GetExactDist2d(target) > BITE_RANGE)
    {
        return MoveTo(target->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // If in range and can see target, cast the bite
    if (bot->IsWithinLOS(x, y, z) && bot->GetExactDist2d(target) <= BITE_RANGE)
    {
        // Final alive check before casting
        if (!target->IsAlive())
        {
            return false;
        }

        if (botAI->CanCastSpell(70946, target))
        {
            return botAI->CastSpell(70946, target);
        }
    }

    return false;
}

//VDW

//Valkyre 38248 spear, 50307 spear in inv, Sister Svalna 37126, aether shield 71463

bool IccValkyreSpearAction::Execute(Event event)
{
    // Find the nearest spear
    Creature* spear = bot->FindNearestCreature(38248, 100.0f);
    if (!spear)
        return false;

    // Move to the spear if not in range
    if (!spear->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
    {
        return MoveTo(spear, INTERACTION_DISTANCE);
    }

    // Remove shapeshift forms
    botAI->RemoveShapeshift();

    // Stop movement and click the spear
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    spear->HandleSpellClick(bot);

    // Dismount if mounted
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    return false;
}

bool IccSisterSvalnaAction::Execute(Event event)
{
    Unit* svalna = AI_VALUE2(Unit*, "find target", "sister svalna");
    if (!svalna || !svalna->HasAura(71463)) // Check for Aether Shield aura
        return false;

    // Check if bot has the spear item
    if (!botAI->HasItemInInventory(50307))
        return false;

    // Get all items from inventory
    std::vector<Item*> items = botAI->GetInventoryItems();
    for (Item* item : items)
    {
        if (item->GetEntry() == 50307) // Spear ID
        {
            botAI->ImbueItem(item, svalna); // Use spear on Svalna
            return false;
        }
    }

    return false;
}

bool IccValithriaPortalAction::Execute(Event event)
{
    if (!botAI->IsHeal(bot) || bot->getClass() == CLASS_DRUID || bot->HasAura(70766))
        return false;

    // Find the nearest portal
    Creature* portal = bot->FindNearestCreature(37945, 100.0f); // Dream Portal
    if (!portal)
        portal = bot->FindNearestCreature(38430, 100.0f); // Nightmare Portal

    if (!portal)
        return false;

    // Move to the portal if the bot is not at the interact distance
    if (!portal->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
    {
        return MoveTo(portal, INTERACTION_DISTANCE);
    }

    // Remove shapeshift forms
    botAI->RemoveShapeshift();

    // Stop movement and click the portal
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    portal->HandleSpellClick(bot);

    // Dismount if mounted
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    return false;
}

bool IccValithriaHealAction::Execute(Event event)
{
    if (!botAI->IsHeal(bot))
        return false;

    // Find Valithria
    if (Creature* valithria = bot->FindNearestCreature(36789, 100.0f))
    {
        switch (bot->getClass())
        {
            case CLASS_SHAMAN:
                return valithria->HasAura(61301) ? botAI->CastSpell(49273, valithria) : botAI->CastSpell(61301, valithria); // Cast Healing Wave if Riptide is up, otherwise cast Riptide
            case CLASS_PRIEST:
                return valithria->HasAura(48068) ? botAI->CastSpell(48063, valithria) : botAI->CastSpell(48068, valithria); // Cast Greater Heal if Renew is up, otherwise cast Renew
            case CLASS_PALADIN:
                return valithria->HasAura(53563) ? botAI->CastSpell(48782, valithria) : botAI->CastSpell(53563, valithria); // Cast Holy Light if Beacon is up, otherwise cast Beacon of Light
            default:
                return false;
        }
    }

    return false;
}

bool IccValithriaDreamCloudAction::Execute(Event event)
{
    // Only execute if we're in dream state
    if (!bot->HasAura(70766))
        return false;

    // Find nearest cloud of either type that we haven't collected
    Creature* dreamCloud = bot->FindNearestCreature(37985, 100.0f);
    Creature* nightmareCloud = bot->FindNearestCreature(38421, 100.0f);

    // If we have emerald vigor, prioritize dream clouds
    if (bot->HasAura(70873))
    {
        if (dreamCloud)
            return MoveTo(dreamCloud->GetMapId(), dreamCloud->GetPositionX(), dreamCloud->GetPositionY(), dreamCloud->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        if (nightmareCloud)
            return MoveTo(nightmareCloud->GetMapId(), nightmareCloud->GetPositionX(), nightmareCloud->GetPositionY(), nightmareCloud->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }
    // Otherwise prioritize nightmare clouds
    else
    {
        if (nightmareCloud)
            return MoveTo(nightmareCloud->GetMapId(), nightmareCloud->GetPositionX(), nightmareCloud->GetPositionY(), nightmareCloud->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        if (dreamCloud)
            return MoveTo(dreamCloud->GetMapId(), dreamCloud->GetPositionX(), dreamCloud->GetPositionY(), dreamCloud->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

//Sindragosa

bool IccSindragosaTankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
        return false;

    if ((botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot)) && (boss->GetVictim() == bot)) 
    {
        float distBossToCenter = boss->GetExactDist2d(ICC_SINDRAGOSA_CENTER_POSITION);
        float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
        float targetOrientation = M_PI / 2; // We want boss to face east
        float currentOrientation = boss->GetOrientation();
        
        // Normalize both orientations to 0-2π range
        currentOrientation = fmod(currentOrientation + 2 * M_PI, 2 * M_PI);
        targetOrientation = fmod(targetOrientation + 2 * M_PI, 2 * M_PI);
        
        float orientationDiff = currentOrientation - targetOrientation;
        
        // Normalize the difference to be between -PI and PI
        while (orientationDiff > M_PI) orientationDiff -= 2 * M_PI;
        while (orientationDiff < -M_PI) orientationDiff += 2 * M_PI;
        
        
        // Stage 1: Move boss to center if too far
        if (distBossToCenter > 20.0f)
        {
            
            // Calculate direction vector from boss to center
            float dirX = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionX() - boss->GetPositionX();
            float dirY = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionY() - boss->GetPositionY();
            
            // Move 10 yards beyond center in the same direction
            float moveX = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionX() + (dirX / distBossToCenter) * 10.0f;
            float moveY = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionY() + (dirY / distBossToCenter) * 10.0f;
            
            return MoveTo(bot->GetMapId(), moveX, moveY, boss->GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
        
        // Stage 2: Get to tank position when boss is centered
        if (distToTankPos > 5.0f)
        {
            return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_TANK_POSITION.GetPositionX(),
                      ICC_SINDRAGOSA_TANK_POSITION.GetPositionY(),
                      ICC_SINDRAGOSA_TANK_POSITION.GetPositionZ(),
                      false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
        
        // Stage 3: Adjust orientation when in position
        bool needsOrientationAdjust = std::abs(orientationDiff) > 0.15f;
        if (needsOrientationAdjust)
        {
            // When we have negative difference (currentOrientation < targetOrientation)
            // We need to move south to make the orientation more positive
            float currentX = bot->GetPositionX();
            float currentY = bot->GetPositionY();
            float moveX, moveY;
            
            // For negative difference (need to increase orientation) -> move south
            // For positive difference (need to decrease orientation) -> move north
            if (orientationDiff < 0)
            {
                moveX = currentX - 2.0f; // Move south to increase orientation
                moveY = currentY;
            }
            else
            {
                moveX = currentX + 2.0f; // Move north to decrease orientation
                moveY = currentY;
            }
            
            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
        }
        return false;
    }
    
    // Non-tanks should stay on the left flank to avoid both cleave and tail smash
    if (boss && !(boss->GetVictim() == bot) /*&& !bot->HasAura(69762)*/)
    {
        if (botAI->IsRanged(bot))
        {
            float const TOLERANCE = 15.0f;        // 15 yard tolerance
            float const MAX_STEP = 5.0f;          // Maximum distance to move in one step
            
            float distToTarget = bot->GetExactDist2d(ICC_SINDRAGOSA_RANGED_POSITION);

            // Only move if outside tolerance
            if (distToTarget > TOLERANCE)
            {
                // Calculate direction vector to target
                float dirX = ICC_SINDRAGOSA_RANGED_POSITION.GetPositionX() - bot->GetPositionX();
                float dirY = ICC_SINDRAGOSA_RANGED_POSITION.GetPositionY() - bot->GetPositionY();
                
                // Normalize direction vector
                float length = sqrt(dirX * dirX + dirY * dirY);
                dirX /= length;
                dirY /= length;

                // Calculate intermediate point
                float stepSize = std::min(MAX_STEP, distToTarget);
                float moveX = bot->GetPositionX() + dirX * stepSize;
                float moveY = bot->GetPositionY() + dirY * stepSize;

                return MoveTo(bot->GetMapId(), moveX, moveY, ICC_SINDRAGOSA_RANGED_POSITION.GetPositionZ(),
                             false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            return false;
        }
        else
        {
            float const TOLERANCE = 10.0f;  // 10 yard tolerance for melee
            float const MAX_STEP = 5.0f;    // Maximum distance to move in one step

            float distToTarget = bot->GetExactDist2d(ICC_SINDRAGOSA_MELEE_POSITION);

            // Only move if outside tolerance
            if (distToTarget > TOLERANCE)
            {
                // Calculate direction vector to target
                float dirX = ICC_SINDRAGOSA_MELEE_POSITION.GetPositionX() - bot->GetPositionX();
                float dirY = ICC_SINDRAGOSA_MELEE_POSITION.GetPositionY() - bot->GetPositionY();
                
                // Normalize direction vector
                float length = sqrt(dirX * dirX + dirY * dirY);
                dirX /= length;
                dirY /= length;

                // Calculate intermediate point
                float stepSize = std::min(MAX_STEP, distToTarget);
                float moveX = bot->GetPositionX() + dirX * stepSize;
                float moveY = bot->GetPositionY() + dirY * stepSize;

                return MoveTo(bot->GetMapId(), moveX, moveY, ICC_SINDRAGOSA_MELEE_POSITION.GetPositionZ(),
                            false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
            return false;
        }
    }
    return false;
}

bool IccSindragosaTankSwapPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    
    // Move to tank position
    if (distToTankPos > 3.0f)  // Tighter tolerance for tank swap
    {
        return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_TANK_POSITION.GetPositionX(),
                     ICC_SINDRAGOSA_TANK_POSITION.GetPositionY(),
                     ICC_SINDRAGOSA_TANK_POSITION.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool IccSindragosaFrostBeaconAction::Execute(Event event)
{
    float const POSITION_TOLERANCE = 3.0f; // Increased tolerance to reduce jitter
    
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;
   
    // Handle beaconed players
    if (bot->HasAura(70126))
    {
        if (boss && boss->HealthBelowPct(35) && !boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY)) 
        {
            // Only move if not already at position (with tolerance)
            if (bot->GetExactDist2d(ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionX(), ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionY()) > POSITION_TOLERANCE)
            {
                return MoveTo(bot->GetMapId(), 
                            ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionX(),
                            ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionY(),
                            ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionZ(),
                            false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            return false;
        }
        else
        {
            Position const* tombPosition;
            uint8 beaconIndex = 0;
            bool foundSelf = false;
            
            Group* group = bot->GetGroup();
            if (!group)
                return false;

            // Find this bot's index among players with Frost Beacon
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive() || !member->HasAura(70126)) // Only count alive players with Frost Beacon
                    continue;

                if (member == bot)
                {
                    foundSelf = true;
                    break;
                }
                beaconIndex++;
            }

            if (!foundSelf)
                return false;

            switch (beaconIndex) {
                case 0:
                    tombPosition = &ICC_SINDRAGOSA_THOMB1_POSITION;
                    break;
                case 1:
                    tombPosition = &ICC_SINDRAGOSA_THOMB2_POSITION;
                    break;
                case 2:
                    tombPosition = &ICC_SINDRAGOSA_THOMB3_POSITION;
                    break;
                case 3:
                    tombPosition = &ICC_SINDRAGOSA_THOMB4_POSITION;
                    break;
                default:
                    tombPosition = &ICC_SINDRAGOSA_THOMB5_POSITION;
                    break;
            }
            
            // Only move if not already at position (with tolerance)
            float dist = bot->GetExactDist2d(tombPosition->GetPositionX(), tombPosition->GetPositionY());
            if (dist > POSITION_TOLERANCE)
            {
                return MoveTo(bot->GetMapId(), tombPosition->GetPositionX(),
                              tombPosition->GetPositionY(),
                              tombPosition->GetPositionZ(),
                              false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            return false;
        }
    }
    // Handle non-beaconed players
    else
    {
        float const MIN_SAFE_DISTANCE = 13.0f;
        float const MAX_SAFE_DISTANCE = 30.0f;
        float const MOVE_TOLERANCE = 2.0f; // Tolerance for movement to reduce jitter
        
        GuidVector members = AI_VALUE(GuidVector, "group members");
        std::vector<Unit*> beaconedPlayers;
        
        for (auto& member : members)
        {
            Unit* player = botAI->GetUnit(member);
            if (!player || player->GetGUID() == bot->GetGUID())
                continue;
                
            if (player->HasAura(70126)) // Frost Beacon
                beaconedPlayers.push_back(player);
        }
        
        if (beaconedPlayers.empty())
            return false;

        // Different behavior for air phase
        if (boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
        {
            if (!bot->HasAura(70126)) // If not beaconed, move to safe position
            {
                float dist = bot->GetExactDist2d(ICC_SINDRAGOSA_FBOMB_POSITION.GetPositionX(),
                                               ICC_SINDRAGOSA_FBOMB_POSITION.GetPositionY());
                if (dist > POSITION_TOLERANCE)
                {
                    return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_FBOMB_POSITION.GetPositionX(),
                                ICC_SINDRAGOSA_FBOMB_POSITION.GetPositionY(),
                                ICC_SINDRAGOSA_FBOMB_POSITION.GetPositionZ(),
                                false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
            return false;
        }
        else
        {
            // Ground phase - use existing vector-based movement
            bool needToMove = false;
            float moveX = 0, moveY = 0;
            
            for (Unit* beaconedPlayer : beaconedPlayers)
            {
                float dist = bot->GetExactDist2d(beaconedPlayer);
                if (dist < MIN_SAFE_DISTANCE + MOVE_TOLERANCE)
                {
                    needToMove = true;
                    float angle = bot->GetAngle(beaconedPlayer);
                    float moveDistance = std::min(5.0f, MIN_SAFE_DISTANCE - dist + MOVE_TOLERANCE);
                    
                    moveX += cos(angle + M_PI) * moveDistance;
                    moveY += sin(angle + M_PI) * moveDistance;
                }
            }
            
            if (needToMove && !bot->HasAura(70126))
            {
                float posX = bot->GetPositionX() + moveX;
                float posY = bot->GetPositionY() + moveY;
                float posZ = bot->GetPositionZ();
                bot->UpdateAllowedPositionZ(posX, posY, posZ);
                
                // Only move if the change in position is significant
                if (std::abs(moveX) > MOVE_TOLERANCE || std::abs(moveY) > MOVE_TOLERANCE)
                {
                    return MoveTo(bot->GetMapId(), posX, posY, posZ, 
                                false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }

    return false;
}

bool IccSindragosaBlisteringColdAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Only non-tanks should move out
    if (botAI->IsTank(bot) || botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
        return false;

    float dist = bot->GetDistance(boss);
    // Only move if we're too close to the boss (< 26 yards)
    if (dist < 26.0f)
    {
        float const MAX_STEP = 5.0f;
        float distToSafeSpot = bot->GetDistance2d(ICC_SINDRAGOSA_BLISTERING_COLD_POSITION.GetPositionX(),
                                                 ICC_SINDRAGOSA_BLISTERING_COLD_POSITION.GetPositionY());
        
        if (distToSafeSpot > 0.1f)  // Avoid division by zero
        {
            float ratio = std::min(MAX_STEP / distToSafeSpot, 1.0f);
            float moveX = bot->GetPositionX() + (ICC_SINDRAGOSA_BLISTERING_COLD_POSITION.GetPositionX() - bot->GetPositionX()) * ratio;
            float moveY = bot->GetPositionY() + (ICC_SINDRAGOSA_BLISTERING_COLD_POSITION.GetPositionY() - bot->GetPositionY()) * ratio;

            return MoveTo(bot->GetMapId(), moveX, moveY, ICC_SINDRAGOSA_BLISTERING_COLD_POSITION.GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    return false;
}

bool IccSindragosaUnchainedMagicAction::Execute(Event event)
{
    /*if (bot->HasAura(69762)) // unchained magic
    {
        float const SAFE_DISTANCE = 20.0f;
        float moveIncrement = 5.0f;
        bool needsToMove = false;
        float moveX = bot->GetPositionX();
        float moveY = bot->GetPositionY();

        // Check distance to other players
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (!unit || !unit->IsAlive() || unit == bot)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < SAFE_DISTANCE)
            {
                needsToMove = true;
                float moveDistance = std::min(moveIncrement, SAFE_DISTANCE - dist + 1.0f);
                return MoveAway(unit, moveDistance);
            }
        }
    }*/

    if (Aura* aura = bot->GetAura(69766))
    {
        if (aura->GetStackAmount() >= 6)
            return true; // Stop casting spells
    }

    return false;
}

bool IccSindragosaChilledToTheBoneAction::Execute(Event event)
{
    if (bot->HasAura(70106)) // Chilled to the Bone
    {
         if (Aura* aura = bot->GetAura(70106))
        {
            if (aura->GetStackAmount() >= 8)
            {
                bot->AttackStop();
                return true; // Stop casting spells
            }
        }
    }

    return false;  
}

bool IccSindragosaMysticBuffetAction::Execute(Event event)
{

    // Get boss to check if it exists
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (!bot || !bot->IsAlive())
        return false;

    // Check if we have Mystic Buffet
    Aura* aura = bot->GetAura(70127);
    Aura* aura2 = bot->GetAura(72528);
    if (!aura && !aura2)
        return false;

    if (bot->HasAura(70126))  // Ice Block
        return false;

    //if tank and is victim of boss, do nothing
    if (botAI->IsTank(bot) && (boss->GetVictim() == bot))
        return false;

    // For non-tanks, require 3+ stacks
    if (!((aura && aura->GetStackAmount() >= 3) || (aura2 && aura2->GetStackAmount() >= 3)))
        return false;

    // Find nearest player with ice tomb
    Unit* nearestTomb = nullptr;
    float minDist = 150.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (member->HasAura(70157))  // Ice Tomb
        {
            float dist = bot->GetDistance(member);
            if (dist < minDist)
            {
                minDist = dist;
                nearestTomb = member;
            }
        }
    }

    // If no tombs found, return false
    if (!nearestTomb)
    {
        return false;
    }

    if (nearestTomb->IsWithinDist2d(ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionX(), 
                                        ICC_SINDRAGOSA_THOMBMB2_POSITION.GetPositionY(), 1.0f))
    {
        // Check if already at LOS2 position
        if (bot->IsWithinDist2d(ICC_SINDRAGOSA_LOS2_POSITION.GetPositionX(), 
                               ICC_SINDRAGOSA_LOS2_POSITION.GetPositionY(), 1.0f))
        {
            return false;
        }

        return MoveTo(bot->GetMapId(),
                     ICC_SINDRAGOSA_LOS2_POSITION.GetPositionX(),
                     ICC_SINDRAGOSA_LOS2_POSITION.GetPositionY(),
                     ICC_SINDRAGOSA_LOS2_POSITION.GetPositionZ(),
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return false; // Tomb not at MB2 position
}

bool IccSindragosaFrostBombAction::Execute(Event event)
{
    if (!bot || !bot->IsAlive() || bot->HasAura(70157)) // Skip if dead or in Ice Tomb
        return false;

    // Find frost bomb marker
    Unit* marker = nullptr;
    Player* nearestTombPlayer = nullptr;
    float minDist = 150.0f;
    int tombPlayerCount = 0;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    if (npcs.empty())
        return false;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->HasAura(70022)) // Frost bomb visual
        {
            marker = unit;
            break;
        }
    }

    if (!marker)
        return false;

    // Find players in ice tomb
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        if (member->HasAura(70157))  // Ice Tomb aura
        {
            tombPlayerCount++;
            float dist = bot->GetDistance(member);
            if (dist < minDist)
            {
                minDist = dist;
                nearestTombPlayer = member;
            }
        }
    }

    // If no tombs or only one tomb, stop
    if (!nearestTombPlayer)
        return false;

    if (tombPlayerCount <= 1)
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
        if (!boss)
            return false;

        // If boss HP below 20% and we're already not in LOS of marker, stay in position
        if (boss->GetHealthPct() < 20 && !marker->IsWithinLOS(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
        {
            bot->AttackStop();
            return true;
        }
        
        // Otherwise handle normal single tomb case
        if (!marker->IsWithinLOS(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
        {
            bot->AttackStop();
            return true;
        }
        
        // If we have LOS with marker, we need to move behind tomb
        float angle = marker->GetAngle(nearestTombPlayer);
        float posX = nearestTombPlayer->GetPositionX() + cos(angle) * 5.0f;
        float posY = nearestTombPlayer->GetPositionY() + sin(angle) * 5.0f;
        float posZ = nearestTombPlayer->GetPositionZ();
        
        return MoveTo(bot->GetMapId(), posX, posY, posZ,
                     false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // Calculate position behind tomb for normal case (more than 1 tomb)
    float angle = marker->GetAngle(nearestTombPlayer);
    float posX = nearestTombPlayer->GetPositionX() + cos(angle) * 5.0f;
    float posY = nearestTombPlayer->GetPositionY() + sin(angle) * 5.0f;
    float posZ = nearestTombPlayer->GetPositionZ();

    // If we're already in position and not in LOS of marker, stop
    if (bot->GetDistance2d(posX, posY) < 2.0f && !marker->IsWithinLOS(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
    {
        bot->AttackStop();
        return true;
    }

    // Otherwise move to position
    return MoveTo(bot->GetMapId(), posX, posY, posZ,
                 false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool IccLichKingNecroticPlagueAction::Execute(Event event)
{
    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);

    // Only execute if we have the plague
    if (!hasPlague)
        return false;

    // Find closest shambling
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    Unit* closestHorror = nullptr;
    float minDistance = 100.0f;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        if (!unit->IsAlive())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == 37698 || entry == 39299 || entry == 39300 || entry == 39301) //shambling horror
        {
            float distance = bot->GetDistance(unit);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestHorror = unit;
            }
        }
    }

    // If we found a shambling and we're not close enough, move to it
    if (closestHorror)
    {
        // If we're too far, run to it
        if (minDistance > 3.0f)
        {
            // Use forced movement to ensure we get there quickly
            return MoveTo(closestHorror, 3.0f, MovementPriority::MOVEMENT_FORCED);
        }
        else
        {
            // We're close enough, stop moving
            bot->StopMoving();
            return true;
        }
    }

    return false;
}

bool IccLichKingWinterAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");

    float currentDistance = bot->GetDistance2d(boss);
    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    if (currentDistance < 48.0f)
    {
        if (botAI->IsRanged(bot))
        {
            // Calculate distances to group positions
            float dist1 = bot->GetDistance2d(ICC_LK_FROSTR1_POSITION.GetPositionX(), ICC_LK_FROSTR1_POSITION.GetPositionY());
            float dist2 = bot->GetDistance2d(ICC_LK_FROSTR2_POSITION.GetPositionX(), ICC_LK_FROSTR2_POSITION.GetPositionY());
            float dist3 = bot->GetDistance2d(ICC_LK_FROSTR3_POSITION.GetPositionX(), ICC_LK_FROSTR3_POSITION.GetPositionY());

            const Position* targetPos = &ICC_LK_FROSTR1_POSITION;
            if (dist2 < dist1 && dist2 < dist3)
            targetPos = &ICC_LK_FROSTR2_POSITION;
            else if (dist3 < dist1 && dist3 < dist2)
            targetPos = &ICC_LK_FROSTR3_POSITION;

            float distToTarget = bot->GetDistance2d(targetPos->GetPositionX(), targetPos->GetPositionY());
            if (distToTarget > 7.0f)
            {
                float angle = bot->GetAngle(targetPos);
                float posX = bot->GetPositionX() + cos(angle) * 5.0f;
                float posY = bot->GetPositionY() + sin(angle) * 5.0f;
                return MoveTo(bot->GetMapId(), posX, posY, 840.857f,
                        false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
        else
        {
            // Calculate distances to tank positions
            float dist1 = bot->GetDistance2d(ICC_LK_FROST1_POSITION.GetPositionX(), ICC_LK_FROST1_POSITION.GetPositionY());
            float dist2 = bot->GetDistance2d(ICC_LK_FROST2_POSITION.GetPositionX(), ICC_LK_FROST2_POSITION.GetPositionY());
            float dist3 = bot->GetDistance2d(ICC_LK_FROST3_POSITION.GetPositionX(), ICC_LK_FROST3_POSITION.GetPositionY());

            const Position* targetPos = &ICC_LK_FROST1_POSITION;
            if (dist2 < dist1 && dist2 < dist3)
                targetPos = &ICC_LK_FROST2_POSITION;
            else if (dist3 < dist1 && dist3 < dist2)
                targetPos = &ICC_LK_FROST3_POSITION;

            float distToTarget = bot->GetDistance2d(targetPos->GetPositionX(), targetPos->GetPositionY());
            if (distToTarget > 10.0f)
            {
                float angle = bot->GetAngle(targetPos);
                float posX = bot->GetPositionX() + cos(angle) * 5.0f;
                float posY = bot->GetPositionY() + sin(angle) * 5.0f;
                return MoveTo(bot->GetMapId(), posX, posY, 840.857f,
                    false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    // Check for spheres if we're at a safe distance
    if (bot->getClass() == CLASS_HUNTER)
    {
        Unit* closestSphere = nullptr;
        float closestDist = 100.0f;

        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;

            uint32 entry = unit->GetEntry();
            if (entry == 36633 || entry == 39305 || entry == 39306 || entry == 39307)
            {
                float dist = bot->GetDistance(unit);
                if (!closestSphere || dist < closestDist)
                {
                    closestSphere = unit;
                    closestDist = dist;
                }
            }
        }

        if (closestSphere)
        {
            return Attack(closestSphere);
        }
    }
    
    return false;
}

bool IccLichKingAddsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    Unit* spiritWarden = AI_VALUE2(Unit*, "find target", "spirit warden");

    //temp solution for bots going underground due to buggy ice platfroms and adds that go underground
    if (abs(bot->GetPositionZ() - 840.857f) > 1.0f)
        return bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(),
                          bot->GetPositionY(), 840.857f, bot->GetOrientation());

    //temp soultion for bots when they get teleport far away into another dimension (they are unable to attack spirit warden)
    if (abs(bot->GetPositionY() - -2095.7915f) > 200.0f)
    {
        return bot->TeleportTo(bot->GetMapId(), ICC_LICH_KING_ADDS_POSITION.GetPositionX(),
                          ICC_LICH_KING_ADDS_POSITION.GetPositionY(), ICC_LICH_KING_ADDS_POSITION.GetPositionZ(), bot->GetOrientation());
    }

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(72262) && bot->GetExactDist2d(boss) > 20.0f)    //quake
    {
        float angle = bot->GetAngle(boss);
        float posX = bot->GetPositionX() + cos(angle) * 10.0f;
        float posY = bot->GetPositionY() + sin(angle) * 10.0f;
        return MoveTo(bot->GetMapId(), posX, posY, boss->GetPositionZ(), 
                     false, true, false, false, MovementPriority::MOVEMENT_COMBAT);
    }


    std::vector<Unit*> defiles;
    Unit* closestDefile = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    // First gather all defiles
    GuidVector npcs1 = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs1)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == 38757)
        {
            defiles.push_back(unit);
            float dist = bot->GetDistance(unit);
            if (dist < closestDistance)
            {
                closestDistance = dist;
                closestDefile = unit;
            }
        }
    }
    if (!defiles.empty())
    {
        float baseRadius = 6.0f;
        float safetyMargin = 1.0f;  // Fixed 5-yard safety margin
        
        // First, find if we need to move from any defile
        bool needToMove = false;
        float bestAngle = 0.0f;
        float maxSafeDistance = 0.0f;

        // Check all defiles first to see if we need to move
        for (Unit* defile : defiles)
        {
            if (!defile || !defile->IsAlive())
                continue;

            float currentRadius = baseRadius;
            Aura* growAura = defile->GetAura(72756);
            if (!growAura)
                growAura = defile->GetAura(74162);

            if (growAura)
            {
                uint8 stacks = growAura->GetStackAmount();
                currentRadius = baseRadius + (stacks * 1.25f);
            }

            float dx = bot->GetPositionX() - defile->GetPositionX();
            float dy = bot->GetPositionY() - defile->GetPositionY();
            float distanceToCenter = sqrt(dx * dx + dy * dy);

            if (distanceToCenter < (currentRadius + safetyMargin))
            {
                needToMove = true;
                break;
            }
        }

        // If we need to move, find the safest direction
        if (needToMove)
        {
            // Try 16 different angles for more precise movement
            for (int i = 0; i < 16; i++)
            {
                float testAngle = i * M_PI / 8;
                float moveDistance = 5.0f; // Move in 5-yard increments
                
                float testX = bot->GetPositionX() + moveDistance * cos(testAngle);
                float testY = bot->GetPositionY() + moveDistance * sin(testAngle);
                float testZ = 840.857f;
                
                bot->UpdateAllowedPositionZ(testX, testY, testZ);

                // Skip if not in LOS or too much height difference
                if (!bot->IsWithinLOS(testX, testY, testZ) || 
                    fabs(testZ - bot->GetPositionZ()) >= 5.0f)
                {
                    continue;
                }

                // Calculate minimum distance to any defile from this position
                float minDefileDistance = std::numeric_limits<float>::max();
                float distanceToBoss = boss->GetDistance2d(testX, testY);
                
                for (Unit* defile : defiles)
                {
                    if (!defile || !defile->IsAlive())
                        continue;

                    float dx = testX - defile->GetPositionX();
                    float dy = testY - defile->GetPositionY();
                    float dist = sqrt(dx * dx + dy * dy);
                    minDefileDistance = std::min(minDefileDistance, dist);
                }

                // Favor positions that are both safe from defiles and closer to the boss
                float safetyScore = minDefileDistance;
                float bossScore = 100.0f - std::min(100.0f, distanceToBoss); // Convert distance to a 0-100 score
                float totalScore = safetyScore + (bossScore * 0.5f); // Weight safety more than boss proximity

                // If this position is better than our previous best, update it
                if (totalScore > maxSafeDistance)
                {
                    maxSafeDistance = totalScore;
                    bestAngle = testAngle;
                }
            }

            // Move in the best direction found
            if (maxSafeDistance > 0)
            {
                float moveDistance = 5.0f;
                float testX = bot->GetPositionX() + moveDistance * cos(bestAngle);
                float testY = bot->GetPositionY() + moveDistance * sin(bestAngle);
                float testZ = 840.857f;
                
                bot->UpdateAllowedPositionZ(testX, testY, testZ);
                return MoveTo(bot->GetMapId(), testX, testY, testZ,
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
    }

    // Check if LK is casting Defile - make bots spread
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(72762))
    {
        uint32 playerCount = 0;
        uint32 botIndex = 0;
        uint32 currentIndex = 0;
        
        Map::PlayerList const& players = bot->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player || !player->IsAlive())
                continue;
                
            if (player == bot)
                botIndex = currentIndex;
                
            currentIndex++;
            playerCount++;
        }

        // Calculate this bot's preferred angle based on its index
        float preferredAngle = (float(botIndex) / float(playerCount)) * 2 * M_PI;
        float moveDistance = 12.0f;  // Fixed distance for consistent spread
        
        // Start checking from preferred angle, then try adjacent angles if blocked
        float bestAngle = preferredAngle;
        bool foundSafeSpot = false;
        
        // Try positions at increasing offsets from preferred angle
        for (int offset = 0; offset <= 8; offset++)  // Try up to 8 positions on each side
        {
            for (int direction = -1; direction <= 1; direction += 2)  // Check both clockwise and counterclockwise
            {
                if (offset == 0 && direction > 0)  // Skip second check of preferred angle
                    continue;
                    
                float testAngle = preferredAngle + (direction * offset * M_PI / 16);
                float testX = bot->GetPositionX() + moveDistance * cos(testAngle);
                float testY = bot->GetPositionY() + moveDistance * sin(testAngle);
                float testZ = 840.857f;
                
                bot->UpdateAllowedPositionZ(testX, testY, testZ);
                
                if (!bot->IsWithinLOS(testX, testY, testZ) || 
                    fabs(testZ - bot->GetPositionZ()) >= 5.0f)
                {
                    continue;
                }

                // Check if position is safe from defiles
                bool isSafeFromDefiles = true;
                for (Unit* defile : defiles)
                {
                    if (!defile || !defile->IsAlive())
                        continue;

                    float currentRadius = 6.0f;
                    Aura* growAura = defile->GetAura(72756);
                    if (!growAura)
                        growAura = defile->GetAura(74162);

                    if (growAura)
                    {
                        uint8 stacks = growAura->GetStackAmount();
                        currentRadius = 6.0f + (stacks * 1.25f);
                    }

                    float dx = testX - defile->GetPositionX();
                    float dy = testY - defile->GetPositionY();
                    float distToDefile = sqrt(dx * dx + dy * dy);

                    if (distToDefile < (currentRadius + 1.0f))
                    {
                        isSafeFromDefiles = false;
                        break;
                    }
                }

                if (!isSafeFromDefiles)
                    continue;

                // Check distance to other players
                bool tooCloseToPlayers = false;
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* player = itr->GetSource();
                    if (!player || !player->IsAlive() || player == bot)
                        continue;
                        
                    float dx = testX - player->GetPositionX();
                    float dy = testY - player->GetPositionY();
                    float dist = sqrt(dx * dx + dy * dy);
                    
                    if (dist < 5.0f)  // Minimum spacing between players
                    {
                        tooCloseToPlayers = true;
                        break;
                    }
                }
                
                if (tooCloseToPlayers)
                    continue;

                float distanceToBoss = boss->GetDistance2d(testX, testY);
                if (distanceToBoss > 40.0f)
                    continue;

                // We found a safe spot
                bestAngle = testAngle;
                foundSafeSpot = true;
                break;
            }
            
            if (foundSafeSpot)
                break;
        }
        
        if (foundSafeSpot)
        {
            float testX = bot->GetPositionX() + moveDistance * cos(bestAngle);
            float testY = bot->GetPositionY() + moveDistance * sin(bestAngle);
            float testZ = 840.857f;
            
            bot->UpdateAllowedPositionZ(testX, testY, testZ);
            return MoveTo(bot->GetMapId(), testX, testY, testZ,
                false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    // Check for Val'kyr Shadowguard targeting real players
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() &&
            (unit->GetEntry() == 36609 || unit->GetEntry() == 39120 || unit->GetEntry() == 39121 ||
             unit->GetEntry() == 39122))  // Val'kyr Shadowguard entries
        {
            GuidVector npcs2 = AI_VALUE(GuidVector, "nearest hostile npcs");
            Unit* closestValkyr = nullptr;
            float minValkyrDistance = std::numeric_limits<float>::max();

            // First find the closest Val'kyr to this bot
            for (auto& npc2 : npcs2)
            {
                Unit* unit2 = botAI->GetUnit(npc2);
                if (unit2 && unit2->IsAlive() &&
                    (unit2->GetEntry() == 36609 || unit2->GetEntry() == 39120 || unit2->GetEntry() == 39121 ||
                     unit2->GetEntry() == 39122))  // Val'kyr Shadowguard entries
                {
                    float distance = bot->GetDistance(unit2);
                    if (distance < minValkyrDistance)
                    {
                        minValkyrDistance = distance;
                        closestValkyr = unit2;
                    }
                }
            }

            if (closestValkyr)
            {
                // Check if this Val'kyr is grabbing any player
                bool isGrabbingPlayer = false;
                Map::PlayerList const& playerList = closestValkyr->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
                    Player* player = itr->GetSource();
                    if (!player || !player->IsAlive())
                        continue;

                    // Check if player is being lifted by Val'kyr (close horizontally and elevated)
                    float horizontalDist =
                        std::sqrt(std::pow(closestValkyr->GetPositionX() - player->GetPositionX(), 2) +
                                  std::pow(closestValkyr->GetPositionY() - player->GetPositionY(), 2));
                    float verticalDist = player->GetPositionZ() - closestValkyr->GetPositionZ();

                    if (horizontalDist <= 1.0f && verticalDist > 0.5f)  // Player is close horizontally and lifted up
                    {
                        isGrabbingPlayer = true;
                        break;
                    }
                }

                if (isGrabbingPlayer)
                {
                    // Check if Valkyr is already CC'd
                    if (botAI->HasAura("Frost Nova", closestValkyr) || botAI->HasAura("Deep Freeze", closestValkyr) ||
                        botAI->HasAura("Entangling Roots", closestValkyr) ||
                        botAI->HasAura("Hammer of Justice", closestValkyr) ||
                        botAI->HasAura("Hamstring", closestValkyr) ||
                        botAI->HasAura("Concussive Shot", closestValkyr) ||
                        botAI->HasAura("Kidney Shot", closestValkyr) || botAI->HasAura("Gouge", closestValkyr) ||
                        botAI->HasAura("Frost Shock", closestValkyr) || botAI->HasAura("Chains of Ice", closestValkyr))
                    {
                        return Attack(closestValkyr);
                    }

                    // Try to CC the Val'kyr based on class priority - only stuns and slows
                    if (bot->getClass() == CLASS_MAGE)
                    {
                        if (botAI->CastSpell("Frost Nova", closestValkyr))
                            return true;
                        if (botAI->CastSpell("Deep Freeze", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_DRUID)
                    {
                        if (botAI->CastSpell("Entangling Roots", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_PALADIN)
                    {
                        if (botAI->CastSpell("Hammer of Justice", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_WARRIOR)
                    {
                        if (botAI->CastSpell("Hamstring", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_HUNTER)
                    {
                        if (botAI->CastSpell("Concussive Shot", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_ROGUE)
                    {
                        if (botAI->CastSpell("Kidney Shot", closestValkyr))
                            return true;
                        if (botAI->CastSpell("Gouge", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_SHAMAN)
                    {
                        if (botAI->CastSpell("Frost Shock", closestValkyr))
                            return true;
                    }
                    else if (bot->getClass() == CLASS_DEATH_KNIGHT)
                    {
                        if (botAI->CastSpell("Chains of Ice", closestValkyr))
                            return true;
                    }

                    // If no CC available or all failed, attack the Val'kyr
                    return Attack(closestValkyr);
                }
            }
        }
    }


    if (!botAI->IsAssistTank(bot) && !boss->HealthBelowPct(71))
    {
        // Check for necrotic plague
        bool hasPlague = botAI->HasAura("Necrotic Plague", bot);

        // Only execute if we dont have the plague
        if (!hasPlague)
        {
            float distanceToPosition = bot->GetDistance2d(ICC_LICH_KING_ADDS_POSITION.GetPositionX(), ICC_LICH_KING_ADDS_POSITION.GetPositionY());
            if (distanceToPosition < 15.0f)  // Only move if we're closer than 15 yards to adds position
            {
                // Move directly away from adds position in 5yd increments
                float angle = atan2(bot->GetPositionY() - ICC_LICH_KING_ADDS_POSITION.GetPositionY(), 
                                  bot->GetPositionX() - ICC_LICH_KING_ADDS_POSITION.GetPositionX());
                float moveDistance = 5.0f;  // Always move 5 yards away
                float x = bot->GetPositionX() + cos(angle) * moveDistance;  // Move from current position
                float y = bot->GetPositionY() + sin(angle) * moveDistance;
                float z = bot->GetPositionZ();  // Use bot's Z to prevent going underground
            
                return MoveTo(bot->GetMapId(), x, y, z,
                    false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }

   // Handle assist tanks - keep them at adds position
    if (botAI->IsAssistTank(bot))
    {
        // Actively look for any shambling/spirit/ghoul that isn't targeting us
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");
        for (auto i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (unit && unit->IsAlive() && (unit->GetEntry() == 37698 || unit->GetEntry() == 39299 || 
                    unit->GetEntry() == 39300 || unit->GetEntry() == 39301 || // Shambling entry
                    unit->GetEntry() == 36701 || unit->GetEntry() == 39302 ||
                    unit->GetEntry() == 39303 || unit->GetEntry() == 39304 || // Spirits entry
                    unit->GetEntry() == 37695 || unit->GetEntry() == 39309 ||
                    unit->GetEntry() == 39310 || unit->GetEntry() == 39311)) // Drudge Ghouls entry
            {
                if (!unit->GetVictim() || unit->GetVictim() != bot)
                {
                    bot->SetFacingToObject(unit);
                    return Attack(unit); // Pick up any shambling that isn't targeting us
                }
            }
        }

        // If we have a current target, make sure we're facing it
        if (Unit* currentTarget = bot->GetVictim())
        {
            bot->SetFacingToObject(currentTarget);
        }

        // Return to adds position if we're too far
        if (bot->GetExactDist2d(ICC_LICH_KING_ADDS_POSITION) > 3.0f && !boss->HealthBelowPct(71)) 
        {
            return MoveTo(bot->GetMapId(), ICC_LICH_KING_ADDS_POSITION.GetPositionX(), 
                        ICC_LICH_KING_ADDS_POSITION.GetPositionY(), 
                        ICC_LICH_KING_ADDS_POSITION.GetPositionZ(),
                        false,true, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        return false; // Stay in position and keep facing current target
    }

    if (botAI->IsMainTank(bot) && !boss->HealthBelowPct(71))
    {
        float currentDist = bot->GetDistance(ICC_LICH_KING_ADDS_POSITION);
        if (currentDist < 9.0f || currentDist > 22.0f)  // 15 yards ±3 yards tolerance
        {
            float angle = ICC_LICH_KING_ADDS_POSITION.GetAngle(bot);
            float targetDist = 16.0f;
            float moveDistance = currentDist < targetDist ? 5.0f : -5.0f;  // Move away or towards in 5yd increments
        
            float x = bot->GetPositionX() + cos(angle) * moveDistance;
            float y = bot->GetPositionY() + sin(angle) * moveDistance;
            float z = ICC_LICH_KING_ADDS_POSITION.GetPositionZ();
        
            return MoveTo(bot->GetMapId(), x, y, z,
                false, true, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    if (botAI->IsRanged(bot) && !boss->HealthBelowPct(71))
    {
        float currentDist = bot->GetDistance(ICC_LICH_KING_ADDS_POSITION);
        if (currentDist < 21.0f || currentDist > 35.0f)  // 20 yards ±3 yards tolerance
        {
            float angle = ICC_LICH_KING_ADDS_POSITION.GetAngle(bot);
            float targetDist = 26.0f;
            float moveDistance = currentDist < targetDist ? 5.0f : -5.0f;  // Move away or towards in 5yd increments
        
            float x = bot->GetPositionX() + cos(angle) * moveDistance;
            float y = bot->GetPositionY() + sin(angle) * moveDistance;
            float z = ICC_LICH_KING_ADDS_POSITION.GetPositionZ();
        
            return MoveTo(bot->GetMapId(), x, y, z,
                false, true, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }

    return false;
}
