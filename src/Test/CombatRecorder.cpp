/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS

#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "StringFormat.h"
#include "TestRunner.h"
#include "Unit.h"

// Thin forwarder: all routing/state lives in IntegrationTestMgr, which maps
// participants (bots, watched bosses) to their owning run.
class TestCombatRecorder : public UnitScript
{
public:
    TestCombatRecorder() : UnitScript("TestCombatRecorder", true, { UNITHOOK_ON_DAMAGE, UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK }) { }

    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
    {
        IntegrationTestMgr::instance().RecordDamage(attacker, victim,
            attacker ? attacker->GetName() : "environment", damage);
    }

    // Periodic aura ticks (DoTs, slime/lava zone auras) bypass OnDamage — without
    // this hook they are invisible in death recaps.
    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* spellInfo) override
    {
        // The hook also sees positive periodics (HoT ticks) — recording those as
        // damage pollutes death recaps.
        if (spellInfo && spellInfo->IsPositive())
            return;

        std::string source = attacker ? attacker->GetName() : "aura";
        if (spellInfo)
            source += Acore::StringFormat(" [dot {}]", spellInfo->Id);
        IntegrationTestMgr::instance().RecordDamage(attacker, target, std::move(source), damage);
    }
};

void AddPlayerbotsTestCombatRecorder()
{
    new TestCombatRecorder();
}

#endif  // PLAYERBOTS_INTEGRATION_TESTS
