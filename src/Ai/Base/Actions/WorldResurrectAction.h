/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WORLDRESURRECTACTION_H
#define PLAYERBOTS_WORLDRESURRECTACTION_H

#include "GenericSpellActions.h"
#include "ReachTargetActions.h"

class WorldResurrectAction : public CastSpellAction
{
public:
    WorldResurrectAction(PlayerbotAI* botAI);

    std::string const getName() override { return "world resurrect"; }
    std::string const GetTargetName() override { return "world resurrect target"; }
    Unit* GetTarget() override;
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
    std::vector<NextAction> getPrerequisites() override;
};

class ReachWorldResurrectTargetAction : public ReachTargetAction
{
public:
    ReachWorldResurrectTargetAction(PlayerbotAI* botAI);

    std::string const GetTargetName() override { return "world resurrect target"; }
    Unit* GetTarget() override;
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    bool UpdateRange();
};

#endif
