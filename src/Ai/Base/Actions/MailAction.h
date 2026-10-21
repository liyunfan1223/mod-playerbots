/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAILACTION_H
#define PLAYERBOTS_MAILACTION_H

#include "InventoryAction.h"

class ObjectGuid;
class PlayerbotAI;

struct Mail;

class MailProcessor
{
public:
    virtual bool Before([[maybe_unused]] PlayerbotAI* botAI) { return true; }
    virtual bool Process(uint32 index, Mail* mail, PlayerbotAI* botAI) = 0;
    virtual bool After([[maybe_unused]] PlayerbotAI* botAI) { return true; }

    // The nearest mailbox in sight.
    static ObjectGuid FindMailbox(PlayerbotAI* botAI);
    static void RemoveMail(Player* bot, uint32 id, ObjectGuid mailbox);
};

class MailAction : public InventoryAction
{
public:
    MailAction(PlayerbotAI* botAI) : InventoryAction(botAI, "mail") {}

    bool Execute(Event event) override;

private:
    static std::map<std::string, MailProcessor*> processors;
};

#endif
