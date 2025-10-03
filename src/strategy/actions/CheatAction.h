/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Action.h"

class PlayerbotAI;

enum class BotCheatMask : uint32;

class CheatAction : public Action
{
public:
    CheatAction(PlayerbotAI* botAI) : Action(botAI, "cheat") {}

    bool Execute(Event event) override;

private:
    static BotCheatMask GetCheatMask(std::string const cheat);
    static std::string const GetCheatName(BotCheatMask cheatMask);
    void ListCheats();
};
