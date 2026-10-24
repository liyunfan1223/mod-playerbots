/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EoEEncounter_Drakes.h"
#include "CreatureAI.h"
#include "EoEEncounter_Malygos.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "SpellAuraEffects.h"
#include "Timer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace
{
constexpr uint32 EOE_HEALER_ROSTER_CACHE_MS = 2000;

// Heading the flight holds on the ring, latched per instance. It only ever advances.
struct DrakeStackAngle
{
    uint32 at = 0;
    float angle = DRAKE_STACK_ANGLE;
};

struct HealerRosterEntry
{
    uint32 at = 0;
    std::vector<ObjectGuid> guids;
};

// When the boss picked this bot's drake, and when that was last confirmed.
struct DrakeFixate
{
    uint32 at = 0;
    uint32 seen = 0;
};

// Fixates are keyed on the bot, not its drake: a Skytalon is summoned fresh every pull, so drake keys
// would pile up for as long as the instance stays loaded.
struct DrakeInstanceState
{
    DrakeStackAngle stackAngle;
    HealerRosterEntry healerRoster;
    std::unordered_map<ObjectGuid, DrakeFixate> fixates;
};

RaidInstanceState<DrakeInstanceState> drakeStates;

DrakeInstanceState& DrakeStateFor(Player* bot) { return drakeStates.For(bot->GetInstanceId()); }

// Both come back -1 when there is no power cost worth checking, which callers read as yes.
bool GetDrakeSpellCost(Unit* drake, uint32 spellId, int32& cost, int32& available)
{
    cost = -1;
    available = -1;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    // POWER_HEALTH is -2, which would index the power array out of bounds.
    if (spellInfo->PowerType >= static_cast<uint32>(MAX_POWERS))
        return true;

    int32 const needed = spellInfo->CalcPowerCost(drake, spellInfo->GetSchoolMask());
    if (needed <= 0)
        return true;

    cost = needed;
    available = static_cast<int32>(drake->GetPower(Powers(spellInfo->PowerType)));
    return true;
}
}

void GetStaticFields(Player* bot, std::vector<Unit*>& fields)
{
    GetEoECreatures(bot, NPC_STATIC_FIELD, fields);
}

bool IsClearOfStaticFields(float x, float y, std::vector<Unit*> const& fields, float safeRadius)
{
    for (Unit* field : fields)
    {
        if (field->GetExactDist2d(x, y) < safeRadius)
            return false;
    }
    return true;
}

bool GetDrakeStackPoint(Player* bot, std::vector<Unit*> const& fields, float& x, float& y, float& z)
{
    if (!bot->GetVehicleBase())
        return false;

    Unit* boss = GetMalygos(bot);
    if (!boss)
        return false;

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();
    z = MALYGOS_P3_BOSS_Z;

    DrakeStackAngle& cachedAngle = DrakeStateFor(bot).stackAngle;
    uint32 const nowMs = getMSTime();
    if (cachedAngle.at && getMSTimeDiff(cachedAngle.at, nowMs) >= EOE_LATCH_STALE_MS)
        cachedAngle.angle = DRAKE_STACK_ANGLE;
    cachedAngle.at = nowMs;

    float& held = cachedAngle.angle;

    x = bossX + std::cos(held) * DRAKE_STACK_RADIUS;
    y = bossY + std::sin(held) * DRAKE_STACK_RADIUS;

    if (IsClearOfStaticFields(x, y, fields, STATIC_FIELD_CLEARANCE))
        return true;

    // Dodge along the ring, forward only. Sweeping back or restarting from DRAKE_STACK_ANGLE flips the
    // whole formation when a field behind the flight expires.
    float const step = EOE_TWO_PI / DRAKE_RING_HEADINGS;

    float bestClearance = 0.0f;
    float bestAngle = held;
    float bestX = x;
    float bestY = y;

    for (uint8 i = 1; i <= DRAKE_RING_HEADINGS; ++i)
    {
        float const angle = held + static_cast<float>(i) * step;
        float const cx = bossX + std::cos(angle) * DRAKE_STACK_RADIUS;
        float const cy = bossY + std::sin(angle) * DRAKE_STACK_RADIUS;

        float clearance = std::numeric_limits<float>::max();
        for (Unit* field : fields)
            clearance = std::min(clearance, field->GetExactDist2d(cx, cy));

        if (clearance >= STATIC_FIELD_CLEARANCE)
        {
            held = std::remainder(angle, EOE_TWO_PI);
            x = cx;
            y = cy;
            return true;
        }

        if (clearance > bestClearance)
        {
            bestClearance = clearance;
            bestAngle = angle;
            bestX = cx;
            bestY = cy;
        }
    }

    // Boxed in: take the roomiest heading rather than sit in the field.
    held = std::remainder(bestAngle, EOE_TWO_PI);
    x = bestX;
    y = bestY;
    return true;
}

void GetDrakeApproachPoint(Unit* drake, Unit* boss, float destX, float destY, float& x, float& y)
{
    x = destX;
    y = destY;
    if (!boss)
        return;

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();

    float const from = std::atan2(drake->GetPositionY() - bossY, drake->GetPositionX() - bossX);
    float const to = std::atan2(destY - bossY, destX - bossX);
    float delta = std::remainder(to - from, EOE_TWO_PI);

    // Forward only once on the ring, where the short way back crosses the field just left. Drakes still
    // flying in take the short way, or the flight arrives in two waves; under a hop behind is just drift.
    bool const onRing =
        std::fabs(drake->GetExactDist2d(bossX, bossY) - DRAKE_STACK_RADIUS) <= DRAKE_STACK_TOLERANCE;
    if (onRing && delta < -DRAKE_APPROACH_ARC)
        delta += EOE_TWO_PI;

    if (std::fabs(delta) <= DRAKE_APPROACH_ARC)
        return;

    float const angle = from + std::copysign(DRAKE_APPROACH_ARC, delta);
    x = bossX + std::cos(angle) * DRAKE_STACK_RADIUS;
    y = bossY + std::sin(angle) * DRAKE_STACK_RADIUS;
}

void GetDrakeHealerGuids(PlayerbotAI* botAI, std::vector<ObjectGuid>& out)
{
    out.clear();

    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
    {
        if (PlayerbotAI::IsHeal(bot))
            out.push_back(bot->GetGUID());
        return;
    }

    // Only group makeup and difficulty feed this, and the walk sorts two vectors, so it's cached.
    HealerRosterEntry& cached = DrakeStateFor(bot).healerRoster;
    uint32 const now = getMSTime();
    if (cached.at && getMSTimeDiff(cached.at, now) < EOE_HEALER_ROSTER_CACHE_MS)
    {
        out = cached.guids;
        return;
    }
    cached.at = now;
    cached.guids.clear();

    uint8 const wanted =
        bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL ? DRAKE_HEALERS_25MAN : DRAKE_HEALERS_10MAN;

    std::vector<ObjectGuid> healers;
    std::vector<ObjectGuid> others;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;

        (PlayerbotAI::IsHeal(member) ? healers : others).push_back(member->GetGUID());
    }

    // Guid order is identical on every bot, so the flight agrees on the roster without talking.
    std::sort(healers.begin(), healers.end());
    std::sort(others.begin(), others.end());

    for (size_t i = 0; i < wanted && i < healers.size(); ++i)
        cached.guids.push_back(healers[i]);

    if (cached.guids.size() < wanted)
    {
        size_t const shortfall = wanted - cached.guids.size();
        for (size_t i = 0; i < shortfall && i < others.size(); ++i)
            cached.guids.push_back(others[i]);
    }

    out = cached.guids;
}

bool IsDrakeHealer(PlayerbotAI* botAI, std::vector<ObjectGuid> const& healers)
{
    return std::find(healers.begin(), healers.end(), botAI->GetBot()->GetGUID()) != healers.end();
}

void GetDrakeFlightAndHealerRank(PlayerbotAI* botAI, std::vector<ObjectGuid> const& healers,
    std::vector<Unit*>& drakes, uint8& rank)
{
    drakes.clear();
    rank = 0;

    Player* bot = botAI->GetBot();
    Unit* own = bot->GetVehicleBase();

    Group* group = bot->GetGroup();
    if (!group)
    {
        if (own && own->GetEntry() == NPC_WYRMREST_SKYTALON)
            drakes.push_back(own);
        return;
    }

    ObjectGuid const ownGuid = bot->GetGUID();
    uint32 const ownEnergy = own ? own->GetPower(POWER_ENERGY) : 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;

        Unit* drake = member->GetVehicleBase();
        if (!drake || !drake->IsAlive() || drake->GetEntry() != NPC_WYRMREST_SKYTALON)
            continue;

        drakes.push_back(drake);

        if (!own || member->GetGUID() == ownGuid)
            continue;

        if (std::find(healers.begin(), healers.end(), member->GetGUID()) == healers.end())
            continue;

        uint32 const energy = drake->GetPower(POWER_ENERGY);
        if (energy > ownEnergy || (energy == ownEnergy && member->GetGUID() < ownGuid))
            ++rank;
    }
}

bool DrakeCanAfford(Unit* drake, uint32 spellId)
{
    int32 cost = 0;
    int32 available = 0;
    if (!GetDrakeSpellCost(drake, spellId, cost, available))
        return false;

    return cost < 0 || available >= cost;
}

bool DrakeCanAffordWithShield(Unit* drake, uint32 spellId)
{
    int32 cost = 0;
    int32 available = 0;
    if (!GetDrakeSpellCost(drake, spellId, cost, available))
        return false;
    if (cost < 0)
        return true;

    int32 shieldCost = 0;
    int32 shieldAvailable = 0;
    GetDrakeSpellCost(drake, SPELL_FLAME_SHIELD, shieldCost, shieldAvailable);

    return available >= cost + std::max(shieldCost, 0);
}

uint32 DrakeAuraRemainingMs(Unit* drake, uint32 spellId)
{
    if (!drake)
        return 0;

    Aura* aura = drake->GetAura(spellId);
    if (!aura)
        return 0;

    // Negative is a permanent aura, which none of these are.
    int32 const remaining = aura->GetDuration();
    return remaining > 0 ? static_cast<uint32>(remaining) : 0;
}

bool IsDrakeSurgeTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Unit* drake = bot->GetVehicleBase();
    if (!drake)
        return false;

    Unit* boss = GetMalygos(bot);
    if (!boss)
        return false;

    Creature* bossCreature = boss->ToCreature();
    if (!bossCreature || !bossCreature->AI())
        return false;

    // Both P3 surges (57407, and 60936 in 25-man) are DoCastAOE with no unit target, so the boss
    // publishes its victims in guid slots instead.
    for (uint8 i = 0; i < EOE_NUM_MAX_SURGE_TARGETS; ++i)
    {
        if (bossCreature->AI()->GetGUID(EOE_DATA_FIRST_SURGE_TARGET_GUID + i) == drake->GetGUID())
            return true;
    }
    return false;
}

bool GetDrakeSurgeElapsedMs(PlayerbotAI* botAI, uint32& elapsedMs)
{
    elapsedMs = 0;
    if (!IsDrakeSurgeTarget(botAI))
        return false;

    Player* const fixateBot = botAI->GetBot();
    DrakeFixate& fixate = DrakeStateFor(fixateBot).fixates[fixateBot->GetGUID()];
    uint32 const now = getMSTime();

    // A fresh pick shows as a gap in the flag, or, for back-to-back picks that leave no gap, as a flag
    // older than one SURGE_CYCLE_MS.
    if (!fixate.at || getMSTimeDiff(fixate.seen, now) > DRAKE_FIXATE_GAP_MS ||
        getMSTimeDiff(fixate.at, now) >= SURGE_CYCLE_MS)
        fixate.at = now;
    fixate.seen = now;

    elapsedMs = getMSTimeDiff(fixate.at, now);
    return true;
}
