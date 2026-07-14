/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#ifndef PLAYERBOTS_PALADINBLESSINGACTION_H
#define PLAYERBOTS_PALADINBLESSINGACTION_H

#include "GenericSpellActions.h"
#include "PaladinBlessingHelper.h"
#include "PaladinBlessingPlan.h"
#include "PaladinBlessingTypes.h"
#include <cstddef>
#include <unordered_map>
#include <vector>

class Event;
class Player;
class PlayerbotAI;
class Unit;
class UntypedValue;

namespace ai::blessing
{
    // Builds this bot's single next blessing cast for the tick from shared ground truth.
    // No outward side effects (queries only)
    class PaladinBlessingPlanner
    {
    public:
        PaladinBlessingPlanner(PlayerbotAI* botAI);

        PendingBlessing Plan();

    private:
        struct ClassGroup
        {
            uint8 cls = 0;
            std::vector<Unit*> members;
        };

        void ProcessMember(
            Player* m, BlessingConstraints& constraints,
            std::unordered_map<uint8, ClassGroup>& classGroups, bool& renewedHopePresent,
            bool& recentLogin);

        PendingBlessing SelectCast(
            BlessingConstraints const& constraints,
            std::unordered_map<uint8, ClassGroup> const& classGroups, bool renewedHopePresent);

        ai::blessing::BaseBlessingCategory PlanClass(
            ClassGroup const& group,
            std::vector<PaladinFlags> const& bots, std::size_t selfIdx,
            BlessingConstraints const& constraints, bool renewedHopePresent,
            std::vector<ai::blessing::BaseBlessingCategory>& myCats);

        PendingBlessing SelectGreater(
            std::unordered_map<uint8, ClassGroup> const& groups,
            std::unordered_map<uint8, ai::blessing::BaseBlessingCategory> const& classMajority,
            std::vector<ai::blessing::BaseBlessingCategory> const& myCats);
        PendingBlessing SelectSingle(
            std::unordered_map<uint8, ClassGroup> const& groups,
            std::unordered_map<uint8, ai::blessing::BaseBlessingCategory> const& classMajority,
            std::vector<ai::blessing::BaseBlessingCategory> const& myCats);

        // A valid cast target: alive, assist-valid, in range, and castable now.
        bool CanBlessNow(Unit* m, uint32 spellId) const;

        PlayerbotAI* botAI;
        Player* bot;
        std::unordered_map<ObjectGuid, std::vector<MemberBlessing>> _coverage;
        std::unordered_map<ObjectGuid, ai::blessing::BaseBlessingCategory> _intended;
        std::unordered_map<ObjectGuid, bool> _humanCaster;
    };

    UntypedValue* blessing_to_cast_value(PlayerbotAI* botAI);
}

class CastBlessingAction : public CastBuffSpellAction
{
public:
    CastBlessingAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cast blessing") {}

    Unit* GetTarget() override;
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

#endif
