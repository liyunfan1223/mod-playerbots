/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SeeSpellAction.h"

#include "Event.h"
#include "Formations.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "RTSCValues.h"
#include "RtscAction.h"

Creature* SeeSpellAction::CreateWps(Player* wpOwner, float x, float y, float z, float o, uint32 entry, Creature* lastWp,
                                    bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig->reactDelay;

    if (!important)
        delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSUMMON_TIMED_DESPAWN, delay);
    if (!important)
        wpCreature->SetObjectScale(0.2f);

    return wpCreature;
}

bool SeeSpellAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());  //
    uint32 spellId;
    uint8 castCount, castFlags;
    Player* master = botAI->GetMaster();

    p.rpos(0);
    p >> castCount >> spellId >> castFlags;

    if (!master)
        return false;

    // if (!botAI->HasStrategy("RTSC", botAI->GetState()))
    //     return false;

    if (spellId != RTSC_MOVE_SPELL)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

    SpellCastTargets targets;
    targets.Read(p, botAI->GetMaster());

    WorldPosition spellPosition(master->GetMapId(), targets.GetDst()->_position);
    SET_AI_VALUE(WorldPosition, "see spell location", spellPosition);

    bool selected = AI_VALUE(bool, "RTSC selected");
    bool inRange = spellPosition.distance(bot) <= 10;
    std::string const nextAction = AI_VALUE(std::string, "RTSC next spell action");

    if (nextAction.empty())
    {
        if (!inRange && selected)
            master->SendPlaySpellVisual(bot->GetGUID(), 6372);
        else if (inRange && !selected)
            master->SendPlaySpellVisual(bot->GetGUID(), 5036);

        SET_AI_VALUE(bool, "RTSC selected", inRange);

        if (selected)
            return MoveToSpell(spellPosition);

        return inRange;
    }
    else if (nextAction == "move")
    {
        return MoveToSpell(spellPosition);
    }
    else if (nextAction.find("save ") != std::string::npos)
    {
        std::string locationName;
        if (nextAction.find("save selected ") != std::string::npos)
        {
            if (!selected)
                return false;

            locationName = nextAction.substr(14);
        }
        else
            locationName = nextAction.substr(5);

        SetFormationOffset(spellPosition);

        SET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName, spellPosition);

        Creature* wpCreature =
            bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(),
                                spellPosition.getO(), TEMPSUMMON_TIMED_DESPAWN, 2000.0f);
        wpCreature->SetObjectScale(0.5f);
        RESET_AI_VALUE(std::string, "RTSC next spell action");

        return true;
    }

    return false;
}

bool SeeSpellAction::SelectSpell(WorldPosition& spellPosition)
{
    Player* master = botAI->GetMaster();
    if (spellPosition.distance(bot) <= 5 || AI_VALUE(bool, "RTSC selected"))
    {
        SET_AI_VALUE(bool, "RTSC selected", true);
        master->SendPlaySpellVisual(bot->GetGUID(), 5036);
    }

    return true;
}

bool SeeSpellAction::MoveToSpell(WorldPosition& spellPosition, bool inFormation)
{
    if (inFormation)
        SetFormationOffset(spellPosition);

    if (bot->IsWithinLOS(spellPosition.getX(), spellPosition.getY(), spellPosition.getZ()))
        return MoveNear(spellPosition.getMapId(), spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), 0);

    return MoveTo(spellPosition.getMapId(), spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), false,
                  false);
}

void SeeSpellAction::SetFormationOffset(WorldPosition& spellPosition)
{
    Player* master = botAI->GetMaster();

    Formation* formation = AI_VALUE(Formation*, "formation");

    WorldLocation formationLocation = formation->GetLocation();

    if (formationLocation.GetPositionX() != 0 || formationLocation.GetPositionY() != 0)
    {
        spellPosition -= WorldPosition(master);
        spellPosition += formationLocation;
    }
}
