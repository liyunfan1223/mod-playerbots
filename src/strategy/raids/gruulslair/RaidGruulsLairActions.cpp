#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "PlayerbotAI.h"
#include "SpellAuras.h"
#include "Unit.h"

bool HighKingMaulgarRemoveTankAssistAction::Execute(Event event)
{
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    Unit* maulgar  = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler  = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh    = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm      = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    bool maulgarEncounterActive = (maulgar  && maulgar->IsInCombat()) ||
                                  (kiggler  && kiggler->IsInCombat()) ||
                                  (krosh    && krosh->IsInCombat()) ||
                                  (olm      && olm->IsInCombat()) ||
                                  (blindeye && blindeye->IsInCombat());

    if (maulgarEncounterActive && botAI->HasStrategy("tank assist", BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-tank assist", BOT_STATE_COMBAT);
        {
        return true;
        }
    }
    return false;
}

bool HighKingMaulgarMaulgarTankAction::Execute(Event event)
{
    if (!IsMaulgarTank(botAI, bot))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->IsAlive())
    {
        return false;
    }

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

        if (!bot->IsWithinMeleeRange(maulgar))
        {
            return MoveTo(maulgar->GetMapId(), maulgar->GetPositionX(), maulgar->GetPositionY(), maulgar->GetPositionZ());
        }
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
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
        
        float orientation = atan2(maulgar->GetPositionY() - bot->GetPositionY(), maulgar->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return false;
    }

    else if (!bot->IsWithinMeleeRange(maulgar))
    {
        return MoveTo(maulgar->GetMapId(), maulgar->GetPositionX(), maulgar->GetPositionY(), maulgar->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarOlmTankAction::Execute(Event event)
{
    if (!IsOlmTank(botAI, bot))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    if (!olm || !olm->IsAlive())
    {
        return false;
    }

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

    if (bot->GetVictim() != olm || olm->IsNonMeleeSpellCast(false))
    {
        Attack(olm);
        
        if (!bot->IsWithinMeleeRange(olm))
        {
            return MoveTo(olm->GetMapId(), olm->GetPositionX(), olm->GetPositionY(), olm->GetPositionZ());
        }
    }

    if (olm->GetVictim() == bot && !olm->IsNonMeleeSpellCast(false))
    {
        const TankSpot& spot = GruulsLairTankSpots::Olm;
        const float maxDistance = 3.0f;
        float distanceToOlm = olm->GetExactDist2d(spot.x, spot.y);

        if (distanceToOlm > maxDistance)
        {
            float dX = spot.x - olm->GetPositionX();
            float dY = spot.y - olm->GetPositionY();
            float moveX = spot.x + (dX / distanceToOlm) * maxDistance;
            float moveY = spot.y + (dY / distanceToOlm) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }

        float orientation = atan2(olm->GetPositionY() - bot->GetPositionY(), olm->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return false;
    }

    else if (!bot->IsWithinMeleeRange(olm))
    {
        return MoveTo(olm->GetMapId(), olm->GetPositionX(), olm->GetPositionY(), olm->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarBlindeyeTankAction::Execute(Event event)
{
    if (!IsBlindeyeTank(botAI, bot))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye || !blindeye->IsAlive())
    {
        return false;
    }

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
        
        if (!bot->IsWithinMeleeRange(blindeye))
        {
            return MoveTo(blindeye->GetMapId(), blindeye->GetPositionX(), blindeye->GetPositionY(), blindeye->GetPositionZ());
        }
    }

    if (blindeye->GetVictim() == bot && !blindeye->IsNonMeleeSpellCast(false))
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
            return MoveTo(bot->GetMapId(), moveX, moveY, spot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }

        float orientation = atan2(blindeye->GetPositionY() - bot->GetPositionY(), blindeye->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
        return false;
    }

    else if (!bot->IsWithinMeleeRange(blindeye))
    {
        return MoveTo(blindeye->GetMapId(), blindeye->GetPositionX(), blindeye->GetPositionY(), blindeye->GetPositionZ());
    }

    return false;
}

bool HighKingMaulgarMageTankAction::Execute(Event event)
{
    if (!IsMageTank(botAI, bot))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh || !krosh->IsAlive())
    {
        return false;
    }

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

    if (krosh->HasAura(SPELL_AURA_SPELL_SHIELD) && botAI->CanCastSpell(SPELL_SPELLSTEAL, krosh, true))
    {
        botAI->CastSpell(SPELL_SPELLSTEAL, krosh);
    }

    if (!bot->HasAura(SPELL_AURA_SPELL_SHIELD))
    {
        botAI->CastSpell("fire ward", bot);
    }

    if (bot->GetVictim() != krosh)
    {
        Attack(krosh);
    }

    float const OPTIMAL_RANGED_DISTANCE = 27.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 2.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 2.0f;

    if (bot->IsWithinRange(krosh, MIN_RANGE) || !bot->IsWithinRange(krosh, MAX_RANGE) || 
        !IsPositionSafe(botAI, bot, bot->GetPosition()))
    {
        Position safePos = FindSafePosition(botAI, bot, krosh, OPTIMAL_RANGED_DISTANCE);
        return MoveTo(krosh->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
    }

    return false;
}

bool HighKingMaulgarMoonkinTankAction::Execute(Event event)
{
    if (!IsMoonkinTank(botAI, bot))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    if (!kiggler || !kiggler->IsAlive())
    {
        return false;
    }

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

    float const OPTIMAL_RANGED_DISTANCE = 29.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 1.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 1.0f;

    if (bot->IsWithinRange(kiggler, MIN_RANGE) || !bot->IsWithinRange(kiggler, MAX_RANGE) || 
        !IsPositionSafe(botAI, bot, bot->GetPosition()))
    {
        Position safePos = FindSafePosition(botAI, bot, kiggler, OPTIMAL_RANGED_DISTANCE);
        return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
    }

    return false;
}

bool HighKingMaulgarMeleeDPSAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (!botAI->IsMelee(bot) || 
        (IsMaulgarTank(botAI, bot) && maulgar && maulgar->IsAlive()) || 
        (IsOlmTank(botAI, bot) && olm && olm->IsAlive()) || 
        (IsBlindeyeTank(botAI, bot) && blindeye && blindeye->IsAlive()))
        {
            return false;
        }

    // Target priority 1: Blindeye
    if (blindeye && blindeye->IsAlive())
    {
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

        if (!bot->IsWithinMeleeRange(blindeye) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, blindeye, 5.0f);
            return MoveTo(blindeye->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 2: Olm
    if (olm && olm->IsAlive())
    {
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
        
        if (!bot->IsWithinMeleeRange(olm) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, olm, 5.0f);
            return MoveTo(olm->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 3: Kiggler
    if (kiggler && kiggler->IsAlive())
    {
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

        if (!bot->IsWithinMeleeRange(kiggler) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, kiggler, 5.0f);
            return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 4: Maulgar
    if (maulgar && maulgar->IsAlive())
    {
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
        
        if (!bot->IsWithinMeleeRange(maulgar) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, maulgar, 5.0f);
            return MoveTo(maulgar->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
    }

    return false;
}

bool HighKingMaulgarRangedDPSAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (!botAI->IsRanged(bot) || 
        (IsMageTank(botAI, bot) && krosh && krosh->IsAlive()) || 
        (IsMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive()) || 
        botAI->IsHeal(bot))
        {
            return false;
        }

    float const OPTIMAL_RANGED_DISTANCE = 25.0f;
    float const MIN_RANGE = OPTIMAL_RANGED_DISTANCE - 4.0f;
    float const MAX_RANGE = OPTIMAL_RANGED_DISTANCE + 4.0f;

    // Target priority 1: Blindeye
    if (blindeye && blindeye->IsAlive())
    {
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

        if (bot->IsWithinRange(blindeye, MIN_RANGE) || 
            !bot->IsWithinRange(blindeye, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, blindeye, OPTIMAL_RANGED_DISTANCE);
            return MoveTo(blindeye->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }

        return false;
    }

    // Target priority 2: Olm
    if (olm && olm->IsAlive())
    {
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
        
        if (bot->IsWithinRange(olm, MIN_RANGE) || 
            !bot->IsWithinRange(olm, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, olm, OPTIMAL_RANGED_DISTANCE);
            return MoveTo(olm->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    // Target priority 3: Krosh
    if (krosh && krosh->IsAlive())
    {
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
        
        if (bot->IsWithinRange(krosh, MIN_RANGE) || 
            !bot->IsWithinRange(krosh, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, krosh, OPTIMAL_RANGED_DISTANCE);
            return MoveTo(krosh->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    // Target priority 4: Kiggler
    if (kiggler && kiggler->IsAlive())
    {
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
        
        if (bot->IsWithinRange(kiggler, MIN_RANGE) || 
            !bot->IsWithinRange(kiggler, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, kiggler, OPTIMAL_RANGED_DISTANCE);
            return MoveTo(kiggler->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
        
        return false;
    }

    // Target priority 5: Maulgar
    if (maulgar && maulgar->IsAlive())
    {
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
        
        if (bot->IsWithinRange(maulgar, MIN_RANGE) || 
            !bot->IsWithinRange(maulgar, MAX_RANGE) || 
            !IsPositionSafe(botAI, bot, bot->GetPosition()))
        {
            Position safePos = FindSafePosition(botAI, bot, maulgar, OPTIMAL_RANGED_DISTANCE);
            return MoveTo(maulgar->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
    }

    return false;
}

bool HighKingMaulgarHealerAvoidanceAction::Execute(Event event)
{
    if (!botAI->IsHeal(bot))
    {
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    
    bool needToMove = !IsPositionSafe(botAI, bot, bot->GetPosition());
    if (kiggler && kiggler->IsAlive())
    {
        float distanceToKiggler = bot->GetDistance(kiggler);
        if (distanceToKiggler < 30.0f)
        {
            needToMove = true;
        }
    }
    
    if (needToMove)
    {
        float centerX = 0, centerY = 0, centerZ = 0;
        uint32 count = 0;
        
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot) 
            {
                continue;
            }
            centerX += member->GetPositionX();
            centerY += member->GetPositionY();
            centerZ += member->GetPositionZ();
            count++;
        }
        
        if (count == 0)
        {
            return false;
        }

        centerX /= count;
        centerY /= count;
        centerZ /= count;
        Player* centerPlayer = nullptr;
        float minDistToCenter = 9999.0f;
        
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
            {
                continue;
            }

            float dist = sqrt(pow(member->GetPositionX() - centerX, 2) +
                             pow(member->GetPositionY() - centerY, 2) +
                             pow(member->GetPositionZ() - centerZ, 2));
                             
            if (dist < minDistToCenter)
            {
                minDistToCenter = dist;
                centerPlayer = member;
            }
        }
        
        if (centerPlayer)
        {
            Position safePos = FindSafePosition(botAI, bot, centerPlayer, 30.0f);
            
            if (kiggler && kiggler->IsAlive())
            {
                float safeDistToKiggler = sqrt(pow(safePos.GetPositionX() - kiggler->GetPositionX(), 2) +
                                             pow(safePos.GetPositionY() - kiggler->GetPositionY(), 2));
                
                if (safeDistToKiggler < 31.0f)
                {
                    float dx = safePos.GetPositionX() - kiggler->GetPositionX();
                    float dy = safePos.GetPositionY() - kiggler->GetPositionY();
                    float distance = sqrt(dx*dx + dy*dy);

                    if (distance < 0.001f)
                    {
                        dx = 1.0f;
                        dy = 0.0f;
                        distance = 1.0f;
                    }
                    
                    dx /= distance;
                    dy /= distance;
                    float newX = kiggler->GetPositionX() + dx * 30.0f;
                    float newY = kiggler->GetPositionY() + dy * 30.0f;
                    safePos.m_positionX = newX;
                    safePos.m_positionY = newY;
                }
            }
            
            return MoveTo(bot->GetMapId(), safePos.m_positionX, safePos.m_positionY, safePos.m_positionZ);
        }
    }
    
    return false;
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

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
}

bool HighKingMaulgarHunterMisdirectionAction::Execute(Event event)
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
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER)
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

    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Player* moonkinTank = nullptr;
    Player* mageTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        if (IsMoonkinTank(botAI, member)) moonkinTank = member;
        if (IsMageTank(botAI, member)) mageTank = member;
    }

    switch (hunterIndex)
    {
        case 0:
        if (kiggler && kiggler->GetHealth() > 0.98f * kiggler->GetMaxHealth() && moonkinTank && moonkinTank->IsAlive())
        {
            if (botAI->CanCastSpell("misdirection", moonkinTank))
            {
                botAI->CastSpell("misdirection", moonkinTank);
            }

            if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
            {
                return false;
            }

            botAI->CastSpell("steady shot", kiggler);
            return true;
        }
        break;

        case 1:
            if (krosh && krosh->IsAlive() && krosh->GetHealth() > 0.98f * krosh->GetMaxHealth() && mageTank && mageTank->IsAlive())
            {
                if (botAI->CanCastSpell("misdirection", mageTank))
                    botAI->CastSpell("misdirection", mageTank);

                if (!bot->HasAura(SPELL_AURA_MISDIRECTION))
                    return false;

                botAI->CastSpell("steady shot", krosh);
                return true;
            }
            break;
            
        default:
            break;
    }

    return false;
}

bool GruulTheDragonkillerPositionBossAction::Execute(Event event)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    
    const TankSpot& tankSpot = GruulsLairTankSpots::Gruul;
    const float maxDistance = 5.0f;
    float distanceToTankSpot = gruul->GetExactDist2d(tankSpot.x, tankSpot.y);

    if (distanceToTankSpot > maxDistance)
    {
        float dX = tankSpot.x - gruul->GetPositionX();
        float dY = tankSpot.y - gruul->GetPositionY();
        float moveX = tankSpot.x + (dX / distanceToTankSpot) * maxDistance;
        float moveY = tankSpot.y + (dY / distanceToTankSpot) * maxDistance;

        float moveDistance = bot->GetExactDist2d(moveX, moveY);
        if (moveDistance < 0.5f)
        {
            return false;
        }

        return MoveTo(bot->GetMapId(), moveX, moveY, tankSpot.z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }
    
    float desiredOrientation = atan2(gruul->GetPositionY() - bot->GetPositionY(), gruul->GetPositionX() - bot->GetPositionX());
    float currentOrientation = bot->GetOrientation();
    float delta = desiredOrientation - currentOrientation;
    while (delta > M_PI)
        delta -= 2 * M_PI;
    while (delta < -M_PI)
        delta += 2 * M_PI;
    float orientationDifference = fabs(static_cast<double>(delta));

    const float orientationLeeway = 30.0f * M_PI / 180.0f;
    if (orientationDifference > orientationLeeway)
    {
        bot->SetFacingTo(desiredOrientation);
    }

    return false;   
}

bool GruulTheDragonkillerPositionBossAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsTank(bot) && botAI->HasAggro(gruul) && gruul->GetVictim() == bot;
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

    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    static const TankSpot& tankSpot = GruulsLairTankSpots::Gruul;

    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        float centerX = tankSpot.x;
        float centerY = tankSpot.y;
        float centerZ = bot->GetPositionZ();
        float minRadius = 25.0f;
        float maxRadius = 40.0f;

        uint32 count = 0;
        uint32 botIndex = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRanged(member))
            {
                continue;
            }
            if (member == bot)
            {
                botIndex = count;
            }
            count++;
        }

        if (count == 0)
        {
            return false;
        }

        float angle = 2 * M_PI * botIndex / count;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);
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
            return MoveTo(bot->GetMapId(), targetPosition.GetPositionX(), targetPosition.GetPositionY(), targetPosition.GetPositionZ());
        }

        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float gruulRangedRadius = 25.0f;
    float minSpreadDistance = 10.0f;
    float movementThreshold = 2.0f;
    Unit* closestMember = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member || !member->IsAlive() || member == bot || !botAI->IsRanged(member))
        {
            continue;
        }

        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
        {
            closestMember = member;
        }
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float distanceToGruul = bot->GetExactDist2d(tankSpot.x, tankSpot.y);
    
    if (distanceToGruul < gruulRangedRadius - 3.0f - movementThreshold)
    {
        return MoveTo(bot->GetMapId(), tankSpot.x, tankSpot.y, tankSpot.z);
    }

    return false;
}

bool GruulTheDragonkillerSpreadRangedAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsRanged(bot);
}

bool GruulTheDragonkillerShatterSpreadAction::Execute(Event event)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    float radius = 22.0f;
    Unit* closestMember = nullptr;

    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);

        if (!unit || bot->GetGUID() == member)
        {
            continue;
        }

        if (!closestMember || bot->GetExactDist2d(unit) < bot->GetExactDist2d(closestMember))
        {
            closestMember = unit;
        }

        return MoveAway(closestMember, 6.0f);
    }
    
    return false;
}

bool GruulTheDragonkillerShatterSpreadAction::isUseful()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && 
           (bot->HasAura(SPELL_AURA_GROUND_SLAM_1) || bot->HasAura(SPELL_AURA_GROUND_SLAM_2));
}
