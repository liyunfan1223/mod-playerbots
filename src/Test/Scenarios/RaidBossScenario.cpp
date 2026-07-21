/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "RaidBossScenario.h"

#include "Creature.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "StringFormat.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <memory>

namespace
{
    // Pull geometry: land outside proximity-aggro range (~23y for a +3 boss) so the
    // fight starts by pull, not by teleport.
    constexpr float PULL_LANDING = 30.0f;
    constexpr float PULL_LANDING_STEP = 6.0f;          // second search ring
    constexpr int LANDING_BEARINGS = 12;               // 30-degree spacing around the spot
    constexpr float LANDING_BEARING_RAD = 0.5236f;     // 30 degrees
    constexpr float LANDING_HEIGHT_TOLERANCE = 5.0f;   // same-shelf ground only
    constexpr float ADD_GATHER_RADIUS = 150.0f;        // encounter adds around the boss
    constexpr float BOSS_SEARCH_RADIUS = 200.0f;       // before the map-wide fallback
    constexpr float DRAG_HOME_TOLERANCE = 10.0f;       // boss anchored at the arena spot
    constexpr uint32 DRAG_HOME_CAP_MS = 20000;
    constexpr float ATTACK_ORDER_RANGE = 60.0f;
    constexpr uint32 PULL_TIMEOUT_MS = 180 * IN_MILLISECONDS;
    constexpr uint32 PULL_DIAG_INTERVAL_MS = 15000;
    constexpr uint32 FIGHT_DIAG_INTERVAL_MS = 15000;
    constexpr uint32 DPS_HOLD_MS = 5000;               // tanks establish threat first
    constexpr uint32 ENGAGE_TIMEOUT_MS = 30 * IN_MILLISECONDS;
    constexpr float BOSS_RESET_HEALTH_PCT = 99.0f;
    constexpr uint32 BOSS_RESET_GRACE_MS = 10000;
    constexpr float EXECUTE_PHASE_HEALTH_PCT = 10.0f;
}

RaidBossScenario::RaidBossScenario(std::string name, uint32 bossEntry, uint32 mapId,
                                   float x, float y, float z, float o,
                                   std::vector<BotDef> comp, uint32 tankCount, uint32 level, GearDef gear,
                                   uint32 fightTimeoutMs, std::vector<uint32> encounterAdds, uint8 progression,
                                   Staging staging, PullMode pullMode)
    : _name(std::move(name)), _bossEntry(bossEntry), _mapId(mapId),
      _x(x), _y(y), _z(z), _o(o),
      _comp(std::move(comp)), _tankCount(tankCount), _level(level), _gear(gear),
      _fightTimeoutMs(fightTimeoutMs), _encounterAdds(std::move(encounterAdds)), _progression(progression),
      _staging(staging), _pullMode(pullMode) { }

Creature* RaidBossScenario::AcquireBoss(TestContext& ctx, Player* mt, uint32 bossEntry)
{
    if (!ctx.bossGuid.IsEmpty())
        return ObjectAccessor::GetCreature(*mt, ctx.bossGuid);

    Creature* boss = mt->FindNearestCreature(bossEntry, BOSS_SEARCH_RADIUS);
    if (!boss)
        for (auto const& [spawnId, creature] : mt->GetMap()->GetCreatureBySpawnIdStore())
            if (creature->GetEntry() == bossEntry && creature->IsAlive())
            {
                boss = creature;
                break;
            }

    if (!boss)
        return nullptr;

    ctx.bossGuid = boss->GetGUID();
    ctx.MarkSkull(boss->GetGUID());
    ctx.SetMetric("engage_at_ms", ctx.elapsedMs);

    for (size_t i = 0; i < ctx.botGuids.size(); ++i)
        if (Player* bot = ctx.GetBot(i))
        {
            bot->SetFullHealth();
            bot->SetPower(bot->getPowerType(), bot->GetMaxPower(bot->getPowerType()));
        }

    return boss;
}

StepStatus RaidBossScenario::TickDragHome(TestContext& ctx, Player* mt, Creature* boss,
                                          PullState& state, float spotX, float spotY, float spotZ)
{
    // Anchor the fight at the arena spot, not where the tank crossed the aggro line.
    if (state.engagedAtMs == 0)
        state.engagedAtMs = ctx.StepElapsedMs();

    if (boss->GetExactDist(spotX, spotY, spotZ) > DRAG_HOME_TOLERANCE &&
        ctx.StepElapsedMs() - state.engagedAtMs < DRAG_HOME_CAP_MS)
    {
        mt->GetMotionMaster()->MovePoint(0, spotX, spotY, spotZ);
        return StepStatus::InProgress;
    }

    ctx.NoteReadiness("pull");
    return StepStatus::Done;
}

void RaidBossScenario::RelocatePack(TestContext& ctx, Player* mt, Creature* boss,
                                    std::vector<uint32> const& encounterAdds,
                                    float spotX, float spotY, float spotZ)
{
    // Move the whole pack; keep each add's native offset
    // so a tight ring doesn't make the fight artificially AoE-friendly.
    std::list<Creature*> adds;
    for (uint32 entry : encounterAdds)
    {
        std::list<Creature*> found;
        boss->GetCreatureListWithEntryInGrid(found, entry, ADD_GATHER_RADIUS);
        adds.splice(adds.end(), found);
    }

    // Land along the spot->home bearing, but only where there's LOS and flat, dry,
    // same-shelf ground; else rotate until a bearing qualifies (order and aggro are
    // both LOS-gated). Anchor on the boss's HOME position so the search is deterministic.
    float const homeDx = boss->GetHomePosition().GetPositionX() - spotX;
    float const homeDy = boss->GetHomePosition().GetPositionY() - spotY;
    float const homeDist = std::sqrt(homeDx * homeDx + homeDy * homeDy);
    float dirX = homeDist > 1.0f ? homeDx / homeDist : 1.0f;
    float dirY = homeDist > 1.0f ? homeDy / homeDist : 0.0f;
    float landing = PULL_LANDING;
    bool landingFound = false;
    for (float radius : { PULL_LANDING, PULL_LANDING + PULL_LANDING_STEP })
    {
        for (int stepIdx = 0; stepIdx < LANDING_BEARINGS && !landingFound; ++stepIdx)
        {
            // 0, +30deg, -30deg, +60deg, ...
            float const rot = (stepIdx + 1) / 2 * LANDING_BEARING_RAD * (stepIdx % 2 ? 1.0f : -1.0f);
            float const rx = dirX * std::cos(rot) - dirY * std::sin(rot);
            float const ry = dirX * std::sin(rot) + dirY * std::cos(rot);
            float const cx = spotX + radius * rx, cy = spotY + radius * ry;
            float const ground = mt->GetMap()->GetHeight(mt->GetPhaseMask(), cx, cy, spotZ + 5.0f);
            if (std::fabs(ground - spotZ) >= LANDING_HEIGHT_TOLERANCE)
                continue;

            // Liquid passes the height check but is off-navmesh; a boss in liquid evade-locks.
            if (mt->GetMap()->GetLiquidData(mt->GetPhaseMask(), cx, cy, ground + 1.0f,
                    DEFAULT_COLLISION_HEIGHT, MAP_ALL_LIQUIDS).Status != LIQUID_MAP_NO_WATER)
                continue;

            if (mt->GetMap()->isInLineOfSight(spotX, spotY, spotZ + 2.0f, cx, cy, spotZ + 2.0f,
                                              mt->GetPhaseMask(), LINEOFSIGHT_ALL_CHECKS,
                                              VMAP::ModelIgnoreFlags::Nothing))
            {
                if (rot != 0.0f || radius != PULL_LANDING)
                    ctx.AddNote(Acore::StringFormat("pull landing rotated to ({:.0f},{:.0f})", cx, cy));
                landingFound = true;
                landing = radius;
                dirX = rx;
                dirY = ry;
            }
        }
        if (landingFound)
            break;
    }

    if (!landingFound)
        ctx.AddNote(Acore::StringFormat(
            "pull landing: no flat/dry/LOS bearing at {:.0f}y — falling back to the spawn bearing",
            PULL_LANDING));

    // Push the landing out until no add ends up inside it either.
    float const baseLanding = landing;
    for (Creature* add : adds)
        if (add->IsAlive())
        {
            float const along = (add->GetPositionX() - boss->GetPositionX()) * dirX +
                                (add->GetPositionY() - boss->GetPositionY()) * dirY;
            landing = std::max(landing, baseLanding - along);
        }

    for (Creature* add : adds)
        if (add->IsAlive() && !add->GetVictim())
        {
            float const ax = spotX + landing * dirX + (add->GetPositionX() - boss->GetPositionX());
            float const ay = spotY + landing * dirY + (add->GetPositionY() - boss->GetPositionY());
            add->NearTeleportTo(ax, ay, spotZ, add->GetOrientation());
            add->SetHomePosition(ax, ay, spotZ, add->GetOrientation());
        }

    float const bx = spotX + landing * dirX, by = spotY + landing * dirY;
    boss->NearTeleportTo(bx, by, spotZ, boss->GetOrientation());
    boss->SetHomePosition(bx, by, spotZ, boss->GetOrientation());
    boss->CombatStop(true);
    boss->StopMoving();
    boss->GetMotionMaster()->Clear();
    boss->GetMotionMaster()->MoveIdle();
}

void RaidBossScenario::TickApproach(Player* mt, PlayerbotAI* mtAI, Creature* boss)
{
    if (mt->GetDistance(boss) <= ATTACK_ORDER_RANGE && mt->IsWithinLOSInMap(boss))
    {
        mtAI->DoSpecificAction("attack rti target");
        return;
    }

    // Out of order range: run toward the boss until the tag order can land.
    mt->GetMotionMaster()->MovePoint(0, boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ());
}

void RaidBossScenario::NotePullDiagnostics(TestContext& ctx, Player* mt, Creature* boss, PullState& state)
{
    // Log geometry when the pull stalls (a bad arena spot or approach).
    if (ctx.StepElapsedMs() - state.lastDiagMs < PULL_DIAG_INTERVAL_MS)
        return;

    state.lastDiagMs = ctx.StepElapsedMs();
    ctx.AddNote(Acore::StringFormat(
        "pull stuck {}s: mt ({:.0f},{:.0f},{:.1f}) boss ({:.0f},{:.0f},{:.1f}) dist {:.0f} los {} combat {} victim {} landed {}",
        ctx.StepElapsedMs() / 1000, mt->GetPositionX(), mt->GetPositionY(), mt->GetPositionZ(),
        boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ(),
        mt->GetDistance(boss), mt->IsWithinLOSInMap(boss),
        boss->IsInCombat(), boss->GetVictim() ? boss->GetVictim()->GetName() : "none", state.packLanded));
}

void RaidBossScenario::Setup(TestContext& ctx)
{
    // Spare the whole encounter from the clear; each run owns its instance.
    std::vector<uint32> spare = _encounterAdds;
    spare.push_back(_bossEntry);

    // With a staging point, buff far from the boss (aggro range makes a near arena a
    // lottery), then hop to the arena and pull.
    ctx.PrepareRaid(_level, _comp, _gear, _progression, _mapId,
                    _staging.enabled ? _staging.pos : Position(_x, _y, _z, _o), spare);
    if (_staging.enabled)
        ctx.TeleportTo(_mapId, _x, _y, _z, _o);

    uint32 bossEntry = _bossEntry;
    std::vector<uint32> encounterAdds = _encounterAdds;
    float spotX = _x, spotY = _y, spotZ = _z;
    PullMode pullMode = _pullMode;

    std::vector<bool> isHealer;
    for (BotDef const& def : _comp)
        isHealer.push_back(def.spec.find("holy") != std::string::npos ||
                           def.spec.find("resto") != std::string::npos ||
                           def.spec.find("disc") != std::string::npos);

    // Start by pull: the raid holds the open arena spot (mutual LOS; target selection
    // drops out-of-LOS bots), and the main tank tags the boss and drags him.
    auto state = std::make_shared<PullState>();
    ctx.Step("pull boss to raid", PULL_TIMEOUT_MS, [bossEntry, encounterAdds, spotX, spotY, spotZ, pullMode, state](TestContext& ctx) -> StepStatus
    {
        Player* mt = ctx.GetBot(0);
        if (!mt || !mt->IsAlive())
        {
            ctx.AddNote("pull: main tank missing or dead");
            return StepStatus::Failed;
        }

        Creature* boss = AcquireBoss(ctx, mt, bossEntry);
        if (!boss)
        {
            ctx.AddNote(ctx.bossGuid.IsEmpty() ? "boss not found anywhere on the map"
                                               : "pull: boss guid no longer resolves (despawned?)");
            return StepStatus::Failed;
        }

        PlayerbotAI* mtAI = GET_PLAYERBOT_AI(mt);
        if (!mtAI)
        {
            ctx.AddNote("pull: main tank has no AI");
            return StepStatus::Failed;
        }

        if (boss->IsInCombat() && boss->GetVictim())
            return TickDragHome(ctx, mt, boss, *state, spotX, spotY, spotZ);

        if (pullMode == PullMode::Relocate && !state->packLanded &&
            boss->GetExactDist(spotX, spotY, spotZ) > PULL_LANDING - DRAG_HOME_TOLERANCE)
        {
            state->packLanded = true;
            RelocatePack(ctx, mt, boss, encounterAdds, spotX, spotY, spotZ);
        }

        TickApproach(mt, mtAI, boss);
        NotePullDiagnostics(ctx, mt, boss, *state);
        return StepStatus::InProgress;
    });

    // Dps hold a few seconds so the tanks build threat on the boss and his adds;
    uint32 tankCount = _tankCount;
    ctx.Step("raid engage", ENGAGE_TIMEOUT_MS, [tankCount](TestContext& ctx) -> StepStatus
    {
        if (ctx.StepElapsedMs() < DPS_HOLD_MS)
        {
            ctx.OrderAttack(0);
            return StepStatus::InProgress;
        }

        ctx.OrderAttackAll(tankCount);
        return StepStatus::Done;
    });

    auto lastDiagMs = std::make_shared<uint32>(0);
    auto manaDumped = std::make_shared<bool>(false);
    ctx.Step("boss dead", _fightTimeoutMs, [isHealer, lastDiagMs, manaDumped](TestContext& ctx) -> StepStatus
    {
        Player* ref = ctx.FirstAliveBot();
        if (!ref)
        {
            ctx.AddNote("raid wiped");
            return StepStatus::Failed;
        }

        Creature* boss = ObjectAccessor::GetCreature(*ref, ctx.bossGuid);
        if (!boss)
            return StepStatus::InProgress;  // despawned corpse also lands here; kill detection below

        if (boss->isDead())
        {
            ctx.SetMetric("kill_time_ms", ctx.elapsedMs - ctx.metrics["engage_at_ms"]);
            return StepStatus::Done;
        }

        // Boss evaded back to full health: fail now instead of burning the timeout.
        if (!boss->IsInCombat() && boss->GetHealthPct() > BOSS_RESET_HEALTH_PCT &&
            ctx.StepElapsedMs() > BOSS_RESET_GRACE_MS)
        {
            ctx.AddNote("boss reset (evaded to full health) — fight lost");
            return StepStatus::Failed;
        }

        // Healer mana at the execute phase: separates OOM from alive-but-outdamaged.
        if (!*manaDumped && boss->GetHealthPct() <= EXECUTE_PHASE_HEALTH_PCT)
        {
            *manaDumped = true;
            for (size_t i = 0; i < ctx.botGuids.size(); ++i)
                if (i < isHealer.size() && isHealer[i])
                    if (Player* h = ctx.GetBot(i))
                        ctx.AddNote(Acore::StringFormat("[{}s] boss 10%: healer {} mana {}% alive {}",
                            ctx.elapsedMs / 1000, h->GetName(),
                            h->GetMaxPower(POWER_MANA) ? h->GetPower(POWER_MANA) * 100 / h->GetMaxPower(POWER_MANA) : 0,
                            h->IsAlive()));
        }

        // Healer reachability: no LOS or >40y drops a tank from heal target selection,
        // so an offtank dragged out of reach dies unhealed. The boss-state line names evade-locks.
        if (ctx.elapsedMs - *lastDiagMs >= FIGHT_DIAG_INTERVAL_MS)
        {
            *lastDiagMs = ctx.elapsedMs;

            Player* mt0 = ctx.GetBot(0);
            ctx.AddNote(Acore::StringFormat(
                "[{}s] boss ({:.0f},{:.0f}) hp {:.0f}% combat {} victim {} evade {} | mt ({:.0f},{:.0f}) dist {:.0f}",
                ctx.elapsedMs / 1000, boss->GetPositionX(), boss->GetPositionY(), boss->GetHealthPct(),
                boss->IsInCombat(), boss->GetVictim() ? boss->GetVictim()->GetName() : "none",
                boss->IsInEvadeMode(), mt0 ? mt0->GetPositionX() : 0.0f, mt0 ? mt0->GetPositionY() : 0.0f,
                mt0 ? mt0->GetDistance(boss) : -1.0f));

            Player* healer = nullptr;
            for (size_t i = 0; i < ctx.botGuids.size() && !healer; ++i)
                if (i < isHealer.size() && isHealer[i])
                    if (Player* h = ctx.GetBot(i))
                        if (h->IsAlive())
                            healer = h;

            if (healer)
                for (size_t t = 0; t < isHealer.size() && t < ctx.botGuids.size(); ++t)
                {
                    if (isHealer[t])
                        continue;
                    Player* tank = ctx.GetBot(t);
                    if (!tank || t >= 3)
                        break;
                    ctx.AddNote(Acore::StringFormat("[{}s] healer {} -> tank {}: dist {:.0f} los {} | tank hp {:.0f}% alive {}",
                        ctx.elapsedMs / 1000, healer->GetName(), tank->GetName(), healer->GetDistance2d(tank),
                        healer->IsWithinLOSInMap(tank), tank->GetHealthPct(), tank->IsAlive()));
                }
        }

        return StepStatus::InProgress;
    });
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
