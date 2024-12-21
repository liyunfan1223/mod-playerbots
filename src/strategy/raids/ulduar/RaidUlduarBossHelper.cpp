#include "RaidUlduarBossHelper.h"
#include "ObjectAccessor.h"
#include "GameObject.h"
#include "Group.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "World.h"

std::unordered_map<ObjectGuid, time_t> RazorscaleBossHelper::_harpoonCooldowns;

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

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (botAI->IsMainTank(member))
            return true;
    }

    return false;
}

bool RazorscaleBossHelper::CanSwapRoles() const
{
    std::time_t currentTime = std::time(nullptr);
    return (currentTime - _lastRoleSwapTime) >= _roleSwapCooldown;
}

void RazorscaleBossHelper::AssignRolesBasedOnHealth()
{
    if (!CanSwapRoles())
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    // Gather all tank bots in the group, excluding those with Fuse Armor
    std::vector<Player*> tankBots;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsTank(member) || !member->IsAlive())
            continue;

        Aura* fuseArmor = member->GetAura(SPELL_FUSEARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= FUSEARMOR_THRESHOLD)
            continue;

        tankBots.push_back(member);
    }

    if (tankBots.empty())
        return;

    // Sort tanks by max health descending
    std::sort(tankBots.begin(), tankBots.end(),
        [](Player* a, Player* b)
        {
            return a->GetMaxHealth() > b->GetMaxHealth();
        }
    );

    Player* newMainTank = tankBots[0];

    // Remove all MAINTANK flags
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
            group->SetGroupMemberFlag(member->GetGUID(), false, MEMBER_FLAG_MAINTANK);
    }

    // Assign the single main tank
    group->SetGroupMemberFlag(newMainTank->GetGUID(), true, MEMBER_FLAG_MAINTANK);

    // Notify if the new main tank is a real player
    if (GET_PLAYERBOT_AI(newMainTank)->IsRealPlayer())
    {
        const std::string playerName = newMainTank->GetName();
        const auto text = BOT_TEXT2("%s please taunt Razorscale now!", { playerName });
        botAI->Say(text);
    }

    _lastRoleSwapTime = std::time(nullptr);
}

