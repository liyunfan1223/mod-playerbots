#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "CreatureAI.h"
#include "Playerbots.h"
#include "Unit.h"

using namespace GruulsLairHelpers;

bool HighKingMaulgarMaulgarTankAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Group* group = bot->GetGroup();

    ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != maulgar->GetGUID())
    {
        group->SetTargetIcon(squareIcon, bot->GetGUID(), maulgar->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "square" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != maulgar)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
    }

    if (bot->GetVictim() != maulgar)
    {
        Attack(maulgar);
    }

    if (maulgar->GetVictim() == bot)
    {
        const TankSpot& spot = GruulsLairTankSpots::Maulgar;
        const float maxDistance = 3.0f;
        float distanceToMaulgar = maulgar->GetExactDist2d(spot.x, spot.y);
        
        if (distanceToMaulgar > maxDistance)
        {
            float dX = spot.x - maulgar->GetPositionX();
            float dY = spot.y - maulgar->GetPositionY();
            float moveX = spot.x + (dX / distanceToMaulgar) * maxDistance;
            float moveY = spot.y + (dY / distanceToMaulgar) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        float orientation = atan2(maulgar->GetPositionY() - bot->GetPositionY(), 
                                  maulgar->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(maulgar))
    {
        return MoveTo(maulgar->GetMapId(), maulgar->GetPositionX(), 
                      maulgar->GetPositionY(), maulgar->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarMaulgarTankAction::isUseful()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Group* group = bot->GetGroup();

    return botAI->IsMainTank(bot) && maulgar && maulgar->IsAlive() && group;
}

bool HighKingMaulgarOlmTankAction::Execute(Event event)
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Group* group = bot->GetGroup();

    ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != olm->GetGUID())
    {
        group->SetTargetIcon(circleIcon, bot->GetGUID(), olm->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "circle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != olm)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
    }

    if (bot->GetVictim() != olm)
    {
        Attack(olm);
    }

    if (olm->GetVictim() == bot)
    {
        const TankSpot& spot = GruulsLairTankSpots::Olm;
        const float maxDistance = 3.0f;
        const float olmAtSpotLeeway = 30.0f;
        float distanceToOlm = olm->GetExactDist2d(spot.x, spot.y);

        if (distanceToOlm > olmAtSpotLeeway)
        {
            float dX = spot.x - olm->GetPositionX();
            float dY = spot.y - olm->GetPositionY();
            float moveX = spot.x + (dX / distanceToOlm) * maxDistance;
            float moveY = spot.y + (dY / distanceToOlm) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }
    else if (!bot->IsWithinMeleeRange(olm))
    {
        return MoveTo(olm->GetMapId(), olm->GetPositionX(), 
                      olm->GetPositionY(), olm->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarOlmTankAction::isUseful()
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");

    return botAI->IsAssistTankOfIndex(bot, 0) && olm && olm->IsAlive();
}

bool HighKingMaulgarBlindeyeTankAction::Execute(Event event)
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Group* group = bot->GetGroup();

    ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != blindeye->GetGUID())
    {
        group->SetTargetIcon(starIcon, bot->GetGUID(), blindeye->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "star" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != blindeye)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
    }

    if (bot->GetVictim() != blindeye)
    {
        Attack(blindeye);
    }

    if (blindeye->GetVictim() == bot)
    {
        const TankSpot& spot = GruulsLairTankSpots::Blindeye;
        const float maxDistance = 3.0f;
        float distanceToBlindeye = blindeye->GetExactDist2d(spot.x, spot.y);

        if (distanceToBlindeye > maxDistance)
        {
            float dX = spot.x - blindeye->GetPositionX();
            float dY = spot.y - blindeye->GetPositionY();
            float moveX = spot.x + (dX / distanceToBlindeye) * maxDistance;
            float moveY = spot.y + (dY / distanceToBlindeye) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        float orientation = atan2(blindeye->GetPositionY() - bot->GetPositionY(), 
                                  blindeye->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(blindeye))
    {
        return MoveTo(blindeye->GetMapId(), blindeye->GetPositionX(), 
                      blindeye->GetPositionY(), blindeye->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarBlindeyeTankAction::isUseful()
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsAssistTankOfIndex(bot, 1) && blindeye && blindeye->IsAlive();
}

bool HighKingMaulgarKroshMageTankAction::Execute(Event event)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Group* group = bot->GetGroup();

    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != krosh->GetGUID())
    {
        group->SetTargetIcon(triangleIcon, bot->GetGUID(), krosh->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "triangle" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != krosh)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(krosh);
    }

    if (krosh->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_SHIELD)) && botAI->CanCastSpell("spellsteal", krosh))
    {
        botAI->CastSpell("spellsteal", krosh);
    }

    if (!bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_SHIELD)) && botAI->CanCastSpell("fire ward", bot))
    {
        botAI->CastSpell("fire ward", bot);
    }

    if (bot->GetVictim() != krosh)
    {
        Attack(krosh);
    }

    if (krosh->GetVictim() == bot)
    {
        const TankSpot& spot = GruulsLairTankSpots::Krosh;
        float distanceToKrosh = krosh->GetExactDist2d(spot.x, spot.y);
        const float minDistance = 16.0f;
        const float maxDistance = 29.0f;
        const float spotLeeway = 1.0f;

        if (distanceToKrosh > minDistance && distanceToKrosh < maxDistance)
        {
            if (!bot->IsWithinDist2d(spot.x, spot.y, spotLeeway))
            {
                return MoveTo(bot->GetMapId(), spot.x, spot.y, spot.z, false, false, false, false, 
                            MovementPriority::MOVEMENT_COMBAT, true, false);
            }

            float orientation = atan2(krosh->GetPositionY() - bot->GetPositionY(), 
                                      krosh->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
        else
        {
            Position safePos;
            if (FindSafePosition(botAI, bot, safePos))
            {
                return MoveTo(krosh->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
            }
        }
    }

    return false;
}

bool HighKingMaulgarKroshMageTankAction::isUseful()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Group* group = bot->GetGroup();

    return IsKroshMageTank(botAI, bot) && krosh && krosh->IsAlive() && group;
}

bool HighKingMaulgarKigglerMoonkinTankAction::Execute(Event event)
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Group* group = bot->GetGroup();

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != kiggler->GetGUID())
    {
        group->SetTargetIcon(diamondIcon, bot->GetGUID(), kiggler->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" && 
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != kiggler)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
    }

    if (bot->GetVictim() != kiggler)
    {
        Attack(kiggler);
    }

    Position safePos;
    if (FindSafePosition(botAI, bot, safePos))
    {
        return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
    }

    return false;
}

bool HighKingMaulgarKigglerMoonkinTankAction::isUseful()
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Group* group = bot->GetGroup();

    return IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive() && group;
}

bool HighKingMaulgarMeleeDPSAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    // Melee target priority 1: Blindeye
    if (blindeye && blindeye->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(blindeye->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "star" || rtiTarget != blindeye)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
        }

        if (bot->GetVictim() != blindeye)
        {
            Attack(blindeye);
        }

        return false;
    }

    // Melee target priority 2: Olm
    if (olm && olm->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(olm->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "circle" || rtiTarget != olm)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
        }

        if (bot->GetVictim() != olm)
        {
            Attack(olm);
        }

        return false;
    }

    // Melee target priority 3: Kiggler
    if (kiggler && kiggler->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "diamond" || rtiTarget != kiggler)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        }

        if (bot->GetVictim() != kiggler)
        {
            Attack(kiggler);
        }

        return false;
    }

    // Melee target priority 4: Maulgar
    if (maulgar && maulgar->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(maulgar->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "square" || rtiTarget != maulgar)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        }

        if (bot->GetVictim() != maulgar)
        {
            Attack(maulgar);
        }
    }

    return false;
}

bool HighKingMaulgarMeleeDPSAction::isUseful()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsMelee(bot) &&
           !(botAI->IsMainTank(bot) && maulgar && maulgar->IsAlive()) &&
           !(botAI->IsAssistTankOfIndex(bot, 0) && olm && olm->IsAlive()) &&
           !(botAI->IsAssistTankOfIndex(bot, 1) && blindeye && blindeye->IsAlive());
}

bool HighKingMaulgarRangedDPSAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    // Ranged target priority 1: Blindeye
    if (blindeye && blindeye->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(blindeye->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "star" || rtiTarget != blindeye)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(blindeye);
        }

        if (bot->GetVictim() != blindeye)
        {
            Attack(blindeye);
        }

        return false;
    }

    // Ranged target priority 2: Olm
    if (olm && olm->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(olm->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "circle" || rtiTarget != olm)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(olm);
        }

        if (bot->GetVictim() != olm)
        {
            Attack(olm);
        }

        return false;
    }

    // Ranged target priority 3: Krosh
    if (krosh && krosh->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(krosh->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "triangle" || rtiTarget != krosh)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(krosh);
        }

        if (bot->GetVictim() != krosh)
        {
            Attack(krosh);
        }

        return false;
    }

    // Ranged target priority 4: Kiggler
    if (kiggler && kiggler->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "diamond" || rtiTarget != kiggler)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(kiggler);
        }

        if (bot->GetVictim() != kiggler)
        {
            Attack(kiggler);
        }

        return false;
    }

    // Ranged target priority 5: Maulgar
    if (maulgar && maulgar->IsAlive())
    {
        Position safePos;
        if (FindSafePosition(botAI, bot, safePos))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(maulgar->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        if (rtiValue != "square" || rtiTarget != maulgar)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(maulgar);
        }

        if (bot->GetVictim() != maulgar)
        {
            Attack(maulgar);
        }
    }

    return false;
}

bool HighKingMaulgarRangedDPSAction::isUseful()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsRanged(bot) && !botAI->IsHeal(bot) &&
           !(IsKroshMageTank(botAI, bot) && krosh && krosh->IsAlive()) &&
           !(IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive());
}

bool HighKingMaulgarHealerAvoidanceAction::Execute(Event event)
{
    const TankSpot& fightCenter = GruulsLairTankSpots::MaulgarRoomCenter;
    const float maxDistanceFromFight = 50.0f;
    float distToFight = bot->GetExactDist2d(fightCenter.x, fightCenter.y);

    if (distToFight > maxDistanceFromFight)
    {
        float angle = atan2(bot->GetPositionY() - fightCenter.y, bot->GetPositionX() - fightCenter.x);
        float destX = fightCenter.x + 40.0f * cos(angle);
        float destY = fightCenter.y + 40.0f * sin(angle);
        float destZ = fightCenter.z;

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
            bot->GetPositionZ(), destX, destY, destZ))
        {
            return false;
        }

        return MoveTo(bot->GetMapId(), destX, destY, destZ);
    }

    Position safePos;
    if (FindSafePosition(botAI, bot, safePos))
    {
        return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
    }

    return false;
}

bool HighKingMaulgarHealerAvoidanceAction::isUseful()
{
    return botAI->IsHeal(bot) && IsAnyOgreBossAlive(botAI);
}

bool HighKingMaulgarWhirlwindRunAwayAction::Execute(Event event)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    const float safeDistance = 10.0f;
    float distance = bot->GetExactDist2d(maulgar);

    if (distance < safeDistance)
    {
        float angle = atan2(bot->GetPositionY() - maulgar->GetPositionY(), 
                            bot->GetPositionX() - maulgar->GetPositionX());
        float destX = maulgar->GetPositionX() + safeDistance * cos(angle);
        float destY = maulgar->GetPositionY() + safeDistance * sin(angle);
        float destZ = bot->GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
            bot->GetPositionZ(), destX, destY, destZ))
        {
            return false;
        }

        float destDist = maulgar->GetExactDist2d(destX, destY);

        if (destDist >= safeDistance - 0.1f)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveTo(maulgar->GetMapId(), destX, destY, destZ);
        }
    }

    return false;
}

bool HighKingMaulgarWhirlwindRunAwayAction::isUseful()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    return maulgar && maulgar->IsAlive() && maulgar->HasAura(static_cast<uint32>(GruulsLairSpells::WHIRLWIND)) && !botAI->IsMainTank(bot);
}

bool HighKingMaulgarBanishFelstalkerAction::Execute(Event event)
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");
    if (botAI->CanCastSpell("banish", felStalker))
    {
        return botAI->CastSpell("banish", felStalker);
    }

    return false;
}

bool HighKingMaulgarBanishFelstalkerAction::isUseful()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && !felStalker->HasAura(static_cast<uint32>(GruulsLairSpells::BANISH)) && 
           bot->getClass() == CLASS_WARLOCK;
}

bool HighKingMaulgarHunterMisdirectionAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        else if (botAI->IsAssistTankOfIndex(bot, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(bot, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
        case 0:
            botAI->CastSpell("misdirection", olmTank);
            if (bot->HasAura(static_cast<uint32>(GruulsLairSpells::MISDIRECTION)))
            {
                Pet* pet = bot->GetPet();
                if (pet && pet->IsAlive() && pet->GetVictim() != blindeye)
                {
                    pet->ClearUnitState(UNIT_STATE_FOLLOW);
                    pet->AttackStop();
                    pet->SetTarget(blindeye->GetGUID());
                    if (pet->GetCharmInfo())
                    {
                        pet->GetCharmInfo()->SetIsCommandAttack(true);
                        pet->GetCharmInfo()->SetIsAtStay(false);
                        pet->GetCharmInfo()->SetIsFollowing(false);
                        pet->GetCharmInfo()->SetIsCommandFollow(false);
                        pet->GetCharmInfo()->SetIsReturning(false);
                    }
                    pet->ToCreature()->AI()->AttackStart(blindeye);
                }
                botAI->CastSpell("steady shot", olm);
            }
            break;

        case 1:
            botAI->CastSpell("misdirection", blindeyeTank);
            if (bot->HasAura(static_cast<uint32>(GruulsLairSpells::MISDIRECTION)))
            {
                botAI->CastSpell("steady shot", blindeye);
            }
            break;
            
        default:
            break;
    }

    return false;
}

bool HighKingMaulgarHunterMisdirectionAction::isUseful()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
    {
        return false;
    }

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1 || hunterIndex > 1)
    {
        return false;
    }

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        else if (botAI->IsAssistTankOfIndex(bot, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(bot, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
        case 0:
        return olm && olm->IsAlive() && olm->GetHealth() > 0.98f * olm->GetMaxHealth() && 
               olmTank && olmTank->IsAlive() && botAI->CanCastSpell("misdirection", olmTank);

        case 1:
        return blindeye && blindeye->IsAlive() && blindeye->GetHealth() > 0.90f * blindeye->GetMaxHealth() && 
               blindeyeTank && blindeyeTank->IsAlive() && botAI->CanCastSpell("misdirection", blindeyeTank);

        default:
            break;
    }

    return false;
}

bool GruulTheDragonkillerMainTankAction::Execute(Event event)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    
    if (bot->GetVictim() != gruul)
    {
        Attack(gruul);
    }
    
    if (gruul->GetVictim() == bot)
    {
        const TankSpot& tankSpot = GruulsLairTankSpots::Gruul;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = tankSpot.x - bot->GetPositionX();
        float dY = tankSpot.y - bot->GetPositionY();
        float distanceToTankSpot = bot->GetExactDist2d(tankSpot.x, tankSpot.y);

        if (distanceToTankSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToTankSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToTankSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToTankSpot) * step;
            const float moveZ = tankSpot.z;
            {
                return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }

        float orientation = atan2(gruul->GetPositionY() - bot->GetPositionY(), 
                                  gruul->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(gruul))
    {
        return MoveTo(gruul->GetMapId(), gruul->GetPositionX(), 
                      gruul->GetPositionY(), gruul->GetPositionZ());
    }

    return false;
}

bool GruulTheDragonkillerMainTankAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsMainTank(bot);
}

bool GruulTheDragonkillerSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (gruul && gruul->IsAlive() && gruul->GetHealth() == gruul->GetMaxHealth())
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const TankSpot& tankSpot = GruulsLairTankSpots::Gruul;
    const float centerX = tankSpot.x;
    const float centerY = tankSpot.y;
    float centerZ = bot->GetPositionZ();
    const float minRadius = 25.0f;
    const float maxRadius = 40.0f;

    std::vector<Player*> members;
    Player* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();
    Group* group = bot->GetGroup();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        members.push_back(member);
        if (member != bot)
        {
            float dist = bot->GetExactDist2d(member);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestMember = member;
            }
        }
    }

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint32 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint32 count = members.size();

        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) / 
                       static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, centerZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            float destX = targetPosition.GetPositionX();
            float destY = targetPosition.GetPositionY();
            float destZ = targetPosition.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
            {
                return false;
            }

            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }

        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float gruulRangedRadius = 25.0f;
    const float minSpreadDistance = 10.0f;
    const float movementThreshold = 2.0f;

    if (closestMember && closestDist < minSpreadDistance - movementThreshold)
    {
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(), 
                            closestMember->GetPositionZ()), minSpreadDistance, 0);
    }

    float distanceToGruul = bot->GetExactDist2d(tankSpot.x, tankSpot.y);
    if (distanceToGruul < minRadius - movementThreshold)
    {
        float angle = atan2(bot->GetPositionY() - tankSpot.y, bot->GetPositionX() - tankSpot.x);
        float radius = minRadius + static_cast<float>(rand()) / 
                       static_cast<float>(RAND_MAX) * (maxRadius - minRadius);
        float destX = tankSpot.x + radius * cos(angle);
        float destY = tankSpot.y + radius * sin(angle);
        float destZ = bot->GetPositionZ();

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
            bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
        {
            return false;
        }

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return MoveTo(bot->GetMapId(), destX, destY, destZ);
    }

    return false;
}

bool GruulTheDragonkillerSpreadRangedAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    Group* group = bot->GetGroup();

    return gruul && gruul->IsAlive() && botAI->IsRanged(bot) && group;
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event event)
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    Unit* closestMember = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (!unit || bot->GetGUID() == member)
        {
            continue;
        }
        const float dist = bot->GetExactDist2d(unit);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestMember = unit;
        }
    }

    if (closestMember)
    {
        return FleePosition(Position(closestMember->GetPositionX(), closestMember->GetPositionY(), 
                            closestMember->GetPositionZ()), 6.0f, 0);
    }

    return false;
}

bool GruulTheDragonkillerShatterSpreadAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    Group* group = bot->GetGroup();

    return gruul && gruul->IsAlive() && group &&
           (bot->HasAura(static_cast<uint32>(GruulsLairSpells::GROUND_SLAM_1)) || 
            bot->HasAura(static_cast<uint32>(GruulsLairSpells::GROUND_SLAM_2)));
}
