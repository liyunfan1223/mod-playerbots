/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChangeTalentsAction.h"

#include "AiFactory.h"
#include "ChatHelper.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "AiObjectContext.h"
#include "Log.h"

bool ChangeTalentsAction::Execute(Event event)
{
    auto* flag = botAI->GetAiObjectContext()->GetValue<bool>("custom_glyphs"); // Added for custom Glyphs

    if (flag->Get()) // Added for custom Glyphs
    {
        flag->Set(false);
        LOG_INFO("playerbots", "Custom Glyph Flag set to OFF");
    }
    std::string param = event.getParam();

    std::ostringstream out;

    if (!param.empty())
    {
        if (param.find("help") != std::string::npos)
        {
            out << TalentsHelp();
        }
        else if (param.find("switch") != std::string::npos)
        {
            if (param.find("switch 1") != std::string::npos)
            {
                bot->ActivateSpec(0);
                out << "Active first talent";
                botAI->ResetStrategies();
            }
            else if (param.find("switch 2") != std::string::npos)
            {
                if (bot->GetSpecsCount() == 1 && bot->GetLevel() >= sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
                {
                    bot->CastSpell(bot, 63680, true, nullptr, nullptr, bot->GetGUID());
                    bot->CastSpell(bot, 63624, true, nullptr, nullptr, bot->GetGUID());
                }
                bot->ActivateSpec(1);
                out << "Active second talent";
                botAI->ResetStrategies();
            }
        }
        else if (param.find("autopick") != std::string::npos)
        {
            PlayerbotFactory factory(bot, bot->GetLevel());
            factory.InitTalentsTree(true);
            out << "Auto pick talents";
            botAI->ResetStrategies();
        }
        else if (param.find("spec list") != std::string::npos)
        {
            out << SpecList();
        }
        else if (param.find("spec ") != std::string::npos)
        {
            param = param.substr(5);
            out << SpecPick(param);
            botAI->ResetStrategies();
        }
        else if (param.find("apply ") != std::string::npos)
        {
            param = param.substr(6);
            out << SpecApply(param);
            botAI->ResetStrategies();
        }
        else
        {
            out << "Unknown command.";
        }
    }
    else
    {
        uint32 tab = AiFactory::GetPlayerSpecTab(bot);
        out << "My current talent spec is: "
            << "|h|cffffffff";
        out << chat->FormatClass(bot, tab) << "\n";
        out << TalentsHelp();
    }

    botAI->TellMaster(out);

    return true;
}

std::string ChangeTalentsAction::TalentsHelp()
{
    std::ostringstream out;
    out << "Talents usage: talents switch <1/2>, talents autopick, talents spec list, "
           "talents spec <specName>, talents apply <link>.";
    return out.str();
}

std::string ChangeTalentsAction::SpecList()
{
    int cls = bot->getClass();
    int specFound = 0;
    std::ostringstream out;
    for (int specNo = 0; specNo < MAX_SPECNO; ++specNo)
    {
        if (sPlayerbotAIConfig->premadeSpecName[cls][specNo].size() == 0)
        {
            break;
        }
        specFound++;
        std::ostringstream out;
        std::vector<std::vector<uint32>> parsed = sPlayerbotAIConfig->parsedSpecLinkOrder[cls][specNo][80];
        std::unordered_map<int, int> tabCount;
        tabCount[0] = tabCount[1] = tabCount[2] = 0;
        for (auto& item : parsed)
        {
            tabCount[item[0]] += item[3];
        }
        out << specFound << ". " << sPlayerbotAIConfig->premadeSpecName[cls][specNo] << " (";
        out << tabCount[0] << "-" << tabCount[1] << "-" << tabCount[2] << ")";
        botAI->TellMasterNoFacing(out.str());
    }
    out << "Total " << specFound << " specs found";
    return out.str();
}

std::string ChangeTalentsAction::SpecPick(std::string param)
{
    int cls = bot->getClass();
    // int specFound = 0; //not used, line marked for removal.
    for (int specNo = 0; specNo < MAX_SPECNO; ++specNo)
    {
        if (sPlayerbotAIConfig->premadeSpecName[cls][specNo].size() == 0)
        {
            break;
        }
        if (sPlayerbotAIConfig->premadeSpecName[cls][specNo] == param)
        {
            PlayerbotFactory::InitTalentsBySpecNo(bot, specNo, true);

            PlayerbotFactory factory(bot, bot->GetLevel());
            factory.InitGlyphs(false);

            std::ostringstream out;
            out << "Picking " << sPlayerbotAIConfig->premadeSpecName[cls][specNo];
            return out.str();
        }
    }
    std::ostringstream out;
    out << "Spec " << param << " not found";
    return out.str();
}

std::string ChangeTalentsAction::SpecApply(std::string param)
{
    int cls = bot->getClass();
    std::ostringstream out;
    std::vector<std::vector<uint32>> parsedSpecLink = PlayerbotAIConfig::ParseTempTalentsOrder(cls, param);
    if (parsedSpecLink.size() == 0)
    {
        out << "Invalid link " << param;
        return out.str();
    }
    PlayerbotFactory::InitTalentsByParsedSpecLink(bot, parsedSpecLink, true);
    out << "Applying " << param;
    return out.str();
}

// std::vector<TalentPath*> ChangeTalentsAction::getPremadePaths(std::string const findName)
// {
//     std::vector<TalentPath*> ret;
//     // for (auto& path : sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath)
//     // {
//     //     if (findName.empty() || path.name.find(findName) != std::string::npos)
//     //     {
//     //         ret.push_back(&path);
//     //     }
//     // }

//     return std::move(ret);
// }

// std::vector<TalentPath*> ChangeTalentsAction::getPremadePaths(TalentSpec* oldSpec)
// {
//     std::vector<TalentPath*> ret;

//     // for (auto& path : sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath)
//     // {
//     //     TalentSpec newSpec = *GetBestPremadeSpec(path.id);
//     //     newSpec.CropTalents(bot->GetLevel());
//     //     if (oldSpec->isEarlierVersionOf(newSpec))
//     //     {
//     //         ret.push_back(&path);
//     //     }
//     // }

//     return std::move(ret);
// }

// TalentPath* ChangeTalentsAction::getPremadePath(uint32 id)
// {
//     // for (auto& path : sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath)
//     // {
//     //     if (id == path.id)
//     //     {
//     //         return &path;
//     //     }
//     // }

//     // return &sPlayerbotAIConfig->classSpecs[bot->getClass()].talentPath[0];
//     return nullptr;
// }

// void ChangeTalentsAction::listPremadePaths(std::vector<TalentPath*> paths, std::ostringstream* out)
// {
//     if (paths.size() == 0)
//     {
//         *out << "No predefined talents found..";
//     }

//     *out << "|h|cffffffff";

//     for (auto path : paths)
//     {
//         *out << path->name << " (" << path->talentSpec.back().FormatSpec(bot) << "), ";
//     }

//     out->seekp(-2, out->cur);
//     *out << ".";
// }

// TalentPath* ChangeTalentsAction::PickPremadePath(std::vector<TalentPath*> paths, bool useProbability)
// {
//     uint32 totProbability = 0;
//     uint32 curProbability = 0;

//     if (paths.size() == 1)
//         return paths[0];

//     for (auto path : paths)
//     {
//         totProbability += useProbability ? path->probability : 1;
//     }

//     totProbability = urand(0, totProbability);

//     for (auto path : paths)
//     {
//         curProbability += (useProbability ? path->probability : 1);
//         if (curProbability >= totProbability)
//             return path;
//     }

//     return paths[0];
// }

// bool ChangeTalentsAction::AutoSelectTalents(std::ostringstream* out)
// {
//     // Does the bot have talentpoints?
//     if (bot->GetLevel() < 10)
//     {
//         *out << "No free talent points.";
//         return false;
//     }

//     uint32 specNo = sRandomPlayerbotMgr->GetValue(bot->GetGUID().GetCounter(), "specNo");
//     uint32 specId = specNo - 1;
//     std::string specLink = sRandomPlayerbotMgr->GetData(bot->GetGUID().GetCounter(), "specLink");

//     //Continue the current spec
//     if (specNo > 0)
//     {
//         TalentSpec newSpec = *GetBestPremadeSpec(specId);
//         newSpec.CropTalents(bot->GetLevel());
//         newSpec.ApplyTalents(bot, out);
//         if (newSpec.GetTalentPoints() > 0)
//         {
//             *out << "Upgrading spec " << "|h|cffffffff" << getPremadePath(specId)->name << "" <<
//             newSpec.FormatSpec(bot);
//         }
//     }
//     else if (!specLink.empty())
//     {
//         TalentSpec newSpec(bot, specLink);
//         newSpec.CropTalents(bot->GetLevel());
//         newSpec.ApplyTalents(bot, out);
//         if (newSpec.GetTalentPoints() > 0)
//         {
//             *out << "Upgrading saved spec "
//                  << "|h|cffffffff" << chat->FormatClass(bot, newSpec.highestTree()) << " (" <<
//                  newSpec.FormatSpec(bot) << ")";
//         }
//     }

//     //Spec was not found or not sufficient
//     if (bot->GetFreeTalentPoints() > 0 || (!specNo && specLink.empty()))
//     {
//         TalentSpec oldSpec(bot);
//         std::vector<TalentPath*> paths = getPremadePaths(&oldSpec);

//         if (paths.size() == 0) //No spec like the old one found. Pick any.
//         {
//             if (bot->CalculateTalentsPoints() > 0)
//                 *out << "No specs like the current spec found. ";

//             paths = getPremadePaths("");
//         }

//         if (paths.size() == 0)
//         {
//             *out << "No predefined talents found for this class.";
//             specId = -1;
//             // specLink = "";
//         }
//         else if (paths.size() > 1 && false/*!sPlayerbotAIConfig->autoPickTalents*/ &&
//         !sRandomPlayerbotMgr->IsRandomBot(bot))
//         {
//             *out << "Found multiple specs: ";
//             listPremadePaths(paths, out);
//         }
//         else
//         {
//             specId = PickPremadePath(paths, sRandomPlayerbotMgr->IsRandomBot(bot))->id;
//             TalentSpec newSpec = *GetBestPremadeSpec(specId);
//             specLink = newSpec.GetTalentLink();
//             newSpec.CropTalents(bot->GetLevel());
//             newSpec.ApplyTalents(bot, out);

//             if (paths.size() > 1)
//                 *out << "Found " << paths.size() << " possible specs to choose from. ";

//             *out << "Apply spec " << "|h|cffffffff" << getPremadePath(specId)->name << " " <<
//             newSpec.FormatSpec(bot);
//         }
//     }

//     sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specNo", specId + 1);

//     if (!specLink.empty() && specId == -1)
//         sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specLink", 1, specLink);
//     else
//         sRandomPlayerbotMgr->SetValue(bot->GetGUID().GetCounter(), "specLink", 0);

//     return (specNo == 0) ? false : true;
// }

// //Returns a pre-made talentspec that best suits the bots current talents.
// TalentSpec* ChangeTalentsAction::GetBestPremadeSpec(uint32 specId)
// {
//     TalentPath* path = getPremadePath(specId);
//     for (auto& spec : path->talentSpec)
//     {
//         if (spec.points >= bot->CalculateTalentsPoints())
//             return &spec;
//     }

//     if (path->talentSpec.size())
//         return &path->talentSpec.back();

//     // return &sPlayerbotAIConfig->classSpecs[bot->getClassMask()].baseSpec;
//     return nullptr;
// }

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
