/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RtiAction.h"

#include "Event.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

bool RtiAction::Execute(Event event)
{
    std::string text = event.getParam();
    std::string type = "rti";
    if (text.find("cc ") == 0)
    {
        type = "rti cc";
        text = text.substr(3);
    }

    if (text.empty() || text == "?")
    {
        std::ostringstream outRti;
        outRti << "rti"
               << ": ";
        AppendRti(outRti, "rti");
        botAI->TellMaster(outRti);

        std::ostringstream outRtiCc;
        outRtiCc << "rti cc"
                 << ": ";
        AppendRti(outRtiCc, "rti cc");
        botAI->TellMaster(outRtiCc);
        return true;
    }

    context->GetValue<std::string>(type)->Set(text);

    std::ostringstream out;
    out << type << " set to: ";
    AppendRti(out, type);
    botAI->TellMaster(out);
    return true;
}

void RtiAction::AppendRti(std::ostringstream& out, std::string const type)
{
    out << AI_VALUE(std::string, type);

    std::ostringstream n;
    n << type << " target";

    if (Unit* target = AI_VALUE(Unit*, n.str()))
        out << " (" << target->GetName() << ")";
}

bool MarkRtiAction::Execute(Event event)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->InBattleground())
        return false;

    Unit* target = nullptr;
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        // do not mark players
        if (unit->IsPlayer())
            continue;

        bool marked = false;
        for (uint8 i = 0; i < 8; i++)
        {
            ObjectGuid iconGUID = group->GetTargetIcon(i);
            if (iconGUID == unit->GetGUID())
            {
                marked = true;
                break;
            }
        }

        if (marked)
            continue;

        if (!target || target->GetHealth() > unit->GetHealth())
            target = unit;
    }

    if (!target)
        return false;

    std::string const rti = AI_VALUE(std::string, "rti");
    uint8 index = RtiTargetValue::GetRtiIndex(rti);
    group->SetTargetIcon(index, bot->GetGUID(), target->GetGUID());
    return true;
}
