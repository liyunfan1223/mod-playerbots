/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SaveManaAction.h"

#include "Event.h"
#include "Playerbots.h"

bool SaveManaAction::Execute(Event event)
{
    std::string const text = event.getParam();
    double value = AI_VALUE(double, "mana save level");

    if (text == "?")
    {
        std::ostringstream out;
        out << "Mana save level: " << Format(value);
        botAI->TellMaster(out);
        return true;
    }

    if (text == "*")
    {
        switch (bot->getClass())
        {
            case CLASS_HUNTER:
            case CLASS_SHAMAN:
            case CLASS_DRUID:
                value = 5.0;
                break;
            case CLASS_MAGE:
            case CLASS_PRIEST:
            case CLASS_WARLOCK:
                value = 2.0;
                break;
            default:
                value = 3.0;
        }
    }
    else if (text.empty())
    {
        value = 1.0;
    }
    else
    {
        value = std::atof(text.c_str());
    }

    value = std::min(10.0, value);
    value = std::max(1.0, value);
    value = std::floor(value * 100 + 0.5) / 100.0;

    botAI->GetAiObjectContext()->GetValue<double>("mana save level")->Set(value);

    std::ostringstream out;
    out << "Mana save level set: " << Format(value);
    botAI->TellMaster(out);

    return true;
}

std::string const SaveManaAction::Format(double value)
{
    std::ostringstream out;

    if (value <= 1.0)
        out << "|cFF808080";
    else if (value <= 5.0)
        out << "|cFF00FF00";
    else if (value <= 7.0)
        out << "|cFFFFFF00";
    else
        out << "|cFFFF0000";

    out << value << "|cffffffff";
    return out.str();
}
