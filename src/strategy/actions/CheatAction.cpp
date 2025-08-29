/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CheatAction.h"

#include "Playerbots.h"

bool CheatAction::Execute(Event event)
{
    std::string const param = event.getParam();

    uint32 cheatMask = (uint32)botAI->GetCheat();

    std::vector<std::string> splitted = split(param, ',');
    for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
    {
        const char* name = i->c_str();
        switch (name[0])
        {
            case '+':
                cheatMask |= (uint32)GetCheatMask(name + 1);
                break;
            case '-':
                cheatMask ^= (uint32)GetCheatMask(name + 1);
                break;
            case '?':
                ListCheats();
                return true;
        }
    }

    botAI->SetCheat(BotCheatMask(cheatMask));

    return true;
}

BotCheatMask CheatAction::GetCheatMask(std::string const cheat)
{
    if (cheat == "taxi")
        return BotCheatMask::taxi;

    if (cheat == "gold")
        return BotCheatMask::gold;

    if (cheat == "health")
        return BotCheatMask::health;

    if (cheat == "mana")
        return BotCheatMask::mana;

    if (cheat == "power")
        return BotCheatMask::power;

    if (cheat == "raid")
        return BotCheatMask::raid;

    return BotCheatMask::none;
}

std::string const CheatAction::GetCheatName(BotCheatMask cheatMask)
{
    switch (cheatMask)
    {
        case BotCheatMask::taxi:
            return "taxi";
        case BotCheatMask::gold:
            return "gold";
        case BotCheatMask::health:
            return "health";
        case BotCheatMask::mana:
            return "mana";
        case BotCheatMask::power:
            return "power";
        case BotCheatMask::raid:
            return "raid";
        default:
            return "none";
    }
}

void CheatAction::ListCheats()
{
    std::ostringstream out;
    for (int i = 0; i < log2((uint32)BotCheatMask::maxMask); i++)
    {
        BotCheatMask cheatMask = BotCheatMask(1 << i);
        if ((uint32)cheatMask & (uint32)sPlayerbotAIConfig->botCheatMask)
            out << "[conf:" << GetCheatName(BotCheatMask(cheatMask)) << "]";
        else if (botAI->HasCheat(cheatMask))
            out << "[" << GetCheatName(BotCheatMask(cheatMask)) << "]";
    }

    botAI->TellMasterNoFacing(out);
}
