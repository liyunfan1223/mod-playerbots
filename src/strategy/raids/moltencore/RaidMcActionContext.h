#ifndef _PLAYERBOT_RAIDMCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidMcActions.h"

class RaidMcActionContext : public NamedObjectContext<Action>
{
public:
    RaidMcActionContext()
    {
        creators["mc check should move from group"] = &RaidMcActionContext::check_should_move_from_group;
        creators["mc move from baron geddon"] = &RaidMcActionContext::move_from_baron_geddon;
    }

private:
    static Action* check_should_move_from_group(PlayerbotAI* ai) { return new McCheckShouldMoveFromGroupAction(ai); }
    static Action* move_from_baron_geddon(PlayerbotAI* ai) { return new McMoveFromBaronGeddonAction(ai); }
};

#endif
