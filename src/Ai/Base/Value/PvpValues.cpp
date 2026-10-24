/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PvpValues.h"

#include <algorithm>

#include "BattleGroundTactics.h"
#include "BattlegroundEY.h"
#include "BattlegroundMgr.h"
#include "BattlegroundWS.h"
#include "GameObject.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"

bool IsUnavailableCombatTarget(Player const* player, Unit const* target)
{
    return !player || !target || !target->IsInWorld() || target->GetMap() != player->GetMap() || !target->IsAlive() ||
           target->HasSpiritOfRedemptionAura() ||
           (target->IsPlayer() && target->ToPlayer()->HasPlayerFlag(PLAYER_FLAGS_GHOST));
}

bool IsValidWsgCarrier(Player const* player, Unit const* carrier, bool friendly)
{
    Battleground const* bg = player ? player->GetBattleground() : nullptr;
    if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS || bg->GetStatus() != STATUS_IN_PROGRESS || !carrier ||
        !carrier->IsPlayer() || IsUnavailableCombatTarget(player, carrier))
        return false;
    TeamId flagTeam = friendly ? bg->GetOtherTeamId(player->GetTeamId()) : player->GetTeamId();
    BattlegroundWS const* warsong = static_cast<BattlegroundWS const*>(bg);
    return (carrier->ToPlayer()->GetTeamId() == player->GetTeamId()) == friendly &&
           warsong->GetFlagState(flagTeam) == BG_WS_FLAG_STATE_ON_PLAYER &&
           warsong->GetFlagPickerGUID(flagTeam) == carrier->GetGUID();
}

bool CanRespondToWsgCarrier(PlayerbotAI* botAI, Unit* carrier)
{
    Player* bot = botAI->GetBot();
    if (!IsValidWsgCarrier(bot, carrier, false) || !bot->CanSeeOrDetect(carrier) ||
        !ServerFacade::instance().IsDistanceLessOrEqualThan(ServerFacade::instance().GetDistance2d(bot, carrier),
                                                            VISIBILITY_DISTANCE_NORMAL) ||
        BGTactics::ShouldYieldWsgTactics(botAI))
        return false;
    WsgTeamAssignment assignment =
        botAI->GetAiObjectContext()->GetValue<WsgTeamAssignment>("wsg team assignment")->Get();
    Battleground* bg = bot->GetBattleground();
    return assignment.Valid && assignment.Returner &&
           static_cast<BattlegroundWS*>(bg)->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetTeamId())) != bot->GetGUID();
}

bool IsWsgEscortTargetAllowed(PlayerbotAI* botAI, Unit const* target)
{
    Player* bot = botAI->GetBot();
    Battleground* bg = bot->GetBattleground();
    if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS)
        return true;
    WsgTeamAssignment assignment =
        botAI->GetAiObjectContext()->GetValue<WsgTeamAssignment>("wsg team assignment")->Get();
    Unit* carrier = botAI->GetAiObjectContext()->GetValue<Unit*>("team flag carrier")->Get();
    if (!assignment.Valid || !assignment.Escort || carrier == bot || !IsValidWsgCarrier(bot, carrier, true))
        return true;
    if (IsUnavailableCombatTarget(bot, target))
        return false;
    constexpr float carrierThreatRadius = 45.0f;
    constexpr float selfDefenseRadius = 15.0f;
    return target->IsWithinDistInMap(carrier, carrierThreatRadius) || target->GetVictim() == carrier ||
           bot->IsWithinMeleeRange(target) ||
           (target->GetVictim() == bot && bot->IsWithinDistInMap(target, selfDefenseRadius));
}

bool ShouldRegroupWsgEscort(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Battleground* bg = bot->GetBattleground();
    if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS || bg->GetStatus() != STATUS_IN_PROGRESS || !bot->IsAlive() ||
        !bot->IsInCombat() || BGTactics::ShouldYieldWsgTactics(botAI))
        return false;
    AiObjectContext* context = botAI->GetAiObjectContext();
    WsgTeamAssignment assignment = context->GetValue<WsgTeamAssignment>("wsg team assignment")->Get();
    Unit* carrier = context->GetValue<Unit*>("team flag carrier")->Get();
    Unit* enemy = context->GetValue<Unit*>("current target")->Get();
    constexpr float regroupDistance = 30.0f;
    return assignment.Valid && assignment.Escort && carrier != bot && IsValidWsgCarrier(bot, carrier, true) && enemy &&
           !IsUnavailableCombatTarget(bot, enemy) && !bot->IsWithinDistInMap(carrier, regroupDistance) &&
           !IsWsgEscortTargetAllowed(botAI, enemy);
}

bool IsWsgSupportTarget(PlayerbotAI* botAI, Unit const* target)
{
    Player* bot = botAI->GetBot();
    Battleground* bg = bot->GetBattleground();
    if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS || bg->GetStatus() != STATUS_IN_PROGRESS || !target ||
        !target->IsPlayer() || IsUnavailableCombatTarget(bot, target))
        return false;
    WsgTeamAssignment assignment =
        botAI->GetAiObjectContext()->GetValue<WsgTeamAssignment>("wsg team assignment")->Get();
    if (!assignment.Valid)
        return false;
    Unit* carrier = botAI->GetAiObjectContext()->GetValue<Unit*>("team flag carrier")->Get();
    constexpr float carrierThreatRadius = 30.0f;
    constexpr float baseThreatRadius = 45.0f;
    bool nearObjective = false;
    if (assignment.Escort && carrier != bot && IsValidWsgCarrier(bot, carrier, true))
        nearObjective = target->GetVictim() == carrier || target->IsWithinDistInMap(carrier, carrierThreatRadius);
    else if (assignment.Defender && !assignment.Escort)
    {
        GameObject* base =
            bg->GetBGObject(bot->GetTeamId() == TEAM_ALLIANCE ? BG_WS_OBJECT_A_FLAG : BG_WS_OBJECT_H_FLAG);
        nearObjective = base && target->IsWithinDistInMap(base, baseThreatRadius);
    }
    return nearObjective && bot->CanSeeOrDetect(target) && bot->IsValidAttackTarget(target) &&
           bot->IsWithinLOSInMap(target);
}

WsgTeamAssignment WsgTeamAssignmentValue::Calculate()
{
    WsgTeamAssignment assignment;
    Battleground* bg = bot->GetBattleground();
    if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS || !bot->IsAlive() || !bot->IsInWorld())
        return assignment;

    if (bg->GetStatus() == STATUS_WAIT_JOIN)
    {
        // Only populate the waiting-position rank, not live roles or human-defense observations.
        for (auto const& [guid, player] : bg->GetPlayers())
        {
            if (!player || !player->IsInWorld() || !player->IsAlive() || player->GetMap() != bot->GetMap() ||
                player->GetTeamId() != bot->GetTeamId() || !player->GetSession() || !player->GetSession()->IsHeadless())
                continue;
            if (guid < bot->GetGUID())
                ++assignment.Role;
        }
        return assignment;
    }
    if (bg->GetStatus() != STATUS_IN_PROGRESS)
        return assignment;

    BattlegroundWS* warsong = static_cast<BattlegroundWS*>(bg);
    ObjectGuid carrierGuid = warsong->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetTeamId()));
    ObjectGuid friendlyCarrierGuid = carrierGuid;
    Player* friendlyCarrier =
        friendlyCarrierGuid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(bg->GetBgMap(), friendlyCarrierGuid);
    if (!IsValidWsgCarrier(bot, friendlyCarrier, true))
        friendlyCarrier = nullptr;
    std::vector<ObjectGuid> bots;
    std::vector<ObjectGuid> allBots;
    std::map<ObjectGuid, std::pair<uint8, float>> strength;
    bool foundSelf = false;
    for (auto const& [guid, player] : bg->GetPlayers())
    {
        if (!player || !player->IsInWorld() || player->GetMap() != bot->GetMap() ||
            player->GetTeamId() != bot->GetTeamId())
            continue;
        if (!player->GetSession() || !player->GetSession()->IsHeadless())
            continue;
        allBots.push_back(guid);
        if (guid == bot->GetGUID())
            foundSelf = true;
        if (player->IsAlive() && guid != carrierGuid)
        {
            bots.push_back(guid);
            strength.emplace(guid, std::make_pair(player->GetLevel(), player->GetAverageItemLevel()));
        }
    }
    if (!foundSelf)
        return assignment;

    std::sort(allBots.begin(), allBots.end());
    std::sort(bots.begin(), bots.end(),
              [&](ObjectGuid const& left, ObjectGuid const& right)
              {
                  if (strength.at(left) != strength.at(right))
                      return strength.at(left) > strength.at(right);
                  return left < right;
              });
    uint32 teamSize = bots.size();
    WSBotStrategy strategy = static_cast<WSBotStrategy>(BGTactics::GetBotStrategyForTeam(bg, bot->GetTeamId()));
    uint32 targetDefenders = BGTactics::GetWsgDefenderCount(bg, bot->GetTeamId());
    uint32 targetAttackers = strategy == WS_STRATEGY_OFFENSIVE   ? WSG_ATTACKER_ROLES_OFFENSIVE
                             : strategy == WS_STRATEGY_DEFENSIVE ? WSG_ATTACKER_ROLES_DEFENSIVE
                                                                 : WSG_ATTACKER_ROLES_BALANCED;
    uint32 defenderQuota = std::min<uint32>(targetDefenders, teamSize);
    uint32 attackerQuota = std::min<uint32>(targetAttackers, teamSize - defenderQuota);
    std::vector<ObjectGuid> defenders;
    std::vector<ObjectGuid> attackers;
    bool strongestDefenders = strategy == WS_STRATEGY_OFFENSIVE;
    if (strongestDefenders)
        defenders.assign(bots.begin(), bots.begin() + defenderQuota);
    else
        attackers.assign(bots.begin(), bots.begin() + attackerQuota);
    for (ObjectGuid const& guid : bots)
    {
        if (strongestDefenders && attackers.size() < attackerQuota &&
            std::find(defenders.begin(), defenders.end(), guid) == defenders.end())
            attackers.push_back(guid);
        else if (!strongestDefenders && defenders.size() < defenderQuota &&
                 std::find(attackers.begin(), attackers.end(), guid) == attackers.end())
            defenders.push_back(guid);
    }
    ObjectGuid baseGuard = defenders.empty() ? ObjectGuid::Empty : defenders.front();
    _escortGuids.clear();
    if (friendlyCarrier)
        for (ObjectGuid const& guid : bots)
        {
            if (_escortGuids.size() >= 2)
                break;
            if (std::find(defenders.begin(), defenders.end(), guid) == defenders.end() &&
                std::find(attackers.begin(), attackers.end(), guid) == attackers.end())
                _escortGuids.push_back(guid);
        }
    if (friendlyCarrier)
        for (ObjectGuid const& guid : attackers)
        {
            constexpr uint32 maxEscorts = 2;
            if (_escortGuids.size() >= maxEscorts)
                break;
            _escortGuids.push_back(guid);
        }
    assignment.Role = static_cast<uint32>(std::find(allBots.begin(), allBots.end(), bot->GetGUID()) - allBots.begin());
    assignment.Defenders = static_cast<uint8>(defenderQuota);
    assignment.Escorts = static_cast<uint8>(_escortGuids.size());
    auto escort = std::find(_escortGuids.begin(), _escortGuids.end(), bot->GetGUID());
    assignment.Escort = escort != _escortGuids.end();
    if (assignment.Escort)
        assignment.EscortIndex = static_cast<uint8>(escort - _escortGuids.begin());
    assignment.BaseDefender = bot->GetGUID() == baseGuard;
    assignment.Defender = std::find(defenders.begin(), defenders.end(), bot->GetGUID()) != defenders.end();
    assignment.Attacker = std::find(attackers.begin(), attackers.end(), bot->GetGUID()) != attackers.end();
    GameObject* enemyBase =
        bg->GetBGObject(bot->GetTeamId() == TEAM_ALLIANCE ? BG_WS_OBJECT_H_FLAG : BG_WS_OBJECT_A_FLAG);
    bool enemyBaseReady = enemyBase && enemyBase->isSpawned() && enemyBase->GetGoState() == GO_STATE_READY &&
                          warsong->GetFlagState(bg->GetOtherTeamId(bot->GetTeamId())) == BG_WS_FLAG_STATE_ON_BASE;
    constexpr float flagRoomCommitRadius = 100.0f;
    constexpr float flagObjectiveTolerance = 25.0f;
    PositionInfo objective = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    assignment.CommittedAttack = !assignment.Defender && !assignment.Escort && enemyBaseReady && objective.isSet() &&
                                 objective.mapId == bot->GetMapId() &&
                                 bot->IsWithinDistInMap(enemyBase, flagRoomCommitRadius) &&
                                 enemyBase->GetDistance(objective.x, objective.y, objective.z) < flagObjectiveTolerance;

    // Flexible bots respond by proximity to the public EFC marker, not by GUID
    // rank. Escorts and near-complete flag-room pushes remain committed.
    constexpr uint32 maxReturners = 3;
    ObjectGuid enemyCarrierGuid = warsong->GetFlagPickerGUID(bot->GetTeamId());
    Player* enemyCarrier =
        enemyCarrierGuid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(bot->GetMap(), enemyCarrierGuid);
    if (warsong->GetFlagState(bot->GetTeamId()) == BG_WS_FLAG_STATE_ON_PLAYER && enemyCarrier &&
        enemyCarrier->IsAlive() && enemyCarrier->IsInWorld() && enemyCarrier->GetMap() == bot->GetMap())
    {
        std::vector<std::pair<uint32, ObjectGuid>> responders;
        constexpr float returnDistanceBand = 25.0f;
        for (ObjectGuid const& guid : bots)
        {
            if (std::find(_escortGuids.begin(), _escortGuids.end(), guid) != _escortGuids.end())
                continue;
            Player* player = ObjectAccessor::GetPlayer(bot->GetMap(), guid);
            if (!player)
                continue;
            if (enemyBaseReady && player->IsWithinDistInMap(enemyBase, flagRoomCommitRadius))
                continue;
            // Coarse distance bands plus GUID ties avoid tiny-distance switches
            // without keeping incompatible per-bot responder histories.
            uint32 band = static_cast<uint32>(player->GetDistance2d(enemyCarrier) / returnDistanceBand);
            responders.emplace_back(band, guid);
        }
        // If everyone eligible is at the enemy room, still keep one responder
        // rather than silently losing all return coverage on a zero-defense team.
        if (responders.empty())
            for (ObjectGuid const& guid : bots)
            {
                if (std::find(_escortGuids.begin(), _escortGuids.end(), guid) != _escortGuids.end())
                    continue;
                Player* player = ObjectAccessor::GetPlayer(bot->GetMap(), guid);
                if (player)
                    responders.emplace_back(
                        static_cast<uint32>(player->GetDistance2d(enemyCarrier) / returnDistanceBand), guid);
            }
        std::sort(responders.begin(), responders.end());
        bool bothFlagsHeld = warsong->GetFlagState(bg->GetOtherTeamId(bot->GetTeamId())) == BG_WS_FLAG_STATE_ON_PLAYER;
        // In a standoff, every available non-carrier/non-escort bot helps return.
        // Keep the small response quota only while an offensive flag push is possible.
        uint32 returnQuota = bothFlagsHeld ? static_cast<uint32>(responders.size())
                                           : std::min<uint32>(maxReturners, std::max<uint32>(1, defenderQuota));
        for (uint32 i = 0; i < std::min<uint32>(returnQuota, responders.size()); ++i)
        {
            if (responders[i].second == bot->GetGUID())
                assignment.Returner = true;
        }
    }
    if (assignment.Returner)
        assignment.CommittedAttack = false;
    if (assignment.CommittedAttack)
        assignment.Returner = false;
    assignment.Valid = true;
    return assignment;
}

ObjectGuid WsgSupportTargetValue::Calculate()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg || bg->GetBgTypeID(true) != BATTLEGROUND_WS || bg->GetStatus() != STATUS_IN_PROGRESS || !bot->IsAlive())
        return ObjectGuid::Empty;
    if (static_cast<BattlegroundWS*>(bg)->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetTeamId())) == bot->GetGUID())
        return ObjectGuid::Empty;
    WsgTeamAssignment assignment = context->GetValue<WsgTeamAssignment>("wsg team assignment")->Get();
    if (!assignment.Valid || (!assignment.Defender && !assignment.Escort))
        return ObjectGuid::Empty;
    ObjectGuid result;
    float bestDistance = sPlayerbotAIConfig.SightDistance;
    for (ObjectGuid const& guid : AI_VALUE(GuidVector, "nearest enemy players"))
    {
        Player* enemy = ObjectAccessor::GetPlayer(bot->GetMap(), guid);
        if (!IsWsgSupportTarget(botAI, enemy))
            continue;
        float distance = bot->GetDistance(enemy);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            result = guid;
        }
    }
    return result;
}

namespace
{
// a game from the random queue records BATTLEGROUND_RB as the player's type: use the rolled map
BattlegroundTypeId RealBgType(Player* bot)
{
    BattlegroundTypeId bgType = bot->GetBattlegroundTypeId();
    if (bgType == BATTLEGROUND_RB && bot->GetBattleground())
        bgType = bot->GetBattleground()->GetBgTypeID(true);
    return bgType;
}
}  // namespace

Unit* FlagCarrierValue::Calculate()
{
    Unit* carrier = nullptr;

    if (botAI->GetBot()->InBattleground())
    {
        Battleground* battleground = bot->GetBattleground();
        if (battleground && battleground->GetBgTypeID(true) == BATTLEGROUND_WS)
        {
            BattlegroundWS* warsong = static_cast<BattlegroundWS*>(battleground);
            TeamId flagTeam = sameTeam ? battleground->GetOtherTeamId(bot->GetTeamId()) : bot->GetTeamId();
            ObjectGuid guid = warsong->GetFlagPickerGUID(flagTeam);
            carrier = guid.IsEmpty() ? nullptr : ObjectAccessor::GetPlayer(bot->GetMap(), guid);
            return IsValidWsgCarrier(bot, carrier, sameTeam) &&
                           (ignoreRange || bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig.SightDistance))
                       ? carrier
                       : nullptr;
        }

        if (RealBgType(botAI->GetBot()) == BATTLEGROUND_EY)
        {
            BattlegroundEY* bg = (BattlegroundEY*)botAI->GetBot()->GetBattleground();

            if (!bg)
                return nullptr;

            if (bg->GetFlagPickerGUID().IsEmpty())
                return nullptr;

            Player* fc = ObjectAccessor::GetPlayer(bg->GetBgMap(), bg->GetFlagPickerGUID());
            if (!fc)
                return nullptr;

            if (!sameTeam && (fc->GetTeamId() != bot->GetTeamId()))
                carrier = fc;

            if (sameTeam && (fc->GetTeamId() == bot->GetTeamId()))
                carrier = fc;

            if (carrier)
            {
                if (ignoreRange || bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig.SightDistance))
                {
                    return carrier;
                }
                else
                    return nullptr;
            }
        }
    }

    return carrier;
}

std::vector<CreatureData const*> BgMastersValue::Calculate()
{
    BattlegroundTypeId bgTypeId = (BattlegroundTypeId)stoi(qualifier);

    std::vector<uint32> entries;
    std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> battleMastersCache =
        sRandomPlayerbotMgr.getBattleMastersCache();
    entries.insert(entries.end(), battleMastersCache[TEAM_NEUTRAL][bgTypeId].begin(),
                   battleMastersCache[TEAM_NEUTRAL][bgTypeId].end());
    entries.insert(entries.end(), battleMastersCache[TEAM_ALLIANCE][bgTypeId].begin(),
                   battleMastersCache[TEAM_ALLIANCE][bgTypeId].end());
    entries.insert(entries.end(), battleMastersCache[TEAM_HORDE][bgTypeId].begin(),
                   battleMastersCache[TEAM_HORDE][bgTypeId].end());

    std::vector<CreatureData const*> bmGuids;

    for (auto entry : entries)
    {
        for (auto creaturePair : WorldPosition().getCreaturesNear(0, entry))
        {
            bmGuids.push_back(creaturePair);
        }
    }

    return bmGuids;
}

CreatureData const* BgMasterValue::Calculate()
{
    CreatureData const* bmPair = NearestBm(false);
    if (!bmPair)
        bmPair = NearestBm(true);

    return bmPair;
}

CreatureData const* BgMasterValue::NearestBm(bool allowDead)
{
    WorldPosition botPos(bot);

    std::vector<CreatureData const*> bmPairs = AI_VALUE2(std::vector<CreatureData const*>, "bg masters", qualifier);

    float rDist = 0.0f;
    CreatureData const* rbmPair = nullptr;

    for (auto& bmPair : bmPairs)
    {
        if (!bmPair)
            continue;

        WorldPosition bmPos(bmPair->mapid, bmPair->posX, bmPair->posY, bmPair->posZ, bmPair->orientation);

        float dist = botPos.distance(bmPos);  // This is the aproximate travel distance.

        // Did we already find a closer unit that is not dead?
        if (rbmPair && rDist <= dist)
            continue;

        CreatureTemplate const* bmTemplate = sObjectMgr->GetCreatureTemplate(bmPair->id);
        if (!bmTemplate)
            continue;

        FactionTemplateEntry const* bmFactionEntry = sFactionTemplateStore.LookupEntry(bmTemplate->faction);

        // Is the unit hostile?
        if (Unit::GetFactionReactionTo(bot->GetFactionTemplateEntry(), bmFactionEntry) < REP_NEUTRAL)
            continue;

        AreaTableEntry const* area = bmPos.getArea();

        if (!area)
            continue;

        // Is the area hostile?
        if (area->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE)
            continue;
        if (area->team == 2 && bot->GetTeamId() == TEAM_HORDE)
            continue;

        if (!allowDead)
        {
            Unit* unit = botAI->GetUnit(bmPair);

            if (!unit)
                continue;

            // Is the unit dead?
            if (unit->getDeathState() == DeathState::Dead)
                continue;
        }

        rbmPair = bmPair;
        rDist = dist;
    }

    return rbmPair;
}

BattlegroundTypeId RpgBgTypeValue::Calculate()
{
    GuidPosition guidPosition = AI_VALUE(GuidPosition, "rpg target");

    if (guidPosition)
        for (uint32 i = 1; i < MAX_BATTLEGROUND_QUEUE_TYPES; i++)
        {
            BattlegroundQueueTypeId queueTypeId = (BattlegroundQueueTypeId)i;

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);

            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bg)
                continue;

            if (bot->GetLevel() < bg->GetMinLevel())
                continue;

            // check if already in queue
            if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
                continue;

            std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> battleMastersCache =
                sRandomPlayerbotMgr.getBattleMastersCache();

            for (auto& entry : battleMastersCache[TEAM_NEUTRAL][bgTypeId])
                if (entry == guidPosition.GetEntry())
                    return bgTypeId;

            for (auto& entry : battleMastersCache[bot->GetTeamId()][bgTypeId])
                if (entry == guidPosition.GetEntry())
                    return bgTypeId;
        }

    return BATTLEGROUND_TYPE_NONE;
}
