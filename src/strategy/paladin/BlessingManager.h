#ifndef _BLESSING_ASSIGNMENT_H_
#define _BLESSING_ASSIGNMENT_H_

#include "PlayerbotAI.h"
#include "Group.h"
#include "Player.h"

#include <map>
#include <vector>
#include <set>
#include <algorithm>

// -----------------------------------------------------------------------------
// Minimal Blessing Type enum
// -----------------------------------------------------------------------------
enum GreaterBlessingType
{
    GREATER_BLESSING_OF_WISDOM,
    GREATER_BLESSING_OF_MIGHT,
    GREATER_BLESSING_OF_KINGS,
    GREATER_BLESSING_OF_SANCTUARY
};

// -----------------------------------------------------------------------------
// A simple structure to hold which blessings each class should get,
// depending on how many Paladins are in the group.
// Using 'inline static' ensures there's exactly one definition in the program.
// -----------------------------------------------------------------------------
inline static std::map<int, std::map<uint8 /*classId*/, std::vector<GreaterBlessingType>>> BlessingTemplates =
{
    // 1 Paladin: everyone just gets Kings
    {
        1,
        {
            { CLASS_WARRIOR,       { GREATER_BLESSING_OF_KINGS } },
            { CLASS_PALADIN,       { GREATER_BLESSING_OF_KINGS } },
            { CLASS_HUNTER,        { GREATER_BLESSING_OF_KINGS } },
            { CLASS_ROGUE,         { GREATER_BLESSING_OF_KINGS } },
            { CLASS_PRIEST,        { GREATER_BLESSING_OF_KINGS } },
            { CLASS_DEATH_KNIGHT,  { GREATER_BLESSING_OF_KINGS } },
            { CLASS_SHAMAN,        { GREATER_BLESSING_OF_KINGS } },
            { CLASS_MAGE,          { GREATER_BLESSING_OF_KINGS } },
            { CLASS_WARLOCK,       { GREATER_BLESSING_OF_KINGS } },
            { CLASS_DRUID,         { GREATER_BLESSING_OF_KINGS } }
        }
    },
    // 2 Paladins: physical classes prefer Might, casters prefer Wisdom, all get Kings
    {
        2,
        {
            { CLASS_WARRIOR,       { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_KINGS } },
            { CLASS_PALADIN,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_KINGS } },
            { CLASS_HUNTER,        { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_KINGS } },
            { CLASS_ROGUE,         { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_KINGS } },
            { CLASS_PRIEST,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_KINGS } },
            { CLASS_DEATH_KNIGHT,  { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_KINGS } },
            { CLASS_SHAMAN,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_KINGS } },
            { CLASS_MAGE,          { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_KINGS } },
            { CLASS_WARLOCK,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_KINGS } },
            { CLASS_DRUID,         { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_KINGS } }
        }
    },
    // 3 Paladins: might see some Sanctuary usage as well
    {
        3,
        {
            { CLASS_WARRIOR,       { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_PALADIN,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT,     GREATER_BLESSING_OF_KINGS } },
            { CLASS_HUNTER,        { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_WISDOM,     GREATER_BLESSING_OF_KINGS } },
            { CLASS_ROGUE,         { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_PRIEST,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_DEATH_KNIGHT,  { GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_SHAMAN,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT,     GREATER_BLESSING_OF_KINGS } },
            { CLASS_MAGE,          { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_WARLOCK,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_DRUID,         { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT,     GREATER_BLESSING_OF_KINGS } }
        }
    },
    // 4 Paladins: basically everything is on the table
    {
        4,
        {
            { CLASS_WARRIOR,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_PALADIN,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_HUNTER,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_ROGUE,         { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_PRIEST,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_DEATH_KNIGHT,  { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_SHAMAN,        { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_MAGE,          { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_WARLOCK,       { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } },
            { CLASS_DRUID,         { GREATER_BLESSING_OF_WISDOM, GREATER_BLESSING_OF_MIGHT, GREATER_BLESSING_OF_SANCTUARY, GREATER_BLESSING_OF_KINGS } }
        }
    }
};

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

// Checks if a Paladin has the talent required to cast the given blessing.
bool PaladinHasTalentForBlessing(Player* paladin, GreaterBlessingType blessing);

// Returns all Paladins (Player*) in the same group/raid as the bot.
std::vector<Player*> GetPaladinsInGroup(PlayerbotAI* botAI);

// Main function to assign blessings for the current group/raid.
// Returns a map of (Paladin pointer) -> (classId -> assigned blessing).
std::map<Player*, std::map<uint8, GreaterBlessingType>> AssignBlessingsForGroup(PlayerbotAI* botAI);

#endif // _BLESSING_ASSIGNMENT_H_
