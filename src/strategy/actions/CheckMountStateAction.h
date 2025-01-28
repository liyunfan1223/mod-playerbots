/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHECKMOUNTSTATEACTION_H
#define _PLAYERBOT_CHECKMOUNTSTATEACTION_H

#include "UseItemAction.h"

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
    ShapeshiftForm masterInShapeshiftForm;
    float CalculateDismountDistance() const;
    float CalculateMountDistance() const;
    void Dismount();
    bool ShouldFollowMasterMountState(Player* master, bool noAttackers, bool shouldMount) const;
    bool ShouldDismountForMaster(Player* master) const;
    int32 CalculateMasterMountSpeed(Player* master) const;
    bool CheckForSwiftMount() const;
    std::map<uint32, std::map<int32, std::vector<uint32>>> GetAllMountSpells() const;
    bool TryPreferredMount(Player* master) const;
    uint32 GetMountType(Player* master) const;
    void FilterMountsBySpeed(std::map<int32, std::vector<uint32>>& spells, int32 masterSpeed) const;
    bool TryRandomMount(const std::map<int32, std::vector<uint32>>& spells) const;
};

#endif
