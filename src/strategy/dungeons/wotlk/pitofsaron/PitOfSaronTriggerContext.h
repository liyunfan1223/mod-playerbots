#ifndef _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERCONTEXT_H
#define _PLAYERBOT_WOTLKDUNGEONPOSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "PitOfSaronTriggers.h"

class WotlkDungeonPoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonPoSTriggerContext()
    {
        creators["ick and krick"] = &WotlkDungeonPoSTriggerContext::ick_and_krick;
        creators["tyrannus"] = &WotlkDungeonPoSTriggerContext::tyrannus;
    }

private:
    static Trigger* ick_and_krick(PlayerbotAI* ai) { return new IckAndKrickTrigger(ai); }
    static Trigger* tyrannus(PlayerbotAI* ai) { return new TyrannusTrigger(ai); }
};

#endif
