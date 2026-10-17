/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TOCActions.h"
#include "AttackersValue.h"
#include "CellImpl.h"
#include "GenericSpellActions.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr uint32 JOUST_IDLE_SCAN_MS = 500;
constexpr float JOUST_SCAN_RANGE = 60.0f;
constexpr float JOUST_THRUST_RANGE = 6.0f;
constexpr float JOUST_CHARGE_MIN = 8.0f;
constexpr float JOUST_CHARGE_MAX = 25.0f;
constexpr float JOUST_CHARGE_IDEAL = 14.0f;
constexpr float JOUST_FLEE_DISTANCE = 20.0f;
constexpr float JOUST_FLEE_LANE = static_cast<float>(M_PI) / 5.0f;
constexpr float JOUST_FLEE_CONE = static_cast<float>(M_PI) / 6.0f;
constexpr float JOUST_FLEE_ARRIVAL = 2.0f;
constexpr uint32 JOUST_FLEE_TIMEOUT_MS = 3000;
constexpr float JOUST_TRAMPLE_RANGE = 3.0f;
constexpr uint32 JOUST_DEFEND_CHECK_MS = 1000;
constexpr float JOUST_KITE_RADIUS = 12.0f;
constexpr float JOUST_KITE_RING_GAP = 3.0f;
constexpr uint8 JOUST_KITE_RINGS = 3;
constexpr float JOUST_KITE_ARC = 12.0f;
constexpr float JOUST_MOVE_REFRESH_DIST = 5.0f;
constexpr uint32 JOUST_MOVE_REFRESH_MS = 750;
constexpr float ARENA_RADIUS = 42.0f;
constexpr float ARENA_MARGIN = 3.0f;
constexpr uint8 DEFEND_MAX_STACKS = 3;

Position const ARENA_CENTER = {748.309f, 619.448f, 411.3f};

uint32 FactionMountEntry(Player* bot)
{
    return bot->GetTeamId() == TEAM_HORDE ? NPC_ARGENT_BATTLEWORG : NPC_ARGENT_WARHORSE;
}

bool IsEntryIn(std::vector<uint32> const& entries, uint32 entry)
{
    return std::find(entries.begin(), entries.end(), entry) != entries.end();
}

enum JoustSpellSlot : uint8
{
    JOUST_SPELL_DEFEND,
    JOUST_SPELL_CHARGE,
    JOUST_SPELL_THRUST,
    JOUST_SPELL_SHIELD_BREAKER
};

char const* const JOUST_SPELL_NAMES[] = {"Defend", "Charge", "Thrust", "Shield-Breaker"};

uint32 CooldownLeft(Unit* vehicleBase, uint32 spellId)
{
    Creature* creature = vehicleBase->ToCreature();
    return creature ? creature->GetSpellCooldown(spellId) : 0;
}

void ExtendCooldown(Unit* vehicleBase, uint32 spellId, uint32 duration)
{
    if (spellId && duration > CooldownLeft(vehicleBase, spellId))
        vehicleBase->AddSpellCooldown(spellId, 0, duration);
}

bool IsShielded(Unit* unit)
{
    return unit->HasAura(SPELL_BOSS_DEFEND) || unit->HasAura(SPELL_MINIONS_DEFEND);
}

uint8 DefendStacks(Unit* unit)
{
    Aura* aura = unit->GetAura(SPELL_DEFEND);
    return aura ? aura->GetStackAmount() : 0;
}

class JoustCreatureCheck
{
public:
    bool operator()(Creature* creature) const
    {
        return IsEntryIn(availableTargets, creature->GetEntry()) && creature->IsAlive() &&
               creature->GetExactDist2d(ARENA_CENTER) <= JOUST_SCAN_RANGE;
    }
};
}

bool ToCLanceAction::Execute(Event /*event*/)
{
    // If already has lance equipped, do nothing
    if (bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return false;

    // Store current mainhand item
    Item* oldWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    // Search inventory for the lance item
    Item* lanceItem = nullptr;

    // Check main inventory
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && item->GetEntry() == ITEM_LANCE)
        {
            lanceItem = item;
            break;
        }
    }

    // Check bags if not found in main inventory
    if (!lanceItem)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; bag++)
        {
            Bag* pBag = bot->GetBagByPos(bag);
            if (pBag)
            {
                for (uint8 slot = 0; slot < pBag->GetBagSize(); slot++)
                {
                    Item* item = bot->GetItemByPos(bag, slot);
                    if (item && item->GetEntry() == ITEM_LANCE)
                    {
                        lanceItem = item;
                        break;
                    }
                }
            }
            if (lanceItem)
                break;
        }
    }

    // If we found the lance, equip it
    if (lanceItem)
    {
        // First unequip current weapon if it exists
        if (oldWeapon)
            bot->SwapItem(oldWeapon->GetPos(), lanceItem->GetPos());

        else
            bot->EquipItem(EQUIPMENT_SLOT_MAINHAND, lanceItem, true);

        return true;
    }

    if (bot->HasItemCount(ITEM_LANCE, 1))
        return false;

    GameObject* lanceRack = bot->FindNearestGameObject(OBJECT_LANCE_RACK, 100.0f);
    if (!lanceRack)
        return false;

    if (!lanceRack->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
        return MoveTo(lanceRack, INTERACTION_DISTANCE);

    botAI->RemoveShapeshift();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    lanceRack->Use(bot);

    return false;
}

bool ToCUELanceAction::Execute(Event event)
{
    if (!bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return false;

    // Call EquipUpgradeAction
    EquipUpgradeAction equipUpgradeAction(botAI);
    if (!equipUpgradeAction.Execute(event))
        return false;

    return false;
}

void ToCMountedAction::ScanArena(std::vector<Unit*>& hostiles, std::vector<Unit*>& walking)
{
    std::vector<Creature*> creatures;
    JoustCreatureCheck check;
    Acore::CreatureListSearcher<JoustCreatureCheck> searcher(bot, creatures, check);
    Cell::VisitObjects(ARENA_CENTER.GetPositionX(), ARENA_CENTER.GetPositionY(), bot->GetMap(), searcher,
                       JOUST_SCAN_RANGE);

    for (Creature* creature : creatures)
    {
        if (IsEntryIn(availableChampions, creature->GetEntry()) &&
            !creature->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID) && creature->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            walking.push_back(creature);
        else if (!creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE_2 |
                                                          UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE))
            hostiles.push_back(creature);
    }
}

Unit* ToCMountedAction::FindJoustTarget(std::vector<Unit*>& hostiles)
{
    std::sort(hostiles.begin(), hostiles.end(),
              [](Unit* a, Unit* b)
              {
                  if (a->GetHealth() != b->GetHealth())
                      return a->GetHealth() < b->GetHealth();

                  return a->GetGUID() < b->GetGUID();
              });

    for (Unit* unit : hostiles)
    {
        if (AttackersValue::IsPossibleTarget(unit, bot))
            return unit;
    }

    return nullptr;
}

std::vector<ObjectGuid> ToCMountedAction::MountedRiders(bool tramplersOnly)
{
    std::vector<ObjectGuid> riders;
    uint32 mountEntry = FactionMountEntry(bot);
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                continue;

            if (tramplersOnly && !GET_PLAYERBOT_AI(member))
                continue;

            Unit* base = member->GetVehicleBase();
            if (!base)
                continue;

            uint32 entry = base->GetEntry();
            if (tramplersOnly ? entry != mountEntry : (entry != NPC_ARGENT_BATTLEWORG && entry != NPC_ARGENT_WARHORSE))
                continue;

            riders.push_back(member->GetGUID());
        }
    }
    else
        riders.push_back(bot->GetGUID());

    std::sort(riders.begin(), riders.end());
    return riders;
}

Unit* ToCMountedAction::FindTrampleTarget(std::vector<Unit*>& walking)
{
    if (walking.empty())
        return nullptr;

    std::sort(walking.begin(), walking.end(),
              [](Unit* a, Unit* b) { return a->GetGUID() < b->GetGUID(); });

    std::vector<ObjectGuid> riders = MountedRiders(true);
    for (size_t i = 0; i < riders.size() && i < walking.size(); ++i)
    {
        if (riders[i] == bot->GetGUID())
            return walking[i];
    }

    return nullptr;
}

size_t ToCMountedAction::RiderLane(std::vector<ObjectGuid> const& riders)
{
    size_t lane = std::find(riders.begin(), riders.end(), bot->GetGUID()) - riders.begin();
    return lane < riders.size() ? lane : 0;
}

uint32 ToCMountedAction::JoustSpellId(Unit* vehicleBase, uint8 slot)
{
    if (spellIdsBase != vehicleBase->GetGUID())
    {
        spellIdsBase = vehicleBase->GetGUID();
        for (uint8 i = 0; i < 4; ++i)
            spellIds[i] = AI_VALUE2(uint32, "vehicle spell id", JOUST_SPELL_NAMES[i]);
    }

    return spellIds[slot];
}

bool ToCMountedAction::CastJoustSpell(uint32 spellId, Unit* target)
{
    if (!botAI->CanCastVehicleSpell(spellId, target) || !botAI->CastVehicleSpell(spellId, target))
        return false;

    Unit* vehicleBase = bot->GetVehicleBase();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!vehicleBase || !spellInfo)
        return true;

    if (uint32 category = spellInfo->GetCategory())
    {
        for (uint8 slot = 0; slot < 4; ++slot)
        {
            SpellInfo const* other = sSpellMgr->GetSpellInfo(JoustSpellId(vehicleBase, slot));
            if (other && other->GetCategory() == category)
                ExtendCooldown(vehicleBase, other->Id, spellInfo->CategoryRecoveryTime);
        }
    }

    ExtendCooldown(vehicleBase, spellId, spellInfo->RecoveryTime);
    return true;
}

bool ToCMountedAction::AimAndCast(Unit* vehicleBase, uint32 spellId, Unit* target)
{
    if (!spellId || vehicleBase->HasSpellCooldown(spellId))
        return false;

    if (vehicleBase->HasInArc(CAST_ANGLE_IN_FRONT, target))
        return CastJoustSpell(spellId, target);

    if (!botAI->CanCastVehicleSpell(spellId, target))
        return false;

    vehicleBase->GetMotionMaster()->Clear();
    vehicleBase->StopMoving();
    vehicleBase->SetFacingToObject(target);
    moveMode = JOUST_MOVE_NONE;
    return true;
}

bool ToCMountedAction::DelaysCharge(Unit* vehicleBase, uint32 spellId, uint32 chargeWait)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    SpellInfo const* chargeInfo = sSpellMgr->GetSpellInfo(JoustSpellId(vehicleBase, JOUST_SPELL_CHARGE));
    if (!spellInfo || !chargeInfo || !spellInfo->GetCategory() || spellInfo->GetCategory() != chargeInfo->GetCategory())
        return false;

    return spellInfo->CategoryRecoveryTime > chargeWait;
}

bool ToCMountedAction::CastDefend(Unit* vehicleBase, uint8 belowStacks, uint32 chargeWait)
{
    uint32 defendId = JoustSpellId(vehicleBase, JOUST_SPELL_DEFEND);
    return std::max(DefendStacks(bot), DefendStacks(vehicleBase)) < belowStacks &&
           !vehicleBase->HasSpellCooldown(defendId) && !DelaysCharge(vehicleBase, defendId, chargeWait) &&
           CastJoustSpell(defendId, vehicleBase);
}

bool ToCMountedAction::RefreshDefend(Unit* vehicleBase, uint32 now)
{
    if (now < defendCheckAt)
        return false;

    defendCheckAt = now + JOUST_DEFEND_CHECK_MS;
    return CastDefend(vehicleBase, DEFEND_MAX_STACKS, UINT32_MAX);
}

bool ToCMountedAction::UseFillers(Unit* vehicleBase, Unit* target, float dist, uint32 chargeWait)
{
    if (CastDefend(vehicleBase, 1, chargeWait))
        return true;

    if (dist > JOUST_THRUST_RANGE && dist <= JOUST_CHARGE_MAX && IsShielded(target))
    {
        uint32 breakerId = JoustSpellId(vehicleBase, JOUST_SPELL_SHIELD_BREAKER);
        if (!DelaysCharge(vehicleBase, breakerId, chargeWait) && AimAndCast(vehicleBase, breakerId, target))
            return true;
    }

    return CastDefend(vehicleBase, DEFEND_MAX_STACKS, chargeWait);
}

void ToCMountedAction::FleeDestination(Unit* target, std::vector<Unit*> const& hostiles, float& x, float& y)
{
    std::vector<Unit*> nearby;
    float cx = 0.0f;
    float cy = 0.0f;
    for (Unit* unit : hostiles)
    {
        if (bot->GetDistance2d(unit) > JOUST_CHARGE_MAX)
            continue;

        nearby.push_back(unit);
        cx += unit->GetPositionX();
        cy += unit->GetPositionY();
    }

    if (nearby.empty())
    {
        nearby.push_back(target);
        cx = target->GetPositionX();
        cy = target->GetPositionY();
    }
    else
    {
        cx /= nearby.size();
        cy /= nearby.size();
    }

    Position const centroid(cx, cy);
    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float away;
    if (bot->GetExactDist2d(centroid) > 1.0f)
        away = centroid.GetAngle(bot);
    else if (bot->GetDistance2d(target) > 1.0f)
        away = target->GetAngle(bot);
    else
        away = bot->GetOrientation() + static_cast<float>(M_PI);

    std::vector<ObjectGuid> riders = MountedRiders(false);
    if (riders.empty())
        riders.push_back(bot->GetGUID());

    size_t lane = RiderLane(riders);
    away += (static_cast<float>(lane) - static_cast<float>(riders.size() - 1) * 0.5f) * JOUST_FLEE_LANE;

    bool found = false;
    bool insideOnly = false;
    for (int step = 0; step <= 6 && !found; ++step)
    {
        for (int sign = 1; sign >= -1 && !found; sign -= 2)
        {
            if (step == 0 && sign < 0)
                continue;

            float bearing = away + sign * step * JOUST_FLEE_CONE;
            float px = bx + std::cos(bearing) * JOUST_FLEE_DISTANCE;
            float py = by + std::sin(bearing) * JOUST_FLEE_DISTANCE;
            if (ARENA_CENTER.GetExactDist2d(px, py) > ARENA_RADIUS)
                continue;

            if (!insideOnly)
            {
                x = px;
                y = py;
                insideOnly = true;
            }

            bool blocked = false;
            for (Unit* unit : nearby)
            {
                float delta = Position::NormalizeOrientation(bot->GetAngle(unit) - bearing);
                if (delta > static_cast<float>(M_PI))
                    delta = 2.0f * static_cast<float>(M_PI) - delta;

                if (delta < JOUST_FLEE_CONE && bot->GetDistance2d(unit) < JOUST_FLEE_DISTANCE)
                {
                    blocked = true;
                    break;
                }
            }

            if (blocked)
                continue;

            x = px;
            y = py;
            found = true;
        }
    }

    if (!found && !insideOnly)
    {
        float bearing = bot->GetAngle(&ARENA_CENTER);
        x = bx + std::cos(bearing) * JOUST_FLEE_DISTANCE;
        y = by + std::sin(bearing) * JOUST_FLEE_DISTANCE;
    }
}

bool ToCMountedAction::StartFlee(Unit* target, std::vector<Unit*> const& hostiles, uint32 now)
{
    float x, y;
    FleeDestination(target, hostiles, x, y);
    if (!MoveJoust(JOUST_MOVE_FLEE, x, y, now))
        return false;

    if (!fleeUntil)
        fleeUntil = now + JOUST_FLEE_TIMEOUT_MS;

    return true;
}

void ToCMountedAction::KiteDestination(Unit* target, float& x, float& y)
{
    size_t lane = RiderLane(MountedRiders(false));
    if (!kiteDir)
        kiteDir = lane % 2 ? -1 : 1;

    float radius = JOUST_KITE_RADIUS + static_cast<float>(lane % JOUST_KITE_RINGS) * JOUST_KITE_RING_GAP;
    float bearing = target->GetExactDist2d(bot) > 0.5f ? target->GetAngle(bot)
                                                         : bot->GetOrientation() + static_cast<float>(M_PI);
    float limit = ARENA_RADIUS - ARENA_MARGIN;

    for (uint8 attempt = 0; attempt < 2; ++attempt)
    {
        float angle = bearing + kiteDir * JOUST_KITE_ARC / radius;
        x = target->GetPositionX() + std::cos(angle) * radius;
        y = target->GetPositionY() + std::sin(angle) * radius;
        if (ARENA_CENTER.GetExactDist2d(x, y) <= limit)
            return;

        kiteDir = -kiteDir;
    }

    float angle = ARENA_CENTER.GetAngle(x, y);
    x = ARENA_CENTER.GetPositionX() + std::cos(angle) * limit;
    y = ARENA_CENTER.GetPositionY() + std::sin(angle) * limit;
}

bool ToCMountedAction::Kite(Unit* vehicleBase, Unit* target, uint32 now)
{
    if (KeepMoving(vehicleBase, JOUST_MOVE_KITE, now))
        return true;

    float x, y;
    KiteDestination(target, x, y);
    return MoveJoust(JOUST_MOVE_KITE, x, y, now);
}

bool ToCMountedAction::Approach(Unit* vehicleBase, Unit* target, uint32 now)
{
    if (KeepMoving(vehicleBase, JOUST_MOVE_APPROACH, now))
        return true;

    float angle = target->GetAngle(bot);
    return MoveJoust(JOUST_MOVE_APPROACH, target->GetPositionX() + std::cos(angle) * JOUST_CHARGE_IDEAL,
                     target->GetPositionY() + std::sin(angle) * JOUST_CHARGE_IDEAL, now);
}

bool ToCMountedAction::KeepMoving(Unit* vehicleBase, JoustMove mode, uint32 now)
{
    return moveMode == mode && now < moveRefreshAt && vehicleBase->isMoving() &&
           vehicleBase->GetExactDist2d(&moveDest) > JOUST_MOVE_REFRESH_DIST;
}

bool ToCMountedAction::MoveJoust(JoustMove mode, float x, float y, uint32 now)
{
    float z = bot->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    if (!MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT))
        return false;

    moveMode = mode;
    moveDest.Relocate(x, y, z);
    moveRefreshAt = now + JOUST_MOVE_REFRESH_MS;
    return true;
}

bool ToCMountedAction::ChargeCycle(Unit* vehicleBase, Unit* target, std::vector<Unit*> const& hostiles, float dist,
                                   uint32 now)
{
    if (fleeUntil)
    {
        bool done = dist >= JOUST_CHARGE_IDEAL || now > fleeUntil ||
                    vehicleBase->GetExactDist2d(&moveDest) < JOUST_FLEE_ARRIVAL;
        if (!done && ((moveMode == JOUST_MOVE_FLEE && vehicleBase->isMoving()) || StartFlee(target, hostiles, now)))
            return true;

        if (dist < JOUST_CHARGE_MIN)
            fleeBlockedUntil = now + JOUST_FLEE_TIMEOUT_MS;

        fleeUntil = 0;
    }

    if (dist > JOUST_CHARGE_MAX)
        return Approach(vehicleBase, target, now);

    if (dist >= JOUST_CHARGE_MIN)
        return AimAndCast(vehicleBase, JoustSpellId(vehicleBase, JOUST_SPELL_CHARGE), target);

    return now > fleeBlockedUntil && StartFlee(target, hostiles, now);
}

bool ToCMountedAction::Execute(Event /*event*/)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    if (vehicleBase->HasUnitState(UNIT_STATE_CHARGING))
        return true;

    uint32 now = getMSTime();
    if (now < idleUntil)
        return RefreshDefend(vehicleBase, now);

    std::vector<Unit*> hostiles;
    std::vector<Unit*> walking;
    ScanArena(hostiles, walking);

    if (Unit* champion = FindTrampleTarget(walking))
    {
        fleeUntil = 0;
        if (moveMode != JOUST_MOVE_NONE)
        {
            AI_VALUE(LastMovement&, "last movement").Set(nullptr);
            moveMode = JOUST_MOVE_NONE;
        }

        if (bot->GetDistance2d(champion) > JOUST_TRAMPLE_RANGE &&
            !IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
            MoveTo(champion, 1.0f, MovementPriority::MOVEMENT_COMBAT);

        RefreshDefend(vehicleBase, now);
        return true;
    }

    Unit* target = FindJoustTarget(hostiles);
    if (!target)
    {
        fleeUntil = 0;
        kiteDir = 0;
        idleUntil = now + JOUST_IDLE_SCAN_MS;
        return RefreshDefend(vehicleBase, now);
    }

    uint32 chargeId = JoustSpellId(vehicleBase, JOUST_SPELL_CHARGE);
    uint32 chargeWait = chargeId ? CooldownLeft(vehicleBase, chargeId) : UINT32_MAX;
    float dist = bot->GetDistance2d(target);

    if (dist <= JOUST_THRUST_RANGE && vehicleBase->HasInArc(CAST_ANGLE_IN_FRONT, target) &&
        CastJoustSpell(JoustSpellId(vehicleBase, JOUST_SPELL_THRUST), target))
        return true;

    if (chargeWait)
        fleeUntil = 0;
    else if (ChargeCycle(vehicleBase, target, hostiles, dist, now))
        return true;

    if (UseFillers(vehicleBase, target, dist, chargeWait))
        return true;

    return Kite(vehicleBase, target, now);
}

bool ToCMountAction::Execute(Event /*event*/)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    Unit* vehicleToEnter = FindMount(botAI);
    if (!vehicleToEnter)
        return false;

    return EnterVehicle(vehicleToEnter, true);
}

Unit* ToCMountAction::FindMount(PlayerbotAI* botAI)
{
    Player* player = botAI->GetBot();
    Unit* preferredMount = nullptr;
    Unit* otherMount = nullptr;
    uint32 preferredEntry = FactionMountEntry(player);
    GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest vehicles")->Get();
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (!vehicleBase->IsFriendlyTo(player))
            continue;

        if (!vehicleBase->GetVehicleKit() || !vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        uint32 entry = vehicleBase->GetEntry();
        if (entry != NPC_ARGENT_BATTLEWORG && entry != NPC_ARGENT_WARHORSE)
            continue;

        Unit*& slot = entry == preferredEntry ? preferredMount : otherMount;
        if (!slot || player->GetExactDist(slot) > player->GetExactDist(vehicleBase))
            slot = vehicleBase;
    }

    return preferredMount ? preferredMount : otherMount;
}

bool ToCMountAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = bot->GetDistance(vehicleBase);

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    botAI->RemoveShapeshift();

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool ToCEadricAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "eadric the pure");
    if (!boss)
        return false;

    // If Eadric is casting Radiance, face the opposite direction
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_RADIANCE))
    {
        // Calculate the opposite direction
        float angle = bot->GetAngle(boss);
        float newAngle = Position::NormalizeOrientation(angle + M_PI);  // Add 180 degrees (PI radians)

        // Set the bot's orientation to face away from Eadric
        bot->SetFacingTo(newAngle);
        return true;
    }

    return false;
}
