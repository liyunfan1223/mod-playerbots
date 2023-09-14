#include "AutoTeleportForLevelAction.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"

bool AutoTeleportForLevelAction::Execute(Event event) {
    AutoUpgradeEquip();

    if (!sPlayerbotAIConfig->autoTeleportForLevel || !sRandomPlayerbotMgr->IsRandomBot(bot)) {
        return false;
    }
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
    return true;
}

void AutoTeleportForLevelAction::AutoUpgradeEquip() {
    if (!sPlayerbotAIConfig->autoUpgradeEquip || !sRandomPlayerbotMgr->IsRandomBot(bot)) {
        return;
    }
    PlayerbotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_RARE);
    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel) {
        factory.InitEquipment(true);
    }
    factory.InitAmmo();
}