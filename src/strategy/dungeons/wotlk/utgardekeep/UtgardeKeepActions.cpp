#include "Playerbots.h"
#include "UtgardeKeepActions.h"
#include "UtgardeKeepStrategy.h"

bool AttackFrostTombAction::isUseful() { return !botAI->IsHeal(bot); }
bool AttackFrostTombAction::Execute(Event event)
{
    Unit* frostTomb = nullptr;

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_FROST_TOMB)
        {
            frostTomb = unit;
            break;
        }
    }
    if (!frostTomb || AI_VALUE(Unit*, "current target") == frostTomb)
    {
        return false;
    }
    return Attack(frostTomb);
}

// TODO: Possibly add player stacking behaviour close to tank, to prevent Skarvald charging ranged
bool AttackDalronnAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalronn the controller");
    if (!boss) { return false; }
    
    if (AI_VALUE(Unit*, "current target") == boss)
    {
        return false;
    }
    return Attack(boss);
}

bool IngvarStopCastingAction::Execute(Event event)
{
    // Doesn't work, this action gets queued behind the current spell instead of interrupting it
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    int32 my_spell_id = AI_VALUE(uint32, "active spell");
    if (!my_spell_id || my_spell_id == 0)
    {
        return false;
    }
    
    Spell* spell = bot->FindCurrentSpellBySpellId(my_spell_id);
    if (!spell) { return false; }

    // bot->Yell("cancelling spell="+std::to_string(my_spell_id), LANG_UNIVERSAL);
    bot->InterruptSpell(spell->GetCurrentContainer(), false, true, true);
    
    // Can slightly optimise by allowing bot to keep casting if they will finish the cast
    // before boss spell goes off, however need to hook boss AI for cast remaining.
    return true;
}

bool IngvarDodgeSmashAction::isUseful() { return !AI_VALUE2(bool, "behind", "current target"); }
bool IngvarDodgeSmashAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    // Extra units to move into the boss, instead of being just 1 pixel past his midpoint.
    // Can be adjusted - this value tends to mirror how a human would play,
    // and visibly ensures you won't get hit while not creating excessive movements.
    float distanceExtra = 2.0f;
    return Move(bot->GetAngle(boss), distance + distanceExtra);
}

bool IngvarSmashReturnAction::isUseful() { return AI_VALUE2(bool, "behind", "current target"); }
bool IngvarSmashReturnAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    if (!boss) { return false; }

    float distance = bot->GetExactDist2d(boss->GetPosition());
    return Move(bot->GetAngle(boss), distance + bot->GetMeleeReach());
}
