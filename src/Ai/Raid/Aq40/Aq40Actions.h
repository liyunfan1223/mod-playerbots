/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40ACTIONS_H
#define PLAYERBOTS_AQ40ACTIONS_H

#include "AttackAction.h"
#include "Multiplier.h"
#include "Trigger.h"
#include <array>
#include <chrono>
#include <map>
#include <type_traits>

namespace Aq40
{
template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
constexpr uint32 Id(T value)
{
    return static_cast<uint32>(value);
}

// Values match the instance script's boss data ids.
enum class Aq40Encounter : uint32
{
    None,
    Skeram,
    Trio,
    Sartura,
    Fankriss,
    Viscidus,
    Huhuran,
    Twins,
    Ouro,
    Cthun
};

enum class Aq40Npcs : uint32
{
    // The Prophet Skeram
    NPC_SKERAM = 15263,

    // Bug Trio
    NPC_KRI = 15511,
    NPC_YAUJ = 15543,
    NPC_VEM = 15544,
    NPC_YAUJ_BROOD = 15621,
    NPC_POISON_CLOUD = 15933,

    // Battleguard Sartura
    NPC_SARTURA = 15516,
    NPC_SARTURA_ROYAL_GUARD = 15984,

    // Fankriss the Unyielding
    NPC_FANKRISS = 15510,
    NPC_SPAWN_OF_FANKRISS = 15630,
    NPC_VEKNISS_HATCHLING = 15962,

    // Viscidus
    NPC_VISCIDUS = 15299,
    NPC_GLOB_OF_VISCIDUS = 15667,
    NPC_TOXIC_SLIME = 15925,

    // Princess Huhuran
    NPC_HUHURAN = 15509,

    // Twin Emperors
    NPC_VEKNILASH = 15275,
    NPC_VEKLOR = 15276,
    NPC_QIRAJI_SCARAB = 15316,
    NPC_QIRAJI_SCORPION = 15317,
    NPC_ANUBISATH_DEFENDER = 15277,

    // Ouro
    NPC_OURO = 15517,
    NPC_DIRT_MOUND = 15712,
    NPC_OURO_SCARAB = 15718,

    // C'Thun
    NPC_EYE_OF_CTHUN = 15589,
    NPC_CTHUN = 15727,
    NPC_EYE_TENTACLE = 15726,
    NPC_CLAW_TENTACLE = 15725,
    NPC_GIANT_EYE_TENTACLE = 15334,
    NPC_GIANT_CLAW_TENTACLE = 15728,
    NPC_FLESH_TENTACLE = 15802,
    NPC_EXIT_TRIGGER = 15800
};

enum class Aq40Spells : uint32
{
    // The Prophet Skeram
    SPELL_TRUE_FULFILLMENT = 785,
    SPELL_ARCANE_EXPLOSION = 26192,

    // Bug Trio
    SPELL_YAUJ_HEAL = 25807,

    // Battleguard Sartura
    SPELL_WHIRLWIND = 26083,
    SPELL_GUARD_WHIRLWIND = 26038,

    // Fankriss the Unyielding
    SPELL_MORTAL_WOUND = 25646,

    // Viscidus
    SPELL_VISCIDUS_FREEZE = 25937,
    SPELL_INVIS_SELF = 25905,
    SPELL_TOXIN = 26575,

    // Princess Huhuran
    SPELL_FRENZY = 26051,
    SPELL_WYVERN_STING = 26180,
    SPELL_ACID_SPIT = 26050,
    SPELL_POISON_BOLT = 26052,

    // Twin Emperors
    SPELL_MUTATE_BUG = 802,
    SPELL_EXPLODE_BUG = 804,
    SPELL_TWIN_TELEPORT = 799,
    SPELL_TWIN_TELEPORT_VISUAL = 26638,
    SPELL_BLIZZARD = 26607,

    // Anubisath Defender
    SPELL_SHADOW_FROST_REFLECT = 19595,
    SPELL_FIRE_ARCANE_REFLECT = 13022,

    // Ouro
    SPELL_SAND_BLAST = 26102,

    // C'Thun
    SPELL_DARK_GLARE = 26029,
    SPELL_EYE_BEAM = 26134,
    SPELL_RED_COLORATION = 22518,
    SPELL_DIGESTIVE_ACID = 26476,
    SPELL_CARAPACE_CTHUN = 26156,
    SPELL_PURPLE_COLORATION = 22581,  // shown while C'Thun is Weakened

    // Druid
    SPELL_TREE_OF_LIFE = 33891
};

// Viscidus: Toxin (25989) is a 5-yard persistent cloud that never despawns, so the room fills up.
// The raid is pulled from a ramp that descends from the entrance (z -24) to the floor (z -45..-53).
constexpr float ToxinRadius = 5.0f;
constexpr float ToxinMargin = 3.0f;
constexpr float ViscidusRampZ = -40.0f;
constexpr float ViscidusRampY = 985.0f;
constexpr float ViscidusRingOffset = 18.0f;
constexpr float ViscidusRingTolerance = 6.0f;
constexpr float ViscidusStrandedOffset = 40.0f;
inline Position const ViscidusRoomCenter{-7992.36f, 908.19f, -52.62f};

bool InStomach(Player* player);
bool Whirling(Unit* unit);
uint32 Stacks(Unit* unit, uint32 spell);
}  // namespace Aq40

// State is owned by one group bot's action context, not a process-wide instance cache.
// All persistent references are GUIDs; dead owners remain eligible to avoid reset on every death.
class Aq40ControlAction : public AttackAction
{
public:
    Aq40ControlAction(PlayerbotAI* botAI) : AttackAction(botAI, "aq40 control") {}
    static Aq40ControlAction* Get(PlayerbotAI* botAI);
    void Refresh(PlayerbotAI* observer);
    bool Execute(Event event) override;
    Aq40::Aq40Encounter Encounter() const { return _encounter; }
    // Changes whenever the shared encounter state is reset or refreshed.
    uint32 Version() const { return _version; }
    std::vector<Creature*> Units(uint32 entry = 0);
    std::vector<Player*> Members(bool tanks = false);
    Player* Member(ObjectGuid guid);
    Creature* Boss();
    Player* TankFor(Unit* unit);
    Unit* Target(Player* player);
    bool IsTankRole(Player* player);
    bool IsTwinLock(Player* player);
    bool IsTwinWaiter(Unit* unit) const { return unit && _twinWaiter && unit->GetGUID() == _twinWaiter; }
    bool IsTwinPlatformTank(Unit* unit) const
    {
        return unit && ((_twinTanks[0] && unit->GetGUID() == _twinTanks[0]) ||
                        (_twinTanks[1] && unit->GetGUID() == _twinTanks[1]));
    }
    // A tank-spec bot without a platform melees Vek'nilash as the second name on his threat list.
    bool IsTwinSpareTank(Player* player);
    Position TwinUnstuckSpot(Unit* emperor);
    Position TwinLockSpot(uint32 side) const;
    bool TwinSwapWindow() const;
    uint32 TwinSideOf(Position const& pos) const;
    // Healing threat splits onto both emperors, and Vek'nilash cannot be taunted back. For the first
    // seconds of the pull healers only save players in real danger while the tanks build threat.
    bool TwinOpeningHold(Unit* target) const
    {
        return _encounter == Aq40::Aq40Encounter::Twins && target && target->GetHealthPct() >= 60.0f &&
               std::chrono::steady_clock::now() - _encounterStart < std::chrono::seconds(8);
    }
    bool Held(Unit* unit);
    bool Danger(Player* player, Position& goal);
    // Phase 1: where a bot in Dark Glare's path steps to, kept on the floor.
    Position GlareDodge(Player* player, Unit* eye, float bearing, float distance);
    bool Formation(Player* player, Position& goal);
    bool UrgentHeal(PlayerbotAI* botAI);
    bool Interrupt(PlayerbotAI* botAI);
    bool GiantEyeResponse(PlayerbotAI* botAI);
    bool Special(PlayerbotAI* botAI);
    bool AllowedDamage(Player* player, Unit* unit);
    bool StomachExitNeeded(Player* player);
    // C'Thun: each raid member's spot from Aq40Rules::CthunSlots, and what it may hit from there.
    bool CthunSpot(Player* player, Position& spot);
    bool CthunMeleeBurn(Player* player);
    bool CthunEyeMelee(Player* player);
    Unit* CthunTarget(Player* player);
    bool CthunPhase2();
    // Phase 2: the two ranged bots that take the Eye Tentacles out of reach of the stack.
    bool CthunTentacleKiller(Player* player);
    bool NeedSwap(Unit* boss, uint32 aura, uint32 stacks);
    bool SkeramPlatform(Player* player, Position& platform);
    Position ViscidusRing(Player* player, Creature* boss);
    bool ToxinEscape(Player* player, Position& goal);

private:
    void Reset();
    void AssignTanks();
    void AssignSkeram(std::vector<Player*> const& tanks);
    Unit* DamageTarget(Player* player);
    Position TwinWaitSpot(Player* player, Unit* veklor);
    ObjectGuid _group;
    Aq40::Aq40Encounter _encounter = Aq40::Aq40Encounter::None;
    std::vector<ObjectGuid> _units;
    std::map<ObjectGuid, ObjectGuid> _owners;
    std::array<ObjectGuid, 3> _platformTanks{};
    std::array<Position, 3> _platforms{};
    bool _platformsReady = false;
    std::map<ObjectGuid, uint32> _skeramBodies;
    std::map<ObjectGuid, Position> _skeramPositions;
    std::array<ObjectGuid, 2> _twinTanks{};
    ObjectGuid _twinLock;                    // the warlock holding Vek'lor right now
    std::array<ObjectGuid, 2> _twinLocks{};  // per side: Square = side 0 (Vek'nilash's), Moon = side 1
    std::chrono::steady_clock::time_point _twinLastSwap{};
    ObjectGuid _twinWaiter;
    bool _twinTeleported = false;
    std::map<ObjectGuid, std::pair<float, std::chrono::steady_clock::time_point>> _waitAngles;
    std::array<Position, 2> _twinLast{};
    bool _twinLastReady = false;
    std::array<Position, 2> _twinSides{};
    std::map<ObjectGuid, uint32> _raidSides;
    bool _sidesReady = false;
    ObjectGuid _focus;
    ObjectGuid _mainTank;
    std::vector<ObjectGuid> _soakers;
    std::map<ObjectGuid, float> _angles;
    std::map<ObjectGuid, uint32> _cthunSpots;
    bool _cthunRedeal = false;
    ObjectGuid _cthunPuller;  // takes the three opening Eye Beams; stays put while the raid spreads
    float _lastGlare = 0.0f;
    float _glareDirection = 0.0f;
    bool _glareActive = false;
    std::map<ObjectGuid, std::pair<std::chrono::steady_clock::time_point, Position>> _glareDodges;
    std::chrono::steady_clock::time_point _refresh{};
    uint32 _version = 0;
    std::chrono::steady_clock::time_point _encounterStart{};
};

class Aq40SkeramInterruptAction : public Action
{
public:
    Aq40SkeramInterruptAction(PlayerbotAI* botAI) : Action(botAI, "aq40 skeram interrupt") {}
    bool Execute(Event event) override;
};

// Before the Twin Emperors pull, the raid-marked warlocks walk to their start spots on their own:
// neither "follow" (drags them back to the master) nor "stay" (keeps pulling them back mid-fight) works.
bool Aq40TwinPrepullSpot(PlayerbotAI* botAI, Position& spot);
bool Aq40CthunPrepullSpot(PlayerbotAI* botAI, Position& spot);

class Aq40TwinPrepullTrigger : public Trigger
{
public:
    Aq40TwinPrepullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twins prepull", 1) {}
    bool IsActive() override;
};

class Aq40TwinPrepullAction : public MovementAction
{
public:
    Aq40TwinPrepullAction(PlayerbotAI* botAI) : MovementAction(botAI, "aq40 twins prepull") {}
    bool Execute(Event event) override;
};

class Aq40Trigger : public Trigger
{
public:
    Aq40Trigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 encounter", 500) {}
    bool IsActive() override;
};

class Aq40MoveAction : public MovementAction
{
public:
    Aq40MoveAction(PlayerbotAI* botAI, bool safety, bool tactical = false)
        : MovementAction(botAI, safety     ? "aq40 safety"
                                : tactical ? "aq40 tactics"
                                           : "aq40 position"),
          _safety(safety),
          _tactical(tactical)
    {
    }
    bool Execute(Event event) override;

private:
    bool _safety;
    bool _tactical;
    std::chrono::steady_clock::time_point _exitAttempt{};
    std::chrono::steady_clock::time_point _spitAttempt{};
};

class Aq40Multiplier : public Multiplier
{
public:
    Aq40Multiplier(PlayerbotAI* botAI) : Multiplier(botAI, "aq40 encounter") {}
    float GetValue(Action* action) override;
};

#endif
