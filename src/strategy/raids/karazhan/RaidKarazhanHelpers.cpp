#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "PlayerbotMgr.h"
#include "AiObjectContext.h"
#include "Position.h"
#include "Spell.h"

#include <algorithm>
#include <map>

void RaidKarazhanHelpers::MarkTargetWithSkull(Unit* target)
{
    if (!target)
        return;

    if (Group* group = bot->GetGroup())
    {
        constexpr uint8_t skullIconId = 7;
        ObjectGuid skullGuid = group->GetTargetIcon(skullIconId);

        if (skullGuid != target->GetGUID())
            group->SetTargetIcon(skullIconId, bot->GetGUID(), target->GetGUID());
    }
}

Unit* RaidKarazhanHelpers::GetFirstAliveUnit(const std::vector<Unit*>& units)
{
    for (Unit* unit : units)
        if (unit && unit->IsAlive())
            return unit;

    return nullptr;
}

Unit* RaidKarazhanHelpers::GetFirstAliveUnitByEntry(uint32 entry)
{
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);

        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            return unit;
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
                continue;

            if (bot->GetExactDist2d(member) < radius)
                return member;
        }
    }
    return nullptr;
}

bool RaidKarazhanHelpers::IsFlameWreathActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!boss)
        return false;

    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (currentSpell && currentSpell->m_spellInfo && currentSpell->m_spellInfo->Id == SPELL_FLAME_WREATH_CAST)
    {
        bot->Yell("I will not move when Flame Wreath is cast or the raid blows up.", LANG_UNIVERSAL);
        return true;
    }

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;
            if (member->HasAura(SPELL_FLAME_WREATH_AURA))
                return true;
        }
    }
    return false;
}

// Blue beam blockers: non-Rogue/Warrior DPS, no Nether Exhaustion Blue and <25 stacks of Blue Beam debuff
std::vector<Player*> RaidKarazhanHelpers::GetBlueBlockers()
{
    std::vector<Player*> blueBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            bool isDps = botAI->IsDps(member);
            bool isWarrior = member->getClass() == CLASS_WARRIOR;
            bool isRogue = member->getClass() == CLASS_ROGUE;
            bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_BLUE);
            Aura* blueBuff = member->GetAura(SPELL_BLUE_BEAM_DEBUFF);
            bool overStack = blueBuff && blueBuff->GetStackAmount() >= 25;

            if (isDps && !isWarrior && !isRogue && !hasExhaustion && !overStack)
            {
                blueBlockers.push_back(member);
            }
        }
    }
    return blueBlockers;
}

// Green beam blockers:
// (1) Rogues and non-tank Warriors, no Nether Exhaustion Green
// (2) Healers, no Nether Exhaustion Green and <25 stacks of Green Beam debuff
std::vector<Player*> RaidKarazhanHelpers::GetGreenBlockers()
{
    std::vector<Player*> greenBlockers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN);
            Aura* greenBuff = member->GetAura(SPELL_GREEN_BEAM_DEBUFF);
            bool overStack = greenBuff && greenBuff->GetStackAmount() >= 25;
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
        return Position(bx, by, bz);

    dx /= length;
    dy /= length;

    float targetX = bx + dx * distanceFromBoss;
    float targetY = by + dy * distanceFromBoss;
    float targetZ = bz;

    return Position(targetX, targetY, targetZ);
}

std::tuple<Player*, Player*, Player*> RaidKarazhanHelpers::GetCurrentBeamBlockers()
{
    Player* redBlocker = nullptr;
    Player* greenBlocker = nullptr;
    Player* blueBlocker = nullptr;
    std::vector<Player*> redBlockers;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;
            PlayerbotAI* memberAI = sPlayerbotsMgr->GetPlayerbotAI(member);
            if (!memberAI || !memberAI->IsTank(member))
                continue;
            if (member->HasAura(SPELL_NETHER_EXHAUSTION_RED))
                continue;
            redBlockers.push_back(member);
        }
    }

    if (!redBlockers.empty())
        redBlocker = redBlockers.front();

    std::vector<Player*> greenBlockers = GetGreenBlockers();
    if (!greenBlockers.empty())
        greenBlocker = greenBlockers.front();

    std::vector<Player*> blueBlockers = GetBlueBlockers();
    if (!blueBlockers.empty())
        blueBlocker = blueBlockers.front();

    return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
}

std::vector<Unit*> RaidKarazhanHelpers::GetAllVoidZones()
{
    std::vector<Unit*> voidZones;
    const float radius = 15.0f;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (!unit || unit->GetEntry() != NPC_VOID_ZONE)
            continue;

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
            return false;
    }
    return true;
}

std::vector<Unit*> RaidKarazhanHelpers::GetSpawnedInfernals() const
{
    std::vector<Unit*> infernals;
    const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();

    for (const auto& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);

        if (unit && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL)
            infernals.push_back(unit);
    }
    return infernals;
}
