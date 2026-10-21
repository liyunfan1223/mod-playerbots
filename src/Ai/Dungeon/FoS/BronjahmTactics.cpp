// Bronjahm bot tactics: step away with Corrupt Soul, kill soul fragments, hold the tank spot, close in for Soulstorm.

#include "ChooseTargetActions.h"
#include "FoSActions.h"
#include "FoSMultipliers.h"
#include "FoSPolicy.h"
#include "FoSTriggers.h"
#include "Group.h"
#include "Playerbots.h"

namespace
{
Position const BRONJAHM_TANK_POSITION = Position(5297.920f, 2506.698f, 686.068f);

bool SoulFragmentAlive(Player* bot)
{
    Unit* soul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    return soul && soul->IsAlive();
}

bool SoulstormActive(PlayerbotAI* botAI, Unit* boss)
{
    return botAI->GetAura("soulstorm", boss) || boss->FindCurrentSpellBySpellId(SPELL_SOULSTORM_VISUAL) ||
           boss->FindCurrentSpellBySpellId(SPELL_SOULSTORM_VISUAL2);
}
}  // namespace

bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    return boss && boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL) && bot->HasAura(SPELL_CORRUPT_SOUL);
}

bool SwitchToSoulFragment::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "bronjahm") && SoulFragmentAlive(bot);
}

bool BronjahmPositionTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "bronjahm") && !bot->HasAura(SPELL_CORRUPT_SOUL);
}

bool MoveFromBronjahmAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->GetExactDist2d(boss) < ForgeOfSouls::CorruptSoulFleeRange)
        return FleePosition(boss->GetPosition(), ForgeOfSouls::CorruptSoulFleeDistance, 2000U);
    return true;
}

bool AttackCorruptedSoulFragmentAction::Execute(Event /*event*/)
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_CORRUPTED_SOUL_FRAGMENT)
            continue;

        // Skull the fragment so the whole group switches to it
        if (Group* group = bot->GetGroup())
            if (group->GetTargetIcon(7) != unit->GetGUID())
                group->SetTargetIcon(7, bot->GetGUID(), unit->GetGUID());
        break;
    }

    return false;
}

bool BronjahmGroupPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    bool soulAlive = SoulFragmentAlive(bot);
    if (!PlayerbotAI::IsTank(bot) || !botAI->HasAggro(boss))
    {
        float step = ForgeOfSouls::SoulstormStep(PlayerbotAI::IsRanged(bot), bot->GetExactDist2d(boss), soulAlive,
                                                 SoulstormActive(botAI, boss));
        return step > 0.0f && Move(bot->GetAngle(boss), step);
    }

    if (!soulAlive)
    {
        if (bot->GetExactDist2d(BRONJAHM_TANK_POSITION) <= ForgeOfSouls::TankSpotTolerance)
            return false;

        return MoveTo(bot->GetMapId(), BRONJAHM_TANK_POSITION.GetPositionX(), BRONJAHM_TANK_POSITION.GetPositionY(),
                      BRONJAHM_TANK_POSITION.GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_NORMAL);
    }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Unit* soul = botAI->GetUnit(guid);
        if (!soul || soul->GetEntry() != NPC_CORRUPTED_SOUL_FRAGMENT)
            continue;

        float soulToBoss = soul->GetExactDist2d(boss);
        if (ForgeOfSouls::SoulBetweenTankAndBoss(bot->GetExactDist2d(boss), soulToBoss, soul->GetExactDist2d(bot)))
        {
            // Step behind the boss as seen from the soul, so it has to pass him to reach the tank
            float angle = Position::NormalizeOrientation(boss->GetAngle(soul) + float(M_PI));
            float x = boss->GetPositionX() + ForgeOfSouls::BehindBossStep * std::cos(angle);
            float y = boss->GetPositionY() + ForgeOfSouls::BehindBossStep * std::sin(angle);
            return MoveTo(bot->GetMapId(), x, y, boss->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_NORMAL);
        }

        if (soulToBoss < ForgeOfSouls::SoulNearBossRange)
            return FleePosition(soul->GetPosition(), ForgeOfSouls::SoulFleeDistance, 1000U);

        // A soul is up but clear of both: hold here rather than walk to the tank spot
        bot->SetFacingToObject(boss);
        return true;
    }

    return false;
}

bool BronjahmGroupPositionAction::isUseful() { return true; }

float BronjahmMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "bronjahm"))
        return 1.0f;

    bool tankAssist = dynamic_cast<TankAssistAction*>(action) != nullptr;
    bool fleeing = dynamic_cast<MoveFromBronjahmAction*>(action) != nullptr;
    bool movement = dynamic_cast<MovementAction*>(action) != nullptr;
    return ForgeOfSouls::BronjahmAllows(tankAssist, fleeing, movement, bot->HasAura(SPELL_CORRUPT_SOUL)) ? 1.0f : 0.0f;
}

float AttackFragmentMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsDps(bot) || !dynamic_cast<BronjahmGroupPositionAction*>(action))
        return 1.0f;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
            return 0.0f;
    }
    return 1.0f;
}
