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
#include <unordered_map>
#include <unordered_set>

namespace
{
// How long (ms) all bots stay at a cloud before advancing to the next one.
// This window lets every portal bot teleport in and collect the cloud together.
constexpr uint32 CLOUD_SYNC_WAIT_MS = 1000;
}

bool IccValithriaGroupAction::Execute(Event /*event*/)
{
    std::vector<Creature*> const portalList = IccGetCreaturesByEntries(
        bot, {NPC_DREAM_PORTAL, NPC_DREAM_PORTAL_PRE_EFFECT, NPC_NIGHTMARE_PORTAL, NPC_NIGHTMARE_PORTAL_PRE_EFFECT},
        100.0f);
    Creature* portal = portalList.empty() ? nullptr : portalList.front();

    Creature* worm = bot->FindNearestCreature(NPC_ROT_WORM, 100.0f);
    Creature* manaVoid = bot->FindNearestCreature(NPC_MANA_VOID, 100.0f);

    // Column of Frost units - still hostile so the hostile list is fine here
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> frostColumns;
    for (ObjectGuid const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_COLUMN_OF_FROST)
            frostColumns.push_back(unit);
    }

    // Tanks collect stray Gluttonous Abominations / Rot Worms anywhere.
    // Each tank picks NEAREST stray so two tanks naturally split work.
    // In taunt range -> taunt. Out of range -> step toward add to close gap.
    if (PlayerbotAI::IsTank(bot))
    {
        constexpr float ADD_TAUNT_RANGE = 30.0f;
        constexpr float CHASE_STEP = 10.0f;

        Unit* strayAdd = nullptr;
        float bestDist = FLT_MAX;
        for (ObjectGuid const& guid : AI_VALUE(GuidVector, "possible targets"))
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;
            if (unit->GetEntry() != NPC_GLUTTONOUS_ABOMINATION && unit->GetEntry() != NPC_ROT_WORM)
                continue;

            Unit* victim = unit->GetVictim();
            Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
            if (victimPlayer && PlayerbotAI::IsTank(victimPlayer))
                continue;

            float const d = bot->GetExactDist2d(unit);
            if (d < bestDist)
            {
                bestDist = d;
                strayAdd = unit;
            }
        }

        if (strayAdd)
        {
            if (bestDist <= ADD_TAUNT_RANGE)
            {
                IccCastClassTaunt(bot, botAI, strayAdd);
            }
            else
            {
                float dx = strayAdd->GetPositionX() - bot->GetPositionX();
                float dy = strayAdd->GetPositionY() - bot->GetPositionY();
                float const len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f)
                {
                    dx /= len;
                    dy /= len;
                    float const step = std::min(CHASE_STEP, bestDist - ADD_TAUNT_RANGE + 5.0f);
                    float const moveX = bot->GetPositionX() + dx * step;
                    float const moveY = bot->GetPositionY() + dy * step;
                    MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                           MovementPriority::MOVEMENT_COMBAT);
                    return true;
                }
            }
        }
    }

    // Healers move toward the heal position when no portal is active
    if (PlayerbotAI::IsHeal(bot) && !portal && bot->GetExactDist2d(ICC_VDW_HEAL_POSITION) > 30.0f)
    {
        return MoveTo(bot->GetMapId(), ICC_VDW_HEAL_POSITION.GetPositionX(), ICC_VDW_HEAL_POSITION.GetPositionY(),
                      ICC_VDW_HEAL_POSITION.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_NORMAL);
    }

    // Avoidance
    if (manaVoid && bot->GetExactDist2d(manaVoid) < 10.0f && !botAI->GetAura("Twisted Nightmares", bot) &&
        !botAI->GetAura("Emerald Vigor", bot))
    {
        botAI->Reset();
        FleePosition(manaVoid->GetPosition(), 11.0f, 250U);
    }

    for (Unit* column : frostColumns)
    {
        if (column && bot->GetExactDist2d(column) < 7.0f)
        {
            botAI->Reset();
            FleePosition(column->GetPosition(), 8.0f, 250U);
        }
    }

    if (worm && worm->IsAlive() && worm->GetVictim() == bot && !PlayerbotAI::IsTank(bot))
    {
        botAI->Reset();
        FleePosition(worm->GetPosition(), 10.0f, 250U);
    }

    // Zombie handling: only exploding-zombie flee here (any non-tank within 20f).
    // Per-bot kiting when zombie victim handled by IccValithriaZombieKiteAction
    // (higher priority trigger ACTION_EMERGENCY+9, blocks lower actions).
    std::list<Creature*> allZombies;
    bot->GetCreatureListWithEntryInGrid(allZombies, NPC_BLISTERING_ZOMBIE, 100.0f);

    Creature* explodingZombie = nullptr;
    Creature* nearbyZombie = nullptr;
    float nearbyDist = FLT_MAX;

    for (Creature* z : allZombies)
    {
        if (!z || !z->IsAlive())
            continue;

        bool const exploding =
            z->HealthBelowPct(5) || z->FindCurrentSpellBySpellId(SPELL_ACID_BURST) || z->HasAura(SPELL_ACID_BURST);
        float const d = bot->GetExactDist2d(z);

        if (exploding && d < 20.0f && (!explodingZombie || d < bot->GetExactDist2d(explodingZombie)))
            explodingZombie = z;

        if (d < nearbyDist)
        {
            nearbyDist = d;
            nearbyZombie = z;
        }
    }

    if (explodingZombie)
    {
        botAI->Reset();
        return FleePosition(explodingZombie->GetPosition(), 20.0f, 250U);
    }

    // Non-tank melee must never sit inside 15f of any zombie: the zombie can
    // flip victim at any time and at melee range the bot has no time to kite.
    if (nearbyZombie && PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot) && nearbyDist < 15.0f)
    {
        botAI->Reset();
        return FleePosition(nearbyZombie->GetPosition(), 15.0f, 250U);
    }

    if (nearbyZombie && !PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsHeal(bot) && nearbyZombie->GetVictim() != bot)
        ApplyCrowdControl(nearbyZombie);

    // Leash: every role stays within 35f of the boss anchor unless doing an
    // exclusive task (zombie kite, portal work, dream-state cloud collection).
    // Zombie kite action runs at ACTION_EMERGENCY+9 so it preempts this action;
    // extra skip-conditions below guard the remaining exclusive tasks.
    constexpr float LEASH_RADIUS = 35.0f;
    bool const inDreamState = bot->HasAura(SPELL_DREAM_STATE);
    bool hasPortalClaim = false;
    if (PlayerbotAI::IsHeal(bot))
    {
        auto& claims = IcecrownHelpers::IccState(bot->GetMap()->GetInstanceId()).vtPortalClaim;
        if (claims.find(bot->GetGUID()) != claims.end())
            hasPortalClaim = true;
    }
    bool const hasZombieThreat = nearbyZombie && nearbyZombie->GetVictim() == bot;

    Unit* const victim = bot->GetVictim();
    bool const engagedDps = victim && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot) &&
                            victim->GetExactDist2d(ICC_VDW_HEAL_POSITION) <= 50.0f;

    if (!inDreamState && !hasPortalClaim && !hasZombieThreat && !engagedDps)
    {
        float const distToAnchor = bot->GetExactDist2d(ICC_VDW_HEAL_POSITION);
        if (distToAnchor > LEASH_RADIUS)
        {
            constexpr float STEP = 8.0f;
            float const dx = ICC_VDW_HEAL_POSITION.GetPositionX() - bot->GetPositionX();
            float const dy = ICC_VDW_HEAL_POSITION.GetPositionY() - bot->GetPositionY();
            float const dz = ICC_VDW_HEAL_POSITION.GetPositionZ() - bot->GetPositionZ();
            float const dist = std::hypot(dx, dy);
            float moveX;
            float moveY;
            float moveZ;
            if (dist > STEP)
            {
                moveX = bot->GetPositionX() + (dx / dist) * STEP;
                moveY = bot->GetPositionY() + (dy / dist) * STEP;
                moveZ = bot->GetPositionZ() + (dz / dist) * STEP;
            }
            else
            {
                moveX = ICC_VDW_HEAL_POSITION.GetPositionX();
                moveY = ICC_VDW_HEAL_POSITION.GetPositionY();
                moveZ = ICC_VDW_HEAL_POSITION.GetPositionZ();
            }
            MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
                   MovementPriority::MOVEMENT_COMBAT, true, false);
            return true;
        }
    }

    Difficulty const diff = bot->GetRaidDifficulty();
    Group* group = bot->GetGroup();

    if (group && (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC))
        return Handle25ManGroupLogic();

    return Handle10ManGroupLogic();
}

bool IccValithriaGroupAction::ApplyCrowdControl(Unit* zombie)
{
    return IccTryClassCC(bot, botAI, zombie).value_or(false);
}

bool IccValithriaGroupAction::Handle25ManGroupLogic()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Priority order follows Wowhead/Icy-Veins strategy:
    // Blazing Skeleton (AoE Lay Waste) -> Suppresser (stacks -10% heal) ->
    // Risen Archmage (Mana Void/Column of Frost) -> Blistering Zombie (ranged
    // kites it, melee flees at 15f) -> Gluttonous Abomination (tank keeps it
    // faced away) -> Rot Worm (cleanup from dead Abominations).
    static constexpr std::array<uint32, 6> ADD_PRIORITY_CHECK = {NPC_BLAZING_SKELETON,       NPC_SUPPRESSER,
                                                                 NPC_RISEN_ARCHMAGE,         NPC_BLISTERING_ZOMBIE,
                                                                 NPC_GLUTTONOUS_ABOMINATION, NPC_ROT_WORM};
    constexpr float MARK_RADIUS = 45.0f;

    int priorityAddsAlive = 0;
    GuidVector const targets = AI_VALUE(GuidVector, "possible targets");
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
        uint32 const e = unit->GetEntry();
        bool isPriority = false;
        for (uint32 pe : ADD_PRIORITY_CHECK)
        {
            if (pe == e)
            {
                isPriority = true;
                break;
            }
        }
        if (!isPriority)
            continue;
        float const d =
            unit->GetExactDist2d(ICC_VDW_HEAL_POSITION.GetPositionX(), ICC_VDW_HEAL_POSITION.GetPositionY());
        if (d > MARK_RADIUS)
            continue;
        ++priorityAddsAlive;
        if (priorityAddsAlive >= 2)
            break;
    }

    bool const singleMarkMode = priorityAddsAlive <= 1;

    std::vector<Player*> eligible;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        if (!memberAI || IsSelfBot(member))
            continue;

        if (PlayerbotAI::IsHeal(member) && member->HasAura(SPELL_DREAM_STATE))
            continue;

        if (!PlayerbotAI::IsTank(member) && !PlayerbotAI::IsDps(member) && !PlayerbotAI::IsHeal(member))
            continue;

        eligible.push_back(member);
    }

    std::sort(eligible.begin(), eligible.end(),
              [](Player const* a, Player const* b) { return a->GetGUID() < b->GetGUID(); });

    bool inGroup1 = false;
    bool inGroup2 = false;

    if (singleMarkMode)
    {
        inGroup1 = std::any_of(eligible.begin(), eligible.end(), [this](Player* p) { return p == bot; });
    }
    else
    {
        // Bucket by role (tank / melee dps / ranged dps / healer) so each group
        // gets roughly the same number of each role. Within a bucket alternate
        // assignment; flip the starting side on odd-sized buckets so leftover
        // slots don't all pile onto Group1.
        std::vector<Player*> tanks;
        std::vector<Player*> meleeDps;
        std::vector<Player*> rangedDps;
        std::vector<Player*> healers;
        for (Player* p : eligible)
        {
            PlayerbotAI* pai = GET_PLAYERBOT_AI(p);
            if (!pai)
                continue;
            if (PlayerbotAI::IsTank(p))
                tanks.push_back(p);
            else if (PlayerbotAI::IsHeal(p))
                healers.push_back(p);
            else if (PlayerbotAI::IsMelee(p))
                meleeDps.push_back(p);
            else
                rangedDps.push_back(p);
        }

        std::vector<Player*> group1;
        std::vector<Player*> group2;
        int parity = 0;
        auto splitBucket = [&](std::vector<Player*> const& bucket)
        {
            for (size_t i = 0; i < bucket.size(); ++i)
            {
                if (((i + parity) & 1) == 0)
                    group1.push_back(bucket[i]);
                else
                    group2.push_back(bucket[i]);
            }
            if (bucket.size() & 1)
                parity ^= 1;
        };
        splitBucket(tanks);
        splitBucket(meleeDps);
        splitBucket(rangedDps);
        splitBucket(healers);

        inGroup1 = std::any_of(group1.begin(), group1.end(), [this](Player* p) { return p == bot; });
        inGroup2 = std::any_of(group2.begin(), group2.end(), [this](Player* p) { return p == bot; });
    }

    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsDps(bot) || PlayerbotAI::IsHeal(bot))
        HandleMarkingLogic(inGroup1, inGroup2, singleMarkMode);

    return false;
}

bool IccValithriaGroupAction::HandleMarkingLogic(bool inGroup1, bool inGroup2, bool singleMarkMode)
{
    static constexpr uint8 SKULL_ICON = RtiTargetValue::skullIndex;
    static constexpr uint8 CROSS_ICON = RtiTargetValue::crossIndex;
    // Kill priority per Wowhead/Icy-Veins: Lay Waste skeletons first, then
    // heal-debuff suppressers, then archmages, then zombies (ranged only -
    // melee flee at 15f), then abominations (tank-faced), then rot worm cleanup.
    static constexpr std::array<uint32, 6> ADD_PRIORITY = {NPC_BLAZING_SKELETON,       NPC_SUPPRESSER,
                                                           NPC_RISEN_ARCHMAGE,         NPC_BLISTERING_ZOMBIE,
                                                           NPC_GLUTTONOUS_ABOMINATION, NPC_ROT_WORM};

    uint8 iconIndex;
    std::string rtiValue;

    if (singleMarkMode)
    {
        if (!inGroup1)
            return false;
        iconIndex = SKULL_ICON;
        rtiValue = "skull";
    }
    else if (inGroup1)
    {
        iconIndex = SKULL_ICON;
        rtiValue = "skull";
    }
    else if (inGroup2)
    {
        iconIndex = CROSS_ICON;
        rtiValue = "cross";
    }
    else
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr float MARK_RADIUS = 45.0f;

    auto inMarkRange = [&](Unit* unit) -> bool
    {
        return unit->GetExactDist2d(ICC_VDW_HEAL_POSITION.GetPositionX(), ICC_VDW_HEAL_POSITION.GetPositionY()) <=
               MARK_RADIUS;
    };

    context->GetValue<std::string>("rti")->Set(rtiValue);

    bool const dispatchRti = !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot);
    bool const isMeleeDps = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);

    auto tryDispatchRti = [&](ObjectGuid rtiGuid)
    {
        if (!dispatchRti || rtiGuid.IsEmpty())
            return;
        // Melee DPS must not attack zombies - they explode at melee range.
        // Ranged DPS still handle the marked zombie via RTI.
        Unit* rtiUnit = botAI->GetUnit(rtiGuid);
        if (rtiUnit && rtiUnit->GetEntry() == NPC_BLISTERING_ZOMBIE && isMeleeDps)
            return;
        Unit* const victim = bot->GetVictim();
        if (victim && victim->GetGUID() == rtiGuid)
            return;
        botAI->DoSpecificAction("attack rti target");
    };

    GuidVector const adds = AI_VALUE(GuidVector, "possible targets");

    auto getPriorityRank = [&](uint32 entry) -> int
    {
        for (size_t i = 0; i < ADD_PRIORITY.size(); ++i)
            if (ADD_PRIORITY[i] == entry)
                return static_cast<int>(i);
        return -1;
    };

    auto isValidAdd = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;
        if (getPriorityRank(unit->GetEntry()) < 0)
            return false;
        return inMarkRange(unit);
    };

    ObjectGuid otherIconGuid;
    if (!singleMarkMode)
    {
        uint8 const otherIcon = (iconIndex == SKULL_ICON) ? CROSS_ICON : SKULL_ICON;
        otherIconGuid = group->GetTargetIcon(otherIcon);
    }

    Unit* currentIconUnit = botAI->GetUnit(group->GetTargetIcon(iconIndex));

    // Split the room by a world-Y line through the heal anchor. Group1 (skull)
    // owns one half, Group2 (cross) owns the other. A group only falls back to
    // the other half when its own side has no marking candidate.
    auto isOwnSide = [&](Unit* unit) -> bool
    {
        if (singleMarkMode)
            return true;
        bool const sideA = unit->GetPositionY() < ICC_VDW_HEAL_POSITION.GetPositionY();
        return inGroup1 ? sideA : !sideA;
    };

    // Pick the reference point for "closest add" selection so DPS travels the
    // shortest distance when the mark flips: prefer the current icon's
    // position (alive or corpse), then the other group's still-live mark,
    // then the heal anchor.
    Position refPos = ICC_VDW_HEAL_POSITION;
    if (currentIconUnit)
        refPos = currentIconUnit->GetPosition();
    else if (!singleMarkMode)
    {
        if (Unit* otherUnit = botAI->GetUnit(otherIconGuid))
            if (otherUnit->IsAlive())
                refPos = otherUnit->GetPosition();
    }

    Unit* bestTarget = nullptr;
    int bestRank = static_cast<int>(ADD_PRIORITY.size());
    bool bestTargetOwnSide = false;

    // Two-pass scan: own side first, then any side. Within each pass walk
    // priorities top-down and within the first tier that has any candidate
    // pick the one closest to refPos.
    for (int pass = 0; pass < 2 && !bestTarget; ++pass)
    {
        bool const ownSideOnly = (pass == 0);
        for (size_t rank = 0; rank < ADD_PRIORITY.size(); ++rank)
        {
            uint32 const entry = ADD_PRIORITY[rank];
            float bestDist = FLT_MAX;
            for (ObjectGuid const& guid : adds)
            {
                if (!singleMarkMode && guid == otherIconGuid)
                    continue;
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
                    continue;
                if (!inMarkRange(unit))
                    continue;
                if (ownSideOnly && !isOwnSide(unit))
                    continue;
                float const d = unit->GetExactDist2d(refPos.GetPositionX(), refPos.GetPositionY());
                if (d < bestDist)
                {
                    bestDist = d;
                    bestTarget = unit;
                    bestRank = static_cast<int>(rank);
                    bestTargetOwnSide = ownSideOnly;
                }
            }
            if (bestTarget)
                break;
        }
    }

    if (isValidAdd(currentIconUnit))
    {
        int currentRank = -1;
        if (currentIconUnit)
            currentRank = getPriorityRank(currentIconUnit->GetEntry());

        bool const currentOnOwnSide = isOwnSide(currentIconUnit);

        // Side takes precedence: if an own-side candidate exists and the current
        // icon is on the other side, flip regardless of priority rank.
        bool const sideFlip = bestTarget && bestTargetOwnSide && !currentOnOwnSide;

        if (!sideFlip && (!bestTarget || bestRank >= currentRank))
        {
            if (currentIconUnit)
                tryDispatchRti(currentIconUnit->GetGUID());
            return false;
        }
    }

    if (!bestTarget)
    {
        if (currentIconUnit && !group->GetTargetIcon(iconIndex).IsEmpty())
            group->SetTargetIcon(iconIndex, bot->GetGUID(), ObjectGuid::Empty);
        return false;
    }

    group->SetTargetIcon(iconIndex, bot->GetGUID(), bestTarget->GetGUID());

    if (singleMarkMode)
    {
        uint8 const crossIcon = CROSS_ICON;
        Unit* currentCrossUnit = botAI->GetUnit(group->GetTargetIcon(crossIcon));
        if (currentCrossUnit && !group->GetTargetIcon(crossIcon).IsEmpty())
            group->SetTargetIcon(crossIcon, bot->GetGUID(), ObjectGuid::Empty);
    }

    tryDispatchRti(bestTarget->GetGUID());

    return false;
}

bool IccValithriaGroupAction::Handle10ManGroupLogic()
{
    static constexpr uint8 DEFAULT_ICON = RtiTargetValue::skullIndex;
    // Kill priority per Wowhead/Icy-Veins: skeleton > suppresser > archmage >
    // zombie (ranged) > abomination (tank) > rot worm (cleanup).
    static constexpr std::array<uint32, 6> ADD_PRIORITY = {NPC_BLAZING_SKELETON,       NPC_SUPPRESSER,
                                                           NPC_RISEN_ARCHMAGE,         NPC_BLISTERING_ZOMBIE,
                                                           NPC_GLUTTONOUS_ABOMINATION, NPC_ROT_WORM};

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    PlayerbotAI* selfAI = GET_PLAYERBOT_AI(bot);
    if (!selfAI || IsSelfBot(bot))
        return false;
    if (PlayerbotAI::IsHeal(bot) && bot->HasAura(SPELL_DREAM_STATE))
        return false;

    constexpr float MARK_RADIUS = 45.0f;

    auto inMarkRange = [&](Unit* unit) -> bool
    {
        return unit->GetExactDist2d(ICC_VDW_HEAL_POSITION.GetPositionX(), ICC_VDW_HEAL_POSITION.GetPositionY()) <=
               MARK_RADIUS;
    };

    context->GetValue<std::string>("rti")->Set("skull");

    bool const dispatchRti = !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot);
    bool const isMeleeDps = PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);

    auto tryDispatchRti = [&](ObjectGuid rtiGuid)
    {
        if (!dispatchRti || rtiGuid.IsEmpty())
            return;
        // Melee DPS must not attack zombies - ranged only.
        Unit* rtiUnit = botAI->GetUnit(rtiGuid);
        if (rtiUnit && rtiUnit->GetEntry() == NPC_BLISTERING_ZOMBIE && isMeleeDps)
            return;
        Unit* const victim = bot->GetVictim();
        if (victim && victim->GetGUID() == rtiGuid)
            return;
        botAI->DoSpecificAction("attack rti target");
    };

    GuidVector const adds = AI_VALUE(GuidVector, "possible targets");

    auto getPriorityRank = [&](uint32 entry) -> int
    {
        for (size_t i = 0; i < ADD_PRIORITY.size(); ++i)
            if (ADD_PRIORITY[i] == entry)
                return static_cast<int>(i);
        return -1;
    };

    auto isValidAdd = [&](Unit* unit) -> bool
    {
        if (!unit || !unit->IsAlive())
            return false;
        if (getPriorityRank(unit->GetEntry()) < 0)
            return false;
        return inMarkRange(unit);
    };

    Unit* currentIconUnit = botAI->GetUnit(group->GetTargetIcon(DEFAULT_ICON));

    // Use the current mark's position (alive or corpse) as the reference so
    // DPS moves the least when the mark flips; fall back to the heal anchor.
    Position refPos = ICC_VDW_HEAL_POSITION;
    if (currentIconUnit)
        refPos = currentIconUnit->GetPosition();

    Unit* bestTarget = nullptr;
    int bestRank = static_cast<int>(ADD_PRIORITY.size());

    for (size_t rank = 0; rank < ADD_PRIORITY.size(); ++rank)
    {
        uint32 const entry = ADD_PRIORITY[rank];
        float bestDist = FLT_MAX;
        for (ObjectGuid const& guid : adds)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
                continue;
            if (!inMarkRange(unit))
                continue;
            float const d = unit->GetExactDist2d(refPos.GetPositionX(), refPos.GetPositionY());
            if (d < bestDist)
            {
                bestDist = d;
                bestTarget = unit;
                bestRank = static_cast<int>(rank);
            }
        }
        if (bestTarget)
            break;
    }
    if (isValidAdd(currentIconUnit))
    {
        int currentRank;
        if (currentIconUnit)
            currentRank = getPriorityRank(currentIconUnit->GetEntry());
        else
        {
            // Handle the null case appropriately, e.g.:
            currentRank = -1;  // or another default/error value
        }

        if (!bestTarget || bestRank >= currentRank)
        {
            if (currentIconUnit)
                tryDispatchRti(currentIconUnit->GetGUID());
            return false;
        }
    }

    if (!bestTarget)
    {
        if (currentIconUnit && !group->GetTargetIcon(DEFAULT_ICON).IsEmpty())
            group->SetTargetIcon(DEFAULT_ICON, bot->GetGUID(), ObjectGuid::Empty);
        return false;
    }

    group->SetTargetIcon(DEFAULT_ICON, bot->GetGUID(), bestTarget->GetGUID());

    tryDispatchRti(bestTarget->GetGUID());

    return false;
}

bool IccValithriaPortalAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsHeal(bot) || bot->HasAura(SPELL_DREAM_STATE))
        return false;

    // Healer just dropped Dream State - if still airborne (portal exit leaves
    // bots above the floor) snap to floor Z immediately so they don't fall and die.
    constexpr float MAX_Z = 367.961f;
    constexpr float TARGET_Z = 365.0f;
    if (bot->GetPositionZ() > MAX_Z)
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), TARGET_Z, bot->GetOrientation());

    constexpr float SEARCH_RANGE = 200.0f;

    std::vector<Creature*> preEffectPortals =
        IccGetCreaturesByEntries(bot, {NPC_DREAM_PORTAL_PRE_EFFECT, NPC_NIGHTMARE_PORTAL_PRE_EFFECT}, SEARCH_RANGE);

    std::vector<Creature*> realPortals =
        IccGetCreaturesByEntries(bot, {NPC_DREAM_PORTAL, NPC_NIGHTMARE_PORTAL}, SEARCH_RANGE);

    // Evict stale claims BEFORE early-return so claims don't leak after fight.
    // Scope eviction to this instance only - other ICC instances have their
    // own portal GUID universes and we must not erase their claims.
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    auto& claims = IcecrownHelpers::IccState(instanceId).vtPortalClaim;

    std::unordered_set<ObjectGuid> livePortalGuids;
    for (Creature* p : preEffectPortals)
        if (p)
            livePortalGuids.insert(p->GetGUID());
    for (Creature* p : realPortals)
        if (p)
            livePortalGuids.insert(p->GetGUID());

    for (auto it = claims.begin(); it != claims.end();)
    {
        if (!livePortalGuids.count(it->second))
            it = claims.erase(it);
        else
            ++it;
    }

    if (preEffectPortals.empty() && realPortals.empty())
        return false;

    // Collect OTHER healers' claims so we never pick a portal already taken.
    std::unordered_set<ObjectGuid> reservedPortals;
    for (auto const& kv : claims)
        if (kv.first != bot->GetGUID())
            reservedPortals.insert(kv.second);

    auto pickClosestUnreserved = [&](std::vector<Creature*> const& portals) -> Creature*
    {
        Creature* best = nullptr;
        float bestDist = FLT_MAX;
        for (Creature* p : portals)
        {
            if (!p)
                continue;
            if (reservedPortals.count(p->GetGUID()))
                continue;
            float const d = bot->GetExactDist2d(p);
            if (d < bestDist)
            {
                bestDist = d;
                best = p;
            }
        }
        return best;
    };

    // Prefer pre-effect (claim early so others see it). Fall through to real
    // portal if no pre-effect available.
    Creature* assigned = nullptr;
    if (!preEffectPortals.empty())
        assigned = pickClosestUnreserved(preEffectPortals);
    if (!assigned && !realPortals.empty())
        assigned = pickClosestUnreserved(realPortals);

    // If everything reserved (more healers than portals), fall back to nearest
    // overall so this healer still does something useful.
    if (!assigned)
    {
        std::vector<Creature*> const& pool = !preEffectPortals.empty() ? preEffectPortals : realPortals;
        float bestDist = FLT_MAX;
        for (Creature* p : pool)
        {
            if (!p)
                continue;
            float const d = bot->GetExactDist2d(p);
            if (d < bestDist)
            {
                bestDist = d;
                assigned = p;
            }
        }
    }

    if (!assigned)
        return false;

    claims[bot->GetGUID()] = assigned->GetGUID();

    if (bot->GetDistance2d(assigned->GetPositionX(), assigned->GetPositionY()) > 0.5f)
    {
        MoveTo(assigned->GetMapId(), assigned->GetPositionX(), assigned->GetPositionY(), assigned->GetPositionZ(),
               false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
    }

    botAI->RemoveShapeshift();

    // Click the real portal once it spawns within reach. Only click our claimed
    // portal (if it became real) or any real portal at our exact location.
    Creature* clickTarget = nullptr;
    float minDist = FLT_MAX;
    for (Creature* portal : realPortals)
    {
        if (!portal)
            continue;
        float const d = bot->GetDistance2d(portal);
        if (d < 3.0f && d < minDist)
        {
            clickTarget = portal;
            minDist = d;
        }
    }

    if (clickTarget)
    {
        botAI->RemoveShapeshift();
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        bot->SetFacingToObject(clickTarget);
        clickTarget->HandleSpellClick(bot);
        return true;
    }

    return false;
}

bool IccValithriaHealAction::Execute(Event /*event*/)
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    // Snap to floor Z first - healer may have just dropped Dream State and be
    // airborne above MAX_Z. Run BEFORE the HP gate so it fires every tick.
    constexpr float MAX_Z = 367.961f;
    constexpr float TARGET_Z = 365.0f;
    if (bot->GetPositionZ() > MAX_Z)
        bot->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), TARGET_Z, bot->GetOrientation());

    if (bot->GetHealthPct() < 50.0f)
        return false;

    if (!bot->HasAura(SPELL_DREAM_STATE))
    {
        constexpr float NORMAL_SPEED = 1.0f;
        bot->SetSpeed(MOVE_RUN, NORMAL_SPEED, true);
        bot->SetSpeed(MOVE_WALK, NORMAL_SPEED, true);
        bot->SetSpeed(MOVE_FLIGHT, NORMAL_SPEED, true);
    }

    Creature* valithria = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!valithria)
        return false;

    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            constexpr uint32 SPELL_REJUVENATION = 48441;
            constexpr uint32 SPELL_REGROWTH = 48443;
            constexpr uint32 SPELL_LIFEBLOOM = 48451;
            constexpr uint32 SPELL_WILD_GROWTH = 53251;
            constexpr uint8 LIFEBLOOM_MAX = 3;

            if (!valithria->HasAura(SPELL_REJUVENATION, bot->GetGUID()))
                return botAI->CastSpell(SPELL_REJUVENATION, valithria);

            if (!valithria->HasAura(SPELL_REGROWTH, bot->GetGUID()))
                return botAI->CastSpell(SPELL_REGROWTH, valithria);

            Aura* lb = valithria->GetAura(SPELL_LIFEBLOOM, bot->GetGUID());
            if (!lb || lb->GetStackAmount() < LIFEBLOOM_MAX)
                return botAI->CastSpell(SPELL_LIFEBLOOM, valithria);

            return botAI->CastSpell(SPELL_WILD_GROWTH, valithria);
        }
        case CLASS_SHAMAN:
        {
            constexpr uint32 SPELL_RIPTIDE = 61301;
            constexpr uint32 SPELL_HEALING_WAVE = 49273;
            return valithria->HasAura(SPELL_RIPTIDE, bot->GetGUID()) ? botAI->CastSpell(SPELL_HEALING_WAVE, valithria)
                                                                     : botAI->CastSpell(SPELL_RIPTIDE, valithria);
        }
        case CLASS_PRIEST:
        {
            constexpr uint32 SPELL_RENEW = 48068;
            constexpr uint32 SPELL_GREATER_HEAL = 48063;
            return valithria->HasAura(SPELL_RENEW, bot->GetGUID()) ? botAI->CastSpell(SPELL_GREATER_HEAL, valithria)
                                                                   : botAI->CastSpell(SPELL_RENEW, valithria);
        }
        case CLASS_PALADIN:
        {
            constexpr uint32 SPELL_BEACON = 53563;
            constexpr uint32 SPELL_HOLY_LIGHT = 48782;
            return valithria->HasAura(SPELL_BEACON, bot->GetGUID()) ? botAI->CastSpell(SPELL_HOLY_LIGHT, valithria)
                                                                    : botAI->CastSpell(SPELL_BEACON, valithria);
        }
        default:
            break;
    }
    return false;
}

bool IccValithriaDreamCloudAction::Execute(Event /*event*/)
{
    if (!bot->HasAura(SPELL_DREAM_STATE))
        return false;

    bot->SetSpeed(MOVE_RUN, 2.0f, true);
    bot->SetSpeed(MOVE_WALK, 2.0f, true);
    bot->SetSpeed(MOVE_FLIGHT, 2.0f, true);

    std::vector<Player*> allDream;
    std::vector<Player*> realDream;

    Map::PlayerList const& playerList = bot->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
    {
        Player* player = itr->GetSource();
        if (!player || !player->IsAlive() || !player->HasAura(SPELL_DREAM_STATE))
            continue;

        allDream.push_back(player);

        PlayerbotAI* playerBotAI = GET_PLAYERBOT_AI(player);
        if (!playerBotAI || IsSelfBot(player))
            realDream.push_back(player);
    }

    if (allDream.empty())
        return false;

    std::vector<Player*>& leaderPool = realDream.empty() ? allDream : realDream;

    std::sort(leaderPool.begin(), leaderPool.end(),
              [](Player const* a, Player const* b) { return a->GetGUID() < b->GetGUID(); });

    Player* leader = leaderPool.front();

    if (!realDream.empty())
    {
        constexpr float STACK_DIST = 1.0f;
        if (bot->GetDistance(leader) > STACK_DIST)
        {
            bot->TeleportTo(bot->GetMapId(), leader->GetPositionX(), leader->GetPositionY(), leader->GetPositionZ(),
                            bot->GetOrientation());
        }
        return false;
    }

    // No real players - synchronized bot cloud collection
    uint32 const instanceId = bot->GetInstanceId();
    uint32 const nowMs = getMSTime();

    IcecrownHelpers::ValithriaCloudSync& sync = IcecrownHelpers::IccState(instanceId).vtCloudSync;

    std::vector<Creature*> dreamClouds = CollectClouds(NPC_DREAM_CLOUD, leader);
    std::vector<Creature*> nightmareClouds = CollectClouds(NPC_NIGHTMARE_CLOUD, leader);

    std::vector<Creature*> allClouds;
    allClouds.insert(allClouds.end(), dreamClouds.begin(), dreamClouds.end());
    allClouds.insert(allClouds.end(), nightmareClouds.begin(), nightmareClouds.end());

    if (allClouds.empty())
    {
        sync = IcecrownHelpers::ValithriaCloudSync{};
        return false;
    }

    // Find whether the shared target cloud is still alive
    Creature* target = nullptr;
    if (!sync.targetCloudGuid.IsEmpty())
    {
        for (Creature* c : allClouds)
        {
            if (c->GetGUID() == sync.targetCloudGuid)
            {
                target = c;
                break;
            }
        }
    }

    // Advance to a new cloud only when the current one is gone or the wait window has passed.
    // The wait window gives every portal bot time to teleport in and collect the same cloud.
    if (!target || nowMs >= sync.moveOnAfterMs)
    {
        Creature* closest = nullptr;
        float minDist = FLT_MAX;
        for (Creature* c : allClouds)
        {
            float const d = leader->GetExactDist(c);
            if (d < minDist)
            {
                minDist = d;
                closest = c;
            }
        }

        target = closest;
        if (target)
            sync.targetCloudGuid = target->GetGUID();
        else
            sync.targetCloudGuid = ObjectGuid::Empty;
        sync.moveOnAfterMs = nowMs + CLOUD_SYNC_WAIT_MS;
    }

    // All bots teleport to the shared target so none miss the cloud
    if (target)
        bot->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                        bot->GetOrientation());

    return false;
}

// Collect all live clouds sorted by distance from the given reference unit
std::vector<Creature*> IccValithriaDreamCloudAction::CollectClouds(uint32 entry, Unit* reference)
{
    constexpr float SEARCH_RANGE = 200.0f;

    std::list<Creature*> raw;
    bot->GetCreatureListWithEntryInGrid(raw, entry, SEARCH_RANGE);

    std::vector<Creature*> result;
    result.reserve(raw.size());
    for (Creature* c : raw)
        if (c && c->IsAlive())
            result.push_back(c);

    // Sort by reference unit's distance so all bots get the same ordering
    std::sort(result.begin(), result.end(), [reference](Creature const* a, Creature const* b)
              { return reference->GetExactDist(a) < reference->GetExactDist(b); });

    return result;
}

bool IccValithriaZombieKiteAction::Execute(Event /*event*/)
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    std::list<Creature*> allZombies;
    bot->GetCreatureListWithEntryInGrid(allZombies, NPC_BLISTERING_ZOMBIE, 100.0f);

    std::vector<Creature*> threatZombies;
    Creature* nearestThreat = nullptr;
    float nearestDist = FLT_MAX;
    for (Creature* z : allZombies)
    {
        if (!z || !z->IsAlive())
            continue;
        if (z->GetVictim() != bot)
            continue;
        threatZombies.push_back(z);
        float const d = bot->GetExactDist2d(z);
        if (d < nearestDist)
        {
            nearestDist = d;
            nearestThreat = z;
        }
    }

    if (threatZombies.empty())
        return false;

    if (!bot->HasAura(SPELL_NITRO_BOOSTS))
        bot->AddAura(SPELL_NITRO_BOOSTS, bot);

    // Stop active channeling that would root the bot
    bot->CastStop();

    constexpr float ANCHOR_RADIUS_LIMIT = 25.0f;  // kite stays within this radius of heal anchor
    constexpr float STEP = 18.0f;                 // how far we move per tick
    constexpr float SAFE_FROM_ZOMBIE = 14.0f;     // candidate must be this far from every zombie
    constexpr float PATH_CLEARANCE = 8.0f;        // path segment must miss every zombie by this much
    constexpr int NUM_CANDIDATES = 36;

    float const anchorX = ICC_VDW_HEAL_POSITION.GetPositionX();
    float const anchorY = ICC_VDW_HEAL_POSITION.GetPositionY();

    auto segmentDistToPoint = [](float x1, float y1, float x2, float y2, float px, float py) -> float
    {
        float const vx = x2 - x1;
        float const vy = y2 - y1;
        float const wx = px - x1;
        float const wy = py - y1;
        float const len2 = vx * vx + vy * vy;
        float t = len2 > 0.0001f ? (vx * wx + vy * wy) / len2 : 0.0f;
        if (t < 0.0f)
            t = 0.0f;
        else if (t > 1.0f)
            t = 1.0f;
        float const cx = x1 + t * vx;
        float const cy = y1 + t * vy;
        float const dx = px - cx;
        float const dy = py - cy;
        return std::sqrt(dx * dx + dy * dy);
    };

    // Compute weighted-away-from-all-zombies vector. Closer zombies pull harder.
    float awayX = 0.0f;
    float awayY = 0.0f;
    for (Creature* z : threatZombies)
    {
        float const dx = bot->GetPositionX() - z->GetPositionX();
        float const dy = bot->GetPositionY() - z->GetPositionY();
        float const d = std::sqrt(dx * dx + dy * dy);
        if (d < 0.1f)
            continue;
        float const w = 1.0f / std::max(d, 1.0f);
        awayX += (dx / d) * w;
        awayY += (dy / d) * w;
    }
    float const awayLen = std::sqrt(awayX * awayX + awayY * awayY);
    float baseAngle = 0.0f;
    if (awayLen > 0.001f)
        baseAngle = std::atan2(awayY / awayLen, awayX / awayLen);
    else if (nearestThreat)
        baseAngle = std::atan2(bot->GetPositionY() - nearestThreat->GetPositionY(),
                               bot->GetPositionX() - nearestThreat->GetPositionX());

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    float bestX = bx;
    float bestY = by;
    float bestScore = -FLT_MAX;

    // Sample arc centered on away-direction, +/-150 degrees
    for (int i = 0; i < NUM_CANDIDATES; ++i)
    {
        float const offset = (-150.0f + (300.0f * i) / (NUM_CANDIDATES - 1)) * static_cast<float>(M_PI) / 180.0f;
        float const angle = baseAngle + offset;
        float const cx = bx + STEP * std::cos(angle);
        float const cy = by + STEP * std::sin(angle);

        // Stay within the anchor radius so kite doesn't drag bot out of raid
        float const dxAnchor = cx - anchorX;
        float const dyAnchor = cy - anchorY;
        if (std::sqrt(dxAnchor * dxAnchor + dyAnchor * dyAnchor) > ANCHOR_RADIUS_LIMIT)
            continue;

        // Reject if endpoint too close to any zombie OR straight path passes near one
        float minEndDist = FLT_MAX;
        bool unsafe = false;
        for (Creature* z : threatZombies)
        {
            float const zx = z->GetPositionX();
            float const zy = z->GetPositionY();
            float const dxe = cx - zx;
            float const dye = cy - zy;
            float const endDist = std::sqrt(dxe * dxe + dye * dye);
            if (endDist < SAFE_FROM_ZOMBIE)
            {
                unsafe = true;
                break;
            }
            float const pathDist = segmentDistToPoint(bx, by, cx, cy, zx, zy);
            if (pathDist < PATH_CLEARANCE)
            {
                unsafe = true;
                break;
            }
            if (endDist < minEndDist)
                minEndDist = endDist;
        }
        if (unsafe)
            continue;

        // Prefer points that maximize distance from the closest zombie.
        // Mild bonus for being aligned with away-direction (smaller offset).
        float const alignBonus = 1.5f * (1.0f - std::fabs(offset) / static_cast<float>(M_PI));
        float const score = minEndDist + alignBonus;
        if (score > bestScore)
        {
            bestScore = score;
            bestX = cx;
            bestY = cy;
        }
    }

    if (bestScore == -FLT_MAX)
    {
        // No safe arc point - sprint straight away from the weighted center,
        // but clamp endpoint to the anchor radius so we don't flee out of raid.
        float fx = bx + STEP * std::cos(baseAngle);
        float fy = by + STEP * std::sin(baseAngle);
        float const dxA = fx - anchorX;
        float const dyA = fy - anchorY;
        float const distA = std::sqrt(dxA * dxA + dyA * dyA);
        if (distA > ANCHOR_RADIUS_LIMIT)
        {
            fx = anchorX + (dxA / distA) * ANCHOR_RADIUS_LIMIT;
            fy = anchorY + (dyA / distA) * ANCHOR_RADIUS_LIMIT;
        }
        return MoveTo(bot->GetMapId(), fx, fy, bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED);
    }

    return MoveTo(bot->GetMapId(), bestX, bestY, bot->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED);
}
