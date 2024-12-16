#include "RaidOsActions.h"
#include "RaidOsTriggers.h"

#include "Playerbots.h"

/**
 * Executes the action for positioning the tank for the Sartharion encounter.
 * 
 * @param event The event that triggered this action.
 * @return True if the action was executed successfully, false otherwise.
 */
bool SartharionTankPositionAction::Execute(Event event)
{
    // Find the Sartharion boss unit
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    // Initialize drake units to nullptr
    Unit* drakes[3] = { nullptr, nullptr, nullptr };

    // Detect incoming drakes before they are on aggro table
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (!unit) { continue; }

        // Assign the unit to the corresponding drake variable based on its entry ID
        switch (unit->GetEntry())
        {
            case NPC_SHADRON:
                drakes[0] = unit;
                break;
            case NPC_TENEBRON:
                drakes[1] = unit;
                break;
            case NPC_VESPERON:
                drakes[2] = unit;
                break;
            default:
                continue;
        }
    }

    // Get the current position of the bot
    Position currentPos = bot->GetPosition();
    // Adjustable, this is the acceptable distance to stack point that will be accepted as "safe"
    float looseDistance = 12.0f;

    // Main tank positioning
    if (botAI->IsMainTank(bot))
    {
        // Move to the main tank position if the bot is too far from it
        if (bot->GetExactDist2d(SARTHARION_MAINTANK_POSITION.first, SARTHARION_MAINTANK_POSITION.second) > looseDistance)
        {
            return MoveTo(OS_MAP_ID, SARTHARION_MAINTANK_POSITION.first, SARTHARION_MAINTANK_POSITION.second, currentPos.GetPositionZ(),
                false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    // Offtank grab drakes
    else
    {
        float triggerDistance = 100.0f;
        for (Unit* drake : drakes)
        {
            if (drake && bot->GetExactDist2d(drake) < triggerDistance &&
                drake->GetTarget() != bot->GetGUID() && AI_VALUE(Unit*, "current target") != drake)
            {
                return Attack(drake);
            }
        }

        // Check if any drake is in combat and move to the offtank position if necessary
        bool drakeInCombat = std::any_of(std::begin(drakes), std::end(drakes), [&](Unit* drake) {
            return drake && bot->GetExactDist2d(drake) < triggerDistance;
        });

        if (drakeInCombat && bot->GetExactDist2d(SARTHARION_OFFTANK_POSITION.first, SARTHARION_OFFTANK_POSITION.second) > looseDistance)
        {
            return MoveTo(OS_MAP_ID, SARTHARION_OFFTANK_POSITION.first, SARTHARION_OFFTANK_POSITION.second, currentPos.GetPositionZ(),
                false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    return false;
}

/**
 * Executes the action to avoid the Twilight Fissure.
 * 
 * @param event The event that triggered this action.
 * @return True if the action was executed successfully, false otherwise.
 */
bool AvoidTwilightFissureAction::Execute(Event event)
{
    const float radius = 5.0f;

    // Get the nearest hostile NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_TWILIGHT_FISSURE)
        {
            float currentDistance = bot->GetDistance2d(unit);
            // Move away from the Twilight Fissure if the bot is too close
            if (currentDistance < radius)
            {
                return MoveAway(unit, radius - currentDistance);
            }
        }
    }
    return false;
}

/**
 * Executes the action to avoid the Flame Tsunami.
 * 
 * @param event The event that triggered this action.
 * @return True if the action was executed successfully, false otherwise.
 */
bool AvoidFlameTsunamiAction::Execute(Event event)
{
    // Adjustable, this is the acceptable distance to stack point that will be accepted as "safe"
    float looseDistance = 4.0f;

    // Get the nearest hostile NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_FLAME_TSUNAMI)
        {
            Position currentPos = bot->GetPosition();

            // Determine the wave direction based on the Y position of the unit
            int posY = static_cast<int>(unit->GetPositionY());
            bool wavePassed = (posY == 505 || posY == 555) ? currentPos.GetPositionX() > unit->GetPositionX() : currentPos.GetPositionX() < unit->GetPositionX();
            if (wavePassed)
            {
                return false;
            }

            // Move to the safe position based on the wave direction and bot's role
            float safePosition = (posY == 505 || posY == 555) ? TSUNAMI_RIGHT_SAFE_ALL : (botAI->IsMelee(bot) ? TSUNAMI_LEFT_SAFE_MELEE : TSUNAMI_LEFT_SAFE_RANGED);
            if (bot->GetExactDist2d(currentPos.GetPositionX(), safePosition) > looseDistance)
            {
                return MoveTo(OS_MAP_ID, currentPos.GetPositionX(), safePosition, currentPos.GetPositionZ(),
                    false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    return false;
}

/**
 * Executes the action to prioritize attacking targets in the Sartharion encounter.
 * 
 * @param event The event that triggered this action.
 * @return True if the action was executed successfully, false otherwise.
 */
bool SartharionAttackPriorityAction::Execute(Event event)
{
    // Find the target units for the encounter
    Unit* targets[] = {
        AI_VALUE2(Unit*, "find target", "acolyte of shadron"),
        AI_VALUE2(Unit*, "find target", "vesperon"),
        AI_VALUE2(Unit*, "find target", "tenebron"),
        AI_VALUE2(Unit*, "find target", "shadron"),
        AI_VALUE2(Unit*, "find target", "sartharion")
    };

    // Prioritize targets based on their importance
    for (Unit* target : targets)
    {
        if (target && AI_VALUE(Unit*, "current target") != target)
        {
            return Attack(target);
        }
    }

    return false;
}

/**
 * Executes the action to enter the Twilight Portal.
 * 
 * @param event The event that triggered this action.
 * @return True if the action was executed successfully, false otherwise.
 */
bool EnterTwilightPortalAction::Execute(Event event)
{
    // Find the Sartharion boss unit and check if it has the "Gift of Twilight Fire" aura
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss || !boss->HasAura(SPELL_GIFT_OF_TWILIGHT_FIRE)) { return false; }

    // Find the nearest Twilight Portal game object
    GameObject* portal = bot->FindNearestGameObject(GO_TWILIGHT_PORTAL, 100.0f);
    if (!portal) { return false; }

    // Move to the portal if the bot is not at the interact distance
    if (!portal->IsAtInteractDistance(bot))
    {
        return MoveTo(portal, fmaxf(portal->GetInteractionDistance() - 1.0f, 0.0f));
    }

    // Go through the portal
    WorldPacket data1(CMSG_GAMEOBJ_USE);
    data1 << portal->GetGUID();
    bot->GetSession()->HandleGameObjectUseOpcode(data1);

    return true;
}

/**
 * Executes the action to exit the Twilight Portal.
 * 
 * @param event The event that triggered this action.
 * @return True if the action was executed successfully, false otherwise.
 */
bool ExitTwilightPortalAction::Execute(Event event)
{
    // Find the nearest normal portal game object
    GameObject* portal = bot->FindNearestGameObject(GO_NORMAL_PORTAL, 100.0f);
    if (!portal) { return false; }

    // Move to the portal if the bot is not at the interact distance
    if (!portal->IsAtInteractDistance(bot))
    {
        return MoveTo(portal, fmaxf(portal->GetInteractionDistance() - 1.0f, 0.0f));
    }

    // Go through the portal
    WorldPacket data1(CMSG_GAMEOBJ_USE);
    data1 << portal->GetGUID();
    bot->GetSession()->HandleGameObjectUseOpcode(data1);

    return true;
}
