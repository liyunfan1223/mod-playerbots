#include "RaidGruulsLairHelpers.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "Group.h"
#include "GroupReference.h"
#include "PlayerbotAI.h"
#include "Unit.h"

namespace GruulsLairTankSpots 
{
	const TankSpot Maulgar  = { 90.686f, 167.047f, -13.234f, 3.009f };
	const TankSpot Olm      = { 99.392f, 192.834f, -10.883f, 6.265f };
	const TankSpot Blindeye = { 100.728f, 206.389f, -10.514f, 6.218f };
	const TankSpot Gruul    = { 241.238f, 365.025f, -4.220f, 0.0f };
}

bool IsAnyOgreBossAlive(PlayerbotAI* botAI)
{
    Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();
    Unit* kiggler = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kiggler the crazed")->Get();
    Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
    Unit* olm = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "olm the summoner")->Get();
    Unit* blindeye = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "blindeye the seer")->Get();

    return (maulgar && maulgar->IsAlive()) ||
           (kiggler && kiggler->IsAlive()) ||
           (krosh && krosh->IsAlive()) ||
           (olm && olm->IsAlive()) ||
           (blindeye && blindeye->IsAlive());
}

bool IsMageTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Player* highestHpMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
        {
            continue;
        }
        if (member->getClass() == CLASS_MAGE)
        {
            uint32 hp = member->GetMaxHealth();
            if (!highestHpMage || hp > highestHp)
            {
                highestHpMage = member;
                highestHp = hp;
            }
        }
    }
    return highestHpMage == bot;
}

bool IsMoonkinTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Player* highestHpMoonkin = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
        {
            continue;
        }

        if (member->getClass() == CLASS_DRUID)
        {
            int tab = AiFactory::GetPlayerSpecTab(member);
            if (tab == DRUID_TAB_BALANCE)
            {
                uint32 hp = member->GetMaxHealth();
                if (!highestHpMoonkin || hp > highestHp)
                {
                    highestHpMoonkin = member;
                    highestHp = hp;
                }
            }
        }
    }
    return highestHpMoonkin == bot;
}

bool IsMaulgarTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            return member == bot;
        }
    }
    return false;
}

bool IsOlmTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 1)
            {
                return member == bot;
            }
            ++tankIndex;
        }
    }
    return false;
}

bool IsBlindeyeTank(PlayerbotAI* botAI, Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group || !botAI->IsTank(bot))
    {
        return false;
    }
    int tankIndex = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member) continue;
        if (botAI->IsTank(member))
        {
            if (tankIndex == 2)
            {
                return member == bot;
            }
            ++tankIndex;
        }
    }
    return false;
}

bool IsPositionSafe(PlayerbotAI* botAI, Unit* bot, Position pos)
{
    const float KROSH_SAFE_DISTANCE = 21.0f;
    const float MAULGAR_WHIRLWIND_DISTANCE = 10.0f;
    bool isSafe = true;

    Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
    if (krosh && krosh->IsAlive())
    {
        float dist = sqrt(pow(pos.GetPositionX() - krosh->GetPositionX(), 2) + 
                          pow(pos.GetPositionY() - krosh->GetPositionY(), 2));
        if (dist < KROSH_SAFE_DISTANCE)
        {
            isSafe = false;
        }
    }
    Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();
    if (maulgar && maulgar->IsAlive() && maulgar->HasAura(SPELL_WHIRLWIND))
    {
        float dist = sqrt(pow(pos.GetPositionX() - maulgar->GetPositionX(), 2) + 
                          pow(pos.GetPositionY() - maulgar->GetPositionY(), 2));

        if (dist < MAULGAR_WHIRLWIND_DISTANCE)
        {
            isSafe = false;
        }
    }
    
    return isSafe;
}

Position FindSafePosition(PlayerbotAI* botAI, Unit* bot, Unit* target, float optimalDistance)
{
    Position bestPos;
    bestPos.m_positionX = bot->GetPositionX();
    bestPos.m_positionY = bot->GetPositionY();
    bestPos.m_positionZ = bot->GetPositionZ();
    
    Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
    Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();

    bool dangerousKrosh = krosh && krosh->IsAlive();
    bool dangerousMaulgar = maulgar && maulgar->IsAlive() && maulgar->HasAura(SPELL_WHIRLWIND);
    
    if (!dangerousKrosh && !dangerousMaulgar)
    {
        return bestPos;
    }

    if (IsPositionSafe(botAI, bot, bestPos))
    {
        return bestPos;
    }

    const int NUM_POSITIONS = 32;
    float bestScore = 99999.0f;
    bool foundSafeSpot = false;
    
    for (int i = 0; i < NUM_POSITIONS; i++)
    {
        float angle = 2 * M_PI * i / NUM_POSITIONS;
        Position candidatePos;
        candidatePos.m_positionX = target->GetPositionX() + optimalDistance * cos(angle);
        candidatePos.m_positionY = target->GetPositionY() + optimalDistance * sin(angle);
        candidatePos.m_positionZ = target->GetPositionZ();
        
        if (IsPositionSafe(botAI, bot, candidatePos))
        {
            float movementDistance = sqrt(pow(candidatePos.GetPositionX() - bot->GetPositionX(), 2) + 
                                         pow(candidatePos.GetPositionY() - bot->GetPositionY(), 2));
            
            if (movementDistance < bestScore)
            {
                bestScore = movementDistance;
                bestPos = candidatePos;
                foundSafeSpot = true;
            }
        }
    }

    if (foundSafeSpot)
    {
        return bestPos;
    }

    float dx = 0, dy = 0;

    if (dangerousKrosh && bot->GetDistance(krosh) < 30.0f)
    {
        dx += bot->GetPositionX() - krosh->GetPositionX();
        dy += bot->GetPositionY() - krosh->GetPositionY();
    }
    
    if (dangerousMaulgar && bot->GetDistance(maulgar) < 50.0f)
    {
        dx += bot->GetPositionX() - maulgar->GetPositionX();
        dy += bot->GetPositionY() - maulgar->GetPositionY();
    }

    float dist = sqrt(dx*dx + dy*dy);
    if (dist > 0.1f)
    {
        float escapeDistance = 20.0f;
        bestPos.m_positionX = bot->GetPositionX() + (dx/dist) * escapeDistance;
        bestPos.m_positionY = bot->GetPositionY() + (dy/dist) * escapeDistance;
        bestPos.m_positionZ = bot->GetPositionZ();
    }

    return bestPos;
}
