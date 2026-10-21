/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LIVING_ECONOMY_ACTIONS_H
#define PLAYERBOTS_LIVING_ECONOMY_ACTIONS_H

#include <unordered_map>

#include "MovementActions.h"
#include "Value.h"

namespace LivingProfessions
{
struct Stock
{
    std::unordered_map<uint32, uint32> reserve;
    std::unordered_map<uint32, uint32> costs;
};
std::vector<Item*> BagItems(Player* player);
Stock ProfessionStock(Player* player);
}  // namespace LivingProfessions

enum class LivingEconomyJob
{
    Supplies,
    Auction,
    Mail
};

enum class LivingSupplyKind
{
    Reagent,
    Tool,
    FishingPole
};

struct LivingEconomyPlan
{
    ObjectGuid target;
    ObjectGuid item;
    uint32 entry = 0;
    uint32 slot = 0;
    uint32 needed = 0;
    uint32 recipe = 0;
    LivingSupplyKind supplyKind = LivingSupplyKind::Reagent;
    uint32 category = 0;
    bool flip = false;  // auction job: look for an underpriced listing to buy and resell
};

class LivingEconomyValue : public CalculatedValue<LivingEconomyPlan>
{
public:
    LivingEconomyValue(PlayerbotAI* botAI, std::string const name, LivingEconomyJob job)
        : CalculatedValue(botAI, name, 60000), _job(job)
    {
    }
    LivingEconomyPlan Calculate() override;

private:
    LivingEconomyJob _job;
};

// A trader holding surplus to sell heads for the nearest friendly auctioneer on its map when it
// goes visiting NPCs; once within reach, the "living auction" job lists the goods.
struct LivingMarketTrip
{
    bool go = false;
    WorldLocation destination;
};

class LivingMarketTripValue : public CalculatedValue<LivingMarketTrip>
{
public:
    LivingMarketTripValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "living market trip", 60000) {}
    LivingMarketTrip Calculate() override;
};

class LivingEconomyAction : public MovementAction
{
public:
    LivingEconomyAction(PlayerbotAI* botAI, std::string const name, LivingEconomyJob job)
        : MovementAction(botAI, name), _job(job)
    {
    }
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;

private:
    LivingEconomyJob _job;
};

#endif
