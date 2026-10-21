/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DungeonCorpseRunAction.h"

#include <array>

#include "DBCStores.h"
#include "DungeonCorpseRunValue.h"
#include "InstanceSaveMgr.h"
#include "InstanceScript.h"
#include "MapMgr.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "WorldSession.h"

namespace
{
constexpr uint32 RUN_LIFETIME = 10 * MINUTE * IN_MILLISECONDS;
constexpr uint32 ENTRY_RETRY_DELAY = 5 * IN_MILLISECONDS;
constexpr uint8 MAX_ENTRY_ATTEMPTS = 3;
constexpr float ENTRANCE_CONFIRM_DISTANCE = 30.0f;

DungeonCorpseRunState& GetRun(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<DungeonCorpseRunState>("dungeon corpse run")->RefGet();
}

bool HasResurrectionSpell(Player* member)
{
    // Check learned ranks, rather than assuming every priest/paladin already knows resurrection.
    constexpr std::array<uint32, 5> firstRanks = {2006, 7328, 2008, 50769, 20484};
    for (uint32 firstRank : firstRanks)
    {
        // The ordinary druid bot resurrection strategy casts Revive, not Rebirth.
        if (firstRank == 20484 && !IsRealPlayer(member))
            continue;

        for (uint32 spellId = firstRank; spellId; spellId = sSpellMgr->GetNextSpellInChain(spellId))
        {
            if (!member->HasSpell(spellId) || member->HasSpellCooldown(spellId))
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            bool hasReagents = true;
            if (!member->CanNoReagentCast(spellInfo))
            {
                for (uint32 i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    if (spellInfo->Reagent[i] > 0 &&
                        !member->HasItemCount(spellInfo->Reagent[i], spellInfo->ReagentCount[i]))
                    {
                        hasReagents = false;
                        break;
                    }
                }
            }

            if (hasReagents)
                return true;
        }
    }

    return false;
}

bool HasResurrectionOption(Player* bot)
{
    if (bot->isResurrectRequested() || bot->GetUInt32Value(PLAYER_SELF_RES_SPELL) || bot->HasPreventResurectionAura())
    {
        return true;
    }

    for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && member->IsInWorld() && !member->IsBeingTeleported() &&
            member->GetMap() == bot->GetMap() && bot->InSamePhase(member) && HasResurrectionSpell(member))
        {
            return true;
        }
    }

    return false;
}

Player* GetRunMaster(PlayerbotAI* botAI, DungeonCorpseRunState const& run)
{
    Player* bot = botAI->GetBot();
    Player* master = botAI->GetMaster();
    return master && master->GetGUID() == run.MasterGuid && IsRealPlayer(master) && master->GetSession() &&
                   !master->GetSession()->PlayerLogout() && master->IsInWorld() && !master->IsDuringRemoveFromWorld() &&
                   bot->GetGroup() && bot->GetGroup()->GetGUID() == run.GroupGuid &&
                   master->GetGroup() == bot->GetGroup()
               ? master
               : nullptr;
}

bool IsMasterInInstance(Player* master, DungeonCorpseRunState const& run)
{
    return master->IsAlive() && !master->IsBeingTeleported() && master->GetMapId() == run.MapId &&
           master->GetInstanceId() == run.InstanceId && master->GetMap()->IsNonRaidDungeon();
}

bool IsGroupFighting(Player* master)
{
    if (InstanceScript* instance = master->GetInstanceScript())
        if (instance->IsEncounterInProgress())
            return true;

    for (GroupReference* ref = master->GetGroup()->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsInWorld() && member->GetMap() == master->GetMap() && member->IsInCombat())
            return true;
    }

    return false;
}

bool IsBusy(Player* player)
{
    return player->IsBeingTeleported() || player->IsInFlight() || player->IsFlying() || player->IsFalling() ||
           player->GetTransport() || player->GetVehicle() || player->IsInCombat() ||
           player->IsNonMeleeSpellCast(false) || player->GetTradeData() || player->duel;
}

bool IsExpired(DungeonCorpseRunState const& run)
{
    if (run.ResurrectedAt)
    {
        uint32 catchUpLifetime = (sPlayerbotAIConfig.DungeonCorpseRunTeleportDelay + 5 * MINUTE) * IN_MILLISECONDS;
        return getMSTimeDiff(run.ResurrectedAt, getMSTime()) >= catchUpLifetime;
    }

    return getMSTimeDiff(run.StartedAt, getMSTime()) >= RUN_LIFETIME;
}
}  // namespace

bool DungeonCorpseRun::IsEligible(PlayerbotAI* botAI)
{
    if (!sPlayerbotAIConfig.DungeonCorpseRunEnabled || !botAI)
        return false;

    Player* bot = botAI->GetBot();
    Player* master = botAI->GetMaster();
    return bot && !bot->IsAlive() && bot->IsInWorld() && !bot->InBattleground() && !bot->InArena() && !IsSelfBot(bot) &&
           bot->GetMap() && bot->GetMap()->IsNonRaidDungeon() && bot->GetGroup() && master && master != bot &&
           IsRealPlayer(master) && master->IsInWorld() && master->GetGroup() == bot->GetGroup() &&
           master->GetMap() == bot->GetMap();
}

bool DungeonCorpseRun::ShouldRelease(PlayerbotAI* botAI)
{
    if (!IsEligible(botAI) || HasResurrectionOption(botAI->GetBot()))
        return false;

    AreaTriggerTeleport const* entrance = sObjectMgr->GetMapEntranceTrigger(botAI->GetBot()->GetMapId());
    return entrance && MapMgr::IsValidMapCoord(entrance->target_mapId, entrance->target_X, entrance->target_Y,
                                               entrance->target_Z, entrance->target_Orientation);
}

bool DungeonCorpseRun::PrepareRelease(PlayerbotAI* botAI)
{
    if (!ShouldRelease(botAI))
        return false;

    Player* bot = botAI->GetBot();
    AreaTriggerTeleport const* entrance = sObjectMgr->GetMapEntranceTrigger(bot->GetMapId());
    DungeonCorpseRunState& run = GetRun(botAI);
    run = {};
    run.MasterGuid = botAI->GetMaster()->GetGUID();
    run.GroupGuid = bot->GetGroup()->GetGUID();
    run.MapId = bot->GetMapId();
    run.InstanceId = bot->GetInstanceId();
    run.StartedAt = getMSTime();
    run.EntranceX = entrance->target_X;
    run.EntranceY = entrance->target_Y;
    run.EntranceZ = entrance->target_Z;
    run.EntranceOrientation = entrance->target_Orientation;
    return true;
}

bool DungeonCorpseRun::IsPending(PlayerbotAI* botAI)
{
    if (!sPlayerbotAIConfig.DungeonCorpseRunEnabled || !botAI->GetBot()->GetGroup() ||
        !IsRealPlayer(botAI->GetMaster()))
        return false;

    DungeonCorpseRunState const& run = GetRun(botAI);
    return run.MasterGuid && !IsExpired(run);
}

bool DungeonCorpseRunAction::isUseful()
{
    return sPlayerbotAIConfig.DungeonCorpseRunEnabled && !bot->IsAlive() && bot->HasPlayerFlag(PLAYER_FLAGS_GHOST) &&
           bot->GetGroup() && IsRealPlayer(botAI->GetMaster()) && GetRun(botAI).MasterGuid;
}

bool DungeonCorpseRunAction::Execute(Event /*event*/)
{
    if (bot->IsBeingTeleported() || !bot->IsInWorld())
        return false;

    DungeonCorpseRunState& run = GetRun(botAI);
    Player* master = GetRunMaster(botAI, run);
    if (!sPlayerbotAIConfig.DungeonCorpseRunEnabled || !master || IsExpired(run) || bot->InBattleground() ||
        bot->InArena() || bot->GetCorpseLocation().GetMapId() != run.MapId || run.Attempts >= MAX_ENTRY_ATTEMPTS ||
        (bot->GetMap()->Instanceable() && (bot->GetMapId() != run.MapId || bot->GetInstanceId() != run.InstanceId)) ||
        (master && master->IsAlive() && master->GetMapId() == run.MapId && master->GetInstanceId() != run.InstanceId))
    {
        run = {};
        return false;
    }

    if (!bot->HasPlayerFlag(PLAYER_FLAGS_GHOST) || bot->IsAlive() || bot->isResurrectRequested() ||
        !IsMasterInInstance(master, run) || !bot->InSamePhase(master) || IsGroupFighting(master) ||
        (run.Attempts && getMSTimeDiff(run.LastAttemptAt, getMSTime()) < ENTRY_RETRY_DELAY))
    {
        return false;
    }

    Difficulty difficulty = bot->GetDifficulty(false);
    if (!GetDownscaledMapDifficultyData(run.MapId, difficulty) || difficulty != master->GetMap()->GetDifficulty() ||
        sInstanceSaveMgr->PlayerGetDestinationInstanceId(bot, run.MapId, difficulty) != run.InstanceId)
    {
        run = {};
        return false;
    }

    ++run.Attempts;
    run.LastAttemptAt = getMSTime();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    // Force a worldport even if a graveyard is on the dungeon map. The normal core ACK performs resurrection.
    run.EntranceRequested = bot->TeleportTo(run.MapId, run.EntranceX, run.EntranceY, run.EntranceZ,
                                            run.EntranceOrientation, 0, nullptr, true);
    return run.EntranceRequested;
}

bool DungeonCorpseRunCatchUpAction::isUseful()
{
    return sPlayerbotAIConfig.DungeonCorpseRunEnabled && bot->IsAlive() && bot->GetGroup() &&
           IsRealPlayer(botAI->GetMaster()) && bot->GetMap() && bot->GetMap()->IsNonRaidDungeon() &&
           GetRun(botAI).MasterGuid;
}

bool DungeonCorpseRunCatchUpAction::Execute(Event /*event*/)
{
    if (!bot->IsAlive() || bot->IsBeingTeleported() || !bot->IsInWorld())
        return false;

    DungeonCorpseRunState& run = GetRun(botAI);
    Player* master = GetRunMaster(botAI, run);
    if (!sPlayerbotAIConfig.DungeonCorpseRunEnabled || !master || IsExpired(run) || !run.EntranceRequested ||
        bot->GetMapId() != run.MapId || bot->GetInstanceId() != run.InstanceId)
    {
        run = {};
        return false;
    }

    if (!run.ResurrectedAt)
    {
        if (!bot->IsWithinDist3d(run.EntranceX, run.EntranceY, run.EntranceZ, ENTRANCE_CONFIRM_DISTANCE))
        {
            run = {};
            return false;
        }

        run.ResurrectedAt = getMSTime();
        return true;
    }

    if (getMSTimeDiff(run.ResurrectedAt, getMSTime()) <
        sPlayerbotAIConfig.DungeonCorpseRunTeleportDelay * IN_MILLISECONDS)
    {
        return false;
    }

    if (!IsMasterInInstance(master, run) || !bot->InSamePhase(master) || IsBusy(bot) || IsBusy(master) ||
        IsGroupFighting(master))
    {
        return false;
    }

    if (bot->IsWithinDistInMap(master, sPlayerbotAIConfig.DungeonCorpseRunTeleportDistance))
    {
        run = {};
        return false;
    }

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    if (!bot->TeleportTo(run.MapId, master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                         master->GetOrientation(), 0, master))
    {
        return false;
    }

    run = {};
    return true;
}
