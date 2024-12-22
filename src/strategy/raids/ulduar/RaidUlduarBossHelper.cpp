#include "ChatHelper.h"
#include "RaidUlduarBossHelper.h"
#include "ObjectAccessor.h"
#include "GameObject.h"
#include "Group.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "World.h"

// Prevent harpoon spam
std::unordered_map<ObjectGuid, time_t> RazorscaleBossHelper::_harpoonCooldowns;
// Prevent role assignment spam
std::unordered_map<ObjectGuid, std::time_t> RazorscaleBossHelper::_lastRoleSwapTime;
const std::time_t RazorscaleBossHelper::_roleSwapCooldown; 

bool RazorscaleBossHelper::UpdateBossAI()
{
    _boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (_boss)
    {
        Group* group = bot->GetGroup();
        if (group && !AreRolesAssigned())
        {
            AssignRolesBasedOnHealth();
        }
        return true;
    }
    return false;
}

Unit* RazorscaleBossHelper::GetBoss() const
{
    return _boss;
}

bool RazorscaleBossHelper::IsGroundPhase() const
{
    return _boss && _boss->IsAlive() &&
           (_boss->GetPositionZ() <= RAZORSCALE_FLYING_Z_THRESHOLD) &&
           (_boss->GetHealthPct() < 50.0f) &&
           !_boss->HasAura(SPELL_STUN_AURA);
}

bool RazorscaleBossHelper::IsFlyingPhase() const
{
    return _boss && (!IsGroundPhase() || _boss->GetPositionZ() >= RAZORSCALE_FLYING_Z_THRESHOLD);
}

bool RazorscaleBossHelper::IsHarpoonFired(uint32 chainSpellId) const
{
    return _boss && _boss->HasAura(chainSpellId);
}

bool RazorscaleBossHelper::IsHarpoonReady(GameObject* harpoonGO)
{
    if (!harpoonGO)
        return false;

    auto it = _harpoonCooldowns.find(harpoonGO->GetGUID());
    if (it != _harpoonCooldowns.end())
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
    _harpoonCooldowns[harpoonGO->GetGUID()] = currentTime;
}

GameObject* RazorscaleBossHelper::FindNearestHarpoon(float x, float y, float z) const
{
    GameObject* nearestHarpoon = nullptr;
    float minDistanceSq = std::numeric_limits<float>::max();

    for (const auto& harpoon : GetHarpoonData())
    {
        if (GameObject* harpoonGO = bot->FindNearestGameObject(harpoon.gameObjectEntry, 200.0f))
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

const std::vector<RazorscaleBossHelper::HarpoonData>& RazorscaleBossHelper::GetHarpoonData()
{
    static const std::vector<HarpoonData> harpoonData =
    {
        { GO_RAZORSCALE_HARPOON_1, SPELL_CHAIN_1 },
        { GO_RAZORSCALE_HARPOON_2, SPELL_CHAIN_2 },
        { GO_RAZORSCALE_HARPOON_3, SPELL_CHAIN_3 },
        { GO_RAZORSCALE_HARPOON_4, SPELL_CHAIN_4 },
    };
    return harpoonData;
}

bool RazorscaleBossHelper::AreRolesAssigned() const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Retrieve the group member slot list (GUID + flags + other info)
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    for (auto const& slot : slots)
    {
        // Check if this member has the MAINTANK flag
        if (slot.flags & MEMBER_FLAG_MAINTANK)
        {
            return true;
        }
    }

    return false;
}

bool RazorscaleBossHelper::CanSwapRoles() const
{
    // Identify the GUID of the current bot
    ObjectGuid botGuid = bot->GetGUID();
    if (!botGuid)
        return false;

    // If no entry exists yet for this bot, initialize it to 0
    auto it = _lastRoleSwapTime.find(botGuid);
    if (it == _lastRoleSwapTime.end())
    {
        _lastRoleSwapTime[botGuid] = 0;
        it = _lastRoleSwapTime.find(botGuid);
    }

    // Compare the current time against the stored time
    std::time_t currentTime = std::time(nullptr);
    std::time_t lastSwapTime = it->second;

    return (currentTime - lastSwapTime) >= _roleSwapCooldown;
}


void RazorscaleBossHelper::AssignRolesBasedOnHealth()
{
    // Check if enough time has passed since last swap
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
        if (!member || !botAI->IsTank(member, true) || !member->IsAlive())
            continue;

        Aura* fuseArmor = member->GetAura(SPELL_FUSEARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= FUSEARMOR_THRESHOLD)
            continue;

        tankCandidates.push_back(member);
    }

    // If there are no viable tanks, do nothing
    if (tankCandidates.empty())
        return;

    // Sort by highest max health first
    std::sort(tankCandidates.begin(), tankCandidates.end(),
        [](Player* a, Player* b)
        {
            return a->GetMaxHealth() > b->GetMaxHealth();
        }
    );

    // Pick the top candidate
    Player* newMainTank = tankCandidates[0];
    if (!newMainTank) // Safety check
        return;

    // Unflag everyone from main tank
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
            group->SetGroupMemberFlag(member->GetGUID(), false, MEMBER_FLAG_MAINTANK);
    }

    // Assign the single main tank
    group->SetGroupMemberFlag(newMainTank->GetGUID(), true, MEMBER_FLAG_MAINTANK);

    // Yell a message regardless of whether the new main tank is a bot or a real player
    const std::string playerName = newMainTank->GetName();
    const std::string text = playerName + " set as main tank!";
    bot->Yell(text, LANG_UNIVERSAL);

    ObjectGuid botGuid = bot->GetGUID();
    if (!botGuid)
        return;

    // Set current time in the cooldown map for this bot to start cooldown
    _lastRoleSwapTime[botGuid] = std::time(nullptr);
}
