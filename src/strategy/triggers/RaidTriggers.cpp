/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RaidTriggers.h"
#include "Group.h"

bool IsBotMainTankTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool botIsMainTank = botAI->IsMainTank(bot);
    bool botIsAssistTankOfIndex0 = botAI->IsAssistTankOfIndex(bot, 0);
    bool botIsAssistTankOfIndex1 = botAI->IsAssistTankOfIndex(bot, 1);
    bool botIsAssistTankOfIndex2 = botAI->IsAssistTankOfIndex(bot, 2);

    if (!botIsMainTank && !botIsAssistTankOfIndex0 && !botIsAssistTankOfIndex1 && !botIsAssistTankOfIndex2)
    {
        return false;
    }

    Player* mainTank = nullptr;
    Player* assistTankOfIndex0 = nullptr;
    Player* assistTankOfIndex1 = nullptr;
    Player* assistTankOfIndex2 = nullptr;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
        {
            continue;
        }

        if (botAI->IsMainTank(member) && GET_PLAYERBOT_AI(member))
        {
            mainTank = member;
            break;
        }
        else
        {
            for (int i = 0; i < 3; ++i)
            {
                if (botAI->IsAssistTankOfIndex(member, i) && GET_PLAYERBOT_AI(member))
                {
                    if (i == 0)
                    {
                        assistTankOfIndex0 = member;
                        break;
                    }
                    else if (i == 1)
                    {
                        assistTankOfIndex1 = member;
                        break;
                    }
                    else if (i == 2)
                    {
                        assistTankOfIndex2 = member;
                        break;
                    }
                }
            }
        }
    }

    if (mainTank && botIsMainTank)
    {
        return true;
    }
    else if (!mainTank)
    {
        if (assistTankOfIndex0 && botIsAssistTankOfIndex0)
        {
            return true;
        }
        else if (!assistTankOfIndex0 && assistTankOfIndex1 && botIsAssistTankOfIndex1)
        {
            return true;
        }
        else if (!assistTankOfIndex0 && !assistTankOfIndex1 && assistTankOfIndex2 && botIsAssistTankOfIndex2)
        {
            return true;
        }
    }

    return false;
}
