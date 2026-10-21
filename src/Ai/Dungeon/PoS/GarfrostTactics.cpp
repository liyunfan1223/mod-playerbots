// Forgemaster Garfrost bot tactics: hide behind boulders from Permafrost, dodge Throw Saronite, tank him mid-room.

#include <limits>

#include "DungeonTactics.h"
#include "GameObject.h"
#include "Group.h"
#include "Playerbots.h"
#include "PoSActions.h"
#include "PoSMultipliers.h"
#include "PoSTriggers.h"
#include "Spell.h"

namespace
{
// Between the two forges: the group fights here so boulders (and cover) land mid-room, and waits here during the leaps
Position const GARFROST_ROOM_MIDDLE = Position(682.0f, -212.0f, 527.2f);

uint32 PermafrostStacks(Unit* unit)
{
    for (uint32 spell : {uint32(SPELL_PERMAFROST_DEBUFF), uint32(SPELL_PERMAFROST_DEBUFF_HC)})
        if (Aura* aura = unit->GetAura(spell))
            return aura->GetStackAmount();
    return 0;
}

bool IsGarfrostForging(Unit* boss)
{
    Creature* creature = boss->ToCreature();
    return (creature && creature->GetReactState() == REACT_PASSIVE) ||
           boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STOMP) ||
           boss->FindCurrentSpellBySpellId(SPELL_FORGE_BLADE) || boss->FindCurrentSpellBySpellId(SPELL_FORGE_MACE);
}

bool IsShieldedFromGarfrost(Unit* boss, Unit* unit)
{
    // Same test as spell_garfrost_permafrost: a boulder within 4 yd of the line, and not in his melee range
    if (boss->IsWithinMeleeRange(unit))
        return false;

    std::list<GameObject*> rocks;
    boss->GetGameObjectListWithEntryInGrid(rocks, GO_SARONITE_ROCK, 100.0f);
    for (GameObject* rock : rocks)
        if (!rock->IsInvisibleDueToDespawn() && rock->IsInBetween(boss, unit, 4.0f))
            return true;
    return false;
}

PitOfSaron::Cover CoverNeed(Player* bot, Unit* boss)
{
    PitOfSaron::Role role = PlayerbotAI::IsTank(bot)     ? PitOfSaron::Role::Tank
                            : PlayerbotAI::IsRanged(bot) ? PitOfSaron::Role::Ranged
                                                         : PitOfSaron::Role::Melee;
    return PitOfSaron::CoverNeed(role, PermafrostStacks(bot), IsGarfrostForging(boss));
}

bool GetSaroniteLanding(Unit* boss, Position& landing)
{
    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_THROW_SARONITE);
    if (!spell)
        return false;

    // The boulder lands where its target stands when the cast ends (TARGET_DEST_TARGET_ANY), so it follows them
    if (Unit* target = spell->m_targets.GetUnitTarget())
    {
        landing.Relocate(*target);
        return true;
    }
    if (spell->m_targets.HasDst())
    {
        landing.Relocate(*spell->m_targets.GetDstPos());
        return true;
    }
    return false;
}

Player* FindGroupHealer(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->IsInWorld() && PlayerbotAI::IsHeal(member))
            return member;
    }
    return nullptr;
}
}  // namespace

bool GarfrostPermafrostTrigger::IsActive()
{
    // A bot that only stays hidden while shielded never goes looking for a boulder
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    return boss && CoverNeed(bot, boss) == PitOfSaron::Cover::Yes && !IsShieldedFromGarfrost(boss, bot);
}

bool GarfrostTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    return boss && PlayerbotAI::IsTank(bot) && boss->GetVictim() == bot && !IsGarfrostForging(boss);
}

bool GarfrostSaroniteTrigger::IsActive()
{
    // The target moves off the group; everyone else gets clear of the target
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    Position landing;
    return boss && GetSaroniteLanding(boss, landing) &&
           bot->GetExactDist2d(landing.GetPositionX(), landing.GetPositionY()) < PitOfSaron::SaroniteAvoidRadius;
}

bool GarfrostTakeCoverAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    if (!boss)
        return false;

    // Hide on the far side of a boulder, near the healer's, and casters within range of the boss
    std::list<GameObject*> rocks;
    boss->GetGameObjectListWithEntryInGrid(rocks, GO_SARONITE_ROCK, 100.0f);
    bool healer = PlayerbotAI::IsHeal(bot);
    Player* groupHealer = healer ? nullptr : FindGroupHealer(bot);
    Unit* tank = boss->GetVictim();
    bool forging = IsGarfrostForging(boss);
    bool ranged = PlayerbotAI::IsRanged(bot);

    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (GameObject* rock : rocks)
    {
        if (rock->IsInvisibleDueToDespawn())
            continue;

        float angle = boss->GetAngle(rock);
        for (float back : PitOfSaron::CoverBehindRock)
        {
            // No line-of-sight test from the bot: the boulder itself blocks the view of the spot behind it
            float x = rock->GetPositionX() + back * std::cos(angle);
            float y = rock->GetPositionY() + back * std::sin(angle);
            float z = bot->GetPositionZ();
            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > PitOfSaron::CoverMaxClimb)
                continue;

            if (!PitOfSaron::CoverInCastRange(ranged, forging, boss->GetExactDist2d(x, y)))
                continue;

            // Boulders block heals too: the healer never hides where the tank drops out of sight
            if (healer && !forging && tank && tank != bot &&
                (tank->GetExactDist2d(x, y) > PitOfSaron::HealerTankRange || !tank->IsWithinLOS(x, y, z + 1.0f)))
                continue;

            float score = PitOfSaron::CoverScore(bot->GetExactDist2d(x, y), GARFROST_ROOM_MIDDLE.GetExactDist2d(x, y),
                                                 groupHealer != nullptr,
                                                 groupHealer ? groupHealer->GetExactDist2d(x, y) : 0.0f,
                                                 groupHealer && groupHealer->IsWithinLOS(x, y, z + 1.0f));
            if (score < bestScore)
            {
                bestScore = score;
                bestX = x;
                bestY = y;
                bestZ = z;
            }
        }
    }

    if (bestScore == std::numeric_limits<float>::max() || bot->GetExactDist2d(bestX, bestY) < 1.0f)
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool GarfrostTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    if (!boss || IsGarfrostForging(boss) || boss->GetVictim() != bot)
        return false;

    float step = PitOfSaron::WalkBackStep(bot->GetExactDist2d(GARFROST_ROOM_MIDDLE));
    if (step <= 0.0f)
        return false;

    float angle = bot->GetAngle(&GARFROST_ROOM_MIDDLE);
    float x = bot->GetPositionX() + step * std::cos(angle);
    float y = bot->GetPositionY() + step * std::sin(angle);
    float z = bot->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);
    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool GarfrostDodgeSaroniteAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    Position landing;
    if (!boss || !GetSaroniteLanding(boss, landing))
        return false;

    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_THROW_SARONITE);
    bool isTarget = spell && spell->m_targets.GetUnitTarget() == bot;
    std::vector<Player*> others;
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member != bot && member->IsAlive() && member->IsInWorld() && member->GetMapId() == bot->GetMapId())
                    others.push_back(member);

    float centerX = isTarget ? bot->GetPositionX() : landing.GetPositionX();
    float centerY = isTarget ? bot->GetPositionY() : landing.GetPositionY();
    float radius = PitOfSaron::SaroniteAvoidRadius + 1.0f;
    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (int i = 0; i < PitOfSaron::SaroniteDirections; ++i)
    {
        float angle = float(i) * 2.0f * float(M_PI) / float(PitOfSaron::SaroniteDirections);
        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);
        float z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        if (std::fabs(z - bot->GetPositionZ()) > PitOfSaron::MaxClimb || !bot->IsWithinLOS(x, y, z + 1.0f))
            continue;

        float nearest = std::numeric_limits<float>::max();
        if (isTarget)
            for (Player* other : others)
                nearest = std::min(nearest, other->GetExactDist2d(x, y));

        float score = PitOfSaron::SaroniteSpotScore(isTarget, bot->GetExactDist2d(x, y), nearest);
        if (score < bestScore)
        {
            bestScore = score;
            bestX = x;
            bestY = y;
            bestZ = z;
        }
    }

    if (bestScore == std::numeric_limits<float>::max())
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

float GarfrostMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "forgemaster garfrost");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<GarfrostTakeCoverAction*>(action) || dynamic_cast<GarfrostDodgeSaroniteAction*>(action) ||
        dynamic_cast<GarfrostTankPositionAction*>(action))
        return 1.0f;

    // Only movement and attacks are held behind a boulder, and the boulder test is a grid search
    DungeonTactics::ActionShape shape = DungeonTactics::ShapeOf(action);
    bool forging = IsGarfrostForging(boss);
    bool hiding = shape.movement && !shape.avoidAoe && !forging && CoverNeed(bot, boss) != PitOfSaron::Cover::No &&
                  IsShieldedFromGarfrost(boss, bot);
    return PitOfSaron::GarfrostAllows(shape, forging, hiding, PlayerbotAI::IsRanged(bot)) ? 1.0f : 0.0f;
}
