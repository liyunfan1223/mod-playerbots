/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHANGETALENTSACTION_H
#define _PLAYERBOT_CHANGETALENTSACTION_H

#include "Action.h"
#include "Talentspec.h"

class PlayerbotAI;

class ChangeTalentsAction : public Action
{
public:
    ChangeTalentsAction(PlayerbotAI* botAI, std::string const name = "talents") : Action(botAI, name) {}

    bool Execute(Event event);
    // bool AutoSelectTalents(std::ostringstream* out);

private:
    // std::vector<TalentPath*> getPremadePaths(std::string const findName);
    // std::vector<TalentPath*> getPremadePaths(TalentSpec* oldSpec);
    // TalentPath* getPremadePath(uint32 id);
    // void listPremadePaths(std::vector<TalentPath*> paths, std::ostringstream* out);
    // TalentPath* PickPremadePath(std::vector<TalentPath*> paths, bool useProbability);
    // TalentSpec* GetBestPremadeSpec(uint32 spec);
    std::string TalentsHelp();
    std::string SpecList();
    std::string SpecPick(std::string param);
    std::string SpecApply(std::string param);
};

class AutoSetTalentsAction : public ChangeTalentsAction
{
public:
    AutoSetTalentsAction(PlayerbotAI* botAI) : ChangeTalentsAction(botAI, "auto talents") {}

    bool Execute(Event event) override;
};

#endif
