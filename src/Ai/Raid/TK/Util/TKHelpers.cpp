/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "TKKaelthasBossAI.h"
#include <limits>
#include <list>

using namespace EncounterHelpers;

namespace TkHelpers
{

// General

std::pair<Unit*, Unit*> GetTargetUnitPair(PlayerbotAI* botAI, uint32 entry)
{
    Unit* lowest = nullptr;
    Unit* highest = nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    for (ObjectGuid const targetGuid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
        {
            if (!lowest || unit->GetGUID().GetRawValue() < lowest->GetGUID().GetRawValue())
                lowest = unit;

            if (!highest || unit->GetGUID().GetRawValue() > highest->GetGUID().GetRawValue())
                highest = unit;
        }
    }

    return {lowest, highest};
}

Player* GetNearestNonTankPlayerInRadius(Player* bot, float radius)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* nearestPlayer = nullptr;
    float nearestDistance = radius;

    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || PlayerbotAI::IsTank(member))
            continue;

        float distance = bot->GetExactDist2d(member);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestPlayer = member;
        }
    }

    return nearestPlayer;
}

// Trash

// "attackers" already drops polymorphed units so no check for it is necessary.
Unit* GetCenturionCastingArcaneFlurry(PlayerbotAI* botAI)
{
    Unit* target = nullptr;

    AiObjectContext* context = botAI->GetAiObjectContext();
    for (ObjectGuid const attackerGuid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(attackerGuid);
        if (!attacker || attacker->GetEntry() != Id(TkNpcs::NPC_CRIMSON_HAND_CENTURION) ||
            !attacker->HasAura(Id(TkSpells::SPELL_ARCANE_FLURRY)))
        {
            continue;
        }

        if (!target || attacker->GetGUID() < target->GetGUID())
            target = attacker;
    }

    return target;
}

// Al'ar <Phoenix God>

std::unordered_map<uint32, bool> lastRebirthState;
std::unordered_map<uint32, bool> isAlarInPhase2;

// Entry into phase 2 is measured as the moment that Rebirth (34342) finishes casting.
bool IsAlarInPhase2(uint32 instanceId)
{
    auto const it = isAlarInPhase2.find(instanceId);
    return it != isAlarInPhase2.end() && it->second;
}

int8 GetAlarCurrentLocationIndex(Unit* alar)
{
    if (!alar)
        return LOCATION_NONE;

    static std::array const locations = {
        ALAR_LANDING_PLATFORM_0,
        ALAR_LANDING_PLATFORM_1,
        ALAR_LANDING_PLATFORM_2,
        ALAR_LANDING_PLATFORM_3,
        ALAR_POINT_QUILL_OR_DIVE,
        ALAR_POINT_MIDDLE,
    };

    float minDist = std::numeric_limits<float>::max();
    int8 locationIndex = LOCATION_NONE;
    for (int8 i = 0; i < TOTAL_ALAR_LOCATIONS; ++i)
    {
        float distToLocation = alar->GetPosition().GetExactDist2d(&locations[i]);
        if (distToLocation < minDist)
        {
            minDist = distToLocation;
            locationIndex = i;
        }
    }

    if (minDist > 0.1f)
        return LOCATION_NONE;

    return locationIndex;
}

int8 GetAlarDestinationLocationIndex(Unit* alar)
{
    if (!alar)
        return LOCATION_NONE;

    float x, y, z;
    if (!alar->GetMotionMaster()->GetDestination(x, y, z))
        return LOCATION_NONE;

    Position dest(x, y, z);

    static std::array const locations = {
        ALAR_LANDING_PLATFORM_0,
        ALAR_LANDING_PLATFORM_1,
        ALAR_LANDING_PLATFORM_2,
        ALAR_LANDING_PLATFORM_3,
        ALAR_POINT_QUILL_OR_DIVE,
        ALAR_POINT_MIDDLE,
    };

    float minDist = std::numeric_limits<float>::max();
    int8 locationIndex = LOCATION_NONE;
    for (int8 i = 0; i < TOTAL_ALAR_LOCATIONS; ++i)
    {
        float distToLocation = dest.GetExactDist2d(&locations[i]);
        if (distToLocation < minDist)
        {
            minDist = distToLocation;
            locationIndex = i;
        }
    }

    if (minDist > 0.1f)
        return LOCATION_NONE;

    return locationIndex;
}

int8 GetAlarPlatformIndex(Unit* alar)
{
    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    if (locationIndex < PLATFORM_0_IDX || locationIndex > PLATFORM_3_IDX)
        return LOCATION_NONE;

    return locationIndex;
}

// The nearest platform's index is also the ground (landing) spot beneath it.
static_assert(ALAR_GROUND_POSITIONS.size() == ALAR_LANDING_PLATFORM_POSITIONS.size());

Position const& GetClosestGroundPosition(Position const& botPos)
{
    size_t closestPlatform = 0;
    float minDistSq = botPos.GetExactDist2dSq(ALAR_LANDING_PLATFORM_POSITIONS[0]);

    for (size_t i = 1; i < ALAR_LANDING_PLATFORM_POSITIONS.size(); ++i)
    {
        float const distSq = botPos.GetExactDist2dSq(ALAR_LANDING_PLATFORM_POSITIONS[i]);
        if (distSq < minDistSq)
        {
            minDistSq = distSq;
            closestPlatform = i;
        }
    }

    return ALAR_GROUND_POSITIONS[closestPlatform];
}

// The main tank rotates between the W (where Al'ar initially lands) and NE platforms in phase 1
// and starts on Al'ar in phase 2.
bool IsFirstAlarTank(Player* bot)
{
    return PlayerbotAI::IsMainTank(bot);
}

// The first assist tank rotates between the NW and E platforms in phase 1.
bool IsSecondAlarTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

// The second assist tank is the primary ember tank.
bool IsPrimaryEmberTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

// The secondary Ember Tank is needed only during phase 2, and it is initially the first assist
// tank (i.e., the SecondAlarTank). When Al'ar melts the armor of the main tank, then the main
// tank becomes the secondary Ember tank. The two tanks swap from then on.
Player* GetSecondaryEmberTank(Player* bot)
{
    Player* mainTank = GetGroupMainTank(bot);
    Player* assistTank = GetGroupAssistTank(bot, 0);

    if (!mainTank || !assistTank)
        return nullptr;

    if (mainTank->HasAura(Id(TkSpells::SPELL_MELT_ARMOR)))
        return mainTank;

    return assistTank;
}

GuidVector FindFlamePatchGuids(Player* bot)
{
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(TkNpcs::NPC_FLAME_PATCH), ALAR_FLAME_PATCH_SEARCH_DISTANCE);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    return guids;
}

std::vector<Unit*> GetFlamePatches(PlayerbotAI* botAI)
{
    GuidVector const& guids =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("tk flame patches")->RefGet();

    std::vector<Unit*> flamePatches;
    flamePatches.reserve(guids.size());
    for (ObjectGuid const guid : guids)
    {
        Unit* flamePatch = botAI->GetUnit(guid);
        if (flamePatch && flamePatch->IsAlive())
            flamePatches.push_back(flamePatch);
    }

    return flamePatches;
}

// Void Reaver

std::unordered_map<uint32, std::vector<ArcaneOrbData>> voidReaverArcaneOrbs;

std::vector<Position> GetActiveArcaneOrbs(uint32 instanceId)
{
    std::vector<Position> activeOrbs;

    auto const it = voidReaverArcaneOrbs.find(instanceId);
    if (it == voidReaverArcaneOrbs.end())
        return activeOrbs;

    uint32 const now = getMSTime();
    for (ArcaneOrbData const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, now) <= ARCANE_ORB_DURATION_MS)
            activeOrbs.push_back(orb.destination);
    }

    return activeOrbs;
}

bool IsNearArcaneOrb(Player* bot, std::vector<Position> const& orbs, float radius)
{
    for (Position const& orb : orbs)
    {
        if (bot->GetExactDist2d(orb.GetPositionX(), orb.GetPositionY()) < radius)
            return true;
    }

    return false;
}

bool IsNearActiveArcaneOrb(Player* bot, float radius)
{
    return IsNearArcaneOrb(bot, GetActiveArcaneOrbs(bot->GetInstanceId()), radius);
}

// High Astromancer Solarian

bool HasWrathOfTheAstromancer(Player* bot)
{
    return bot->HasAura(Id(TkSpells::SPELL_WRATH_OF_THE_ASTROMANCER));
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

namespace
{

GuidVector const& GetLegendaryWeaponGuids(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<GuidVector>("tk legendary weapons")->RefGet();
}

Creature* GetLegendaryWeaponByState(PlayerbotAI* botAI, uint32 weaponEntry, bool alive)
{
    for (ObjectGuid const guid : GetLegendaryWeaponGuids(botAI))
    {
        Creature* weapon = botAI->GetCreature(guid);
        if (weapon && weapon->GetEntry() == weaponEntry && weapon->IsAlive() == alive)
            return weapon;
    }

    return nullptr;
}

} // end anonymous namespace

std::unordered_map<uint32, uint32> advisorDpsWaitTimer;

uint32 GetKaelthasTkPhase(Unit* kaelthas)
{
    if (!kaelthas)
        return PHASE_NONE;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI ? kaelAI->GetPhase() : Id(PHASE_NONE);
}

// The non-attackable unit flag covers the period in phase 1 before the advisor activates.
// The ironically named "Permanent Feign Death" is the aura that advisors have when they are
// "killed" in phase 1 until they are "resurrected" in phase 3.
bool IsAdvisorActive(Unit* advisor)
{
    return advisor && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !advisor->HasAura(Id(TkSpells::SPELL_PERMANENT_FEIGN_DEATH));
}

// (1) First priority is an assistant Warlock (real player or bot).
// (2) If no assistant Warlock, then look for any Warlock bot.
Player* GetCapernianTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->getClass() != CLASS_WARLOCK || !member->IsAlive() ||
            member->GetMapId() != TK_MAP_ID)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

bool IsCapernianTank(Player* bot)
{
    return bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot;
}

// One Hunter will start on Sanguinar in phase 3 (with melee) to apply Armor Disruption from the
// Netherstrand Longbow.
// (1) First priority is an assistant Hunter (real player or bot).
// (2) If no assistant Hunter, then look for any Hunter bot.
bool IsSanguinarDebuffHunter(Player* bot)
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* fallbackHunter = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->getClass() != CLASS_HUNTER || !member->IsAlive() ||
            member->GetMapId() != TK_MAP_ID)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member == bot;

        if (!fallbackHunter && GET_PLAYERBOT_AI(member))
            fallbackHunter = member;
    }

    return fallbackHunter == bot;
}

GuidVector FindLegendaryWeaponGuids(Player* bot)
{
    static std::vector<uint32> const weaponEntries = {
        Id(TkNpcs::NPC_STAFF_OF_DISINTEGRATION),
        Id(TkNpcs::NPC_COSMIC_INFUSER),
        Id(TkNpcs::NPC_INFINITY_BLADES),
        Id(TkNpcs::NPC_WARP_SLICER),
        Id(TkNpcs::NPC_PHASESHIFT_BULWARK),
        Id(TkNpcs::NPC_NETHERSTRAND_LONGBOW),
        Id(TkNpcs::NPC_DEVASTATION),
    };

    std::list<Creature*> weapons;
    bot->GetCreatureListWithEntryInGrid(weapons, weaponEntries, KAELTHAS_ROOM_SEARCH_DISTANCE);

    GuidVector guids;
    guids.reserve(weapons.size());
    for (Creature* weapon : weapons)
    {
        if (weapon)
            guids.push_back(weapon->GetGUID());
    }

    return guids;
}

Unit* GetLegendaryWeapon(PlayerbotAI* botAI, uint32 weaponEntry)
{
    return GetLegendaryWeaponByState(botAI, weaponEntry, true);
}

Creature* GetDeadLegendaryWeapon(PlayerbotAI* botAI, uint32 weaponEntry)
{
    return GetLegendaryWeaponByState(botAI, weaponEntry, false);
}

bool HasDeadLegendaryWeapon(PlayerbotAI* botAI)
{
    for (ObjectGuid const guid : GetLegendaryWeaponGuids(botAI))
    {
        Creature* weapon = botAI->GetCreature(guid);
        if (weapon && !weapon->IsAlive())
            return true;
    }

    return false;
}

bool IsLegendaryWeaponItem(uint32 itemId)
{
    static constexpr std::array legendaryItems = {
        TkItems::ITEM_WARP_SLICER,
        TkItems::ITEM_INFINITY_BLADE,
        TkItems::ITEM_STAFF_OF_DISINTEGRATION,
        TkItems::ITEM_PHASESHIFT_BULWARK,
        TkItems::ITEM_DEVASTATION,
        TkItems::ITEM_COSMIC_INFUSER,
        TkItems::ITEM_NETHERSTRAND_LONGBOW,
    };

    for (TkItems item : legendaryItems)
    {
        if (Id(item) == itemId)
            return true;
    }

    return false;
}

bool HasEquippableItemForSlot(Player* bot, uint8 slot)
{
    for (uint8 i = 0; i < 5; ++i)
    {
        uint8 bag = (i == 0) ? INVENTORY_SLOT_BAG_0 : (INVENTORY_SLOT_BAG_START + i - 1);
        uint8 startSlot = (bag == INVENTORY_SLOT_BAG_0) ? INVENTORY_SLOT_ITEM_START : 0;
        uint8 endSlot = (bag == INVENTORY_SLOT_BAG_0) ? uint8(INVENTORY_SLOT_ITEM_END)
            : (bot->GetBagByPos(bag) ? bot->GetBagByPos(bag)->GetBagSize() : uint8(0));

        for (uint8 bagSlot = startSlot; bagSlot < endSlot; ++bagSlot)
        {
            Item* item = bot->GetItemByPos(bag, bagSlot);
            if (!item || !item->GetTemplate())
                continue;

            uint16 dest = 0;
            if (bot->CanEquipItem(slot, dest, item, false) == EQUIP_ERR_OK)
                return true;
        }
    }

    return false;
}

Item* GetEquippedItemInSlot(Player* bot, uint8 slot, uint32 itemId)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    return item && item->GetEntry() == itemId ? item : nullptr;
}

Creature* GetNearestFlameStrikeInRadius(Player* bot, float radius)
{
    return bot->FindNearestCreature(Id(TkNpcs::NPC_FLAME_STRIKE_TRIGGER), radius);
}

Creature* GetPhoenixEgg(Player* bot)
{
    constexpr float searchRadius = 75.0f;
    return bot->FindNearestCreature(Id(TkNpcs::NPC_PHOENIX_EGG), searchRadius);
}

}
