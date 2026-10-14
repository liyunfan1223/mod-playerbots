/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTCOMMANDSERVER_H
#define PLAYERBOTS_PLAYERBOTCOMMANDSERVER_H

class PlayerbotCommandServer
{
public:
    static PlayerbotCommandServer& instance()
    {
        static PlayerbotCommandServer instance;

        return instance;
    }

    void Start();

private:
    PlayerbotCommandServer() = default;
    ~PlayerbotCommandServer() = default;

    PlayerbotCommandServer(PlayerbotCommandServer const&) = delete;
    PlayerbotCommandServer& operator=(PlayerbotCommandServer const&) = delete;

    PlayerbotCommandServer(PlayerbotCommandServer&&) = delete;
    PlayerbotCommandServer& operator=(PlayerbotCommandServer&&) = delete;
};

#endif
