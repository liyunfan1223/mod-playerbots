/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATHELPER_H
#define _PLAYERBOT_CHATHELPER_H

#include <map>

#include "Common.h"
#include "ObjectGuid.h"
#include "PlayerbotAIAware.h"
#include "SharedDefines.h"

class GameObject;
class Quest;
class Player;
class PlayerbotAI;
class SpellInfo;
class WorldObject;

struct ItemTemplate;

typedef std::set<uint32> ItemIds;
typedef std::set<uint32> SpellIds;

class ChatHelper : public PlayerbotAIAware
{
public:
    ChatHelper(PlayerbotAI* botAI);

    static std::string const formatMoney(uint32 copper);
    static uint32 parseMoney(std::string const text);
    static ItemIds parseItems(std::string const text);
    uint32 parseSpell(std::string const text);
    static std::string parseValue(const std::string& type, const std::string& text);

    static std::string const FormatQuest(Quest const* quest);
    static std::string const FormatItem(ItemTemplate const* proto, uint32 count = 0, uint32 total = 0);
    static std::string const FormatQItem(uint32 itemId);
    static std::string const FormatSpell(SpellInfo const* spellInfo);
    static std::string const FormatGameobject(GameObject* go);
    static std::string const FormatWorldobject(WorldObject* wo);
    static std::string const FormatWorldEntry(int32 entry);
    static std::string const FormatQuestObjective(std::string const name, uint32 available, uint32 required);
    static GuidVector parseGameobjects(std::string const text);

    static ChatMsg parseChat(std::string const text);
    static std::string const FormatChat(ChatMsg chat);

    static std::string const FormatClass(Player* player, int8 spec);
    static std::string const FormatClass(uint8 cls);
    static std::string const FormatRace(uint8 race);
    static std::string const FormatSkill(uint32 skill);
    static std::string const FormatBoolean(bool flag);

    static uint32 parseItemQuality(std::string const text);
    static bool parseItemClass(std::string const text, uint32* itemClass, uint32* itemSubClass);
    static uint32 parseSlot(std::string const text);
    uint32 parseSkill(std::string const text);

    static bool parseable(std::string const text);

    void eraseAllSubStr(std::string& mainStr, std::string const toErase);

    static std::set<uint32> ExtractAllQuestIds(const std::string& text);
    static std::set<uint32> ExtractAllItemIds(const std::string& text);

private:
    static std::map<std::string, uint32> consumableSubClasses;
    static std::map<std::string, uint32> tradeSubClasses;
    static std::map<std::string, uint32> itemQualities;
    static std::map<std::string, uint32> projectileSubClasses;
    static std::map<std::string, uint32> slots;
    static std::map<std::string, uint32> skills;
    static std::map<std::string, ChatMsg> chats;
    static std::map<uint8, std::string> classes;
    static std::map<uint8, std::string> races;
    static std::map<uint8, std::map<uint8, std::string>> specs;
};

#endif
