/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported with permission from Dustin HendricksonBased from mod-player-bot-level-brackets
 * and mod-player-bot-reset modules with contributors NoxMax (level reset), jimm0thy (friend-list exclusion),
 * Jered Little (arena-team exclusion).
 */

#include "RandomBotLevelMgr.h"
#include "ArenaTeamMgr.h"
#include "DatabaseEnv.h"
#include "LFGMgr.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "QueryResult.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "World.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

// True if bot's name is present in excludeList.
static bool IsNameInExcludeList(Player* bot, std::vector<std::string> const& excludeList)
{
    if (!bot)
        return false;

    return std::find(excludeList.begin(), excludeList.end(), bot->GetName()) != excludeList.end();
}

// Checks if the given bot is present in any real player's friends list.
static bool BotInFriendList(Player* bot, std::vector<uint32> const& socialFriendsList)
{
    if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->IsLoggingOut() ||
        bot->IsDuringRemoveFromWorld())
        return false;

    return std::find(socialFriendsList.begin(), socialFriendsList.end(), bot->GetGUID().GetCounter()) !=
        socialFriendsList.end();
}

static bool IsDisabledBracket(std::vector<LevelBracketConfig> const& configured, uint8 index)
{
    return index < configured.size() && configured[index].Pct == 0;
}

// Checks if the given bot is a member of any arena team.
static bool BotInArenaTeam(Player* bot)
{
    if (!bot)
        return false;
    for (uint8 slot = ARENA_SLOT_2v2; slot <= ARENA_SLOT_5v5; ++slot)
    {
        if (sArenaTeamMgr->GetArenaTeamById(bot->GetArenaTeamId(slot)))
            return true;
    }
    return false;
}

// Checks if a bot is currently in a safe state to perform a level reset (alive, not in combat, not
// in a battleground/arena/dungeon queue or flight, and grouped only with other bots).
static bool IsBotSafeForLevelReset(Player* bot)
{
    if (!bot || !bot->GetSession() || bot->GetSession()->IsLoggingOut() || bot->IsDuringRemoveFromWorld())
        return false;

    if (!bot->IsInWorld())
        return false;

    if (!bot->IsAlive())
        return false;

    if (bot->IsInCombat())
        return false;

    if (bot->InBattleground() || bot->InArena() || bot->inRandomLfgDungeon() || bot->InBattlegroundQueue())
        return false;

    if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
        return false;

    if (Group* group = bot->GetGroup())
    {
        if (sLFGMgr->GetState(group->GetGUID()) != lfg::LFG_STATE_NONE)
            return false;
    }

    if (bot->IsInFlight())
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsInWorld() && !GET_PLAYERBOT_AI(member))
                return false;
        }
    }
    return true;
}

// =============================================================================
// LEVEL BRACKETS FEATURE
// =============================================================================

std::vector<LevelBracketConfig>& RandomBotLevelMgr::GetFactionRanges(TeamId team)
{
    return (team == TEAM_ALLIANCE) ? _allianceRanges : _hordeRanges;
}

// Copies the bracket definitions from PlayerbotAIConfig into the working state and resets the
// working bounds/percentages. Dynamic distribution and the clamp/rebalance pass below mutate these
// working copies at runtime, so PlayerbotAIConfig's own vectors are never touched after this point.
void RandomBotLevelMgr::LoadConfig()
{
    _allianceRanges = sPlayerbotAIConfig.LevelBracketsAlliance;
    _hordeRanges = sPlayerbotAIConfig.LevelBracketsHorde;
    _numRanges = sPlayerbotAIConfig.LevelBracketsNumRanges;
    _randomBotMinLevel = static_cast<uint8>(sPlayerbotAIConfig.RandomBotMinLevel);
    _randomBotMaxLevel = static_cast<uint8>(sPlayerbotAIConfig.RandomBotMaxLevel);

    ClampAndBalanceBrackets();
}

void RandomBotLevelMgr::LogStartupSummary() const
{
    if (!sPlayerbotAIConfig.LevelBracketsEnabled)
        LOG_INFO("playerbots", "[RandomBotLevelMgr] Level brackets sub-feature disabled via configuration.");
    else
    {
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] Level brackets loaded. Check frequency: {} seconds, flagged check frequency: {} "
            "seconds.",
            sPlayerbotAIConfig.LevelBracketsCheckFrequency, sPlayerbotAIConfig.LevelBracketsFlaggedCheckFrequency);
        for (uint8 i = 0; i < _numRanges; ++i)
            LOG_DEBUG("playerbots", "[RandomBotLevelMgr] Alliance Range {}: {}-{}, Desired Percentage: {}%", i + 1,
                _allianceRanges[i].Lower, _allianceRanges[i].Upper, _allianceRanges[i].Pct);
        for (uint8 i = 0; i < _numRanges; ++i)
            LOG_DEBUG("playerbots", "[RandomBotLevelMgr] Horde Range {}: {}-{}, Desired Percentage: {}%", i + 1,
                _hordeRanges[i].Lower, _hordeRanges[i].Upper, _hordeRanges[i].Pct);
    }

    if (!sPlayerbotAIConfig.ResetBotLevelEnabled)
        LOG_INFO("playerbots", "[RandomBotLevelMgr] Level reset sub-feature disabled via configuration.");
    else
    {
        LOG_INFO("playerbots",
            "[RandomBotLevelMgr] Level reset loaded. MaxLevel = {} ({}), ResetToLevel = {}, SkipFromLevel = {} ({}), "
            "SkipToLevel = {}, ResetChance = {}%, ScaledChance = {}, RestrictTimePlayed = {}, "
            "IgnoreGuildBotsWithRealPlayers = {}, ExcludedNames = {}.",
            static_cast<int>(sPlayerbotAIConfig.ResetBotLevelMaxLevel),
            sPlayerbotAIConfig.ResetBotLevelMaxLevel > 0 ? "Enabled" : "Disabled",
            static_cast<int>(sPlayerbotAIConfig.ResetBotLevelResetTo),
            static_cast<int>(sPlayerbotAIConfig.ResetBotLevelSkipFrom),
            sPlayerbotAIConfig.ResetBotLevelSkipFrom > 0 ? "Enabled" : "Disabled",
            static_cast<int>(sPlayerbotAIConfig.ResetBotLevelSkipTo),
            static_cast<int>(sPlayerbotAIConfig.ResetBotLevelChance),
            sPlayerbotAIConfig.ResetBotLevelScaledChance ? "Enabled" : "Disabled",
            sPlayerbotAIConfig.ResetBotLevelRestrictTimePlayed ? "Enabled" : "Disabled",
            sPlayerbotAIConfig.ResetBotLevelIgnoreGuildWithRealPlayers ? "Enabled" : "Disabled",
            sPlayerbotAIConfig.ResetBotLevelExcludeNames.empty()
                ? "None"
                : std::to_string(sPlayerbotAIConfig.ResetBotLevelExcludeNames.size()) + " names");
    }
}

// Clamps bracket bounds to [_randomBotMinLevel, _randomBotMaxLevel] and rebalances the desired
// percentages (per faction) back to summing to 100, if they don't already.
void RandomBotLevelMgr::ClampAndBalanceBrackets()
{
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        if (_allianceRanges[i].Lower < _randomBotMinLevel)
            _allianceRanges[i].Lower = _randomBotMinLevel;
        if (_allianceRanges[i].Upper > _randomBotMaxLevel)
            _allianceRanges[i].Upper = _randomBotMaxLevel;
        if (_allianceRanges[i].Lower > _allianceRanges[i].Upper)
            _allianceRanges[i].Pct = 0;
    }
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        if (_hordeRanges[i].Lower < _randomBotMinLevel)
            _hordeRanges[i].Lower = _randomBotMinLevel;
        if (_hordeRanges[i].Upper > _randomBotMaxLevel)
            _hordeRanges[i].Upper = _randomBotMaxLevel;
        if (_hordeRanges[i].Lower > _hordeRanges[i].Upper)
            _hordeRanges[i].Pct = 0;
    }

    uint32 totalAlliance = 0;
    uint32 totalHorde = 0;
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        totalAlliance += _allianceRanges[i].Pct;
        totalHorde += _hordeRanges[i].Pct;
    }

    if (totalAlliance != 100 && totalAlliance > 0)
    {
        LOG_TRACE("playerbots",
            "[RandomBotLevelMgr] Alliance: Sum of percentages is {} (expected 100). Auto adjusting.", totalAlliance);
        int missing = 100 - totalAlliance;
        while (missing > 0)
        {
            for (uint8 i = 0; i < _numRanges && missing > 0; ++i)
            {
                if (_allianceRanges[i].Lower <= _allianceRanges[i].Upper && _allianceRanges[i].Pct > 0)
                {
                    _allianceRanges[i].Pct++;
                    missing--;
                }
            }
        }
    }
    if (totalHorde != 100 && totalHorde > 0)
    {
        LOG_TRACE("playerbots", "[RandomBotLevelMgr] Horde: Sum of percentages is {} (expected 100). Auto adjusting.",
            totalHorde);
        int missing = 100 - totalHorde;
        while (missing > 0)
        {
            for (uint8 i = 0; i < _numRanges && missing > 0; ++i)
            {
                if (_hordeRanges[i].Lower <= _hordeRanges[i].Upper && _hordeRanges[i].Pct > 0)
                {
                    _hordeRanges[i].Pct++;
                    missing--;
                }
            }
        }
    }
}

// Normalizes real-player-census weights into desiredPercent values that sum to 100 for one
// faction's working bracket vector. Shared by both the synced and per-faction weighting branches.
void RandomBotLevelMgr::ApplyBracketWeights(std::vector<LevelBracketConfig>& ranges, std::vector<float> const& weights)
{
    float total = 0.0f;
    for (uint8 i = 0; i < _numRanges; ++i)
        total += weights[i];

    int pctSum = 0;
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        uint8 pct = (total > 0.0f) ? static_cast<uint8>(std::round((weights[i] / total) * 100)) : 0;
        ranges[i].Pct = pct;
        pctSum += pct;
    }
    // Fix rounding drift so sum = 100.
    int missing = 100 - pctSum;
    for (uint8 i = 0; i < _numRanges && missing > 0; ++i)
    {
        if (ranges[i].Lower <= ranges[i].Upper && ranges[i].Pct > 0)
        {
            ranges[i].Pct++;
            missing--;
        }
    }
}

// Returns the index of the level range containing the given level for the given team.
int RandomBotLevelMgr::GetLevelRangeIndex(uint8 level, TeamId team)
{
    if (level < _randomBotMinLevel || level > _randomBotMaxLevel)
        return -1;

    if (team != TEAM_ALLIANCE && team != TEAM_HORDE)
        return -1;

    std::vector<LevelBracketConfig> const& ranges = GetFactionRanges(team);
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        if (level >= ranges[i].Lower && level <= ranges[i].Upper)
            return i;
    }

    return -1;
}

// Adjusts a bot's level to fit within the given bracket for its faction. Death Knights are never
// assigned below CONFIG_START_HEROIC_PLAYER_LEVEL. The faction's level ranges are resolved via
// GetFactionRanges() at call time (rather than through a cached reference), since those vectors can
// be resized on a config reload.
void RandomBotLevelMgr::AdjustBotToRange(Player* bot, int targetRangeIndex, TeamId team)
{
    if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->IsLoggingOut() ||
        bot->IsDuringRemoveFromWorld())
        return;

    if (targetRangeIndex < 0 || targetRangeIndex >= _numRanges)
        return;

    std::vector<LevelBracketConfig> const& factionRanges = GetFactionRanges(team);
    if (static_cast<size_t>(targetRangeIndex) >= factionRanges.size())
        return;

    if (bot->IsMounted())
        bot->Dismount();

    uint8 botOriginalLevel = bot->GetLevel();
    uint8 newLevel = 0;

    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));

    if (bot->getClass() == CLASS_DEATH_KNIGHT)
    {
        uint8 lowerBound = factionRanges[targetRangeIndex].Lower;
        uint8 upperBound = factionRanges[targetRangeIndex].Upper;
        if (upperBound < dkMinLevel)
        {
            LOG_TRACE("playerbots",
                "[RandomBotLevelMgr] AdjustBotToRange: Cannot assign {} Death Knight '{}' ({}) to range {}-{} "
                "(below level {}).",
                (team == TEAM_ALLIANCE) ? "Alliance" : "Horde", bot->GetName(), botOriginalLevel, lowerBound,
                upperBound, dkMinLevel);
            return;
        }
        if (lowerBound < dkMinLevel)
            lowerBound = dkMinLevel;
        if (lowerBound > upperBound)
            return;
        newLevel = urand(lowerBound, upperBound);
    }
    else
    {
        LevelBracketConfig const& range = factionRanges[targetRangeIndex];
        if (range.Lower > range.Upper)
        {
            LOG_TRACE("playerbots", "[RandomBotLevelMgr] AdjustBotToRange: Invalid range {}-{} for {} bot '{}'.",
                range.Lower, range.Upper, (team == TEAM_ALLIANCE) ? "Alliance" : "Horde", bot->GetName());
            return;
        }
        newLevel = urand(range.Lower, range.Upper);
    }

    PlayerbotFactory newFactory(bot, newLevel);
    newFactory.Randomize(false);

    // Force reset talents if equipment and spec persistence is enabled and the bot rolled to max
    // level. This works around an issue with how randomization interacts with equipment/spec
    // persistence for max-level bots.
    if (newLevel == _randomBotMaxLevel && sPlayerbotAIConfig.EquipAndSpecPersistence)
    {
        PlayerbotFactory tempFactory(bot, newLevel);
        tempFactory.InitTalentsTree(false, true, true);
    }

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    LOG_TRACE("playerbots",
        "[RandomBotLevelMgr] AdjustBotToRange: {} Bot '{}' - {} ({}) adjusted to level {} (target range {}-{}).",
        (team == TEAM_ALLIANCE) ? "Alliance" : "Horde", bot->GetName(),
        botAI ? botAI->GetChatHelper()->FormatClass(bot->getClass()) : "Unknown", botOriginalLevel, newLevel,
        factionRanges[targetRangeIndex].Lower, factionRanges[targetRangeIndex].Upper);
}

// Loads the list of social friend low GUIDs (character_social, flags = 1) into _socialFriendsList.
void RandomBotLevelMgr::LoadSocialFriendList()
{
    _socialFriendsList.clear();
    QueryResult result = CharacterDatabase.Query("SELECT friend FROM character_social WHERE flags = 1");

    if (!result || result->GetRowCount() == 0)
        return;

    do
    {
        _socialFriendsList.push_back(result->Fetch()->Get<uint32>());
    } while (result->NextRow());
}

// Returns the bracket index for a player, flagging it for a pending level reset (to the closest
// bracket) if it currently falls outside every defined range for its faction.
//
// Only random bots are ever enqueued into _pendingLevelResets. This is also called for real
// players during the dynamic-distribution real-player census; a real player outside all brackets
// simply returns -1 (its census contribution is skipped) rather than being queued for a level
// reset - queuing a real player here would eventually reset that player's level, which is a bug
// inherited from the original module that this port fixes.
int RandomBotLevelMgr::GetOrFlagPlayerBracket(Player* player)
{
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(player);

    if (isRandomBot && IsNameInExcludeList(player, sPlayerbotAIConfig.LevelBracketsExcludeNames))
        return -1;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (isRandomBot && sPlayerbotAIConfig.LevelBracketsIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        return -1;

    if (isRandomBot && sPlayerbotAIConfig.LevelBracketsIgnoreArenaTeamBots && BotInArenaTeam(player))
        return -1;

    // Exclude bots grouped with a real player from bracket processing.
    if (isRandomBot)
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsInWorld() && !GET_PLAYERBOT_AI(member))
                    return -1;
            }
        }
    }

    TeamId team = player->GetTeamId();
    int rangeIndex = GetLevelRangeIndex(player->GetLevel(), team);
    if (rangeIndex >= 0)
        return rangeIndex;

    if (team != TEAM_ALLIANCE && team != TEAM_HORDE)
        return -1;

    // Only random bots may be queued for a level reset below. Real players (and non-random bots)
    // outside every bracket simply fall through and return -1.
    if (!isRandomBot)
        return -1;

    std::vector<LevelBracketConfig> const& factionRanges = GetFactionRanges(team);

    int targetRange = -1;
    int smallestDiff = std::numeric_limits<int>::max();
    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    for (int i = 0; i < _numRanges; ++i)
    {
        if (factionRanges[i].Lower > factionRanges[i].Upper)
            continue;

        if (factionRanges[i].Pct == 0)
            continue;

        // Skip brackets that Death Knights cannot be assigned to.
        if (player->getClass() == CLASS_DEATH_KNIGHT && factionRanges[i].Upper < dkMinLevel)
            continue;

        int diff = 0;
        if (player->GetLevel() < factionRanges[i].Lower)
            diff = factionRanges[i].Lower - player->GetLevel();
        else if (player->GetLevel() > factionRanges[i].Upper)
            diff = player->GetLevel() - factionRanges[i].Upper;
        if (diff < smallestDiff)
        {
            smallestDiff = diff;
            targetRange = i;
        }
    }

    if (targetRange >= 0)
    {
        bool alreadyFlagged = false;
        ObjectGuid guid = player->GetGUID();
        for (auto const& entry : _pendingLevelResets)
        {
            if (entry.botGuid == guid)
            {
                alreadyFlagged = true;
                break;
            }
        }
        if (!alreadyFlagged)
            _pendingLevelResets.push_back({guid, targetRange, team});
    }

    return -1;
}

// Moves bots from an over-populated range into ranges that still need bots. Called once for
// safeBots and once for flaggedBots by ProcessFactionDistribution.
void RandomBotLevelMgr::RedistributeSurplusBots(std::vector<Player*>& sourceBots, int fromRange, TeamId team,
    std::vector<int>& actualCounts, std::vector<int> const& desiredCounts, std::vector<int> const& targetRanges)
{
    size_t targetIdx = 0;
    while (actualCounts[fromRange] > desiredCounts[fromRange] && !sourceBots.empty() && targetIdx < targetRanges.size())
    {
        Player* bot = sourceBots.back();
        sourceBots.pop_back();

        int targetRange = targetRanges[targetIdx];
        if (actualCounts[targetRange] >= desiredCounts[targetRange])
        {
            ++targetIdx;
            continue;
        }

        ObjectGuid botGuid = bot->GetGUID();
        bool alreadyFlagged = false;
        for (auto const& entry : _pendingLevelResets)
        {
            if (entry.botGuid == botGuid)
            {
                alreadyFlagged = true;
                break;
            }
        }
        if (!alreadyFlagged)
            _pendingLevelResets.push_back({botGuid, targetRange, team});

        actualCounts[fromRange]--;
        actualCounts[targetRange]++;
        if (actualCounts[targetRange] >= desiredCounts[targetRange])
            ++targetIdx;
    }
}

// Computes desired-vs-actual counts for one faction and flags surplus bots (safe bots first, then
// flagged ones) for a pending reset into under-populated ranges. Shared by both factions to avoid
// duplicating the ~100-line Alliance/Horde block that used to exist here.
void RandomBotLevelMgr::ProcessFactionDistribution(TeamId team, uint32 totalBots, std::vector<int>& actualCounts,
    std::vector<std::vector<Player*>>& botsByRange)
{
    if (totalBots == 0)
        return;

    std::vector<LevelBracketConfig> const& ranges = GetFactionRanges(team);
    char const* factionName = (team == TEAM_ALLIANCE) ? "Alliance" : "Horde";

    std::vector<int> desiredCounts(_numRanges, 0);
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        desiredCounts[i] = static_cast<int>(std::round((ranges[i].Pct / 100.0) * totalBots));
        LOG_DEBUG("playerbots", "[RandomBotLevelMgr] {} Range {} ({}-{}): Desired = {}, Actual = {}.",
            factionName, i + 1, ranges[i].Lower, ranges[i].Upper, desiredCounts[i], actualCounts[i]);
    }

    for (uint8 i = 0; i < _numRanges; ++i)
    {
        std::vector<Player*> safeBots;
        std::vector<Player*> flaggedBots;
        for (Player* bot : botsByRange[i])
        {
            if (IsBotSafeForLevelReset(bot))
                safeBots.push_back(bot);
            else
                flaggedBots.push_back(bot);
        }

        std::vector<int> targetRanges;
        for (uint8 j = 0; j < _numRanges; ++j)
        {
            if (actualCounts[j] < desiredCounts[j])
                targetRanges.push_back(j);
        }

        RedistributeSurplusBots(safeBots, i, team, actualCounts, desiredCounts, targetRanges);
        RedistributeSurplusBots(flaggedBots, i, team, actualCounts, desiredCounts, targetRanges);
    }
}

// Runs the periodic bot level distribution pass: optionally recalculates dynamic bracket
// percentages based on the real-player census, then flags surplus bots in over-populated brackets
// for a pending reset into under-populated ones (per faction, via ProcessFactionDistribution).
void RandomBotLevelMgr::RunLevelBracketsDistribution()
{
    auto const& allPlayers = ObjectAccessor::GetPlayers();

    LoadSocialFriendList();

    if (sPlayerbotAIConfig.LevelBracketsDynamicDistribution)
    {
        // Calculate real player bracket counts.
        std::vector<int> allianceRealCounts(_numRanges, 0);
        std::vector<int> hordeRealCounts(_numRanges, 0);
        uint32 totalAllianceReal = 0;
        uint32 totalHordeReal = 0;

        for (auto const& itr : allPlayers)
        {
            Player* player = itr.second;
            if (!player || !player->IsInWorld())
                continue;
            if (GET_PLAYERBOT_AI(player))
                continue; // Only count real players.
            int rangeIndex = GetOrFlagPlayerBracket(player);
            if (rangeIndex < 0)
                continue;
            if (player->GetTeamId() == TEAM_ALLIANCE)
            {
                allianceRealCounts[rangeIndex]++;
                totalAllianceReal++;
            }
            else if (player->GetTeamId() == TEAM_HORDE)
            {
                hordeRealCounts[rangeIndex]++;
                totalHordeReal++;
            }
        }

        float const baseline = 1.0f;
        std::vector<float> allianceWeights(_numRanges, 0.0f);
        std::vector<float> hordeWeights(_numRanges, 0.0f);

        // SYNCED MODE: real player weighting is combined for both factions, applied to both bracket tables.
        if (sPlayerbotAIConfig.LevelBracketsSyncFactions)
        {
            uint32 totalCombinedReal = totalAllianceReal + totalHordeReal;
            for (uint8 i = 0; i < _numRanges; ++i)
            {
                int combinedReal = allianceRealCounts[i] + hordeRealCounts[i];
                float weight = baseline + sPlayerbotAIConfig.LevelBracketsRealPlayerWeight *
                    (totalCombinedReal > 0 ? (1.0f / float(totalCombinedReal)) : 1.0f) * std::log(1 + combinedReal);

                allianceWeights[i] = IsDisabledBracket(sPlayerbotAIConfig.LevelBracketsAlliance, i) ? 0.0f : weight;
                hordeWeights[i] = IsDisabledBracket(sPlayerbotAIConfig.LevelBracketsHorde, i) ? 0.0f : weight;
            }
        }
        else
        {
            // Separate dynamic weighting for each faction.
            for (uint8 i = 0; i < _numRanges; ++i)
            {
                if (_allianceRanges[i].Lower > _allianceRanges[i].Upper ||
                    IsDisabledBracket(sPlayerbotAIConfig.LevelBracketsAlliance, i))
                    allianceWeights[i] = 0.0f;
                else
                    allianceWeights[i] = baseline + sPlayerbotAIConfig.LevelBracketsRealPlayerWeight *
                        (totalAllianceReal > 0 ? (1.0f / totalAllianceReal) : 1.0f) *
                        std::log(1 + allianceRealCounts[i]);

                if (_hordeRanges[i].Lower > _hordeRanges[i].Upper ||
                    IsDisabledBracket(sPlayerbotAIConfig.LevelBracketsHorde, i))
                    hordeWeights[i] = 0.0f;
                else
                    hordeWeights[i] = baseline + sPlayerbotAIConfig.LevelBracketsRealPlayerWeight *
                        (totalHordeReal > 0 ? (1.0f / totalHordeReal) : 1.0f) * std::log(1 + hordeRealCounts[i]);
            }
        }

        ApplyBracketWeights(_allianceRanges, allianceWeights);
        ApplyBracketWeights(_hordeRanges, hordeWeights);

        // Ensure brackets respect global min/max levels and percentages sum to 100.
        ClampAndBalanceBrackets();

        for (uint8 i = 0; i < _numRanges; ++i)
            LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] Final Range {}: {}-{}, Alliance Desired: {}%, Horde Desired: {}%", i + 1,
                _allianceRanges[i].Lower, _allianceRanges[i].Upper, _allianceRanges[i].Pct, _hordeRanges[i].Pct);
    }

    uint32 totalAllianceBots = 0;
    std::vector<int> allianceActualCounts(_numRanges, 0);
    std::vector<std::vector<Player*>> allianceBotsByRange(_numRanges);

    uint32 totalHordeBots = 0;
    std::vector<int> hordeActualCounts(_numRanges, 0);
    std::vector<std::vector<Player*>> hordeBotsByRange(_numRanges);

    for (auto const& itr : allPlayers)
    {
        Player* player = itr.second;
        if (!player || !player->IsInWorld())
            continue;
        if (!sRandomPlayerbotMgr.IsRandomBot(player))
            continue;
        if (IsNameInExcludeList(player, sPlayerbotAIConfig.LevelBracketsExcludeNames))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (sPlayerbotAIConfig.LevelBracketsIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
            continue;
        if (sPlayerbotAIConfig.LevelBracketsIgnoreFriendListed && BotInFriendList(player, _socialFriendsList))
            continue;
        if (sPlayerbotAIConfig.LevelBracketsIgnoreArenaTeamBots && BotInArenaTeam(player))
            continue;

        if (player->GetTeamId() == TEAM_ALLIANCE)
        {
            totalAllianceBots++;
            int rangeIndex = GetOrFlagPlayerBracket(player);
            if (rangeIndex >= 0)
            {
                allianceActualCounts[rangeIndex]++;
                allianceBotsByRange[rangeIndex].push_back(player);
            }
        }
        else if (player->GetTeamId() == TEAM_HORDE)
        {
            totalHordeBots++;
            int rangeIndex = GetOrFlagPlayerBracket(player);
            if (rangeIndex >= 0)
            {
                hordeActualCounts[rangeIndex]++;
                hordeBotsByRange[rangeIndex].push_back(player);
            }
        }
    }

    LOG_DEBUG("playerbots", "[RandomBotLevelMgr] Total Alliance Bots: {}. Total Horde Bots: {}.",
        totalAllianceBots, totalHordeBots);

    ProcessFactionDistribution(TEAM_ALLIANCE, totalAllianceBots, allianceActualCounts, allianceBotsByRange);
    ProcessFactionDistribution(TEAM_HORDE, totalHordeBots, hordeActualCounts, hordeBotsByRange);

    LOG_DEBUG("playerbots",
        "[RandomBotLevelMgr] Distribution adjustment complete. Alliance bots: {}, Horde bots: {}.", totalAllianceBots,
        totalHordeBots);
}

// Processes the pending level reset queue, applying up to FlaggedProcessLimit resets per cycle
// (0 = unlimited). Bots are dropped from the queue if they've gone offline, become excluded, joined
// a guild/friend-list/arena-team/group that should protect them, stopped being a random bot, or
// otherwise stopped being eligible; they are reset (and dropped) once they're confirmed safe.
void RandomBotLevelMgr::ProcessPendingLevelResets()
{
    if (_pendingLevelResets.empty())
        return;

    uint32 processed = 0;
    for (auto it = _pendingLevelResets.begin(); it != _pendingLevelResets.end();)
    {
        if (sPlayerbotAIConfig.LevelBracketsFlaggedProcessLimit > 0 &&
            processed >= sPlayerbotAIConfig.LevelBracketsFlaggedProcessLimit)
            break;

        Player* bot = ObjectAccessor::FindPlayer(it->botGuid);

        if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->IsLoggingOut() ||
            bot->IsDuringRemoveFromWorld())
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        // Defensive: never resolve a queue entry to anything but a random bot. Real players must
        // never end up in this queue (see GetOrFlagPlayerBracket), but guard here too in case a
        // player's bot status changes between enqueue and processing.
        if (!sRandomPlayerbotMgr.IsRandomBot(bot))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        if (IsNameInExcludeList(bot, sPlayerbotAIConfig.LevelBracketsExcludeNames))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        int targetRange = it->targetRange;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (sPlayerbotAIConfig.LevelBracketsIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        if (sPlayerbotAIConfig.LevelBracketsIgnoreFriendListed && BotInFriendList(bot, _socialFriendsList))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        if (sPlayerbotAIConfig.LevelBracketsIgnoreArenaTeamBots && BotInArenaTeam(bot))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        // Check if the bot is now grouped with a real player.
        if (Group* group = bot->GetGroup())
        {
            bool hasRealPlayer = false;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsInWorld() && !GET_PLAYERBOT_AI(member))
                {
                    hasRealPlayer = true;
                    break;
                }
            }
            if (hasRealPlayer)
            {
                it = _pendingLevelResets.erase(it);
                continue;
            }
        }

        if (IsBotSafeForLevelReset(bot))
        {
            AdjustBotToRange(bot, targetRange, it->team);
            it = _pendingLevelResets.erase(it);
            ++processed;
        }
        else
            ++it;
    }
}

// =============================================================================
// LEVEL RESET FEATURE
// =============================================================================

// Computes the percent chance that a bot at the given level should be reset. When
// Playerbots.ResetBotLevel.ScaledChance is enabled, the chance scales linearly from 0 at level 1
// up to Playerbots.ResetBotLevel.ResetChance at Playerbots.ResetBotLevel.MaxLevel.
uint8 RandomBotLevelMgr::ComputeResetChance(uint8 level) const
{
    uint8 chance = sPlayerbotAIConfig.ResetBotLevelChance;
    if (sPlayerbotAIConfig.ResetBotLevelScaledChance)
    {
        chance = static_cast<uint8>((static_cast<float>(level) / sPlayerbotAIConfig.ResetBotLevelMaxLevel) *
            sPlayerbotAIConfig.ResetBotLevelChance);
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] ComputeResetChance: For level {} / {} with scaling, computed chance = {}%", level,
            sPlayerbotAIConfig.ResetBotLevelMaxLevel, chance);
    }
    else
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] ComputeResetChance: For level {} / {} without scaling, chance = {}%", level,
            sPlayerbotAIConfig.ResetBotLevelMaxLevel, chance);
    return chance;
}

// Resets a bot down to Playerbots.ResetBotLevel.ResetToLevel (or the Death Knight starting level,
// whichever is higher) via a full PlayerbotFactory randomize.
void RandomBotLevelMgr::ResetBot(Player* player, uint8 currentLevel)
{
    uint8 levelToResetTo = sPlayerbotAIConfig.ResetBotLevelResetTo;

    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    if (player->getClass() == CLASS_DEATH_KNIGHT && levelToResetTo < dkMinLevel)
        levelToResetTo = dkMinLevel;

    // Dismount before randomization to prevent a wrong mount at the new level.
    if (player->IsMounted())
        player->Dismount();

    PlayerbotFactory newFactory(player, levelToResetTo);
    newFactory.Randomize(false);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    LOG_DEBUG("playerbots", "[RandomBotLevelMgr] ResetBot: Bot '{}' - {} at level {} was reset to level {}.",
        player->GetName(), botAI ? botAI->GetChatHelper()->FormatClass(player->getClass()) : "Unknown", currentLevel,
        levelToResetTo);
}

// Sends a bot straight to Playerbots.ResetBotLevel.SkipToLevel (or the Death Knight starting
// level, whichever is higher) via a full PlayerbotFactory randomize.
void RandomBotLevelMgr::SkipBotLevel(Player* player, uint8 currentLevel)
{
    uint8 levelToSkipTo = sPlayerbotAIConfig.ResetBotLevelSkipTo;

    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    if (player->getClass() == CLASS_DEATH_KNIGHT && levelToSkipTo < dkMinLevel)
        levelToSkipTo = dkMinLevel;

    // Dismount before randomization to prevent a wrong mount at the new level.
    if (player->IsMounted())
        player->Dismount();

    PlayerbotFactory newFactory(player, levelToSkipTo);
    newFactory.Randomize(false);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    LOG_DEBUG("playerbots", "[RandomBotLevelMgr] SkipBotLevel: Bot '{}' - {} at level {} was skipped to level {}.",
        player->GetName(), botAI ? botAI->GetChatHelper()->FormatClass(player->getClass()) : "Unknown", currentLevel,
        levelToSkipTo);
}

// Runs the periodic played-time-based reset check for bots sitting at or above MaxLevel. Only
// reached when Enabled, RestrictTimePlayed, and MaxLevel > 0 (see Update()).
void RandomBotLevelMgr::RunResetPlayedTimeCheck()
{
    LOG_DEBUG("playerbots", "[RandomBotLevelMgr] OnUpdate: Starting time-based reset check...");

    auto const& allPlayers = ObjectAccessor::GetPlayers();
    for (auto const& itr : allPlayers)
    {
        Player* candidate = itr.second;
        if (!candidate || !candidate->IsInWorld())
            continue;
        if (!sRandomPlayerbotMgr.IsRandomBot(candidate))
            continue;

        if (IsNameInExcludeList(candidate, sPlayerbotAIConfig.ResetBotLevelExcludeNames))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(candidate);
        if (sPlayerbotAIConfig.ResetBotLevelIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
            continue;

        uint8 currentLevel = candidate->GetLevel();
        if (currentLevel < sPlayerbotAIConfig.ResetBotLevelMaxLevel)
            continue;

        // Only reset if the bot has played at least MinTimePlayed seconds at this level.
        if (candidate->GetLevelPlayedTime() < sPlayerbotAIConfig.ResetBotLevelMinTimePlayed)
        {
            LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] OnUpdate: Bot '{}' at level {} has insufficient played time ({} < {} "
                "seconds).",
                candidate->GetName(), currentLevel, candidate->GetLevelPlayedTime(),
                sPlayerbotAIConfig.ResetBotLevelMinTimePlayed);
            continue;
        }

        uint8 resetChance = ComputeResetChance(currentLevel);
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnUpdate: Bot '{}' qualifies for time-based reset. Level: {}, "
            "LevelPlayedTime: {} seconds, computed reset chance: {}%.",
            candidate->GetName(), currentLevel, candidate->GetLevelPlayedTime(), resetChance);
        if (urand(0, 99) < resetChance)
        {
            LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] OnUpdate: Reset chance check passed for bot '{}'. Resetting bot.",
                candidate->GetName());
            ResetBot(candidate, currentLevel);
        }
    }
}

// =============================================================================
// SHARED UPDATE / HOOKS
// =============================================================================

void RandomBotLevelMgr::Update(uint32 diff)
{
    if (sPlayerbotAIConfig.LevelBracketsEnabled)
    {
        _bracketsTimer += diff;
        _flaggedTimer += diff;

        if (_flaggedTimer >= sPlayerbotAIConfig.LevelBracketsFlaggedCheckFrequency * IN_MILLISECONDS)
        {
            ProcessPendingLevelResets();
            _flaggedTimer = 0;
        }

        if (_bracketsTimer >= sPlayerbotAIConfig.LevelBracketsCheckFrequency * IN_MILLISECONDS)
        {
            _bracketsTimer = 0;
            RunLevelBracketsDistribution();
        }
    }

    if (sPlayerbotAIConfig.ResetBotLevelEnabled && sPlayerbotAIConfig.ResetBotLevelRestrictTimePlayed &&
        sPlayerbotAIConfig.ResetBotLevelMaxLevel > 0)
    {
        _resetTimer += diff;
        if (_resetTimer >= sPlayerbotAIConfig.ResetBotLevelPlayedTimeCheckFrequency * IN_MILLISECONDS)
        {
            _resetTimer = 0;
            RunResetPlayedTimeCheck();
        }
    }
}

void RandomBotLevelMgr::OnBotLogin(Player* player)
{
    if (!sRandomPlayerbotMgr.IsRandomBot(player))
        return;

    if (IsNameInExcludeList(player, sPlayerbotAIConfig.ResetBotLevelExcludeNames))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (sPlayerbotAIConfig.ResetBotLevelIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        return;

    uint8 currentLevel = player->GetLevel();

    if (sPlayerbotAIConfig.ResetBotLevelMaxLevel > 0)
    {
        // Bot is above MaxLevel: reset immediately.
        if (currentLevel > sPlayerbotAIConfig.ResetBotLevelMaxLevel)
        {
            LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] OnPlayerLogin: Bot '{}' above max level {}. Resetting immediately.",
                player->GetName(), sPlayerbotAIConfig.ResetBotLevelMaxLevel);
            ResetBot(player, currentLevel);
            return;
        }

        // Bot is exactly at MaxLevel: apply the time-played restriction (if any) and chance.
        if (currentLevel == sPlayerbotAIConfig.ResetBotLevelMaxLevel)
        {
            if (!sPlayerbotAIConfig.ResetBotLevelRestrictTimePlayed ||
                player->GetLevelPlayedTime() >= sPlayerbotAIConfig.ResetBotLevelMinTimePlayed)
            {
                uint8 resetChance = ComputeResetChance(currentLevel);
                if (urand(0, 99) < resetChance)
                {
                    LOG_DEBUG("playerbots",
                        "[RandomBotLevelMgr] OnPlayerLogin: Bot '{}' meets reset criteria. Resetting.",
                        player->GetName());
                    ResetBot(player, currentLevel);
                }
            }
        }
    }

    if (sPlayerbotAIConfig.ResetBotLevelSkipFrom > 0 && currentLevel == sPlayerbotAIConfig.ResetBotLevelSkipFrom)
    {
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLogin: Bot '{}' at skip level {}. Applying skip.", player->GetName(),
            currentLevel);
        SkipBotLevel(player, currentLevel);
    }
}

void RandomBotLevelMgr::OnBotLevelChanged(Player* player, uint8 oldLevel)
{
    if (!sRandomPlayerbotMgr.IsRandomBot(player))
        return;

    // Only react to a natural level up.
    if (player->GetLevel() != oldLevel + 1)
        return;

    if (IsNameInExcludeList(player, sPlayerbotAIConfig.ResetBotLevelExcludeNames))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (sPlayerbotAIConfig.ResetBotLevelIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        return;

    uint8 newLevel = player->GetLevel();

    if (newLevel == 1)
        return;

    if (player->getClass() == CLASS_DEATH_KNIGHT &&
        newLevel == static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)))
        return;

    // SkipFromLevel takes priority and is not affected by ScaledChance or RestrictTimePlayed.
    if (sPlayerbotAIConfig.ResetBotLevelSkipFrom > 0 && newLevel == sPlayerbotAIConfig.ResetBotLevelSkipFrom)
    {
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '{}' reached skip level {}. Skipping to level {}.",
            player->GetName(), newLevel, sPlayerbotAIConfig.ResetBotLevelSkipTo);
        SkipBotLevel(player, newLevel);
        return;
    }

    if (sPlayerbotAIConfig.ResetBotLevelMaxLevel == 0)
        return;

    // Strictly above MaxLevel: reset immediately regardless of time played.
    if (newLevel > sPlayerbotAIConfig.ResetBotLevelMaxLevel)
    {
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '{}' exceeded max level {}. Resetting immediately.",
            player->GetName(), sPlayerbotAIConfig.ResetBotLevelMaxLevel);
        ResetBot(player, newLevel);
        return;
    }

    // Exactly at MaxLevel with a time-played restriction: defer to the OnUpdate timer.
    if (sPlayerbotAIConfig.ResetBotLevelRestrictTimePlayed && newLevel == sPlayerbotAIConfig.ResetBotLevelMaxLevel)
    {
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '{}' at level {} deferred to OnUpdate due to "
            "time-played restriction.",
            player->GetName(), newLevel);
        return;
    }

    uint8 resetChance = ComputeResetChance(newLevel);
    if (sPlayerbotAIConfig.ResetBotLevelScaledChance || newLevel >= sPlayerbotAIConfig.ResetBotLevelMaxLevel)
    {
        LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '{}' at level {} has reset chance {}%.",
            player->GetName(), newLevel, resetChance);
        if (urand(0, 99) < resetChance)
            ResetBot(player, newLevel);
    }
}

void RandomBotLevelMgr::OnPlayerLogout(Player* player)
{
    // Level brackets: drop the bot from the pending-reset queue. Safe to run even when the
    // brackets sub-feature is disabled, since the queue is then always empty.
    ObjectGuid guid = player->GetGUID();
    _pendingLevelResets.erase(
        std::remove_if(_pendingLevelResets.begin(), _pendingLevelResets.end(),
            [guid](PendingResetEntry const& entry) { return entry.botGuid == guid; }),
        _pendingLevelResets.end());
}

// =============================================================================
// WORLD SCRIPT
// =============================================================================
class RandomBotLevelWorldScript : public WorldScript
{
public:
    RandomBotLevelWorldScript()
        : WorldScript("RandomBotLevelWorldScript",
              { WORLDHOOK_ON_STARTUP, WORLDHOOK_ON_UPDATE, WORLDHOOK_ON_AFTER_CONFIG_LOAD })
    {
    }

    void OnStartup() override
    {
        RandomBotLevelMgr::instance().LoadConfig();
        RandomBotLevelMgr::instance().LogStartupSummary();
    }

    // Picks up ".reload config": the core reloads all config files (including playerbots.conf)
    // into sConfigMgr before firing this hook. The initial (reload == false) call happens before
    // PlayerbotAIConfig::Initialize() has run, so only act on actual reloads - OnStartup covers
    // the initial load.
    void OnAfterConfigLoad(bool reload) override
    {
        if (!reload)
            return;

        sPlayerbotAIConfig.LoadRandomBotLevelConfig();
        RandomBotLevelMgr::instance().LoadConfig();
        LOG_INFO("playerbots", "[RandomBotLevelMgr] Level management config reloaded.");
    }

    void OnUpdate(uint32 diff) override
    {
        RandomBotLevelMgr::instance().Update(diff);
    }
};

// =============================================================================
// PLAYER SCRIPT
// =============================================================================
class RandomBotLevelPlayerScript : public PlayerScript
{
public:
    RandomBotLevelPlayerScript()
        : PlayerScript("RandomBotLevelPlayerScript",
              { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LEVEL_CHANGED, PLAYERHOOK_ON_LOGOUT })
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!sPlayerbotAIConfig.ResetBotLevelEnabled)
            return;
        RandomBotLevelMgr::instance().OnBotLogin(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
    {
        if (!sPlayerbotAIConfig.ResetBotLevelEnabled)
            return;
        RandomBotLevelMgr::instance().OnBotLevelChanged(player, oldLevel);
    }

    void OnPlayerLogout(Player* player) override
    {
        RandomBotLevelMgr::instance().OnPlayerLogout(player);
    }
};

// -----------------------------------------------------------------------------
// ENTRY POINT
// -----------------------------------------------------------------------------
void AddSC_randombot_level_mgr()
{
    new RandomBotLevelWorldScript();
    new RandomBotLevelPlayerScript();
}
