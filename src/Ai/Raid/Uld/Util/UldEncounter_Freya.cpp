/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Freya.h"

#include "AttackersValue.h"
#include "CellImpl.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "UldScripts.h"
#include "Unit.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <vector>

using namespace EncounterHelpers;

namespace
{
// True when atMs already holds this tick's answer; otherwise stamps it and the caller refills.
bool FreshThisTick(uint32& atMs)
{
    uint32 const now = getMSTime();
    if (atMs && atMs == now)
        return true;

    atMs = now;
    return false;
}

// The check behind "nearest npcs", with the entry tested first.
struct FreyaStalkerInRangeCheck
{
    Acore::AnyUnitInObjectRangeCheck inRange;

    bool operator()(Unit* unit)
    {
        uint32 const entry = unit->GetEntry();

        return (entry == NPC_HEALTHY_SPORE || entry == NPC_FREYA_SUN_BEAM ||
                entry == NPC_FREYA_UNSTABLE_SUN_BEAM) &&
               inRange(unit);
    }
};

// The check behind "possible targets no los", with the entry tested first.
struct FreyaUnfriendlyOfEntryCheck
{
    Acore::AnyUnfriendlyUnitInObjectRangeCheck inRange;
    uint32 entry;

    bool operator()(Unit* unit) { return unit->GetEntry() == entry && inRange(unit); }
};
}  // namespace

const Position ULDUAR_FREYA_TANK_ANCHOR = Position(2360.0847f, -43.1235f, 425.333f);

std::vector<Unit*> FreyaWaveState::LivingTrio() const
{
    std::vector<Unit*> living;
    for (Unit* member : {snaplasher, stormLasher, waterSpirit})
    {
        if (member && member->IsAlive())
            living.push_back(member);
    }

    return living;
}

bool FreyaWaveState::TrioLocked() const
{
    for (Unit* member : LivingTrio())
    {
        if (member->GetHealthPct() < ULDUAR_FREYA_TRIO_SYNC_WINDOW_PCT)
            return true;
    }

    return false;
}

bool FreyaWaveState::TrioReleased() const
{
    std::vector<Unit*> const living = LivingTrio();
    if (living.empty())
        return false;

    for (Unit* member : living)
    {
        if (member->GetHealthPct() > ULDUAR_FREYA_TRIO_FLOOR_RELEASE_PCT)
            return false;
    }

    return true;
}

void GatherFreyaWaveState(PlayerbotAI* botAI, FreyaWaveState& state)
{
    // Two waves of the same add can be up at once. Keeping the more damaged one finishes the older wave first and,
    // since scan order isn't stable, stops the split flipping between identical adds.
    auto const keepMoreDamaged = [](Unit*& slot, Unit* candidate)
    {
        if (!slot || candidate->GetHealth() < slot->GetHealth())
            slot = candidate;
    };

    // "possible targets" enforces line of sight, which drops adds behind Freya's tree trunks out of
    // the scan and makes the split disagree between bots standing on opposite sides.
    for (ObjectGuid const& guid : GetFreyaScan(botAI).PossibleTargets())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        switch (unit->GetEntry())
        {
            case NPC_EONARS_GIFT:
                keepMoreDamaged(state.eonarsGift, unit);
                break;
            case NPC_ANCIENT_CONSERVATOR:
                keepMoreDamaged(state.conservator, unit);
                break;
            case NPC_SNAPLASHER:
                keepMoreDamaged(state.snaplasher, unit);
                break;
            case NPC_STORM_LASHER:
                keepMoreDamaged(state.stormLasher, unit);
                break;
            case NPC_ANCIENT_WATER_SPIRIT:
                keepMoreDamaged(state.waterSpirit, unit);
                break;
            case NPC_DETONATING_LASHER:
                state.detonatingLashers.push_back(unit);
                break;
            default:
                break;
        }
    }
}

bool FreyaTrioSyncSuppress(FreyaWaveState const& state, Unit* target)
{
    if (!target)
        return false;

    if (target != state.snaplasher && target != state.stormLasher && target != state.waterSpirit)
        return false;

    std::vector<Unit*> const living = state.LivingTrio();

    // Below three the window is already open and counting down; holding damage back now only lets the
    // ones already dead come back.
    if (living.size() < 3)
        return false;

    if (state.TrioReleased())
        return false;

    if (target->GetHealthPct() > ULDUAR_FREYA_TRIO_HARD_FLOOR_PCT)
        return false;

    for (Unit* member : living)
    {
        if (member != target && member->GetHealthPct() > ULDUAR_FREYA_TRIO_FLOOR_RELEASE_PCT)
            return true;
    }

    return false;
}

Unit* GetFreyaTrioAssignment(PlayerbotAI* botAI, FreyaWaveState const& state)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    std::vector<Unit*> candidates;
    for (Unit* member : state.LivingTrio())
    {
        if (!FreyaTrioSyncSuppress(state, member))
            candidates.push_back(member);
    }

    // Shouldn't happen, the release check clears the floor once the last member joins the band. Without this a bot
    // with nothing to hit would fall through to the boss.
    if (candidates.empty())
        candidates = state.LivingTrio();

    if (candidates.empty())
        return nullptr;

    std::vector<uint32> assigned(candidates.size(), 0);
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsDps(member))
            continue;

        // Join whichever member would carry the most remaining health per attacker. Over the group that splits
        // bots in proportion to remaining health, which has to reach zero at the same time.
        size_t pick = 0;
        float best = -1.0f;
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            float const share = float(candidates[i]->GetHealth()) / float(assigned[i] + 1);
            if (share > best)
            {
                best = share;
                pick = i;
            }
        }

        ++assigned[pick];

        if (member == bot)
            return candidates[pick];
    }

    return nullptr;
}

Unit* GetFreyaRangedLasherFocus(FreyaWaveState const& state)
{
    Unit* best = nullptr;
    for (Unit* lasher : state.detonatingLashers)
    {
        if (!lasher || !lasher->IsAlive())
            continue;

        // GUID breaks the tie so a wave of untouched lashers does not resolve differently per bot.
        if (!best || lasher->GetHealth() < best->GetHealth() ||
            (lasher->GetHealth() == best->GetHealth() && lasher->GetGUID() < best->GetGUID()))
            best = lasher;
    }

    return best;
}

Unit* GetFreyaLocalLasherTarget(PlayerbotAI* botAI, FreyaWaveState const& state, Unit* currentTarget, float range)
{
    Player* bot = botAI->GetBot();

    Unit* selected = nullptr;
    if (currentTarget && currentTarget->IsAlive() && currentTarget->GetEntry() == NPC_DETONATING_LASHER &&
        bot->GetExactDist2d(currentTarget) <= range)
        selected = currentTarget;

    // The margin stops two lashers at similar range from trading the bot back and forth every tick.
    constexpr float switchMargin = 10.0f;
    for (Unit* candidate : state.detonatingLashers)
    {
        if (!candidate || !candidate->IsAlive() || candidate == selected)
            continue;

        if (bot->GetExactDist2d(candidate) > range)
            continue;

        if (!selected)
        {
            selected = candidate;
            continue;
        }

        if (candidate->GetExactDist2d(bot) + switchMargin < selected->GetExactDist2d(bot))
            selected = candidate;
    }

    return selected;
}

Unit* GetFreyaConservatorSpore(PlayerbotAI* botAI, Unit* conservator)
{
    if (!conservator || !conservator->IsAlive())
        return nullptr;

    std::list<Creature*> found;
    conservator->GetCreatureListWithEntryInGrid(found, NPC_HEALTHY_SPORE, ULDUAR_FREYA_SPORE_SEARCH_RADIUS);

    // Nearest the Conservator, not the caller, so the tank and the melee agree. Self-stabilising: the parked spore
    // sits at ~0 yd until it despawns, and new ones spawn 20 yd out.
    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* spore : found)
    {
        if (!spore || !spore->IsAlive())
            continue;

        float const dist = spore->GetExactDist2d(conservator);
        if (!best || dist < bestDist)
        {
            best = spore;
            bestDist = dist;
        }
    }

    return best;
}

std::vector<Unit*> GetFreyaSpores(PlayerbotAI* botAI)
{
    std::vector<Unit*> spores;
    for (ObjectGuid const& guid : GetFreyaScan(botAI).Stalkers())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_HEALTHY_SPORE)
            spores.push_back(unit);
    }

    return spores;
}

Unit* GetFreyaTargetSpore(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    // Melee take the parked spore, not their own nearest. Anywhere else the DPS node drags them back out of the
    // aura to reach the boss, and the two nodes fight all wave.
    if (PlayerbotAI::IsMelee(bot))
        if (Unit* parked = GetFreyaConservatorSpore(botAI, GetFreyaScanUnitByEntry(botAI, NPC_ANCIENT_CONSERVATOR)))
            return parked;

    // Ranged and healers: nearest spore with room, since nearest outright piles them all on the melee spore. No
    // latch, the trigger stops on the aura. Skip spores holding a beam pool, it covers 5 yd of the spore's 6.
    std::vector<Position> const beams = GetFreyaSunBeamPositions(botAI, ULDUAR_FREYA_HAZARD_SEARCH_RADIUS);
    auto const pooled = [&beams](Unit* spore)
    {
        for (Position const& beam : beams)
            if (spore->GetExactDist2d(beam.GetPositionX(), beam.GetPositionY()) < ULDUAR_FREYA_SPORE_RADIUS)
                return true;

        return false;
    };

    Unit* nearest = nullptr;
    Unit* clean = nullptr;
    Unit* roomy = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    float cleanDistance = std::numeric_limits<float>::max();
    float roomyDistance = std::numeric_limits<float>::max();
    for (Unit* spore : GetFreyaSpores(botAI))
    {
        float const distance = bot->GetDistance2d(spore);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearest = spore;
        }

        if (pooled(spore))
            continue;

        if (distance < cleanDistance)
        {
            cleanDistance = distance;
            clean = spore;
        }

        if (distance < roomyDistance &&
            CountFreyaRaidNear(botAI, spore->GetPosition(), ULDUAR_FREYA_SPORE_RADIUS, bot) <
                ULDUAR_FREYA_SPORE_CROWD)
        {
            roomyDistance = distance;
            roomy = spore;
        }
    }

    // Every spore full, or every one pooled: both happen with three alive against fifteen bots that need one.
    // Sheltered and stacked, or sheltered and burning, still beats pacified.
    if (roomy)
        return roomy;

    return clean ? clean : nearest;
}

uint32 CountFreyaRaidNear(PlayerbotAI* botAI, Position const& centre, float radius, Player* except)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == except || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (member->GetExactDist2d(centre.GetPositionX(), centre.GetPositionY()) <= radius)
            ++count;
    }

    return count;
}

Unit* GetFreyaNaturesFuryShelter(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    Unit* best = nullptr;
    float bestDistance = std::numeric_limits<float>::max();
    for (Unit* spore : GetFreyaSpores(botAI))
    {
        // The carrier itself does not count against the spore it is running to.
        if (CountFreyaRaidNear(botAI, spore->GetPosition(), ULDUAR_FREYA_NATURES_FURY_RADIUS, bot))
            continue;

        float const distance = bot->GetDistance2d(spore);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = spore;
        }
    }

    return best;
}

// Highest health percent first, never a suppressed member. Percent since the thresholds are percent and max health
// differs nearly 2x, so absolute health calls the Snaplasher furthest from dying even when it's closest.
static Unit* GetFreyaTankTrioTarget(FreyaWaveState const& state, Unit* currentTarget)
{
    Unit* best = nullptr;
    for (Unit* member : state.LivingTrio())
    {
        if (FreyaTrioSyncSuppress(state, member))
            continue;

        if (!best || member->GetHealthPct() > best->GetHealthPct())
            best = member;
    }

    if (!best || !currentTarget || currentTarget == best)
        return best;

    // Hold the member the tank is already on until another is clear of it by the margin, or its own
    // damage closing the gap makes it swap every few ticks.
    for (Unit* member : state.LivingTrio())
    {
        if (member != currentTarget || FreyaTrioSyncSuppress(state, member))
            continue;

        if (best->GetHealthPct() - member->GetHealthPct() < ULDUAR_FREYA_TANK_TRIO_SWITCH_PCT)
            return member;

        break;
    }

    return best;
}

Unit* GetFreyaTankTarget(PlayerbotAI* botAI, FreyaWaveState const& state, Unit* currentTarget)
{
    Player* bot = botAI->GetBot();
    Unit* freya = GetFreyaScanUnitByEntry(botAI, NPC_FREYA);

    if (PlayerbotAI::IsMainTank(bot))
        return freya;

    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return nullptr;

    if (state.snaplasher && state.snaplasher->IsAlive())
        return state.snaplasher;

    if (state.conservator && state.conservator->IsAlive())
        return state.conservator;

    if (Unit* member = GetFreyaTankTrioTarget(state, currentTarget))
        return member;

    // Same local rule and leash as melee DPS: hit the one next to it, never tow one into the raid.
    if (Unit* lasher = GetFreyaLocalLasherTarget(botAI, state, currentTarget, ULDUAR_FREYA_MELEE_LASHER_RANGE))
        return lasher;

    return freya;
}

bool FreyaHasLivingRangedDps(PlayerbotAI* botAI)
{
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && PlayerbotAI::IsRangedDps(member))
            return true;
    }

    return false;
}

std::vector<Position> GetFreyaNatureBombPositions(Player* bot, float searchRadius)
{
    std::list<GameObject*> bombs;
    bot->GetGameObjectListWithEntryInGrid(bombs, GOBJECT_NATURE_BOMB, searchRadius);

    std::vector<Position> positions;
    positions.reserve(bombs.size());
    for (GameObject* bomb : bombs)
    {
        if (bomb)
            positions.push_back(bomb->GetPosition());
    }

    return positions;
}

std::vector<Position> GetFreyaSunBeamPositions(PlayerbotAI* botAI, float searchRadius)
{
    Player* bot = botAI->GetBot();

    std::vector<Position> beams;
    for (ObjectGuid const& guid : GetFreyaScan(botAI).Stalkers())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() != NPC_FREYA_SUN_BEAM && unit->GetEntry() != NPC_FREYA_UNSTABLE_SUN_BEAM)
            continue;

        if (bot->GetExactDist2d(unit) < searchRadius)
            beams.push_back(unit->GetPosition());
    }

    return beams;
}

std::vector<EncounterHelpers::HazardCircle> BuildFreyaEscapeHazards(std::vector<Position> const& bombs,
                                                                    std::vector<Position> const& beams,
                                                                    std::vector<Position> const& lashers)
{
    std::vector<EncounterHelpers::HazardCircle> hazards;
    hazards.reserve(bombs.size() + beams.size() + lashers.size());
    for (Position const& bomb : bombs)
        hazards.emplace_back(bomb, ULDUAR_FREYA_NATURE_BOMB_CLEAR_RADIUS);

    for (Position const& beam : beams)
        hazards.emplace_back(beam, ULDUAR_FREYA_SUN_BEAM_CLEARANCE);

    for (Position const& lasher : lashers)
        hazards.emplace_back(lasher, ULDUAR_FREYA_DETONATE_RADIUS);

    return hazards;
}

std::vector<Position> GetFreyaEscapeLasherPositions(PlayerbotAI* botAI, float searchRadius)
{
    Player* bot = botAI->GetBot();

    // Melee and tanks stand on what they hit, a spot 15 yd off the pile is no firing position. Only low lashers
    // count: Detonate lands on death, and clearing all ten is running from something faster than a bot.
    if (PlayerbotAI::IsTank(bot) || !(PlayerbotAI::IsRangedDps(bot) || PlayerbotAI::IsHeal(bot)))
        return {};

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    return GetFreyaLowLasherPositions(botAI, state, ULDUAR_FREYA_LASHER_BAIL_PCT, searchRadius);
}

std::vector<EncounterHelpers::HazardCircle> GetFreyaEscapeHazards(PlayerbotAI* botAI, float searchRadius)
{
    return BuildFreyaEscapeHazards(GetFreyaNatureBombPositions(botAI->GetBot(), searchRadius),
                                   GetFreyaSunBeamPositions(botAI, searchRadius),
                                   GetFreyaEscapeLasherPositions(botAI, searchRadius));
}

Player* GetFreyaRangedCampAnchor(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return PlayerbotAI::IsRangedDps(bot) ? bot : nullptr;

    Player* anchor = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsRangedDps(member))
            continue;

        if (!anchor || member->GetGUID() < anchor->GetGUID())
            anchor = member;
    }

    return anchor;
}

uint32 GetFreyaRangedDpsRank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsRangedDps(bot))
        return std::numeric_limits<uint32>::max();

    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    uint32 rank = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(member) || !PlayerbotAI::IsRangedDps(member))
            continue;

        if (member->GetGUID() < bot->GetGUID())
            ++rank;
    }

    return rank;
}

uint32 CountFreyaLashersNear(Position const& centre, FreyaWaveState const& state, float radius)
{
    uint32 count = 0;
    for (Unit* lasher : state.detonatingLashers)
    {
        if (lasher && lasher->IsAlive() && centre.GetExactDist2d(lasher->GetPosition()) <= radius)
            ++count;
    }

    return count;
}

std::vector<Position> GetFreyaLowLasherPositions(PlayerbotAI* botAI, FreyaWaveState const& state, float maxPct,
                                                 float radius)
{
    Player* bot = botAI->GetBot();
    std::vector<Position> low;

    for (Unit* lasher : state.detonatingLashers)
    {
        if (!lasher || !lasher->IsAlive() || lasher->GetHealthPct() >= maxPct)
            continue;

        if (bot->GetExactDist2d(lasher) <= radius)
            low.push_back(lasher->GetPosition());
    }

    return low;
}

Position GetFreyaLasherCampSpot(PlayerbotAI* botAI)
{
    return GetFreyaScan(botAI).LasherCamp();
}

bool FreyaShedsSnaplasherThreat(PlayerbotAI* botAI, Unit* snaplasher, uint32 seenForMs, bool shedding)
{
    Player* bot = botAI->GetBot();
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (seenForMs < ULDUAR_FREYA_SNAPLASHER_TANK_LEAD_MS)
        return true;

    float tankThreat = 0.0f;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || !PlayerbotAI::IsTank(member))
            continue;

        tankThreat = std::max(tankThreat, snaplasher->GetThreatMgr().GetThreat(member));
    }

    // Nobody is tanking it, and it still has to die with the other two.
    if (tankThreat <= 0.0f)
        return false;

    float const pct = snaplasher->GetThreatMgr().GetThreat(bot) * 100.0f / tankThreat;

    return pct >=
           (shedding ? ULDUAR_FREYA_SNAPLASHER_THREAT_RETURN_PCT : ULDUAR_FREYA_SNAPLASHER_THREAT_SHED_PCT);
}

Unit* GetFreyaLasherPackFocus(FreyaWaveState const& state)
{
    Unit* best = nullptr;
    uint32 bestCount = 0;

    for (Unit* lasher : state.detonatingLashers)
    {
        if (!lasher || !lasher->IsAlive())
            continue;

        uint32 const count = CountFreyaLashersNear(lasher->GetPosition(), state, ULDUAR_FREYA_LASHER_PACK_RADIUS);

        // GUID breaks the tie so a wave of evenly spread lashers does not resolve differently per bot,
        // which would split the raid's AoE across two piles and fire neither.
        if (!best || count > bestCount || (count == bestCount && lasher->GetGUID() < best->GetGUID()))
        {
            best = lasher;
            bestCount = count;
        }
    }

    return best;
}

bool IsFreyaLasherPackFinishing(FreyaWaveState const& state, Position const& centre)
{
    uint32 count = 0;
    for (Unit* lasher : state.detonatingLashers)
    {
        if (!lasher || !lasher->IsAlive())
            continue;

        if (centre.GetExactDist2d(lasher->GetPosition()) > ULDUAR_FREYA_LASHER_PACK_RADIUS)
            continue;

        // One healthy lasher in the pile means the raid is still in the AoE phase: novaing and walking
        // out now would leave it alive behind the snare with nobody near enough to finish it.
        if (lasher->GetHealthPct() > ULDUAR_FREYA_LASHER_FINISH_PCT)
            return false;

        ++count;
    }

    return count >= ULDUAR_FREYA_LASHER_PACK_MIN_COUNT;
}

Unit* GetFreyaFinishingPackNear(PlayerbotAI* botAI, FreyaWaveState const& state, float radius)
{
    Unit* focus = GetFreyaLasherPackFocus(state);
    if (!focus || botAI->GetBot()->GetExactDist2d(focus) > radius)
        return nullptr;

    return IsFreyaLasherPackFinishing(state, focus->GetPosition()) ? focus : nullptr;
}

Unit* FreyaScan::Boss()
{
    if (!FreshThisTick(bossAtMs))
    {
        Unit* found = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "freya")->Get();
        boss = found ? found->GetGUID() : ObjectGuid::Empty;
    }

    return boss.IsEmpty() ? nullptr : botAI->GetUnit(boss);
}

GuidVector const& FreyaScan::PossibleTargets()
{
    if (!FreshThisTick(targetsAtMs))
        targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    return targets;
}

GuidVector const& FreyaScan::Stalkers()
{
    if (FreshThisTick(stalkersAtMs))
        return stalkers;

    stalkers.clear();

    Player* bot = botAI->GetBot();
    if (!bot)
        return stalkers;

    // Same visit, range check and order as "nearest npcs" for these entries, but the entry is tested first so the
    // LOS ray only goes to spores and beams, not every pet, ghoul and bomb in sight.
    float const range = sPlayerbotAIConfig.SightDistance;

    std::vector<Unit*> matches;
    FreyaStalkerInRangeCheck check{Acore::AnyUnitInObjectRangeCheck(bot, range)};
    Acore::UnitListSearcher<FreyaStalkerInRangeCheck> searcher(bot, matches, check);
    Cell::VisitObjects(bot, searcher, range);

    for (Unit* unit : matches)
        if (!unit->IsPlayer() && bot->IsWithinLOSInMap(unit))
            stalkers.push_back(unit->GetGUID());

    return stalkers;
}

Position const& FreyaScan::LasherCamp()
{
    if (!FreshThisTick(campAtMs))
    {
        Player* anchor = GetFreyaRangedCampAnchor(botAI);
        camp = anchor ? anchor->GetPosition() : Position();
    }

    return camp;
}

FreyaScan& GetFreyaScan(PlayerbotAI* botAI)
{
    return *botAI->GetAiObjectContext()->GetValue<FreyaScan*>("freya scan")->Get();
}

Unit* GetFreyaScanUnitByEntry(PlayerbotAI* botAI, uint32 entry)
{
    for (ObjectGuid const& guid : GetFreyaScan(botAI).PossibleTargets())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

Unit* GetFreyaBossByEntry(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    // Same order and check as "possible targets no los" without building the rest of the list. IsPossibleTarget
    // only runs on units already known to be her.
    float const range = sPlayerbotAIConfig.SightDistance;

    std::vector<Unit*> matches;
    FreyaUnfriendlyOfEntryCheck check{Acore::AnyUnfriendlyUnitInObjectRangeCheck(bot, bot, range), NPC_FREYA};
    Acore::UnitListSearcher<FreyaUnfriendlyOfEntryCheck> searcher(bot, matches, check);
    Cell::VisitObjects(bot, searcher, range);

    for (Unit* unit : matches)
        if (unit->IsAlive() && AttackersValue::IsPossibleTarget(unit, bot, range))
            return unit;

    return nullptr;
}

bool IsFreyaGroundTremorCasting(Unit* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell)
        return false;

    return spell->m_spellInfo->Id == SPELL_FREYA_GROUND_TREMOR_10 ||
           spell->m_spellInfo->Id == SPELL_FREYA_GROUND_TREMOR_25;
}

Unit* GetFreyaSunbeamTarget(Unit* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return nullptr;

    Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell)
        return nullptr;

    if (spell->m_spellInfo->Id != SPELL_FREYA_SUNBEAM_10 && spell->m_spellInfo->Id != SPELL_FREYA_SUNBEAM_25)
        return nullptr;

    return spell->m_targets.GetUnitTarget();
}
