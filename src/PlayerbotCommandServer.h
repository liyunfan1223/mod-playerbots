/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTCOMMANDSERVER_H
#define _PLAYERBOT_PLAYERBOTCOMMANDSERVER_H

class PlayerbotCommandServer
{
public:
    PlayerbotCommandServer() {}
    virtual ~PlayerbotCommandServer() {}
    static PlayerbotCommandServer* instance()
    {
        static PlayerbotCommandServer instance;
        return &instance;
    }

    void Start();
};

#define sPlayerbotCommandServer PlayerbotCommandServer::instance()

#endif
