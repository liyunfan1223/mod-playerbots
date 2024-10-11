#include "Playerbots.h"
#include "NexusActions.h"
#include "NexusStrategy.h"

bool MoveFromWhirlwindAction::Execute(Event event)
{
    Unit* boss = nullptr;
    uint8 faction = bot->GetTeamId();
    float targetDist = 10.0f; // Whirlwind has range of 8, add a couple for safety buffer
    
    switch (bot->GetMap()->GetDifficulty())
    {
        case DUNGEON_DIFFICULTY_NORMAL:
            if (faction == TEAM_ALLIANCE)
            {
                boss = AI_VALUE2(Unit*, "find target", "horde commander");
            }
            else //if (faction == TEAM_HORDE)
            {
                boss = AI_VALUE2(Unit*, "find target", "alliance commander");
            }
            break;
        case DUNGEON_DIFFICULTY_HEROIC:
            if (faction == TEAM_ALLIANCE)
            {
                boss = AI_VALUE2(Unit*, "find target", "commander kolurg");
            }
            else //if (faction == TEAM_HORDE)
            {
                boss = AI_VALUE2(Unit*, "find target", "commander stoutbeard");
            }
            break;
        default:
            break;
    }
    if (!boss || bot->GetExactDist2d(boss->GetPosition()) > targetDist)
    {
        return false;
    }
    return MoveAway(boss, targetDist - bot->GetExactDist2d(boss->GetPosition()));
}

bool FirebombSpreadAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand magus telestra");
    float radius = 5.0f;
    float targetDist = radius + 1.0f;
    if (!boss) { return false; }

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (bot->GetGUID() == member)
        {
            continue;
        }
        if (bot->GetExactDist2d(botAI->GetUnit(member)) < targetDist)
        {
            return MoveAway(botAI->GetUnit(member), targetDist);
        }
    }
    return false;
}

bool TelestraSplitTargetAction::isUseful() { return !botAI->IsHeal(bot); }
bool TelestraSplitTargetAction::Execute(Event event)
{
    GuidVector attackers = AI_VALUE(GuidVector, "attackers");
    Unit* splitTargets[3] = {nullptr, nullptr, nullptr};

    for (auto& attacker : attackers)
    {
        Unit* npc = botAI->GetUnit(attacker);
        if (!npc)
        {
            continue;
        }
        switch (npc->GetEntry())
        {
            // Focus arcane clone first
            case NPC_ARCANE_MAGUS:
                splitTargets[0] = npc;
                break;
            // Then the frost clone
            case NPC_FROST_MAGUS:
                splitTargets[1] = npc;
                break;
            // Fire clone last
            case NPC_FIRE_MAGUS:
                splitTargets[2] = npc;
                break;
        }
    }

    for (Unit* target : splitTargets)
    {
        // Attack the first valid split target in the priority list
        if (target)
        {
            if (AI_VALUE(Unit*, "current target") != target)
            {
                return Attack(target);
            }
            // Don't continue loop here, the target exists so we don't
            // want to move down the prio list. We just don't need to send attack
            // command again, just return false and exit the loop that way
            return false;
        }
    }

    return false;
}

bool ChaoticRiftTargetAction::isUseful() { return !botAI->IsHeal(bot); }
bool ChaoticRiftTargetAction::Execute(Event event)
{
    Unit* chaoticRift = nullptr;

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetName() == "Chaotic Rift")
        {
            chaoticRift = unit;
            break;
        }
    }
    if (!chaoticRift || AI_VALUE(Unit*, "current target") == chaoticRift)
    {
        return false;
    }
    return Attack(chaoticRift);
}

bool DodgeSpikesAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ormorok the tree-shaper");
    return bot->GetExactDist2d(boss) > 0.5f;
}
bool DodgeSpikesAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ormorok the tree-shaper");
    return Move(bot->GetAngle(boss), bot->GetExactDist2d(boss) - 0.3f);
}

bool IntenseColdJumpAction::Execute(Event event)
{
    // This needs improving but maybe it should be done in the playerbot core.
    // Jump doesn't seem to support zero offset (eg. jump on the spot) so need to add a tiny delta.
    // This does a tiny bunnyhop that takes a couple of ms, it doesn't do a natural jump.
    // Adding extra Z offset causes floating, and appears to scale the jump speed based on Z difference.
    // Probably best to revisit once bot movement is improved
    return JumpTo(bot->GetMap()->GetId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() + 0.01f);
    // bot->GetMotionMaster()->MoveFall();
}

bool RearFlankPositionAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "keristrasza");
    if (!boss) { return false; }

    // Need to double the front angle check to account for mirrored angle.
    // Total 180 degrees (whole front half)
    bool inFront = boss->HasInArc(2.f * DRAGON_MELEE_MIN_ANGLE, bot);
    // Rear check does not need to double this angle as the logic is inverted
    // and we are subtracting from 2pi.
    bool inBack = !boss->HasInArc((2.f * M_PI) - DRAGON_MELEE_MAX_ANGLE, bot);

    return inFront || inBack;
}
bool RearFlankPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "keristrasza");
    if (!boss) { return false; }

    // float angleToMove = minAngle + rand_norm() * (maxAngle - minAngle);
    float angle = frand(DRAGON_MELEE_MIN_ANGLE, DRAGON_MELEE_MAX_ANGLE);
    // Need to reduce this value very slightly, or the bots get the jitters -
    // may be due to rounding errors. Need to bring them just inside their attack range.
    // This boss has a big hitbox so we can reduce by 50% and it's still fine and looks better.
    // TODO: Investigate using bot->GetObjectSize() for sizing
    float distance = bot->GetMeleeRange(boss) * 0.5f;
    // Alternatively, summing both unit's melee ranges seems to give a fairly natural range.
    // Use whichever gives the best results..
    // float distanceOffset = bot->GetMeleeReach() + boss->GetMeleeReach();
    Position leftFlank = boss->GetPosition();
    Position rightFlank = boss->GetPosition();
    Position* destination = nullptr;
    leftFlank.RelocatePolarOffset(angle, distance);
    rightFlank.RelocatePolarOffset(-angle, distance);

    if (bot->GetExactDist2d(leftFlank) < bot->GetExactDist2d(rightFlank))
    {
        destination = &leftFlank;
    }
    else
    {
        destination = &rightFlank;
    }

    return MoveTo(bot->GetMapId(), destination->GetPositionX(), destination->GetPositionY(), destination->GetPositionZ());
}
