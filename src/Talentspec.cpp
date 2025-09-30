/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Talentspec.h"

#include "Event.h"
#include "Playerbots.h"

uint32 TalentSpec::TalentListEntry::tabPage() const
{
    return talentTabInfo->TalentTabID == 41 ? 1 : talentTabInfo->tabpage;
}

// Checks a talent link on basic validity.
bool TalentSpec::CheckTalentLink(std::string const link, std::ostringstream* out)
{
    std::string const validChar = "-";
    std::string const validNums = "012345";
    uint32 nums = 0;

    for (char const& c : link)
    {
        if (validChar.find(c) == std::string::npos && validNums.find(c) == std::string::npos)
        {
            *out << "talent link is invalid. Must be in format 0-0-0";
            return false;
        }

        if (validNums.find(c) != std::string::npos)
            nums++;
    }

    if (nums == 0)
    {
        *out << "talent link is invalid. Needs atleast one number.";
        return false;
    }

    return true;
}

uint32 TalentSpec::LeveltoPoints(uint32 level) const
{
    uint32 talentPointsForLevel = level < 10 ? 0 : level - 9;
    return uint32(talentPointsForLevel * sWorld->getRate(RATE_TALENT));
}

uint32 TalentSpec::PointstoLevel(uint32 points) const
{
    return uint32(ceil(points / sWorld->getRate(RATE_TALENT))) + 9;
}

TalentSpec::TalentSpec(uint32 classMask) { GetTalents(classMask); }

TalentSpec::TalentSpec(TalentSpec* base, std::string const link)
{
    talents = base->talents;
    ReadTalents(link);
}

TalentSpec::TalentSpec(Player* bot)
{
    GetTalents(bot->getClassMask());
    ReadTalents(bot);
}

TalentSpec::TalentSpec(Player* bot, std::string const link)
{
    GetTalents(bot->getClassMask());
    ReadTalents(link);
}

// Check the talentspec for errors.
bool TalentSpec::CheckTalents(uint32 level, std::ostringstream* out)
{
    for (auto& entry : talents)
    {
        if (entry.rank > entry.maxRank)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry.talentInfo->RankID[0]);
            *out << "spec is not for this class. " << spellInfo->SpellName[0] << " has " << (entry.rank - entry.maxRank)
                 << " points above max rank.";
            return false;
        }

        if (entry.rank > 0 && entry.talentInfo->DependsOn)
        {
            if (sTalentStore.LookupEntry(entry.talentInfo->DependsOn))
            {
                bool found = false;
                SpellInfo const* spellInfodep = nullptr;

                for (auto& dep : talents)
                    if (dep.talentInfo->TalentID == entry.talentInfo->DependsOn)
                    {
                        spellInfodep = sSpellMgr->GetSpellInfo(dep.talentInfo->RankID[0]);
                        if (dep.rank >= entry.talentInfo->DependsOnRank)
                            found = true;
                    }

                if (!found)
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry.talentInfo->RankID[0]);
                    *out << "spec is invalid. Talent:" << spellInfo->SpellName[0]
                         << " needs: " << spellInfodep->SpellName[0] << " at rank: " << entry.talentInfo->DependsOnRank;
                    return false;
                }
            }
        }
    }

    for (uint8 i = 0; i < 3; i++)
    {
        std::vector<TalentListEntry> talentTree = GetTalentTree(i);
        uint32 points = 0;

        for (auto& entry : talentTree)
        {
            if (entry.rank > 0 && entry.talentInfo->Row * 5 > points)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry.talentInfo->RankID[0]);
                *out << "spec is is invalid. Talent " << spellInfo->SpellName[0] << " is selected with only " << points
                     << " in row below it.";
                return false;
            }

            points += entry.rank;
        }
    }

    if (points > LeveltoPoints(level))
    {
        *out << "spec is for a higher level. (" << PointstoLevel(points) << ")";
        return false;
    }

    return true;
}

// Set the talents for the bots to the current spec.
void TalentSpec::ApplyTalents(Player* bot, std::ostringstream* out)
{
    for (auto& entry : talents)
    {
        if (entry.rank == 0)
            continue;
        bot->LearnTalent(entry.talentInfo->TalentID, entry.rank - 1);
    }
}

// Returns a base talentlist for a class.
void TalentSpec::GetTalents(uint32 classMask)
{
    TalentListEntry entry;

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        entry.entry = i;
        entry.rank = 0;
        entry.talentInfo = talentInfo;
        entry.talentTabInfo = talentTabInfo;

        for (uint8 rank = 0; rank < MAX_TALENT_RANK; ++rank)
        {
            uint32 spellId = talentInfo->RankID[rank];
            if (!spellId)
                continue;

            entry.maxRank = rank + 1;
        }

        talents.push_back(entry);
    }

    SortTalents(talents, SORT_BY_DEFAULT);
}

// Sorts a talent list by page, row, column.
bool sortTalentMap(TalentSpec::TalentListEntry i, TalentSpec::TalentListEntry j, uint32* tabSort)
{
    uint32 itab = i.tabPage();
    uint32 jtab = j.tabPage();
    if (tabSort[itab] < tabSort[jtab])
        return true;

    if (tabSort[itab] > tabSort[jtab])
        return false;

    if (i.talentInfo->Row < j.talentInfo->Row)
        return true;

    if (i.talentInfo->Row > j.talentInfo->Row)
        return false;

    if (i.talentInfo->Col < j.talentInfo->Col)
        return true;

    return false;
}

void TalentSpec::SortTalents(uint32 sortBy) { SortTalents(talents, sortBy); }

// Sort the talents.
void TalentSpec::SortTalents(std::vector<TalentListEntry>& talents, uint32 sortBy)
{
    switch (sortBy)
    {
        case SORT_BY_DEFAULT:
        {
            uint32 tabSort[] = {0, 1, 2};
            std::sort(talents.begin(), talents.end(),
                      [&tabSort](TalentSpec::TalentListEntry i, TalentSpec::TalentListEntry j)
                      { return sortTalentMap(i, j, tabSort); });
            break;
        }
        case SORT_BY_POINTS_TREE:
        {
            uint32 tabSort[] = {GetTalentPoints(talents, 0) * 100 - urand(0, 99),
                                GetTalentPoints(talents, 1) * 100 - urand(0, 99),
                                GetTalentPoints(talents, 2) * 100 - urand(0, 99)};
            std::sort(talents.begin(), talents.end(),
                      [&tabSort](TalentSpec::TalentListEntry i, TalentSpec::TalentListEntry j)
                      { return sortTalentMap(i, j, tabSort); });
            break;
        }
    }
}

// Set the talent ranks to the current rank of the player.
void TalentSpec::ReadTalents(Player* bot)
{
    for (auto& entry : talents)
        for (uint8 rank = 0; rank < MAX_TALENT_RANK; ++rank)
        {
            uint32 spellId = entry.talentInfo->RankID[rank];
            if (!spellId)
                continue;

            if (bot->HasSpell(spellId))
            {
                entry.rank = rank + 1;
                points += 1;
            }
        }
}

// Set the talent ranks to the ranks of the link.
void TalentSpec::ReadTalents(std::string const link)
{
    //uint32 rank = 0; //not used, line marked for removal.
    uint32 pos = 0;
    uint32 tab = 0;
    std::string chr;

    if (link.substr(pos, 1) == "-")
    {
        pos++;
        tab++;
    }

    if (link.substr(pos, 1) == "-")
    {
        pos++;
        tab++;
    }

    for (auto& entry : talents)
    {
        if (entry.tabPage() == tab)
        {
            chr = link.substr(pos, 1);

            if (chr == " " || chr == "#")
                break;

            entry.rank = stoi(chr);
            points += entry.rank;

            pos++;
            if (pos <= link.size() && link.substr(pos, 1) == "-")
            {
                pos++;
                tab++;
            }

            if (pos <= link.size() && link.substr(pos, 1) == "-")
            {
                pos++;
                tab++;
            }
        }

        if (pos > link.size() - 1)
            break;
    }
}

// Returns only a specific tree from a talent list.
std::vector<TalentSpec::TalentListEntry> TalentSpec::GetTalentTree(uint32 tabpage)
{
    std::vector<TalentListEntry> retList;

    for (auto& entry : talents)
        if (entry.tabPage() == tabpage)
            retList.push_back(entry);

    return std::move(retList);
}

uint32 TalentSpec::GetTalentPoints(int32 tabpage) { return GetTalentPoints(talents, tabpage); };

// Counts the point in a talent list.
uint32 TalentSpec::GetTalentPoints(std::vector<TalentListEntry>& talents, int32 tabpage)
{
    if (tabpage == -1)
        return points;

    uint32 tPoints = 0;
    for (auto& entry : talents)
        if (entry.tabPage() == tabpage)
            tPoints = tPoints + entry.rank;

    return tPoints;
}

// Generates a wow-head link from a talent list.
std::string const TalentSpec::GetTalentLink()
{
    std::string link = "";
    std::string treeLink[3];
    uint32 points[3];
    uint32 curPoints = 0;

    for (uint8 i = 0; i < 3; i++)
    {
        points[i] = GetTalentPoints(i);

        for (auto& entry : GetTalentTree(i))
        {
            curPoints += entry.rank;
            treeLink[i] += std::to_string(entry.rank);
            if (curPoints >= points[i])
            {
                curPoints = 0;
                break;
            }
        }
    }

    link = treeLink[0];

    if (treeLink[1] != "0" || treeLink[2] != "0")
        link = link + "-" + treeLink[1];

    if (treeLink[2] != "0")
        link = link + "-" + treeLink[2];

    return std::move(link);
}

uint32 TalentSpec::highestTree()
{
    uint32 p1 = GetTalentPoints(0);
    uint32 p2 = GetTalentPoints(1);
    uint32 p3 = GetTalentPoints(2);

    if (p1 > p2 && p1 > p3)
        return 0;

    if (p2 > p1 && p2 > p3)
        return 1;

    if (p3 > p1 && p3 > p2)
        return 2;

    if (p1 > p2 || p1 > p3)
        return 0;

    if (p2 > p3 || p2 > p1)
        return 1;

    return 0;
}

std::string const TalentSpec::FormatSpec(Player* bot)
{
    // uint8 cls = bot->getClass(); //not used, (used in lined 403), line marked for removal.

    std::ostringstream out;
    // out << chathelper:: specs[cls][highestTree()] << " (";

    uint32 c0 = GetTalentPoints(0);
    uint32 c1 = GetTalentPoints(1);
    uint32 c2 = GetTalentPoints(2);

    out << (c0 ? "|h|cff00ff00" : "") << c0 << "|h|cffffffff/";
    out << (c1 ? "|h|cff00ff00" : "") << c1 << "|h|cffffffff/";
    out << (c2 ? "|h|cff00ff00" : "") << c2 << "|h|cffffffff";

    return out.str();
}

// Removes talentpoints to match the level
void TalentSpec::CropTalents(uint32 level)
{
    if (points <= LeveltoPoints(level))
        return;

    SortTalents(talents, SORT_BY_POINTS_TREE);

    uint32 points = 0;
    for (auto& entry : talents)
    {
        if (points + entry.rank > LeveltoPoints(level))
            entry.rank = std::max(0u, LeveltoPoints(level) - points);

        points += entry.rank;
    }

    SortTalents(talents, SORT_BY_DEFAULT);
}

// Substracts ranks. Follows the sorting of the newList.
std::vector<TalentSpec::TalentListEntry> TalentSpec::SubTalentList(std::vector<TalentListEntry>& oldList,
                                                                   std::vector<TalentListEntry>& newList,
                                                                   uint32 reverse = SUBSTRACT_OLD_NEW)
{
    std::vector<TalentSpec::TalentListEntry> deltaList = newList;

    for (auto& newentry : deltaList)
        for (auto& oldentry : oldList)
            if (oldentry.entry == newentry.entry)
            {
                if (reverse == ABSOLUTE_DIST)
                    newentry.rank = std::abs(int32(newentry.rank - oldentry.rank));
                else if (reverse == ADDED_POINTS || reverse == REMOVED_POINTS)
                    newentry.rank = std::max(0u, (newentry.rank - oldentry.rank) * (reverse / 2));
                else
                    newentry.rank = (newentry.rank - oldentry.rank) * reverse;
            }

    return deltaList;
}

bool TalentSpec::isEarlierVersionOf(TalentSpec& newSpec)
{
    for (auto& newentry : newSpec.talents)
        for (auto& oldentry : talents)
            if (oldentry.entry == newentry.entry)
                if (oldentry.rank > newentry.rank)
                    return false;
    return true;
}

// Modifies current talents towards new talents up to a maxium of points.
void TalentSpec::ShiftTalents(TalentSpec* currentSpec, uint32 level)
{
    //uint32 currentPoints = currentSpec->GetTalentPoints(); //not used, line marked for removal.
    if (points >= LeveltoPoints(level))  // We have no more points to spend. Better reset and crop
    {
        CropTalents(level);
        return;
    }

    SortTalents(SORT_BY_POINTS_TREE);  // Apply points first to the largest new tree.

    std::vector<TalentSpec::TalentListEntry> deltaList = SubTalentList(currentSpec->talents, talents);

    for (auto& entry : deltaList)
    {
        if (entry.rank < 0)  // We have to remove talents. Might as well reset and crop the new list.
        {
            CropTalents(level);
            return;
        }
    }

    // Start from the current spec.
    talents = currentSpec->talents;

    for (auto& entry : deltaList)
    {
        if (entry.rank + points > LeveltoPoints(level))  // Running out of points. Only apply what we have left.
            entry.rank = std::max(0, std::abs(int32(LeveltoPoints(level) - points)));

        for (auto& subentry : talents)
            if (entry.entry == subentry.entry)
                subentry.rank = subentry.rank + entry.rank;

        points = points + entry.rank;
    }
}
