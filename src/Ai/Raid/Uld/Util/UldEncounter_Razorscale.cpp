/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Razorscale.h"

#include "CellImpl.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "SpellAuras.h"
#include "Timer.h"
#include "UldScripts.h"
#include "Unit.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

using namespace EncounterHelpers;

namespace
{

struct RazorscaleInstanceState
{
    // Prevent harpoon spam
    std::unordered_map<ObjectGuid, std::time_t> harpoonCooldowns;
    // Prevent role assignment spam, per bot
    std::unordered_map<ObjectGuid, std::time_t> lastRoleSwapTime;
};

RaidInstanceState<RazorscaleInstanceState> razorscaleStates;

}  // namespace

bool RazorscaleBossHelper::UpdateBossAI()
{
    _boss = GetRazorscaleScan(botAI).Boss();
    if (_boss)
    {
        Group* group = bot->GetGroup();
        if (group && !AreRolesAssigned())
            AssignRolesBasedOnHealth();
        return true;
    }
    return false;
}

Unit* RazorscaleBossHelper::GetBoss() const
{
    return _boss;
}

bool RazorscaleBossHelper::IsGroundPhaseFor(Unit* boss)
{
    return boss && boss->IsAlive() &&
           (boss->GetPositionZ() <= RAZORSCALE_FLYING_Z_THRESHOLD) &&
           (boss->GetHealthPct() < RAZORSCALE_PERMANENT_GROUND_HEALTH_PCT) &&
           !boss->HasAura(SPELL_STUN_AURA);
}

bool RazorscaleBossHelper::IsFlyingPhaseFor(Unit* boss)
{
    return boss && (!IsGroundPhaseFor(boss) || boss->GetPositionZ() >= RAZORSCALE_FLYING_Z_THRESHOLD);
}

bool RazorscaleBossHelper::IsGroundPhase() const
{
    return IsGroundPhaseFor(_boss);
}

bool RazorscaleBossHelper::IsFlyingPhase() const
{
    return IsFlyingPhaseFor(_boss);
}

Unit* RazorscaleBossHelper::FindDevouringFlameNear(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();

    Unit* nearest = nullptr;
    float best = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != UNIT_DEVOURING_FLAME)
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance > radius)
            continue;

        if (!nearest || distance < best)
        {
            nearest = unit;
            best = distance;
        }
    }

    return nearest;
}

void RazorscaleBossHelper::CollectDevouringFlames(Player* bot, float radius, std::vector<Position>& out)
{
    out.clear();
    if (!bot)
        return;

    // Grid search rather than "nearest hostile npcs": that value ranks by distance to the bot, and the
    // question here is about points he is not standing on yet.
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, UNIT_DEVOURING_FLAME, radius);

    out.reserve(found.size());
    for (Creature* flame : found)
    {
        if (!flame || !flame->IsAlive())
            continue;

        out.push_back(flame->GetPosition());
    }
}

bool RazorscaleBossHelper::DevouringFlameBlocks(std::vector<Position> const& flames, float x, float y)
{
    for (Position const& flame : flames)
    {
        if (flame.GetExactDist2d(x, y) < DEVOURING_FLAME_CLEAR_RADIUS)
            return true;
    }

    return false;
}

bool RazorscaleBossHelper::DevouringFlameBlocks(Player* bot, float x, float y)
{
    if (!bot)
        return false;

    std::vector<Position> flames;
    CollectDevouringFlames(bot, DEVOURING_FLAME_CLEAR_RADIUS + bot->GetExactDist2d(x, y), flames);

    return DevouringFlameBlocks(flames, x, y);
}

bool RazorscaleBossHelper::IsHarpoonReady(GameObject* harpoonGO)
{
    if (!harpoonGO)
        return false;

    // A spent harpoon keeps standing there, flagged unselectable, until the controller rebuilds it.
    if (harpoonGO->HasGameObjectFlag(GO_FLAG_NOT_SELECTABLE))
        return false;

    auto const& harpoonCooldowns = razorscaleStates.For(harpoonGO->GetInstanceId()).harpoonCooldowns;
    auto it = harpoonCooldowns.find(harpoonGO->GetGUID());
    if (it != harpoonCooldowns.end())
    {
        time_t currentTime = std::time(nullptr);
        time_t elapsedTime = currentTime - it->second;
        if (elapsedTime < HARPOON_COOLDOWN_DURATION)
            return false;
    }

    return harpoonGO->GetGoState() == GO_STATE_READY;
}

void RazorscaleBossHelper::SetHarpoonOnCooldown(GameObject* harpoonGO)
{
    if (!harpoonGO)
        return;

    time_t currentTime = std::time(nullptr);
    razorscaleStates.For(harpoonGO->GetInstanceId()).harpoonCooldowns[harpoonGO->GetGUID()] = currentTime;
}

GameObject* RazorscaleBossHelper::FindNearestHarpoon(float x, float y, float z) const
{
    GameObject* nearestHarpoon = nullptr;
    float minDistanceSq = std::numeric_limits<float>::max();

    for (auto const& harpoon : GetHarpoonData())
    {
        if (GameObject* harpoonGO = bot->FindNearestGameObject(harpoon.gameObjectEntry, HARPOON_SEARCH_RANGE))
        {
            float dx = harpoonGO->GetPositionX() - x;
            float dy = harpoonGO->GetPositionY() - y;
            float dz = harpoonGO->GetPositionZ() - z;
            float distanceSq = dx * dx + dy * dy + dz * dz;

            if (distanceSq < minDistanceSq)
            {
                minDistanceSq = distanceSq;
                nearestHarpoon = harpoonGO;
            }
        }
    }

    return nearestHarpoon;
}

std::vector<RazorscaleBossHelper::HarpoonData> const& RazorscaleBossHelper::GetHarpoonData()
{
    // Only two of these exist in 10-man; the missing entries simply never resolve to a GameObject.
    static const std::vector<HarpoonData> harpoonData =
    {
        { GO_RAZORSCALE_HARPOON_1 },
        { GO_RAZORSCALE_HARPOON_2 },
        { GO_RAZORSCALE_HARPOON_3 },
        { GO_RAZORSCALE_HARPOON_4 },
    };
    return harpoonData;
}

bool RazorscaleBossHelper::AreRolesAssigned() const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Group::MemberSlotList const& slots = group->GetMemberSlots();
    for (auto const& slot : slots)
    {
        if (slot.flags & MEMBER_FLAG_MAINTANK)
            return true;
    }

    return false;
}

bool RazorscaleBossHelper::CanSwapRoles() const
{
    ObjectGuid botGuid = bot->GetGUID();
    if (!botGuid)
        return false;

    auto& lastRoleSwapTime = razorscaleStates.For(bot->GetInstanceId()).lastRoleSwapTime;
    auto it = lastRoleSwapTime.find(botGuid);
    if (it == lastRoleSwapTime.end())
    {
        lastRoleSwapTime[botGuid] = 0;
        it = lastRoleSwapTime.find(botGuid);
    }

    std::time_t currentTime = std::time(nullptr);
    std::time_t lastSwapTime = it->second;

    return (currentTime - lastSwapTime) >= _roleSwapCooldown;
}

void RazorscaleBossHelper::AssignRolesBasedOnHealth()
{
    if (!CanSwapRoles())
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    // Gather all tank-capable players (bots + real players), excluding those with too many Fuse Armor stacks
    std::vector<Player*> tankCandidates;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsTank(member, true) || !member->IsAlive())
            continue;

        Aura* fuseArmor = member->GetAura(SPELL_FUSE_ARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= FUSEARMOR_THRESHOLD)
            continue;

        tankCandidates.push_back(member);
    }

    if (tankCandidates.empty())
        return;

    std::sort(tankCandidates.begin(), tankCandidates.end(),
        [](Player* a, Player* b)
        {
            return a->GetMaxHealth() > b->GetMaxHealth();
        }
    );

    Player* newMainTank = tankCandidates[0];
    if (!newMainTank)
        return;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && PlayerbotAI::IsMainTank(member))
            group->SetGroupMemberFlag(member->GetGUID(), false, MEMBER_FLAG_MAINTANK);
    }

    group->SetGroupMemberFlag(newMainTank->GetGUID(), true, MEMBER_FLAG_MAINTANK);

    const std::string playerName = newMainTank->GetName();
    const std::string text = playerName + " set as main tank!";
    bot->Yell(text, LANG_UNIVERSAL);

    ObjectGuid botGuid = bot->GetGUID();
    if (!botGuid)
        return;

    razorscaleStates.For(bot->GetInstanceId()).lastRoleSwapTime[botGuid] = std::time(nullptr);
}

namespace
{

// True when atMs already holds this tick's answer; otherwise stamps it and the caller refills.
bool FreshThisTick(uint32& atMs)
{
    uint32 const now = getMSTime();
    if (atMs && atMs == now)
        return true;

    atMs = now;
    return false;
}

struct AnyRazorscaleHarpoonCheck
{
    bool operator()(GameObject* go) const
    {
        for (RazorscaleBossHelper::HarpoonData const& harpoon : RazorscaleBossHelper::GetHarpoonData())
            if (go->GetEntry() == harpoon.gameObjectEntry)
                return true;

        return false;
    }
};

}

Unit* RazorscaleScan::Boss()
{
    if (!FreshThisTick(bossAtMs))
    {
        Unit* found = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "razorscale")->Get();
        boss = found ? found->GetGUID() : ObjectGuid::Empty;
    }

    return boss.IsEmpty() ? nullptr : botAI->GetUnit(boss);
}

GuidVector const& RazorscaleScan::Hostiles()
{
    if (!FreshThisTick(hostilesAtMs))
        hostiles = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

    return hostiles;
}

GuidVector const& RazorscaleScan::PossibleTargets()
{
    if (!FreshThisTick(targetsAtMs))
        targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    return targets;
}

std::array<ObjectGuid, RazorscaleBossHelper::HARPOON_ENTRY_COUNT> const& RazorscaleScan::NearestHarpoons()
{
    if (FreshThisTick(harpoonsAtMs))
        return harpoons;

    harpoons.fill(ObjectGuid::Empty);

    Player* bot = botAI->GetBot();
    if (!bot)
        return harpoons;

    // One visit, in the order FindNearestGameObject's own searcher walks them (same phase filter, same
    // cells), so replaying its range check per entry over the list picks the same object it would.
    AnyRazorscaleHarpoonCheck anyHarpoon;
    std::vector<GameObject*> found;
    Acore::GameObjectListSearcher<AnyRazorscaleHarpoonCheck> searcher(bot, found, anyHarpoon);
    Cell::VisitObjects(bot, searcher, RazorscaleBossHelper::HARPOON_SEARCH_RANGE);

    std::vector<RazorscaleBossHelper::HarpoonData> const& harpoonData = RazorscaleBossHelper::GetHarpoonData();
    for (std::size_t i = 0; i < harpoonData.size() && i < harpoons.size(); ++i)
    {
        Acore::NearestGameObjectEntryInObjectRangeCheck nearest(*bot, harpoonData[i].gameObjectEntry,
                                                                RazorscaleBossHelper::HARPOON_SEARCH_RANGE);
        GameObject* pick = nullptr;
        for (GameObject* go : found)
            if (nearest(go))
                pick = go;

        if (pick)
            harpoons[i] = pick->GetGUID();
    }

    return harpoons;
}

RazorscaleScan& GetRazorscaleScan(PlayerbotAI* botAI)
{
    return *botAI->GetAiObjectContext()->GetValue<RazorscaleScan*>("razorscale scan")->Get();
}

bool IsRazorscaleHarpoonCrew(Player* bot)
{
    return PlayerbotAI::IsRanged(bot) && PlayerbotAI::IsDps(bot) && !PlayerbotAI::IsHeal(bot);
}

GameObject* GetRazorscaleClosestReadyHarpoon(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    GameObject* closest = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (ObjectGuid const& guid : GetRazorscaleScan(botAI).NearestHarpoons())
    {
        if (guid.IsEmpty())
            continue;

        GameObject* harpoon = ObjectAccessor::GetGameObject(*bot, guid);
        if (!harpoon || !RazorscaleBossHelper::IsHarpoonReady(harpoon))
            continue;

        float const distance = bot->GetDistance2d(harpoon);
        if (distance < minDistance)
        {
            minDistance = distance;
            closest = harpoon;
        }
    }

    return closest;
}

Unit* GetRazorscaleAddKillTarget(PlayerbotAI* botAI)
{
    // All three tiers off one pass. Lowest health Sentinel so the skull doesn't flip between two as the
    // marking bot moves (list order is per bot and not stable); first found for the other two.
    Unit* sentinel = nullptr;
    Unit* watcher = nullptr;
    Unit* guardian = nullptr;

    for (ObjectGuid const& guid : GetRazorscaleScan(botAI).PossibleTargets())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL:
                if (!sentinel || unit->GetHealth() < sentinel->GetHealth())
                    sentinel = unit;
                break;
            case RazorscaleBossHelper::UNIT_DARK_RUNE_WATCHER:
                if (!watcher)
                    watcher = unit;
                break;
            case RazorscaleBossHelper::UNIT_DARK_RUNE_GUARDIAN:
                if (!guardian)
                    guardian = unit;
                break;
            default:
                break;
        }
    }

    if (sentinel)
        return sentinel;

    return watcher ? watcher : guardian;
}

bool IsRazorscaleAtFirstLanding(Unit* boss)
{
    return std::fabs(boss->GetPositionX() - RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_X) <
               RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_TOLERANCE &&
           std::fabs(boss->GetPositionY() - RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_Y) <
               RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_TOLERANCE &&
           std::fabs(boss->GetPositionZ() - RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_Z) <
               RazorscaleBossHelper::RAZORSCALE_FIRST_LANDING_Z_TOLERANCE;
}

Unit* GetRazorscaleKillTarget(PlayerbotAI* botAI)
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return nullptr;

    if (boss->GetPositionZ() <= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
        return boss;

    return GetRazorscaleAddKillTarget(botAI);
}
