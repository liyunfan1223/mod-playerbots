/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTActions.h"
#include "BTHelpers.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <vector>

using namespace BlackTempleHelpers;
using namespace EncounterHelpers;

// General

bool BlackTempleEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const ObjectGuid guid = bot->GetGUID();
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    if (PlayerbotAI::IsTank(bot))
    {
        bool erased = false;
        if (!AI_VALUE2(Unit*, "find target", "illidan stormrage") &&
            !AI_VALUE2(Unit*, "find target", "flame of azzinoth"))
        {
            if (illidanBossDpsWaitTimer.erase(instanceId) > 0)
                erased = true;
            if (illidanFlameDpsWaitTimer.erase(instanceId) > 0)
                erased = true;
            if (illidanLastPhase.erase(instanceId) > 0)
                erased = true;
            if (illidanShadowTrapGuid.erase(guid) > 0)
                erased = true;
            if (illidanShadowTrapDestination.erase(guid) > 0)
                erased = true;
            if (flameTankWaypointIndex.erase(guid) > 0)
                erased = true;
            if (westFlameGuid.erase(instanceId) > 0)
                erased = true;
            if (eastFlameGuid.erase(instanceId) > 0)
                erased = true;
        }
        if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        {
            if (councilDpsWaitTimer.erase(instanceId) > 0)
                erased = true;
            if (gathiosTankStep.erase(guid) > 0)
                erased = true;
        }
        if (!AI_VALUE2(Unit*, "find target", "mother shahraz") &&
            shahrazTankStep.erase(guid) > 0)
        {
            erased = true;
        }
        return erased;
    }
    else if (PlayerbotAI::IsHeal(bot))
    {
        if (zerevorHealStep.erase(guid) > 0)
            return true;
        else
            return false;
    }
    else
    {
        bool erased = false;
        if (!AI_VALUE2(Unit*, "find target", "supremus") &&
            supremusPhaseTimer.erase(instanceId) > 0)
        {
            erased = true;
        }
        if (!AI_VALUE2(Unit*, "find target", "ashtongue channeler") &&
            hasReachedAkamaChannelerPosition.erase(guid) > 0)
        {
            erased = true;
        }
        if (!AI_VALUE2(Unit*, "find target", "gurtogg bloodboil") &&
            gurtoggPhaseTimer.erase(instanceId) > 0)
        {
            erased = true;
        }
        return erased;
    }
}

// High Warlord Naj'entus

bool HighWarlordNajentusMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", najentus))
    {
        return botAI->CastSpell("steady shot", najentus);
    }

    return false;
}

bool HighWarlordNajentusTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    if (AI_VALUE(Unit*, "current target") != najentus)
        return Attack(najentus);

    if (najentus->GetVictim() == bot && bot->IsWithinMeleeRange(najentus))
    {
        Position const& position = NAJENTUS_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool HighWarlordNajentusDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    constexpr uint32 minInterval = 1000;

    constexpr float safeDistFromBoss = 10.0f;
    if (bot->GetExactDist2d(najentus) < safeDistFromBoss &&
        FleePosition(najentus->GetPosition(), safeDistFromBoss, minInterval))
    {
        return true;
    }

    constexpr float safeDistFromPlayer = 7.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

    return false;
}

bool HighWarlordNajentusRemoveImpalingSpineAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* impaledPlayer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_IMPALING_SPINE)))
        {
            impaledPlayer = member;
            break;
        }
    }
    if (!impaledPlayer)
        return false;

    constexpr float searchRadius = 30.0f;
    GameObject* spineGo = bot->FindNearestGameObject(
        static_cast<uint32>(BlackTempleObjects::GO_NAJENTUS_SPINE), searchRadius, true);
    if (!spineGo)
        return false;

    if (bot->GetExactDist2d(spineGo) > 3.0f)
    {
        const uint32 delay = urand(2000, 3000);
        const ObjectGuid spineGuid = spineGo->GetGUID();

        botAI->AddTimedEvent(
            [this, spineGuid]()
            {
                if (GameObject* targetSpine = botAI->GetGameObject(spineGuid))
                {
                    MoveTo(BLACK_TEMPLE_MAP_ID, targetSpine->GetPositionX(),
                           targetSpine->GetPositionY(), bot->GetPositionZ(),
                           false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                           true, false);
                }
            },
            delay);

        return true;
    }
    else
    {
        const uint32 delay = urand(1000, 2000);
        const ObjectGuid spineGuid = spineGo->GetGUID();

        botAI->AddTimedEvent(
            [this, spineGuid]()
            {
                if (GameObject* targetSpine = botAI->GetGameObject(spineGuid))
                    targetSpine->Use(bot);
            },
            delay);

        return true;
    }

    return false;
}

bool HighWarlordNajentusThrowImpalingSpineAction::Execute(Event /*event*/)
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus)
        return false;

    if (bot->GetExactDist2d(najentus) > 24.0f)
    {
        const float angle = atan2(bot->GetPositionY() - najentus->GetPositionY(),
                                  bot->GetPositionX() - najentus->GetPositionX());
        const float targetX = najentus->GetPositionX() + 23.0f * std::cos(angle);
        const float targetY = najentus->GetPositionY() + 23.0f * std::sin(angle);

        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                      true, false);
    }

    if (bot->GetItemByEntry(static_cast<uint32>(BlackTempleItems::ITEM_NAJENTUS_SPINE)))
    {
        const uint32 delay = urand(500, 1500);
        const ObjectGuid najentusGuid = najentus->GetGUID();

        botAI->AddTimedEvent(
            [this, najentusGuid]()
            {
                Item* targetSpine = bot->GetItemByEntry(
                    static_cast<uint32>(BlackTempleItems::ITEM_NAJENTUS_SPINE));
                Unit* targetNajentus = botAI->GetUnit(najentusGuid);
                if (targetSpine && targetNajentus)
                    botAI->ImbueItem(targetSpine, targetNajentus);
            },
            delay);

        return true;
    }

    return false;
}

// Supremus

bool SupremusMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref && hunters.size() < 3; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER)
            hunters.push_back(member);
    }

    if (hunters.empty())
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(bot, 1);

    Player* misdirectTarget = nullptr;
    if (bot == hunters[0] && mainTank)
        misdirectTarget = mainTank;
    else if (hunters.size() > 1 && bot == hunters[1] && firstAssistTank)
        misdirectTarget = firstAssistTank;
    else if (hunters.size() > 2 && bot == hunters[2] && secondAssistTank)
        misdirectTarget = secondAssistTank;

    if (!misdirectTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", misdirectTarget))
        return botAI->CastSpell("misdirection", misdirectTarget);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", supremus))
    {
        return botAI->CastSpell("steady shot", supremus);
    }

    return false;
}

bool SupremusDisperseRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 8.0f;
    constexpr uint32 minInterval = 1000;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}

bool SupremusKiteBossAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    constexpr float safeDistance = 25.0f;
    const float currentDistance = bot->GetDistance2d(supremus);
    if (currentDistance < safeDistance)
        return MoveAway(supremus, safeDistance - currentDistance);

    return false;
}

bool SupremusMoveAwayFromVolcanosAction::Execute(Event /*event*/)
{
    auto const& volcanos = GetAllSupremusVolcanos();
    if (volcanos.empty())
        return false;

    constexpr float hazardRadius = 16.0f;
    bool inDanger = false;
    for (Unit* volcano : volcanos)
    {
        if (bot->GetDistance2d(volcano) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxRadius = 40.0f;
    Position safestPos = FindSafestNearbyPosition(volcanos, maxRadius, hazardRadius);

    return MoveTo(BLACK_TEMPLE_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

Position SupremusMoveAwayFromVolcanosAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& volcanos, float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = 0.0f;
         distance <= maxRadius; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);

            bool isSafe = true;
            for (Unit* volcano : volcanos)
            {
                if (volcano->GetDistance2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, bot->GetPositionZ());

            bool pathSafe =
                IsPathSafeFromVolcanos(bot->GetPosition(), testPos, volcanos, hazardRadius);
            if (pathSafe || !foundSafe)
            {
                float moveDistance = bot->GetExactDist2d(x, y);

                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool SupremusMoveAwayFromVolcanosAction::IsPathSafeFromVolcanos(Position const& start,
    Position const& end, std::vector<Unit*> const& volcanos, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* volcano : volcanos)
        {
            float distToVol = volcano->GetDistance2d(checkX, checkY);
            if (distToVol < hazardRadius)
                return false;
        }
    }

    return true;
}

std::vector<Unit*> SupremusMoveAwayFromVolcanosAction::GetAllSupremusVolcanos()
{
    std::vector<Unit*> volcanos;
    constexpr float searchRadius = 40.0f;

    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(
        BlackTempleNpcs::NPC_SUPREMUS_VOLCANO), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            volcanos.push_back(creature);
    }

    return volcanos;
}

bool SupremusManagePhaseTimerAction::Execute(Event /*event*/)
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    supremusPhaseTimer.try_emplace(
        supremus->GetMap()->GetInstanceId(), std::time(nullptr));

    return false;
}

// Shade of Akama

bool ShadeOfAkamaMeleeDpsPrioritizeChannelersAction::Execute(Event /*event*/)
{
    if (!hasReachedAkamaChannelerPosition.count(bot->GetGUID()))
    {
        Position const& position = AKAMA_CHANNELER_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
        else
        {
            hasReachedAkamaChannelerPosition.insert(bot->GetGUID());
        }
    }

    constexpr float searchRadius = 30.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(BlackTempleNpcs::NPC_ASHTONGUE_CHANNELER), searchRadius);

    std::vector<Creature*> channelers;
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            channelers.push_back(creature);
    }

    if (channelers.empty())
        return false;

    std::sort(channelers.begin(), channelers.end(),
        [](Creature* first, Creature* second) { return first->GetGUID() < second->GetGUID(); });

    Creature* const channeler = channelers.front();

    if (MarkTargetWithSkull(bot, channeler))
        return true;

    if (AI_VALUE(Unit*, "current target") != channeler)
        return Attack(channeler);

    return false;
}

// Teron Gorefiend

bool TeronGorefiendMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", gorefiend))
    {
        return botAI->CastSpell("steady shot", gorefiend);
    }

    return false;
}

bool TeronGorefiendTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    if (MarkTargetWithSkull(bot, gorefiend))
        return true;

    if (AI_VALUE(Unit*, "current target") != gorefiend)
        return Attack(gorefiend);

    if (gorefiend->GetVictim() == bot && bot->IsWithinMeleeRange(gorefiend))
    {
        Position const& position = GOREFIEND_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 3.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Assume positions in arc at the edge of the balcony (farthest from Constructs)
bool TeronGorefiendPositionRangedOnBalconyAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    size_t count = rangedMembers.size();
    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex = (findIt != rangedMembers.end()) ?
        std::distance(rangedMembers.begin(), findIt) : 0;

    constexpr float arcSpan = 2.0f * M_PI / 5.0f;
    constexpr float arcCenter = 6.279f;
    constexpr float arcStart = arcCenter - arcSpan / 2.0f;

    constexpr float radius = 12.0f;
    const float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));

    const float targetX = GOREFIEND_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    const float targetY = GOREFIEND_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TeronGorefiendAvoidShadowOfDeathAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_HUNTER:
        return botAI->CanCastSpell("feign death", bot) &&
               botAI->CastSpell("feign death", bot);

        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                   botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                   botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("vanish", bot) &&
                   botAI->CastSpell("vanish", bot);

        default:
            return false;
    }
}

bool TeronGorefiendMoveToCornerToDieAction::Execute(Event /*event*/)
{
    Position const& position = GOREFIEND_DIE_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool TeronGorefiendControlAndDestroyShadowyConstructsAction::Execute(Event /*event*/)
{
    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    Unit* spirit = bot->GetCharm();
    if (!spirit)
        return false;

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();
    Unit* priorityTarget = nullptr;
    uint32 highestHp = std::numeric_limits<uint32>::min();

    float closestToGorefiend = std::numeric_limits<float>::max();

    for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() ||
            unit->GetEntry() != static_cast<uint32>(BlackTempleNpcs::NPC_SHADOWY_CONSTRUCT))
            continue;

        uint32 hp = unit->GetHealth();
        float distToGorefiend = gorefiend->GetExactDist2d(unit);

        if (hp > highestHp)
        {
            highestHp = hp;
            priorityTarget = unit;
            closestToGorefiend = distToGorefiend;
        }
        else if ((hp == highestHp) && (distToGorefiend < closestToGorefiend))
        {
            priorityTarget = unit;
            closestToGorefiend = distToGorefiend;
        }
    }

    if (priorityTarget)
    {
        const float distToTarget = spirit->GetExactDist2d(priorityTarget);
        constexpr float desiredDist = 10.0f;
        if (distToTarget > desiredDist)
        {
            const float moveDist = distToTarget - desiredDist + 2.0f;
            const float dX = priorityTarget->GetPositionX() - spirit->GetPositionX();
            const float dY = priorityTarget->GetPositionY() - spirit->GetPositionY();
            const float moveX = spirit->GetPositionX() + (dX / distToTarget) * moveDist;
            const float moveY = spirit->GetPositionY() + (dY / distToTarget) * moveDist;

            spirit->GetMotionMaster()->MovePoint(0, moveX, moveY, spirit->GetPositionZ());
            return true;
        }

        // Adding cooldowns manually is needed due to the charmed creature not observing cooldowns,
        // including the GCD. The ordering, including repeating some spells, is the product of testing
        // to try to keep the bot from breaking chains with volley, which tends to happen when volley
        // is cast before chains (maybe due to projectile travel time?)
        if (!spirit->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS)) &&
            priorityTarget->GetHealthPct() == 100.0f)
        {
            spirit->CastSpell(
                priorityTarget, static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS), true);
            spirit->AddSpellCooldown(
                static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS), 0, 15000);
            return true;
        }
        else if (!spirit->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_LANCE)))
        {
            spirit->CastSpell(
                priorityTarget, static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_LANCE), true);
            spirit->AddSpellCooldown(
                static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_LANCE), 0, 1000);
            return true;
        }
        else if (!spirit->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS)))
        {
            spirit->CastSpell(
                priorityTarget, static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS), true);
            spirit->AddSpellCooldown(
                static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS), 0, 15000);
            return true;
        }
        else if (!spirit->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_LANCE)))
        {
            spirit->CastSpell
                (priorityTarget, static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_LANCE), true);
            spirit->AddSpellCooldown(
                static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_LANCE), 0, 1000);
            return true;
        }
        else if (!spirit->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_VOLLEY)) &&
                 !priorityTarget->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_CHAINS)))
        {
            spirit->CastSpell
                (priorityTarget, static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_VOLLEY), true);
            spirit->AddSpellCooldown(
                static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_VOLLEY), 0, 15000);
            return true;
        }
    }
    else
    {
        const float distToGorefiend = spirit->GetExactDist2d(gorefiend);
        constexpr float targetDist = 5.0f;
        if (distToGorefiend > targetDist)
        {
            const float moveDist = distToGorefiend - targetDist;
            const float dX = gorefiend->GetPositionX() - spirit->GetPositionX();
            const float dY = gorefiend->GetPositionY() - spirit->GetPositionY();
            const float moveX = spirit->GetPositionX() + (dX / distToGorefiend) * moveDist;
            const float moveY = spirit->GetPositionY() + (dY / distToGorefiend) * moveDist;

            spirit->GetMotionMaster()->MovePoint(0, moveX, moveY, spirit->GetPositionZ());
            return true;
        }
        else if (!spirit->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_STRIKE)))
        {
            spirit->CastSpell(
                gorefiend, static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_STRIKE), true);
            spirit->AddSpellCooldown(
                static_cast<uint32>(BlackTempleSpells::SPELL_SPIRIT_STRIKE), 0, 1000);
            return true;
        }
    }

    return false;
}

// Gurtogg Bloodboil

bool GurtoggBloodboilMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", gurtogg))
    {
        return botAI->CastSpell("steady shot", gurtogg);
    }

    return false;
}

bool GurtoggBloodboilTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    if (AI_VALUE(Unit*, "current target") != gurtogg)
        return Attack(gurtogg);

    Unit* victim = gurtogg->GetVictim();
    Player* playerVictim = victim ? victim->ToPlayer() : nullptr;
    if (playerVictim && PlayerbotAI::IsTank(playerVictim) && bot->IsWithinMeleeRange(gurtogg))
    {
        Position const& position = GURTOGG_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool GurtoggBloodboilRotateRangedGroupsAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto groups = GetGurtoggRangedRotationGroups(bot);
    int activeGroup = GetGurtoggActiveRotationGroup(gurtogg);

    bool inActiveGroup = false;
    if (activeGroup >= 0 && (size_t)activeGroup < groups.size())
    {
        auto const& group = groups[activeGroup];
        inActiveGroup = std::find(group.begin(), group.end(), bot) != group.end();
    }

    Position const& nearPosition = GURTOGG_RANGED_POSITION;
    Position const& farPosition = GURTOGG_SOAKER_POSITION;
    constexpr float distFromPos = 2.0f;

    if (inActiveGroup && bot->GetExactDist2d(farPosition) > distFromPos)
    {
        return MoveInside(BLACK_TEMPLE_MAP_ID, farPosition.GetPositionX(),
                          farPosition.GetPositionY(), bot->GetPositionZ(),
                          distFromPos, MovementPriority::MOVEMENT_FORCED);
    }
    else if (!inActiveGroup && bot->GetExactDist2d(nearPosition) > distFromPos)
    {
        return MoveInside(BLACK_TEMPLE_MAP_ID, nearPosition.GetPositionX(),
                          nearPosition.GetPositionY(), bot->GetPositionZ(),
                          distFromPos, MovementPriority::MOVEMENT_FORCED);
    }

    return false;
}

bool GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* enragedPlayer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(
                static_cast<uint32>(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)))
        {
            enragedPlayer = member;
            break;
        }
    }

    constexpr float safeDistance = 20.0f;
    constexpr uint32 minInterval = 0;
    if (enragedPlayer && bot->GetExactDist2d(enragedPlayer) < safeDistance)
        return FleePosition(enragedPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}

bool GurtoggBloodboilManagePhaseTimerAction::Execute(Event /*event*/)
{
    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = gurtogg->GetMap()->GetInstanceId();

    if (gurtogg->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_BOSS_FEL_RAGE)))
    {
        return gurtoggPhaseTimer.erase(instanceId) > 0;
    }
    else
    {
        auto [it, inserted] = gurtoggPhaseTimer.try_emplace(instanceId, now);
        return inserted;
    }
}

// Reliquary of Souls

bool ReliquaryOfSoulsMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");

    if (!desire && !anger)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    Unit* target = desire ? desire : anger;

    if (target->GetHealthPct() > 95.0f)
    {
        if (botAI->CanCastSpell("misdirection", mainTank))
            return botAI->CastSpell("misdirection", mainTank);

        if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
            botAI->CanCastSpell("steady shot", target))
        {
            return botAI->CastSpell("steady shot", target);
        }
    }

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    if (PlayerbotAI::IsTank(bot) && bot->GetHealthPct() > 25.0f)
        return TanksMoveToMinimumRange(suffering);
    else if (PlayerbotAI::IsMelee(bot) && bot->GetVictim() != suffering)
        return MeleeDpsStayAtMaximumRange(suffering);
    else if (PlayerbotAI::IsRanged(bot))
        return RangedMoveAwayFromBoss(suffering);

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::TanksMoveToMinimumRange(Unit* suffering)
{
    const float distanceToBoss = bot->GetExactDist2d(suffering);
    if (distanceToBoss > 2.0f)
    {
        const float dX = suffering->GetPositionX() - bot->GetPositionX();
        const float dY = suffering->GetPositionY() - bot->GetPositionY();
        const float targetX = bot->GetPositionX() + (dX / distanceToBoss);
        const float targetY = bot->GetPositionY() + (dY / distanceToBoss);

        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::MeleeDpsStayAtMaximumRange(Unit* suffering)
{
    const float desiredDist = bot->GetMeleeRange(suffering);
    const float behindAngle = Position::NormalizeOrientation(suffering->GetOrientation() + M_PI);
    const float targetX = suffering->GetPositionX() + desiredDist * std::cos(behindAngle);
    const float targetY = suffering->GetPositionY() + desiredDist * std::sin(behindAngle);

    if (bot->GetExactDist2d(targetX, targetY) > 0.25f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool ReliquaryOfSoulsAdjustDistanceFromSufferingAction::RangedMoveAwayFromBoss(Unit* suffering)
{
    constexpr float safeDistance = 15.0f;
    constexpr uint32 minInterval = 1000;
    if (bot->GetExactDist2d(suffering) < safeDistance)
        return FleePosition(suffering->GetPosition(), safeDistance, minInterval);

    return false;
}

bool ReliquaryOfSoulsHealersDpsSufferingAction::Execute(Event /*event*/)
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    if (!suffering)
        return false;

    if (bot->getClass() == CLASS_DRUID)
    {
        if (botAI->HasAura("tree of life", bot))
            botAI->RemoveAura("tree of life");

        bool casted = false;

        if (botAI->CanCastSpell("barkskin", bot) &&
            botAI->CastSpell("barkskin", bot))
            casted = true;

        if (botAI->CanCastSpell("wrath", suffering) &&
            botAI->CastSpell("wrath", suffering))
            casted = true;

        return casted;
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        bool casted = false;

        if (botAI->CanCastSpell("avenging wrath", bot) &&
            botAI->CastSpell("avenging wrath", bot))
            casted = true;

        if (botAI->CanCastSpell("consecration", bot) &&
            botAI->CastSpell("consecration", bot))
            casted = true;

        if (botAI->CanCastSpell("exorcism", suffering) &&
            botAI->CastSpell("exorcism", suffering))
            casted = true;

        if (botAI->CanCastSpell("hammer of wrath", suffering) &&
            botAI->CastSpell("hammer of wrath", suffering))
            casted = true;

        if (botAI->CanCastSpell("holy shock", suffering) &&
            botAI->CastSpell("holy shock", suffering))
            casted = true;

        if (botAI->CanCastSpell("judgement of light", suffering) &&
            botAI->CastSpell("judgement of light", suffering))
            casted = true;

        return casted;
    }
    else if (bot->getClass() == CLASS_PRIEST)
    {
        if (botAI->CanCastSpell("smite", suffering))
            return botAI->CastSpell("smite", suffering);
    }
    else if (bot->getClass() == CLASS_SHAMAN)
    {
        bool casted = false;

        if (botAI->CanCastSpell("earth shock", suffering) &&
            botAI->CastSpell("earth shock", suffering))
            casted = true;

        if (botAI->CanCastSpell("chain lightning", suffering) &&
            botAI->CastSpell("chain lightning", suffering))
            casted = true;

        if (botAI->CanCastSpell("lightning bolt", suffering) &&
            botAI->CastSpell("lightning bolt", suffering))
            casted = true;

        return casted;
    }

    return false;
}

bool ReliquaryOfSoulsSpellstealRuneShieldAction::Execute(Event /*event*/)
{
    if (Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
        desire && botAI->CanCastSpell("spellsteal", desire))
    {
        return botAI->CastSpell("spellsteal", desire);
    }

    return false;
}

bool ReliquaryOfSoulsSpellReflectDeadenAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

// Mother Shahraz

bool MotherShahrazMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", shahraz))
    {
        return botAI->CastSpell("steady shot", shahraz);
    }

    return false;
}

bool MotherShahrazTanksPositionBossUnderPillarAction::Execute(Event /*event*/)
{
    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz)
        return false;

    if (AI_VALUE(Unit*, "current target") != shahraz)
        return Attack(shahraz);

    Unit* victim = shahraz->GetVictim();
    Player* playerVictim = victim ? victim->ToPlayer() : nullptr;
    if (playerVictim && PlayerbotAI::IsTank(playerVictim))
    {
        const ObjectGuid guid = bot->GetGUID();
        auto it = shahrazTankStep.try_emplace(
            guid, TankPositionState::MovingToTransition).first;
        TankPositionState state = it->second;

        constexpr float maxDistance = 0.5f;
        Position const& position = state == TankPositionState::MovingToTransition ?
            SHAHRAZ_TRANSITION_POSITION : SHAHRAZ_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position);

        if (distToPosition > maxDistance && bot->IsWithinMeleeRange(shahraz))
        {
            const bool backwards = (shahraz->GetVictim() == bot);
            return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }

        if (state == TankPositionState::MovingToTransition && distToPosition <= maxDistance)
            shahrazTankStep[guid] = TankPositionState::MovingToFinal;

        if (state != TankPositionState::MovingToTransition && distToPosition <= maxDistance)
        {
            const float orientation = atan2(shahraz->GetPositionY() - bot->GetPositionY(),
                                            shahraz->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
            shahrazTankStep[guid] = TankPositionState::Positioned;
        }
    }

    return false;
}

bool MotherShahrazMeleeDpsWaitAtSafePositionAction::Execute(Event /*event*/)
{
    return MoveTo(BLACK_TEMPLE_MAP_ID, SHAHRAZ_RANGED_POSITION.GetPositionX(),
                  SHAHRAZ_RANGED_POSITION.GetPositionY(), bot->GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// This doesn't matter for bots since they don't take fall damage, and it's actually easier
// to tank her closer to her starting position, but I want to simulate a player strategy
bool MotherShahrazPositionRangedUnderPillarAction::Execute(Event /*event*/)
{
    Position const& position = SHAHRAZ_RANGED_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 1.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MotherShahrazRunAwayToBreakFatalAttractionAction::Execute(Event /*event*/)
{
    std::vector<Player*> attractedPlayers = GetAttractedPlayers();
    if (attractedPlayers.size() < 2)
        return false;

    float centerX = 0.0f, centerY = 0.0f;
    for (Player* member : attractedPlayers)
    {
        centerX += member->GetPositionX();
        centerY += member->GetPositionY();
    }
    centerX /= attractedPlayers.size();
    centerY /= attractedPlayers.size();

    auto botIt = std::find(attractedPlayers.begin(), attractedPlayers.end(), bot);
    if (botIt == attractedPlayers.end())
        return false;

    const float spreadAngle =
        2.0f * M_PI * std::distance(attractedPlayers.begin(), botIt) / attractedPlayers.size();

    constexpr float maxSpreadDistance = 35.0f;
    constexpr float distanceStep = 1.0f;
    float lastValidX = bot->GetPositionX();
    float lastValidY = bot->GetPositionY();
    float lastValidZ = bot->GetPositionZ();

    for (float currentDistance = distanceStep;
         currentDistance <= maxSpreadDistance;
         currentDistance += distanceStep)
    {
        float testX = centerX + std::cos(spreadAngle) * currentDistance;
        float testY = centerY + std::sin(spreadAngle) * currentDistance;
        float testZ = lastValidZ;

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), testX, testY, testZ))
        {
            break;
        }

        lastValidX = testX;
        lastValidY = testY;
        lastValidZ = testZ;
    }

    if (MoveTo(BLACK_TEMPLE_MAP_ID, lastValidX, lastValidY, lastValidZ, false, false,
               false, false, MovementPriority::MOVEMENT_FORCED, true, false))
    {
        return true;
    }
    else
    {
        // In case bots get stuck, try a 5-yard random move
        const float angle = frand(0.0f, 2.0f * M_PI);
        constexpr float dist = 5.0f;
        float randX = bot->GetPositionX() + std::cos(angle) * dist;
        float randY = bot->GetPositionY() + std::sin(angle) * dist;
        float randZ = lastValidZ;
        bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            randX, randY, randZ);

        return MoveTo(BLACK_TEMPLE_MAP_ID, randX, randY, randZ, false, false, false,
                      false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
}

std::vector<Player*> MotherShahrazRunAwayToBreakFatalAttractionAction::GetAttractedPlayers()
{
    std::vector<Player*> attractedPlayers;
    Group* group = bot->GetGroup();
    if (!group)
        return attractedPlayers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(static_cast<uint32>(
            BlackTempleSpells::SPELL_FATAL_ATTRACTION)))
        {
            attractedPlayers.push_back(member);
        }
    }

    std::sort(attractedPlayers.begin(), attractedPlayers.end(),
        [](Player* a, Player* b) {
            return a->GetGUID().GetCounter() < b->GetGUID().GetCounter();
        });

    return attractedPlayers;
}

// Illidari Council

bool IllidariCouncilMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 4)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* councilTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
        tankTarget = GetZerevorMageTank(bot);
    }
    else if (hunterIndex == 1)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "lady malande");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetGroupAssistTank(bot, 0))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetGroupMainTank(bot))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 3)
    {
        councilTarget = AI_VALUE2(Unit*, "find target", "veras darkshadow");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = GetGroupAssistTank(bot, 1))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!councilTarget || !tankTarget || !tankTarget->IsAlive())
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", councilTarget))
    {
        return botAI->CastSpell("steady shot", councilTarget);
    }

    return false;
}

bool IllidariCouncilMainTankPositionGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BLACK_TEMPLE_MAP_ID, gathios->GetPositionX(), gathios->GetPositionY(),
                        gathios->GetPositionZ(), bot->GetOrientation());
    }

    if (MarkTargetWithSquare(bot, gathios))
        return true;

    SetRtiTarget(botAI, "square");

    if (AI_VALUE(Unit*, "current target") != gathios)
        return Attack(gathios);

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = gathiosTankStep.count(guid) ? gathiosTankStep[guid] : 0;

    Position const& position = GATHIOS_TANK_POSITIONS[index];

    constexpr float maxDistance = 2.0f;
    float distToPosition = bot->GetExactDist2d(position);

    if (gathios->GetVictim() == bot && bot->IsWithinMeleeRange(gathios))
    {
        if (distToPosition <= maxDistance && HasDangerousCouncilAura(bot))
        {
            index = (index + 1) % 4;
            gathiosTankStep[guid] = index;
            Position const& newPosition = GATHIOS_TANK_POSITIONS[index];
            const float newDistToPosition = bot->GetExactDist2d(newPosition);
            if (newDistToPosition > maxDistance)
            {
                const float dX = newPosition.GetPositionX() - bot->GetPositionX();
                const float dY = newPosition.GetPositionY() - bot->GetPositionY();
                const float moveDist = std::min(5.0f, newDistToPosition);
                const float moveX = bot->GetPositionX() + (dX / newDistToPosition) * moveDist;
                const float moveY = bot->GetPositionY() + (dY / newDistToPosition) * moveDist;

                return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                              false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
        else if (distToPosition > maxDistance)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool IllidariCouncilMainTankReflectJudgementOfCommandAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("spell reflection", bot))
        return botAI->CastSpell("spell reflection", bot);

    return false;
}

bool IllidariCouncilFirstAssistTankFocusMalandeAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BLACK_TEMPLE_MAP_ID, malande->GetPositionX(), malande->GetPositionY(),
                        malande->GetPositionZ(), bot->GetOrientation());
    }

    if (MarkTargetWithStar(bot, malande))
        return true;

    SetRtiTarget(botAI, "star");

    if (AI_VALUE(Unit*, "current target") != malande)
        return Attack(malande);

    return false;
}

bool IllidariCouncilSecondAssistTankPositionDarkshadowAction::Execute(Event /*event*/)
{
    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (!darkshadow)
        return false;

    // Failsafe for if bot falls through the floor, which tends to happen upon the pull
    if (bot->GetPositionZ() < COUNCIL_FLOOR_Z_THRESHOLD)
    {
        bot->TeleportTo(BLACK_TEMPLE_MAP_ID, darkshadow->GetPositionX(), darkshadow->GetPositionY(),
                        darkshadow->GetPositionZ(), bot->GetOrientation());
    }

    if (MarkTargetWithCircle(bot, darkshadow))
        return true;

    SetRtiTarget(botAI, "circle");

    if (AI_VALUE(Unit*, "current target") != darkshadow)
        return Attack(darkshadow);

    if (darkshadow->GetVictim() == bot)
    {
        Player* mainTank = GetGroupMainTank(bot);
        if (!mainTank)
            return false;

        const float distToPosition = bot->GetExactDist2d(mainTank->GetPositionX(),
                                                         mainTank->GetPositionY());
        if (distToPosition > 2.0f)
        {
            const float dX = mainTank->GetPositionX() - bot->GetPositionX();
            const float dY = mainTank->GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            const bool backwards = bot->GetExactDist2d(mainTank) < 10.0f;
            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
        }
    }

    return false;
}

bool IllidariCouncilMageTankPositionZerevorAction::Execute(Event /*event*/)
{
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor)
        return false;

    if (zerevor->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_DAMPEN_MAGIC)) &&
        botAI->CanCastSpell("spellsteal", zerevor))
    {
        return botAI->CastSpell("spellsteal", zerevor);
    }

    if (MarkTargetWithTriangle(bot, zerevor))
        return true;

    SetRtiTarget(botAI, "triangle");

    if (AI_VALUE(Unit*, "current target") != zerevor)
        return Attack(zerevor);

    if (zerevor->GetVictim() == bot)
    {
        Position const& position = ZEREVOR_TANK_POSITION;
        const float distToPosition = bot->GetExactDist2d(position.GetPositionX(),
                                                         position.GetPositionY());
        if (distToPosition > 2.0f)
        {
            const float dX = position.GetPositionX() - bot->GetPositionX();
            const float dY = position.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(10.0f, distToPosition);
            const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool IllidariCouncilPositionMageTankHealerAction::Execute(Event /*event*/)
{
    Player* mageTank = GetZerevorMageTank(bot);
    if (!mageTank)
        return false;

    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (!zerevor || zerevor->GetVictim() != mageTank)
        return false;

    const ObjectGuid guid = bot->GetGUID();
    uint8 index = zerevorHealStep.count(guid) ? zerevorHealStep[guid] : 0;

    Position const& position = ZEREVOR_HEALER_POSITIONS[index];

    constexpr float maxDistance = 1.0f;
    const float distToPosition = bot->GetExactDist2d(position);

    if (distToPosition <= maxDistance && HasDangerousCouncilAura(bot))
    {
        index = (index + 1) % 2;
        zerevorHealStep[guid] = index;
        Position const& newPosition = ZEREVOR_HEALER_POSITIONS[index];
        const float newDistToPosition = bot->GetExactDist2d(newPosition);
        if (newDistToPosition > maxDistance)
        {
            return MoveTo(BLACK_TEMPLE_MAP_ID, newPosition.GetPositionX(),
                          newPosition.GetPositionY(), bot->GetPositionZ(),
                          false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    else if (distToPosition > maxDistance)
    {
        const float dX = position.GetPositionX() - bot->GetPositionX();
        const float dY = position.GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(10.0f, distToPosition);
        const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool IllidariCouncilDisperseRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistance = 4.0f;
    constexpr uint32 minInterval = 1000;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        return FleePosition(nearestPlayer->GetPosition(), safeDistance, minInterval);

    return false;
}

bool IllidariCouncilCommandPetsToAttackGathiosAction::Execute(Event /*event*/)
{
    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios)
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->IsAlive() && pet->GetVictim() != gathios)
    {
        pet->ClearUnitState(UNIT_STATE_FOLLOW);
        pet->AttackStop();
        pet->SetTarget(gathios->GetGUID());

        if (pet->GetCharmInfo())
        {
            pet->GetCharmInfo()->SetIsCommandAttack(true);
            pet->GetCharmInfo()->SetIsAtStay(false);
            pet->GetCharmInfo()->SetIsFollowing(false);
            pet->GetCharmInfo()->SetIsCommandFollow(false);
            pet->GetCharmInfo()->SetIsReturning(false);

            pet->AI()->AttackStart(gathios);
            return true;
        }
    }

    return false;
}

bool IllidariCouncilAssignDpsTargetsAction::Execute(Event /*event*/)
{
    Unit* malande = AI_VALUE2(Unit*, "find target", "lady malande");
    if (!malande)
        return false;

    bool shouldAttackMalande = false;
    Unit* zerevor = AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
    if (zerevor && zerevor->GetExactDist2d(malande) < 15.0f)
    {
        shouldAttackMalande = false;
    }
    else if (bot->getClass() == CLASS_ROGUE ||
             (bot->getClass() == CLASS_WARRIOR && PlayerbotAI::IsDps(bot)))
    {
        shouldAttackMalande = !malande->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_BLESSING_OF_PROTECTION));
    }
    else if (bot->getClass() == CLASS_SHAMAN && PlayerbotAI::IsDps(bot))
    {
        shouldAttackMalande = !malande->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_BLESSING_OF_SPELL_WARDING));
    }

    if (shouldAttackMalande)
    {
        SetRtiTarget(botAI, "star");

        if (AI_VALUE(Unit*, "current target") != malande)
            return Attack(malande);
    }
    else if (Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
             darkshadow && !darkshadow->HasAura(
                static_cast<uint32>(BlackTempleSpells::SPELL_VANISH)))
    {
        SetRtiTarget(botAI, "circle");

        if (AI_VALUE(Unit*, "current target") != darkshadow)
            return Attack(darkshadow);
    }
    else if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        SetRtiTarget(botAI, "square");

        if (AI_VALUE(Unit*, "current target") != gathios)
            return Attack(gathios);
    }

    return false;
}

bool IllidariCouncilManageDpsTimerAction::Execute(Event /*event*/)
{
    if (Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
    {
        return councilDpsWaitTimer.try_emplace(
            gathios->GetMap()->GetInstanceId(), std::time(nullptr)).second;
    }

    return false;
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageMisdirectToTankAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int phase = GetIllidanPhase(illidan);

    if (phase == 2 && TryMisdirectToFlameTanks(group))
        return true;

    return phase == 4 && TryMisdirectToWarlockTank(illidan);
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToFlameTanks(Group* group)
{
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
        {
            hunters.push_back(member);
        }

        if (hunters.size() >= 2)
            break;
    }

    int8 hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int8>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    if (!eastFlame || !westFlame || eastFlame == westFlame)
        return false;

    Player* firstAssistTank = GetGroupAssistTank(bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(bot, 1);
    if (!firstAssistTank || !secondAssistTank)
        return false;

    if (hunters.size() == 1)
    {
        if (eastFlame->GetHealthPct() < 99.0f)
            return false;

        if (botAI->CanCastSpell("misdirection", secondAssistTank))
            return botAI->CastSpell("misdirection", secondAssistTank);

        if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
            botAI->CanCastSpell("steady shot", eastFlame))
        {
            return botAI->CastSpell("steady shot", eastFlame);
        }

        return false;
    }

    Player* tankTarget = nullptr;
    Unit* flame = nullptr;

    if (hunterIndex == 0)
    {
        tankTarget = secondAssistTank;
        flame = eastFlame;
    }
    else if (hunterIndex == 1)
    {
        tankTarget = firstAssistTank;
        flame = westFlame;
    }
    else
        return false;

    if (!tankTarget || !tankTarget->IsAlive() || flame->GetHealthPct() < 90.0f)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", flame))
    {
        return botAI->CastSpell("steady shot", flame);
    }

    return false;
}

bool IllidanStormrageMisdirectToTankAction::TryMisdirectToWarlockTank(Unit* illidan)
{
    Player* warlockTank = GetIllidanWarlockTank(bot);
    if (!warlockTank)
        return false;

    if (botAI->CanCastSpell("misdirection", warlockTank))
        return botAI->CastSpell("misdirection", warlockTank);

    if (bot->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", illidan))
    {
        return botAI->CastSpell("steady shot", illidan);
    }

    return false;
}

bool IllidanStormrageMainTankRepositionBossAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (AI_VALUE(Unit*, "current target") != illidan)
        return Attack(illidan);

    if (GetIllidanPhase(illidan) == 5)
    {
        GameObject* trap = FindNearestTrap(botAI, bot);
        if (trap && bot->GetExactDist2d(trap) < 40.0f && illidan->GetVictim() == bot)
            return MoveToShadowTrap(trap);
    }
    else
    {
        illidanShadowTrapGuid.erase(bot->GetGUID());
        illidanShadowTrapDestination.erase(bot->GetGUID());
    }

    if (illidan->GetVictim() != bot)
    {
        illidanShadowTrapGuid.erase(bot->GetGUID());
        illidanShadowTrapDestination.erase(bot->GetGUID());
        return false;
    }

    auto const& flameCrashes = GetAllFlameCrashes(bot);
    if (flameCrashes.empty())
        return false;

    constexpr float hazardRadius = 12.0f;
    bool inDanger = false;
    for (Unit* flameCrash : flameCrashes)
    {
        if (bot->GetDistance2d(flameCrash) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxRadius = 30.0f;
    Position safestPos = FindSafestNearbyPosition(flameCrashes, maxRadius, hazardRadius);

    return MoveTo(BLACK_TEMPLE_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, true);
}

bool IllidanStormrageMainTankRepositionBossAction::MoveToShadowTrap(GameObject* trap)
{
    if (!trap)
        return false;

    ObjectGuid const botGuid = bot->GetGUID();
    ObjectGuid const trapGuid = trap->GetGUID();
    Position target;

    auto const cachedTrapIt = illidanShadowTrapGuid.find(botGuid);
    auto const cachedDestinationIt = illidanShadowTrapDestination.find(botGuid);
    if (cachedTrapIt != illidanShadowTrapGuid.end() &&
        cachedDestinationIt != illidanShadowTrapDestination.end() &&
        cachedTrapIt->second == trapGuid)
    {
        target = cachedDestinationIt->second;
    }
    else
    {
        const float trapX = trap->GetPositionX();
        const float trapY = trap->GetPositionY();

        const float distToTrap = trap->GetExactDist2d(bot);
        if (distToTrap <= 0.0f)
            return false;

        constexpr float distBeyondTrap = 6.0f;

        const float dx = trapX - bot->GetPositionX();
        const float dy = trapY - bot->GetPositionY();
        const float targetX = trapX + (dx / distToTrap) * distBeyondTrap;
        const float targetY = trapY + (dy / distToTrap) * distBeyondTrap;

        target = Position(targetX, targetY, trap->GetPositionZ());
        illidanShadowTrapGuid[botGuid] = trapGuid;
        illidanShadowTrapDestination[botGuid] = target;
    }

    const float targetDist = bot->GetExactDist2d(target);

    if (targetDist > 2.0f && bot->GetHealthPct() > 50.0f)
    {
        const float dX = target.GetPositionX() - bot->GetPositionX();
        const float dY = target.GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(5.0f, targetDist);
        const float moveX = bot->GetPositionX() + (dX / targetDist) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / targetDist) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY, trap->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

Position IllidanStormrageMainTankRepositionBossAction::FindSafestNearbyPosition(
    std::vector<Unit*> const& flameCrashes, float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 16.0f;
    constexpr float minDistance = 2.0f;
    constexpr float distanceStep = 1.0f;

    float backwardsAngle = Position::NormalizeOrientation(bot->GetOrientation() + M_PI);

    Position bestPos;
    float bestAngleDiff = M_PI * 2.0f;
    float bestDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = minDistance; distance <= maxRadius; distance += distanceStep)
    {
        for (float angleOffset = 0.0f; angleOffset < 2 * M_PI; angleOffset += searchStep)
        {
            for (int sign = -1; sign <= 1; sign += 2)
            {
                const float testAngle =
                    Position::NormalizeOrientation(backwardsAngle + sign * angleOffset);
                const float x = bot->GetPositionX() + distance * std::cos(testAngle);
                const float y = bot->GetPositionY() + distance * std::sin(testAngle);

                Position testPos(x, y, bot->GetPositionZ());

                bool isSafe = true;
                for (Unit* flameCrash : flameCrashes)
                {
                    if (flameCrash->GetDistance2d(x, y) < hazardRadius)
                    {
                        isSafe = false;
                        break;
                    }
                }
                if (!isSafe)
                    continue;

                bool pathSafe = IsPathSafeFromFlameCrashes(bot->GetPosition(), testPos,
                                                           flameCrashes, hazardRadius);

                float angleDiff = std::abs(Position::NormalizeOrientation(
                                           testAngle - backwardsAngle));
                if (angleDiff > M_PI)
                    angleDiff = 2 * M_PI - angleDiff;

                if (pathSafe && (!foundSafe || angleDiff < bestAngleDiff ||
                    (angleDiff == bestAngleDiff && distance < bestDistance)))
                {
                    bestPos = testPos;
                    bestAngleDiff = angleDiff;
                    bestDistance = distance;
                    foundSafe = true;
                }
                else if (!foundSafe && angleDiff < bestAngleDiff)
                {
                    bestPos = testPos;
                    bestAngleDiff = angleDiff;
                    bestDistance = distance;
                }
            }
            if (foundSafe)
                break;
        }
        if (foundSafe)
            break;
    }

    return bestPos;
}

bool IllidanStormrageMainTankRepositionBossAction::IsPathSafeFromFlameCrashes(
    Position const& start, Position const& end, std::vector<Unit*> const& flameCrashes,
    float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* flameCrash : flameCrashes)
        {
            float distToFlameCrash = flameCrash->GetDistance2d(checkX, checkY);
            if (distToFlameCrash < hazardRadius)
                return false;
        }
    }

    return true;
}

bool IllidanStormrageIsolateBotWithParasiteAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);

    if (phase == 1)
    {
        constexpr float safeDistance = 15.0f;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance))
        {
            const float currentDistance = bot->GetExactDist2d(nearestPlayer);
            if (currentDistance < safeDistance)
                return MoveAway(nearestPlayer, safeDistance - currentDistance);
        }
    }
    else
    {
        const float angle = illidan->GetOrientation() + M_PI;
        constexpr float distBehindIllidan = 35.0f;

        const float targetX = illidan->GetPositionX() + std::cos(angle) * distBehindIllidan;
        const float targetY = illidan->GetPositionY() + std::sin(angle) * distBehindIllidan;
        const Position target(targetX, targetY, bot->GetPositionZ());

        if (HasParasiticShadowfiend(bot))
            return InfectedBotMoveFromGroup(illidan, target);

        if (GetIllidanTrapperHunter(bot) == bot)
            return FreezeTrapShadowfiend(bot, illidan, target);
    }

    return false;
}

bool IllidanStormrageIsolateBotWithParasiteAction::InfectedBotMoveFromGroup(
    Unit*, Position const& target)
{
    if (bot->GetExactDist2d(target) < 1.0f)
        return false;

    return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                  target.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageIsolateBotWithParasiteAction::FreezeTrapShadowfiend(
    Player* bot, Unit*, Position const& target)
{
    if (bot->HasSpellCooldown(static_cast<uint32>(BlackTempleSpells::SPELL_FROST_TRAP)))
        return false;

    Player* infected = GetBotWithParasiticShadowfiend(bot);
    if (!infected)
        return false;

    if (bot->GetExactDist2d(target) > 2.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, target.GetPositionX(), target.GetPositionY(),
                      target.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else if (bot->GetExactDist2d(infected) < 2.0f &&
             botAI->CanCastSpell(static_cast<uint32>(BlackTempleSpells::SPELL_FROST_TRAP), bot))
    {
        return botAI->CastSpell(static_cast<uint32>(BlackTempleSpells::SPELL_FROST_TRAP), bot);
    }

    return false;
}

bool IllidanStormrageSetEarthbindTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell("earthbind totem", bot) &&
           botAI->CastSpell("earthbind totem", bot);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::Execute(Event /*event*/)
{
    auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
    // The second assist tank's flame is killed first; this is so that if the tank
    // for the second flame dies after the first flame is down, the dead flame's
    // tank will become the first assist tank and take over the remaining flame
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        if (eastFlame && westFlame)
        {
            if (AI_VALUE(Unit*, "current target") != eastFlame)
                return Attack(eastFlame);

            if (eastFlame->GetVictim() != bot)
            {
                if (!bot->IsWithinMeleeRange(eastFlame))
                {
                    return MoveTo(BLACK_TEMPLE_MAP_ID, eastFlame->GetPositionX(),
                                  eastFlame->GetPositionY(), eastFlame->GetPositionZ(),
                                  false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                return false;
            }
        }
        else if (!eastFlame && !westFlame)
        {
            // (1) Before flames spawn, go to the waiting position
            // (2) If both flames are dead and the waiting position is too close to hazards,
            //     move to a grate position
            std::list<Creature*> demonFires;
            constexpr float searchRadius = 40.0f;
            bot->GetCreatureListWithEntryInGrid(
                demonFires, static_cast<uint32>(BlackTempleNpcs::NPC_DEMON_FIRE), searchRadius);

            Position const& pos = demonFires.empty() ?
                ILLIDAN_E_GLAIVE_WAITING_POSITION : ILLIDAN_E_GRATE_POSITION;

            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
        // After the first flame dies, its tank waits with other bots
        else if (!eastFlame && westFlame)
        {
            Position const& pos = ILLIDAN_E_GRATE_POSITION;
            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        if (westFlame)
        {
            if (AI_VALUE(Unit*, "current target") != westFlame)
                return Attack(westFlame);

            if (westFlame->GetVictim() != bot)
            {
                if (!bot->IsWithinMeleeRange(westFlame))
                {
                    return MoveTo(BLACK_TEMPLE_MAP_ID, westFlame->GetPositionX(),
                                  westFlame->GetPositionY(), westFlame->GetPositionZ(),
                                  false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
                return false;
            }
        }
        else
        {
            // (1) Before flames spawn, go to the waiting position
            // (2) If both flames are dead and the waiting position is too close to hazards,
            //     move to a grate position
            std::list<Creature*> demonFires;
            constexpr float searchRadius = 40.0f;
            bot->GetCreatureListWithEntryInGrid(
                demonFires, static_cast<uint32>(BlackTempleNpcs::NPC_DEMON_FIRE), searchRadius);

            Position const& pos = demonFires.empty() ?
                ILLIDAN_W_GLAIVE_WAITING_POSITION : ILLIDAN_W_GRATE_POSITION;

            if (bot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) > 0.5f)
            {
                return MoveTo(BLACK_TEMPLE_MAP_ID, pos.GetPositionX(), pos.GetPositionY(),
                              pos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    EyeBlastDangerArea dangerArea = GetEyeBlastDangerArea(bot);

    // Only consider the eye blast if its trigger NPC is within 30 yards of the tank
    constexpr float eyeBlastTriggerRadius = 30.0f;
    if (dangerArea.width > 0.0f &&
        bot->GetExactDist2d(dangerArea.start) <= eyeBlastTriggerRadius)
    {
        return RepositionToAvoidEyeBlast(illidan, dangerArea);
    }
    else
    {
        return RepositionToAvoidBlaze(eastFlame, westFlame);
    }

    return false;
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidEyeBlast(
    Unit*, EyeBlastDangerArea const& dangerArea)
{
    if (!IsPositionInEyeBlastDangerArea(bot->GetPosition(), dangerArea))
        return false;

    const float dx = dangerArea.end.GetPositionX() - dangerArea.start.GetPositionX();
    const float dy = dangerArea.end.GetPositionY() - dangerArea.start.GetPositionY();
    const float length = dangerArea.start.GetExactDist2d(dangerArea.end);

    const float px = bot->GetPositionX();
    const float py = bot->GetPositionY();
    const float sx = dangerArea.start.GetPositionX();
    const float sy = dangerArea.start.GetPositionY();

    const float projection = std::clamp(
        ((px - sx) * dx + (py - sy) * dy) / (length * length), 0.0f, 1.0f);
    const float closestX = sx + projection * dx;
    const float closestY = sy + projection * dy;

    const float distToLine = bot->GetExactDist2d(closestX, closestY);
    const float moveDist = (dangerArea.width - distToLine) + 0.5f;
    if (moveDist <= 0.0f)
        return false;

    const float rawDirX = px - closestX;
    const float rawDirY = py - closestY;
    const float rawDirLength = std::sqrt(rawDirX * rawDirX + rawDirY * rawDirY);
    const float dirX = rawDirLength == 0.0f ? -(dy / length) : rawDirX / rawDirLength;
    const float dirY = rawDirLength == 0.0f ? dx / length : rawDirY / rawDirLength;

    const float safeX = px + dirX * moveDist;
    const float safeY = py + dirY * moveDist;
    const float safeZ = bot->GetPositionZ();
    const Position safePosition(safeX, safeY, safeZ);

    constexpr float minGrateDistance = 10.0f;
    const bool tooCloseToNorthGrate =
        safePosition.GetExactDist2d(ILLIDAN_N_GRATE_POSITION) < minGrateDistance;
    const bool tooCloseToEastGrate =
        safePosition.GetExactDist2d(ILLIDAN_E_GRATE_POSITION) < minGrateDistance;
    const bool tooCloseToWestGrate =
        safePosition.GetExactDist2d(ILLIDAN_W_GRATE_POSITION) < minGrateDistance;

    if (tooCloseToNorthGrate || tooCloseToEastGrate || tooCloseToWestGrate)
        return false;

    return MoveTo(BLACK_TEMPLE_MAP_ID, safeX, safeY, safeZ, false, false, false,
                  false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction::RepositionToAvoidBlaze(
    Unit* eastFlame, Unit* westFlame)
{
    std::array<Position, 7> const* waypoints = nullptr;
    constexpr size_t numWaypoints = 7;

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 1, true))
    {
        if (!eastFlame || eastFlame->GetVictim() != bot ||
            !bot->IsWithinMeleeRange(eastFlame))
        {
            return false;
        }
        waypoints = &E_GLAIVE_TANK_POSITIONS;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
    {
        if (!westFlame || westFlame->GetVictim() != bot ||
            !bot->IsWithinMeleeRange(westFlame))
        {
            return false;
        }
        waypoints = &W_GLAIVE_TANK_POSITIONS;
    }

    if (!waypoints)
        return false;

    size_t& waypointIndex = flameTankWaypointIndex[bot->GetGUID()];
    Position const& target = (*waypoints)[waypointIndex];

    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible triggers")->Get();

    bool blazeNearby = false;
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == static_cast<uint32>(BlackTempleNpcs::NPC_BLAZE) &&
            bot->GetDistance2d(unit) <= 8.0f)
        {
            blazeNearby = true;
            break;
        }
    }

    float distToPosition =
        bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY());
    if (blazeNearby && distToPosition <= 0.2f)
    {
        waypointIndex = (waypointIndex + 1) % numWaypoints;
        Position const& newTarget = (*waypoints)[waypointIndex];
        const float distToNewPosition =
            bot->GetExactDist2d(newTarget.GetPositionX(), newTarget.GetPositionY());

        if (distToNewPosition > 0.2f)
        {
            const float dX = newTarget.GetPositionX() - bot->GetPositionX();
            const float dY = newTarget.GetPositionY() - bot->GetPositionY();
            const float moveDist = std::min(5.0f, distToNewPosition);
            const float moveX = bot->GetPositionX() + (dX / distToNewPosition) * moveDist;
            const float moveY = bot->GetPositionY() + (dY / distToNewPosition) * moveDist;

            return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY,
                          bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }
    else if (distToPosition > 0.2f)
    {
        const float dX = target.GetPositionX() - bot->GetPositionX();
        const float dY = target.GetPositionY() - bot->GetPositionY();
        const float moveDist = std::min(3.0f, distToPosition);
        const float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        const float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(BLACK_TEMPLE_MAP_ID, moveX, moveY,
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

// Pets grab aggro right away during Phase 2 and wipe the raid if not put on passive
// Just like players, pets cannot melee Illidan during Phase 4
bool IllidanStormrageControlPetAggressionAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    int phase = GetIllidanPhase(illidan);

    if ((phase == 2 || phase == 4) &&
        pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }
    else if (pet->GetReactState() == REACT_PASSIVE)
    {
        pet->SetReactState(REACT_DEFENSIVE);
    }

    return false;
}

bool IllidanStormragePositionAboveGrateAction::Execute(Event /*event*/)
{
    std::array<Position, 3> const& gratePositions = GRATE_POSITIONS;
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> bots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && !PlayerbotAI::IsAssistTankOfIndex(member, 0, true) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 1, true))
        {
            bots.push_back(member);
        }
    }

    if (bots.empty())
        return false;

    std::sort(bots.begin(), bots.end(),
        [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); });

    auto it = std::find(bots.begin(), bots.end(), bot);
    if (it == bots.end())
        return false;

    const size_t botIndex = std::distance(bots.begin(), it);
    const uint8 index = botIndex % gratePositions.size();

    Position const& position = gratePositions[index];
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageRemoveDarkBarrageAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                   botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                   botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                   botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool IllidanStormrageMoveAwayFromLandingPointAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float safeDistance = 20.0f;
    const float currentDistance = bot->GetExactDist2d(illidan);
    if (currentDistance < safeDistance)
        return MoveAway(illidan, safeDistance - currentDistance);

    return false;
}

// NOTE: Illidan's bounding radius is 0.459f, and combatreach is 7.5f
bool IllidanStormrageDisperseRangedAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int phase = GetIllidanPhase(illidan);

    if (phase == 4)
    {
        return SpreadInCircleInDemonPhase(illidan, group);
    }
    else if (GetBotWithParasiticShadowfiend(bot) == bot ||
             (GetIllidanTrapperHunter(bot) == bot &&
              GetBotWithParasiticShadowfiend(bot)))
    {
        return false;
    }
    else
    {
        return FanOutBehindInHumanPhase(illidan, group);
    }
}

bool IllidanStormrageDisperseRangedAction::FanOutBehindInHumanPhase(
    Unit* illidan, Group* group)
{
    auto const& flameCrashes = GetAllFlameCrashes(bot);

    std::vector<Player*> healers;
    std::vector<Player*> rangedDps;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        if (PlayerbotAI::IsHeal(member))
            healers.push_back(member);
        else
            rangedDps.push_back(member);
    }

    constexpr float arcSpan = M_PI;
    const float arcCenter = illidan->GetOrientation() + M_PI;
    const float arcStart = arcCenter - arcSpan / 2.0f;

    const float radius = PlayerbotAI::IsHeal(bot) ? 18.0f : 25.0f;
    auto& bots = PlayerbotAI::IsHeal(bot) ? healers : rangedDps;
    const size_t count = bots.size();
    auto findIt = std::find(bots.begin(), bots.end(), bot);
    const size_t botIndex = (findIt != bots.end()) ?
        std::distance(bots.begin(), findIt) : 0;

    const float angle = (count == 1) ? arcCenter :
        (arcStart + arcSpan * static_cast<float>(botIndex) /
         static_cast<float>(count - 1));

    const float targetX = illidan->GetPositionX() + radius * std::cos(angle);
    const float targetY = illidan->GetPositionY() + radius * std::sin(angle);

    constexpr float hazardRadius = 12.0f;
    bool safe = true;
    for (Unit* flameCrash : flameCrashes)
    {
        if (flameCrash->GetDistance2d(targetX, targetY) < hazardRadius)
        {
            safe = false;
            break;
        }
    }

    if (!safe)
        return false;

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT,
                      true, false);
    }

    return false;
}

bool IllidanStormrageDisperseRangedAction::SpreadInCircleInDemonPhase(
    Unit* illidan, Group* group)
{
    Player* warlockTank = GetIllidanWarlockTank(bot);
    if (!warlockTank)
    {
        constexpr float safeDistFromBoss = 24.0f;
        if (bot->GetExactDist2d(illidan) < safeDistFromBoss)
        {
            constexpr uint32 minInterval = 0;
            if (FleePosition(illidan->GetPosition(), safeDistFromBoss, minInterval))
                return true;
        }

        constexpr float safeDistFromPlayer = 6.0f;
        constexpr uint32 minInterval = 1000;
        if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
            return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);

        return false;
    }

    std::vector<Player*> rangedBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        rangedBots.push_back(member);
    }

    if (rangedBots.empty())
        return false;

    const size_t count = rangedBots.size();
    auto findIt = std::find(rangedBots.begin(), rangedBots.end(), bot);
    const size_t botIndex = (findIt != rangedBots.end()) ?
        std::distance(rangedBots.begin(), findIt) : 0;

    const float dx = warlockTank->GetPositionX() - illidan->GetPositionX();
    const float dy = warlockTank->GetPositionY() - illidan->GetPositionY();
    const float warlockAngle = std::atan2(dy, dx);

    constexpr float forbiddenArc = (2.0f / 3.0f) * M_PI;
    constexpr float allowedArc = (4.0f / 3.0f) * M_PI;

    const float arcStart = Position::NormalizeOrientation(warlockAngle + forbiddenArc / 2.0f);
    constexpr float radius = 25.0f;

    const float angle = (count == 1) ?
        Position::NormalizeOrientation(arcStart + allowedArc / 2.0f) :
            Position::NormalizeOrientation(
                arcStart + allowedArc * static_cast<float>(botIndex) /
                static_cast<float>(count - 1));

    const float targetX = illidan->GetPositionX() + radius * std::cos(angle);
    const float targetY = illidan->GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 1.0f)
    {
        if (MoveTo(BLACK_TEMPLE_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, false))
        {
            return true;
        }
        else
        {
            constexpr float safeDistFromTank = 25.0f;
            const float currentDistFromTank = bot->GetExactDist2d(warlockTank);
            if (currentDistFromTank < safeDistFromTank)
                return MoveAway(warlockTank, safeDistFromTank - currentDistFromTank);
        }
    }

    return false;
}

// Melee cannot attack Demon Form Illidan
bool IllidanStormrageMeleeGoSomewhereToNotDieAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float demonSearchRadius = 25.0f;
    constexpr float shadowfiendSearchRadius = 15.0f;

    Unit* illidanVictim = illidan->GetVictim();
    // But they can attack Shadow Demons and Shadowfiends, if far enough from Illidan
    Unit* shadowDemon = bot->FindNearestCreature(
        static_cast<uint32>(BlackTempleNpcs::NPC_SHADOW_DEMON), demonSearchRadius, true);

    if (shadowDemon && shadowDemon->GetDistance2d(illidan) > 15.0f &&
        (!illidanVictim || shadowDemon->GetDistance2d(illidanVictim) > 24.0f))
    {
        return false;
    }
    else
    {
        Unit* shadowfiend = bot->FindNearestCreature(
            static_cast<uint32>(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND),
            shadowfiendSearchRadius, true);

        if (shadowfiend && shadowfiend->GetDistance2d(illidan) > 15.0f &&
            shadowfiend->GetHealthPct() < 30.0f &&
            (!illidanVictim || shadowfiend->GetDistance2d(illidanVictim) > 24.0f))
        {
            return false;
        }
    }

    // 30y is closer than ideal but is a compromise to allow melee to reach targets in time
    constexpr float safeDistFromBoss = 30.0f;
    const float currentDistFromBoss = bot->GetExactDist2d(illidan);
    if (currentDistFromBoss < safeDistFromBoss)
        MoveAway(illidan, safeDistFromBoss - currentDistFromBoss);

    if (Player* warlockTank = GetIllidanWarlockTank(bot))
    {
        constexpr float safeDistFromTank = 25.0f;
        const float currentDistFromTank = bot->GetExactDist2d(warlockTank);
        if (currentDistFromTank < safeDistFromTank)
            MoveAway(warlockTank, safeDistFromTank - currentDistFromTank);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
        MoveAway(nearestPlayer, safeDistFromPlayer - bot->GetDistance2d(nearestPlayer));

    return true;
}

bool IllidanStormrageWarlockTankHandleDemonBossAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    constexpr float safeDistance = 24.0f;
    const float currentDistance = bot->GetExactDist2d(illidan);
    if (currentDistance < safeDistance &&
        MoveAway(illidan, safeDistance - currentDistance))
    {
        return true;
    }

    if (botAI->CanCastSpell("shadow ward", bot) &&
        botAI->CastSpell("shadow ward", bot))
    {
        return true;
    }

    if (botAI->CanCastSpell("searing pain", illidan))
        return botAI->CastSpell("searing pain", illidan);

    return false;
}

bool IllidanStormrageDpsPrioritizeAddsAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);

    std::vector<Unit*> targets;

    if (phase == 4)
    {
        constexpr float searchRadius = 35.0f;

        Unit* shadowDemon = bot->FindNearestCreature(
            static_cast<uint32>(BlackTempleNpcs::NPC_SHADOW_DEMON), searchRadius, true);

        if (GetIllidanWarlockTank(bot) == bot)
        {
            targets = { shadowDemon, illidan };
        }
        else
        {
            Unit* shadowfiend = bot->FindNearestCreature(
                static_cast<uint32>(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true);

            if (PlayerbotAI::IsRanged(bot))
            {
                if (shadowDemon)
                    targets = { shadowDemon };
                else if (shadowfiend && bot->GetDistance2d(shadowfiend) > 10.0f)
                    targets = { shadowfiend };
                else
                    targets = { illidan };
            }
            else if (PlayerbotAI::IsMelee(bot))
            {
                targets = { shadowDemon, shadowfiend };
            }
        }
    }
    else if (PlayerbotAI::IsRanged(bot))
    {
        if (phase == 1 || phase == 3 || phase == 5)
        {
            constexpr float searchRadius = 35.0f;
            Unit* shadowfiend = bot->FindNearestCreature(
                static_cast<uint32>(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true);

            if (shadowfiend && bot->GetDistance2d(shadowfiend) > 10.0f)
                targets = { shadowfiend };
            else
                targets = { illidan };
        }
        else if (phase == 2)
        {
            constexpr float searchRadius = 20.0f;
            Unit* shadowfiend = bot->FindNearestCreature(
                static_cast<uint32>(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true);

            if (shadowfiend && bot->GetDistance2d(shadowfiend) > 5.0f)
            {
                targets = { shadowfiend };
            }
            else
            {
                auto [eastFlame, westFlame] = GetFlamesOfAzzinoth(bot);
                targets = { eastFlame, westFlame };
            }
        }
    }

    for (Unit* candidate : targets)
    {
        if (candidate && candidate->IsAlive())
        {
            if (AI_VALUE(Unit*, "current target") != candidate)
                return Attack(candidate);

            return false;
        }
    }

    return false;
}

bool IllidanStormrageUseShadowTrapAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    GameObject* trap = FindNearestTrap(botAI, bot);
    if (!trap || illidan->GetExactDist2d(trap) >= 4.0f)
        return false;

    if (bot->GetExactDist2d(trap) < 3.0f)
    {
        trap->Use(bot);
        return true;
    }
    else
    {
        return MoveTo(BLACK_TEMPLE_MAP_ID, trap->GetPositionX(), trap->GetPositionY(),
                      trap->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IllidanStormrageManageDpsTimerAndRtiAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    const time_t now = std::time(nullptr);
    const uint32 instanceId = illidan->GetMap()->GetInstanceId();

    bool updated = false;
    const int phase = GetIllidanPhase(illidan);
    int lastPhase = -1;
    if (auto const it = illidanLastPhase.find(instanceId); it != illidanLastPhase.end())
        lastPhase = it->second;
    const bool phaseChanged = lastPhase != phase;
    illidanLastPhase[instanceId] = phase;

    if (phaseChanged)
    {
        if (phase == 1 || phase == 3 || phase == 4 || phase == 5)
        {
            illidanBossDpsWaitTimer[instanceId] = now;
            updated = true;
        }
        else if (phase == 2)
        {
            if (illidanBossDpsWaitTimer.erase(instanceId) > 0)
                updated = true;
        }

        if (phase != 2 && illidanFlameDpsWaitTimer.erase(instanceId) > 0)
            updated = true;
    }

    if (phase == 2)
    {
        if (eastFlameGuid.find(instanceId) == eastFlameGuid.end() &&
            westFlameGuid.find(instanceId) == westFlameGuid.end())
        {
            std::list<Creature*> creatureList;
            constexpr float searchRadius = 50.0f;
            illidan->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(
                BlackTempleNpcs::NPC_FLAME_OF_AZZINOTH), searchRadius);

            std::vector<Creature*> flames;
            for (Creature* creature : creatureList)
            {
                if (creature && creature->IsAlive())
                    flames.push_back(creature);
            }

            if (flames.size() == 2)
            {
                const float eastDist0 =
                    flames[0]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);
                const float eastDist1 =
                    flames[1]->GetExactDist2d(ILLIDAN_E_GLAIVE_WAITING_POSITION);

                if (eastDist0 < eastDist1)
                {
                    eastFlameGuid[instanceId] = flames[0]->GetGUID();
                    westFlameGuid[instanceId] = flames[1]->GetGUID();
                }
                else
                {
                    eastFlameGuid[instanceId] = flames[1]->GetGUID();
                    westFlameGuid[instanceId] = flames[0]->GetGUID();
                }

                illidanFlameDpsWaitTimer[instanceId] = now;

                updated = true;
            }
        }
    }
    else
    {
        if (eastFlameGuid.erase(instanceId) > 0)
            updated = true;
        if (westFlameGuid.erase(instanceId) > 0)
            updated = true;
    }

    return updated;
}

bool IllidanStormrageDestroyHazardsAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);
    constexpr float searchRadius = 50.0f;
    std::list<Creature*> hazards;
    std::vector<uint32> entries;

    if (phase == 2)
    {
        entries = {
            static_cast<uint32>(BlackTempleNpcs::NPC_FLAME_CRASH)
        };
    }
    else if (phase == 4)
    {
        entries = {
            static_cast<uint32>(BlackTempleNpcs::NPC_FLAME_CRASH)
        };
    }
    else if (phase == 0)
    {
        entries = {
            static_cast<uint32>(BlackTempleNpcs::NPC_DEMON_FIRE),
            static_cast<uint32>(BlackTempleNpcs::NPC_BLAZE)
        };
    }

    if (!entries.empty())
        bot->GetCreatureListWithEntryInGrid(hazards, entries, searchRadius);

    for (Creature* creature : hazards)
    {
        if (creature && creature->IsAlive())
        {
            creature->Kill(bot, creature);
            return true;
        }
    }

    return false;
}

// Reduce Shadow Demon to 25% health and kill residual Shadowfiends in Phase 2
bool IllidanStormrageHandleAddsCheatAction::Execute(Event /*event*/)
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    if (GetIllidanPhase(illidan) == 2)
    {
        constexpr float searchRadius = 20.0f;
        if (Unit* shadowfiend = bot->FindNearestCreature(
                static_cast<uint32>(BlackTempleNpcs::NPC_PARASITIC_SHADOWFIEND),
                searchRadius, true))
        {
            shadowfiend->Kill(bot, shadowfiend);
            return true;
        }
    }
    else
    {
        constexpr float searchRadius = 75.0f;
        Unit* shadowDemon = bot->FindNearestCreature(
            static_cast<uint32>(BlackTempleNpcs::NPC_SHADOW_DEMON), searchRadius, true);

        if (shadowDemon && shadowDemon->GetHealthPct() > 25.0f)
        {
            uint32 desiredDamage = 0;
            const uint32 quarterHealth = shadowDemon->GetMaxHealth() / 4;
            if (shadowDemon->GetHealth() > quarterHealth)
                desiredDamage = shadowDemon->GetHealth() - quarterHealth;

            Unit::DealDamage(bot, shadowDemon, desiredDamage, nullptr, DIRECT_DAMAGE,
                             SPELL_SCHOOL_MASK_NORMAL, nullptr, false, false, nullptr);
            return true;
        }
    }

    return false;
}
