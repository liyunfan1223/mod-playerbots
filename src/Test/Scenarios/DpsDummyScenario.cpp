/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "DpsDummyScenario.h"

#include "MoltenCoreDefs.h"

#include "Creature.h"
#include "Group.h"
#include "PassiveAI.h"
#include "StringFormat.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#include <cmath>
#include <memory>

// Pure throughput bench: the raid unloads on a passive boss-level target for a fixed
// window. Each run gets its own instance, so parallel benches can't contaminate each other.
class DpsDummyScenario : public TestScenario
{
public:
    DpsDummyScenario(std::string name, uint32 level, std::vector<BotDef> comp, GearDef gear,
                     uint32 dummyEntry, uint32 durationSec, uint8 progression)
        : _name(std::move(name)), _level(level), _comp(std::move(comp)), _gear(gear),
          _dummyEntry(dummyEntry), _durationSec(durationSec), _progression(progression) { }

    std::string GetName() const override { return _name; }

    void Setup(TestContext& ctx) override
    {
        ctx.PrepareRaid(_level, _comp, _gear, _progression, MoltenCoreTest::MAP_ID, MoltenCoreTest::ENTRANCE);

        constexpr float SUMMON_DISTANCE = 20.0f;
        uint32 const entry = _dummyEntry;
        uint32 const level = _level;
        ctx.Do("summon dummy", [entry, level](TestContext& ctx)
        {
            Player* leader = ctx.GetBot(0);
            if (!leader)
                return false;

            float const x = leader->GetPositionX() + SUMMON_DISTANCE * std::cos(leader->GetOrientation());
            float const y = leader->GetPositionY() + SUMMON_DISTANCE * std::sin(leader->GetOrientation());
            float z = leader->GetPositionZ();
            leader->UpdateGroundPositionZ(x, y, z);
            Creature* dummy = leader->SummonCreature(entry, x, y, z,
                leader->GetOrientation() + float(M_PI), TEMPSUMMON_MANUAL_DESPAWN);
            if (!dummy)
                return false;

            dummy->AIM_Initialize(new NullCreatureAI(dummy));
            // Boss-level (+3) hit/crit/glancing tables regardless of the template level.
            dummy->SetLevel(level + 3);
            dummy->SetRegeneratingHealth(false);
            dummy->SetMaxHealth(2000000000u);
            dummy->SetFullHealth();
            dummy->Attack(leader, true);

            ctx.bossGuid = dummy->GetGUID();
            ctx.MarkSkull(dummy->GetGUID());
            ctx.SetMetric("engage_at_ms", ctx.elapsedMs);
            return true;
        });

        ctx.Do("attack orders", [](TestContext& ctx)
        {
            ctx.OrderAttackAll();
            return true;
        });

        uint32 const durationSec = _durationSec;
        auto diagDone = std::make_shared<bool>(false);
        ctx.Step("dps window", (durationSec + 60) * IN_MILLISECONDS, [durationSec, diagDone](TestContext& ctx) -> StepStatus
        {
            uint32 const windowMs = ctx.elapsedMs - uint32(ctx.metrics["engage_at_ms"]);

            // Per-bot state at 60s: names target-drop failures (cleared target or no LOS).
            if (!*diagDone && windowMs >= 60000)
            {
                *diagDone = true;
                Player* ref0 = ctx.FirstAliveBot();
                Creature* d = ref0 ? ObjectAccessor::GetCreature(*ref0, ctx.bossGuid) : nullptr;
                ctx.AddNote(Acore::StringFormat("[60s] dummy victim {} combat {}",
                    d && d->GetVictim() ? d->GetVictim()->GetName() : "none", d ? d->IsInCombat() : false));
                for (size_t i = 0; i < ctx.botGuids.size(); ++i)
                    if (Player* bot = ctx.GetBot(i))
                    {
                        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
                        GuidVector attackers = botAI
                            ? botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get()
                            : GuidVector();
                        ctx.AddNote(Acore::StringFormat(
                            "[60s] {} [{}] dist {:.0f} combat {} target {} victim {} attackers {} los {}",
                            bot->GetName(), i < ctx.botSpecs.size() ? ctx.botSpecs[i] : "?",
                            d ? bot->GetDistance(d) : -1.0f, bot->IsInCombat(),
                            bot->GetTarget() == ctx.bossGuid ? "dummy" : bot->GetTarget().ToString().substr(0, 30),
                            bot->GetVictim() && d && bot->GetVictim() == d->ToUnit() ? "dummy" : "none",
                            attackers.size(), d ? bot->IsWithinLOSInMap(d) : false));
                    }
            }
            if (windowMs >= durationSec * IN_MILLISECONDS)
            {
                ctx.SetMetric("kill_time_ms", windowMs);  // bench "kill" column = the window
                return StepStatus::Done;
            }

            Player* ref = ctx.FirstAliveBot();
            Creature* dummy = ref ? ObjectAccessor::GetCreature(*ref, ctx.bossGuid) : nullptr;
            if (!dummy || !dummy->IsAlive())
                return StepStatus::Failed;

            return StepStatus::InProgress;
        });

        ctx.Do("despawn dummy", [](TestContext& ctx)
        {
            Player* ref = ctx.FirstAliveBot();
            if (Creature* dummy = ref ? ObjectAccessor::GetCreature(*ref, ctx.bossGuid) : nullptr)
                dummy->DespawnOrUnsummon();
            return true;
        });
    }

private:
    std::string _name;
    uint32 _level;
    std::vector<BotDef> _comp;
    GearDef _gear;
    uint32 _dummyEntry;
    uint32 _durationSec;
    uint8 _progression;
};

std::unique_ptr<TestScenario> MakeDpsDummyScenario(std::string name, uint32 level, std::vector<BotDef> comp,
                                                   GearDef gear, uint32 dummyEntry, uint32 durationSec,
                                                   uint8 progression)
{
    return std::make_unique<DpsDummyScenario>(std::move(name), level, std::move(comp), gear,
                                              dummyEntry, durationSec, progression);
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
