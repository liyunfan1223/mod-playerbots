/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellLosAction.h"
#include "Event.h"
#include "ChatHelper.h"
#include "Playerbots.h"

bool TellLosAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits("--- Targets ---", *context->GetValue<GuidVector>("possible targets"));
        ListUnits("--- Targets (All) ---", *context->GetValue<GuidVector>("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits("--- NPCs ---", *context->GetValue<GuidVector>("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits("--- Corpses ---", *context->GetValue<GuidVector>("nearest corpses"));
    }

    if (param.empty() || param == "gos" || param == "game objects")
    {
        ListGameObjects("--- Game objects ---", *context->GetValue<GuidVector>("nearest game objects"));
    }

    if (param.empty() || param == "players")
    {
        ListUnits("--- Friendly players ---", *context->GetValue<GuidVector>("nearest friendly players"));
    }

    return true;
}

void TellLosAction::ListUnits(std::string const title, GuidVector units)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : units)
    {
        if (Unit* unit = botAI->GetUnit(guid))
            botAI->TellMaster(unit->GetName());
    }

}
void TellLosAction::ListGameObjects(std::string const title, GuidVector gos)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            botAI->TellMaster(chat->FormatGameobject(go));
    }
}
