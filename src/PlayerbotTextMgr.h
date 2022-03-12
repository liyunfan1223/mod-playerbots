/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTTEXTMGR_H
#define _PLAYERBOT_PLAYERBOTTEXTMGR_H

#include "Common.h"

#include <map>
#include <vector>

class PlayerbotTextMgr
{
    public:
        PlayerbotTextMgr() { };
        virtual ~PlayerbotTextMgr() { };
        static PlayerbotTextMgr* instance()
        {
            static PlayerbotTextMgr instance;
            return &instance;
        }

        std::string const Format(std::string const key, std::map<std::string, std::string> placeholders);

    private:
        void LoadTemplates();

        std::map<std::string, std::vector<std::string>> templates;
};

#define sPlayerbotTextMgr PlayerbotTextMgr::instance()

#endif
