/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChangeTalentsAction.h"
#include "ChatHelper.h"
#include "Event.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"

bool ChangeTalentsAction::Execute(Event event)
{
    std::string param = event.getParam();

    std::ostringstream out;

    TalentSpec botSpec(bot);
    
    if (!param.empty())
    {
        // if (param.find("auto") != std::string::npos)
        // {
        //     AutoSelectTalents(&out);
        // }
        // else  if (param.find("list ") != std::string::npos)
        // {
        //     listPremadePaths(getPremadePaths(param.substr(5)), &out);
        // }
        // else  if (param.find("list") != std::string::npos)
        // {
        //     listPremadePaths(getPremadePaths(""), &out);
        if (param == "1") {
            bot->ActivateSpec(0);
        } else if (param == "2") {
            bot->ActivateSpec(1);
        }
        // else
        // {
        //     bool crop = false;
        //     bool shift = false;
        //     if (param.find("do ") != std::string::npos)
        //     {
        //         crop = true;
        //         param = param.substr(3);
        //     }
        //     else if (param.find("shift ") != std::string::npos)
        //     {
        //         shift = true;
        //         param = param.substr(6);
        //     }

        //     out << "Apply talents [" << param << "] ";
        //     if (botSpec.CheckTalentLink(param, &out))
        //     {
        //         TalentSpec newSpec(bot, param);
        //         std::string const specLink = newSpec.GetTalentLink();

        //         if (crop)
        //         {
        //             newSpec.CropTalents(bot->getLevel());
        //             out << "becomes: " << newSpec.GetTalentLink();
        //         }

        //         if (shift)
        //         {
        //             TalentSpec botSpec(bot);
        //             newSpec.ShiftTalents(&botSpec, bot->getLevel());
        //             out << "becomes: " << newSpec.GetTalentLink();
        //         }

        //         if (newSpec.CheckTalents(bot->getLevel(), &out))
        //         {
        //             newSpec.ApplyTalents(bot, &out);
        //             sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", 0);
        //             sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specLink", 1, specLink);
        //         }
        //     }
        //     else
        //     {
        //         std::vector<TalentPath*> paths = getPremadePaths(param);
        //         if (paths.size() > 0)
        //         {
        //             out.str("");
        //             out.clear();

        //             if (paths.size() > 1 && false/*!sPlayerbotAIConfig->autoPickTalents*/)
        //             {
        //                 out << "Found multiple specs: ";
        //                 listPremadePaths(paths, &out);
        //             }
        //             else
        //             {
        //                 if (paths.size() > 1)
        //                     out << "Found " << paths.size() << " possible specs to choose from. ";

        //                 TalentPath* path = PickPremadePath(paths, sRandomPlayerbotMgr->IsRandomBot(bot));
        //                 TalentSpec newSpec = *GetBestPremadeSpec(path->id);
        //                 std::string const specLink = newSpec.GetTalentLink();
        //                 newSpec.CropTalents(bot->getLevel());
        //                 newSpec.ApplyTalents(bot, &out);

        //                 if (newSpec.GetTalentPoints() > 0)
        //                 {
        //                     out << "Apply spec " << "|h|cffffffff" << path->name << " " << newSpec.FormatSpec(bot);
        //                     sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", path->id + 1);
        //                     sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specLink", 0);
        //                 }
        //             }
        //         }
        //     }
        // }
    }
    // else
    // {
    //     uint32 specId = sRandomPlayerbotMgr->GetValue(bot->GetGUID().GetCounter(), "specNo") - 1;
    //     std::string specName = "";
    //     TalentPath* specPath;
    //     if (specId)
    //     {
    //         specPath = getPremadePath(specId);
    //         if (specPath->id == specId)
    //             specName = specPath->name;
    //     }

    //     out << "My current talent spec is: " << "|h|cffffffff";
    //     if (specName != "")
    //         out << specName << " (" << botSpec.FormatSpec(bot) << ")";
    //     else
    //         out << chat->FormatClass(bot, botSpec.highestTree());
    //     out << " Link: ";
    //     out << botSpec.GetTalentLink();
    // }

    botAI->TellMaster(out);

    return true;
}

std::vector<TalentPath*> ChangeTalentsAction::getPremadePaths(std::string const findName)
{
    std::vector<TalentPath*> ret;
    for (auto& path : sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath)
    {
        if (findName.empty() || path.name.find(findName) != std::string::npos)
        {
            ret.push_back(&path);
        }
    }

    return std::move(ret);
}

std::vector<TalentPath*> ChangeTalentsAction::getPremadePaths(TalentSpec* oldSpec)
{
    std::vector<TalentPath*> ret;

    for (auto& path : sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath)
    {
        TalentSpec newSpec = *GetBestPremadeSpec(path.id);
        newSpec.CropTalents(bot->getLevel());
        if (oldSpec->isEarlierVersionOf(newSpec))
        {
            ret.push_back(&path);
        }
    }

    return std::move(ret);
}

TalentPath* ChangeTalentsAction::getPremadePath(uint32 id)
{
    for (auto& path : sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath)
    {
        if (id == path.id)
        {
            return &path;
        }
    }

    return &sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath[0];
}

void ChangeTalentsAction::listPremadePaths(std::vector<TalentPath*> paths, std::ostringstream* out)
{
    if (paths.size() == 0)
    {
        *out << "No predefined talents found..";
    }

    *out << "|h|cffffffff";

    for (auto path : paths)
    {
        *out << path->name << " (" << path->talentSpec.back().FormatSpec(bot) << "), ";
    }

    out->seekp(-2, out->cur);
    *out << ".";
}

TalentPath* ChangeTalentsAction::PickPremadePath(std::vector<TalentPath*> paths, bool useProbability)
{
    uint32 totProbability = 0;
    uint32 curProbability = 0;

    if (paths.size() == 1)
        return paths[0];

    for (auto path : paths)
    {
        totProbability += useProbability ? path->probability : 1;
    }

    totProbability = urand(0, totProbability);

    for (auto path : paths)
    {
        curProbability += (useProbability ? path->probability : 1);
        if (curProbability >= totProbability)
            return path;
    }

    return paths[0];
}

bool ChangeTalentsAction::AutoSelectTalents(std::ostringstream* out)
{
    // Does the bot have talentpoints?
    if (bot->getLevel() < 10)
    {
        *out << "No free talent points.";
        return false;
    }

    uint32 specNo = sRandomPlayerbotMgr->GetValue(bot->GetGUID().GetCounter(), "specNo");
    uint32 specId = specNo - 1;
    std::string specLink = sRandomPlayerbotMgr->GetData(bot->GetGUID().GetCounter(), "specLink");

    //Continue the current spec
    if (specNo > 0)
    {
        TalentSpec newSpec = *GetBestPremadeSpec(specId);
        newSpec.CropTalents(bot->getLevel());
        newSpec.ApplyTalents(bot, out);
        if (newSpec.GetTalentPoints() > 0)
        {
            *out << "Upgrading spec " << "|h|cffffffff" << getPremadePath(specId)->name << "" << newSpec.FormatSpec(bot);
        }
    }
    else if (!specLink.empty())
    {
        TalentSpec newSpec(bot, specLink);
        newSpec.CropTalents(bot->getLevel());
        newSpec.ApplyTalents(bot, out);
        if (newSpec.GetTalentPoints() > 0)
        {
            *out << "Upgrading saved spec "
                 << "|h|cffffffff" << chat->FormatClass(bot, newSpec.highestTree()) << " (" << newSpec.FormatSpec(bot) << ")";
        }
    }

    //Spec was not found or not sufficient
    if (bot->GetFreeTalentPoints() > 0 || (!specNo && specLink.empty()))
    {
        TalentSpec oldSpec(bot);
        std::vector<TalentPath*> paths = getPremadePaths(&oldSpec);

        if (paths.size() == 0) //No spec like the old one found. Pick any.
        {
            if (bot->CalculateTalentsPoints() > 0)
                *out << "No specs like the current spec found. ";

            paths = getPremadePaths("");
        }

        if (paths.size() == 0)
        {
            *out << "No predefined talents found for this class.";
            specId = -1;
            // specLink = "";
        }
        else if (paths.size() > 1 && false/*!sPlayerbotAIConfig->autoPickTalents*/ && !sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            *out << "Found multiple specs: ";
            listPremadePaths(paths, out);
        }
        else
        {
            specId = PickPremadePath(paths, sRandomPlayerbotMgr->IsRandomBot(bot))->id;
            TalentSpec newSpec = *GetBestPremadeSpec(specId);
            specLink = newSpec.GetTalentLink();
            newSpec.CropTalents(bot->getLevel());
            newSpec.ApplyTalents(bot, out);

            if (paths.size() > 1)
                *out << "Found " << paths.size() << " possible specs to choose from. ";

            *out << "Apply spec " << "|h|cffffffff" << getPremadePath(specId)->name << " " << newSpec.FormatSpec(bot);
        }
    }

    sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", specId + 1);

    if (!specLink.empty() && specId == -1)
        sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specLink", 1, specLink);
    else
        sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specLink", 0);

    return (specNo == 0) ? false : true;
}

//Returns a pre-made talentspec that best suits the bots current talents.
TalentSpec* ChangeTalentsAction::GetBestPremadeSpec(uint32 specId)
{
    TalentPath* path = getPremadePath(specId);
    for (auto& spec : path->talentSpec)
    {
        if (spec.points >= bot->CalculateTalentsPoints())
            return &spec;
    }

    if (path->talentSpec.size())
        return &path->talentSpec.back();

    return &sPlayerbotAIConfig->classSpecs[bot->getClassMask()].baseSpec;
}

bool AutoSetTalentsAction::Execute(Event event)
{
    std::ostringstream out;

    if (!sPlayerbotAIConfig->autoPickTalents || !sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if (bot->GetFreeTalentPoints() <= 0)
        return false;

    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitTalentsTree(true, true, true);
    factory.InitPetTalents();
    botAI->TellMaster(out);

    return true;
}
