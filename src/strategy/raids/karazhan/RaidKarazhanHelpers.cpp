#include <algorithm>
#include <map>

#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"
#include "AiObjectContext.h"
#include "PlayerbotMgr.h"
#include "Position.h"
#include "Spell.h"

const Position KARAZHAN_MAIDEN_OF_VIRTUE_BOSS_POSITION = Position(-10945.881f, -2103.782f, 92.712f);
const Position KARAZHAN_MAIDEN_OF_VIRTUE_RANGED_POSITION[8] =
{
    { -10931.178f, -2116.580f, 92.179f },
    { -10925.828f, -2102.425f, 92.180f },
    { -10933.089f, -2088.5017f, 92.180f },
    { -10947.59f, -2082.8147f, 92.180f },
    { -10960.912f, -2090.4368f, 92.179f },
    { -10966.017f, -2105.288f, 92.175f },
    { -10959.242f, -2119.6172f, 92.180f },
    { -10944.495f, -2123.857f, 92.180f },
};

const Position KARAZHAN_BIG_BAD_WOLF_BOSS_POSITION = Position(-10913.391f, -1773.508f, 90.477f);
const Position KARAZHAN_BIG_BAD_WOLF_RUN_POSITION[4] = 
{
    { -10875.456f, -1779.036f, 90.477f },
    { -10872.281f, -1751.638f, 90.477f },
    { -10910.492f, -1747.401f, 90.477f },    
    { -10913.391f, -1773.508f, 90.477f },
};

const Position KARAZHAN_THE_CURATOR_BOSS_POSITION = Position(-11139.463f, -1884.645f, 165.765f);

void RaidKarazhanHelpers::MarkTargetWithSkull(Unit* target)
{
    if (!target)
    {
        return;
    }

    if (Group* group = bot->GetGroup())
    {
        constexpr uint8_t skullIconId = 7;
        ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);

        if (skullGuid != target->GetGUID())
        {
            group->SetTargetIcon(skullIconId, bot->GetGUID(), target->GetGUID());
        }
    }
}

Unit* RaidKarazhanHelpers::GetFirstAliveUnit(const std::vector<Unit*>& units)
{
    for (Unit* unit : units)
    {
        if (unit && unit->IsAlive())
        {
            return unit;
        }
    }

    return nullptr;
}

Unit* RaidKarazhanHelpers::GetFirstAliveUnitByEntry(uint32 entry)
{
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);

        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
        {
            return unit;
        }
    }

    return nullptr;
}

Unit* RaidKarazhanHelpers::GetNearestPlayerInRadius(float radius)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->IsAlive() || member == bot)
            {
                continue;
            }

            if (bot->GetExactDist2d(member) < radius)
            {
                return member;
            }
        }
    }

    return nullptr;
}

bool RaidKarazhanHelpers::IsFlameWreathActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    Spell* currentSpell = boss ? boss->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    if (currentSpell && currentSpell->m_spellInfo && currentSpell->m_spellInfo->Id == SPELL_FLAME_WREATH)
    {
        return true;
    }

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
            {
                continue;
            }
            if (member->HasAura(SPELL_AURA_FLAME_WREATH))
            {
                return true;
            }
        }
    }

    return false;
}

// Red beam blockers: tank bots, no Nether Exhaustion Red
std::vector<Player*> RaidKarazhanHelpers::GetRedBlockers()
{
    std::vector<Player*> redBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsTank(member) || !GET_PLAYERBOT_AI(member) ||
                member->HasAura(SPELL_NETHER_EXHAUSTION_RED))
            {
                continue;
            }
            redBlockers.push_back(member);
        }
    }

    return redBlockers;
}

// Blue beam blockers: non-Rogue/Warrior DPS bots, no Nether Exhaustion Blue and ≤25 stacks of Blue Beam debuff
std::vector<Player*> RaidKarazhanHelpers::GetBlueBlockers()
{
    std::vector<Player*> blueBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
            {
                continue;
            }
            bool isDps = botAI->IsDps(member);
            bool isWarrior = member->getClass() == CLASS_WARRIOR;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_BLUE);
            Aura* blueBuff = member->GetAura(SPELL_BLUE_BEAM_DEBUFF);
            bool overStack = blueBuff && blueBuff->GetStackAmount() >= 26;
            if (isDps && !isWarrior && !isRogue && !hasExhaustion && !overStack)
            {
                blueBlockers.push_back(member);
            }
        }
    }

    return blueBlockers;
}

// Green beam blockers:
// (1) Rogue and non-tank Warrior bots, no Nether Exhaustion Green
// (2) Healer bots, no Nether Exhaustion Green and ≤25 stacks of Green Beam debuff
std::vector<Player*> RaidKarazhanHelpers::GetGreenBlockers()
{
    std::vector<Player*> greenBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
            {
                continue;
            }
            bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN);
            Aura* greenBuff = member->GetAura(SPELL_GREEN_BEAM_DEBUFF);
            bool overStack = greenBuff && greenBuff->GetStackAmount() >= 26;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool isDpsWarrior = member->getClass() == CLASS_WARRIOR && botAI->IsDps(member);
            bool eligibleRogueWarrior = (isRogue || isDpsWarrior) && !hasExhaustion;
            bool isHealer = botAI->IsHeal(member);
            bool eligibleHealer = isHealer && !hasExhaustion && !overStack;
            if (eligibleRogueWarrior || eligibleHealer)
            {
                greenBlockers.push_back(member);
            }
        }
    }

    return greenBlockers;
}

Position RaidKarazhanHelpers::GetPositionOnBeam(Unit* boss, Unit* portal, float distanceFromBoss)
{
    float bx = boss->GetPositionX();
    float by = boss->GetPositionY();
    float bz = boss->GetPositionZ();
    float px = portal->GetPositionX();
    float py = portal->GetPositionY();

    float dx = px - bx;
    float dy = py - by;
    float length = sqrt(dx*dx + dy*dy);
    if (length == 0.0f)
    {
        return Position(bx, by, bz);
    }

    dx /= length;
    dy /= length;
    float targetX = bx + dx * distanceFromBoss;
    float targetY = by + dy * distanceFromBoss;
    float targetZ = bz;

    return Position(targetX, targetY, targetZ);
}

std::tuple<Player*, Player*, Player*> RaidKarazhanHelpers::GetCurrentBeamBlockers()
{
    static ObjectGuid currentRedBlocker;
    static ObjectGuid currentGreenBlocker;
    static ObjectGuid currentBlueBlocker;

    Player* redBlocker = nullptr;
    Player* greenBlocker = nullptr;
    Player* blueBlocker = nullptr;

    std::vector<Player*> redBlockers = GetRedBlockers();
    if (!redBlockers.empty())
    {
        auto it = std::find_if(redBlockers.begin(), redBlockers.end(), [](Player* p) 
        {
            return p && p->GetGUID() == currentRedBlocker;
        });
        if (it != redBlockers.end())
        {
            redBlocker = *it;
        }
        else
        {
            redBlocker = redBlockers.front();
        }
        currentRedBlocker = redBlocker ? redBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentRedBlocker = ObjectGuid::Empty;
        redBlocker = nullptr;
    }

    std::vector<Player*> greenBlockers = GetGreenBlockers();
    if (!greenBlockers.empty())
    {
        auto it = std::find_if(greenBlockers.begin(), greenBlockers.end(), [](Player* p) 
        {
            return p && p->GetGUID() == currentGreenBlocker;
        });
        if (it != greenBlockers.end())
        {
            greenBlocker = *it;
        }
        else
        {
            greenBlocker = greenBlockers.front();
        }
        currentGreenBlocker = greenBlocker ? greenBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentGreenBlocker = ObjectGuid::Empty;
        greenBlocker = nullptr;
    }

        std::vector<Player*> blueBlockers = GetBlueBlockers();
        if (!blueBlockers.empty())
        {
            auto it = std::find_if(blueBlockers.begin(), blueBlockers.end(), [](Player* p) 
            {
                return p && p->GetGUID() == currentBlueBlocker;
            });
            if (it != blueBlockers.end())
            {
                blueBlocker = *it;
            }
            else
            {
                blueBlocker = blueBlockers.front();
            }
            currentBlueBlocker = blueBlocker ? blueBlocker->GetGUID() : ObjectGuid::Empty;
        }
        else
        {
            currentBlueBlocker = ObjectGuid::Empty;
            blueBlocker = nullptr;
        }

    return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
}

std::vector<Unit*> RaidKarazhanHelpers::GetAllVoidZones()
{
    std::vector<Unit*> voidZones;
    const float radius = 30.0f;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || unit->GetEntry() != NPC_VOID_ZONE)
        {
            continue;
        }
        float dist = bot->GetExactDist2d(unit);
        if (dist < radius)
        {
            voidZones.push_back(unit);
        }
    }

    return voidZones;
}

bool RaidKarazhanHelpers::IsSafePosition(float x, float y, float z,
     const std::vector<Unit*>& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        float dist = std::sqrt(std::pow(x - hazard->GetPositionX(), 2) + std::pow(y - hazard->GetPositionY(), 2));
        if (dist < hazardRadius)
        {
            return false;
        }
    }

    return true;
}

std::vector<Unit*> RaidKarazhanHelpers::GetSpawnedInfernals() const
{
    std::vector<Unit*> infernals;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL)
        {
            infernals.push_back(unit);
        }
    }

    return infernals;
}

bool RaidKarazhanHelpers::IsStraightPathSafe(const Position& start, const Position& target, const std::vector<Unit*>& hazards, float hazardRadius, float stepSize)
{
    float sx = start.GetPositionX();
    float sy = start.GetPositionY();
    float sz = start.GetPositionZ();
    float tx = target.GetPositionX();
    float ty = target.GetPositionY();
    float tz = target.GetPositionZ();
    float totalDist = std::sqrt(std::pow(tx - sx, 2) + std::pow(ty - sy, 2));
    if (totalDist == 0.0f)
    {
        return true;
    }
    
    for (float checkDist = 0.0f; checkDist <= totalDist; checkDist += stepSize)
    {
        float t = checkDist / totalDist;
        float checkX = sx + (tx - sx) * t;
        float checkY = sy + (ty - sy) * t;
        float checkZ = sz + (tz - sz) * t;
        for (Unit* hazard : hazards)
        {
            float hazardDist = std::sqrt(std::pow(checkX - hazard->GetPositionX(), 2) + std::pow(checkY - hazard->GetPositionY(), 2));
            if (hazardDist < hazardRadius)
            {
                return false;
            }
        }
    }
    
    return true;
}
