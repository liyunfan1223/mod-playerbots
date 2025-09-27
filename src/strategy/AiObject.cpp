/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AiObject.h"

#include "Playerbots.h"

AiObject::AiObject(PlayerbotAI* botAI)
    : PlayerbotAIAware(botAI), bot(botAI->GetBot()), context(botAI->GetAiObjectContext()), chat(botAI->GetChatHelper())
{
}

Player* AiObject::GetMaster() { return botAI->GetMaster(); }
