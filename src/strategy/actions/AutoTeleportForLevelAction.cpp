#include "AutoTeleportForLevelAction.h"

#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"

bool AutoTeleportForLevelAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig->autoTeleportForLevel || !sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        return false;
    }
    if (botAI->HasRealPlayerMaster())
    {
        return false;
    }
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
    return true;
}