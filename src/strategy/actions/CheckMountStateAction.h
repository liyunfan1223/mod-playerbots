/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHECKMOUNTSTATEACTION_H
#define _PLAYERBOT_CHECKMOUNTSTATEACTION_H

#include <unordered_map>
#include <vector>

#include "UseItemAction.h"

const uint16 SPELL_TRAVEL_FORM = 783;
const uint16 SPELL_FLIGHT_FORM = 33943;
const uint16 SPELL_SWIFT_FLIGHT_FORM = 40120;

struct MountData
{
    bool swiftMount = false;
    // Outer map: index (0 for ground, 1 for flight), inner map: effect speed -> vector of spell IDs.
    std::map<uint32, std::map<int32, std::vector<uint32>>> allSpells;
    // Default mount speed.
    int32 maxSpeed = 59;
};

struct PreferredMountCache
{
    std::vector<uint32> groundMounts;
    std::vector<uint32> flightMounts;
};

class PlayerbotAI;

class CheckMountStateAction : public UseItemAction
{
public:
    CheckMountStateAction(PlayerbotAI* botAI) : UseItemAction(botAI, "check mount state", true) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override { return true; }
    bool Mount();

private:
    Player* master;
    ShapeshiftForm masterInShapeshiftForm;
    ShapeshiftForm botInShapeshiftForm;
    static std::unordered_map<uint32, PreferredMountCache> mountCache;
    static bool preferredMountTableChecked;
    float CalculateDismountDistance() const;
    float CalculateMountDistance() const;
    void Dismount();
    bool ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const;
    bool ShouldDismountForMaster(Player* master) const;
    int32 CalculateMasterMountSpeed(Player* master, const MountData& mountData) const;
    bool CheckForSwiftMount() const;
    std::map<uint32, std::map<int32, std::vector<uint32>>> GetAllMountSpells() const;
    bool TryForms(Player* master, int32 masterMountType, int32 masterSpeed) const;
    bool TryPreferredMount(Player* master) const;
    uint32 GetMountType(Player* master) const;
    bool TryRandomMountFiltered(const std::map<int32, std::vector<uint32>>& spells, int32 masterSpeed) const;
};

#endif
