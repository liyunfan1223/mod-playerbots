/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BUYEXTENDEDCOSTITEMACTION_H
#define PLAYERBOTS_BUYEXTENDEDCOSTITEMACTION_H

#include <vector>
#include "MovementActions.h"

class PlayerbotAI;
struct VendorItemData;

// Buys gear paid with honor, arena points or emblems (npc_vendor.ExtendedCost -> ItemExtendedCost.dbc).
class BuyExtendedCostItemAction : public MovementAction
{
public:
    BuyExtendedCostItemAction(PlayerbotAI* botAI) : MovementAction(botAI, "buy extended cost item") {}

    bool isUseful() override;
    bool Execute(Event event) override;

private:
    struct Choice
    {
        uint32 slot = 0;
        uint32 itemId = 0;
        float score = 0.0f;
    };

    bool CanShop() const;
    bool FindBest(VendorItemData const* items, Choice& best);
    bool TryBuyFromNearbyVendor();
    bool TravelToVendor();
    bool TravelToFlightMaster();
    bool TryPlanTaxiTrip(uint32 now);
    void ResetTravel();

    uint32 nextCheck = 0;       // getMSTime() before which nothing new is evaluated
    uint32 windowStart = 0;     // start of the current purchase window
    uint32 windowPurchases = 0; // purchases done in the current window
    bool travelling = false;
    uint32 arrivalTries = 0;
    uint32 travelStart = 0;
    uint32 vendorEntry = 0;
    uint32 vendorMap = 0;
    float vendorX = 0.0f;
    float vendorY = 0.0f;
    float vendorZ = 0.0f;

    // Item Y: cross-map purchase by taxi. The bot first walks to a flight master on its own map,
    // flies to the node nearest to the destination vendor, then walks to the vendor as usual.
    bool toFlightMaster = false;
    bool flying = false;
    uint32 fmEntry = 0;
    uint32 fmMap = 0;
    float fmX = 0.0f;
    float fmY = 0.0f;
    float fmZ = 0.0f;
    std::vector<uint32> taxiNodes;
};

#endif
