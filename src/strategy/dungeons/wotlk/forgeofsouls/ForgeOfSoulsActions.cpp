#include "Playerbots.h"
#include "ForgeOfSoulsActions.h"
#include "ForgeOfSoulsStrategy.h"
#include "SharedDefines.h"

bool MoveFromBronjahmAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->GetExactDist2d(boss) < 10.0f)
        return FleePosition(boss->GetPosition(), 15.0f, 2000U);
    else
        return true;

    return false;
}


bool AttackCorruptedSoulFragmentAction::Execute(Event event)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    // If no valid skull target, search for corrupted soul fragment
    Unit* empoweredPrince = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
                empoweredPrince = unit;

                // Mark corrupted soul fragment with skull if in group and not already marked
                if (Group* group = bot->GetGroup())
                {
                    ObjectGuid currentSkullGuid = group->GetTargetIcon(7);
                    if (currentSkullGuid.IsEmpty() || currentSkullGuid != unit->GetGUID())
                    {
                        group->SetTargetIcon(7, bot->GetGUID(), unit->GetGUID());  // 7 = skull
                    }
                }
                break;
        }

    }

    return false;
}


bool BronjahmGroupPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("soulstorm", boss);
    bool hasAura = aura;

    Unit* corruptedSoul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    bool activeSoulExists = corruptedSoul && corruptedSoul->IsAlive();

    if (botAI->IsTank(bot) && botAI->HasAggro(boss))
    {
        // If any corrupted soul exists, handle positioning carefully
        if (activeSoulExists)
        {
            GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (auto& npc : npcs)
            {
                Unit* unit = botAI->GetUnit(npc);
                if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
                {
                    // Soul exists - check positions
                    float soulToBossDist = unit->GetExactDist2d(boss);
                    float tankToBossDist = bot->GetExactDist2d(boss);
                    float soulToTankDist = unit->GetExactDist2d(bot);

                    // Handle edge case: if soul is between tank and boss
                    // This happens when: (tankToBossDist > soulToBossDist) AND (soulToTankDist < tankToBossDist)
                    if (tankToBossDist > soulToBossDist && soulToTankDist < tankToBossDist)
                    {
                        // Calculate position 5 yards behind boss from the soul's perspective
                        float angle = boss->GetAngle(unit);                                  // Angle from boss to soul
                        float oppositeAngle = Position::NormalizeOrientation(angle + M_PI);  // Opposite direction

                        // Calculate position 5 yards behind boss
                        float x = boss->GetPositionX() + 5.0f * cos(oppositeAngle);
                        float y = boss->GetPositionY() + 5.0f * sin(oppositeAngle);
                        float z = boss->GetPositionZ();

                        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                                      MovementPriority::MOVEMENT_NORMAL);
                    }

                    // If soul is near boss, flee from boss
                    if (soulToBossDist < 10.0f)
                        return FleePosition(unit->GetPosition(), 13.0f, 1000U);

                    // If soul exists but none of the above conditions, don't move to tank position yet
                    bot->SetFacingToObject(boss);
                    return true;
                }
            }
        }
        else
        {
            // No souls exist, safe to move to tank position
            if (bot->GetExactDist2d(BRONJAHM_TANK_POSITION) > 5.0f)
            {
                return MoveTo(bot->GetMapId(), BRONJAHM_TANK_POSITION.GetPositionX(),
                              BRONJAHM_TANK_POSITION.GetPositionY(), BRONJAHM_TANK_POSITION.GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }
    else
    {
        float maxMovement = 10.0f;

        if (bot->GetExactDist2d(boss) > maxMovement && !activeSoulExists && (hasAura || boss->FindCurrentSpellBySpellId(SPELL_SOULSTORM_VISUAL) || boss->FindCurrentSpellBySpellId(SPELL_SOULSTORM_VISUAL2)))
        {
            if (botAI->IsRanged(bot))
            {
                return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 6.5f, maxMovement));
            }
            else
            {
                return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 2.0f, maxMovement));
            }
        }
        else
            return false;
    }

    return false;
}

bool BronjahmGroupPositionAction::isUseful() { return true; }

bool DevourerOfSoulsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("mirrored soul", boss);
    bool hasAura = aura;

    if (!botAI->IsTank(bot) && !botAI->IsHeal(bot) && hasAura)
    {
        // Calculate the opposite direction
        float angle = bot->GetAngle(boss);
        float newAngle = Position::NormalizeOrientation(angle + M_PI);  // Add 180 degrees (PI radians)

        // Set the bot's orientation to face away from DoS = no attacks
        bot->SetFacingTo(newAngle);
        return true;
    }

    return false;
}


