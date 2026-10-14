/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

// Lord Marrowgar

// Group iteration filter for same-instance, alive, in-world members.
static bool IsValidLmMember(Player* member, Player* bot)
{
    if (!member || !member->IsInWorld() || !member->IsAlive())
        return false;
    if (member->GetMapId() != bot->GetMapId())
        return false;
    if (member->GetInstanceId() != bot->GetInstanceId())
        return false;
    if (member->HasAura(SPELL_LM_IMPALED))
        return false;
    return true;
}

// Up to two lowest-GUID ranged bots in same instance, hunter-priority.
static std::vector<Player*> PickBoneStormRangedTargets(Player* bot)
{
    std::vector<Player*> result;

    Group* group = bot->GetGroup();
    if (!group)
        return result;

    std::vector<Player*> ranged;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!IsValidLmMember(member, bot))
            continue;
        if (PlayerbotAI::IsTank(member))
            continue;
        if (!PlayerbotAI::IsRanged(member))
            continue;

        ranged.push_back(member);
    }

    if (ranged.empty())
        return result;

    std::sort(ranged.begin(), ranged.end(),
              [](Player const* a, Player const* b) { return a->GetGUID() < b->GetGUID(); });

    for (Player* p : ranged)
    {
        if (p->getClass() != CLASS_HUNTER)
            continue;
        result.push_back(p);
        if (result.size() == 2)
            return result;
    }

    for (Player* p : ranged)
    {
        if (p->getClass() == CLASS_HUNTER)
            continue;
        result.push_back(p);
        if (result.size() == 2)
            return result;
    }

    return result;
}

// True if any coldflame line sits within 10f of the anchor position.
// Used to widen the tank's "stay-put" tolerance so AvoidAoe can move them
// off the line without IccLmTankPositionAction dragging them back.
static bool ColdflameNearAnchor(Player* bot, Position const& anchor, float leash)
{
    std::list<Creature*> coldflames;
    bot->GetCreatureListWithEntryInGrid(coldflames, NPC_COLDFLAME, 200.0f);
    for (Creature* c : coldflames)
        if (c->GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY()) < leash)
            return true;
    return false;
}

bool IccLmTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    bool const isBossInBoneStorm = botAI->GetAura("Bone Storm", boss) != nullptr;
    float const maxDistanceThreshold = 3.0f;

    if (isBossInBoneStorm)
    {
        std::vector<Player*> const rangedTargets = PickBoneStormRangedTargets(bot);
        if (std::find(rangedTargets.begin(), rangedTargets.end(), bot) != rangedTargets.end())
        {
            float const anchorDist = bot->GetExactDist2d(ICC_LM_BONE_STORM_AT_POSITION.GetPositionX(),
                                                         ICC_LM_BONE_STORM_AT_POSITION.GetPositionY());

            float const bossDist = bot->GetExactDist2d(boss);
            float const proximityTrigger = 20.0f;
            float const leash = 10.0f;

            // Boss too close or standing in coldflame: reposition within leash from anchor
            bool const bossNear = bossDist < proximityTrigger;
            bool const inColdflame = [&]()
            {
                std::list<Creature*> coldflames;
                bot->GetCreatureListWithEntryInGrid(coldflames, NPC_COLDFLAME, 4.0f);
                return !coldflames.empty();
            }();

            if (bossNear || inColdflame)
            {
                // Try eight candidate offsets from the anchor at the leash radius;
                // pick first one that is far from boss and clear of coldflames.
                float bestX = bot->GetPositionX();
                float bestY = bot->GetPositionY();
                float bestScore = -1.0f;
                bool found = false;

                for (int i = 0; i < 8; ++i)
                {
                    float const angle = (float)i * (float)M_PI / 4.0f;
                    float const cx = ICC_LM_BONE_STORM_AT_POSITION.GetPositionX() + std::cos(angle) * leash;
                    float const cy = ICC_LM_BONE_STORM_AT_POSITION.GetPositionY() + std::sin(angle) * leash;

                    float const dx = cx - boss->GetPositionX();
                    float const dy = cy - boss->GetPositionY();
                    float const distToBoss = std::sqrt(dx * dx + dy * dy);

                    std::list<Creature*> coldflames;
                    bot->GetCreatureListWithEntryInGrid(coldflames, NPC_COLDFLAME, 200.0f);
                    bool hitColdflame = false;
                    for (Creature* c : coldflames)
                    {
                        if (c->GetExactDist2d(cx, cy) < 4.0f)
                        {
                            hitColdflame = true;
                            break;
                        }
                    }
                    if (hitColdflame)
                        continue;

                    if (distToBoss > bestScore)
                    {
                        bestScore = distToBoss;
                        bestX = cx;
                        bestY = cy;
                        found = true;
                    }
                }

                if (found)
                    return MoveTo(bot->GetMapId(), bestX, bestY, ICC_LM_BONE_STORM_AT_POSITION.GetPositionZ(), false,
                                  false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }

            if (anchorDist > maxDistanceThreshold)
                return MoveTo(bot->GetMapId(), ICC_LM_BONE_STORM_AT_POSITION.GetPositionX(),
                              ICC_LM_BONE_STORM_AT_POSITION.GetPositionY(),
                              ICC_LM_BONE_STORM_AT_POSITION.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT);
            return true;
        }

        float const tankLeash =
            ColdflameNearAnchor(bot, ICC_LM_TANK_POSITION, 10.0f) ? 10.0f : maxDistanceThreshold;

        if (PlayerbotAI::IsMainTank(bot))
        {
            float const distance =
                bot->GetExactDist2d(ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY());
            if (distance > tankLeash)
                return MoveTowardPosition(ICC_LM_TANK_POSITION, maxDistanceThreshold);
            return false;
        }

        if (PlayerbotAI::IsAssistTank(bot))
        {
            float const distance =
                bot->GetExactDist2d(ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY());
            if (distance > tankLeash)
                return MoveTo(bot->GetMapId(), ICC_LM_TANK_POSITION.GetPositionX(),
                              ICC_LM_TANK_POSITION.GetPositionY(), ICC_LM_TANK_POSITION.GetPositionZ(), false, false,
                              false, false, MovementPriority::MOVEMENT_COMBAT);
            return false;
        }

        // Non-tanks: if too far from mid position, move toward it
        float const distance =
            bot->GetExactDist2d(ICC_LM_MID_POSITION.GetPositionX(), ICC_LM_MID_POSITION.GetPositionY());
        if (distance > 35.0f)
            return MoveTowardPosition(ICC_LM_MID_POSITION, 15.0f);

        return false;
    }

    float const tankLeash =
        ColdflameNearAnchor(bot, ICC_LM_TANK_POSITION, 10.0f) ? 10.0f : maxDistanceThreshold;

    if (botAI->HasAggro(boss) && PlayerbotAI::IsMainTank(bot) && boss->GetVictim() == bot)
    {
        float const distance =
            bot->GetExactDist2d(ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY());

        if (distance > tankLeash)
            return MoveTowardPosition(ICC_LM_TANK_POSITION, maxDistanceThreshold);
    }

    if (PlayerbotAI::IsAssistTank(bot))
    {
        float const distance =
            bot->GetExactDist2d(ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY());

        if (distance > tankLeash)
            return MoveTo(bot->GetMapId(), ICC_LM_TANK_POSITION.GetPositionX(), ICC_LM_TANK_POSITION.GetPositionY(),
                          ICC_LM_TANK_POSITION.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT);

        if (distance < maxDistanceThreshold)
        {
            bot->SetFacingToObject(boss);
            return true;
        }
    }

    return false;
}

bool IccLmTankPositionAction::MoveTowardPosition(Position const& position, float incrementSize)
{
    float moveX;
    float moveY;
    if (!IccStepToward(bot, position, incrementSize, moveX, moveY))
        return false;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IccSpikeAction::Execute(Event /*event*/)
{
    if (bot->HasAura(SPELL_LM_IMPALED))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    bool const isBossInBoneStorm = botAI->GetAura("Bone Storm", boss) != nullptr;
    std::vector<Unit*> const spikes = FindAliveSpikes();

    if (!spikes.empty())
    {
        HandleSpikeMarking(spikes, boss);
        return HandleSpikeAssignment(spikes, boss);
    }

    // No spikes alive -- skull on boss, clear cross, all bots on skull
    HandleNoSpikesMarking(boss);

    // Melee non-tanks in front of boss should reposition
    if (boss->isInFront(bot) && !PlayerbotAI::IsTank(bot) && !isBossInBoneStorm)
    {
        Position const safePosition = {-390.6757f, 2230.5283f, 0.0f};
        float const distance = bot->GetExactDist2d(safePosition.GetPositionX(), safePosition.GetPositionY());
        if (distance > 3.0f)
            return MoveTowardPosition(safePosition, 3.0f);
    }

    return false;
}

std::vector<Unit*> IccSpikeAction::FindAliveSpikes()
{
    // All difficulty variants — AzerothCore spawns a different entry per difficulty mode.
    // Bonespike NPCs have UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC so they
    // never appear in "possible targets no los". Use a direct grid search instead.
    std::vector<Creature*> const found = IccGetCreaturesByEntries(bot,
        {NPC_SPIKE1, NPC_SPIKE1_10H, NPC_SPIKE1_25N, NPC_SPIKE1_25H,
         NPC_SPIKE2, NPC_SPIKE2_10H, NPC_SPIKE2_25N, NPC_SPIKE2_25H,
         NPC_SPIKE3, NPC_SPIKE3_10H, NPC_SPIKE3_25N, NPC_SPIKE3_25H},
        200.0f);
    return std::vector<Unit*>(found.begin(), found.end());
}

bool IccSpikeAction::HandleSpikeMarking(std::vector<Unit*> const& spikes, Unit* boss)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static uint8 const Icons[] = {RtiTargetValue::skullIndex, RtiTargetValue::crossIndex, RtiTargetValue::starIndex};

    std::vector<ObjectGuid> aliveSpikeGuids;
    aliveSpikeGuids.reserve(spikes.size());
    for (Unit* spike : spikes)
        aliveSpikeGuids.push_back(spike->GetGUID());

    for (uint8 const iconIdx : Icons)
    {
        ObjectGuid const iconGuid = group->GetTargetIcon(iconIdx);
        if (iconGuid.IsEmpty())
            continue;
        if (std::find(aliveSpikeGuids.begin(), aliveSpikeGuids.end(), iconGuid) != aliveSpikeGuids.end())
            continue;

        Unit* marked = botAI->GetUnit(iconGuid);
        if (marked && !marked->IsAlive())
            group->SetTargetIcon(iconIdx, bot->GetGUID(), ObjectGuid::Empty);
    }

    // Check if the only spike left is a tank spike
    Player* firstSpikeVictim = spikes.size() == 1 ? GetSpikeVictim(spikes[0]) : nullptr;
    bool const onlyTankSpike = firstSpikeVictim && PlayerbotAI::IsTank(firstSpikeVictim);

    if (onlyTankSpike)
    {
        // Skull on spike, Cross on boss
        if (group->GetTargetIcon(RtiTargetValue::skullIndex) != spikes[0]->GetGUID())
            group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), spikes[0]->GetGUID());

        if (group->GetTargetIcon(RtiTargetValue::crossIndex) != boss->GetGUID())
            group->SetTargetIcon(RtiTargetValue::crossIndex, bot->GetGUID(), boss->GetGUID());

        if (!group->GetTargetIcon(RtiTargetValue::starIndex).IsEmpty())
            group->SetTargetIcon(RtiTargetValue::starIndex, bot->GetGUID(), ObjectGuid::Empty);

        return true;
    }

    // Clear icon slots beyond the current spike count
    for (size_t i = spikes.size(); i < sizeof(Icons); ++i)
    {
        if (!group->GetTargetIcon(Icons[i]).IsEmpty())
            group->SetTargetIcon(Icons[i], bot->GetGUID(), ObjectGuid::Empty);
    }

    // Assign Skull/Cross/Star to each alive spike
    for (size_t i = 0; i < spikes.size() && i < sizeof(Icons); ++i)
    {
        uint8 const iconIdx = Icons[i];
        if (group->GetTargetIcon(iconIdx) != spikes[i]->GetGUID())
            group->SetTargetIcon(iconIdx, bot->GetGUID(), spikes[i]->GetGUID());
    }

    return true;
}

bool IccSpikeAction::HandleNoSpikesMarking(Unit* boss)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Clear cross and star
    for (uint8 const iconIdx : {uint8(6), uint8(0)})
    {
        if (!group->GetTargetIcon(iconIdx).IsEmpty())
            group->SetTargetIcon(iconIdx, bot->GetGUID(), ObjectGuid::Empty);
    }

    // Skull on boss
    if (group->GetTargetIcon(RtiTargetValue::skullIndex) != boss->GetGUID())
        group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());

    // Per-bot context value -- every bot needs this for its own ChooseTarget.
    context->GetValue<std::string>("rti")->Set("skull");
    return true;
}

bool IccSpikeAction::HandleSpikeAssignment(std::vector<Unit*> const& spikes, Unit* boss)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const isMelee = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);
    bool const isAssistTank = PlayerbotAI::IsAssistTank(bot);

    auto isTankSpike = [&](Unit* spike) -> bool
    {
        Player* victim = GetSpikeVictim(spike);
        return victim && PlayerbotAI::IsTank(victim);
    };

    // Assist tank: only attack tank spike, ignore all others
    if (isAssistTank)
    {
        for (Unit* spike : spikes)
        {
            if (isTankSpike(spike))
            {
                Attack(spike);
                return false;
            }
        }
        return false;
    }

    // Only tank spike left -- ranged go skull (spike), melee go cross (boss)
    bool const onlyTankSpike = spikes.size() == 1 && isTankSpike(spikes[0]);
    if (onlyTankSpike)
    {
        if (isMelee)
        {
            context->GetValue<std::string>("rti")->Set("cross");
            Attack(boss);
        }
        else
        {
            context->GetValue<std::string>("rti")->Set("skull");
            Attack(spikes[0]);
        }
        return false;
    }

    // Melee DPS: pick closest safe spike within 20y, never tank spikes
    if (isMelee)
    {
        Unit* bestSpike = nullptr;
        float bestDist = 20.0f;
        for (Unit* spike : spikes)
        {
            if (isTankSpike(spike))
                continue;

            if (boss->isInFront(spike, 7.0f))
                continue;

            if (IsSpikeInColdFlame(spike))
                continue;

            float const dist = bot->GetExactDist2d(spike);
            if (dist < bestDist)
            {
                bestDist = dist;
                bestSpike = spike;
            }
        }

        if (bestSpike)
            Attack(bestSpike);

        return false;
    }

    // Ranged / healers: balanced assignment across all spikes
    std::vector<Player*> rangedMembers;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member->HasAura(SPELL_LM_IMPALED))
            continue;

        if (PlayerbotAI::IsMainTank(member) || PlayerbotAI::IsAssistTank(member))
            continue;

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsTank(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    std::sort(rangedMembers.begin(), rangedMembers.end(),
              [](Player const* a, Player const* b) { return a->GetGUID() < b->GetGUID(); });

    auto const it = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    if (it == rangedMembers.end())
        return false;

    size_t const myIndex = std::distance(rangedMembers.begin(), it);
    std::vector<size_t> const groupSizes = IccBalancedGroupSizes(rangedMembers.size(), spikes.size());
    size_t const spikeIndex = IccAssignedBucketIndex(myIndex, groupSizes);

    if (spikeIndex >= spikes.size())
        return false;

    Unit* mySpike = spikes[spikeIndex];
    context->GetValue<std::string>("rti")->Set(IccRtiNameForBucket(spikeIndex));

    Attack(mySpike);
    return false;
}

bool IccSpikeAction::MoveTowardPosition(Position const& position, float incrementSize)
{
    float moveX;
    float moveY;
    if (!IccStepToward(bot, position, incrementSize, moveX, moveY))
        return false;

    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

Player* IccSpikeAction::GetSpikeVictim(Unit* spike)
{
    // Spike holds player via vehicle; GetVictim() is unreliable (NPC not in combat)
    if (Vehicle* veh = spike->GetVehicleKit())
    {
        for (auto const& [seatId, seatInfo] : veh->Seats)
        {
            if (Unit* passenger = ObjectAccessor::GetUnit(*spike, seatInfo.Passenger.Guid))
                return passenger->ToPlayer();
        }
    }
    return nullptr;
}

bool IccSpikeAction::IsSpikeInColdFlame(Unit* spike)
{
    float const checkRadius = 6.0f;
    std::list<Creature*> coldflames;
    spike->GetCreatureListWithEntryInGrid(coldflames, NPC_COLDFLAME, checkRadius);
    return !coldflames.empty();
}
