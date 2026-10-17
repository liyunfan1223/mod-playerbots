/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSA_H
#define PLAYERBOTS_RSA_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RSScripts.h"
#include "RSShared.h"
#include "RSStrategy.h"
#include "RSTriggers.h"
#include "RtiTargetValue.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <set>
#include <utility>
#include <vector>

inline const Position RS_SAVIANA_TANK_POSITION = Position(3152.36f, 619.4369f, 78.645874f);
inline const Position RS_ZARITHRIAN_TANK_POSITION = Position(3049.7f, 528.125f, 89.5219f);
inline const Position RS_BALTHARUS_TANK_POSITION = Position(3138.6873f, 381.2857f, 87.57249f);
inline const Position RS_BALTHARUS_HEALER_POSITION = Position(3133.5005f, 414.67023f, 85.509735f);
inline const Position RS_HALION_TANK_POSITION = Position(3141.0532f, 532.5485f, 72.88935f);
inline const Position RS_HALION_CENTER_POSITION = Position(3156.0f, 533.811f, 72.9882f);
inline const Position RS_HALION_TANK_METEOR_SPOT = Position(3170.5989f, 530.64606f, 72.887024f);
inline const Position RS_HALION_COMBUSTION_SPOT_A = Position(3186.9033f, 497.85657f, 72.289635f);
inline const Position RS_HALION_COMBUSTION_SPOT_B = Position(3164.8992f, 580.4305f, 72.873024f);
inline const Position RS_HALION_METEOR_SPOT_A = Position(3167.8013f, 510.99084f, 72.85342f);
inline const Position RS_HALION_METEOR_SPOT_B = Position(3160.1658f, 553.1889f, 72.88719f);
inline const Position RS_HALION_METEOR_MID = Position(3158.5593f, 534.84015f, 72.88689f);

inline float RsAngleDiff(float a, float b)
{
    float diff = a - b;
    while (diff > static_cast<float>(M_PI))
        diff -= 2.0f * static_cast<float>(M_PI);
    while (diff < -static_cast<float>(M_PI))
        diff += 2.0f * static_cast<float>(M_PI);
    return diff;
}

inline bool RsSpotInFrontArc(Unit* caster, float x, float y, float range)
{
    if (!caster || !caster->IsAlive())
        return false;

    float const dx = x - caster->GetPositionX();
    float const dy = y - caster->GetPositionY();
    if (std::sqrt(dx * dx + dy * dy) > range)
        return false;

    float const diff = RsAngleDiff(std::atan2(dy, dx), caster->GetOrientation());
    return std::fabs(diff) <= static_cast<float>(M_PI) / 2.0f;
}

inline bool RsCastClassTaunt(PlayerbotAI* botAI, Player* bot, Unit* target)
{
    if (!target || !target->IsAlive())
        return false;

    bool const threatReset = botAI->HasCheat(BotCheatMask::raid);

    if (threatReset)
        RsForceThreat(target, bot);

    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            if (threatReset)
                bot->RemoveSpellCooldown(RS_SPELL_TAUNT_PALADIN, true);
            if (botAI->CastSpell("hand of reckoning", target))
                return true;
            break;
        case CLASS_DEATH_KNIGHT:
            if (threatReset)
                bot->RemoveSpellCooldown(RS_SPELL_TAUNT_DK, true);
            if (botAI->CastSpell("dark command", target))
                return true;
            break;
        case CLASS_DRUID:
            if (threatReset)
                bot->RemoveSpellCooldown(RS_SPELL_TAUNT_DRUID, true);
            if (botAI->CastSpell("growl", target))
                return true;
            break;
        case CLASS_WARRIOR:
            if (threatReset)
                bot->RemoveSpellCooldown(RS_SPELL_TAUNT_WARRIOR, true);
            if (botAI->CastSpell("taunt", target))
                return true;
            break;
        default:
            break;
    }

    if (botAI->CastSpell("shoot", target) || botAI->CastSpell("throw", target))
        return true;

    return false;
}

inline float RsHalionBeamSignedDist(float px, float py, Unit* a, Unit* b)
{
    float const ax = a->GetPositionX();
    float const ay = a->GetPositionY();
    float const ex = b->GetPositionX() - ax;
    float const ey = b->GetPositionY() - ay;
    float const len = std::sqrt(ex * ex + ey * ey);
    if (len < 0.01f)
        return std::numeric_limits<float>::max();

    return ((px - ax) * ey - (py - ay) * ex) / len;
}

inline bool RsHalionEngaged(PlayerbotAI* botAI)
{
    Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "halion")->Get();
    return boss && boss->IsAlive() && boss->IsInCombat() &&
           (boss->GetEntry() == NPC_HALION || boss->GetEntry() == NPC_TWILIGHT_HALION);
}

inline Unit* RsHalionPhase1Boss(PlayerbotAI* botAI)
{
    Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "halion")->Get();
    if (!boss || !boss->IsAlive() || boss->GetEntry() != NPC_HALION)
        return nullptr;

    if (!boss->HealthAbovePct(75))
        return nullptr;

    if (boss->GetDisplayId() != boss->GetNativeDisplayId())
        return nullptr;

    return boss;
}

inline bool RsHalionInTwilight(Player* bot)
{
    return bot && bot->HasAura(SPELL_TWILIGHT_REALM);
}

inline Unit* RsHalionFindBoss(PlayerbotAI* botAI, std::string const& name, uint32 entry)
{
    Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
    if (boss && boss->IsAlive() && boss->GetEntry() == entry)
        return boss;

    return RsFindTarget(botAI, [entry](Unit* unit) { return unit->GetEntry() == entry; });
}

inline Unit* RsHalionTwilightBoss(PlayerbotAI* botAI)
{
    ObjectGuid const guid = RubySanctumHelpers::RsState(botAI->GetBot()->GetInstanceId()).cutterTiming.bossGuid;

    if (!guid.IsEmpty())
        if (Unit* boss = botAI->GetUnit(guid); boss && boss->IsAlive() && boss->GetEntry() == NPC_TWILIGHT_HALION)
            return boss;

    return RsHalionFindBoss(botAI, "twilight halion", NPC_TWILIGHT_HALION);
}

inline Unit* RsHalionPhase2Boss(PlayerbotAI* botAI)
{
    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss || !boss->HealthAbovePct(50))
        return nullptr;

    return boss;
}

inline Unit* RsHalionAnyPhysicalBoss(PlayerbotAI* botAI)
{
    ObjectGuid const guid =
        RubySanctumHelpers::RsState(botAI->GetBot()->GetInstanceId()).halionCorporeality.physicalGuid;

    if (!guid.IsEmpty())
        if (Unit* boss = botAI->GetUnit(guid); boss && boss->IsAlive() && boss->GetEntry() == NPC_HALION)
            return boss;

    return RsHalionFindBoss(botAI, "halion", NPC_HALION);
}

inline bool RsHalionIsPhase3(PlayerbotAI* botAI)
{
    Unit* physical = RsHalionAnyPhysicalBoss(botAI);
    if (!physical || physical->HealthAbovePct(50))
        return false;
    return RsHalionTwilightBoss(botAI) != nullptr;
}

inline constexpr uint8 RS_HALION_CORP_BALANCED = 5;
inline constexpr uint8 RS_HALION_CORP_STOP_AT = 4;
inline constexpr uint8 RS_HALION_CORP_STOP_ALL_AT = 2;
inline constexpr uint32 RS_HALION_CORP_FRESH_MS = 3000;

inline bool RsHalionOwnCorpIndex(PlayerbotAI* botAI, Player* bot, uint8& outIndex)
{
    if (!RsHalionIsPhase3(botAI))
        return false;

    RubySanctumHelpers::HalionCorporeality const& corp =
        RubySanctumHelpers::RsState(bot->GetInstanceId()).halionCorporeality;
    bool const inTwilight = RsHalionInTwilight(bot);
    uint32 const ownStamp = inTwilight ? corp.twilightStamp : corp.physicalStamp;
    if (ownStamp == 0 || getMSTimeDiff(ownStamp, getMSTime()) > RS_HALION_CORP_FRESH_MS)
        return false;

    outIndex = inTwilight ? corp.twilightIndex : corp.physicalIndex;
    return true;
}

inline bool RsHalionRealmThrottled(PlayerbotAI* botAI, Player* bot)
{
    uint8 ownIndex = RS_HALION_CORP_BALANCED;
    if (!RsHalionOwnCorpIndex(botAI, bot, ownIndex))
        return false;

    bool& held = RubySanctumHelpers::RsState(bot->GetInstanceId()).realmThrottled[bot->GetGUID()];

    if (ownIndex <= RS_HALION_CORP_STOP_AT)
        held = true;
    else if (ownIndex >= RS_HALION_CORP_BALANCED)
        held = false;

    return held;
}

inline bool RsHalionLeadingTooMuch(PlayerbotAI* botAI, Player* bot)
{
    uint8 ownIndex = RS_HALION_CORP_BALANCED;
    if (!RsHalionOwnCorpIndex(botAI, bot, ownIndex))
        return false;

    return ownIndex <= RS_HALION_CORP_STOP_ALL_AT;
}

inline bool RsHalionInThrottledHalf(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return true;

    uint32 const instanceId = bot->GetInstanceId();
    bool const inTwilight = RsHalionInTwilight(bot);
    ObjectGuid const selfGuid = bot->GetGUID();

    uint32 total = 0;
    uint32 rank = 0;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member->GetInstanceId() != instanceId)
            continue;
        if (!GET_PLAYERBOT_AI(member))
            continue;
        if (PlayerbotAI::IsTank(member) || PlayerbotAI::IsHeal(member))
            continue;
        if (RsHalionInTwilight(member) != inTwilight)
            continue;

        ++total;
        if (member->GetGUID() < selfGuid)
            ++rank;
    }

    return rank < total / 2;
}

inline bool RsHalionP3TwilightAssigned(Player* bot)
{
    if (PlayerbotAI::IsMainTank(bot))
        return false;
    if (PlayerbotAI::IsTank(bot))
        return true;

    std::map<ObjectGuid, bool>& assignment = RubySanctumHelpers::RsState(bot->GetInstanceId()).p3TwilightAssignment;
    ObjectGuid const selfGuid = bot->GetGUID();

    auto it = assignment.find(selfGuid);
    if (it != assignment.end())
        return it->second;

    Group* group = bot->GetGroup();
    if (!group)
    {
        assignment[selfGuid] = false;
        return false;
    }

    std::vector<ObjectGuid> melee, ranged, heals;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || PlayerbotAI::IsTank(member))
            continue;
        ObjectGuid const guid = member->GetGUID();
        if (PlayerbotAI::IsHeal(member))
            heals.push_back(guid);
        else if (PlayerbotAI::IsRanged(member))
            ranged.push_back(guid);
        else
            melee.push_back(guid);
    }

    auto assignHalf = [&](std::vector<ObjectGuid>& guids)
    {
        std::sort(guids.begin(), guids.end());
        int const half = static_cast<int>(guids.size()) / 2;
        for (int i = 0; i < static_cast<int>(guids.size()); ++i)
            assignment[guids[i]] = (i >= half);
    };

    for (ObjectGuid const& guid : ranged)
        assignment[guid] = false;
    for (ObjectGuid const& guid : melee)
        assignment[guid] = true;

    assignHalf(heals);

    it = assignment.find(selfGuid);
    return it != assignment.end() && it->second;
}

inline GameObject* RsHalionFindPortal(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    constexpr float SEARCH_RANGE = 200.0f;

    if (RsHalionInTwilight(bot))
        return bot->FindNearestGameObject(GO_HALION_PORTAL_4, SEARCH_RANGE, true);

    if (GameObject* go = bot->FindNearestGameObject(GO_HALION_PORTAL_1, SEARCH_RANGE, true))
        return go;
    if (GameObject* go = bot->FindNearestGameObject(GO_HALION_PORTAL_2, SEARCH_RANGE, true))
        return go;
    if (GameObject* go = bot->FindNearestGameObject(GO_HALION_PORTAL_3, SEARCH_RANGE, true))
        return go;
    return bot->FindNearestGameObject(GO_HALION_PORTAL_4, SEARCH_RANGE, true);
}

inline constexpr uint32 RS_ZARITHRIAN_CLEAVE_SWAP_STACKS = 5;
inline constexpr float RS_BALTHARUS_TANK_FACING = 0.0f;
inline constexpr float RS_BALTHARUS_HEALER_REACH = 5.0f;
inline constexpr float RS_BALTHARUS_BRAND_RANGE = 14.0f;

class RsBaltharusBrandAction : public MoveAwayFromPlayerWithDebuffAction
{
public:
    RsBaltharusBrandAction(PlayerbotAI* botAI)
        : MoveAwayFromPlayerWithDebuffAction(botAI, "rs baltharus brand", SPELL_ENERVATING_BRAND,
                                             RS_BALTHARUS_BRAND_RANGE) {}
    bool Execute(Event event) override;
};

class RsBaltharusHealerPositionAction : public AttackAction
{
public:
    RsBaltharusHealerPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs baltharus healer position") {}
    bool Execute(Event event) override;
};

class RsBaltharusAvoidFrontAction : public AttackAction
{
public:
    RsBaltharusAvoidFrontAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs baltharus avoid front") {}
    bool Execute(Event event) override;
};

class RsBaltharusTankPositionAction : public AttackAction
{
public:
    RsBaltharusTankPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs baltharus tank position") {}
    bool Execute(Event event) override;

private:
    bool HoldAt(Position const& spot, Unit* faceAwayFrom);
};

class RsSavianaConflagrationAction : public MoveAwayFromPlayerWithDebuffAction
{
public:
    RsSavianaConflagrationAction(PlayerbotAI* botAI)
        : MoveAwayFromPlayerWithDebuffAction(botAI, "rs saviana conflagration", SPELL_FLAME_BEACON, 10.0f) {}
    bool Execute(Event event) override;
};

class RsSavianaTankPositionAction : public AttackAction
{
public:
    RsSavianaTankPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs saviana tank position") {}
    bool Execute(Event event) override;
};

class RsSavianaAvoidFrontAction : public AttackAction
{
public:
    RsSavianaAvoidFrontAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs saviana avoid front") {}
    bool Execute(Event event) override;
};

class RsSavianaMeleeSpreadAction : public AttackAction
{
public:
    RsSavianaMeleeSpreadAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs saviana melee spread") {}
    bool Execute(Event event) override;
};

class RsZarithrianAddsAction : public AttackAction
{
public:
    RsZarithrianAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs zarithrian adds") {}
    bool Execute(Event event) override;

private:
    Unit* FindPriorityAdd();
};

class RsZarithrianTankAction : public AttackAction
{
public:
    RsZarithrianTankAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs zarithrian tank") {}
    bool Execute(Event event) override;

private:
    uint32 GetCleaveStacks(Unit* unit);
    Unit* FindBoss();
    void CollectAdds(std::vector<Unit*>& adds);
    bool HoldBoss(Unit* boss);
    bool RunAddsToBoss(Unit* boss, std::vector<Unit*> const& adds);
};

inline constexpr float RS_TRASH_TANK_SPREAD = 20.0f;
inline constexpr float RS_TRASH_RANGED_DIST = 25.0f;
inline constexpr float RS_TRASH_RANGED_TOL = 10.0f;
inline constexpr float RS_TRASH_RANGED_STEP = 7.0f;
inline constexpr float RS_TRASH_TANK_SPACE_TOL = 5.0f;
inline constexpr float RS_TRASH_DETECT_RANGE = 40.0f;
inline constexpr float RS_TRASH_TANK_GRAB_RANGE = 100.0f;

inline bool RsTrashIsCommander(uint32 entry)
{
    return entry == NPC_CHARSCALE_COMMANDER || entry == NPC_CHARSCALE_COMMANDER_H;
}

inline bool RsTrashIsInvoker(uint32 entry)
{
    return entry == NPC_CHARSCALE_INVOKER || entry == NPC_CHARSCALE_INVOKER_H;
}

inline bool RsTrashIsCharscale(uint32 entry)
{
    switch (entry)
    {
        case NPC_CHARSCALE_INVOKER:
        case NPC_CHARSCALE_INVOKER_H:
        case NPC_CHARSCALE_ASSAULTER:
        case NPC_CHARSCALE_ASSAULTER_H:
        case NPC_CHARSCALE_ELITE:
        case NPC_CHARSCALE_ELITE_H:
        case NPC_CHARSCALE_COMMANDER:
        case NPC_CHARSCALE_COMMANDER_H:
            return true;
        default:
            return false;
    }
}

inline bool RsTrashIsTrashEntry(uint32 entry)
{
    return RsTrashIsCharscale(entry) || entry == NPC_ONYX_FLAMECALLER;
}

inline int RsTrashPriorityRank(uint32 entry)
{
    switch (entry)
    {
        case NPC_CHARSCALE_COMMANDER:
        case NPC_CHARSCALE_COMMANDER_H:
            return 0;
        case NPC_CHARSCALE_INVOKER:
        case NPC_CHARSCALE_INVOKER_H:
            return 1;
        case NPC_ONYX_FLAMECALLER:
            return 2;
        case NPC_CHARSCALE_ASSAULTER:
        case NPC_CHARSCALE_ASSAULTER_H:
        case NPC_CHARSCALE_ELITE:
        case NPC_CHARSCALE_ELITE_H:
            return 3;
        default:
            return std::numeric_limits<int>::max();
    }
}

inline void RsTrashCollect(PlayerbotAI* botAI, std::vector<Unit*>& out)
{
    RsCollectTargets(botAI, out, [](Unit* unit) { return RsTrashIsTrashEntry(unit->GetEntry()); });
}

inline bool RsTrashActive(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || !bot->IsInCombat())
        return false;

    return RsFindTarget(botAI, [bot](Unit* unit)
    {
        return RsTrashIsCharscale(unit->GetEntry()) && bot->GetExactDist2d(unit) <= RS_TRASH_DETECT_RANGE;
    }) != nullptr;
}

class RsTrashAddsAction : public AttackAction
{
public:
    RsTrashAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs trash adds") {}
    bool Execute(Event event) override;

private:
    Unit* FindPriorityAdd();
};

class RsTrashTankAction : public AttackAction
{
public:
    RsTrashTankAction(PlayerbotAI* botAI, std::string const name) : AttackAction(botAI, name) {}

protected:
    bool HoldAt(std::vector<Unit*> const& assigned, float spotX, float spotY, float spotZ, bool moveToSpot,
                bool hasAway, float awayOri);
};

class RsTrashMainTankAction : public RsTrashTankAction
{
public:
    RsTrashMainTankAction(PlayerbotAI* botAI) : RsTrashTankAction(botAI, "rs trash main tank") {}
    bool Execute(Event event) override;
};

class RsTrashAssistTankAction : public RsTrashTankAction
{
public:
    RsTrashAssistTankAction(PlayerbotAI* botAI) : RsTrashTankAction(botAI, "rs trash assist tank") {}
    bool Execute(Event event) override;
};

class RsTrashRangedAction : public AttackAction
{
public:
    RsTrashRangedAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs trash ranged") {}
    bool Execute(Event event) override;
};

inline float RsHalionTargetOrientation()
{
    float angle = std::atan2(RS_HALION_TANK_POSITION.GetPositionY() - RS_HALION_CENTER_POSITION.GetPositionY(),
                             RS_HALION_TANK_POSITION.GetPositionX() - RS_HALION_CENTER_POSITION.GetPositionX());
    if (angle < 0.0f)
        angle += 2.0f * static_cast<float>(M_PI);
    return angle;
}

inline constexpr float RS_HALION_COMBUSTION_REACH = 4.0f;

inline constexpr float RS_HALION_COMBUSTION_RANGE = 10.0f;

inline bool RsHalionHasCombustion(Unit* u)
{
    return u && (u->HasAura(SPELL_FIERY_COMBUSTION) || u->HasAura(SPELL_MARK_OF_COMBUSTION));
}

inline Position const& RsHalionCombustionSpot(uint32 instanceId)
{
    uint32 const count = RubySanctumHelpers::RsState(instanceId).meteorPingPong.count;
    return (count % 2u) == 0u ? RS_HALION_COMBUSTION_SPOT_A : RS_HALION_COMBUSTION_SPOT_B;
}

inline Position const& RsHalionCombustionSpot(Player* bot)
{
    return RsHalionCombustionSpot(bot->GetInstanceId());
}

inline bool RsHalionCombustionAtSpot(Unit* u)
{
    if (!u)
        return false;

    Player* p = u->ToPlayer();
    if (!p)
        return false;

    Position const& spot = RsHalionCombustionSpot(p);
    return p->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= RS_HALION_COMBUSTION_REACH;
}

inline constexpr float RS_HALION_COMBUSTION_PLAYER_SAFE_DIST = 35.0f;

inline bool RsHalionCombustionPlayerSafe(Player* p)
{
    if (!p)
        return false;

    return p->GetExactDist2d(RS_HALION_CENTER_POSITION.GetPositionX(),
                             RS_HALION_CENTER_POSITION.GetPositionY()) >=
           RS_HALION_COMBUSTION_PLAYER_SAFE_DIST;
}

inline bool RsHalionCombustionNotClear(Player* bot)
{
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && RsHalionHasCombustion(member) && !RsHalionCombustionAtSpot(member))
            return true;
    }
    return false;
}

inline bool RsHalionIsCombustionDispeller(PlayerbotAI* botAI)
{
    return PlayerbotAI::IsHeal(botAI->GetBot()) && RsHalionCombustionNotClear(botAI->GetBot());
}

inline constexpr float RS_HALION_CONSUMPTION_OUT_DIST = 46.0f;
inline constexpr float RS_HALION_CONSUMPTION_OUT_REACH = 4.0f;
inline constexpr float RS_HALION_CONSUMPTION_STEP = 10.0f;
inline constexpr float RS_HALION_CONSUMPTION_POOL_CLEAR = 9.0f;

inline bool RsHalionHasConsumption(Unit* u)
{
    return u && (u->HasAura(SPELL_MARK_OF_CONSUMPTION) || u->HasAura(SPELL_SOUL_CONSUMPTION));
}

inline void RsHalionCollectHazardPools(Unit* from, std::vector<Unit*>& pools)
{
    for (uint32 const entry : {NPC_CONSUMPTION, NPC_COMBUSTION})
    {
        std::list<Creature*> found;
        from->GetCreatureListWithEntryInGrid(found, entry, 100.0f);
        for (Creature* c : found)
            if (c && c->IsAlive())
                pools.push_back(c);
    }
}

inline bool RsHalionSpotClearOfPools(std::vector<Unit*> const& pools, float px, float py)
{
    for (Unit* pool : pools)
        if (pool->GetExactDist2d(px, py) <= RS_HALION_CONSUMPTION_POOL_CLEAR)
            return false;
    return true;
}

inline bool RsHalionPathClearOfPools(std::vector<Unit*> const& pools, float fromX, float fromY,
                                     float toX, float toY)
{
    float const dx = toX - fromX;
    float const dy = toY - fromY;
    float const lenSq = dx * dx + dy * dy;

    for (Unit* pool : pools)
    {
        float const px = pool->GetPositionX();
        float const py = pool->GetPositionY();

        float t = 0.0f;
        if (lenSq > 0.0001f)
            t = std::clamp(((px - fromX) * dx + (py - fromY) * dy) / lenSq, 0.0f, 1.0f);

        float const nearX = px - (fromX + dx * t);
        float const nearY = py - (fromY + dy * t);
        if (std::sqrt(nearX * nearX + nearY * nearY) <= RS_HALION_CONSUMPTION_POOL_CLEAR)
            return false;
    }
    return true;
}

inline bool RsHalionConsumptionAtSpot(PlayerbotAI* botAI, Player* member)
{
    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss || !member)
        return false;

    return member->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY()) >=
           RS_HALION_CONSUMPTION_OUT_DIST - RS_HALION_CONSUMPTION_OUT_REACH;
}

inline bool RsHalionConsumptionNotClear(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && RsHalionHasConsumption(member) &&
            !RsHalionConsumptionAtSpot(botAI, member))
            return true;
    }
    return false;
}

inline constexpr uint32 RS_HALION_CONSUMPTION_HOLD_PCT = 75;
inline constexpr float RS_HALION_CONSUMPTION_HEAL_PCT = 90.0f;

inline Player* RsHalionConsumptionHealTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* worst = nullptr;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || !RsHalionHasConsumption(member))
            continue;
        if (RsHalionInTwilight(member) != RsHalionInTwilight(bot))
            continue;
        if (member->GetHealthPct() >= RS_HALION_CONSUMPTION_HEAL_PCT)
            continue;
        if (!worst || member->GetHealthPct() < worst->GetHealthPct())
            worst = member;
    }
    return worst;
}

inline bool RsHalionIsAdd(Unit* unit)
{
    return unit->GetEntry() == NPC_LIVING_INFERNO || unit->GetEntry() == NPC_LIVING_EMBER;
}

inline void RsHalionCollectAdds(PlayerbotAI* botAI, std::vector<Unit*>& adds)
{
    RsCollectTargets(botAI, adds, RsHalionIsAdd);
}

inline bool RsHalionAnyAddAlive(PlayerbotAI* botAI)
{
    return RsFindTarget(botAI, RsHalionIsAdd) != nullptr;
}

inline constexpr uint32 RS_HALION_PORTAL_ADD_CLEAR_MS = 5000;

inline bool RsHalionPortalHeldForAdds(PlayerbotAI* botAI)
{
    if (!RsHalionFindPortal(botAI))
        return false;

    RubySanctumHelpers::RsInstanceState& state = RubySanctumHelpers::RsState(botAI->GetBot()->GetInstanceId());

    bool const bossHigh = state.bossHealth.stamp != 0 &&
                          GetMSTimeDiffToNow(state.bossHealth.stamp) <= RS_HALION_CORP_FRESH_MS &&
                          state.bossHealth.pct >= 65;
    if (!bossHigh)
        return false;

    if (RsHalionAnyAddAlive(botAI))
        return true;

    RubySanctumHelpers::PortalAddGate& gate = state.portalAddGate;
    uint32 const now = getMSTime();

    if (gate.armTime == 0)
        gate.armTime = now;

    uint32 const since = std::max(gate.armTime, gate.lastAddAliveTime);
    return getMSTimeDiff(since, now) < RS_HALION_PORTAL_ADD_CLEAR_MS;
}

inline uint32 RsHalionPortalAddHoldRemainingMs(PlayerbotAI* botAI)
{
    if (RsHalionAnyAddAlive(botAI))
        return RS_HALION_PORTAL_ADD_CLEAR_MS;

    RubySanctumHelpers::PortalAddGate const& gate =
        RubySanctumHelpers::RsState(botAI->GetBot()->GetInstanceId()).portalAddGate;
    if (gate.armTime == 0 && gate.lastAddAliveTime == 0)
        return RS_HALION_PORTAL_ADD_CLEAR_MS;

    uint32 const since = std::max(gate.armTime, gate.lastAddAliveTime);
    uint32 const elapsed = getMSTimeDiff(since, getMSTime());
    return elapsed >= RS_HALION_PORTAL_ADD_CLEAR_MS ? 0 : RS_HALION_PORTAL_ADD_CLEAR_MS - elapsed;
}

inline bool RsHalionCutterShouldMove(uint32 instanceId);

inline bool RsHalionEnteringTwilight(PlayerbotAI* botAI, Player* bot)
{
    if (PlayerbotAI::IsMainTank(bot) || RsHalionInTwilight(bot))
        return false;

    Unit* physBoss = RsHalionAnyPhysicalBoss(botAI);
    bool const phase3 = physBoss != nullptr && !physBoss->HealthAbovePct(50);
    if (phase3 && !RsHalionP3TwilightAssigned(bot))
        return false;

    if (RsHalionPortalHeldForAdds(botAI))
        return false;

    if (RsHalionCutterShouldMove(bot->GetInstanceId()))
        return false;

    return RsHalionFindPortal(botAI) != nullptr;
}

template <typename Filter>
inline Player* RsHalionPickTank(Group* group, Filter const& filter)
{
    Player* assist = nullptr;
    Player* anyTank = nullptr;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member) || !filter(member))
            continue;

        if (!anyTank || member->GetGUID() < anyTank->GetGUID())
            anyTank = member;

        if (PlayerbotAI::IsAssistTank(member) && (!assist || member->GetGUID() < assist->GetGUID()))
            assist = member;
    }

    return assist ? assist : anyTank;
}

inline Player* RsHalionAddTank(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return nullptr;

    return RsHalionPickTank(group, [](Player* member) { return !RsHalionInTwilight(member); });
}

inline Player* RsHalionTwilightTankUncached(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
    {
        Player* bot = botAI->GetBot();
        return (PlayerbotAI::IsTank(bot) && bot->IsAlive() && RsHalionInTwilight(bot)) ? bot : nullptr;
    }

    return RsHalionPickTank(group, [](Player* member) { return RsHalionInTwilight(member); });
}

inline Player* RsHalionTwilightTank(PlayerbotAI* botAI)
{
    Player* selfBot = botAI->GetBot();
    if (!selfBot->GetGroup())
        return RsHalionTwilightTankUncached(botAI);

    RubySanctumHelpers::TankCache& slot = RubySanctumHelpers::RsState(selfBot->GetInstanceId()).twilightTankCache;
    uint32 const now = getMSTime();
    if (slot.stamp == now)
    {
        if (slot.guid.IsEmpty())
            return nullptr;
        if (Player* cached = botAI->GetPlayer(slot.guid); cached && cached->IsAlive())
            return cached;
    }

    Player* result = RsHalionTwilightTankUncached(botAI);
    slot.stamp = now;
    slot.guid = result ? result->GetGUID() : ObjectGuid::Empty;
    return result;
}

inline Player* RsHalionFirstCrosser(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
    {
        Player* bot = botAI->GetBot();
        return (PlayerbotAI::IsTank(bot) && bot->IsAlive() && !PlayerbotAI::IsMainTank(bot)) ? bot : nullptr;
    }

    return RsHalionPickTank(group, [](Player* member) { return !PlayerbotAI::IsMainTank(member); });
}

inline Player* RsHalionLiveMainTank(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
    {
        Player* bot = botAI->GetBot();
        return (PlayerbotAI::IsMainTank(bot) && bot->IsAlive()) ? bot : nullptr;
    }

    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && PlayerbotAI::IsMainTank(member))
            return member;
    }

    return nullptr;
}

inline Player* RsHalionBossTankUncached(PlayerbotAI* botAI)
{
    if (Player* mt = RsHalionLiveMainTank(botAI))
        return mt;

    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return nullptr;

    Player* anyTank = nullptr;
    for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member))
            continue;

        if (!anyTank || member->GetGUID() < anyTank->GetGUID())
            anyTank = member;
    }

    return anyTank;
}

inline Player* RsHalionBossTank(PlayerbotAI* botAI)
{
    Player* selfBot = botAI->GetBot();
    if (!selfBot->GetGroup())
        return RsHalionBossTankUncached(botAI);

    RubySanctumHelpers::TankCache& slot = RubySanctumHelpers::RsState(selfBot->GetInstanceId()).bossTankCache;
    uint32 const now = getMSTime();
    if (slot.stamp == now)
    {
        if (slot.guid.IsEmpty())
            return nullptr;
        if (Player* cached = botAI->GetPlayer(slot.guid); cached && cached->IsAlive())
            return cached;
    }

    Player* result = RsHalionBossTankUncached(botAI);
    slot.stamp = now;
    slot.guid = result ? result->GetGUID() : ObjectGuid::Empty;
    return result;
}

inline bool RsHalionTanksAdds(PlayerbotAI* botAI, Player* bot)
{
    return RsHalionAnyAddAlive(botAI) && RsHalionAddTank(botAI) == bot;
}

inline bool RsHalionAssistTankAsMelee(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!RsHalionAnyPhysicalBoss(botAI))
        return false;

    if (RsHalionBossTank(botAI) == bot)
        return false;

    return !RsHalionTanksAdds(botAI, bot);
}

inline constexpr float RS_HALION_TANK_LEASH = 10.0f;
inline constexpr float RS_HALION_METEOR_LEASH = 25.0f;
inline constexpr float RS_HALION_METEOR_REACH = 4.0f;

inline Position const& RsHalionMeteorSpot(uint32 instanceId)
{
    uint32 const count = RubySanctumHelpers::RsState(instanceId).meteorPingPong.count;
    return (count % 2u) == 0u ? RS_HALION_METEOR_SPOT_A : RS_HALION_METEOR_SPOT_B;
}

inline constexpr uint32 RS_HALION_METEOR_RECENT_MS = 5000;

inline bool RsHalionMeteorCastRecently(uint32 instanceId)
{
    uint32 const lastCast = RubySanctumHelpers::RsState(instanceId).meteorPingPong.lastCastTime;
    return lastCast != 0 && GetMSTimeDiffToNow(lastCast) <= RS_HALION_METEOR_RECENT_MS;
}

inline bool RsHalionMeteorTargetedBossTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (RsHalionBossTank(botAI) != bot)
        return false;

    uint32 const t = RubySanctumHelpers::RsState(bot->GetInstanceId()).meteorPingPong.tankMeteorTime;
    return t != 0 && GetMSTimeDiffToNow(t) <= RS_HALION_METEOR_RECENT_MS;
}

inline bool RsHalionTankMeteorCommitted(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    RubySanctumHelpers::RsInstanceState& state = RubySanctumHelpers::RsState(bot->GetInstanceId());
    auto& committed = state.meteorCommitted;
    ObjectGuid const guid = bot->GetGUID();

    if (RsHalionBossTank(botAI) != bot)
    {
        committed.erase(guid);
        return false;
    }

    uint32 const homeTime = state.meteorPingPong.tankMeteorTime;
    uint32 const returnTime = state.meteorPingPong.tankReturnTime;

    bool const homeRecent = homeTime != 0 && GetMSTimeDiffToNow(homeTime) <= RS_HALION_METEOR_RECENT_MS;
    bool const returnRecent = returnTime != 0 && GetMSTimeDiffToNow(returnTime) <= RS_HALION_METEOR_RECENT_MS;

    if (returnRecent && returnTime >= homeTime)
    {
        committed.erase(guid);
        return false;
    }

    if (homeRecent)
        committed.insert(guid);

    return committed.count(guid) != 0;
}

inline bool RsHalionMeteorShouldRally(Player* bot)
{
    auto& committed = RubySanctumHelpers::RsState(bot->GetInstanceId()).rallyCommitted;
    ObjectGuid const guid = bot->GetGUID();

    if (RsHalionInTwilight(bot))
    {
        committed.erase(guid);
        return false;
    }

    float const dist = bot->GetExactDist2d(RsHalionMeteorSpot(bot->GetInstanceId()));

    if (dist <= RS_HALION_METEOR_REACH)
    {
        committed.erase(guid);
        return false;
    }

    if (dist > RS_HALION_METEOR_LEASH)
        committed.insert(guid);

    return committed.count(guid) != 0;
}

inline bool RsHalionCombustionReturning(Player* bot)
{
    auto& returning = RubySanctumHelpers::RsState(bot->GetInstanceId()).combustionReturning;
    ObjectGuid const guid = bot->GetGUID();

    if (RsHalionHasCombustion(bot))
    {
        returning.insert(guid);
        return false;
    }

    if (!returning.count(guid))
        return false;

    Position const& spot = RsHalionMeteorSpot(bot->GetInstanceId());
    if (bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) <= RS_HALION_METEOR_REACH)
    {
        returning.erase(guid);
        return false;
    }

    return true;
}

inline constexpr float RS_HALION_LINE_MELEE_MIN = 5.0f;
inline constexpr float RS_HALION_LINE_MELEE_MAX = 18.0f;
inline constexpr float RS_HALION_LINE_RANGED_MIN = 20.0f;
inline constexpr float RS_HALION_LINE_RANGED_MAX = 30.0f;
inline constexpr float RS_HALION_LINE_OFFSET_OK = 4.0f;
inline constexpr float RS_HALION_LINE_LEASH = 10.0f;

inline constexpr float RS_HALION_CUTTER_HALF_WIDTH = 2.5f;
inline constexpr float RS_HALION_CUTTER_MARGIN = 4.0f;
inline constexpr float RS_HALION_CUTTER_BOT_CLEAR = 5.0f;
inline constexpr float RS_HALION_CUTTER_FLANK_DIST = 15.0f;
inline constexpr float RS_HALION_CUTTER_SAFE_DIST = 15.0f;
inline constexpr float RS_HALION_CUTTER_TANK_SAFE_DIST = 15.0f;
inline constexpr float RS_HALION_CUTTER_TANK_BEAM_GAP = 12.0f;
inline constexpr float RS_HALION_CUTTER_TANK_TRAIL = 3.0f;
inline constexpr float RS_HALION_CUTTER_TANK_TRAIL_TOL = 1.0f;
inline constexpr float RS_HALION_CUTTER_TANK_TRAIL_NORMAL = 8.0f;
inline constexpr float RS_HALION_CUTTER_LEADER_NEAR = 5.0f;
inline constexpr float RS_HALION_CUTTER_LEADER_FAR = 8.0f;

inline constexpr uint32 RS_HALION_CUTTER_CYCLE_MS = 29000;
inline constexpr uint32 RS_HALION_CUTTER_LEAD_MS = 5000;
inline constexpr uint32 RS_HALION_FIRST_SHOOT_MS = 21000;

inline bool RsHalionCutterShouldMove(uint32 instanceId)
{
    RubySanctumHelpers::CutterTiming const& state = RubySanctumHelpers::RsState(instanceId).cutterTiming;

    if (state.lastShootTime == 0)
    {
        if (state.active)
            return true;
        if (state.encounterStart == 0)
            return false;

        uint32 const sinceStart = GetMSTimeDiffToNow(state.encounterStart);
        if (sinceStart < RS_HALION_FIRST_SHOOT_MS)
            return sinceStart >= RS_HALION_FIRST_SHOOT_MS - RS_HALION_CUTTER_LEAD_MS;

        uint32 const intoCycle = (sinceStart - RS_HALION_FIRST_SHOOT_MS) % RS_HALION_CUTTER_CYCLE_MS;
        return intoCycle >= RS_HALION_CUTTER_CYCLE_MS - RS_HALION_CUTTER_LEAD_MS;
    }

    uint32 const since = GetMSTimeDiffToNow(state.lastShootTime);

    if (state.active)
        return true;

    uint32 const intoCycle = since % RS_HALION_CUTTER_CYCLE_MS;
    return intoCycle >= RS_HALION_CUTTER_CYCLE_MS - RS_HALION_CUTTER_LEAD_MS;
}

inline bool RsHalionCutterActive(uint32 instanceId)
{
    return RubySanctumHelpers::RsState(instanceId).cutterTiming.active;
}

inline uint32 RsHalionCutterMsUntilShoot(uint32 instanceId)
{
    RubySanctumHelpers::CutterTiming const& state = RubySanctumHelpers::RsState(instanceId).cutterTiming;
    if (state.active)
        return 0;

    if (state.lastShootTime == 0)
    {
        if (state.encounterStart == 0)
            return RS_HALION_CUTTER_CYCLE_MS;
        uint32 const elapsed = GetMSTimeDiffToNow(state.encounterStart);
        return elapsed >= RS_HALION_FIRST_SHOOT_MS ? 0 : RS_HALION_FIRST_SHOOT_MS - elapsed;
    }

    uint32 const since = GetMSTimeDiffToNow(state.lastShootTime);
    uint32 const intoCycle = since % RS_HALION_CUTTER_CYCLE_MS;
    return RS_HALION_CUTTER_CYCLE_MS - intoCycle;
}

inline bool RsHalionCollectOrbPairs(Unit* boss, std::vector<std::pair<Unit*, Unit*>>& pairs)
{
    Creature* carrier = boss->FindNearestCreature(NPC_ORB_CARRIER, 200.0f);
    if (!carrier)
        return false;

    Vehicle* vehicle = carrier->GetVehicleKit();
    if (!vehicle)
        return false;

    Unit* north = vehicle->GetPassenger(0);
    Unit* south = vehicle->GetPassenger(1);
    if (north && south)
        pairs.emplace_back(north, south);

    Unit* east = vehicle->GetPassenger(2);
    Unit* west = vehicle->GetPassenger(3);
    if (east && west)
        pairs.emplace_back(east, west);

    return !pairs.empty();
}

inline constexpr float RS_HALION_P2_MELEE_DIST = 18.0f;
inline constexpr float RS_HALION_P2_RANGED_DIST = 25.0f;
inline constexpr float RS_HALION_P2_STEP = 7.0f;
inline constexpr float RS_HALION_CUTTER_DANGER = 7.0f;

inline bool RsHalionCutterBeamDanger(PlayerbotAI* botAI, Player* bot)
{
    if (!RsHalionInTwilight(bot) || !RsHalionCutterShouldMove(bot->GetInstanceId()))
        return false;

    auto& cache = RubySanctumHelpers::RsState(bot->GetInstanceId()).cutterDangerCache;
    uint32 const now = getMSTime();
    auto it = cache.find(bot->GetGUID());
    if (it != cache.end() && getMSTimeDiff(it->second.first, now) < 250)
        return it->second.second;

    bool danger = false;
    if (Unit* boss = RsHalionTwilightBoss(botAI))
    {
        std::vector<std::pair<Unit*, Unit*>> pairs;
        if (RsHalionCollectOrbPairs(boss, pairs))
        {
            for (auto const& pair : pairs)
            {
                if (std::fabs(RsHalionBeamSignedDist(bot->GetPositionX(), bot->GetPositionY(),
                                                     pair.first, pair.second)) <= RS_HALION_CUTTER_DANGER)
                {
                    danger = true;
                    break;
                }
            }
        }
    }

    cache[bot->GetGUID()] = {now, danger};
    return danger;
}

inline constexpr float RS_HALION_PORTAL_FINISH_DIST = 15.0f;
inline constexpr uint32 RS_HALION_PORTAL_COMMIT_MARGIN_MS = 1200;

inline bool RsHalionPortalCommit(PlayerbotAI* botAI, Player* bot)
{
    if (!RsHalionInTwilight(bot))
        return false;

    Unit* twilightBoss = RsHalionTwilightBoss(botAI);
    if (!twilightBoss || twilightBoss->HealthAbovePct(50))
        return false;
    if (RsHalionP3TwilightAssigned(bot))
        return false;
    if (RsHalionHasConsumption(bot))
        return false;

    GameObject* portal = RsHalionFindPortal(botAI);
    if (!portal)
        return false;

    float const dist = bot->GetExactDist2d(portal->GetPositionX(), portal->GetPositionY());
    if (dist <= RS_HALION_PORTAL_FINISH_DIST)
        return true;

    if (RsHalionCutterActive(bot->GetInstanceId()))
        return false;

    float speed = bot->GetSpeed(MOVE_RUN);
    if (speed < 1.0f)
        speed = 7.0f;
    uint32 const reachMs = static_cast<uint32>((dist / speed) * IN_MILLISECONDS);
    return reachMs + RS_HALION_PORTAL_COMMIT_MARGIN_MS <= RsHalionCutterMsUntilShoot(bot->GetInstanceId());
}

class RsHalionTankPositionAction : public AttackAction
{
public:
    RsHalionTankPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion tank position") {}
    bool Execute(Event event) override;
};

class RsHalionAvoidConesAction : public AttackAction
{
public:
    RsHalionAvoidConesAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion avoid cones") {}
    bool Execute(Event event) override;
};

class RsHalionCombustionAction : public MoveAwayFromPlayerWithDebuffAction
{
public:
    RsHalionCombustionAction(PlayerbotAI* botAI)
        : MoveAwayFromPlayerWithDebuffAction(botAI, "rs halion combustion", SPELL_MARK_OF_COMBUSTION,
                                             RS_HALION_COMBUSTION_RANGE) {}
    bool Execute(Event event) override;
};

class RsHalionMeteorAction : public AttackAction
{
public:
    RsHalionMeteorAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion meteor") {}
    bool Execute(Event event) override;
};

class RsHalionAddsAction : public AttackAction
{
public:
    RsHalionAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion adds") {}
    bool Execute(Event event) override;
};

class RsHalionAddTankAction : public AttackAction
{
public:
    RsHalionAddTankAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion add tank") {}
    bool Execute(Event event) override;
};

class RsHalionStartPositionAction : public AttackAction
{
public:
    RsHalionStartPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion start position") {}
    bool Execute(Event event) override;
};

class RsHalionEnterPortalAction : public AttackAction
{
public:
    RsHalionEnterPortalAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion enter portal") {}
    bool Execute(Event event) override;
};

class RsHalionP2TankPositionAction : public AttackAction
{
public:
    RsHalionP2TankPositionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion p2 tank position") {}
    bool Execute(Event event) override;
};

class RsHalionP2AvoidConesAction : public AttackAction
{
public:
    RsHalionP2AvoidConesAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion p2 avoid cones") {}
    bool Execute(Event event) override;
};

class RsHalionConsumptionAction : public AttackAction
{
public:
    RsHalionConsumptionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion consumption") {}
    bool Execute(Event event) override;
};

class RsHalionCutterAction : public AttackAction
{
public:
    RsHalionCutterAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion cutter") {}
    bool Execute(Event event) override;
};

class RsHalionHealConsumptionAction : public AttackAction
{
public:
    RsHalionHealConsumptionAction(PlayerbotAI* botAI) : AttackAction(botAI, "rs halion heal consumption") {}
    bool Execute(Event event) override;
};

#endif
