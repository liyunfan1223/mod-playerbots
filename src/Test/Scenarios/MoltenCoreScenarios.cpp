/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "MoltenCoreScenarios.h"

#include "MoltenCoreDefs.h"

#include "Creature.h"
#include "CreatureAI.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "StringFormat.h"

#include <cmath>
#include <memory>

using namespace MoltenCoreTest;

namespace
{
    InstanceScript* GetInstanceScript(TestContext& ctx)
    {
        Player* ref = ctx.FirstAliveBot();
        if (!ref || !ref->GetMap() || !ref->GetMap()->ToInstanceMap())
            return nullptr;

        return ref->GetMap()->ToInstanceMap()->GetInstanceScript();
    }

    void MarkBossDone(InstanceScript* script, uint8 data)
    {
        if (script->GetBossState(data) == TO_BE_DECIDED)
            script->SetBossState(data, NOT_STARTED);

        script->SetBossState(data, DONE);
    }

    void QueueMarkAndEngage(TestContext& ctx)
    {
        ctx.Do("engage", [](TestContext& ctx)
        {
            Player* leader = ctx.GetBot(0);
            if (!leader || !ObjectAccessor::GetCreature(*leader, ctx.bossGuid))
                return false;

            ctx.MarkSkull(ctx.bossGuid);
            ctx.SetMetric("engage_at_ms", ctx.elapsedMs);
            ctx.OrderAttackAll();
            return true;
        });
    }

    // Waits for a creature of the given entry to exist near the raid and stores it
    // as the watched boss.
    void QueueWaitForSpawn(TestContext& ctx, std::string stepName, uint32 entry, uint32 timeoutMs)
    {
        auto noted = std::make_shared<bool>(false);
        ctx.Step(std::move(stepName), timeoutMs, [entry, noted](TestContext& ctx) -> StepStatus
        {
            Player* ref = ctx.FirstAliveBot();
            if (!ref)
                return StepStatus::Failed;

            if (!*noted)
            {
                *noted = true;
                ctx.AddNote(Acore::StringFormat("waiting for {} near ({:.0f},{:.0f},{:.1f})",
                    entry, ref->GetPositionX(), ref->GetPositionY(), ref->GetPositionZ()));
            }

            if (Creature* creature = ref->FindNearestCreature(entry, 200.0f))
            {
                ctx.bossGuid = creature->GetGUID();
                return StepStatus::Done;
            }

            return StepStatus::InProgress;
        });
    }

}

// Majordomo Executus: summoned once the eight prior bosses are DONE; the encounter
// completes when his eight adds die (he never dies himself).
class MCMajordomoScenario : public TestScenario
{
public:
    MCMajordomoScenario(std::string name, std::vector<BotDef> comp, GearDef gear)
        : _name(std::move(name)), _comp(std::move(comp)), _gear(gear) { }

    std::string GetName() const override { return _name; }

    void Setup(TestContext& ctx) override
    {
        // Buff at the entrance, hop to the terrace. Map-wide clear spares nothing
        // (Majordomo is summoned after).
        ctx.PrepareRaid(60, _comp, _gear, PROGRESSION_VANILLA_START, MAP_ID, ENTRANCE);
        ctx.TeleportTo(MAP_ID, MAJORDOMO_ARENA);

        ctx.Do("mark prior bosses done", [](TestContext& ctx)
        {
            InstanceScript* script = GetInstanceScript(ctx);
            if (!script)
                return false;

            for (uint8 data = DATA_MAGMADAR; data < DATA_MAJORDOMO; ++data)
                MarkBossDone(script, data);

            // Summon him directly; his AI reads the boss states above and configures itself.
            Player* ref = ctx.FirstAliveBot();
            if (!ref || !ref->GetMap()->SummonCreature(NPC_MAJORDOMO, MAJORDOMO_SUMMON_POS))
                return false;

            return true;
        });

        QueueWaitForSpawn(ctx, "majordomo spawned", NPC_MAJORDOMO, SPAWN_WAIT_TIMEOUT_MS);

        // The tank walks into the pack's aggro radius to start the fight. Majordomo
        // himself can't die (he reflects); the encounter ends when his eight adds die.
        ctx.Step("walk in to engage", WALK_IN_TIMEOUT_MS, [](TestContext& ctx) -> StepStatus
        {
            Player* mt = ctx.GetBot(0);
            if (!mt || !mt->IsAlive())
                return StepStatus::Failed;

            if (mt->IsInCombat())
                return StepStatus::Done;

            mt->GetMotionMaster()->MovePoint(0, MAJORDOMO_WALK_IN);
            return StepStatus::InProgress;
        });

        ctx.Do("note engage", [](TestContext& ctx)
        {
            ctx.SetMetric("engage_at_ms", ctx.elapsedMs);
            return true;
        });

        ctx.Step("majordomo encounter done", 10 * MINUTE * IN_MILLISECONDS, [](TestContext& ctx) -> StepStatus
        {
            if (ctx.AliveBotCount() == 0)
            {
                ctx.AddNote("raid wiped");
                return StepStatus::Failed;
            }

            InstanceScript* script = GetInstanceScript(ctx);
            if (!script)
                return StepStatus::Failed;

            if (script->GetBossState(DATA_MAJORDOMO) == DONE)
            {
                ctx.SetMetric("kill_time_ms", ctx.elapsedMs - ctx.metrics["engage_at_ms"]);
                return StepStatus::Done;
            }

            return StepStatus::InProgress;
        });
    }

private:
    std::string _name;
    std::vector<BotDef> _comp;
    GearDef _gear;
};

// Ragnaros: with Majordomo DONE, the friendly Majordomo spawns at the lair; his
// START_RAGNAROS_INTRO action runs the summon RP and emerges Ragnaros.
class MCRagnarosScenario : public TestScenario
{
public:
    MCRagnarosScenario(std::string name, std::vector<BotDef> comp, GearDef gear)
        : _name(std::move(name)), _comp(std::move(comp)), _gear(gear) { }

    std::string GetName() const override { return _name; }

    void Setup(TestContext& ctx) override
    {
        // Buff at the entrance, hop to the lair. Map-wide clear spares nothing
        // (Majordomo and Ragnaros both appear after).
        ctx.PrepareRaid(60, _comp, _gear, PROGRESSION_VANILLA_START, MAP_ID, ENTRANCE);
        ctx.TeleportTo(MAP_ID, RAGNAROS_ARENA);

        ctx.Do("mark bosses done", [](TestContext& ctx)
        {
            InstanceScript* script = GetInstanceScript(ctx);
            if (!script)
                return false;

            // Majordomo DONE first so his AI spawns as the friendly gossip version.
            MarkBossDone(script, DATA_MAJORDOMO);
            for (uint8 data = DATA_MAGMADAR; data < DATA_MAJORDOMO; ++data)
                MarkBossDone(script, data);

            Player* ref = ctx.FirstAliveBot();
            if (!ref || !ref->GetMap()->SummonCreature(NPC_MAJORDOMO, MAJORDOMO_LAIR_POS))
                return false;

            return true;
        });

        QueueWaitForSpawn(ctx, "majordomo at lair", NPC_MAJORDOMO, SPAWN_WAIT_TIMEOUT_MS);

        ctx.Do("start ragnaros intro", [](TestContext& ctx)
        {
            Player* ref = ctx.FirstAliveBot();
            if (!ref)
                return false;

            Creature* domo = ObjectAccessor::GetCreature(*ref, ctx.bossGuid);
            if (!domo || !domo->AI())
                return false;

            domo->AI()->DoAction(ACTION_START_RAGNAROS_INTRO);
            return true;
        });

        // The summon RP runs ~90s; Ragnaros is a submerged, non-attackable prop until
        // it ends. Wait until he is actually attackable before engaging.
        ctx.Step("ragnaros emerged", EMERGE_TIMEOUT_MS, [](TestContext& ctx) -> StepStatus
        {
            Player* ref = ctx.FirstAliveBot();
            if (!ref)
                return StepStatus::Failed;

            Creature* rag = ref->FindNearestCreature(NPC_RAGNAROS, 200.0f);
            if (!rag)
                return StepStatus::InProgress;

            ctx.bossGuid = rag->GetGUID();
            if (rag->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) || rag->IsImmuneToPC() || !rag->IsAlive())
                return StepStatus::InProgress;

            return StepStatus::Done;
        });
        QueueMarkAndEngage(ctx);

        ctx.Step("ragnaros dead", FIGHT_TIMEOUT_MS, [](TestContext& ctx) -> StepStatus
        {
            Player* ref = ctx.FirstAliveBot();
            if (!ref)
            {
                ctx.AddNote("raid wiped");
                return StepStatus::Failed;
            }

            Creature* boss = ObjectAccessor::GetCreature(*ref, ctx.bossGuid);
            if (!boss)
                return StepStatus::InProgress;

            if (boss->isDead())
            {
                ctx.SetMetric("kill_time_ms", ctx.elapsedMs - ctx.metrics["engage_at_ms"]);
                return StepStatus::Done;
            }

            if (!boss->IsInCombat() && boss->GetHealthPct() > 99.0f && ctx.StepElapsedMs() > 15000)
            {
                ctx.AddNote("boss reset (evaded to full health) — fight lost");
                return StepStatus::Failed;
            }

            return StepStatus::InProgress;
        });
    }

private:
    std::string _name;
    std::vector<BotDef> _comp;
    GearDef _gear;
};

std::unique_ptr<TestScenario> MakeMoltenCoreBoss(McBoss const& b, std::string nameSuffix,
                                                 std::vector<BotDef> comp, uint32 tankCount, GearDef gear)
{
    float ax = b.x + b.arenaOffset * std::cos(b.o);
    float ay = b.y + b.arenaOffset * std::sin(b.o);
    RaidBossScenario::Staging entrance{ ENTRANCE, true };
    return std::make_unique<RaidBossScenario>(
        b.name + std::move(nameSuffix), b.entry, MAP_ID,
        ax, ay, b.z, b.o,
        std::move(comp), tankCount, 60, gear,
        FIGHT_TIMEOUT_MS, b.adds, PROGRESSION_VANILLA_START, entrance, b.pullMode);
}

std::unique_ptr<TestScenario> MakeMCMajordomoScenario(std::string name, std::vector<BotDef> comp, GearDef gear)
{
    return std::make_unique<MCMajordomoScenario>(std::move(name), std::move(comp), gear);
}

std::unique_ptr<TestScenario> MakeMCRagnarosScenario(std::string name, std::vector<BotDef> comp, GearDef gear)
{
    return std::make_unique<MCRagnarosScenario>(std::move(name), std::move(comp), gear);
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
