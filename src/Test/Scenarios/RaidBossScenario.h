/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifdef PLAYERBOTS_INTEGRATION_TESTS
#ifndef _PLAYERBOT_RAIDBOSSSCENARIO_H
#define _PLAYERBOT_RAIDBOSSSCENARIO_H

#include "TestScenario.h"

#include <string>
#include <vector>

class Creature;
class Player;

// How the fight is started.
enum class PullMode
{
    // Default: MT runs to the boss, tags, and drags him to the arena spot.
    WalkIn,
    // Roaming bosses / broken approach: relocate the whole pack to a validated
    // landing outside proximity-aggro range, then pull.
    Relocate,
};

// Generic single-boss raid scenario: prepare a raid at the arena spot, start
// the fight per PullMode, wait out the kill.
class RaidBossScenario : public TestScenario
{
public:
    struct Staging
    {
        Position pos;
        bool enabled = false;
    };

    RaidBossScenario(std::string name, uint32 bossEntry, uint32 mapId,
                     float x, float y, float z, float o,
                     std::vector<BotDef> comp, uint32 tankCount, uint32 level, GearDef gear,
                     uint32 fightTimeoutMs, std::vector<uint32> encounterAdds = {}, uint8 progression = 0,
                     Staging staging = {}, PullMode pullMode = PullMode::WalkIn);

    std::string GetName() const override { return _name; }
    void Setup(TestContext& ctx) override;

private:
    struct PullState
    {
        bool packLanded = false;
        uint32 lastDiagMs = 0;
        uint32 engagedAtMs = 0;
    };

    // First resolution finds the boss anywhere on the map and initializes the
    // fight bookkeeping (skull, engage metric, topped-off raid).
    static Creature* AcquireBoss(TestContext& ctx, Player* mt, uint32 bossEntry);
    // Engaged: anchor the fight at the arena spot before releasing the raid.
    static StepStatus TickDragHome(TestContext& ctx, Player* mt, Creature* boss,
                                   PullState& state, float spotX, float spotY, float spotZ);
    // Relocate mode: move boss + adds to a validated landing near the spot.
    static void RelocatePack(TestContext& ctx, Player* mt, Creature* boss,
                             std::vector<uint32> const& encounterAdds,
                             float spotX, float spotY, float spotZ);
    // Not engaged yet: close in (WalkIn) and issue the tag order.
    static void TickApproach(Player* mt, PlayerbotAI* mtAI, Creature* boss);
    static void NotePullDiagnostics(TestContext& ctx, Player* mt, Creature* boss, PullState& state);

    std::string _name;
    uint32 _bossEntry;
    uint32 _mapId;
    float _x, _y, _z, _o;
    std::vector<BotDef> _comp;
    uint32 _tankCount;
    uint32 _level;
    GearDef _gear;
    uint32 _fightTimeoutMs;
    std::vector<uint32> _encounterAdds;
    uint8 _progression;
    Staging _staging;
    PullMode _pullMode;
};

#endif
#endif  // PLAYERBOTS_INTEGRATION_TESTS
