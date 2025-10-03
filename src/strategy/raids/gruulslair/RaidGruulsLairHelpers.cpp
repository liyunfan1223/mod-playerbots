#include "RaidGruulsLairHelpers.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "Group.h"
#include "GroupReference.h"
#include "PlayerbotAI.h"
#include "Unit.h"

namespace GruulsLairTankSpots 
{
	const TankSpot Maulgar           = { 90.686f, 167.047f, -13.234f };
    // Olm does not chase properly due to the Core's caster movement issues
    // Thus, the below "TankSpot" is beyond the actual desired tanking location
    // It is the spot to which the OlmTank runs to to pull Olm to a decent tanking location
    const TankSpot Olm               = { 87.485f, 234.942f, -3.635f };
    const TankSpot Blindeye          = { 99.681f, 213.989f, -10.345f };
    const TankSpot Krosh             = { 116.880f, 166.208f, -14.231f };
    // "MaulgarRoomCenter" is to keep healers from getting too far away
    const TankSpot MaulgarRoomCenter = { 88.754f, 150.759f, -11.569f };
	const TankSpot Gruul             = { 241.238f, 365.025f, -4.220f };
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

bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot)
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
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
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

bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot)
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
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
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
        if (!member) 
        {
            continue;
        }
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
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
        {
            continue;
        }
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
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
        {
            continue;
        }
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

bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos)
{
    const float KROSH_SAFE_DISTANCE = 20.0f;
    const float MAULGAR_SAFE_DISTANCE = 10.0f;
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

    if (botAI->IsRanged(bot))
    {
        Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();
        if (maulgar && maulgar->IsAlive())
        {
            float dist = sqrt(pow(pos.GetPositionX() - maulgar->GetPositionX(), 2) + 
                              pow(pos.GetPositionY() - maulgar->GetPositionY(), 2));
            if (dist < MAULGAR_SAFE_DISTANCE)
            {
                isSafe = false;
            }
        }
    }

    return isSafe;
}

bool FindSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos)
{
    const float SEARCH_RADIUS = 30.0f;
    const int NUM_POSITIONS = 32;
    outPos = { bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() };

    if (IsPositionSafe(botAI, bot, outPos))
    {
        return false;
    }

    float bestScore = std::numeric_limits<float>::max();
    bool foundSafeSpot = false;

    for (int i = 0; i < NUM_POSITIONS; ++i)
    {
        float angle = 2 * M_PI * i / NUM_POSITIONS;
        Position candidatePos;
        candidatePos.m_positionX = bot->GetPositionX() + SEARCH_RADIUS * cos(angle);
        candidatePos.m_positionY = bot->GetPositionY() + SEARCH_RADIUS * sin(angle);
        candidatePos.m_positionZ = bot->GetPositionZ();

        float destX = candidatePos.m_positionX, destY = candidatePos.m_positionY, destZ = candidatePos.m_positionZ;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), destX, destY, destZ, true))
        {
            continue;
        }
        if (destX != candidatePos.m_positionX || destY != candidatePos.m_positionY)
        {
            continue;
        }

        candidatePos.m_positionX = destX;
        candidatePos.m_positionY = destY;
        candidatePos.m_positionZ = destZ;

        if (IsPositionSafe(botAI, bot, candidatePos))
        {
            float movementDistance = sqrt(pow(destX - bot->GetPositionX(), 2) + pow(destY - bot->GetPositionY(), 2));
            if (movementDistance < bestScore)
            {
                bestScore = movementDistance;
                outPos = candidatePos;
                foundSafeSpot = true;
            }
        }
    }

    return foundSafeSpot;
}
