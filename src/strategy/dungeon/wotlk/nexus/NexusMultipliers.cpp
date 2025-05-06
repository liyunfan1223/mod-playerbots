#include "NexusMultipliers.h"
#include "NexusActions.h"
#include "GenericSpellActions.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "NexusTriggers.h"

float FactionCommanderMultiplier::GetValue(Action* action)
{
    Unit* boss = nullptr;
    uint8 faction = bot->GetTeamId();
    
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
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) &&
        boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
    {
        // Prevent movement actions other than flee during a whirlwind, to prevent running back in early.
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromWhirlwindAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float TelestraMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand magus telestra");
    if (boss && boss->GetEntry() != NPC_TELESTRA)
    {
        // boss is split into clones, do not auto acquire target
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float AnomalusMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anomalus");
    if (boss && boss->HasAura(BUFF_RIFT_SHIELD))
    {
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float OrmorokMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ormorok the tree-shaper");
    if (!boss) { return 1.0f; }
    
    // These are used for auto ranged repositioning, need to suppress so ranged dps don't ping-pong
    if (dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }
    // This boss is annoying and shuffles around a lot. Don't let tank move once fight has started.
    // Extra checks are to allow the tank to close distance and engage the boss initially
    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<DodgeSpikesAction*>(action)
        && botAI->IsTank(bot) && bot->IsWithinMeleeRange(boss)
        && AI_VALUE2(bool, "facing", "current target"))
        {
            return 0.0f;
        }
    return 1.0f;
}

