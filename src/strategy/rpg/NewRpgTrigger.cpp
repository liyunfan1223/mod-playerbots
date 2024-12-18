#include "NewRpgTriggers.h"
#include "PlayerbotAI.h"

bool NewRpgStatusTrigger::IsActive() { return status == botAI->rpgInfo.status; }