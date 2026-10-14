/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

bool IccValkyreSpearAction::Execute(Event /*event*/)
{
    // Find the nearest spear
    Creature* spear = bot->FindNearestCreature(NPC_SPEAR, 100.0f);
    if (!spear)
        return false;

    // Move to the spear if not in range
    if (!spear->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
        return MoveTo(spear, INTERACTION_DISTANCE);

    // Remove shapeshift forms
    botAI->RemoveShapeshift();

    // Stop movement and click the spear
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    spear->HandleSpellClick(bot);

    // Dismount if mounted
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    return false;
}

bool IccSisterSvalnaAction::Execute(Event /*event*/)
{
    Unit* svalna = AI_VALUE2(Unit*, "find target", "sister svalna");
    if (!svalna || !svalna->HasAura(SPELL_AETHER_SHIELD))
        return false;

    // Check if bot has the spear item
    if (!botAI->HasItemInInventory(ITEM_SPEAR))
        return false;

    // Get all items from inventory
    std::vector<Item*> items = botAI->GetInventoryItems();
    for (Item* item : items)
    {
        if (item->GetEntry() == ITEM_SPEAR)
        {
            // Use spear on Svalna
            botAI->ImbueItem(item, svalna);
            return false;
        }
    }

    return false;
}
