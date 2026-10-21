// Pit of Saron ground hazard tactics: step out of Toxic Waste and Icy Blast, drag the boss clear, and stay out.

#include <limits>

#include "AiObjectContext.h"
#include "Cell.h"
#include "CellImpl.h"
#include "DungeonTactics.h"
#include "DynamicObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "PoSActions.h"
#include "PoSMultipliers.h"
#include "PoSTriggers.h"

std::vector<PitOfSaron::Hazard> FindPoSGroundHazards(WorldObject* searcher)
{
    std::list<WorldObject*> objects;
    Acore::AllWorldObjectsInRange check(searcher, PitOfSaron::HazardScanRange);
    Acore::WorldObjectListSearcher<Acore::AllWorldObjectsInRange> list(searcher, objects, check,
                                                                        GRID_MAP_TYPE_MASK_DYNAMICOBJECT);
    Cell::VisitObjects(searcher, list, PitOfSaron::HazardScanRange);

    std::vector<PitOfSaron::Hazard> hazards;
    for (WorldObject* object : objects)
    {
        DynamicObject* dynObj = object->ToDynObject();
        if (!dynObj || !dynObj->IsInWorld())
            continue;

        uint32 spell = dynObj->GetSpellId();
        if (spell == SPELL_TOXIC_WASTE || spell == SPELL_TOXIC_WASTE_HC || spell == SPELL_ICY_BLAST_AREA ||
            spell == SPELL_ICY_BLAST_AREA_HC)
            hazards.push_back({dynObj->GetPositionX(), dynObj->GetPositionY(), dynObj->GetRadius()});
    }
    return hazards;
}

Unit* FindPoSBoss(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (Unit* tyrannus = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus"))
        return tyrannus;
    return AI_VALUE2(Unit*, "find target", "ick");
}

bool IsPoisonNovaCasting(Unit* boss)
{
    return boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS) ||
                                                      boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS_HC));
}

bool PoSGroundHazardTrigger::IsActive()
{
    Unit* boss = FindPoSBoss(botAI);
    if (!boss)
        return false;

    std::vector<PitOfSaron::Hazard> hazards = FindPoSGroundHazards(bot);
    if (PitOfSaron::InHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), 1.0f))
        return true;

    return PlayerbotAI::IsTank(bot) && boss->GetVictim() == bot &&
           PitOfSaron::InHazard(hazards, boss->GetPositionX(), boss->GetPositionY(), 0.0f);
}

bool PoSAvoidGroundHazardAction::Execute(Event /*event*/)
{
    Unit* boss = FindPoSBoss(botAI);
    if (!boss)
        return false;

    std::vector<PitOfSaron::Hazard> hazards = FindPoSGroundHazards(bot);
    bool inHazard = PitOfSaron::InHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), 1.0f);
    bool nova = IsPoisonNovaCasting(boss);
    bool dragBoss = !inHazard && !nova && PlayerbotAI::IsTank(bot) &&
                    PitOfSaron::InHazard(hazards, boss->GetPositionX(), boss->GetPositionY(), 0.0f);
    if (!inHazard && !dragBoss)
        return false;

    // Melee step out on the side nearest the boss; the tank pulls the boss clear so he leaves the patch too;
    // if nothing is clear take the spot under the fewest patches rather than standing still
    bool stayNearBoss = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);
    float clearance = PitOfSaron::HazardClearance(dragBoss, boss->GetCombatReach());
    float bestScore = std::numeric_limits<float>::max();
    uint32 bestCovering = 0;
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (float step : PitOfSaron::HazardSteps)
    {
        for (int i = 0; i < PitOfSaron::HazardDirections; ++i)
        {
            float angle = float(i) * 2.0f * float(M_PI) / float(PitOfSaron::HazardDirections);
            float x = bot->GetPositionX() + step * std::cos(angle);
            float y = bot->GetPositionY() + step * std::sin(angle);
            float z = bot->GetPositionZ();
            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > PitOfSaron::MaxClimb || !bot->IsWithinLOS(x, y, z + 1.0f))
                continue;

            float bossDist = boss->GetExactDist2d(x, y);
            if (!PitOfSaron::StepClearsNova(nova, bossDist))
                continue;

            uint32 covering = PitOfSaron::HazardsCovering(hazards, x, y, clearance);
            float score = PitOfSaron::HazardStepScore(covering, stayNearBoss, bossDist, step);
            if (score < bestScore)
            {
                bestScore = score;
                bestCovering = covering;
                bestX = x;
                bestY = y;
                bestZ = z;
            }
        }
    }

    if (bestScore == std::numeric_limits<float>::max() || bot->GetExactDist2d(bestX, bestY) < 1.0f)
        return false;

    // Moving onto ground under as many patches as here gains nothing
    Unit* mover = dragBoss ? boss : bot;
    uint32 hereCovering =
        PitOfSaron::HazardsCovering(hazards, mover->GetPositionX(), mover->GetPositionY(), dragBoss ? 0.0f : 1.0f);
    if (bestCovering >= hereCovering)
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

float PoSHazardMultiplier::GetValue(Action* action)
{
    Unit* boss = FindPoSBoss(botAI);
    if (!boss || dynamic_cast<PoSAvoidGroundHazardAction*>(action))
        return 1.0f;

    DungeonTactics::ActionShape shape = DungeonTactics::ShapeOf(action);
    if (!DungeonTactics::IsRepositioning(shape))
        return 1.0f;

    bool meleeNotTank = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);
    return PitOfSaron::StaysOutOfHazards(FindPoSGroundHazards(bot), bot->GetPositionX(), bot->GetPositionY(),
                                         boss->GetPositionX(), boss->GetPositionY(), meleeNotTank)
               ? 1.0f
               : 0.0f;
}
