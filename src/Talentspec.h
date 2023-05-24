/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TALENTSPEC_H
#define _PLAYERBOT_TALENTSPEC_H

#include "Action.h"

struct TalentEntry;
struct TalentTabEntry;

#define SORT_BY_DEFAULT 0
#define SORT_BY_POINTS_TREE 1
#define ABSOLUTE_DIST 0
#define SUBSTRACT_OLD_NEW 1
#define SUBSTRACT_NEW_OLD -1
#define ADDED_POINTS 2
#define REMOVED_POINTS -2

class TalentSpec
{
    public:
        struct TalentListEntry
        {
            uint32 entry;
            uint32 rank;
            uint32 maxRank;
            TalentEntry const* talentInfo;
            TalentTabEntry const* talentTabInfo;
            uint32 tabPage() const;
        };

        TalentSpec() { };
        virtual ~TalentSpec() { }
        TalentSpec(uint32 classMask);
        TalentSpec(TalentSpec* base, std::string const link);
        TalentSpec(Player* bot);
        TalentSpec(Player* bot, std::string const link);

        uint32 points = 0;
        std::vector<TalentListEntry> talents;

        bool CheckTalentLink(std::string const link, std::ostringstream* out);
        virtual bool CheckTalents(uint32 maxPoints, std::ostringstream* out);
        void CropTalents(uint32 level);
        void ShiftTalents(TalentSpec* oldTalents, uint32 level);
        void ApplyTalents(Player* bot, std::ostringstream* out);

        uint32 GetTalentPoints(std::vector<TalentListEntry>& talents, int32 tabpage = -1);
        uint32 GetTalentPoints(int32 tabpage = -1);
        bool isEarlierVersionOf(TalentSpec& newSpec);

        std::string const GetTalentLink();
        uint32 highestTree();
        std::string const FormatSpec(Player* bot);

    protected:
        uint32 LeveltoPoints(uint32 level) const;
        uint32 PointstoLevel(uint32 points) const;
        void GetTalents(uint32 classMask);
        void SortTalents(std::vector<TalentListEntry>& talents, uint32 sortBy);
        void SortTalents(uint32 sortBy);

        void ReadTalents(Player* bot);
        void ReadTalents(std::string const link);

        std::vector<TalentListEntry> GetTalentTree(uint32 tabpage);
        std::vector<TalentListEntry> SubTalentList(std::vector<TalentListEntry>& oldList, std::vector<TalentListEntry>& newList, uint32 reverse);
};

class TalentPath
{
    public:
        TalentPath(uint32 pathId, std::string const pathName, uint32 pathProbability)
        {
            id = pathId;
            name = pathName;
            probability = pathProbability;
        };

        uint32 id = 0;
        std::string name = "";
        uint32 probability = 100;
        std::vector<TalentSpec> talentSpec;
};

class ClassSpecs
{
    public:
        ClassSpecs() { };
        ClassSpecs(uint32 specsClassMask)
        {
            classMask = specsClassMask;
            baseSpec = TalentSpec(specsClassMask);
        }

        uint32 classMask = 0;
        TalentSpec baseSpec;

        std::vector<TalentPath> talentPath;
};

#endif
