#include <ctime>
#include <unordered_map>

#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;
static std::unordered_map<uint32, time_t> magtheridonBlastNovaTimer;

bool MagtheridonHellfireChannelerMainTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    if (channelerSquare && channelerSquare->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(squareIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerSquare->GetGUID())
            group->SetTargetIcon(squareIcon, bot->GetGUID(), channelerSquare->GetGUID());
    }
    if (channelerStar && channelerStar->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(starIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerStar->GetGUID())
            group->SetTargetIcon(starIcon, bot->GetGUID(), channelerStar->GetGUID());
    }
    if (channelerCircle && channelerCircle->IsAlive())
    {
        ObjectGuid currentIconGuid = group->GetTargetIcon(circleIcon);
        if (currentIconGuid.IsEmpty() || currentIconGuid != channelerCircle->GetGUID())
            group->SetTargetIcon(circleIcon, bot->GetGUID(), channelerCircle->GetGUID());
    }

    if ((!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar   || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()))
    {
        const TankSpot& spot = MagtheridonTankSpots::WaitingForMagtheridon;
        if (!bot->IsWithinDist2d(spot.x, spot.y, 2.0f))
        {
            return MoveTo(bot->GetMapId(), spot.x, spot.y, spot.z, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        bot->SetFacingTo(spot.orientation);
        return true;
    }

    Creature* currentTarget = nullptr;
    std::string rtiName;
    if (channelerSquare && channelerSquare->IsAlive())
    {
        currentTarget = channelerSquare;
        rtiName = "square";
    }
    else if (channelerStar && channelerStar->IsAlive())
    {
        currentTarget = channelerStar;
        rtiName = "star";
    }
    else if (channelerCircle && channelerCircle->IsAlive())
    {
        currentTarget = channelerCircle;
        rtiName = "circle";
    }

    if (currentTarget &&
        (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != rtiName ||
         botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != currentTarget))
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(currentTarget);
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        Attack(currentTarget);

        if (!bot->IsWithinMeleeRange(currentTarget))
        {
            return MoveTo(currentTarget->GetMapId(), currentTarget->GetPositionX(), 
                          currentTarget->GetPositionY(), currentTarget->GetPositionZ());
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerMainTankAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return group && botAI->IsMainTank(bot) && magtheridon && 
           magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE));
}

bool MagtheridonHellfireChannelerNWChannelerTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    ObjectGuid currentIconGuid = group->GetTargetIcon(diamondIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerDiamond->GetGUID())
    {
        group->SetTargetIcon(diamondIcon, bot->GetGUID(), channelerDiamond->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "diamond" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerDiamond)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
    }

    if (bot->GetVictim() != channelerDiamond)
    {
        Attack(channelerDiamond);
        
        if (!bot->IsWithinMeleeRange(channelerDiamond))
        {
            return MoveTo(channelerDiamond->GetMapId(), channelerDiamond->GetPositionX(),
                          channelerDiamond->GetPositionY(), channelerDiamond->GetPositionZ());
        }
    }

    if (channelerDiamond->GetVictim() == bot)
    {
        const TankSpot& spot = MagtheridonTankSpots::NWChanneler;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = spot.x - bot->GetPositionX();
        float dY = spot.y - bot->GetPositionY();
        float distanceToSpot = bot->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToSpot) * step;
            const float moveZ = spot.z;
            {
                return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerNWChannelerTankAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    return group && botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond && channelerDiamond->IsAlive();
}

bool MagtheridonHellfireChannelerNEChannelerTankAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    ObjectGuid currentIconGuid = group->GetTargetIcon(triangleIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != channelerTriangle->GetGUID())
    {
        group->SetTargetIcon(triangleIcon, bot->GetGUID(), channelerTriangle->GetGUID());
    }

    if (botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get() != "triangle" ||
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get() != channelerTriangle)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerTriangle);
    }

    if (bot->GetVictim() != channelerTriangle)
    {
        Attack(channelerTriangle);

        if (!bot->IsWithinMeleeRange(channelerTriangle))
        {
            return MoveTo(channelerTriangle->GetMapId(), channelerTriangle->GetPositionX(),
                          channelerTriangle->GetPositionY(), channelerTriangle->GetPositionZ());
        }
    }

    if (channelerTriangle->GetVictim())
    {
        const TankSpot& spot = MagtheridonTankSpots::NEChanneler;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = spot.x - bot->GetPositionX();
        float dY = spot.y - bot->GetPositionY();
        float distanceToSpot = bot->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToSpot) * step;
            const float moveZ = spot.z;
            {
                return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerNEChannelerTankAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return group && botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerMisdirectionAction::Execute(Event event)
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
    
    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    bool actionTaken = false;
    switch (hunterIndex)
    {
        case 0:
            if (mainTank && channelerStar && channelerStar->IsAlive() && channelerStar->GetVictim() != mainTank)
            {
                if (botAI->CanCastSpell("misdirection", mainTank))
                    botAI->CastSpell("misdirection", mainTank);

                if (!bot->HasAura(static_cast<uint32>(MagtheridonSpells::MISDIRECTION)))
                    return actionTaken;

                if (botAI->CanCastSpell("steady shot", channelerStar))
                {
                    botAI->CastSpell("steady shot", channelerStar);
                    actionTaken = true;
                }
            }
            break;

        case 1:
            if (mainTank && channelerCircle && channelerCircle->IsAlive() && channelerCircle->GetVictim() != mainTank)
            {
                if (botAI->CanCastSpell("misdirection", mainTank))
                    botAI->CastSpell("misdirection", mainTank);

                if (!bot->HasAura(static_cast<uint32>(MagtheridonSpells::MISDIRECTION)))
                    return actionTaken;

                if (botAI->CanCastSpell("steady shot", channelerCircle))
                {
                    botAI->CastSpell("steady shot", channelerCircle);
                    actionTaken = true;
                }
            }
            break;

        default:
            break;
    }

    return actionTaken;
}

bool MagtheridonHellfireChannelerMisdirectionAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return group && bot->getClass() == CLASS_HUNTER && 
           (channelerStar && channelerStar->IsAlive() || channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonHellfireChannelerDPSPriorityAction::Execute(Event event)
{
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (channelerSquare && channelerSquare->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        if (rtiValue != "square" || rtiTarget != channelerSquare)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("square");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerSquare);
        }
        if (bot->GetVictim() != channelerSquare)
        {
            Attack(channelerSquare);
        }

        return false;
    }

    if (channelerStar && channelerStar->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        
        if (rtiValue != "star" || rtiTarget != channelerStar)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("star");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerStar);
        }
        if (bot->GetVictim() != channelerStar)
        {
            Attack(channelerStar);
        }

        return false;
    }

    if (channelerCircle && channelerCircle->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "circle" || rtiTarget != channelerCircle)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("circle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerCircle);
        }
        if (bot->GetVictim() != channelerCircle)
        {
            Attack(channelerCircle);
        }

        return false;
    }

    if (channelerDiamond && channelerDiamond->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "diamond" || rtiTarget != channelerDiamond)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("diamond");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerDiamond);
        }
        if (bot->GetVictim() != channelerDiamond)
        {
            Attack(channelerDiamond);
        }

        return false;
    }

    if (channelerTriangle && channelerTriangle->IsAlive())
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "triangle" || rtiTarget != channelerTriangle)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("triangle");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(channelerTriangle);
        }
        if (bot->GetVictim() != channelerTriangle)
        {
            Attack(channelerTriangle);
        }

        return false;
    }

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->IsAlive() && 
        !magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)) &&
        (!channelerSquare || !channelerSquare->IsAlive()) &&
        (!channelerStar || !channelerStar->IsAlive()) &&
        (!channelerCircle || !channelerCircle->IsAlive()) &&
        (!channelerDiamond || !channelerDiamond->IsAlive()) &&
        (!channelerTriangle || !channelerTriangle->IsAlive()))
    {
        Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
        std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();

        if (rtiValue != "cross" || rtiTarget != magtheridon)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(magtheridon);
        }
        if (bot->GetVictim() != magtheridon)
        {
            Attack(magtheridon);
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerDPSPriorityAction::isUseful()
{
    Group* group = bot->GetGroup();
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    if (!group || botAI->IsHeal(bot) || botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond->IsAlive() ||
        botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle->IsAlive())
    {
        return false;
    }

    return channeler && channeler->IsAlive() || magtheridon && magtheridon->IsAlive() && 
           !magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE));
}

bool MagtheridonHellfireChannelerWarlockAction::Execute(Event event)
{
    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    bool isAlreadyBanishing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::BURNING_ABYSSAL) && 
            unit->HasAura(static_cast<uint32>(MagtheridonSpells::BANISH)))
        {
            isAlreadyBanishing = true;
            break;
        }
    }

    bool isAlreadyFearing = false;
    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::BURNING_ABYSSAL) && 
            unit->HasAura(static_cast<uint32>(MagtheridonSpells::FEAR)))
        {
            isAlreadyFearing = true;
            break;
        }
    }

    if (!isAlreadyBanishing)
    {
        for (const auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::BURNING_ABYSSAL) && 
                !unit->HasAura(static_cast<uint32>(MagtheridonSpells::BANISH)))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(static_cast<uint32>(MagtheridonSpells::BANISH), unit, true))
                {
                    botAI->CastSpell(static_cast<uint32>(MagtheridonSpells::BANISH), unit);
                    return false;
                }
            }
        }
    }

    if (!isAlreadyFearing)
    {
        for (const auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::BURNING_ABYSSAL) &&
                !unit->HasAura(static_cast<uint32>(MagtheridonSpells::BANISH)) && 
                !unit->HasAura(static_cast<uint32>(MagtheridonSpells::FEAR)))
            {
                if (unit->IsAlive() && botAI->CanCastSpell(static_cast<uint32>(MagtheridonSpells::FEAR), unit, true))
                {
                    botAI->CastSpell(static_cast<uint32>(MagtheridonSpells::FEAR), unit);
                    return false;
                }
            }
        }
    }

    for (const auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->IsAlive() && unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::HELLFIRE_CHANNELER))
        {
            if (!unit->HasAura(static_cast<uint32>(MagtheridonSpells::CURSE_OF_TONGUES)))
            {
                if (botAI->CanCastSpell(static_cast<uint32>(MagtheridonSpells::CURSE_OF_TONGUES), unit, true))
                {
                    botAI->CastSpell(static_cast<uint32>(MagtheridonSpells::CURSE_OF_TONGUES), unit);
                    return false;
                }
            }
        }
    }

    return false;
}

bool MagtheridonHellfireChannelerWarlockAction::isUseful()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_WARLOCK)
    {
        return false;
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    return std::any_of(npcs.begin(), npcs.end(), [&](const ObjectGuid& npc) 
    {
        Unit* unit = botAI->GetUnit(npc);
        return unit && (unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::BURNING_ABYSSAL) || 
                        unit->GetEntry() == static_cast<uint32>(MagtheridonNPCs::HELLFIRE_CHANNELER));
    });
}

bool MagtheridonPositionBossAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    Group* group = bot->GetGroup();
    ObjectGuid currentIconGuid = group->GetTargetIcon(crossIcon);
    if (currentIconGuid.IsEmpty() || currentIconGuid != magtheridon->GetGUID())
    {
        group->SetTargetIcon(crossIcon, bot->GetGUID(), magtheridon->GetGUID());
    }

    Unit* rtiTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();
    std::string rtiValue = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    if (rtiValue != "cross" || rtiTarget != magtheridon)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set("cross");
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(magtheridon);
    }

    if (bot->GetVictim() != magtheridon)
    {
        Attack(magtheridon);
    }

    if (magtheridon->GetVictim() == bot)
    {
        const TankSpot& spot = MagtheridonTankSpots::Magtheridon;
        const float maxStep = 5.0f;
        const float positionLeeway = 3.0f;

        float dX = spot.x - bot->GetPositionX();
        float dY = spot.y - bot->GetPositionY();
        float distanceToSpot = bot->GetExactDist2d(spot.x, spot.y);
        if (distanceToSpot > positionLeeway)
        {
            float step = std::min(maxStep, distanceToSpot);
            float moveX = bot->GetPositionX() + (dX / distanceToSpot) * step;
            float moveY = bot->GetPositionY() + (dY / distanceToSpot) * step;
            const float moveZ = spot.z;
            {
                return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false, 
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }

        float orientation = atan2(magtheridon->GetPositionY() - bot->GetPositionY(),
                                  magtheridon->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(magtheridon))
    {
        return MoveTo(magtheridon->GetMapId(), magtheridon->GetPositionX(), 
                      magtheridon->GetPositionY(), magtheridon->GetPositionZ());
    }

    return false;
}

bool MagtheridonPositionBossAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return magtheridon && magtheridon->IsAlive() && 
           !magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)) && 
           botAI->IsMainTank(bot) && group;
}

bool MagtheridonSpreadRangedAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)))
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const float spreadCenterX = -15.585;
    const float spreadCenterY = 1.761f;

    Group* group = bot->GetGroup();
    std::vector<Player*> raidMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        raidMembers.push_back(member);
    }

    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        size_t botIndex = 0;
        for (size_t i = 0; i < raidMembers.size(); ++i)
        {
            if (raidMembers[i] == bot)
            {
                botIndex = i;
                break;
            }
        }

        float arcRadians = 2.0f * M_PI;
        float startAngle = 0.0f;
        float angleStep = raidMembers.size() > 0 ? arcRadians / raidMembers.size() : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        float minRadius = 1.0f;
        float maxRadius = 25.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = spreadCenterX + radius * cos(angle);
        float targetY = spreadCenterY + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        float destX = targetPosition.GetPositionX();
        float destY = targetPosition.GetPositionY();
        float destZ = bot->GetPositionZ();
        if (!bot->IsWithinDist2d(destX, destY, 3.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                           bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float magtheridonRangedRadius = 15.0f;
    const float minSpreadDistance = 5.0f;
    const float movementThreshold = 2.0f;

    Unit* closestMember = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }
        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
            closestMember = member;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float magX = magtheridon->GetPositionX();
    float magY = magtheridon->GetPositionY();
    float distanceToMagtheridon = bot->GetExactDist2d(magX, magY);

    float destX2 = magX;
    float destY2 = magY;
    float destZ2 = bot->GetPositionZ();
    if ((distanceToMagtheridon < magtheridonRangedRadius - 3.0f - movementThreshold) && 
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                       bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}

bool MagtheridonSpreadRangedAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)), 
                            lastShadowCageState, magtheridonBlastNovaTimer);
    }

    Group* group = bot->GetGroup();
    if (!group || !botAI->IsRanged(bot) || botAI->IsHeal(bot) || !magtheridon->IsAlive() || 
        magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)))
    {
        return false;
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];

        if (now - lastBlastNova >= 48)
        {
            return false;
        }
    }

    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (channeler && channeler->IsAlive())
    {
        return false;
    }

    return true;
}

bool MagtheridonSpreadHealerAction::Execute(Event event)
{
    static std::unordered_map<ObjectGuid, Position> initialPositions;
    static std::unordered_map<ObjectGuid, bool> hasReachedInitialPosition;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon && magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)))
    {
        initialPositions.clear();
        hasReachedInitialPosition.clear();
    }

    const float spreadCenterX = -2.005f;
    const float spreadCenterY = 1.867f;

    Group* group = bot->GetGroup();
    std::vector<Player*> raidMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        raidMembers.push_back(member);
    }

    if (initialPositions.find(bot->GetGUID()) == initialPositions.end())
    {
        size_t botIndex = 0;
        for (size_t i = 0; i < raidMembers.size(); ++i)
        {
            if (raidMembers[i] == bot)
            {
                botIndex = i;
                break;
            }
        }

        float arcRadians = 2.0f * M_PI;
        float startAngle = 0.0f;
        float angleStep = raidMembers.size() > 0 ? arcRadians / raidMembers.size() : 0.0f;
        float angle = startAngle + botIndex * angleStep;

        float minRadius = 1.0f;
        float maxRadius = 15.0f;
        float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

        float targetX = spreadCenterX + radius * cos(angle);
        float targetY = spreadCenterY + radius * sin(angle);
        float targetZ = bot->GetPositionZ();

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, targetZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        float destX = targetPosition.GetPositionX();
        float destY = targetPosition.GetPositionY();
        float destZ = bot->GetPositionZ();
        if (!bot->IsWithinDist2d(destX, destY, 3.0f) && 
            bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                           bot->GetPositionZ(), destX, destY, destZ))
        {
            return MoveTo(bot->GetMapId(), destX, destY, destZ);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    const float magtheridonHealerRadius = 10.0f;
    const float minSpreadDistance = 5.0f;
    const float movementThreshold = 2.0f;

    Unit* closestMember = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member == bot)
        {
            continue;
        }
        if (!closestMember || bot->GetExactDist2d(member) < bot->GetExactDist2d(closestMember))
            closestMember = member;
    }

    if (closestMember && bot->GetExactDist2d(closestMember) < minSpreadDistance - movementThreshold)
    {
        return MoveAway(closestMember, minSpreadDistance);
    }

    float magX = magtheridon->GetPositionX();
    float magY = magtheridon->GetPositionY();
    float distanceToMagtheridon = bot->GetExactDist2d(magX, magY);

    float destX2 = magX;
    float destY2 = magY;
    float destZ2 = bot->GetPositionZ();
    if ((distanceToMagtheridon < magtheridonHealerRadius - 3.0f - movementThreshold) && 
        bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), 
                                                       bot->GetPositionZ(), destX2, destY2, destZ2))
    {
        return MoveTo(bot->GetMapId(), destX2, destY2, destZ2);
    }

    return false;
}

bool MagtheridonSpreadHealerAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)), 
                            lastShadowCageState, magtheridonBlastNovaTimer);
    }

    Group* group = bot->GetGroup();
    if (!group || !magtheridon->IsAlive() || magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)) || 
        !botAI->IsHeal(bot))
    {
        return false;
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];
        if (now - lastBlastNova >= 48)
        {
            return false;
        }
    }

    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    if (channeler && channeler->IsAlive())
    {
        return false;
    }

    return true;
}

bool MagtheridonUseManticronCubeAction::Execute(Event event)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    auto it = botToCubeAssignment.find(bot->GetGUID());
    const CubeInfo& cubeInfo = it->second;
    GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
    if (!cube) 
    {
        return false;
    }

    if (bot->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_GRASP)) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagtheridonSpells::BLAST_NOVA))))
    {
            uint32 delay = urand(200, 3000);
            botAI->AddTimedEvent(
                [this, cube]
                {
                    botAI->Reset();
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            return true;
    }

    time_t now = time(nullptr);
    time_t lastBlastNova = magtheridonBlastNovaTimer[bot->GetMapId()];
    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
    magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagtheridonSpells::BLAST_NOVA));
    if (now - lastBlastNova < 48)
    {
        return false;
    }

    if (!blastNovaActive) 
    {
        const float safeWaitDistance = 8.0f;
        float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

        if (fabs(cubeDist - safeWaitDistance) > 0.5f) 
        {
            for (int i = 0; i < 12; ++i)
            {
                float angle = i * M_PI / 6.0f;
                float targetX = cubeInfo.x + cos(angle) * safeWaitDistance;
                float targetY = cubeInfo.y + sin(angle) * safeWaitDistance;
                float targetZ = bot->GetPositionZ();

                if (IsSafeFromMagtheridonHazards(botAI, bot, targetX, targetY, targetZ))
                {
                    bot->AttackStop();
                    bot->InterruptNonMeleeSpells(false);
                    return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                                  MovementPriority::MOVEMENT_COMBAT, true, false);
                }
            }
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
            float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;
            float fallbackZ = bot->GetPositionZ();
            return MoveTo(bot->GetMapId(), fallbackX, fallbackY, fallbackZ, false, false, false, false, 
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            return true;
        }
    }

    const float interactDistance = 1.0f;
    const float interactDistanceBuffer = 1.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist > interactDistance) 
    {
        if (cubeDist <= interactDistance + interactDistanceBuffer) 
        {
            uint32 delay = urand(200, 1500);
            botAI->AddTimedEvent(
                [this, cube]
                {
                    bot->StopMoving();
                    cube->Use(bot);
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            return true;
        }
        
        float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
        float targetX = cubeInfo.x - cos(angle) * interactDistance;
        float targetY = cubeInfo.y - sin(angle) * interactDistance;
        float targetZ = bot->GetPositionZ();
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(false);

        return MoveTo(bot->GetMapId(), targetX, targetY, targetZ, false, false, false, false, 
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::isUseful()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (magtheridon)
    {
        bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(static_cast<uint32>(MagtheridonSpells::BLAST_NOVA));
        UpdateTransitionTimer(magtheridon, blastNovaActive, lastBlastNovaState, magtheridonBlastNovaTimer);
        UpdateTransitionTimer(magtheridon, magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)), 
                              lastShadowCageState, magtheridonBlastNovaTimer);
    }

    Group* group = bot->GetGroup();
    if (!group || !magtheridon || !magtheridon->IsAlive() || 
        magtheridon->HasAura(static_cast<uint32>(MagtheridonSpells::SHADOW_CAGE)))
    {
        return false;
    }

    bool needsReassign = botToCubeAssignment.empty();
    if (!needsReassign)
    {
        for (const auto& pair : botToCubeAssignment)
        {
            Player* assigned = ObjectAccessor::FindPlayer(pair.first);
            if (!assigned || !assigned->IsAlive())
            {
                needsReassign = true;
                break;
            }
        }
    }

    if (needsReassign)
    {
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
    }

    return botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end();
}
