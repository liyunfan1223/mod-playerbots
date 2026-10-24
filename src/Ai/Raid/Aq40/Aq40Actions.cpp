/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Actions.h"
#include "AiFactory.h"
#include "AllSpellScript.h"
#include "Aq40Rules.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "GenericSpellActions.h"
#include "InstanceScript.h"
#include "LastMovementValue.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Spell.h"
#include "SpellAuras.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <set>
#include <vector>

using namespace Aq40;

namespace
{
constexpr float PI = float(M_PI);
// Edge distance Vek'lor's tank keeps: Arcane Burst triggers within 5 yd; Demoralizing Shout (10 yd) must reach him.
constexpr float TwinBurstClearance = 6.5f;
constexpr uint32 Voidwalker = 1860;
constexpr uint8 TwinLockIcon = 4;        // Moon: the warlock who tanks Vek'lor on his own (north) side.
constexpr uint8 TwinSecondLockIcon = 5;  // Square: the warlock for Vek'nilash's (south) side.
// Tanking spots on the room floor, off the pedestals and their stairs. They are ~118 yd apart because the emperors heal
// each other within 60 yd and drift toward each other after swaps.
Position const TwinFloorSpots[2] = {{-9016.0f, 1262.0f, -112.3f}, {-8900.0f, 1284.0f, -112.3f}};
// Each side's warlock waits ~28 yards behind that side's spot, toward the entrance, with the healers.
Position const TwinLockSpots[2] = {{-9008.0f, 1288.0f, -112.3f}, {-8920.0f, 1304.0f, -112.3f}};
// Teleports come 29-40 s apart (first one at 30 s). Melee leave the landing spots before the window.
constexpr int TwinSwapWindowSeconds = 27;
// Center distance the Vek'lor warlock keeps: inside Searing Pain range, outside the bystander ring.
constexpr float TwinLockDistance = 26.0f;
constexpr float TwinLockMaxDistance = 35.0f;
// Center distance everyone but Vek'lor's tank keeps from him, so that tank (at about 14 yd) is clearly the nearest
// player when Vek'nilash lands in his place.
constexpr float TwinBystanderDistance = 24.0f;
// One large crit of headroom below the 130% ranged pull threshold on Vek'lor's tank.
constexpr float TwinCasterThreatMargin = 1500.0f;

// C'Thun: the Eye and C'Thun share this center. Spots are offsets from it (Aq40Rules::CthunSlots).
Position const CthunCenter{-8578.8f, 1986.2f, 100.7f};
// Melee leave their spot only for a tentacle whose edge is this close to the spot.
constexpr float CthunMeleeLeash = 10.0f;
// Ranged hit what their 30-yard spells reach from the spot (edge to edge, as GetDistance measures).
constexpr float CthunRangedReach = 29.0f;
// Area trigger 4036 (7 yd on the center) spits players out; clients fire it, server bots must send it.
constexpr uint32 CthunSpitOutTrigger = 4036;
constexpr float CthunSpitOutRadius = 6.5f;
// Weakened burn: C'Thun can be hit from about 10 yd of his center, although his combat reach is 1 yd and the server
// lets bot strikes land from ~26 yd. Melee stand at 8 yd, outside the 7-yd spit-out circle.
constexpr float CthunBurnMeleeReach = 10.0f;
// Phase 2: a Giant Claw farther than this from a bot's stack spot is left alone (see CthunTarget).
constexpr float CthunFarClaw = 20.0f;
// The tank marked with Moon walks in alone and pulls from the room's west edge, 76 yd from the Eye and away from the
// raid's path down the entrance slope, so the opening beams can't chain into the arriving raid.
constexpr uint8 CthunPullerIcon = 4;  // Moon (the Twins warlock mark; that use ends once the Twins are dead)
Position const CthunPullSpot{-8554.8f, 2058.2f, 100.7f};
constexpr float CthunBurnMeleeRadius = 8.0f;
// The opening beams land at 3, 6 and 9 s and random beams start at about 17 s.
constexpr int CthunPullerHoldSeconds = 20;
// Before the pull the raid trails the puller this far behind (the Eye Beam jumps about 13 yd).
constexpr float CthunTrailDistance = 22.0f;
// A trailing bot stays at least this much farther from the Eye than the puller.
constexpr float CthunTrailMargin = 10.0f;
// The pre-pull walk works only this close to the Eye and above the floor below the room.
constexpr float CthunPrepullReach = 140.0f;
constexpr float CthunPrepullMinZ = 90.0f;
// Phase 2 stack: Giant Claws and Giant Eyes spawn under a random player outside the stomach, so they spawn on the raid.
// Inside the Eye Tentacle ring (27.5 yd) to miss their spawn knockback, outside the 7-yd spit-out, in range of C'Thun.
Position const CthunStack{-8592.8f, 1986.2f, 100.4f};
constexpr float CthunStackRadius = 5.0f;
// Phase 2 stacks instead of keeping the phase-1 spread, which cost far more deaths and less damage on C'Thun.
constexpr bool CthunPhase2Stack = true;
// Phase 2 ranged chase Eye Tentacles anywhere in the room: their Mind Flay has no range limit.
constexpr float CthunTentacleChase = 70.0f;
// Phase 2 melee leave the stack for tentacles up to this far from their stack spot.
constexpr float CthunStackMeleeLeash = 30.0f;
// Phase 2 Eye Tentacles farther than this from the stack are out of reach for casters standing in it, and live
// long when every ranged bot picks the nearest tentacle; two ranged bots take only those.
constexpr float CthunFarTentacle = 30.0f;
constexpr uint32 CthunTentacleKillers = 2;
// Dark Glare band (5 yd) plus a margin, and how many 1-second ticks ahead a bot starts moving.
constexpr float GlareHalfWidth = 8.0f;
constexpr int GlareLookahead = 3;
// A glare dodge stays at least this far from the Eye, steps in this much at a time where the floor runs out, and
// needs floor this far past its goal.
constexpr float GlareMinRadius = 15.0f;
constexpr float GlareStepIn = 4.0f;
constexpr float GlareFloorMargin = 3.0f;
// Danger() runs several times per bot per tick, so the floor search runs at most this often per bot.
constexpr std::chrono::milliseconds GlareDodgeRefresh{500};
// C'Thun's floor ends 48-64 yd from the Eye in places, with a 17-yard drop below it. Ground is searched from just
// above the floor, and counts as floor within this height of it.
constexpr float CthunFloorProbeHeight = 2.0f;
constexpr float CthunFloorTolerance = 3.0f;

using Aq40Rules::AngleDelta;

Position Around(WorldObject* center, float radius, float angle)
{
    return Position(center->GetPositionX() + radius * std::cos(angle),
                    center->GetPositionY() + radius * std::sin(angle), center->GetPositionZ());
}

bool OnCthunFloor(WorldObject* probe, float x, float y)
{
    return probe->GetMapHeight(x, y, CthunCenter.GetPositionZ() + CthunFloorProbeHeight) >
           CthunCenter.GetPositionZ() - CthunFloorTolerance;
}

bool Attackable(Player* player, Unit* unit)
{
    return unit && unit->IsAlive() && unit->isTargetableForAttack() && player->IsValidAttackTarget(unit);
}

bool Passive(Unit* unit)
{
    Creature* creature = unit ? unit->ToCreature() : nullptr;
    return creature && creature->GetReactState() == REACT_PASSIVE;
}

void Stop(PlayerbotAI* botAI)
{
    botAI->GetBot()->GetMotionMaster()->Clear();
    botAI->GetBot()->StopMoving();
    botAI->GetAiObjectContext()->GetValue<LastMovement&>("last movement")->Get().clear();
}

void StopAttacking(Player* player)
{
    player->AttackStop();
    if (Pet* pet = player->GetPet())
    {
        pet->AttackStop();
        pet->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }
}

SpellInfo const* SpellFor(PlayerbotAI* botAI, std::string const& name)
{
    uint32 id = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
    return sSpellMgr->GetSpellInfo(id);
}

bool Cast(PlayerbotAI* botAI, std::string const& name, Unit* target, bool interruptCast = false)
{
    Player* player = botAI->GetBot();
    SpellInfo const* spell = SpellFor(botAI, name);
    if (!spell || !target || (player->HasUnitState(UNIT_STATE_CASTING) && !interruptCast) ||
        !player->IsWithinLOSInMap(target) ||
        player->GetDistance(target) > spell->GetMaxRange(spell->IsPositive(), player) ||
        target->IsImmunedToSpell(spell))
        return false;
    if (interruptCast)
    {
        if (!spell->HasEffect(SPELL_EFFECT_INTERRUPT_CAST) || !player->HasSpell(spell->Id) ||
            player->HasSpellCooldown(spell->Id) || player->HasUnitState(UNIT_STATE_LOST_CONTROL))
            return false;
        if (player->HasUnitState(UNIT_STATE_CASTING))
        {
            // Check readiness before sacrificing an in-progress heal or Frostbolt to Counterspell.
            Spell check(player, spell, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
            check.m_targets.SetUnitTarget(target);
            SpellCastResult result = check.CheckCast(true);
            if (result != SPELL_CAST_OK && result != SPELL_FAILED_NOT_INFRONT &&
                result != SPELL_FAILED_UNIT_NOT_INFRONT)
                return false;
            player->CastStop();
        }
    }
    if (player->isMoving() && spell->CalcCastTime())
        Stop(botAI);
    return botAI->CanCastSpell(spell->Id, target) && botAI->CastSpell(spell->Id, target);
}

std::string Taunt(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_WARRIOR:
            return "taunt";
        case CLASS_DRUID:
            return "growl";
        case CLASS_PALADIN:
            return "hand of reckoning";
        case CLASS_DEATH_KNIGHT:
            return "dark command";
        default:
            return {};
    }
}

bool IsEncounterBoss(uint32 entry)
{
    switch (entry)
    {
        case Id(Aq40Npcs::NPC_SKERAM):
        case Id(Aq40Npcs::NPC_KRI):
        case Id(Aq40Npcs::NPC_YAUJ):
        case Id(Aq40Npcs::NPC_VEM):
        case Id(Aq40Npcs::NPC_SARTURA):
        case Id(Aq40Npcs::NPC_FANKRISS):
        case Id(Aq40Npcs::NPC_VISCIDUS):
        case Id(Aq40Npcs::NPC_HUHURAN):
        case Id(Aq40Npcs::NPC_VEKNILASH):
        case Id(Aq40Npcs::NPC_VEKLOR):
        case Id(Aq40Npcs::NPC_OURO):
        case Id(Aq40Npcs::NPC_EYE_OF_CTHUN):
        case Id(Aq40Npcs::NPC_CTHUN):
            return true;
        default:
            return false;
    }
}

std::vector<uint32> Entries(Aq40::Aq40Encounter encounter)
{
    switch (encounter)
    {
        case Aq40Encounter::Skeram:
            return {Id(Aq40Npcs::NPC_SKERAM)};
        case Aq40Encounter::Trio:
            return {Id(Aq40Npcs::NPC_KRI), Id(Aq40Npcs::NPC_YAUJ), Id(Aq40Npcs::NPC_VEM), Id(Aq40Npcs::NPC_YAUJ_BROOD),
                    Id(Aq40Npcs::NPC_POISON_CLOUD)};
        case Aq40Encounter::Sartura:
            return {Id(Aq40Npcs::NPC_SARTURA), Id(Aq40Npcs::NPC_SARTURA_ROYAL_GUARD)};
        case Aq40Encounter::Fankriss:
            return {Id(Aq40Npcs::NPC_FANKRISS), Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS),
                    Id(Aq40Npcs::NPC_VEKNISS_HATCHLING)};
        case Aq40Encounter::Viscidus:
            return {Id(Aq40Npcs::NPC_VISCIDUS), Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS), Id(Aq40Npcs::NPC_TOXIC_SLIME)};
        case Aq40Encounter::Huhuran:
            return {Id(Aq40Npcs::NPC_HUHURAN)};
        case Aq40Encounter::Twins:
            return {Id(Aq40Npcs::NPC_VEKNILASH), Id(Aq40Npcs::NPC_VEKLOR), Id(Aq40Npcs::NPC_QIRAJI_SCARAB),
                    Id(Aq40Npcs::NPC_QIRAJI_SCORPION)};
        case Aq40Encounter::Ouro:
            return {Id(Aq40Npcs::NPC_OURO), Id(Aq40Npcs::NPC_DIRT_MOUND), Id(Aq40Npcs::NPC_OURO_SCARAB)};
        case Aq40Encounter::Cthun:
            return {Id(Aq40Npcs::NPC_EYE_OF_CTHUN),       Id(Aq40Npcs::NPC_CTHUN),
                    Id(Aq40Npcs::NPC_EYE_TENTACLE),       Id(Aq40Npcs::NPC_CLAW_TENTACLE),
                    Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE),
                    Id(Aq40Npcs::NPC_FLESH_TENTACLE),     Id(Aq40Npcs::NPC_EXIT_TRIGGER)};
        default:
            return {};
    }
}
}  // namespace

bool Aq40::InStomach(Player* player)
{
    return player->HasAura(Id(Aq40Spells::SPELL_DIGESTIVE_ACID)) || player->GetPositionZ() < -40.0f;
}

bool Aq40::Whirling(Unit* unit)
{
    return unit &&
           (unit->HasAura(Id(Aq40Spells::SPELL_WHIRLWIND)) || unit->HasAura(Id(Aq40Spells::SPELL_GUARD_WHIRLWIND)));
}

uint32 Aq40::Stacks(Unit* unit, uint32 spell)
{
    Aura* aura = unit ? unit->GetAura(spell) : nullptr;
    return aura ? aura->GetStackAmount() : 0;
}

Aq40ControlAction* Aq40ControlAction::Get(PlayerbotAI* botAI)
{
    Player* owner = botAI->GetBot();
    if (Group* group = owner->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member->IsInWorld() && member->GetMap() == botAI->GetBot()->GetMap() &&
                    member->GetGUID() < owner->GetGUID() && GET_PLAYERBOT_AI(member))
                    owner = member;
    PlayerbotAI* ownerAI = GET_PLAYERBOT_AI(owner);
    return static_cast<Aq40ControlAction*>(
        (ownerAI ? ownerAI : botAI)->GetAiObjectContext()->GetAction("aq40 control"));
}

void Aq40ControlAction::Reset()
{
    _group.Clear();
    _encounter = Aq40Encounter::None;
    _units.clear();
    _owners.clear();
    _focus.Clear();
    _mainTank.Clear();
    _platformTanks = {};
    _platformsReady = false;
    _skeramBodies.clear();
    _skeramPositions.clear();
    _twinTanks = {};
    _twinLock.Clear();
    _twinLocks = {};
    _twinWaiter.Clear();
    _twinTeleported = false;
    _waitAngles.clear();
    _twinLastReady = false;
    _sidesReady = false;
    _raidSides.clear();
    _soakers.clear();
    _angles.clear();
    _cthunSpots.clear();
    _cthunPuller.Clear();
    _cthunRedeal = false;
    _glareActive = false;
    _glareDirection = 0.0f;
    _glareDodges.clear();
    _refresh = {};
    ++_version;
}

std::vector<Player*> Aq40ControlAction::Members(bool tanks)
{
    std::vector<Player*> result;
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member->IsAlive() && member->IsInWorld() && member->GetMap() == bot->GetMap() &&
                    !member->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)) && (!tanks || PlayerbotAI::IsTank(member)))
                    result.push_back(member);
    std::sort(result.begin(), result.end(),
              [](Player* left, Player* right) { return left->GetGUID() < right->GetGUID(); });
    return result;
}

Player* Aq40ControlAction::Member(ObjectGuid guid)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    return player && player->IsAlive() && player->IsInWorld() && player->GetMap() == bot->GetMap() &&
                   player->GetGroup() == bot->GetGroup() && !player->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT))
               ? player
               : nullptr;
}

std::vector<Creature*> Aq40ControlAction::Units(uint32 entry)
{
    std::vector<Creature*> result;
    for (ObjectGuid guid : _units)
        if (Creature* unit = bot->GetMap()->GetCreature(guid))
            if (unit->IsAlive() && (!entry || unit->GetEntry() == entry))
                result.push_back(unit);
    return result;
}

Creature* Aq40ControlAction::Boss()
{
    for (Creature* unit : Units())
        if (IsEncounterBoss(unit->GetEntry()) && !unit->IsSummon())
            return unit;
    for (Creature* unit : Units())
        if (IsEncounterBoss(unit->GetEntry()))
            return unit;
    return nullptr;
}

void Aq40ControlAction::Refresh(PlayerbotAI* observer)
{
    Player* player = observer->GetBot();
    InstanceScript* instance = player->GetInstanceScript();
    if (player->GetMapId() != 531 || !instance || !player->GetGroup())
    {
        Reset();
        return;
    }
    Aq40::Aq40Encounter current = Aq40Encounter::None;
    for (uint32 id = Id(Aq40Encounter::Skeram); id <= Id(Aq40Encounter::Cthun); ++id)
        if (instance->GetBossState(id) == IN_PROGRESS)
        {
            current = static_cast<Aq40Encounter>(id);
            break;
        }
    if (current != _encounter || _group != player->GetGroup()->GetGUID())
    {
        Reset();
        _encounter = current;
        _encounterStart = std::chrono::steady_clock::now();
        _twinLastSwap = _encounterStart;
        _group = player->GetGroup()->GetGUID();
    }
    if (_encounter == Aq40Encounter::None)
        return;
    auto now = std::chrono::steady_clock::now();
    if (now - _refresh < std::chrono::milliseconds(250))
        return;
    _refresh = now;
    ++_version;
    // Instance boss GUID is the scan anchor, so a healer's visibility does not change assignments.
    WorldObject* anchor = player;
    if (Creature* boss = instance->GetCreature(Id(_encounter)))
        anchor = boss;
    else if (Creature* boss = Boss())
        anchor = boss;
    _units.clear();
    for (uint32 entry : Entries(_encounter))
    {
        std::list<Creature*> found;
        anchor->GetCreatureListWithEntryInGrid(found, entry, 220.0f);
        for (Creature* unit : found)
            if (unit->IsAlive())
                _units.push_back(unit->GetGUID());
    }
    std::sort(_units.begin(), _units.end());
    AssignTanks();
    if (_encounter == Aq40Encounter::Cthun)
        for (Creature* eye : Units(Id(Aq40Npcs::NPC_EYE_OF_CTHUN)))
        {
            // The puller entered alone and is the member nearest the Eye when the fight starts; the first Eye Beam
            // (3 s) confirms it. The Eye's victim is not reliable: a healer may hold its threat.
            if (!_cthunPuller)
                for (Player* member : Members())
                    if (Group* group = member->GetGroup();
                        group && group->GetTargetIcon(CthunPullerIcon) == member->GetGUID())
                        _cthunPuller = member->GetGUID();
            if (!_cthunPuller)
            {
                Player* nearest = nullptr;
                for (Player* member : Members())
                    if (!nearest || member->GetExactDist2d(eye) < nearest->GetExactDist2d(eye))
                        nearest = member;
                if (nearest)
                    _cthunPuller = nearest->GetGUID();
            }
            if (std::chrono::steady_clock::now() - _encounterStart < std::chrono::seconds(12))
                if (Spell* beam = eye->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (beam->m_spellInfo->Id == Id(Aq40Spells::SPELL_EYE_BEAM))
                        if (Unit* target = beam->m_targets.GetUnitTarget())
                            if (target->IsPlayer())
                                _cthunPuller = target->GetGUID();
            bool glare = eye->HasAura(Id(Aq40Spells::SPELL_RED_COLORATION));
            float angle = eye->GetOrientation();
            if (glare && _glareActive)
            {
                float delta = AngleDelta(angle, _lastGlare);
                if (std::abs(delta) > 0.01f && std::abs(delta) < 0.5f)
                    _glareDirection = delta > 0.0f ? 1.0f : -1.0f;
            }
            else
                _glareDirection = 0.0f;
            // After Dark Glare the raid has moved around ahead of the beam; re-deal spots so each bot takes the nearest
            // free one of its kind instead of walking back past the others, where Eye Beam chains them.
            if (_glareActive && !glare)
                _cthunRedeal = true;
            _glareActive = glare;
            _lastGlare = angle;
        }
}

void Aq40ControlAction::AssignTanks()
{
    std::vector<Player*> tanks = Members(true);
    if (tanks.empty())
        return;
    if (!Member(_mainTank))
    {
        auto main = std::find_if(tanks.begin(), tanks.end(), [](Player* p) { return PlayerbotAI::IsMainTank(p); });
        _mainTank = (main == tanks.end() ? tanks.front() : *main)->GetGUID();
    }
    if (_encounter == Aq40Encounter::Skeram)
    {
        AssignSkeram(tanks);
        return;
    }
    if (_encounter == Aq40Encounter::Twins)
    {
        auto physical = Units(Id(Aq40Npcs::NPC_VEKNILASH));
        auto caster = Units(Id(Aq40Npcs::NPC_VEKLOR));
        if (!_sidesReady && !physical.empty() && !caster.empty())
        {
            // Side 0 is Vek'nilash's spawn side. Sides use the floor spots: the emperors move at engage (Vek'lor walks
            // toward targets beyond 45 yd), and the pedestal stairs break their pathing so they heal to full.
            _twinSides = {TwinFloorSpots[0], TwinFloorSpots[1]};
            if (physical.front()->GetHomePosition().GetExactDist2d(&_twinSides[0]) >
                physical.front()->GetHomePosition().GetExactDist2d(&_twinSides[1]))
                std::swap(_twinSides[0], _twinSides[1]);
            _sidesReady = true;
            // The raid is stacked on its leader at the pull, so proximity puts nearly everyone on one
            // emperor. Deal each role out alternately instead, so both sides get healers, melee and casters.
            std::array<uint32, 3> dealt{};
            for (Player* member : Members())
            {
                if (PlayerbotAI::IsTank(member))
                    continue;
                uint32 role = PlayerbotAI::IsHeal(member) ? 0 : PlayerbotAI::IsMelee(member) ? 1 : 2;
                _raidSides[member->GetGUID()] = dealt[role]++ % 2;
            }
        }
        for (Player* member : Members())
            _raidSides.try_emplace(
                member->GetGUID(),
                member->GetExactDist2d(_twinSides[0]) < member->GetExactDist2d(_twinSides[1]) ? 0 : 1);
        // Vek'lor is not physically immune on this core, so one plate tank per platform holds whichever
        // emperor lands there and taunts on every swap. Warlocks are ordinary casters.
        auto assignSides = [&](std::array<ObjectGuid, 2>& owners, std::vector<Player*> const& candidates)
        {
            for (uint32 side = 0; side < 2; ++side)
            {
                if (Member(owners[side]))
                    continue;
                owners[side].Clear();
                Player* nearest = nullptr;
                for (Player* candidate : candidates)
                    if (candidate->GetGUID() != owners[1 - side] &&
                        (!nearest ||
                         candidate->GetExactDist2d(_twinSides[side]) < nearest->GetExactDist2d(_twinSides[side])))
                        nearest = candidate;
                if (nearest)
                    owners[side] = nearest->GetGUID();
            }
        };
        // Only bots take the platform roles. A human tank-spec player picked by proximity would own
        // an emperor the bots then never pick up.
        std::vector<Player*> botTanks;
        for (Player* tank : tanks)
            if (GET_PLAYERBOT_AI(tank))
                botTanks.push_back(tank);
        // Plate tanks with the most health take the platforms; other tank-spec bots (feral druid, arms/prot warrior)
        // only replace a dead platform tank, rather than whoever stood nearest at the first refresh.
        auto tankPoints = [](Player* tank)
        {
            uint8 tab = tank->getClass() == CLASS_WARRIOR ? 2 : tank->getClass() == CLASS_DEATH_KNIGHT ? 0 : 1;
            auto tabs = AiFactory::GetPlayerSpecTabs(tank);
            auto found = tabs.find(tab);
            return found == tabs.end() ? 0u : found->second;
        };
        std::stable_sort(botTanks.begin(), botTanks.end(),
                         [&](Player* left, Player* right)
                         {
                             bool leftPlate = left->getClass() != CLASS_DRUID;
                             bool rightPlate = right->getClass() != CLASS_DRUID;
                             if (leftPlate != rightPlate)
                                 return leftPlate;
                             uint32 leftPoints = tankPoints(left), rightPoints = tankPoints(right);
                             if (leftPoints != rightPoints)
                                 return leftPoints > rightPoints;
                             return left->GetMaxHealth() > right->GetMaxHealth();
                         });
        if (!Member(_twinTanks[0]) && !Member(_twinTanks[1]) && botTanks.size() >= 2)
        {
            Player* first = botTanks[0];
            Player* second = botTanks[1];
            float straight = first->GetExactDist2d(_twinSides[0]) + second->GetExactDist2d(_twinSides[1]);
            float crossed = first->GetExactDist2d(_twinSides[1]) + second->GetExactDist2d(_twinSides[0]);
            _twinTanks = straight <= crossed ? std::array<ObjectGuid, 2>{first->GetGUID(), second->GetGUID()}
                                             : std::array<ObjectGuid, 2>{second->GetGUID(), first->GetGUID()};
        }
        // A spare tank Vek'nilash is hitting takes over a platform: he can't be taunted back, and mid-room the emperors
        // heal each other. A vacant side goes first, else the side not on Vek'lor.
        if (!physical.empty() && !caster.empty())
            if (Unit* victim = physical.front()->GetVictim())
                if (Player* held = victim->ToPlayer();
                    held && std::find(botTanks.begin(), botTanks.end(), held) != botTanks.end() &&
                    held->GetGUID() != _twinTanks[0] && held->GetGUID() != _twinTanks[1])
                {
                    uint32 replace = !Member(_twinTanks[0]) ? 0 : !Member(_twinTanks[1]) ? 1 : 2;
                    for (uint32 side = 0; side < 2 && replace == 2; ++side)
                        if (caster.front()->GetVictim() != Member(_twinTanks[side]))
                            replace = side;
                    if (replace < 2)
                        _twinTanks[replace] = held->GetGUID();
                }
        assignSides(_twinTanks, botTanks);
        for (uint32 side = 0; side < 2; ++side)
            if (_twinTanks[side])
                _raidSides[_twinTanks[side]] = side;
        if (!physical.empty() && !caster.empty())
        {
            // Match both emperors together. A displaced boss must not give one tank both owners
            // and leave the opposite platform without a pickup assignment.
            float original =
                physical.front()->GetExactDist2d(_twinSides[0]) + caster.front()->GetExactDist2d(_twinSides[1]);
            float swapped =
                physical.front()->GetExactDist2d(_twinSides[1]) + caster.front()->GetExactDist2d(_twinSides[0]);
            // Keep the matching stable while both emperors are off their platforms: flipping owners on small moves
            // hands both to one tank. Re-match after a teleport, for a new or dead owner, or if clearly better.
            uint32 best = original <= swapped ? 0 : 1;
            uint32 physicalSide = best;
            if (_twinLastReady && physical.front()->GetExactDist2d(_twinLast[0]) < 30.0f &&
                caster.front()->GetExactDist2d(_twinLast[1]) < 30.0f && std::abs(original - swapped) < 40.0f)
                for (uint32 side = 0; side < 2; ++side)
                    if (_twinTanks[side] && _owners[physical.front()->GetGUID()] == _twinTanks[side] &&
                        _owners[caster.front()->GetGUID()] == _twinTanks[1 - side])
                        physicalSide = side;
            // Vek'nilash cannot be taunted, so the platform tank he is actually hitting keeps him and
            // brings him home; the other tank takes Vek'lor, who can be taunted.
            if (Unit* victim = physical.front()->GetVictim())
                for (uint32 side = 0; side < 2; ++side)
                    if (_twinTanks[side] && victim->GetGUID() == _twinTanks[side])
                        physicalSide = side;
            if (_twinLastReady && (physical.front()->GetExactDist2d(_twinLast[0]) > 30.0f ||
                                   caster.front()->GetExactDist2d(_twinLast[1]) > 30.0f))
            {
                _twinTeleported = true;
                _twinLastSwap = std::chrono::steady_clock::now();
            }
            _twinLast = {physical.front()->GetPosition(), caster.front()->GetPosition()};
            _twinLastReady = true;
            _owners[physical.front()->GetGUID()] = _twinTanks[physicalSide];
            _owners[caster.front()->GetGUID()] = _twinTanks[1 - physicalSide];
            // At the pull the warriors have no threat on Vek'lor and healing pulls him across the room, so a
            // raid-marked warlock bot holds him with Searing Pain while the other platform tank waits beside him.
            _twinWaiter = _twinTanks[1 - physicalSide];
            // One raid-marked warlock bot per side tanks Vek'lor whenever he is on that side; the side's warrior only
            // holds him for a few seconds after a teleport, since his Shadow Bolts (~3.5k every 2.5 s) kill warriors.
            _twinLocks = {};
            _twinLock.Clear();
            if (Group* group = bot->GetGroup())
                for (auto [icon, side] : {std::pair<uint8, uint32>{TwinLockIcon, 1}, {TwinSecondLockIcon, 0}})
                    if (Player* lock = Member(group->GetTargetIcon(icon)))
                        if (lock->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(lock) && !PlayerbotAI::IsTank(lock))
                            _twinLocks[side] = lock->GetGUID();
            uint32 lorSide = TwinSideOf(caster.front()->GetPosition());
            if (Player* lock = Member(_twinLocks[lorSide]))
            {
                _twinLock = lock->GetGUID();
                _owners[caster.front()->GetGUID()] = _twinLock;
            }
        }
        return;
    }
    std::map<ObjectGuid, uint32> loads;
    for (Player* tank : tanks)
        loads[tank->GetGUID()] = 0;
    for (auto it = _owners.begin(); it != _owners.end();)
        if (!Member(it->second) || !std::binary_search(_units.begin(), _units.end(), it->first))
            it = _owners.erase(it);
        else
        {
            ++loads[it->second];
            ++it;
        }
    for (Creature* unit : Units())
    {
        uint32 entry = unit->GetEntry();
        if (!Attackable(bot, unit) || entry == Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS) ||
            entry == Id(Aq40Npcs::NPC_FLESH_TENTACLE) || entry == Id(Aq40Npcs::NPC_EYE_TENTACLE) ||
            entry == Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE))
            continue;
        if (_owners.count(unit->GetGUID()))
            continue;
        Player* owner = nullptr;
        bool primary = IsEncounterBoss(entry) && entry != Id(Aq40Npcs::NPC_YAUJ) && entry != Id(Aq40Npcs::NPC_VEM) &&
                       !(entry == Id(Aq40Npcs::NPC_SKERAM) && unit->IsSummon());
        if (primary)
            owner = Member(_mainTank);
        if (!owner)
        {
            for (Player* tank : tanks)
            {
                if (tanks.size() > 1 && tank->GetGUID() == _mainTank)
                    continue;
                if (!owner || loads[tank->GetGUID()] < loads[owner->GetGUID()] ||
                    (loads[tank->GetGUID()] == loads[owner->GetGUID()] &&
                     tank->GetDistance(unit) < owner->GetDistance(unit)))
                    owner = tank;
            }
        }
        if (owner)
        {
            _owners[unit->GetGUID()] = owner->GetGUID();
            ++loads[owner->GetGUID()];
        }
    }
    if (_encounter == Aq40Encounter::Cthun)
        for (Creature* claw : Units(Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE)))
        {
            // The nearest tank takes each Giant Claw (the least-loaded one could be across the room while the
            // claw kills whoever it spawned under).
            Player* nearest = nullptr;
            for (Player* tank : tanks)
                if (!InStomach(tank) && (!nearest || tank->GetDistance(claw) < nearest->GetDistance(claw)))
                    nearest = tank;
            if (nearest)
                _owners[claw->GetGUID()] = nearest->GetGUID();
        }
    if (_encounter == Aq40Encounter::Ouro)
        if (Creature* boss = Boss())
            if (Unit* victim = boss->GetVictim())
                if (victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer()) && Member(victim->GetGUID()))
                {
                    // Sand Blast erases the hit tank's threat. Accept the next tank instead of taunting it back.
                    _owners[boss->GetGUID()] = victim->GetGUID();
                    _mainTank = victim->GetGUID();
                }
    if (_encounter == Aq40Encounter::Fankriss || _encounter == Aq40Encounter::Huhuran)
        if (Creature* boss = Boss())
            NeedSwap(boss,
                     _encounter == Aq40Encounter::Fankriss ? Id(Aq40Spells::SPELL_MORTAL_WOUND)
                                                           : Id(Aq40Spells::SPELL_ACID_SPIT),
                     _encounter == Aq40Encounter::Fankriss ? 3 : 5);
    if (_encounter == Aq40Encounter::Huhuran)
    {
        _soakers.erase(std::remove_if(_soakers.begin(), _soakers.end(), [&](ObjectGuid guid) { return !Member(guid); }),
                       _soakers.end());
        SpellInfo const* spell = sSpellMgr->GetSpellInfo(Id(Aq40Spells::SPELL_POISON_BOLT));
        uint32 count = spell ? spell->MaxAffectedTargets : 0;
        auto members = Members();
        std::stable_sort(
            members.begin(), members.end(),
            [](Player* left, Player* right)
            {
                auto rank = [](Player* p) {
                    return PlayerbotAI::IsTank(p) ? 0 : PlayerbotAI::IsHeal(p) ? 3 : PlayerbotAI::IsMelee(p) ? 1 : 2;
                };
                return rank(left) < rank(right);
            });
        for (Player* member : members)
            if (_soakers.size() < count &&
                std::find(_soakers.begin(), _soakers.end(), member->GetGUID()) == _soakers.end())
                _soakers.push_back(member->GetGUID());
    }
}

void Aq40ControlAction::AssignSkeram(std::vector<Player*> const& tanks)
{
    constexpr std::array<uint32, 3> blinks = {4801, 8195, 20449};
    constexpr uint32 center = 1;
    constexpr float teleportDistance = 20.0f;
    if (!_platformsReady)
    {
        for (uint32 index = 0; index < blinks.size(); ++index)
        {
            SpellTargetPosition const* point = sSpellMgr->GetSpellTargetPosition(blinks[index], EFFECT_0);
            if (!point)
                return;
            _platforms[index].Relocate(point->target_X, point->target_Y, point->target_Z);
        }
        _platformsReady = true;
        _platformTanks[center] = _mainTank;
    }

    std::set<ObjectGuid> assigned;
    for (ObjectGuid& guid : _platformTanks)
        if (Member(guid))
            assigned.insert(guid);
        else
            guid.Clear();
    for (uint32 index : {center, 0u, 2u})
    {
        if (!_platformTanks[index])
        {
            Player* nearest = nullptr;
            for (Player* tank : tanks)
                if (!assigned.count(tank->GetGUID()) &&
                    (!nearest || tank->GetExactDist(_platforms[index]) < nearest->GetExactDist(_platforms[index])))
                    nearest = tank;
            if (nearest)
            {
                _platformTanks[index] = nearest->GetGUID();
                assigned.insert(nearest->GetGUID());
            }
        }
        if (_platformTanks[index])
            _raidSides[_platformTanks[index]] = index;
    }

    // Balance healers, mages and rogues separately, so every platform gets healing and interrupts.
    auto members = Members();
    auto role = [](Player* player)
    {
        return PlayerbotAI::IsHeal(player)         ? 0u
               : player->getClass() == CLASS_MAGE  ? 1u
               : player->getClass() == CLASS_ROGUE ? 2u
                                                   : 3u;
    };
    std::array<std::array<uint32, 3>, 4> loads{};
    for (Player* member : members)
        if (!assigned.count(member->GetGUID()))
            if (auto it = _raidSides.find(member->GetGUID()); it != _raidSides.end())
                ++loads[role(member)][it->second];
    for (Player* member : members)
    {
        if (assigned.count(member->GetGUID()) || _raidSides.count(member->GetGUID()))
            continue;
        auto& counts = loads[role(member)];
        uint32 side = center;
        for (uint32 index : {0u, 2u})
            if (counts[index] < counts[side])
                side = index;
        _raidSides[member->GetGUID()] = side;
        ++counts[side];
    }

    _owners.clear();
    for (Creature* unit : Units(Id(Aq40Npcs::NPC_SKERAM)))
    {
        ObjectGuid guid = unit->GetGUID();
        if (Passive(unit))
        {
            // Images are initially summoned on the original, then teleported to their own platforms.
            _skeramBodies.erase(guid);
            _skeramPositions.erase(guid);
            continue;
        }
        auto previous = _skeramPositions.find(guid);
        if (!_skeramBodies.count(guid) || previous == _skeramPositions.end() ||
            unit->GetExactDist(previous->second) > teleportDistance)
        {
            uint32 side = center;
            for (uint32 index : {0u, 2u})
                if (unit->GetExactDist(_platforms[index]) < unit->GetExactDist(_platforms[side]))
                    side = index;
            _skeramBodies[guid] = side;
        }
        // Chasing a player down the stairs must not reassign the body to a different platform's tank.
        _skeramPositions[guid] = unit->GetPosition();
        _owners[guid] = _platformTanks[_skeramBodies[guid]];
    }
}

bool Aq40ControlAction::SkeramPlatform(Player* player, Position& platform)
{
    auto it = _raidSides.find(player->GetGUID());
    if (_encounter != Aq40Encounter::Skeram || !_platformsReady || it == _raidSides.end())
        return false;
    uint32 side = it->second;
    if (!PlayerbotAI::IsHeal(player))
    {
        bool tank = IsTankRole(player);
        Unit* target = Target(player);
        if (!target && !tank)
        {
            // The boss can briefly leave melee reach while following its tank to center.
            // Keep DPS moving to center during that damage hold instead of sending it back upstairs.
            if (Creature* original = Boss())
                if (!original->IsSummon() && AllowedDamage(player, original))
                    if (Player* owner = TankFor(original))
                        if (original->GetVictim() == owner && original->GetThreatMgr().GetThreat(owner) > 0.0f)
                            target = original;
        }
        if (target)
        {
            Player* owner = TankFor(target);
            if (!target->IsSummon() && owner && (!tank || owner == player))
            {
                constexpr uint32 centerPlatform = 1;
                bool acquired = target->GetVictim() == owner && target->GetThreatMgr().GetThreat(owner) > 0.0f;
                // Reach and pick up the real boss before bringing it back. Images stay on their platforms.
                if (tank || acquired)
                {
                    platform = acquired ? _platforms[centerPlatform] : target->GetPosition();
                    return true;
                }
            }
            // DPS joins an image's platform only after its assist tank has picked it up.
            if (!tank)
                if (auto body = _skeramBodies.find(target->GetGUID()); body != _skeramBodies.end())
                    side = body->second;
        }
    }
    platform = _platforms[side];
    return true;
}

bool Aq40ControlAction::NeedSwap(Unit* boss, uint32 aura, uint32 stacks)
{
    Player* owner = TankFor(boss);
    if (!owner || Stacks(owner, aura) < stacks)
        return false;
    for (Player* tank : Members(true))
        if (tank != owner && Stacks(tank, aura) < stacks && tank->GetDistance(boss) < 30.0f)
        {
            _owners[boss->GetGUID()] = tank->GetGUID();
            _mainTank = tank->GetGUID();
            return true;
        }
    return false;
}

Player* Aq40ControlAction::TankFor(Unit* unit)
{
    auto it = unit ? _owners.find(unit->GetGUID()) : _owners.end();
    return it != _owners.end() ? Member(it->second) : nullptr;
}

bool Aq40ControlAction::IsTankRole(Player* player) { return PlayerbotAI::IsTank(player); }

Position Aq40ControlAction::TwinUnstuckSpot(Unit* emperor)
{
    // The platform tops are where the emperors spawn, so their pathing is known good. The stairs down from them are
    // not: Vek'nilash can lose his path to a tank standing right next to him there.
    if (!_sidesReady)
        return emperor->GetPosition();
    uint32 side = emperor->GetExactDist2d(_twinSides[0]) < emperor->GetExactDist2d(_twinSides[1]) ? 0 : 1;
    float inward = _twinSides[side].GetAngle(&_twinSides[1 - side]);
    float offset =
        emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) ? emperor->GetCombatReach() + TwinBurstClearance + 2.0f : 4.0f;
    Position spot = _twinSides[side];
    spot.m_positionX += offset * std::cos(inward);
    spot.m_positionY += offset * std::sin(inward);
    return spot;
}

bool Aq40ControlAction::IsTwinSpareTank(Player* player)
{
    return _encounter == Aq40Encounter::Twins && IsTankRole(player) && !IsTwinPlatformTank(player);
}

bool Aq40ControlAction::IsTwinLock(Player* player)
{
    return _encounter == Aq40Encounter::Twins && player &&
           ((_twinLocks[0] && player->GetGUID() == _twinLocks[0]) ||
            (_twinLocks[1] && player->GetGUID() == _twinLocks[1]));
}

uint32 Aq40ControlAction::TwinSideOf(Position const& pos) const
{
    return pos.GetExactDist2d(&_twinSides[0]) < pos.GetExactDist2d(&_twinSides[1]) ? 0 : 1;
}

Position Aq40ControlAction::TwinLockSpot(uint32 side) const
{
    return TwinLockSpots[0].GetExactDist2d(&_twinSides[side]) < TwinLockSpots[1].GetExactDist2d(&_twinSides[side])
               ? TwinLockSpots[0]
               : TwinLockSpots[1];
}

bool Aq40ControlAction::TwinSwapWindow() const
{
    return _encounter == Aq40Encounter::Twins &&
           std::chrono::steady_clock::now() - _twinLastSwap >= std::chrono::seconds(TwinSwapWindowSeconds);
}

bool Aq40ControlAction::Held(Unit* unit)
{
    if (!unit || Passive(unit))
        return false;
    Player* owner = TankFor(unit);
    // Any tank-role member already holding the unit counts (a human tank, a replacement after a blink, or the previous
    // tank awaiting a taunt), so the raid's damage never stalls on the assigned tank.
    if (Unit* victim = unit->GetVictim())
        if (Player* tank = victim->ToPlayer())
            if (Member(tank->GetGUID()) && IsTankRole(tank))
                owner = tank;
    if (!owner || unit->GetVictim() != owner || unit->GetThreatMgr().GetThreat(owner) <= 0.0f)
        return false;
    // Vek'lor keeps casting after knocking his tank out of melee. That is still a valid pickup;
    // waiting for melee contact here otherwise freezes every caster on its old platform.
    if (_encounter == Aq40Encounter::Twins && unit->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
        return owner == TankFor(unit) && owner->GetDistance(unit) <= 45.0f;
    return owner->IsWithinMeleeRange(unit);
}

bool Aq40ControlAction::AllowedDamage(Player* player, Unit* unit)
{
    if (!Attackable(player, unit))
        return false;
    uint32 entry = unit->GetEntry();
    if (unit->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)) || Whirling(unit))
        return false;
    if (_encounter == Aq40Encounter::Cthun)
    {
        bool inside = InStomach(player);
        if ((entry == Id(Aq40Npcs::NPC_FLESH_TENTACLE)) != inside)
            return false;
        if (entry == Id(Aq40Npcs::NPC_CTHUN) && unit->HasAura(Id(Aq40Spells::SPELL_CARAPACE_CTHUN)))
            return false;
    }
    // The Eye is passive through Dark Glare but still takes damage, so players keep hitting it.
    if (IsEncounterBoss(entry) && Passive(unit) && entry != Id(Aq40Npcs::NPC_EYE_OF_CTHUN))
        return false;
    if (_encounter == Aq40Encounter::Twins && !IsTankRole(player))
    {
        // Damage follows the emperor, not the platform: Vek'nilash is immune to magic and anyone in Vek'lor's melee
        // range triggers Arcane Burst. Melee and hunters take Vek'nilash, casters take Vek'lor, crossing on every swap.
        bool physical = PlayerbotAI::IsMelee(player) || player->getClass() == CLASS_HUNTER;
        if ((entry == Id(Aq40Npcs::NPC_VEKLOR) && physical) || (entry == Id(Aq40Npcs::NPC_VEKNILASH) && !physical))
            return false;
    }
    if (_encounter == Aq40Encounter::Twins &&
        (entry == Id(Aq40Npcs::NPC_QIRAJI_SCARAB) || entry == Id(Aq40Npcs::NPC_QIRAJI_SCORPION)))
    {
        if (unit->HasAura(Id(Aq40Spells::SPELL_EXPLODE_BUG)))
            return false;
        if (unit->HasAura(Id(Aq40Spells::SPELL_MUTATE_BUG)))
            return true;
        // Totems, Blizzard and other area damage also pull the room's bugs. Kill any bug already
        // attacking the raid; leave idle critters alone.
        Unit* victim = unit->GetVictim();
        Player* attacked = victim ? victim->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        return attacked && Member(attacked->GetGUID());
    }
    return entry != Id(Aq40Npcs::NPC_POISON_CLOUD) && entry != Id(Aq40Npcs::NPC_TOXIC_SLIME) &&
           entry != Id(Aq40Npcs::NPC_DIRT_MOUND) && entry != Id(Aq40Npcs::NPC_EXIT_TRIGGER);
}

Unit* Aq40ControlAction::DamageTarget(Player* player)
{
    if (_encounter == Aq40Encounter::Skeram)
    {
        auto side = _raidSides.find(player->GetGUID());
        if (side == _raidSides.end())
            return nullptr;
        Creature* original = nullptr;
        for (Creature* unit : Units(Id(Aq40Npcs::NPC_SKERAM)))
        {
            if (!AllowedDamage(player, unit))
                continue;
            if (!unit->IsSummon())
                original = unit;
            else if (auto body = _skeramBodies.find(unit->GetGUID());
                     body != _skeramBodies.end() && body->second == side->second && Held(unit))
                return unit;
        }
        // Before images spawn, and until the assist tank picks up our image, help the main tank.
        // A missing or moving tank on another platform must not block this target.
        return original && Held(original) ? original : nullptr;
    }
    std::vector<uint32> priorities;
    switch (_encounter)
    {
        case Aq40Encounter::Trio:
            priorities = {Id(Aq40Npcs::NPC_YAUJ_BROOD), Id(Aq40Npcs::NPC_KRI), Id(Aq40Npcs::NPC_YAUJ),
                          Id(Aq40Npcs::NPC_VEM)};
            // A living bug marked skull explicitly overrides the default Kri/Yauj/Vem order.
            if (Group* group = player->GetGroup())
                if (Creature* marked = player->GetMap()->GetCreature(group->GetTargetIcon(7)))
                    if (marked->GetEntry() == Id(Aq40Npcs::NPC_KRI) || marked->GetEntry() == Id(Aq40Npcs::NPC_YAUJ) ||
                        marked->GetEntry() == Id(Aq40Npcs::NPC_VEM))
                        priorities.insert(priorities.begin() + 1, marked->GetEntry());
            break;
        case Aq40Encounter::Sartura:
            priorities = {Id(Aq40Npcs::NPC_SARTURA_ROYAL_GUARD), Id(Aq40Npcs::NPC_SARTURA)};
            break;
        case Aq40Encounter::Fankriss:
            priorities = {Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS), Id(Aq40Npcs::NPC_FANKRISS)};
            break;
        case Aq40Encounter::Viscidus:
            priorities = {Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS), Id(Aq40Npcs::NPC_VISCIDUS)};
            break;
        case Aq40Encounter::Huhuran:
            priorities = {Id(Aq40Npcs::NPC_HUHURAN)};
            break;
        case Aq40Encounter::Twins:
            priorities = PlayerbotAI::IsMelee(player) || player->getClass() == CLASS_HUNTER
                             ? std::vector<uint32>{Id(Aq40Npcs::NPC_QIRAJI_SCARAB), Id(Aq40Npcs::NPC_QIRAJI_SCORPION),
                                                   Id(Aq40Npcs::NPC_VEKNILASH)}
                             : std::vector<uint32>{Id(Aq40Npcs::NPC_QIRAJI_SCARAB), Id(Aq40Npcs::NPC_QIRAJI_SCORPION),
                                                   Id(Aq40Npcs::NPC_VEKLOR)};
            break;
        case Aq40Encounter::Ouro:
            priorities = {Id(Aq40Npcs::NPC_OURO_SCARAB), Id(Aq40Npcs::NPC_OURO)};
            break;
        case Aq40Encounter::Cthun:
            if (InStomach(player))
                priorities = {Id(Aq40Npcs::NPC_FLESH_TENTACLE)};
            else if (Creature* body = player->GetInstanceScript()->GetCreature(Id(Aq40Encounter::Cthun)))
                priorities =
                    body->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION))
                        ? std::vector<uint32>{Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_CTHUN),
                                              Id(Aq40Npcs::NPC_EYE_TENTACLE), Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE),
                                              Id(Aq40Npcs::NPC_CLAW_TENTACLE)}
                        : std::vector<uint32>{Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_EYE_TENTACLE),
                                              Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE), Id(Aq40Npcs::NPC_CLAW_TENTACLE),
                                              Id(Aq40Npcs::NPC_EYE_OF_CTHUN)};
            else
                priorities = {Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_EYE_TENTACLE),
                              Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE), Id(Aq40Npcs::NPC_CLAW_TENTACLE),
                              Id(Aq40Npcs::NPC_EYE_OF_CTHUN)};
            break;
        default:
            return nullptr;
    }
    for (uint32 entry : priorities)
    {
        auto candidates = Units(entry);
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                        [&](Creature* unit)
                                        {
                                            return !AllowedDamage(player, unit) ||
                                                   (_encounter == Aq40Encounter::Cthun && !InStomach(player) &&
                                                    unit->GetDistance(player) > 45.0f) ||
                                                   ((entry == Id(Aq40Npcs::NPC_QIRAJI_SCARAB) ||
                                                     entry == Id(Aq40Npcs::NPC_QIRAJI_SCORPION)) &&
                                                    unit->GetDistance(player) > 30.0f);
                                        }),
                         candidates.end());
        if (candidates.empty())
            continue;
        Creature* selected = candidates.front();
        bool localTarget = entry == Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS) || entry == Id(Aq40Npcs::NPC_FLESH_TENTACLE) ||
                           entry == Id(Aq40Npcs::NPC_EYE_TENTACLE) || entry == Id(Aq40Npcs::NPC_CLAW_TENTACLE) ||
                           entry == Id(Aq40Npcs::NPC_QIRAJI_SCARAB) || entry == Id(Aq40Npcs::NPC_QIRAJI_SCORPION);
        for (Creature* unit : candidates)
            if (localTarget ? unit->GetDistance(player) < selected->GetDistance(player) : unit->GetGUID() == _focus)
                selected = unit;
        if (!localTarget)
            _focus = selected->GetGUID();
        bool needsTank = IsEncounterBoss(entry) && entry != Id(Aq40Npcs::NPC_EYE_OF_CTHUN) &&
                         entry != Id(Aq40Npcs::NPC_CTHUN) && entry != Id(Aq40Npcs::NPC_VISCIDUS);
        needsTank = needsTank || entry == Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS) ||
                    entry == Id(Aq40Npcs::NPC_SARTURA_ROYAL_GUARD) || entry == Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE);
        return needsTank && !Held(selected) ? nullptr : selected;
    }
    return nullptr;
}

Unit* Aq40ControlAction::Target(Player* player)
{
    if (_encounter == Aq40Encounter::None || player->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)))
        return nullptr;
    if (_encounter == Aq40Encounter::Cthun && InStomach(player))
    {
        if (StomachExitNeeded(player))
            return nullptr;
        // No last-Flesh hold: waiting on the Giant tentacles outside wastes 36-94 s per window, while melee on C'Thun
        // and the Giant Eye response make the burn safe without it.
        return DamageTarget(player);
    }
    if (_encounter == Aq40Encounter::Cthun)
        return CthunTarget(player);
    if (IsTwinLock(player))
    {
        // Only the warlock on Vek'lor's side builds threat; the other one waits for his next phase.
        for (Creature* veklor : Units(Id(Aq40Npcs::NPC_VEKLOR)))
            if (TankFor(veklor) == player && AllowedDamage(player, veklor))
                return veklor;
        return nullptr;
    }
    // Vek'nilash cannot be taunted: when his tank dies he goes to the next name on his threat list.
    // A spare tank keeps that name a tank instead of a healer.
    if (IsTwinSpareTank(player))
    {
        // Bugs come first for damage dealers, but a spare tank busy on a scorpion builds no threat on Vek'nilash, who
        // then goes to a damage dealer when his tank dies.
        for (Creature* nilash : Units(Id(Aq40Npcs::NPC_VEKNILASH)))
            if (AllowedDamage(player, nilash) && Held(nilash))
                return nilash;
        return DamageTarget(player);
    }
    if (!IsTankRole(player))
        return DamageTarget(player);
    Unit* selected = nullptr;
    float score = -1.0f;
    for (Creature* unit : Units())
    {
        if (TankFor(unit) != player || !AllowedDamage(player, unit))
            continue;
        float current = unit->GetVictim() == player ? 1.0f : 10.0f;
        if (Unit* victim = unit->GetVictim())
            if (victim->IsPlayer() && PlayerbotAI::IsHeal(victim->ToPlayer()))
                current += 100.0f;
        if (unit->GetEntry() == Id(Aq40Npcs::NPC_SPAWN_OF_FANKRISS))
            current += 20.0f;
        if (!selected || current > score)
        {
            selected = unit;
            score = current;
        }
    }
    if (!selected && _encounter == Aq40Encounter::Ouro)
        if (Creature* boss = Boss())
            if (AllowedDamage(player, boss))
                return boss;  // Backup tanks must keep building threat before the next Sand Blast.
    // An unassigned off-tank can help damage, but must never taunt another tank's boss.
    if (!selected && _encounter != Aq40Encounter::Twins && _encounter != Aq40Encounter::Skeram)
        selected = DamageTarget(player);
    return selected;
}

bool Aq40ControlAction::StomachExitNeeded(Player* player)
{
    if (_encounter != Aq40Encounter::Cthun || !InStomach(player))
        return false;
    // A swallowed tank leaves at once: Giant Claws need it up top.
    if (IsTankRole(player))
        return true;
    bool healerInside = false;
    for (Player* member : Members())
        healerInside = healerInside || (member != player && PlayerbotAI::IsHeal(member) && InStomach(member));
    return Aq40Rules::LeaveStomach(Stacks(player, Id(Aq40Spells::SPELL_DIGESTIVE_ACID)), player->GetHealthPct(),
                                   !Units(Id(Aq40Npcs::NPC_FLESH_TENTACLE)).empty(), healerInside);
}

bool Aq40ControlAction::CthunSpot(Player* player, Position& spot)
{
    using Aq40Rules::CthunSlotKind;
    constexpr uint32 count = sizeof(Aq40Rules::CthunSlots) / sizeof(Aq40Rules::CthunSlots[0]);
    auto role = [this](Player* member)
    {
        if (PlayerbotAI::IsHeal(member))
            return CthunSlotKind::Outer;
        if (PlayerbotAI::IsMelee(member) || IsTankRole(member))
            return CthunSlotKind::EyeMelee;
        return CthunSlotKind::Middle;
    };
    auto taken = [this](uint32 index)
    {
        for (auto const& [guid, slot] : _cthunSpots)
            if (slot == index)
                return true;
        return false;
    };
    auto claim = [&](ObjectGuid guid, std::initializer_list<CthunSlotKind> kinds)
    {
        for (CthunSlotKind kind : kinds)
            for (uint32 index = 0; index < count; ++index)
                if (Aq40Rules::CthunSlots[index].kind == kind && !taken(index))
                {
                    _cthunSpots[guid] = index;
                    return true;
                }
        return false;
    };
    if (_cthunRedeal && !_cthunSpots.empty() && !CthunPhase2())
    {
        _cthunRedeal = false;
        std::map<ObjectGuid, uint32> old;
        old.swap(_cthunSpots);
        for (Player* member : Members())
        {
            auto previous = old.find(member->GetGUID());
            if (previous == old.end())
                continue;
            CthunSlotKind kind = Aq40Rules::CthunSlots[previous->second].kind;
            float best = std::numeric_limits<float>::max();
            uint32 bestIndex = previous->second;
            for (uint32 index = 0; index < count; ++index)
            {
                if (Aq40Rules::CthunSlots[index].kind != kind || taken(index))
                    continue;
                Position spot(CthunCenter.GetPositionX() + Aq40Rules::CthunSlots[index].x,
                              CthunCenter.GetPositionY() + Aq40Rules::CthunSlots[index].y, CthunCenter.GetPositionZ());
                float distance = member->GetExactDist2d(&spot);
                if (distance < best)
                {
                    best = distance;
                    bestIndex = index;
                }
            }
            _cthunSpots[member->GetGUID()] = bestIndex;
        }
        // Anyone not re-dealt (dead, in the stomach) keeps a free spot of its old kind if one is left.
        for (auto const& [guid, index] : old)
            if (!_cthunSpots.count(guid) && !taken(index))
                _cthunSpots[guid] = index;
    }
    if (_cthunSpots.empty())
    {
        // Deal the spots once per pull, by GUID: damage-dealing melee on the Eye first (tanks only if
        // short of melee), healers outside, ranged damage and the remaining melee in the middle ring.
        auto members = Members();
        for (bool tanks : {false, true})
            for (Player* member : members)
                if (role(member) == CthunSlotKind::EyeMelee && IsTankRole(member) == tanks)
                    claim(member->GetGUID(), {CthunSlotKind::EyeMelee});
        for (Player* member : members)
            if (role(member) == CthunSlotKind::Outer)
                claim(member->GetGUID(), {CthunSlotKind::Outer, CthunSlotKind::Middle});
        for (Player* member : members)
            if (!_cthunSpots.count(member->GetGUID()))
                claim(member->GetGUID(), {CthunSlotKind::Middle, CthunSlotKind::Outer, CthunSlotKind::EyeMelee});
    }
    auto found = _cthunSpots.find(player->GetGUID());
    if (found == _cthunSpots.end())
    {
        CthunSlotKind kind = role(player) == CthunSlotKind::EyeMelee ? CthunSlotKind::Middle : role(player);
        if (!claim(player->GetGUID(), {kind, CthunSlotKind::Middle, CthunSlotKind::Outer}))
            return false;
        found = _cthunSpots.find(player->GetGUID());
    }
    if (CthunPhase2Stack && CthunPhase2())
    {
        // A Giant Eye away from the stack is killed where it stands: the stack moves to it so melee can interrupt it
        // and casters see it. Left alone, one behind C'Thun takes no damage while its chaining beam kills the stack.
        Position center = CthunStack;
        for (Creature* giantEye : Units(Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)))
            if (giantEye->GetExactDist2d(&CthunStack) > 20.0f)
            {
                float angle = giantEye->GetAngle(&CthunStack);
                center.Relocate(giantEye->GetPositionX() + 8.0f * std::cos(angle),
                                giantEye->GetPositionY() + 8.0f * std::sin(angle), giantEye->GetPositionZ());
                break;
            }
        // A small spiral around the stack point, so bots don't all walk to one pixel.
        float index = float(found->second);
        float radius = CthunStackRadius * std::sqrt(std::fmod(index, 20.0f) / 20.0f);
        spot.Relocate(center.GetPositionX() + radius * std::cos(index * 2.4f),
                      center.GetPositionY() + radius * std::sin(index * 2.4f), center.GetPositionZ());
        return true;
    }
    Aq40Rules::CthunSlotSpot const& slot = Aq40Rules::CthunSlots[found->second];
    spot.Relocate(CthunCenter.GetPositionX() + slot.x, CthunCenter.GetPositionY() + slot.y, CthunCenter.GetPositionZ());
    return true;
}

bool Aq40ControlAction::CthunPhase2()
{
    // The Eye fakes its death (0 health, still "alive") when phase 2 starts.
    for (Creature* eye : Units(Id(Aq40Npcs::NPC_EYE_OF_CTHUN)))
        if (eye->GetHealth() > 0)
            return false;
    return true;
}

bool Aq40ControlAction::CthunTentacleKiller(Player* player)
{
    if (!CthunPhase2() || InStomach(player))
        return false;
    // Ranged damage bots only (never a human player): hunters first (longest reach), then mages, then the rest.
    std::vector<Player*> killers;
    for (Player* member : Members())
        if (GET_PLAYERBOT_AI(member) && !InStomach(member) && !PlayerbotAI::IsHeal(member) &&
            !PlayerbotAI::IsMelee(member) && !IsTankRole(member))
            killers.push_back(member);
    auto rank = [](Player* member) {
        return member->getClass() == CLASS_HUNTER ? 0u : member->getClass() == CLASS_MAGE ? 1u : 2u;
    };
    std::stable_sort(killers.begin(), killers.end(),
                     [&](Player* left, Player* right) { return rank(left) < rank(right); });
    for (uint32 index = 0; index < killers.size() && index < CthunTentacleKillers; ++index)
        if (killers[index] == player)
            return true;
    return false;
}

bool Aq40ControlAction::CthunMeleeBurn(Player* player)
{
    if (_encounter != Aq40Encounter::Cthun || InStomach(player) ||
        !(PlayerbotAI::IsMelee(player) || IsTankRole(player)))
        return false;
    Unit* target = Target(player);
    return target && target->GetEntry() == Id(Aq40Npcs::NPC_CTHUN) &&
           target->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION));
}

bool Aq40ControlAction::CthunEyeMelee(Player* player)
{
    if (CthunPhase2())
        return false;
    Position spot;
    if (!CthunSpot(player, spot))
        return false;
    auto found = _cthunSpots.find(player->GetGUID());
    return found != _cthunSpots.end() &&
           Aq40Rules::CthunSlots[found->second].kind == Aq40Rules::CthunSlotKind::EyeMelee;
}

Unit* Aq40ControlAction::CthunTarget(Player* player)
{
    // Nobody chases a Giant Claw far from the raid: with no one in melee range it submerges, heals and re-emerges under
    // a random player (boss_cthun), back in the stack. Chasing it strands tanks out of healer range.
    Position spot;
    bool haveSpot = CthunSpot(player, spot);
    auto nearRaid = [&](Unit* claw) { return haveSpot && claw->GetExactDist2d(&spot) <= CthunFarClaw; };
    // A tank takes its assigned Giant Claw when it is near the raid.
    if (IsTankRole(player))
        for (Creature* claw : Units(Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE)))
            if (TankFor(claw) == player && AllowedDamage(player, claw) && nearRaid(claw))
                return claw;
    // The tentacle killers take the far Eye Tentacles, even while C'Thun is Weakened. A Giant Eye still comes first.
    if (Units(Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)).empty() && CthunTentacleKiller(player))
    {
        Creature* best = nullptr;
        for (Creature* tentacle : Units(Id(Aq40Npcs::NPC_EYE_TENTACLE)))
            if (tentacle->GetExactDist2d(&CthunStack) > CthunFarTentacle && AllowedDamage(player, tentacle) &&
                (!best || player->GetDistance(tentacle) < player->GetDistance(best)))
                best = tentacle;
        if (best)
            return best;
    }
    bool melee = PlayerbotAI::IsMelee(player) || IsTankRole(player);
    bool eyeMelee = CthunEyeMelee(player);
    Creature* body =
        player->GetInstanceScript() ? player->GetInstanceScript()->GetCreature(Id(Aq40Encounter::Cthun)) : nullptr;
    // While C'Thun is Weakened, a Giant Eye still dies first: left alone it kills more of the raid during the window
    // than the burn gains. A Giant Claw a tank holds is left to that tank (killing it first takes a large part of
    // the window); one nobody holds is killed first.
    bool weakened = body && body->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION));
    std::vector<uint32> priorities =
        weakened ? std::vector<uint32>{Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE), Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE),
                                       Id(Aq40Npcs::NPC_CTHUN), Id(Aq40Npcs::NPC_EYE_TENTACLE),
                                       Id(Aq40Npcs::NPC_CLAW_TENTACLE)}
                 : std::vector<uint32>{Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE),  Id(Aq40Npcs::NPC_EYE_TENTACLE),
                                       Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE), Id(Aq40Npcs::NPC_CLAW_TENTACLE),
                                       Id(Aq40Npcs::NPC_EYE_OF_CTHUN),        Id(Aq40Npcs::NPC_CTHUN)};
    bool phase2 = CthunPhase2();
    for (uint32 entry : priorities)
    {
        Creature* best = nullptr;
        for (Creature* unit : Units(entry))
        {
            // Phase 2: a Giant Claw hits whoever it spawned under until its tank taunts it, so everyone
            // near starts on it at once instead of waiting for the tank.
            if (!AllowedDamage(player, unit) ||
                (entry == Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE) && !phase2 && !Held(unit)) ||
                (entry == Id(Aq40Npcs::NPC_GIANT_CLAW_TENTACLE) && phase2 &&
                 (!nearRaid(unit) || (weakened && Held(unit)))))
                continue;
            if (phase2)
            {
                if (entry == Id(Aq40Npcs::NPC_CTHUN))
                {
                    // Melee hit C'Thun only while he is Weakened; otherwise they leave him to the casters.
                    if (melee ? !unit->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION))
                              : player->GetDistance(unit) > CthunRangedReach)
                        continue;
                }
                else if (melee)
                {
                    // From the stack, melee cover the near half of the Eye Tentacle ring (up to 14 alive at once).
                    if (!haveSpot || unit->GetExactDist2d(&spot) > CthunStackMeleeLeash + unit->GetCombatReach())
                        continue;
                }
                else if (player->GetDistance(unit) >
                         (entry == Id(Aq40Npcs::NPC_EYE_TENTACLE) || entry == Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)
                              ? CthunTentacleChase
                              : CthunRangedReach))
                    continue;
            }
            else if (melee)
            {
                // Only the four Eye spots stand in the Eye's melee range; everyone else would stack on it.
                if (entry == Id(Aq40Npcs::NPC_EYE_OF_CTHUN) || entry == Id(Aq40Npcs::NPC_CTHUN))
                {
                    if (!eyeMelee)
                        continue;
                }
                else if (!haveSpot || unit->GetExactDist2d(&spot) > CthunMeleeLeash + unit->GetCombatReach())
                    continue;
            }
            else if (player->GetDistance(unit) > CthunRangedReach)
                continue;
            if (!best || player->GetDistance(unit) < player->GetDistance(best))
                best = unit;
        }
        if (best)
            return best;
    }
    return nullptr;
}

Position Aq40ControlAction::ViscidusRing(Player* player, Creature* boss)
{
    float ring = boss->GetCombatReach() + ViscidusRingOffset;
    Position slot = Around(boss, ring, boss->GetAngle(player));
    // The boss is usually held near the entrance, so the slot behind a ramp-bound bot is up the ramp.
    // Fan those bots out on the room-center side of the boss instead.
    if (slot.GetPositionY() > ViscidusRampY || player->GetPositionZ() > ViscidusRampZ)
    {
        auto [it, inserted] = _angles.emplace(player->GetGUID(), float(_angles.size()));
        float spread = (std::fmod(it->second, 5.0f) - 2.0f) * 0.35f;
        slot = Around(boss, ring, boss->GetAngle(&ViscidusRoomCenter) + spread);
    }
    return slot;
}

bool Aq40ControlAction::ToxinEscape(Player* player, Position& goal)
{
    // Clouds never despawn and stack up wherever the raid stands. Escape along the sum of every nearby
    // cloud's push, leaning toward the bot's ring slot, instead of stepping straight off the nearest one.
    constexpr float hazard = ToxinRadius + ToxinMargin;
    constexpr float awareness = hazard + 4.0f;
    float pushX = 0.0f;
    float pushY = 0.0f;
    bool inside = false;
    for (Creature* cloud : Units(Id(Aq40Npcs::NPC_TOXIC_SLIME)))
    {
        float distance = player->GetExactDist2d(cloud);
        if (distance > awareness)
            continue;
        inside = inside || distance < hazard;
        float weight = (awareness - distance) / std::max(distance, 0.5f);
        pushX += (player->GetPositionX() - cloud->GetPositionX()) * weight;
        pushY += (player->GetPositionY() - cloud->GetPositionY()) * weight;
    }
    if (!inside)
        return false;
    float push = std::sqrt(pushX * pushX + pushY * pushY);
    if (push > 0.01f)
    {
        pushX /= push;
        pushY /= push;
    }
    if (Creature* boss = Boss())
    {
        Position slot = ViscidusRing(player, boss);
        float dx = slot.GetPositionX() - player->GetPositionX();
        float dy = slot.GetPositionY() - player->GetPositionY();
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 1.0f)
        {
            pushX += 0.5f * dx / length;
            pushY += 0.5f * dy / length;
        }
    }
    float length = std::sqrt(pushX * pushX + pushY * pushY);
    if (length < 0.01f)
    {
        pushX = 1.0f;
        pushY = 0.0f;
        length = 1.0f;
    }
    goal.Relocate(player->GetPositionX() + hazard * pushX / length, player->GetPositionY() + hazard * pushY / length,
                  player->GetPositionZ());
    return true;
}

bool Aq40ControlAction::UrgentHeal(PlayerbotAI* botAI)
{
    Player* player = botAI->GetBot();
    if (!PlayerbotAI::IsHeal(player))
        return false;
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("party member to heal")->Get();
    // Twin Emperors tanks lose ~40% in a few seconds, so healers stop moving to heal much earlier there.
    float const threshold = _encounter == Aq40Encounter::Twins ? 75.0f : 40.0f;
    return target && target->IsAlive() && target->GetHealthPct() < threshold && target->GetMap() == player->GetMap() &&
           player->GetDistance(target) < botAI->GetRange("heal") - 2.0f && player->IsWithinLOSInMap(target);
}

bool Aq40ControlAction::Interrupt(PlayerbotAI* botAI)
{
    for (Creature* unit : Units())
    {
        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        // Eye Tentacles channel Mind Flay (750 a second for 10 s); kicks stop channels too.
        if (!spell && _encounter == Aq40Encounter::Cthun && unit->GetEntry() == Id(Aq40Npcs::NPC_EYE_TENTACLE))
            spell = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        bool skeram = _encounter == Aq40Encounter::Skeram && unit->GetEntry() == Id(Aq40Npcs::NPC_SKERAM);
        if (!spell || (skeram ? spell->m_spellInfo->Id != Id(Aq40Spells::SPELL_ARCANE_EXPLOSION)
                              : (unit->GetEntry() != Id(Aq40Npcs::NPC_YAUJ) &&
                                 unit->GetEntry() != Id(Aq40Npcs::NPC_EYE_TENTACLE) &&
                                 unit->GetEntry() != Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE))))
            continue;
        if (skeram && (spell->m_spellInfo->PreventionType != SPELL_PREVENTION_TYPE_SILENCE ||
                       !(spell->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT)))
            continue;
        if (_encounter == Aq40Encounter::Cthun && InStomach(botAI->GetBot()))
            continue;
        // A Giant Eye's beam chains through the phase-2 stack, so casters stop their own cast to counter it; the same
        // goes for Eye Tentacle Mind Flay (750 a second for 10 s).
        bool urgent = skeram || (_encounter == Aq40Encounter::Cthun &&
                                 (unit->GetEntry() == Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE) ||
                                  unit->GetEntry() == Id(Aq40Npcs::NPC_EYE_TENTACLE)));
        // No Earth Shock: since 3.0 it no longer interrupts (Wind Shear does), so it would only do damage.
        for (std::string const name :
             {"kick", "pummel", "shield bash", "wind shear", "counterspell", "mind freeze", "silencing shot"})
            if (Cast(botAI, name, unit, urgent))
                return true;
        // C'Thun's tentacles can be stunned (only the Eye and C'Thun are immune): a stun also stops the
        // Giant Eye's chaining beam, which it recasts every 2.1 s.
        if (_encounter == Aq40Encounter::Cthun)
            for (std::string const name :
                 {"hammer of justice", "kidney shot", "gouge", "concussion blow", "bash", "war stomp"})
                if (Cast(botAI, name, unit))
                    return true;
    }
    return false;
}

// A Giant Eye spawns under a random player, often in the phase-2 stack, and its Eye Beam chains through everyone within
// 13 yd at x1.5 a jump. Runs from the safety action, above movement and rotations: any ready interrupt, else a stun.
bool Aq40ControlAction::GiantEyeResponse(PlayerbotAI* botAI)
{
    Player* player = botAI->GetBot();
    if (_encounter != Aq40Encounter::Cthun || InStomach(player) ||
        player->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)))
        return false;
    static char const* const kicks[] = {"kick",         "pummel",      "shield bash",   "wind shear",
                                        "counterspell", "mind freeze", "silencing shot"};
    static char const* const stuns[] = {"hammer of justice", "concussion blow", "bash",
                                        "war stomp",         "kidney shot",     "gouge"};
    for (Creature* eye : Units(Id(Aq40Npcs::NPC_GIANT_EYE_TENTACLE)))
    {
        if (!eye->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            continue;
        for (char const* name : kicks)
            if (Cast(botAI, name, eye, true))
                return true;
        for (char const* name : stuns)
            if (Cast(botAI, name, eye))
                return true;
    }
    return false;
}

bool Aq40ControlAction::Special(PlayerbotAI* botAI)
{
    Player* player = botAI->GetBot();
    if (_encounter == Aq40Encounter::Skeram)
    {
        Group* group = player->GetGroup();
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (member->IsAlive() && member->GetMap() == player->GetMap() &&
                    member->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)) &&
                    !member->HasAuraType(SPELL_AURA_MOD_CONFUSE) && !member->HasAuraType(SPELL_AURA_MOD_STUN))
                    for (std::string const name : {"polymorph", "hammer of justice", "cyclone"})
                        if (Cast(botAI, name, member))
                            return true;
    }
    if (Interrupt(botAI))
        return true;
    if (_encounter == Aq40Encounter::Huhuran)
        if (Creature* boss = Boss())
        {
            if (boss->HasAura(Id(Aq40Spells::SPELL_FRENZY)) && Cast(botAI, "tranquilizing shot", boss))
                return true;
            // Dispel damage is exactly 3,000 in this server's aura script. Protect low-health sleepers.
            for (Player* member : Members())
                if (IsTankRole(member) && member->HasAura(Id(Aq40Spells::SPELL_WYVERN_STING)) &&
                    member->GetHealth() > 4500)
                    for (std::string const name : {"cleanse", "abolish poison", "cure poison", "cleanse spirit"})
                        if (Cast(botAI, name, member))
                            return true;
        }
    if (_encounter == Aq40Encounter::Viscidus)
        if (Creature* boss = Boss())
            if (Units(Id(Aq40Npcs::NPC_GLOB_OF_VISCIDUS)).empty() &&
                !boss->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)) &&
                !boss->HasAura(Id(Aq40Spells::SPELL_INVIS_SELF)) && !PlayerbotAI::IsHeal(player))
            {
                // Rank-one Frostbolt maximizes hit frequency; use the normal spell engine and costs.
                if (player->HasSpell(116) && player->GetDistance(boss) < 28.0f && player->IsWithinLOSInMap(boss))
                {
                    if (player->isMoving())
                        Stop(botAI);
                    if (botAI->CanCastSpell(116, boss) && botAI->CastSpell(116, boss))
                        return true;
                }
                for (std::string const name : {"frost shock", "icy touch", "ice lance"})
                    if (Cast(botAI, name, boss))
                        return true;
            }
    return false;
}

bool Aq40ControlAction::Execute(Event /*event*/)
{
    Aq40ControlAction* control = Get(botAI);
    control->Refresh(botAI);
    if (control->Encounter() == Aq40Encounter::None || bot->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)) ||
        bot->HasUnitState(UNIT_STATE_CASTING))
        return false;
    Position danger;
    if (control->Danger(bot, danger))
        return false;
    // Healing threat reaches both emperors: at the pull a healer can take Vek'lor while the waiter's Taunt is on
    // cooldown and walk him into the casters. Anyone not meant to hold an emperor sheds it with a threat drop.
    if (control->Encounter() == Aq40Encounter::Twins && !control->IsTankRole(bot) && !control->IsTwinLock(bot))
        for (Creature* emperor : control->Units())
            if ((emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) ||
                 emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH)) &&
                emperor->GetVictim() == bot)
            {
                char const* drop = bot->getClass() == CLASS_PRIEST   ? "fade"
                                   : bot->getClass() == CLASS_HUNTER ? "feign death"
                                   : bot->getClass() == CLASS_ROGUE  ? "vanish"
                                                                     : nullptr;
                if (drop && Cast(botAI, drop, bot))
                    return true;
            }
    if (control->Encounter() == Aq40Encounter::Twins && control->IsTankRole(bot))
    {
        // Unbalancing Strike plus a swing can take a tank from full to dead within 2 seconds, so waiting for 35% is too
        // late. Whoever Vek'nilash is hitting spends Shield Wall, then Last Stand, as soon as each is ready.
        bool holdingNilash = false;
        for (Creature* nilash : control->Units(Id(Aq40Npcs::NPC_VEKNILASH)))
            holdingNilash = holdingNilash || nilash->GetVictim() == bot;
        // One at a time, so the two cover separate windows.
        bool covered = botAI->HasAura("shield wall", bot) || botAI->HasAura("last stand", bot);
        if ((holdingNilash && !covered) || bot->GetHealthPct() < 35.0f)
            for (char const* cooldown : {"shield wall", "last stand"})
                if (!botAI->HasAura(cooldown, bot) && Cast(botAI, cooldown, bot))
                    return true;
    }
    if (control->Encounter() == Aq40Encounter::Twins && PlayerbotAI::IsHeal(bot))
    {
        // Unbalancing Strike + Uppercut + melee can remove a tank in about two seconds, so tank healers keep whoever
        // the emperors hit topped up. After a swap Vek'nilash is passive for 2 s; top up the tank he is about to hit.
        Player* focus = nullptr;
        for (Creature* emperor : control->Units())
            if (emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) || emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
            {
                Unit* victim = emperor->GetVictim();
                if (!victim && emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH))
                    victim = control->TankFor(emperor);
                if (Player* hit = victim ? victim->ToPlayer() : nullptr)
                    if (control->Member(hit->GetGUID()) && hit->GetHealthPct() < 90.0f &&
                        bot->GetDistance(hit) < 38.0f && bot->IsWithinLOSInMap(hit) &&
                        (!focus || hit->GetHealthPct() < focus->GetHealthPct()))
                        focus = hit;
            }
        if (focus && !control->TwinOpeningHold(focus))
        {
            std::vector<char const*> heals;
            switch (bot->getClass())
            {
                case CLASS_PRIEST:
                    if (!botAI->HasAura("weakened soul", focus))
                        heals.push_back("power word: shield");
                    heals.insert(heals.end(), {"flash heal", "greater heal", "heal"});
                    break;
                case CLASS_DRUID:
                    if (!botAI->HasAura("rejuvenation", focus, false, true))
                        heals.push_back("rejuvenation");
                    heals.insert(heals.end(), {"regrowth", "healing touch"});
                    break;
                case CLASS_SHAMAN:
                    heals.insert(heals.end(), {"lesser healing wave", "healing wave"});
                    break;
                case CLASS_PALADIN:
                    heals.insert(heals.end(),
                                 {focus->GetHealthPct() < 60.0f ? "holy light" : "flash of light", "holy light"});
                    break;
                default:
                    break;
            }
            for (char const* heal : heals)
                if (Cast(botAI, heal, focus))
                    return true;
        }
    }
    if (control->UrgentHeal(botAI))
        return false;
    if (control->Special(botAI))
        return true;
    // While C'Thun is Weakened there is no healing to be done, so healers damage him too.
    if (control->Encounter() == Aq40Encounter::Cthun && PlayerbotAI::IsHeal(bot) && !InStomach(bot))
        if (Creature* body =
                bot->GetInstanceScript() ? bot->GetInstanceScript()->GetCreature(Id(Aq40Encounter::Cthun)) : nullptr)
            if (body->HasAura(Id(Aq40Spells::SPELL_PURPLE_COLORATION)) && control->AllowedDamage(bot, body))
                for (char const* nuke : {"mind blast", "smite", "starfire", "wrath", "lightning bolt", "holy shock"})
                    if (Cast(botAI, nuke, body))
                        return true;
    // A swallowed healer damages the Flesh Tentacles: its heals can't reach the raid ~170 yd above. Heals on anyone
    // inside still rank above this.
    if (control->Encounter() == Aq40Encounter::Cthun && PlayerbotAI::IsHeal(bot) && InStomach(bot) &&
        !control->StomachExitNeeded(bot))
    {
        Creature* nearest = nullptr;
        for (Creature* flesh : control->Units(Id(Aq40Npcs::NPC_FLESH_TENTACLE)))
            if (control->AllowedDamage(bot, flesh) && (!nearest || bot->GetDistance(flesh) < bot->GetDistance(nearest)))
                nearest = flesh;
        // Tree of Life blocks Wrath, Moonfire and Starfire (NOT_SHAPESHIFT).
        if (nearest && bot->HasAura(Id(Aq40Spells::SPELL_TREE_OF_LIFE)))
            bot->RemoveAurasDueToSpell(Id(Aq40Spells::SPELL_TREE_OF_LIFE));
        if (nearest)
            for (char const* nuke : {"mind blast", "holy fire", "smite", "shadow word: pain", "wrath", "moonfire",
                                     "starfire", "lightning bolt", "flame shock", "holy shock", "exorcism",
                                     "judgement of light", "judgement of wisdom"})
                if (Cast(botAI, nuke, nearest))
                    return true;
    }
    if (control->Encounter() == Aq40Encounter::Twins && bot->getClass() == CLASS_WARLOCK)
        if (Pet* pet = bot->GetPet())
            if (pet->GetReactState() != REACT_PASSIVE)
            {
                // A demon near Vek'lor becomes Vek'nilash's nearest target after a swap, and even the
                // imp's Firebolt threat can take either emperor. Demons stay out of this fight.
                pet->AttackStop();
                pet->SetReactState(REACT_PASSIVE);
                pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            }
    if (PlayerbotAI::IsHeal(bot))
        return false;
    // A transfer owns the bot until it reaches its assigned emperor. Attack setup and
    // ranged casts can otherwise stop the same spline the tactics action just started.
    if (control->Encounter() == Aq40Encounter::Twins && !control->IsTankRole(bot))
    {
        Position formation;
        if (control->Formation(bot, formation))
        {
            StopAttacking(bot);
            return false;
        }
    }
    // Between his phases a side's warlock refills mana with Life Tap (never while tanking: a phase
    // costs ~4000 mana) and puts up Shadow Ward before the teleport window.
    if (control->Encounter() == Aq40Encounter::Twins && control->IsTwinLock(bot) && !control->Target(bot))
    {
        if (bot->GetPowerPct(POWER_MANA) < 85.0f && bot->GetHealthPct() > 60.0f && Cast(botAI, "life tap", bot))
            return true;
        if (control->TwinSwapWindow() && !botAI->HasAura("shadow ward", bot) && Cast(botAI, "shadow ward", bot))
            return true;
    }
    // The warrior waiting beside a warlock-held Vek'lor peels him off anyone else (the puller at the
    // opening, a healer later). His taunt copies only their small threat, so Searing Pain retakes him.
    if (control->Encounter() == Aq40Encounter::Twins && control->IsTwinWaiter(bot))
        for (Creature* veklor : control->Units(Id(Aq40Npcs::NPC_VEKLOR)))
            if (Player* lock = control->TankFor(veklor); lock && lock != bot && control->IsTwinLock(lock))
                if (Unit* victim = veklor->GetVictim(); victim && victim != lock && victim != bot)
                {
                    std::string taunt = Taunt(bot);
                    if (!taunt.empty() && Cast(botAI, taunt, veklor))
                        return true;
                }
    Unit* target = control->Target(bot);
    if (!target)
    {
        bool attacking = bot->GetVictim() || (bot->GetPet() && bot->GetPet()->GetVictim());
        StopAttacking(bot);
        return attacking;
    }
    if (control->TankFor(target) == bot && target->GetVictim() != bot && !Whirling(target))
    {
        std::string taunt = Taunt(bot);
        if (!taunt.empty() && Cast(botAI, taunt, target))
            return true;
    }
    if (control->IsTwinLock(bot) && target->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
    {
        // Vek'lor's Shadow Bolt hits a warlock for ~3-4k. Below half health, a Voidwalker shields him
        // with Sacrifice even though it stays passive for the rest of the fight.
        if (bot->GetHealthPct() < 50.0f)
            if (Pet* pet = bot->GetPet(); pet && pet->IsAlive() && pet->GetEntry() == Voidwalker)
                for (uint32 sacrifice : {47986u, 47985u, 27273u, 19443u, 19442u, 19441u, 19440u, 19438u, 7812u})
                    if (pet->HasSpell(sacrifice))
                    {
                        if (!pet->HasSpellCooldown(sacrifice) && !pet->HasUnitState(UNIT_STATE_CASTING))
                            pet->CastSpell(pet, sacrifice, false);
                        break;
                    }
        if (bot->GetPowerPct(POWER_MANA) < 10.0f && bot->GetHealthPct() > 60.0f && Cast(botAI, "life tap", bot))
            return true;
        if (Cast(botAI, "searing pain", target))
            return true;
    }
    // Vek'lor's tank waits outside melee, so Taunt alone only ties the puller's threat and he
    // drifts back once it fades. The shout reaches him from the waiting spot and builds a margin.
    if (control->Encounter() == Aq40Encounter::Twins && target->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) &&
        control->TankFor(target) == bot && bot->GetDistance(target) <= 10.0f)
    {
        std::string const shout = bot->getClass() == CLASS_WARRIOR ? "demoralizing shout"
                                  : bot->getClass() == CLASS_DRUID ? "demoralizing roar"
                                                                   : "";
        if (!shout.empty() && botAI->CanCastSpell(shout, bot) && botAI->CastSpell(shout, bot))
            return true;
    }
    // Weakened burn: melee hit C'Thun only from within 10 yd of his center; until then they walk in.
    if (control->Encounter() == Aq40Encounter::Cthun && control->CthunMeleeBurn(bot) &&
        bot->GetExactDist2d(target) > CthunBurnMeleeReach)
    {
        bool attacking = bot->GetVictim() != nullptr;
        StopAttacking(bot);
        return attacking;
    }
    if (control->Encounter() == Aq40Encounter::Viscidus && target->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)) &&
        bot->IsWithinMeleeRange(target))
    {
        bool starting = bot->GetVictim() != target || !bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING);
        Attack(target);
        if (starting)
            return bot->Attack(target, true);
        return false;
    }
    return Attack(target);
}

Position Aq40ControlAction::GlareDodge(Player* player, Unit* eye, float bearing, float distance)
{
    auto now = std::chrono::steady_clock::now();
    auto cached = _glareDodges.find(player->GetGUID());
    if (cached != _glareDodges.end() && now - cached->second.first < GlareDodgeRefresh)
        return cached->second.second;
    // Dodging at the bot's own distance carried outer-ring bots past the floor's edge, where they stayed stuck
    // for the rest of the fight. Step in until the dodge spot, a margin beyond it and the way there all have floor;
    // the escape bearing is recomputed for each radius, since the beam's clearance angle grows closer in.
    Position goal;
    float radius = std::max(GlareMinRadius, distance);
    while (true)
    {
        float escape = Aq40Rules::GlareEscapeBearing(bearing, radius, eye->GetOrientation(), _glareDirection,
                                                     GlareHalfWidth, GlareLookahead);
        goal = Around(eye, radius, escape);
        Position beyond = Around(eye, radius + GlareFloorMargin, escape);
        if (radius <= GlareMinRadius || (OnCthunFloor(player, beyond.GetPositionX(), beyond.GetPositionY()) &&
                                         OnCthunFloor(player, (player->GetPositionX() + goal.GetPositionX()) / 2.0f,
                                                      (player->GetPositionY() + goal.GetPositionY()) / 2.0f)))
            break;
        radius = std::max(GlareMinRadius, radius - GlareStepIn);
    }
    goal.m_positionZ = player->GetPositionZ();
    _glareDodges[player->GetGUID()] = {now, goal};
    return goal;
}

bool Aq40ControlAction::Danger(Player* player, Position& goal)
{
    if (_encounter == Aq40Encounter::Cthun && InStomach(player))
    {
        if (StomachExitNeeded(player))
            if (AreaTrigger const* exit = sObjectMgr->GetAreaTrigger(4033))
            {
                goal.Relocate(exit->x, exit->y, exit->z);
                return true;
            }
        return false;
    }
    if (_encounter == Aq40Encounter::Viscidus)
    {
        if (ToxinEscape(player, goal))
            return true;
        // A healer left on the entrance ramp or out of range heals nobody. Walk it in ahead of healing,
        // because the positioning action ranks below every heal and never gets a turn otherwise.
        if (PlayerbotAI::IsHeal(player))
            if (Creature* boss = Boss())
            {
                float ring = boss->GetCombatReach() + ViscidusRingOffset;
                float distance = player->GetDistance2d(boss);
                if (distance > ring + ViscidusStrandedOffset ||
                    (player->GetPositionZ() > ViscidusRampZ && distance > ring + ViscidusRingTolerance))
                {
                    goal = ViscidusRing(player, boss);
                    return true;
                }
            }
    }
    for (Creature* unit : Units())
    {
        bool hazard = (Whirling(unit) && player->GetDistance2d(unit) < 18.0f) ||
                      (unit->GetEntry() == Id(Aq40Npcs::NPC_POISON_CLOUD) && player->GetDistance2d(unit) < 12.0f) ||
                      (unit->GetEntry() == Id(Aq40Npcs::NPC_DIRT_MOUND) && player->GetDistance2d(unit) < 12.0f) ||
                      (unit->HasAura(Id(Aq40Spells::SPELL_EXPLODE_BUG)) && player->GetDistance2d(unit) < 15.0f);
        if (hazard)
        {
            goal = Around(player, 5.0f, unit->GetAngle(player));
            return true;
        }
        if (_encounter == Aq40Encounter::Cthun && unit->GetEntry() == Id(Aq40Npcs::NPC_EYE_OF_CTHUN) &&
            unit->HasAura(Id(Aq40Spells::SPELL_RED_COLORATION)))
        {
            // Only the real band moves anyone: a fixed angle would flag ~26 yd either side at 40 yd.
            float bearing = unit->GetAngle(player);
            float distance = unit->GetExactDist2d(player);
            if (Aq40Rules::InGlarePath(bearing, distance, unit->GetOrientation(), _glareDirection, GlareHalfWidth,
                                       GlareLookahead))
            {
                goal = GlareDodge(player, unit, bearing, distance);
                return true;
            }
        }
        if (_encounter == Aq40Encounter::Ouro && unit->GetEntry() == Id(Aq40Npcs::NPC_OURO) &&
            unit->GetVictim() != player && player->GetDistance2d(unit) < 40.0f &&
            std::abs(AngleDelta(unit->GetAngle(player), unit->GetOrientation())) < PI / 3.0f)
        {
            float angle = unit->GetAngle(player);
            float side = AngleDelta(angle, unit->GetOrientation()) >= 0.0f ? 1.0f : -1.0f;
            goal = Around(unit, std::max(8.0f, unit->GetExactDist2d(player)), angle + side * 0.3f);
            return true;
        }
        // After a swap Vek'nilash is passive for 2 s, then gives the nearest player 2000 threat; he can't be taunted,
        // so damage dealers clear the landing spot until he picks. Healers already hold outside the bystander ring.
        if (_encounter == Aq40Encounter::Twins && unit->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) &&
            unit->IsInCombat() && unit->HasReactState(REACT_PASSIVE) && TankFor(unit) != player &&
            !IsTankRole(player) && !PlayerbotAI::IsHeal(player) &&
            player->GetExactDist2d(unit) < TwinBystanderDistance + 6.0f)
        {
            goal = Around(unit, TwinBystanderDistance + 10.0f, unit->GetAngle(player));
            player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            return true;
        }
        // Arcane Burst fires on anyone in Vek'lor's melee range: ~4.5k damage, a ~33-yard knockback
        // and a 70% slow. His own tank steps just outside it and stays the nearest player to him.
        bool plate = TankFor(unit) == player || (IsTankRole(player) && !IsTwinSpareTank(player));
        if (_encounter == Aq40Encounter::Twins && unit->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) && plate &&
            player->GetDistance2d(unit) < TwinBurstClearance - 1.0f)
        {
            // Swap ownership can lag the teleport by one refresh; a plate tank still only steps out.
            goal = TwinWaitSpot(player, unit);
            return true;
        }
        // After a teleport he lands on the melee stack with a 2.3-second grace, so the escape goal
        // is deep enough that one forced move clears the radius.
        if (_encounter == Aq40Encounter::Twins && unit->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) && !plate &&
            player->GetExactDist2d(unit) < TwinBystanderDistance - 2.0f)
        {
            float escape = unit->GetAngle(player);
            if (_sidesReady)
            {
                uint32 side = unit->GetExactDist2d(_twinSides[0]) < unit->GetExactDist2d(_twinSides[1]) ? 0 : 1;
                float inward = _twinSides[side].GetAngle(&_twinSides[1 - side]);
                float offset = AngleDelta(escape, inward);
                // A bot knocked behind Vek'lor must leave around his side. Retreating straight
                // outward makes its cross-room route pass through him and repeat the escape forever.
                escape = std::cos(offset) < 0.0f ? inward + (offset >= 0.0f ? PI / 2.0f : -PI / 2.0f) : inward;
            }
            // Fan the escapes out: everyone leaving along one bearing ends in a stack that a single Blizzard can kill.
            escape += 0.3f * (float(player->GetGUID().GetCounter() % 7) - 3.0f);
            goal = Around(unit, TwinBystanderDistance + 2.0f, escape);
            return true;
        }
    }
    if (_encounter == Aq40Encounter::Twins)
        for (Position const& blizzard :
             EncounterHelpers::GetDynamicObjectPositions(player, 30.0f, Id(Aq40Spells::SPELL_BLIZZARD)))
            if (player->GetExactDist2d(blizzard) < 12.0f)
            {
                // Blizzard ticks ~1.4k every 2 s in a 10-yard circle. Leave it in one move instead of
                // 5-yard steps that keep a clustered caster group inside for another tick.
                Position center(blizzard.GetPositionX(), blizzard.GetPositionY(), player->GetPositionZ());
                float away =
                    player->GetExactDist2d(blizzard) > 0.5f ? blizzard.GetAngle(player) : player->GetOrientation();
                goal.Relocate(center.GetPositionX() + 15.0f * std::cos(away),
                              center.GetPositionY() + 15.0f * std::sin(away), player->GetPositionZ());
                player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
                return true;
            }
    return false;
}

Position Aq40ControlAction::TwinWaitSpot(Player* player, Unit* veklor)
{
    float wait = veklor->GetCombatReach() + player->GetCombatReach() + TwinBurstClearance;
    float base = veklor->GetAngle(player);
    auto spot = [&](float angle)
    {
        Position goal = Around(veklor, wait, angle);
        player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
        return goal;
    };
    // On the ring already: hold the current bearing, no path search needed.
    if (std::abs(player->GetExactDist2d(veklor) - wait) <= 1.0f)
        return spot(base);
    auto now = std::chrono::steady_clock::now();
    auto cached = _waitAngles.find(player->GetGUID());
    if (cached != _waitAngles.end() && now - cached->second.second < std::chrono::seconds(1))
        return spot(base + cached->second.first);
    float chosen = 0.0f;
    for (float offset : {0.0f, 0.6f, -0.6f, 1.2f, -1.2f, 1.9f, -1.9f, 2.6f, -2.6f, PI})
    {
        Position goal = spot(base + offset);
        PathGenerator route(player);
        if (route.CalculatePath(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ()) &&
            route.GetPathType() == PATHFIND_NORMAL && !route.GetPath().empty())
        {
            auto const& end = route.GetPath().back();
            if (goal.GetExactDist(end.x, end.y, end.z) <= 3.0f)
            {
                chosen = offset;
                break;
            }
        }
    }
    _waitAngles[player->GetGUID()] = {chosen, now};
    return spot(base + chosen);
}

bool Aq40ControlAction::Formation(Player* player, Position& goal)
{
    Creature* boss = Boss();
    if (!boss || (_encounter == Aq40Encounter::Cthun && InStomach(player)))
        return false;
    if (_encounter == Aq40Encounter::Skeram)
    {
        if (!SkeramPlatform(player, goal))
            return false;
        constexpr float platformHeightTolerance = 3.0f;
        Position const tankPlatform = goal;
        uint32 side = _raidSides.at(player->GetGUID());
        bool healer = PlayerbotAI::IsHeal(player);
        float radius = IsTankRole(player) ? 4.0f : 8.0f;
        // In-game healer positions cover the center and their raised platform without standing
        // on the boss's blink destination. Tank ownership and pickup bounds keep using that destination.
        constexpr uint32 lowerYPlatform = 0;   // Blink 4801.
        constexpr uint32 higherYPlatform = 2;  // Blink 20449.
        if (healer && (side == lowerYPlatform || side == higherYPlatform))
        {
            if (side == lowerYPlatform)
                goal.Relocate(-8335.298f, 2058.8733f, 133.08563f);
            else
                goal.Relocate(-8352.727f, 2108.1216f, 133.09175f);
            radius = 3.0f;
        }
        if (player->GetExactDist2d(goal) > radius ||
            std::abs(player->GetPositionZ() - goal.GetPositionZ()) > platformHeightTolerance)
            return true;
        if (healer)
        {
            // Healing target selection excludes units outside LOS. Recover sight of our platform's
            // tank independently, so a null healing target cannot strand the healer behind geometry.
            Player* tank = Member(_platformTanks[side]);
            constexpr float tankPlatformRadius = 12.0f;
            if (tank && tank->GetExactDist2d(tankPlatform) <= tankPlatformRadius &&
                std::abs(tank->GetPositionZ() - tankPlatform.GetPositionZ()) <= platformHeightTolerance &&
                !player->IsWithinLOSInMap(tank))
            {
                goal = tank->GetPosition();
                return true;
            }
        }
        return false;
    }
    if (_encounter == Aq40Encounter::Twins && _sidesReady)
    {
        for (uint32 side = 0; side < 2; ++side)
        {
            if (player->GetGUID() != _twinTanks[side])
                continue;
            // Hold a world-space anchor, not an offset from a boss that follows us. Chasing that
            // moving offset lets knockbacks and teleports ratchet both emperors across the room.
            constexpr float tankOffset = 4.0f;
            float tankTolerance = 2.0f;
            // Vek'nilash follows the tank he is hitting while Vek'lor, held by taunt threat, stays put, so Vek'nilash's
            // tank does the separating: it takes him to whichever platform is clearly farther from Vek'lor.
            uint32 home = side;
            for (Creature* nilash : Units(Id(Aq40Npcs::NPC_VEKNILASH)))
                if (TankFor(nilash) == player)
                    for (Creature* lor : Units(Id(Aq40Npcs::NPC_VEKLOR)))
                    {
                        float mine = lor->GetExactDist2d(_twinSides[side]);
                        float other = lor->GetExactDist2d(_twinSides[1 - side]);
                        if (mine + 20.0f < other)
                            home = 1 - side;
                    }
            float away = _twinSides[1 - home].GetAngle(&_twinSides[home]);
            goal = _twinSides[home];
            goal.m_positionX += tankOffset * std::cos(away);
            goal.m_positionY += tankOffset * std::sin(away);
            for (Creature* incoming : Units())
            {
                bool waiter =
                    incoming->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR) && _twinLock && _twinWaiter == player->GetGUID();
                if (TankFor(incoming) != player && !waiter)
                    continue;
                if (incoming->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
                {
                    // Vek'nilash can't be taunted and after a swap attacks whoever is nearest Vek'lor's old spot, so
                    // wait just outside Arcane Burst. Vek'lor casts at victims within 45 yd; Taunt reclaims him.
                    goal = TwinWaitSpot(player, incoming);
                    tankTolerance = 1.0f;  // A narrow band between Arcane Burst and Demoralizing Shout range.
                    break;
                }
                if (incoming->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) && incoming->GetVictim() != player)
                {
                    // Reach Vek'nilash at his actual position. A fixed pickup radius can strand the
                    // tank outside attack range; once acquired, the anchor above brings him home.
                    goal = Around(incoming, tankOffset, away);
                    player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
                    break;
                }
            }
            return player->GetExactDist2d(goal) > tankTolerance ||
                   std::abs(player->GetPositionZ() - goal.GetPositionZ()) > 3.0f;
        }
        if (IsTwinLock(player))
        {
            uint32 mySide = _twinLocks[0] == player->GetGUID() ? 0 : 1;
            Position spot = TwinLockSpot(mySide);
            for (Creature* veklor : Units(Id(Aq40Npcs::NPC_VEKLOR)))
                if (TankFor(veklor) == player)
                {
                    float distance = player->GetExactDist2d(veklor);
                    // Cast from anywhere inside Searing Pain range and outside the bystander ring. Vek'lor
                    // often walks after the pull; chasing an exact spot would keep the warlock from casting.
                    if (distance >= TwinBystanderDistance - 2.0f && distance <= TwinLockMaxDistance &&
                        player->IsWithinLOSInMap(veklor))
                        return false;
                    goal = Around(veklor, TwinLockDistance, veklor->GetAngle(&spot));
                    player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
                    return true;
                }
            // Not our phase: wait at our spot beside our side's healers, clear of both landing spots.
            goal = spot;
            player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            return player->GetExactDist2d(goal) > 4.0f;
        }
        if (!PlayerbotAI::IsHeal(player) && (!IsTankRole(player) || IsTwinSpareTank(player)))
        {
            bool physical = PlayerbotAI::IsMelee(player) || player->getClass() == CLASS_HUNTER;
            auto emperors = Units(physical ? Id(Aq40Npcs::NPC_VEKNILASH) : Id(Aq40Npcs::NPC_VEKLOR));
            if (emperors.empty())
                return false;
            Creature* emperor = emperors.front();
            uint32 side = emperor->GetExactDist2d(_twinSides[0]) < emperor->GetExactDist2d(_twinSides[1]) ? 0 : 1;
            // Travel follows the emperor even while damage waits for a tank pickup. Otherwise
            // Held() and the old-platform fallback prevent a bot from ever acquiring a distant target.
            bool melee = PlayerbotAI::IsMelee(player);
            // Before the teleport window opens, melee leave Vek'nilash for the middle of the path between
            // the spots: Vek'lor lands where Vek'nilash stood and Arcane Bursts everyone in melee range.
            if (melee && TwinSwapWindow())
            {
                goal.Relocate((_twinSides[0].GetPositionX() + _twinSides[1].GetPositionX()) / 2.0f,
                              (_twinSides[0].GetPositionY() + _twinSides[1].GetPositionY()) / 2.0f,
                              _twinSides[0].GetPositionZ());
                player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
                return player->GetExactDist2d(goal) > 6.0f;
            }
            constexpr float rangedDistance = TwinBystanderDistance + 6.0f;
            constexpr float pickupDistance = 20.0f;
            constexpr float meleeDistance = 3.0f;
            float distance = melee ? (Held(emperor) ? meleeDistance : pickupDistance) : rangedDistance;
            float inward = _twinSides[side].GetAngle(&_twinSides[1 - side]);
            if (!melee)
            {
                uint32 slot = 0;
                uint32 count = 0;
                for (Player* member : Members())
                    if (!IsTankRole(member) && !PlayerbotAI::IsHeal(member) && !PlayerbotAI::IsMelee(member) &&
                        (member->getClass() == CLASS_HUNTER) == physical)
                    {
                        if (member->GetGUID() < player->GetGUID())
                            ++slot;
                        ++count;
                    }
                // Spread ranged over a wide arc and two depths: a 10-yard Blizzard on a tight arc can kill six casters.
                inward += 2.4f * ((float(slot) + 0.5f) / float(std::max(count, 1u)) - 0.5f);
                if (slot % 2)
                    distance += 5.0f;
            }
            goal = Around(emperor, distance, inward);
            player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
            return player->GetExactDist2d(goal) > (melee ? 3.0f : 5.0f) ||
                   std::abs(player->GetPositionZ() - goal.GetPositionZ()) > 3.0f;
        }
        // Only healers consume healing slots. Counting the whole raid pushes the later
        // healers beyond healing range, especially after their tank is knocked back.
        uint32 side = _raidSides[player->GetGUID()];
        Player* tank = Member(_twinTanks[side]);
        // Vek'lor's Shadow Bolts land on his warlock, not on the waiting warrior. The healers of the
        // side Vek'lor is on look after the warlock while he holds him.
        if (Player* lock = Member(_twinLock))
            for (Creature* veklor : Units(Id(Aq40Npcs::NPC_VEKLOR)))
                if (TankFor(veklor) == lock && tank &&
                    veklor->GetExactDist2d(_twinSides[side]) < veklor->GetExactDist2d(_twinSides[1 - side]))
                    tank = lock;
        auto clearOfVeklor = [&](Position const& spot)
        {
            for (Creature* veklor : Units(Id(Aq40Npcs::NPC_VEKLOR)))
                if (veklor->GetExactDist2d(spot) < TwinBystanderDistance)
                    return false;
            return true;
        };
        if (tank && player->GetExactDist(tank) >= 18.0f && player->GetExactDist(tank) <= 30.0f &&
            player->IsWithinLOSInMap(tank) && clearOfVeklor(player->GetPosition()))
            return false;  // Keep healing instead of repositioning for every small tank movement.
        float axis = _twinSides[side].GetAngle(&_twinSides[1 - side]);
        uint32 slot = 0;
        uint32 count = 0;
        for (Player* member : Members())
            if (PlayerbotAI::IsHeal(member) && !IsTankRole(member) && _raidSides[member->GetGUID()] == side)
            {
                if (member->GetGUID() < player->GetGUID())
                    ++slot;
                ++count;
            }
        // Follow the assigned tank on this platform. Keep room for a knockback within
        // normal healing range, and stay outside Vek'lor's Arcane Burst radius.
        float spread = (float(slot) + 0.5f) / float(std::max(count, 1u)) - 0.5f;
        Position anchor = tank ? tank->GetPosition() : _twinSides[side];
        // Healers stand between their tank and their side's warlock spot, toward the entrance.
        Position lockSpot = TwinLockSpot(side);
        float angle =
            (anchor.GetExactDist2d(&lockSpot) > 3.0f ? anchor.GetAngle(&lockSpot) : axis + PI / 2.0f) + spread;
        // Healers stay farther from Vek'lor than his waiting tank, or Vek'nilash lands on them.
        bool found = false;
        Position fallback;
        for (float turn : {0.0f, 0.8f, -0.8f, 1.6f, -1.6f, 2.4f, -2.4f})
        {
            for (float radius : {24.0f, 20.0f, 16.0f})
            {
                goal.Relocate(anchor.GetPositionX() + radius * std::cos(angle + turn),
                              anchor.GetPositionY() + radius * std::sin(angle + turn), anchor.GetPositionZ());
                player->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
                if (turn == 0.0f && radius == 24.0f)
                    fallback = goal;
                if ((!tank || tank->IsWithinLOS(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ())) &&
                    clearOfVeklor(goal))
                {
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!found)
            goal = fallback;
        return player->GetExactDist2d(goal) > 5.0f || std::abs(player->GetPositionZ() - goal.GetPositionZ()) > 3.0f;
    }

    if (_encounter == Aq40Encounter::Cthun)
    {
        if (_glareActive || !CthunSpot(player, goal))
            return false;
        // The first three Eye Beams (3, 6 and 9 s) all go to the puller. He stays where he entered, alone, until the
        // raid has spread, or the opening beams chain through the raid still coming down the slope.
        if (player->GetGUID() == _cthunPuller &&
            std::chrono::steady_clock::now() - _encounterStart < std::chrono::seconds(CthunPullerHoldSeconds))
        {
            // A Moon puller the Eye saw on his way in goes on to the pull spot while the opening beams land on him.
            if (Group* group = player->GetGroup(); group && group->GetTargetIcon(CthunPullerIcon) == player->GetGUID())
            {
                goal = CthunPullSpot;
                return player->GetExactDist2d(goal) > 3.0f;
            }
            return false;
        }
        // Melee may step off their spot to a tentacle next to it (CthunTarget's leash); they come back after.
        if (IsTankRole(player) || PlayerbotAI::IsMelee(player))
        {
            if (Unit* target = Target(player))
            {
                // Weakened burn: stand 8 yd from C'Thun's center on the bot's side of him.
                if (target->GetEntry() == Id(Aq40Npcs::NPC_CTHUN) && CthunPhase2())
                {
                    goal = Around(target, CthunBurnMeleeRadius, target->GetAngle(player));
                    return player->GetExactDist2d(target) > CthunBurnMeleeRadius + 1.0f;
                }
                if (target->GetEntry() != Id(Aq40Npcs::NPC_EYE_OF_CTHUN) &&
                    target->GetEntry() != Id(Aq40Npcs::NPC_CTHUN))
                    return false;
            }
        }
        else if (CthunPhase2())
            if (Unit* target = Target(player))
            {
                if (player->GetDistance(target) > CthunRangedReach)
                    return false;  // Walk into range of a far Eye Tentacle, then come back to the stack.
                // A tentacle killer stays where it can reach its far tentacle until it dies, instead of walking
                // back to the stack and out of range again.
                if (target->GetEntry() == Id(Aq40Npcs::NPC_EYE_TENTACLE) &&
                    target->GetExactDist2d(&CthunStack) > CthunFarTentacle && CthunTentacleKiller(player))
                    return false;
            }
        return player->GetExactDist2d(goal) > 3.0f;
    }
    if (PlayerbotAI::IsTank(player))
    {
        Unit* target = Target(player);
        // Keep the bosses separated without pulling them away from their own healers.
        if (_encounter == Aq40Encounter::Trio && target && TankFor(target) == player && target->GetVictim() == player)
        {
            auto [it, inserted] = _angles.emplace(player->GetGUID(), target->GetAngle(player));
            goal = Around(target, 8.0f, it->second);
            for (Creature* other : Units())
                if (other != target && IsEncounterBoss(other->GetEntry()) && other->GetDistance(target) < 18.0f)
                {
                    goal = Around(player, 4.0f, other->GetAngle(player));
                    return true;
                }
        }
        return false;
    }
    if (_encounter == Aq40Encounter::Viscidus && boss->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)) &&
        !PlayerbotAI::IsHeal(player))
    {
        goal = boss->GetPosition();
        return !player->IsWithinMeleeRange(boss);
    }
    if (_encounter == Aq40Encounter::Huhuran)
    {
        bool soak = std::find(_soakers.begin(), _soakers.end(), player->GetGUID()) != _soakers.end();
        if (soak)
        {
            float desired = PlayerbotAI::IsMelee(player) ? 1.0f : 8.0f;
            goal = Around(boss, desired + boss->GetCombatReach(), boss->GetAngle(player));
            return std::abs(player->GetExactDist2d(boss) - desired - boss->GetCombatReach()) > 4.0f;
        }
        // Noxious Poison silences a 15-yd circle around a random raider. Everyone outside the soak group gets a slot
        // ~15 yd apart on two staggered rings: healers on the tank's side, ranged damage on the far side.
        if (PlayerbotAI::IsMelee(player))
            return false;  // Melee stay on the boss whether or not the soak list exists.
        bool healer = PlayerbotAI::IsHeal(player);
        std::vector<Player*> peers;
        for (Player* member : Members())
            if (PlayerbotAI::IsHeal(member) == healer && !IsTankRole(member) && !PlayerbotAI::IsMelee(member) &&
                std::find(_soakers.begin(), _soakers.end(), member->GetGUID()) == _soakers.end())
                peers.push_back(member);
        auto self = std::find(peers.begin(), peers.end(), player);
        if (self == peers.end())
            return false;
        uint32 index = uint32(self - peers.begin());
        uint32 ring = index % 2;
        uint32 slot = index / 2;
        uint32 slots = (uint32(peers.size()) + 1 - ring) / 2;
        Player* tank = TankFor(boss);
        Unit* anchor = tank ? tank : boss->GetVictim();
        float center = anchor ? boss->GetAngle(anchor) : boss->GetOrientation();
        float span = healer ? 1.4f : 1.7f;
        if (!healer)
            center += PI;
        float angle = center - span + span * 2.0f * (float(slot) + 0.5f) / float(std::max(slots, 1u));
        float radius = boss->GetCombatReach() + (ring == 0 ? 20.0f : 27.0f);
        goal = Around(boss, radius, angle);
        return player->GetExactDist2d(goal) > 5.0f;
    }
    if (!PlayerbotAI::IsHeal(player))
        if (Unit* target = Target(player))
            if (target != boss)
                return false;
    if (_encounter == Aq40Encounter::Viscidus && (PlayerbotAI::IsHeal(player) || PlayerbotAI::IsRanged(player)))
    {
        // The generic ring measures from the boss's center but aims at 28 yards plus his 10-yard reach,
        // so nobody ever settles. Measure and aim with the same reach, and keep the slot off the ramp.
        float ring = boss->GetCombatReach() + ViscidusRingOffset;
        if (player->GetPositionZ() <= ViscidusRampZ &&
            std::abs(player->GetDistance2d(boss) - ring) <= ViscidusRingTolerance)
            return false;
        goal = ViscidusRing(player, boss);
        return true;
    }
    if (PlayerbotAI::IsHeal(player) || PlayerbotAI::IsRanged(player))
    {
        float distance = player->GetDistance2d(boss);
        if (distance < 22.0f || distance > 36.0f)
        {
            goal = Around(boss, 28.0f + boss->GetCombatReach(), boss->GetAngle(player));
            return true;
        }
    }
    return false;
}

bool Aq40SkeramInterruptAction::Execute(Event /*event*/)
{
    Aq40ControlAction* control = Aq40ControlAction::Get(botAI);
    // Also C'Thun: this action runs while the bot is casting, which the control action does not.
    return (control->Encounter() == Aq40Encounter::Skeram || control->Encounter() == Aq40Encounter::Cthun) &&
           !bot->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)) && control->Interrupt(botAI);
}

bool Aq40Trigger::IsActive()
{
    Aq40ControlAction* control = Aq40ControlAction::Get(botAI);
    control->Refresh(botAI);
    return control->Encounter() != Aq40Encounter::None;
}

bool Aq40TwinPrepullSpot(PlayerbotAI* botAI, Position& spot)
{
    Player* bot = botAI->GetBot();
    InstanceScript* instance = bot->GetInstanceScript();
    Group* group = bot->GetGroup();
    if (bot->GetMapId() != 531 || !instance || !group || bot->IsInCombat() || !bot->IsAlive() ||
        bot->getClass() != CLASS_WARLOCK || PlayerbotAI::IsTank(bot))
        return false;
    EncounterState state = instance->GetBossState(Id(Aq40Encounter::Twins));
    if (state == IN_PROGRESS || state == DONE)
        return false;
    // Only inside the Twin Emperors room (and its entrance), never elsewhere in the temple.
    constexpr float roomX = -8962.0f, roomY = 1235.0f;
    if (bot->GetExactDist2d(roomX, roomY) > 125.0f || bot->GetPositionZ() < -118.0f || bot->GetPositionZ() > -90.0f)
        return false;
    if (group->GetTargetIcon(TwinLockIcon) == bot->GetGUID())
        // 31 yd (32 yd in 3D) from Vek'lor on his pedestal: outside his ~29-yd aggro reach (detection 20
        // + 3 for the level gap + both combat reaches), still inside Searing Pain range edge-to-edge.
        spot.Relocate(-8889.6f, 1228.5f, -112.3f);
    else if (group->GetTargetIcon(TwinSecondLockIcon) == bot->GetGUID())
        spot = TwinLockSpots[0];  // south warlock's waiting spot
    else
        return false;
    bot->UpdateAllowedPositionZ(spot.GetPositionX(), spot.GetPositionY(), spot.m_positionZ);
    return true;
}

// Out of combat, the tank marked with Moon walks alone from the entrance ramps to C'Thun's pull spot; the Eye pulls
// when it sees him. Only with the Twin Emperors dead (the Eye evades until then) and near the room.
// The rest of the raid follows him down the slope, CthunTrailDistance behind, so it is that much closer to its spots
// when the random Eye Beams start at about 17 s. Any closer and it reaches him during the three opening beams.
bool Aq40CthunPrepullSpot(PlayerbotAI* botAI, Position& spot)
{
    Player* bot = botAI->GetBot();
    InstanceScript* instance = bot->GetInstanceScript();
    Group* group = bot->GetGroup();
    if (bot->GetMapId() != 531 || !instance || !group || bot->IsInCombat() || !bot->IsAlive())
        return false;
    EncounterState state = instance->GetBossState(Id(Aq40Encounter::Cthun));
    if (instance->GetBossState(Id(Aq40Encounter::Twins)) != DONE || state == IN_PROGRESS || state == DONE)
        return false;
    auto nearRoom = [](Player* player)
    { return player->GetExactDist2d(&CthunCenter) <= CthunPrepullReach && player->GetPositionZ() >= CthunPrepullMinZ; };
    if (!nearRoom(bot))
        return false;
    ObjectGuid pullerGuid = group->GetTargetIcon(CthunPullerIcon);
    if (pullerGuid == bot->GetGUID())
    {
        if (!PlayerbotAI::IsTank(bot))
            return false;
        spot = CthunPullSpot;
        return true;
    }
    // Only while he is walking in, so an idle Moon tank never holds the raid in place.
    Player* puller = ObjectAccessor::GetPlayer(*bot, pullerGuid);
    if (!puller || puller->GetGroup() != group || !puller->IsAlive() || puller->IsInCombat() || !puller->isMoving() ||
        !nearRoom(puller) || !PlayerbotAI::IsTank(puller))
        return false;
    float distance = bot->GetExactDist2d(puller);
    float pullerRange = puller->GetExactDist2d(&CthunCenter);
    spot = bot->GetPosition();
    // Never walk to where the Eye could see a trailing bot before him.
    if (distance <= CthunTrailDistance || bot->GetExactDist2d(&CthunCenter) < pullerRange + CthunTrailMargin)
        return true;
    float angle = puller->GetAngle(bot);
    float share = CthunTrailDistance / distance;
    spot.Relocate(puller->GetPositionX() + CthunTrailDistance * std::cos(angle),
                  puller->GetPositionY() + CthunTrailDistance * std::sin(angle),
                  puller->GetPositionZ() + (bot->GetPositionZ() - puller->GetPositionZ()) * share);
    if (spot.GetExactDist2d(&CthunCenter) < pullerRange + CthunTrailMargin)
        spot = bot->GetPosition();
    return true;
}

bool Aq40TwinPrepullTrigger::IsActive()
{
    Position spot;
    return Aq40TwinPrepullSpot(botAI, spot) || Aq40CthunPrepullSpot(botAI, spot);
}

bool Aq40TwinPrepullAction::Execute(Event /*event*/)
{
    Position spot;
    if (!Aq40TwinPrepullSpot(botAI, spot) && !Aq40CthunPrepullSpot(botAI, spot))
        return false;
    if (bot->GetExactDist2d(spot) > 2.0f)
    {
        if (!bot->isMoving() || bot->movespline->Finalized())
            bot->GetMotionMaster()->MovePoint(0, spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
        return true;
    }
    // Hold the spot: returning true keeps "follow" from walking him back to the master.
    if (bot->isMoving())
        Stop(botAI);
    return true;
}

bool Aq40MoveAction::Execute(Event /*event*/)
{
    Aq40ControlAction* control = Aq40ControlAction::Get(botAI);
    if (control->Encounter() == Aq40Encounter::None || bot->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)))
        return false;
    // Server bots have no client to resume gravity after a knockback; finish the fall before pathfinding (CanMove
    // excludes controlled motion). C'Thun knocks back often and the stomach exit drops players 10 yd up. Not in the
    // stomach, where MoveFall sinks bots below the exit.
    if (_safety &&
        (control->Encounter() == Aq40Encounter::Twins ||
         (control->Encounter() == Aq40Encounter::Cthun && !InStomach(bot))) &&
        !IsSelfBot(bot) && botAI->CanMove() && !bot->IsFlying() && !bot->HasAuraType(SPELL_AURA_HOVER))
    {
        float floor = bot->GetMapHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        // C'Thun: only a short drop (a knockback, the 10-yd stomach drop onto C'Thun). On the entrance slope the height
        // lookup finds ground far below the walkway and MoveFall drops bots through the world.
        float drop = bot->GetPositionZ() - floor;
        if (floor > INVALID_HEIGHT && drop > 3.0f && (control->Encounter() == Aq40Encounter::Twins || drop < 15.0f))
        {
            bot->GetMotionMaster()->MoveFall();
            return true;
        }
    }
    // A player on C'Thun's center is spat out by area trigger 4036, which only a client fires. Send it
    // for the bot, as the stomach exit already does.
    if (_safety && control->Encounter() == Aq40Encounter::Cthun && !IsSelfBot(bot) && !InStomach(bot) &&
        botAI->CanMove() && bot->GetExactDist2d(&CthunCenter) < CthunSpitOutRadius)
    {
        auto now = std::chrono::steady_clock::now();
        if (now - _spitAttempt > std::chrono::seconds(3))
        {
            _spitAttempt = now;
            WorldPacket packet(CMSG_AREATRIGGER);
            packet << uint32(CthunSpitOutTrigger);
            bot->GetSession()->HandleAreaTriggerOpcode(packet);
            return true;
        }
    }
    if (_safety && control->GiantEyeResponse(botAI))
        return true;
    // An emperor that can't path to his target takes no damage and regenerates a third of his health every tick
    // (Creature::RegenerateHealth). His target finishes any fall, then leads him back onto the nearest platform top.
    if (_safety && control->Encounter() == Aq40Encounter::Twins && botAI->CanMove())
        for (Creature* emperor : control->Units())
            if ((emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) ||
                 emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR)) &&
                emperor->GetVictim() == bot && emperor->CanNotReachTarget())
            {
                float floor = bot->GetMapHeight(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                if (floor > INVALID_HEIGHT && bot->GetPositionZ() - floor > 0.5f && !bot->IsFlying())
                {
                    bot->GetMotionMaster()->MoveFall();
                    return true;
                }
                // Standing next to him is not enough when his own path is the problem, so take him back
                // onto the nearest platform top.
                Position spot = control->TwinUnstuckSpot(emperor);
                bot->UpdateAllowedPositionZ(spot.GetPositionX(), spot.GetPositionY(), spot.m_positionZ);
                bot->GetMotionMaster()->MovePoint(0, spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
                return true;
            }
    Position goal;
    bool danger = control->Danger(bot, goal);
    if (_safety && !danger)
    {
        if (control->UrgentHeal(botAI) && bot->isMoving() && botAI->CanMove())
            Stop(botAI);  // Return false so the queued healing spell can run in this same engine update.
        return false;
    }
    if (!_safety)
    {
        bool tactical = control->Encounter() == Aq40Encounter::Skeram || control->Encounter() == Aq40Encounter::Twins ||
                        control->Encounter() == Aq40Encounter::Cthun ||
                        control->Encounter() == Aq40Encounter::Huhuran ||
                        (control->Encounter() == Aq40Encounter::Viscidus && control->Boss() &&
                         control->Boss()->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)));
        if (_tactical != tactical || danger || control->UrgentHeal(botAI) || bot->HasUnitState(UNIT_STATE_CASTING) ||
            !control->Formation(bot, goal))
            return false;
    }
    if (!danger && control->Encounter() == Aq40Encounter::Twins && control->IsTankRole(bot))
        if (Unit* target = control->Target(bot))
            if (target->GetVictim() != bot)
            {
                std::string taunt = Taunt(bot);
                if (!taunt.empty() && Cast(botAI, taunt, target))
                    return true;  // Pause travel only when the pickup casts; CanCastSpell accepts out of range.
            }
    // C'Thun: the entrance ramps reach the floor only by one slope south-southeast of the Eye, so a spot is often
    // 100+ yd of path away and short steps stall on the ramps.
    bool cthunRoute = control->Encounter() == Aq40Encounter::Cthun && !InStomach(bot);
    if ((!danger && control->Encounter() == Aq40Encounter::Skeram) || control->Encounter() == Aq40Encounter::Twins ||
        cthunRoute)
    {
        if (control->Encounter() == Aq40Encounter::Twins || cthunRoute)
            bot->UpdateAllowedPositionZ(goal.GetPositionX(), goal.GetPositionY(), goal.m_positionZ);
        else if (!control->IsTankRole(bot))
            StopAttacking(bot);
        PathGenerator route(bot);
        if (!route.CalculatePath(goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ()) ||
            route.GetPath().empty())
            return false;
        // A long C'Thun route may come back incomplete; walk its reachable part and path again from there.
        bool partial = cthunRoute && route.GetPathType() == PATHFIND_INCOMPLETE;
        if (route.GetPathType() != PATHFIND_NORMAL && !partial)
            return false;
        auto const& end = route.GetPath().back();
        if (!partial && goal.GetExactDist(end.x, end.y, end.z) > 3.0f)
            return false;
        // Twins transfers run continuously across the room: stopping every few yards adds an AI reaction delay per
        // segment and can miss the next teleport. The safety action can still interrupt this route for a hazard.
        float stepLength = control->Encounter() == Aq40Encounter::Twins || cthunRoute ? 250.0f : 4.0f;
        float remaining = stepLength;
        Position next = bot->GetPosition();
        for (auto const& point : route.GetPath())
        {
            float segment = next.GetExactDist(point.x, point.y, point.z);
            if (segment > remaining)
            {
                float fraction = remaining / segment;
                next.Relocate(next.GetPositionX() + (point.x - next.GetPositionX()) * fraction,
                              next.GetPositionY() + (point.y - next.GetPositionY()) * fraction,
                              next.GetPositionZ() + (point.z - next.GetPositionZ()) * fraction);
                break;
            }
            next.Relocate(point.x, point.y, point.z);
            remaining -= segment;
        }
        if (control->Encounter() == Aq40Encounter::Twins && (!control->IsTankRole(bot) || danger))
            StopAttacking(bot);
        if (danger)
            bot->CastStop();
        if ((control->Encounter() == Aq40Encounter::Twins || cthunRoute) && bot->movespline->Finalized())
            AI_VALUE(LastMovement&, "last movement").clear();  // A knockback may have cancelled the old route.
        return MoveTo(bot->GetMapId(), next.GetPositionX(), next.GetPositionY(), next.GetPositionZ(), false, false,
                      true, true, danger ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT, true);
    }
    float distance = bot->GetExactDist2d(goal);
    if (distance < 1.5f)
    {
        if (danger && control->Encounter() == Aq40Encounter::Cthun && InStomach(bot))
        {
            Stop(botAI);  // Stay on the exit while the server's delayed area-trigger checks run.
            auto now = std::chrono::steady_clock::now();
            if (now - _exitAttempt > std::chrono::seconds(4) &&
                std::abs(bot->GetPositionZ() - goal.GetPositionZ()) < 4.0f)
            {
                _exitAttempt = now;
                WorldPacket packet(CMSG_AREATRIGGER);
                packet << uint32(4033);
                bot->GetSession()->HandleAreaTriggerOpcode(packet);
            }
        }
        return false;
    }
    float angle = bot->GetAngle(&goal);
    float length = std::min(distance, danger ? 7.0f : 4.0f);
    // A hazard escape can be walled in (ramp edge, cloud cluster). Turn before giving up, and only
    // interrupt the bot's cast once a step is actually possible, so a stuck healer keeps healing.
    std::vector<float> turns =
        danger ? std::vector<float>{0.0f, 0.9f, -0.9f, 1.8f, -1.8f, PI} : std::vector<float>{0.0f};
    std::vector<Creature*> clouds = danger && control->Encounter() == Aq40Encounter::Viscidus
                                        ? control->Units(Id(Aq40Npcs::NPC_TOXIC_SLIME))
                                        : std::vector<Creature*>{};
    bool found = false;
    float bestX = 0.0f;
    float bestY = 0.0f;
    float bestZ = 0.0f;
    float bestClearance = -1.0f;
    for (float turn : turns)
    {
        float x = bot->GetPositionX() + length * std::cos(angle + turn);
        float y = bot->GetPositionY() + length * std::sin(angle + turn);
        float z = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;
        PathGenerator path(bot);
        if (!path.CalculatePath(x, y, z) || path.GetPathType() != PATHFIND_NORMAL)
            continue;
        float pathLength = 0.0f;
        Position previous = bot->GetPosition();
        for (auto const& point : path.GetPath())
        {
            Position next(point.x, point.y, point.z);
            pathLength += previous.GetExactDist(next);
            previous = next;
        }
        if (pathLength > (danger ? 12.0f : 8.0f))
            continue;
        float clearance = std::numeric_limits<float>::max();
        for (Creature* cloud : clouds)
            clearance = std::min(clearance, cloud->GetExactDist2d(x, y));
        if (found && clearance <= bestClearance)
            continue;
        found = true;
        bestX = x;
        bestY = y;
        bestZ = z;
        bestClearance = clearance;
        if (clouds.empty())
            break;
    }
    if (!found && danger && control->Encounter() == Aq40Encounter::Cthun && InStomach(bot))
    {
        // Players drop from the stomach entrance into the pool, but a bot hangs where it was teleported and no short
        // step validates from mid-air. Go straight to the exit.
        bot->CastStop();
        bot->GetMotionMaster()->MovePoint(0, goal.GetPositionX(), goal.GetPositionY(), goal.GetPositionZ(),
                                          FORCED_MOVEMENT_NONE, 0.0f, 0.0f, false, false);
        return true;
    }
    if (!found)
        return false;
    if (danger)
    {
        StopAttacking(bot);
        bot->CastStop();
    }
    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true,
                  danger ? MovementPriority::MOVEMENT_FORCED : MovementPriority::MOVEMENT_COMBAT);
}

float Aq40Multiplier::GetValue(Action* action)
{
    if (!action || bot->GetMapId() != 531)
        return 1.0f;
    Aq40ControlAction* control = Aq40ControlAction::Get(botAI);
    if (control->Encounter() == Aq40Encounter::None)
    {
        // Trash: an Anubisath Defender reflects two schools for its whole fight (Shadow+Frost or
        // Fire+Arcane, picked on aggro). Casting into the reflect is how bots kill themselves.
        Unit* target = action->GetTarget();
        CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
        if (target && cast && target->GetEntry() == Id(Aq40Npcs::NPC_ANUBISATH_DEFENDER) &&
            bot->IsValidAttackTarget(target))
            if (SpellInfo const* spell = SpellFor(botAI, cast->getSpell()))
                if (!spell->IsPositive())
                {
                    uint32 school = spell->GetSchoolMask();
                    if (target->HasAura(Id(Aq40Spells::SPELL_SHADOW_FROST_REFLECT)) &&
                        (school & (SPELL_SCHOOL_MASK_SHADOW | SPELL_SCHOOL_MASK_FROST)))
                        return 0.0f;
                    if (target->HasAura(Id(Aq40Spells::SPELL_FIRE_ARCANE_REFLECT)) &&
                        (school & (SPELL_SCHOOL_MASK_FIRE | SPELL_SCHOOL_MASK_ARCANE)))
                        return 0.0f;
                }
        return 1.0f;
    }
    // Attack/cast actions can stop a movement spline even when CanMove() is false.
    // Let server-driven knockbacks and falls finish before any action can replace them.
    if (control->Encounter() == Aq40Encounter::Twins && !IsSelfBot(bot) &&
        bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == EFFECT_MOTION_TYPE &&
        !bot->movespline->Finalized())
        return 0.0f;
    if (dynamic_cast<Aq40ControlAction*>(action) || dynamic_cast<Aq40MoveAction*>(action) ||
        dynamic_cast<Aq40SkeramInterruptAction*>(action))
        return 1.0f;
    // C'Thun: every bot's place comes from the strategy. A bot briefly out of combat would otherwise follow the raid
    // leader, possibly back up the entrance ramp during phase 1.
    if (control->Encounter() == Aq40Encounter::Cthun && action->getName() == "follow")
        return 0.0f;
    // A swallowed bot heading out does nothing else: each heal cast stops its walk to the exit and it dies in the pool.
    if (control->Encounter() == Aq40Encounter::Cthun && InStomach(bot) && control->StomachExitNeeded(bot))
        return 0.0f;
    Unit* target = action->GetTarget();
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    SpellInfo const* spell = cast ? SpellFor(botAI, cast->getSpell()) : nullptr;
    bool hostile = target && bot->IsValidAttackTarget(target);
    bool picker = dynamic_cast<AttackAction*>(action) || dynamic_cast<AttackRtiTargetAction*>(action) ||
                  dynamic_cast<AttackLeastHpTargetAction*>(action);
    bool tank = control->IsTankRole(bot);
    Position danger;
    if (control->Danger(bot, danger))
    {
        if (hostile || picker || dynamic_cast<MovementAction*>(action))
            return 0.0f;
    }
    if (control->Encounter() == Aq40Encounter::Twins && PlayerbotAI::IsHeal(bot) && spell && spell->IsPositive() &&
        control->TwinOpeningHold(target ? target : bot))
        return 0.0f;
    // Instant priest spells cast on anyone drain the priests' mana within about two minutes. In this fight Shield and
    // Renew are for the emperors' targets only.
    if (control->Encounter() == Aq40Encounter::Twins && bot->getClass() == CLASS_PRIEST && cast &&
        (cast->getSpell() == "power word: shield" || cast->getSpell() == "renew"))
    {
        Unit* who = target ? target : bot;
        bool emperorTarget = false;
        for (Creature* emperor : control->Units())
            if ((emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH) ||
                 emperor->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR)) &&
                (emperor->GetVictim() == who || control->TankFor(emperor) == who))
                emperorTarget = true;
        if (!emperorTarget || who->GetHealthPct() > 80.0f)
            return 0.0f;
    }
    if (control->Encounter() == Aq40Encounter::Cthun && InStomach(bot) && cast && cast->getSpell() == "tree of life")
        return 0.0f;  // Stays out of Tree of Life to damage the Flesh Tentacles.
    // Nobody on the floor is in range of a healer in the stomach; those casts only fail and cost its turns.
    if (control->Encounter() == Aq40Encounter::Cthun && InStomach(bot) && spell && spell->IsPositive() && target &&
        target != bot && target->GetPositionZ() > -40.0f)
        return 0.0f;
    if (control->UrgentHeal(botAI) && dynamic_cast<MovementAction*>(action))
        return 0.0f;
    if (control->Encounter() == Aq40Encounter::Cthun && InStomach(bot) && dynamic_cast<MovementAction*>(action) &&
        !picker && (!dynamic_cast<ReachTargetAction*>(action) || !target || target->GetPositionZ() > -40.0f))
        return 0.0f;
    if (target && target->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)) && hostile)
        return 0.0f;  // Only the explicit nonlethal crowd-control action may target raid members.
    if (control->Encounter() == Aq40Encounter::Huhuran && spell && target &&
        target->HasAura(Id(Aq40Spells::SPELL_WYVERN_STING)))
        for (SpellEffectInfo const& effect : spell->GetEffects())
            if (effect.Effect == SPELL_EFFECT_DISPEL)
                return 0.0f;  // Dedicated selective dispel handles tanks above the damage safety margin.
    if (EncounterHelpers::IsTauntAction(bot, action) ||
        (control->Encounter() == Aq40Encounter::Twins && spell && spell->HasEffect(SPELL_EFFECT_ATTACK_ME)))
    {
        if (action->getName() == "challenging shout" || action->getName() == "challenging roar" ||
            action->getName() == "righteous defense")
            return 0.0f;
        if (!target || control->TankFor(target) != bot || Whirling(target))
            return 0.0f;
    }
    if (tank && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;
    Unit* desired = control->Target(bot);
    if (picker && (!desired || target != desired))
        return 0.0f;
    if (action->getName() == "pet attack" && !desired)
        return 0.0f;
    if (action->getName() == "pet attack" && control->Encounter() == Aq40Encounter::Twins &&
        bot->getClass() == CLASS_WARLOCK && bot->GetPet())
        return 0.0f;
    if (control->Encounter() == Aq40Encounter::Twins && !tank && !PlayerbotAI::IsHeal(bot) && (hostile || picker))
    {
        Position formation;
        if (control->Formation(bot, formation))
            return 0.0f;
    }
    if (hostile && (!control->AllowedDamage(bot, target) || (!tank && target != desired)))
        return 0.0f;
    if (hostile && control->Encounter() == Aq40Encounter::Cthun && target->GetEntry() == Id(Aq40Npcs::NPC_CTHUN) &&
        control->CthunMeleeBurn(bot) && bot->GetExactDist2d(target) > CthunBurnMeleeReach)
        return 0.0f;  // Weakened burn: no melee strikes on C'Thun from beyond 10 yd.
    if (control->Encounter() == Aq40Encounter::Twins && hostile && spell && target->IsImmunedToDamage(bot, spell))
        return 0.0f;
    // Vek'lor's tank holds him from outside Arcane Burst with taunt threat only, and his Shadow Bolt
    // hits for ~4k. Damage pauses before it would out-threaten that tank.
    if (control->Encounter() == Aq40Encounter::Twins && hostile && !tank &&
        target->GetEntry() == Id(Aq40Npcs::NPC_VEKLOR))
        if (Player* owner = control->TankFor(target); owner && owner != bot)
            if (target->GetThreatMgr().GetThreat(bot) + TwinCasterThreatMargin >=
                1.3f * target->GetThreatMgr().GetThreat(owner))
                return 0.0f;
    // The spare tank stays second on Vek'nilash's threat: close behind his tank, never above him.
    if (control->Encounter() == Aq40Encounter::Twins && hostile && control->IsTwinSpareTank(bot) &&
        target->GetEntry() == Id(Aq40Npcs::NPC_VEKNILASH))
        if (Unit* victim = target->GetVictim();
            victim && victim != bot &&
            target->GetThreatMgr().GetThreat(bot) >= 0.9f * target->GetThreatMgr().GetThreat(victim))
            return 0.0f;
    if (control->Encounter() == Aq40Encounter::Viscidus && hostile && target->GetEntry() == Id(Aq40Npcs::NPC_VISCIDUS))
    {
        if (target->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)))
        {
            if ((spell && spell->CalcCastTime() > 0) || action->getName() == "shoot" ||
                action->getName() == "shoot bow" || action->getName() == "shoot gun" ||
                action->getName() == "shoot crossbow")
                return 0.0f;
        }
        else if (spell && !(spell->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) && !tank)
            return 0.0f;
    }
    // Healing also generates area threat. Only harmful actions belong in the damage hold.
    bool areaThreat = action->getThreatType() == Action::ActionThreatType::Aoe;
    bool areaDamage = spell ? !spell->IsPositive() && (areaThreat || spell->IsAffectingArea()) : hostile && areaThreat;
    if (!tank &&
        (!desired || control->Encounter() == Aq40Encounter::Twins || control->Encounter() == Aq40Encounter::Skeram) &&
        areaDamage)
        return 0.0f;
    if (control->Encounter() == Aq40Encounter::Huhuran && EncounterHelpers::IsDpsCooldownAction(bot, action))
        if (Creature* boss = control->Boss())
            if (boss->GetHealthPct() > 30.0f)
                return 0.0f;
    if (control->Encounter() == Aq40Encounter::Skeram)
    {
        Position platform;
        if (control->SkeramPlatform(bot, platform))
        {
            if (hostile && control->Formation(bot, platform))
                return 0.0f;
            if (dynamic_cast<MovementAction*>(action) && !picker)
            {
                constexpr float localReach = 12.0f;
                if (!dynamic_cast<ReachTargetAction*>(action) || !target ||
                    target->GetExactDist2d(platform) > localReach ||
                    std::abs(target->GetPositionZ() - platform.GetPositionZ()) > 3.0f)
                    return 0.0f;
            }
        }
    }
    // A Twin tank belongs to a platform, including while its boss is displaced. Ordinary chase
    // and charge must not undo the anchor as soon as the positioning action reaches its goal.
    if (control->Encounter() == Aq40Encounter::Twins && tank)
    {
        if (dynamic_cast<MovementAction*>(action) && !picker)
            return 0.0f;
        if (spell && spell->HasEffect(SPELL_EFFECT_CHARGE))
            return 0.0f;
    }
    // Encounter positioning owns movement while a phase or role imposes a specific location.
    if (dynamic_cast<MovementAction*>(action) && !picker)
    {
        Position formation;
        if (control->Formation(bot, formation))
            return 0.0f;
    }
    return 1.0f;
}

// Spell preparation wakes the normal encounter action; it never casts on the bot's behalf.
class SkeramExplosionBotScript : public AllSpellScript
{
public:
    SkeramExplosionBotScript() : AllSpellScript("SkeramExplosionBotScript") {}

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!spellInfo || spellInfo->Id != Id(Aq40Spells::SPELL_ARCANE_EXPLOSION) || !caster ||
            caster->GetMapId() != 531 || caster->GetEntry() != Id(Aq40Npcs::NPC_SKERAM))
            return;
        for (auto const& ref : caster->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)))
                continue;
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("aq40", BOT_STATE_COMBAT))
                continue;
            if (player->HasUnitState(UNIT_STATE_CASTING))
            {
                std::string name = player->getClass() == CLASS_MAGE     ? "counterspell"
                                   : player->getClass() == CLASS_SHAMAN ? "wind shear"
                                                                        : "";
                SpellInfo const* interrupt = name.empty() ? nullptr : SpellFor(botAI, name);
                if (!interrupt || !player->HasSpell(interrupt->Id) || player->HasSpellCooldown(interrupt->Id) ||
                    player->HasUnitState(UNIT_STATE_LOST_CONTROL) || !player->IsWithinLOSInMap(caster) ||
                    player->GetDistance(caster) > interrupt->GetMaxRange(false, player) ||
                    caster->IsImmunedToSpell(interrupt))
                    continue;
                Spell check(player, interrupt, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
                check.m_targets.SetUnitTarget(caster);
                if (check.CheckCast(true) != SPELL_CAST_OK)
                    continue;
                botAI->RequestSpellInterrupt();
            }
            botAI->SetNextCheckDelay(0);
        }
    }
};

// The emperors swap instantly, attack the nearest player 2 s later, and Arcane Burst returns 2.3 s after that. Wake
// every AQ40 bot on the teleport so the melee under Vek'lor run and the landing tank taunts in time.
class TwinTeleportBotScript : public AllSpellScript
{
public:
    TwinTeleportBotScript() : AllSpellScript("TwinTeleportBotScript") {}

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!spellInfo ||
            (spellInfo->Id != Id(Aq40Spells::SPELL_TWIN_TELEPORT) &&
             spellInfo->Id != Id(Aq40Spells::SPELL_TWIN_TELEPORT_VISUAL)) ||
            !caster || caster->GetMapId() != 531 ||
            (caster->GetEntry() != Id(Aq40Npcs::NPC_VEKLOR) && caster->GetEntry() != Id(Aq40Npcs::NPC_VEKNILASH)))
            return;
        for (auto const& ref : caster->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || player->HasAura(Id(Aq40Spells::SPELL_TRUE_FULFILLMENT)))
                continue;
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("aq40", BOT_STATE_COMBAT))
                continue;
            botAI->SetNextCheckDelay(0);
        }
    }
};

void AddSC_Aq40BotScripts()
{
    new SkeramExplosionBotScript();
    new TwinTeleportBotScript();
}
