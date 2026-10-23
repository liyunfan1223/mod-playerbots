/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BuyExtendedCostItemAction.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
#include <vector>
#include "DBCStores.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"
#include "TravelNode.h"

namespace
{
// Emblem items accepted as ExtendedCost currency (reqitem in ItemExtendedCost.dbc).
constexpr std::array<uint32, 6> kEmblemIds = {
    29434, // Badge of Justice
    40752, // Emblem of Heroism
    40753, // Emblem of Valor
    45624, // Emblem of Conquest
    47241, // Emblem of Triumph
    49426  // Emblem of Frost
};

constexpr uint32 kCheckIntervalMs = 5 * 60 * 1000;  // pause after an evaluation that bought nothing
constexpr uint32 kAfterBuyIntervalMs = 30 * 1000;   // pause after a purchase
constexpr uint32 kWindowMs = 15 * 60 * 1000;        // purchase window
constexpr uint32 kMaxPurchasesPerWindow = 3;        // purchases allowed per window
constexpr uint32 kTravelTimeoutMs = 5 * 60 * 1000;  // give up walking to the vendor
constexpr float kArriveDistance = 6.0f;
constexpr uint32 kMaxVendorsEvaluated = 4;
constexpr float kMaxFlightMasterWalk = 2000.0f;     // item Y: max distance to the origin flight master
// Item Y: destination maps searched for vendors, in order (Northrend first, then the capitals' continents).
constexpr std::array<uint32, 3> kTaxiMaps = {571, 0, 1};

bool IsEmblem(uint32 itemId)
{
    return std::find(kEmblemIds.begin(), kEmblemIds.end(), itemId) != kEmblemIds.end();
}

// Cost uses only honor, arena points and emblems (no other turn-in items).
bool IsCurrencyCost(ItemExtendedCostEntry const* iece)
{
    if (!iece)
        return false;

    bool any = iece->reqhonorpoints || iece->reqarenapoints;
    for (uint8 i = 0; i < MAX_ITEM_EXTENDED_COST_REQUIREMENTS; ++i)
    {
        if (!iece->reqitem[i])
            continue;

        if (!IsEmblem(iece->reqitem[i]))
            return false;

        any = true;
    }

    return any;
}

bool IsGearProto(ItemTemplate const* proto)
{
    if (!proto || (proto->Class != ITEM_CLASS_WEAPON && proto->Class != ITEM_CLASS_ARMOR))
        return false;

    return proto->InventoryType != INVTYPE_NON_EQUIP && proto->InventoryType != INVTYPE_BODY &&
           proto->InventoryType != INVTYPE_TABARD && proto->InventoryType != INVTYPE_BAG;
}

bool VendorSellsCurrencyGear(uint32 entry)
{
    CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(entry);
    if (!tmpl || !(tmpl->npcflag & UNIT_NPC_FLAG_VENDOR))
        return false;

    VendorItemData const* items = sObjectMgr->GetNpcVendorItemList(entry);
    if (!items)
        return false;

    for (uint32 i = 0; i < items->GetItemCount(); ++i)
    {
        VendorItem const* vi = items->GetItem(i);
        if (!vi || !vi->ExtendedCost)
            continue;

        if (IsCurrencyCost(sItemExtendedCostStore.LookupEntry(vi->ExtendedCost)) &&
            IsGearProto(sObjectMgr->GetItemTemplate(vi->item)))
            return true;
    }

    return false;
}

struct VendorSpawn
{
    uint32 entry;
    uint32 map;
    float x;
    float y;
    float z;
};

// Spawns of vendors that sell honor/arena/emblem gear, built once from the loaded creature data.
std::vector<VendorSpawn> const& GetVendorSpawns()
{
    static std::vector<VendorSpawn> const spawns = []
    {
        std::vector<VendorSpawn> result;
        std::unordered_map<uint32, bool> eligible;
        for (auto const& pair : sObjectMgr->GetAllCreatureData())
        {
            CreatureData const& data = pair.second;
            auto it = eligible.find(data.id);
            if (it == eligible.end())
                it = eligible.emplace(data.id, VendorSellsCurrencyGear(data.id)).first;

            if (it->second)
                result.push_back({data.id, data.mapid, data.posX, data.posY, data.posZ});
        }

        return result;
    }();

    return spawns;
}

// Item Y: spawns of flight masters, built once from the loaded creature data.
struct FlightSpawn
{
    uint32 entry;
    uint32 map;
    float x;
    float y;
    float z;
};

std::vector<FlightSpawn> const& GetFlightSpawns()
{
    static std::vector<FlightSpawn> const spawns = []
    {
        std::vector<FlightSpawn> result;
        std::unordered_map<uint32, bool> isFlightMaster;
        for (auto const& pair : sObjectMgr->GetAllCreatureData())
        {
            CreatureData const& data = pair.second;
            auto it = isFlightMaster.find(data.id);
            if (it == isFlightMaster.end())
            {
                CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(data.id);
                it = isFlightMaster.emplace(data.id, tmpl && (tmpl->npcflag & UNIT_NPC_FLAG_FLIGHTMASTER)).first;
            }

            if (it->second)
                result.push_back({data.id, data.mapid, data.posX, data.posY, data.posZ});
        }

        return result;
    }();

    return spawns;
}

// Item Y: total gold cost of a taxi route; false when a leg does not exist.
bool TaxiRouteCost(std::vector<uint32> const& nodes, uint32& cost)
{
    cost = 0;
    if (nodes.size() < 2)
        return false;

    for (size_t i = 1; i < nodes.size(); ++i)
    {
        uint32 path = 0;
        uint32 legCost = 0;
        sObjectMgr->GetTaxiPath(nodes[i - 1], nodes[i], path, legCost);
        if (!path)
            return false;

        cost += legCost;
    }

    return true;
}

bool VendorFriendly(Player* bot, uint32 entry)
{
    CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(entry);
    if (!tmpl)
        return false;

    FactionTemplateEntry const* ft = sFactionTemplateStore.LookupEntry(tmpl->faction);
    FactionTemplateEntry const* botFt = bot->GetFactionTemplateEntry();
    if (!ft || !botFt)
        return false;

    return Unit::GetFactionReactionTo(ft, botFt) > REP_UNFRIENDLY;
}
}  // namespace

bool BuyExtendedCostItemAction::CanShop() const
{
    if (!bot->IsAlive() || bot->IsInCombat() || bot->InBattleground() || bot->InArena() || bot->InBattlegroundQueue())
        return false;

    if (bot->IsInFlight() || bot->IsBeingTeleported() || bot->GetTransport() || bot->GetVehicle())
        return false;

    // Instance map (dungeon, raid): never while a group runs it.
    Map* map = bot->GetMap();
    if (!map || map->Instanceable())
        return false;

    Group* group = bot->GetGroup();
    if (group && (group->isRaidGroup() || group->isBGGroup()))
        return false;

    // Bot following a real player: do not wander off.
    Player* master = botAI->GetMaster();
    if (master && master != bot && !GET_PLAYERBOT_AI(master))
        return false;

    return true;
}

bool BuyExtendedCostItemAction::isUseful()
{
    if (!CanShop())
        return false;

    if (travelling)
        return true;

    uint32 now = getMSTime();
    if (int32(now - nextCheck) < 0)
        return false;

    if (now - windowStart > kWindowMs)
    {
        windowStart = now;
        windowPurchases = 0;
    }

    if (windowPurchases >= kMaxPurchasesPerWindow)
        return false;

    // Cheap pre-check: does the bot hold any of the currencies?
    if (bot->GetHonorPoints() || bot->GetArenaPoints())
        return true;

    for (uint32 emblem : kEmblemIds)
    {
        if (bot->GetItemCount(emblem, false))
            return true;
    }

    return false;
}

bool BuyExtendedCostItemAction::FindBest(VendorItemData const* items, Choice& best)
{
    if (!items)
        return false;

    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);
    if (sRandomPlayerbotMgr.IsSpecPvp(bot->GetGUID().GetCounter(), bot->getClass()))
        calculator.SetPvpSpec(true);

    bool found = false;
    for (uint32 slot = 0; slot < items->GetItemCount(); ++slot)
    {
        VendorItem const* vi = items->GetItem(slot);
        if (!vi || !vi->ExtendedCost)
            continue;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(vi->item);
        if (!IsGearProto(proto) || proto->RequiredLevel > bot->GetLevel())
            continue;

        ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(vi->ExtendedCost);
        if (!IsCurrencyCost(iece))
            continue;

        // Affordability: same checks as Player::BuyItemFromVendorSlot.
        if (bot->GetHonorPoints() < iece->reqhonorpoints || bot->GetArenaPoints() < iece->reqarenapoints)
            continue;

        bool hasItems = true;
        for (uint8 i = 0; i < MAX_ITEM_EXTENDED_COST_REQUIREMENTS; ++i)
        {
            if (iece->reqitem[i] && !bot->HasItemCount(iece->reqitem[i], iece->reqitemcount[i]))
            {
                hasItems = false;
                break;
            }
        }

        if (!hasItems || bot->GetMaxPersonalArenaRatingRequirement(iece->reqarenaslot) < iece->reqpersonalarenarating)
            continue;

        if (vi->IsGoldRequired(proto) && proto->BuyPrice > 0 && !bot->HasEnoughMoney(proto->BuyPrice))
            continue;

        if (proto->RequiredReputationFaction &&
            uint32(bot->GetReputationRank(proto->RequiredReputationFaction)) < proto->RequiredReputationRank)
            continue;

        if (bot->GetItemCount(proto->ItemId, true) > 0 || bot->CanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        // Only buy what improves the equipped gear (same criterion as the bot item usage logic).
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", proto->ItemId);
        if (usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_REPLACE)
            continue;

        float score = calculator.CalculateItem(proto->ItemId);
        if (score <= 0.0f)
            continue;

        if (!found || score > best.score)
        {
            best.slot = slot;
            best.itemId = proto->ItemId;
            best.score = score;
            found = true;
        }
    }

    return found;
}

bool BuyExtendedCostItemAction::TryBuyFromNearbyVendor()
{
    GuidVector vendors = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const& guid : vendors)
    {
        Creature* creature = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_VENDOR);
        if (!creature || (travelling && creature->GetEntry() != vendorEntry))
            continue;

        if (!VendorSellsCurrencyGear(creature->GetEntry()))
            continue;

        Choice best;
        if (!FindBest(creature->GetVendorItems(), best))
            continue;

        uint32 oldCount = bot->GetItemCount(best.itemId, true);
        bot->BuyItemFromVendorSlot(guid, best.slot, best.itemId, 1, NULL_BAG, NULL_SLOT);
        if (bot->GetItemCount(best.itemId, true) <= oldCount)
            continue;

        ++windowPurchases;
        nextCheck = getMSTime() + kAfterBuyIntervalMs;
        botAI->DoSpecificAction("equip upgrades packet action", Event(), true);
        return true;
    }

    return false;
}

void BuyExtendedCostItemAction::ResetTravel()
{
    travelling = false;
    toFlightMaster = false;
    flying = false;
    arrivalTries = 0;
}

// Item Y: walk to the origin flight master and take the taxi (fare paid in gold by the core).
bool BuyExtendedCostItemAction::TravelToFlightMaster()
{
    uint32 now = getMSTime();
    if (now - travelStart > kTravelTimeoutMs || bot->GetMapId() != fmMap)
    {
        ResetTravel();
        nextCheck = now + kCheckIntervalMs;
        return false;
    }

    if (bot->GetExactDist(fmX, fmY, fmZ) > kArriveDistance)
        return MoveTo(fmMap, fmX, fmY, fmZ, false, false, true);

    Creature* master = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const& guid : npcs)
    {
        Creature* creature = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
        if (creature && creature->GetEntry() == fmEntry)
        {
            master = creature;
            break;
        }
    }

    if (!master)
    {
        // Close to the spawn but not in interaction range: step up to the flight master itself.
        if (++arrivalTries <= 3)
        {
            for (ObjectGuid const& guid : npcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit && unit->GetEntry() == fmEntry)
                    return MoveTo(unit, 2.5f);
            }
        }

        ResetTravel();
        nextCheck = now + kCheckIntervalMs;
        return false;
    }

    uint32 cost = 0;
    if (!TaxiRouteCost(taxiNodes, cost) || bot->GetMoney() < cost)
    {
        ResetTravel();
        nextCheck = now + kCheckIntervalMs;
        return false;
    }

    botAI->RemoveShapeshift();
    if (bot->IsMounted())
        bot->Dismount();

    if (!bot->ActivateTaxiPathTo(taxiNodes, master, 0))
    {
        ResetTravel();
        nextCheck = now + kCheckIntervalMs;
        return false;
    }

    // In the air: the vendor leg continues after landing (CanShop blocks the action while in flight).
    toFlightMaster = false;
    flying = true;
    arrivalTries = 0;
    travelStart = now;
    return true;
}

// Item Y: no buyable vendor on this map. Look for one on another map reachable by taxi, using only
// flight nodes the bot already discovered (origin and destination) and a fare it can pay.
bool BuyExtendedCostItemAction::TryPlanTaxiTrip(uint32 now)
{
    uint32 curMap = bot->GetMapId();
    TeamId team = bot->GetTeamId();

    // Origin flight masters on this map: friendly, node known, nearest first.
    struct Origin
    {
        FlightSpawn const* spawn;
        uint32 node;
        float dist;
    };

    std::vector<Origin> origins;
    for (FlightSpawn const& fm : GetFlightSpawns())
    {
        if (fm.map != curMap || !VendorFriendly(bot, fm.entry))
            continue;

        float dist = bot->GetExactDist(fm.x, fm.y, fm.z);
        if (dist > kMaxFlightMasterWalk)
            continue;

        uint32 node = sObjectMgr->GetNearestTaxiNode(fm.x, fm.y, fm.z, fm.map, team);
        if (!node || !bot->m_taxi.IsTaximaskNodeKnown(node))
            continue;

        origins.push_back({&fm, node, dist});
    }

    if (origins.empty())
        return false;

    std::sort(origins.begin(), origins.end(), [](Origin const& a, Origin const& b) { return a.dist < b.dist; });

    for (uint32 destMap : kTaxiMaps)
    {
        if (destMap == curMap)
            continue;

        // Vendors on the destination map; nearest to a known flight node first is decided per vendor below.
        std::vector<VendorSpawn const*> vendors;
        for (VendorSpawn const& spawn : GetVendorSpawns())
        {
            if (spawn.map == destMap)
                vendors.push_back(&spawn);
        }

        if (vendors.empty())
            continue;

        std::vector<uint32> evaluated;
        for (VendorSpawn const* vendor : vendors)
        {
            uint32 entry = vendor->entry;
            if (std::find(evaluated.begin(), evaluated.end(), entry) != evaluated.end())
                continue;

            if (evaluated.size() >= kMaxVendorsEvaluated)
                break;

            evaluated.push_back(entry);
            if (!VendorFriendly(bot, entry))
                continue;

            Choice best;
            if (!FindBest(sObjectMgr->GetNpcVendorItemList(entry), best))
                continue;

            // Flight master nearest to this vendor on the destination map; its node must be known.
            FlightSpawn const* destFm = nullptr;
            float destDist = 0.0f;
            for (FlightSpawn const& fm : GetFlightSpawns())
            {
                if (fm.map != destMap || !VendorFriendly(bot, fm.entry))
                    continue;

                float d = std::sqrt((fm.x - vendor->x) * (fm.x - vendor->x) + (fm.y - vendor->y) * (fm.y - vendor->y));
                if (!destFm || d < destDist)
                {
                    destFm = &fm;
                    destDist = d;
                }
            }

            if (!destFm)
                continue;

            uint32 destNode = sObjectMgr->GetNearestTaxiNode(destFm->x, destFm->y, destFm->z, destMap, team);
            if (!destNode || !bot->m_taxi.IsTaximaskNodeKnown(destNode))
                continue;

            for (Origin const& origin : origins)
            {
                std::vector<uint32> route = sTravelNodeMap.FindTaxiPath(origin.node, destNode);
                uint32 cost = 0;
                if (route.empty() || !TaxiRouteCost(route, cost) || bot->GetMoney() < cost)
                    continue;

                travelling = true;
                toFlightMaster = true;
                flying = false;
                arrivalTries = 0;
                travelStart = now;
                fmEntry = origin.spawn->entry;
                fmMap = origin.spawn->map;
                fmX = origin.spawn->x;
                fmY = origin.spawn->y;
                fmZ = origin.spawn->z;
                taxiNodes = std::move(route);
                vendorEntry = entry;
                vendorMap = vendor->map;
                vendorX = vendor->x;
                vendorY = vendor->y;
                vendorZ = vendor->z;
                return MoveTo(fmMap, fmX, fmY, fmZ, false, false, true);
            }
        }
    }

    return false;
}

bool BuyExtendedCostItemAction::TravelToVendor()
{
    if (toFlightMaster)
        return TravelToFlightMaster();

    uint32 now = getMSTime();
    if (flying)
    {
        // Landed: the walking timeout starts now, not at take-off.
        flying = false;
        travelStart = now;
    }

    if (now - travelStart > kTravelTimeoutMs || bot->GetMapId() != vendorMap)
    {
        ResetTravel();
        nextCheck = now + kCheckIntervalMs;
        return false;
    }

    if (bot->GetExactDist(vendorX, vendorY, vendorZ) > kArriveDistance)
        return MoveTo(vendorMap, vendorX, vendorY, vendorZ, false, false, true);

    if (TryBuyFromNearbyVendor())
    {
        ResetTravel();
        return true;
    }

    // Close to the spawn but not in interaction range: step up to the vendor itself.
    if (++arrivalTries <= 3)
    {
        GuidVector vendors = AI_VALUE(GuidVector, "nearest npcs");
        for (ObjectGuid const& guid : vendors)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == vendorEntry)
                return MoveTo(unit, 2.5f);
        }
    }

    ResetTravel();
    nextCheck = now + kCheckIntervalMs;
    return false;
}

bool BuyExtendedCostItemAction::Execute(Event /*event*/)
{
    if (travelling)
        return TravelToVendor();

    uint32 now = getMSTime();
    nextCheck = now + kCheckIntervalMs;

    if (TryBuyFromNearbyVendor())
        return true;

    // Nearest vendors (same map) that sell currency gear, nearest first; evaluate a few distinct entries.
    struct Candidate
    {
        VendorSpawn const* spawn;
        float dist;
    };

    std::vector<Candidate> candidates;
    uint32 mapId = bot->GetMapId();
    for (VendorSpawn const& spawn : GetVendorSpawns())
    {
        if (spawn.map != mapId)
            continue;

        candidates.push_back({&spawn, bot->GetExactDist(spawn.x, spawn.y, spawn.z)});
    }

    std::sort(candidates.begin(), candidates.end(),
              [](Candidate const& a, Candidate const& b) { return a.dist < b.dist; });

    std::vector<uint32> evaluated;
    for (Candidate const& candidate : candidates)
    {
        uint32 entry = candidate.spawn->entry;
        if (std::find(evaluated.begin(), evaluated.end(), entry) != evaluated.end())
            continue;

        if (evaluated.size() >= kMaxVendorsEvaluated)
            break;

        evaluated.push_back(entry);
        if (!VendorFriendly(bot, entry))
            continue;

        Choice best;
        if (!FindBest(sObjectMgr->GetNpcVendorItemList(entry), best))
            continue;

        travelling = true;
        toFlightMaster = false;
        flying = false;
        arrivalTries = 0;
        travelStart = now;
        vendorEntry = entry;
        vendorMap = candidate.spawn->map;
        vendorX = candidate.spawn->x;
        vendorY = candidate.spawn->y;
        vendorZ = candidate.spawn->z;
        return MoveTo(vendorMap, vendorX, vendorY, vendorZ, false, false, true);
    }

    // Item Y: nothing buyable on this map; try a vendor on another map reachable by taxi.
    return TryPlanTaxiTrip(now);
}
