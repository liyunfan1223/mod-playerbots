/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WorldResurrectTargetValue.h"

#include <list>
#include <mutex>
#include <unordered_map>

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Map.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "WorldSession.h"

namespace
{
constexpr uint32 CLAIM_DURATION = 30 * IN_MILLISECONDS;
constexpr uint32 CLAIM_CLEANUP_INTERVAL = 5 * IN_MILLISECONDS;

struct ResurrectionClaim
{
    ObjectGuid BotGuid;
    uint32 StartedAt = 0;
};

std::mutex claimMutex;
std::unordered_map<ObjectGuid, ResurrectionClaim> targetClaims;
std::unordered_map<ObjectGuid, ObjectGuid> botClaims;
uint32 lastClaimCleanup = 0;

// All claim state contains GUIDs only. Map threads resolve players independently at every use.
void RemoveClaim(ObjectGuid targetGuid)
{
    auto claim = targetClaims.find(targetGuid);
    if (claim == targetClaims.end())
        return;

    botClaims.erase(claim->second.BotGuid);
    targetClaims.erase(claim);
}

bool ExpireClaim(ObjectGuid targetGuid)
{
    auto claim = targetClaims.find(targetGuid);
    if (claim == targetClaims.end())
        return true;

    if (getMSTimeDiff(claim->second.StartedAt, getMSTime()) < CLAIM_DURATION)
        return false;

    RemoveClaim(targetGuid);
    return true;
}

void CleanupExpiredClaims()
{
    uint32 now = getMSTime();
    if (getMSTimeDiff(lastClaimCleanup, now) < CLAIM_CLEANUP_INTERVAL)
        return;

    lastClaimCleanup = now;
    for (auto claim = targetClaims.begin(); claim != targetClaims.end();)
    {
        if (getMSTimeDiff(claim->second.StartedAt, now) >= CLAIM_DURATION)
        {
            botClaims.erase(claim->second.BotGuid);
            claim = targetClaims.erase(claim);
        }
        else
            ++claim;
    }
}

ObjectGuid GetClaimedTarget(ObjectGuid botGuid)
{
    std::lock_guard<std::mutex> lock(claimMutex);
    auto claim = botClaims.find(botGuid);
    if (claim == botClaims.end())
        return ObjectGuid::Empty;

    ObjectGuid targetGuid = claim->second;
    if (ExpireClaim(targetGuid))
        return ObjectGuid::Empty;

    return targetGuid;
}

bool IsClaimedByOther(ObjectGuid botGuid, ObjectGuid targetGuid)
{
    std::lock_guard<std::mutex> lock(claimMutex);
    return !ExpireClaim(targetGuid) && targetClaims.find(targetGuid)->second.BotGuid != botGuid;
}

bool IsEligibleTarget(Player* bot, Player* target)
{
    return target && target != bot && IsRealPlayer(target) && target->GetSession() &&
           !target->GetSession()->PlayerLogout() && target->IsInWorld() && !target->IsDuringRemoveFromWorld() &&
           !target->IsBeingTeleported() && !target->IsGameMaster() && !target->IsSpectator() && !target->IsAlive() &&
           target->getDeathState() == DeathState::Corpse && !target->isResurrectRequested() &&
           target->GetMap() == bot->GetMap() && bot->InSamePhase(target) &&
           target->GetTeamId(true) == bot->GetTeamId(true) && bot->IsFriendlyTo(target) && !target->InBattleground() &&
           !target->InArena() && bot->IsWithinDistInMap(target, sPlayerbotAIConfig.WorldResurrectDistance) &&
           bot->IsWithinLOSInMap(target) && !IsClaimedByOther(bot->GetGUID(), target->GetGUID());
}

bool IsEligibleBot(PlayerbotAI* botAI)
{
    if (!sPlayerbotAIConfig.WorldResurrectEnabled || !botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot || !*WorldResurrect::GetSpellName(bot) || !bot->IsInWorld() || bot->IsDuringRemoveFromWorld() ||
        !bot->GetSession() || bot->GetSession()->PlayerLogout() || bot->GetSession()->IsLoggingOut() ||
        !bot->IsAlive() || bot->IsInCombat() || botAI->GetState() != BOT_STATE_NON_COMBAT || botAI->GetMaster() ||
        bot->GetGroup() || bot->GetGroupInvite() || !sRandomPlayerbotMgr.IsRandomBot(bot) || !bot->GetMap() ||
        bot->GetMap()->Instanceable() || bot->InBattleground() || bot->InArena() || bot->InBattlegroundQueue() ||
        bot->IsBeingTeleported() || bot->IsInFlight() || bot->IsFlying() || bot->Unit::IsFalling() ||
        bot->GetTransport() || bot->GetVehicle() || bot->GetTradeData() || bot->duel || bot->IsMounted() ||
        !bot->IsStandState() || !botAI->CanMove())
    {
        return false;
    }

    // A queued LFG proposal belongs to the normal LFG workflow; no global LFG manager lookup is needed.
    return !botAI->GetAiObjectContext()->GetValue<uint32>("lfg proposal")->Get() &&
           botAI->HasSpell(WorldResurrect::GetSpellName(bot));
}

bool IsCastingClaimedResurrection(PlayerbotAI* botAI, ObjectGuid targetGuid)
{
    if (!targetGuid)
        return false;

    Spell* spell = botAI->GetBot()->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->getState() != SPELL_STATE_PREPARING || !spell->GetSpellInfo() ||
        spell->m_targets.GetUnitTargetGUID() != targetGuid)
        return false;

    uint32 spellId =
        botAI->GetAiObjectContext()->GetValue<uint32>("spell id", WorldResurrect::GetSpellName(botAI->GetBot()))->Get();
    return spell->GetSpellInfo()->Id == spellId;
}

void ReleaseBotClaim(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->HasWorldResurrectClaim())
        return;

    if (Player* bot = botAI->GetBot())
        WorldResurrect::ReleaseClaim(bot->GetGUID());

    botAI->SetWorldResurrectClaim(false);
}
}  // namespace

char const* WorldResurrect::GetSpellName(Player const* player)
{
    if (!player)
        return "";

    switch (player->getClass())
    {
        case CLASS_PRIEST:
            return "resurrection";
        case CLASS_PALADIN:
            return "redemption";
        case CLASS_SHAMAN:
            return "ancestral spirit";
        case CLASS_DRUID:
            return "revive";
        default:
            return "";
    }
}

bool WorldResurrect::CanHelp(PlayerbotAI* botAI)
{
    if (!IsEligibleBot(botAI))
    {
        ReleaseBotClaim(botAI);
        return false;
    }

    if (botAI->GetBot()->IsNonMeleeSpellCast(false))
    {
        if (botAI->HasWorldResurrectClaim() &&
            !IsCastingClaimedResurrection(botAI, GetClaimedTarget(botAI->GetBot()->GetGUID())))
            ReleaseBotClaim(botAI);

        return false;
    }

    return true;
}

void WorldResurrect::UpdateClaim(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->HasWorldResurrectClaim())
        return;

    if (!IsEligibleBot(botAI))
    {
        ReleaseBotClaim(botAI);
        return;
    }

    Player* bot = botAI->GetBot();
    ObjectGuid targetGuid = GetClaimedTarget(bot->GetGUID());
    if (!targetGuid)
    {
        botAI->SetWorldResurrectClaim(false);
        return;
    }

    // An active resurrection must keep its claim while the normal casting gate is false.
    if ((bot->IsNonMeleeSpellCast(false) && !IsCastingClaimedResurrection(botAI, targetGuid)) ||
        !IsEligibleTarget(bot, botAI->GetPlayer(targetGuid)))
        ReleaseBotClaim(botAI);
}

Player* WorldResurrect::GetTarget(PlayerbotAI* botAI)
{
    if (!CanHelp(botAI))
        return nullptr;

    ObjectGuid guid = botAI->GetAiObjectContext()->GetValue<ObjectGuid>("world resurrect target")->Get();
    Player* target = guid ? botAI->GetPlayer(guid) : nullptr;
    if (IsEligibleTarget(botAI->GetBot(), target))
        return target;

    if (guid == GetClaimedTarget(botAI->GetBot()->GetGUID()))
        ReleaseBotClaim(botAI);

    return nullptr;
}

bool WorldResurrect::TryClaim(PlayerbotAI* botAI, ObjectGuid targetGuid)
{
    if (!CanHelp(botAI) || !IsEligibleTarget(botAI->GetBot(), botAI->GetPlayer(targetGuid)))
        return false;

    ObjectGuid botGuid = botAI->GetBot()->GetGUID();
    std::lock_guard<std::mutex> lock(claimMutex);
    CleanupExpiredClaims();
    if (!ExpireClaim(targetGuid))
    {
        bool claimed = targetClaims.find(targetGuid)->second.BotGuid == botGuid;
        if (claimed)
            botAI->SetWorldResurrectClaim(true);

        return claimed;
    }

    if (auto existing = botClaims.find(botGuid); existing != botClaims.end())
        RemoveClaim(existing->second);

    targetClaims[targetGuid] = {botGuid, getMSTime()};
    botClaims[botGuid] = targetGuid;
    botAI->SetWorldResurrectClaim(true);
    return true;
}

void WorldResurrect::ReleaseClaim(ObjectGuid botGuid)
{
    std::lock_guard<std::mutex> lock(claimMutex);
    if (auto claim = botClaims.find(botGuid); claim != botClaims.end())
        RemoveClaim(claim->second);
}

ObjectGuid WorldResurrectTargetValue::Calculate()
{
    if (!WorldResurrect::CanHelp(botAI))
        return ObjectGuid::Empty;

    if (ObjectGuid claimedGuid = GetClaimedTarget(bot->GetGUID()))
    {
        if (IsEligibleTarget(bot, botAI->GetPlayer(claimedGuid)))
            return claimedGuid;

        ReleaseBotClaim(botAI);
    }

    std::list<Player*> players;
    Acore::AnyPlayerInObjectRangeCheck check(bot, sPlayerbotAIConfig.WorldResurrectDistance, false, true);
    Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(bot, players, check);
    Cell::VisitObjects(bot, searcher, sPlayerbotAIConfig.WorldResurrectDistance);

    ObjectGuid nearestGuid;
    float nearestDistance = sPlayerbotAIConfig.WorldResurrectDistance;
    for (Player* player : players)
    {
        if (!IsEligibleTarget(bot, player))
            continue;

        float distance = bot->GetDistance(player);
        if (distance < nearestDistance)
        {
            nearestGuid = player->GetGUID();
            nearestDistance = distance;
        }
    }

    return nearestGuid;
}
