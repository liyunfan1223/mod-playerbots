/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "GenericScenarios.h"

#include "RaidComps.h"

#include "Player.h"
#include "SharedDefines.h"
#include "StringFormat.h"

#include <memory>

namespace
{
    // Goldshire: an ordinary open-world destination for the pipeline smoke test.
    Position const BOT_SMOKE_DESTINATION = { -9459.34f, 42.08f, 56.5f, 0.0f };
    constexpr uint32 BOT_SMOKE_LEVEL = 60;

    // Molten Core entrance: a raid-sized staging area far from any boss — a
    // neutral spot to buff up and run the ready-check debugging scenario.
    constexpr uint32 READY_CHECK_MAP = 409;
    Position const READY_CHECK_STAGING = { 1091.89f, -466.99f, -105.08f, 3.14f };
}

// Harness self-check: steps, metrics and timing without any bots.
class SelfTestScenario : public TestScenario
{
public:
    std::string GetName() const override { return "self_test"; }

    void Setup(TestContext& ctx) override
    {
        ctx.Do("set a metric", [](TestContext& ctx)
        {
            ctx.SetMetric("answer", 42.0);
            return true;
        });
        ctx.WaitUntil("wait 3 seconds", 10000, [](TestContext& ctx)
        {
            return ctx.elapsedMs >= 3000;
        });
        ctx.Assert("metric survived", [](TestContext& ctx)
        {
            return ctx.metrics.count("answer") && ctx.metrics["answer"] == 42.0;
        });
    }
};

// Single-bot lifecycle smoke test: spawn, gear, teleport, teardown.
class BotSmokeScenario : public TestScenario
{
public:
    std::string GetName() const override { return "bot_smoke"; }

    void Setup(TestContext& ctx) override
    {
        ctx.SpawnBots(BOT_SMOKE_LEVEL, { { CLASS_WARRIOR, "" } }, ITEM_QUALITY_UNCOMMON);
        ctx.TeleportTo(0, BOT_SMOKE_DESTINATION);
        ctx.Assert("bot alive at destination", [](TestContext& ctx)
        {
            Player* bot = ctx.GetBot(0);
            return bot && bot->IsAlive() && bot->GetMapId() == 0 && bot->GetLevel() == BOT_SMOKE_LEVEL;
        });
    }
};

// All-specs raid runs a ready check; fails on any coverage gap (blessings,
// mana potions, weapon imbues, warlock soulstone).
class ReadyCheckScenario : public TestScenario
{
public:
    std::string GetName() const override { return "ready_check"; }

    void Setup(TestContext& ctx) override
    {
        ctx.SpawnBots(60, AllSpecsComp(), ITEM_QUALITY_RARE);
        ctx.SetProgression(PROGRESSION_VANILLA_START);
        ctx.FormGroup(true);
        ctx.TeleportTo(READY_CHECK_MAP, READY_CHECK_STAGING);

        ctx.Do("readiness before", [](TestContext& ctx)
        {
            ctx.NoteReadiness("before");
            return true;
        });

        ctx.ReadyCheck();

        ctx.Assert("raid fully buffed and stocked", [](TestContext& ctx)
        {
            TestContext::ReadinessSummary summary = ctx.NoteReadiness("after");
            ctx.SetMetric("unblessed", summary.unblessed);
            ctx.SetMetric("unstocked_mana_users", summary.unstockedManaUsers);
            ctx.SetMetric("unimbued_weapons", summary.unimbuedWeapons);
            ctx.SetMetric("missing_soulstones", summary.missingSoulstones);

            if (summary.unblessed || summary.unstockedManaUsers ||
                summary.unimbuedWeapons || summary.missingSoulstones)
            {
                ctx.AddNote(Acore::StringFormat(
                    "coverage gaps after ready check: {} unblessed, {} mana users without potions, "
                    "{} unimbued weapons, {} warlocks without a soulstone out",
                    summary.unblessed, summary.unstockedManaUsers,
                    summary.unimbuedWeapons, summary.missingSoulstones));
                return false;
            }

            return true;
        });
    }
};

std::unique_ptr<TestScenario> MakeSelfTestScenario() { return std::make_unique<SelfTestScenario>(); }
std::unique_ptr<TestScenario> MakeBotSmokeScenario() { return std::make_unique<BotSmokeScenario>(); }
std::unique_ptr<TestScenario> MakeReadyCheckScenario() { return std::make_unique<ReadyCheckScenario>(); }

#endif  // PLAYERBOTS_INTEGRATION_TESTS
