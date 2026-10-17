/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EquipAction.h"
#include "GenericSpellActions.h"
#include "ICCActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RtiValue.h"
#include "Vehicle.h"

bool IccCannonFireAction::Execute(Event /*event*/)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();

    if (!vehicleBase || !vehicle)
        return false;

    Unit* target = FindValidCannonTarget();
    if (!target)
        return false;

    // Use Incinerating Blast when we have enough energy, otherwise Cannon Blast
    static constexpr float ENERGY_THRESHOLD = 90.0f;
    if (vehicleBase->GetPower(POWER_ENERGY) >= ENERGY_THRESHOLD)
    {
        uint32 const blastSpellId = AI_VALUE2(uint32, "vehicle spell id", "incinerating blast");
        if (TryCastCannonSpell(blastSpellId, target, vehicleBase))
            return true;
    }

    uint32 const cannonSpellId = AI_VALUE2(uint32, "vehicle spell id", "cannon blast");
    return TryCastCannonSpell(cannonSpellId, target, vehicleBase);
}

Unit* IccCannonFireAction::FindValidCannonTarget()
{
    // availableTargetsGS is ordered per-faction (earliest = highest priority).
    // For each entry, enumerate all creatures and pick the nearest hostile
    // alive one. FindNearestCreature returns the single closest match which
    // may be a friendly of the same entry on our own ship, causing the entry
    // to be skipped even when a hostile of that entry exists on the enemy
    // ship.
    static constexpr float CANNON_TARGET_RANGE = 150.0f;

    for (size_t i = 0; i < availableTargetsGS.size(); ++i)
    {
        std::list<Creature*> candidates;
        bot->GetCreatureListWithEntryInGrid(candidates, availableTargetsGS[i], CANNON_TARGET_RANGE);

        Unit* best = nullptr;
        float bestDist = FLT_MAX;
        for (Creature* c : candidates)
        {
            if (!c || !c->IsAlive() || !c->IsHostileTo(bot))
                continue;
            float const d = bot->GetExactDist(c);
            if (d < bestDist)
            {
                bestDist = d;
                best = c;
            }
        }

        if (best)
            return best;
    }

    return nullptr;
}

bool IccCannonFireAction::TryCastCannonSpell(uint32 spellId, Unit* target, Unit* vehicleBase)
{
    static constexpr uint32 COOLDOWN_MS = 1000;

    if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
    {
        vehicleBase->AddSpellCooldown(spellId, 0, COOLDOWN_MS);
        return true;
    }

    return false;
}

bool IccGunshipEnterCannonAction::Execute(Event /*event*/)
{
    // Never switch vehicles while already in one
    if (bot->GetVehicle())
        return false;

    // Require rocket pack acquired and equipped before boarding a cannon
    Item* rocketPack = bot->GetItemByEntry(ITEM_GOBLIN_ROCKET_PACK);
    if (!rocketPack || !rocketPack->IsEquipped())
        return false;

    Unit* bestVehicle = FindBestAvailableCannon();
    if (!bestVehicle)
        return false;

    return EnterVehicle(bestVehicle, true);
}

Unit* IccGunshipEnterCannonAction::FindBestAvailableCannon()
{
    Unit* bestVehicle = nullptr;

    GuidVector const npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (ObjectGuid const& npcGuid : npcs)
    {
        Unit* vehicleBase = botAI->GetUnit(npcGuid);
        if (!IsValidCannon(vehicleBase))
            continue;

        // Prefer the closest valid cannon
        if (!bestVehicle || bot->GetExactDist(vehicleBase) < bot->GetExactDist(bestVehicle))
            bestVehicle = vehicleBase;
    }

    return bestVehicle;
}

bool IccGunshipEnterCannonAction::IsValidCannon(Unit* vehicle)
{
    if (!vehicle)
        return false;

    if (vehicle->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    if (!vehicle->IsFriendlyTo(bot))
        return false;

    if (!vehicle->GetVehicleKit() || !vehicle->GetVehicleKit()->GetAvailableSeatCount())
        return false;

    uint32 const entry = vehicle->GetEntry();
    if (entry != NPC_CANNONA && entry != NPC_CANNONH)
        return false;

    // Frozen or disabled cannon - skip
    if (vehicle->HasAura(SPELL_FROZEN_CANNON) || vehicle->HasAura(SPELL_BELOW_ZERO))
        return false;

    return true;
}

bool IccGunshipEnterCannonAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float const dist = bot->GetDistance(vehicleBase);

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

    // Dismount - bots can enter a vehicle while still mounted
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    return true;
}

IccGunshipRocketJumpAction::GunshipSide IccGunshipRocketJumpAction::DetectShip() const
{
    Unit* cannonA = bot->FindNearestCreature(NPC_CANNONA, 100.0f);
    if (cannonA && cannonA->IsFriendlyTo(bot))
        return GunshipSide::ALLY;

    Unit* cannonH = bot->FindNearestCreature(NPC_CANNONH, 100.0f);
    if (cannonH && cannonH->IsFriendlyTo(bot))
        return GunshipSide::HORDE;

    return GunshipSide::NONE;
}

bool IccGunshipRocketJumpAction::Execute(Event /*event*/)
{
    GunshipSide const side = DetectShip();
    if (side == GunshipSide::NONE)
        return false;

    // Z-axis correction: teleport bot back if fallen below deck
    if (!bot->GetVehicle())
    {
        Position const& zRef = (side == GunshipSide::ALLY) ? ICC_GUNSHIP_ROCKET_JUMP_ALLY2 : ICC_GUNSHIP_ROCKET_JUMP_HORDE2;
        Position const& zDest = (side == GunshipSide::ALLY) ? ICC_GUNSHIP_ROCKET_JUMP_ALLY2 : ICC_GUNSHIP_ROCKET_JUMP_HORDE2;
        float const Z_THRESHOLD = zRef.GetPositionZ() - 5.0f;
        if (bot->GetPositionZ() < Z_THRESHOLD)
        {
            bot->TeleportTo(bot->GetMapId(), zDest.GetPositionX(), zDest.GetPositionY(),
                            zDest.GetPositionZ(), bot->GetOrientation());
            botAI->Reset();
            return false;
        }
    }

    float const maxWaitingDistance = (side == GunshipSide::ALLY) ? 30.0f : 25.0f;
    static constexpr float MAX_ATTACK_DISTANCE = 20.0f;
    static constexpr float HOLD_RADIUS = 20.0f;
    static constexpr uint8 SKULL_ICON_INDEX = RtiTargetValue::skullIndex;

    uint32 const mageEntry = (side == GunshipSide::ALLY) ? NPC_KOR_KRON_BATTLE_MAGE : NPC_SKYBREAKER_SORCERER;
    Position const& waitPos = (side == GunshipSide::ALLY) ? ICC_GUNSHIP_ROCKET_JUMP_ALLY2 : ICC_GUNSHIP_ROCKET_JUMP_HORDE_FRIENDLY_POINT;
    Position const& attackPos = (side == GunshipSide::ALLY) ? ICC_GUNSHIP_ROCKET_JUMP_ALLY : ICC_GUNSHIP_ROCKET_JUMP_HORDE;

    Unit* boss = nullptr;
    {
        std::list<Creature*> mages;
        bot->GetCreatureListWithEntryInGrid(mages, mageEntry, 200.0f);
        for (Creature* m : mages)
        {
            if (m && m->IsAlive() && m->HasUnitState(UNIT_STATE_CASTING))
            {
                boss = m;
                break;
            }
        }
        if (!boss)
            boss = bot->FindNearestCreature(mageEntry, 200.0f);
    }

    CleanupSkullIcon(SKULL_ICON_INDEX);

    uint32 const cannonEntry = (side == GunshipSide::ALLY) ? NPC_CANNONA : NPC_CANNONH;
    bool cannonsHaveBelowZero = false;
    {
        std::list<Creature*> cannons;
        bot->GetCreatureListWithEntryInGrid(cannons, cannonEntry, 100.0f);
        for (Creature* c : cannons)
        {
            if (c && c->IsFriendlyTo(bot) && c->HasAura(SPELL_BELOW_ZERO))
            {
                cannonsHaveBelowZero = true;
                break;
            }
        }
    }

    bool const botOnEnemyShip = bot->GetExactDist2d(attackPos) <= MAX_ATTACK_DISTANCE;
    bool const mageAlive = boss && boss->IsAlive() && boss->HasUnitState(UNIT_STATE_CASTING);

    // Assist tank stays on friendly ship to collect and tank adds
    if (PlayerbotAI::IsAssistTank(bot))
    {
        GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

        // Find nearest uncollected add near hold position
        Unit* targetAdd = nullptr;
        float closestDist = FLT_MAX;

        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetExactDist2d(waitPos) > HOLD_RADIUS)
                continue;

            Unit* victim = unit->GetVictim();
            bool alreadyTanked = victim && victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer());
            if (alreadyTanked)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < closestDist)
            {
                closestDist = dist;
                targetAdd = unit;
            }
        }

        if (targetAdd)
        {
            IccCastClassTaunt(bot, botAI, targetAdd);
            bot->SetTarget(targetAdd->GetGUID());
            bot->SetFacingToObject(targetAdd);
            Attack(targetAdd);
            return false;
        }

        // No uncollected adds - keep attacking adds targeting us
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit->GetExactDist2d(waitPos) > HOLD_RADIUS)
                continue;

            if (unit->GetVictim() == bot)
            {
                bot->SetTarget(unit->GetGUID());
                bot->SetFacingToObject(unit);
                Attack(unit);
                return false;
            }
        }

        // No adds — return to hold position if drifted
        if (bot->GetExactDist2d(waitPos) > HOLD_RADIUS)
        {
            return MoveTo(bot->GetMapId(), waitPos.GetPositionX(), waitPos.GetPositionY(), waitPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_NORMAL);
        }

        return false;
    }

    bool const isMainTank = PlayerbotAI::IsMainTank(bot);
    bool const isHorde = (side == GunshipSide::HORDE);
    Position const& middlePoint = ICC_GUNSHIP_ROCKET_JUMP_HORDE_MIDDLE_POINT;
    static constexpr float JUMP_GATE = 30.0f;

    // Main tank: tank captain during below-zero OR while an ally is on enemy ship.
    // Otherwise return to friendly ship — captain one-shots if tanked too long.
    if (isMainTank)
    {
        uint32 const captainEntry = (side == GunshipSide::ALLY) ? NPC_HIGH_OVERLORD_SAURFANG : NPC_MURADIN_BRONZEBEARD;
        Unit* captain = bot->FindNearestCreature(captainEntry, 1000.0f);

        // Battle Fury buff bugged on captain - strip all difficulty variants every tick. todo: remove when boss agro fixed
        if (captain)
        {
            static constexpr uint32 BATTLE_FURY_IDS[] = {SPELL_BATTLE_FURY1, SPELL_BATTLE_FURY2,
                                                        SPELL_BATTLE_FURY3, SPELL_BATTLE_FURY4,
                                                        SPELL_BATTLE_FURY5};
            for (uint32 const id : BATTLE_FURY_IDS)
                if (captain->HasAura(id))
                    captain->RemoveAurasDueToSpell(id);
        }

        bool allyAtMage = false;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member == bot)
                    continue;
                if (PlayerbotAI::IsMainTank(member) || PlayerbotAI::IsAssistTank(member))
                    continue;
                if (member->GetExactDist2d(attackPos) <= 20.0f)
                {
                    allyAtMage = true;
                    break;
                }
            }
        }
        bool const shouldTankCaptain = cannonsHaveBelowZero || allyAtMage;

        // Captain gone or outside tanking window: return to friendly ship if stranded, else hold.
        // Drop target/threat so combat rotation doesn't chase adds on friendly ship.
        if (!captain || !captain->IsAlive() || !shouldTankCaptain)
        {
            if (bot->GetVictim())
            {
                bot->AttackStop();
                bot->CastStop();
                bot->SetTarget(ObjectGuid::Empty);
            }
            if (botOnEnemyShip)
            {
                // Main tank jumps back last - wait for all non-tanks to leave enemy ship
                if (AnyNonTankAwayFromFriendly(side))
                    return false;

                if (ExitCannonIfSeated())
                    return true;

                if (isHorde)
                {
                    float const distFriendly = bot->GetExactDist2d(waitPos);
                    float const distMiddle = bot->GetExactDist2d(middlePoint);
                    if (distMiddle <= 10.0f && distFriendly > 5.0f)
                    {
                        if (UseRocketPack(waitPos, /*walkIfOutOfRange=*/true))
                            return true;
                        return MoveTo(bot->GetMapId(), waitPos.GetPositionX(), waitPos.GetPositionY(),
                                      waitPos.GetPositionZ(), false, false, false, false,
                                      MovementPriority::MOVEMENT_NORMAL);
                    }
                    if (distMiddle <= JUMP_GATE)
                    {
                        if (UseRocketPack(middlePoint, /*walkIfOutOfRange=*/false))
                            return true;
                        return MoveTo(bot->GetMapId(), middlePoint.GetPositionX(), middlePoint.GetPositionY(),
                                      middlePoint.GetPositionZ(), false, false, false, false,
                                      MovementPriority::MOVEMENT_NORMAL);
                    }
                    if (UseRocketPack(middlePoint, /*walkIfOutOfRange=*/true))
                        return true;
                    return MoveTo(bot->GetMapId(), middlePoint.GetPositionX(), middlePoint.GetPositionY(),
                                  middlePoint.GetPositionZ(), false, false, false, false,
                                  MovementPriority::MOVEMENT_NORMAL);
                }

                // Alliance MT return: ALLY_MIDDLE_POINT -> ALLY_FRIENDLY_POINT
                Position const& allyFriendly = ICC_GUNSHIP_ROCKET_JUMP_ALLY_FRIENDLY_POINT;
                Position const& allyMiddle = ICC_GUNSHIP_ROCKET_JUMP_ALLY_MIDDLE_POINT;
                float const distAllyMid = bot->GetExactDist2d(allyMiddle);
                float const distAllyFriendly = bot->GetExactDist2d(allyFriendly);

                if (distAllyMid <= 10.0f && distAllyFriendly > 5.0f)
                {
                    if (UseRocketPack(allyFriendly, /*walkIfOutOfRange=*/true))
                        return true;
                    return MoveTo(bot->GetMapId(), allyFriendly.GetPositionX(), allyFriendly.GetPositionY(),
                                  allyFriendly.GetPositionZ(), false, false, false, false,
                                  MovementPriority::MOVEMENT_NORMAL);
                }
                if (distAllyMid <= JUMP_GATE)
                {
                    if (UseRocketPack(allyMiddle, /*walkIfOutOfRange=*/false))
                        return true;
                    return MoveTo(bot->GetMapId(), allyMiddle.GetPositionX(), allyMiddle.GetPositionY(),
                                  allyMiddle.GetPositionZ(), false, false, false, false,
                                  MovementPriority::MOVEMENT_NORMAL);
                }
                if (UseRocketPack(allyMiddle, /*walkIfOutOfRange=*/true))
                    return true;
                return MoveTo(bot->GetMapId(), allyMiddle.GetPositionX(), allyMiddle.GetPositionY(),
                              allyMiddle.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_NORMAL);
            }
            if (bot->GetExactDist2d(waitPos) > maxWaitingDistance)
            {
                if (ExitCannonIfSeated())
                    return true;
                return UseRocketPack(waitPos, /*walkIfOutOfRange=*/true);
            }
            return false;
        }

        // Not on enemy ship: rocket directly to captain
        if (!botOnEnemyShip)
        {
            if (ExitCannonIfSeated())
                return true;

            Position const captainPos(captain->GetPositionX(), captain->GetPositionY(), captain->GetPositionZ());

            if (isHorde)
            {
                Position const& friendlyPoint = ICC_GUNSHIP_ROCKET_JUMP_HORDE_FRIENDLY_POINT;
                float const distFriendly = bot->GetExactDist2d(friendlyPoint);
                float const distMiddle = bot->GetExactDist2d(middlePoint);
                float const distCaptain = bot->GetExactDist2d(&captainPos);

                if (distCaptain <= JUMP_GATE || distMiddle <= 5.0f)
                    return UseRocketPack(captainPos, /*walkIfOutOfRange=*/false);
                if (distFriendly <= 5.0f)
                    return RocketPackJumpToward(middlePoint);
                if (distMiddle > JUMP_GATE && distCaptain > JUMP_GATE)
                    return UseRocketPack(friendlyPoint, /*walkIfOutOfRange=*/true);

                Position const* nearest = &friendlyPoint;
                float nearestDist = distFriendly;
                if (distMiddle < nearestDist) { nearest = &middlePoint; nearestDist = distMiddle; }
                if (distCaptain < nearestDist) { nearest = &captainPos; nearestDist = distCaptain; }
                return UseRocketPack(*nearest, /*walkIfOutOfRange=*/true);
            }

            // Alliance: ALLY_FRIENDLY_POINT -> ALLY_MIDDLE_POINT -> captain
            Position const& allyFriendly = ICC_GUNSHIP_ROCKET_JUMP_ALLY_FRIENDLY_POINT;
            Position const& allyMiddle = ICC_GUNSHIP_ROCKET_JUMP_ALLY_MIDDLE_POINT;
            float const distAllyFriendly = bot->GetExactDist2d(allyFriendly);
            float const distAllyMid = bot->GetExactDist2d(allyMiddle);
            float const distCaptainA = bot->GetExactDist2d(&captainPos);

            if (distCaptainA <= JUMP_GATE || distAllyMid <= 5.0f)
                return UseRocketPack(captainPos, /*walkIfOutOfRange=*/false);
            if (distAllyFriendly <= 5.0f)
                return RocketPackJumpToward(allyMiddle);
            if (distAllyMid > JUMP_GATE && distCaptainA > JUMP_GATE)
                return UseRocketPack(allyFriendly, /*walkIfOutOfRange=*/true);

            Position const* nearestA = &allyFriendly;
            float nearestDistA = distAllyFriendly;
            if (distAllyMid < nearestDistA) { nearestA = &allyMiddle; nearestDistA = distAllyMid; }
            if (distCaptainA < nearestDistA) { nearestA = &captainPos; nearestDistA = distCaptainA; }
            return UseRocketPack(*nearestA, /*walkIfOutOfRange=*/true);
        }

        // On enemy ship. If not captain's victim, force aggro.
        if (captain->GetVictim() != bot)
        {
            IccCastClassTaunt(bot, botAI, captain);
            bot->SetTarget(captain->GetGUID());
            bot->SetFacingToObject(captain);
            Attack(captain);
            return false;
        }

        // Captain's victim: drag boss to 40f-from-friendly-cannon spot on enemy ship
        static constexpr float TANK_OFFSET_FROM_CANNON = 40.0f;
        static constexpr float TANK_POS_TOLERANCE = 3.0f;
        Unit* friendlyCannon = FindNearestFriendlyCannon(side);
        if (friendlyCannon)
        {
            float const cx = friendlyCannon->GetPositionX();
            float const cy = friendlyCannon->GetPositionY();
            float const dirX = attackPos.GetPositionX() - cx;
            float const dirY = attackPos.GetPositionY() - cy;
            float const dirLen = std::sqrt(dirX * dirX + dirY * dirY);
            if (dirLen > 0.1f)
            {
                float const tx = cx + (dirX / dirLen) * TANK_OFFSET_FROM_CANNON;
                float const ty = cy + (dirY / dirLen) * TANK_OFFSET_FROM_CANNON;
                float const tz = bot->GetPositionZ();
                float const dx = bot->GetPositionX() - tx;
                float const dy = bot->GetPositionY() - ty;
                if (std::sqrt(dx * dx + dy * dy) > TANK_POS_TOLERANCE)
                {
                    bot->SetTarget(captain->GetGUID());
                    return MoveTo(bot->GetMapId(), tx, ty, tz, false, false, false, false,
                                  MovementPriority::MOVEMENT_COMBAT);
                }
            }
        }

        bot->SetTarget(captain->GetGUID());
        bot->SetFacingToObject(captain);
        Attack(captain);
        return false;
    }

    // Non-tank rocket jump logic

    if (cannonsHaveBelowZero && mageAlive)
    {
        UpdateBossSkullIcon(boss, SKULL_ICON_INDEX);
        if (!botOnEnemyShip)
        {
            if (ExitCannonIfSeated())
                return true;

            if (isHorde)
            {
                Position const& friendlyPoint = ICC_GUNSHIP_ROCKET_JUMP_HORDE_FRIENDLY_POINT;
                float const distFriendly = bot->GetExactDist2d(friendlyPoint);
                float const distMiddle = bot->GetExactDist2d(middlePoint);
                float const distMage = bot->GetExactDist2d(attackPos);

                // Mage in range or already at middle: jump mage
                if (distMage <= JUMP_GATE || distMiddle <= 5.0f)
                    return UseRocketPack(attackPos, /*walkIfOutOfRange=*/false);
                // At friendly: jump middle
                if (distFriendly <= 5.0f)
                    return RocketPackJumpToward(middlePoint);
                // Both middle and mage out of range: detour through friendly
                if (distMiddle > JUMP_GATE && distMage > JUMP_GATE)
                    return UseRocketPack(friendlyPoint, /*walkIfOutOfRange=*/true);

                // Fallback: jump nearest of the three
                Position const* nearest = &friendlyPoint;
                float nearestDist = distFriendly;
                if (distMiddle < nearestDist) { nearest = &middlePoint; nearestDist = distMiddle; }
                if (distMage < nearestDist) { nearest = &attackPos; nearestDist = distMage; }
                return UseRocketPack(*nearest, /*walkIfOutOfRange=*/true);
            }

            // Alliance non-tank: ALLY_FRIENDLY_POINT -> ALLY_MIDDLE_POINT -> mage (attackPos)
            Position const& allyFriendly = ICC_GUNSHIP_ROCKET_JUMP_ALLY_FRIENDLY_POINT;
            Position const& allyMiddle = ICC_GUNSHIP_ROCKET_JUMP_ALLY_MIDDLE_POINT;
            float const distAllyFriendly = bot->GetExactDist2d(allyFriendly);
            float const distAllyMid = bot->GetExactDist2d(allyMiddle);
            float const distMageA = bot->GetExactDist2d(attackPos);

            if (distMageA <= JUMP_GATE || distAllyMid <= 5.0f)
                return UseRocketPack(attackPos, /*walkIfOutOfRange=*/false);
            if (distAllyFriendly <= 5.0f)
                return RocketPackJumpToward(allyMiddle);
            if (distAllyMid > JUMP_GATE && distMageA > JUMP_GATE)
                return UseRocketPack(allyFriendly, /*walkIfOutOfRange=*/true);

            Position const* nearestA = &allyFriendly;
            float nearestDistA = distAllyFriendly;
            if (distAllyMid < nearestDistA) { nearestA = &allyMiddle; nearestDistA = distAllyMid; }
            if (distMageA < nearestDistA) { nearestA = &attackPos; nearestDistA = distMageA; }
            return UseRocketPack(*nearestA, /*walkIfOutOfRange=*/true);
        }

        // On enemy ship: attack mage
        context->GetValue<std::string>("rti")->Set("skull");
        if (!bot->GetVehicle())
            Attack(boss);
        return false;
    }
    else
    {
        // Stranded at middle point: force jump back to friendly point
        if (isHorde)
        {
            float const distMiddle = bot->GetExactDist2d(middlePoint);
            float const distFriendly = bot->GetExactDist2d(waitPos);
            if (distMiddle <= 10.0f && distFriendly > 5.0f)
            {
                if (ExitCannonIfSeated())
                    return true;
                if (UseRocketPack(waitPos, /*walkIfOutOfRange=*/true))
                    return true;
                return MoveTo(bot->GetMapId(), waitPos.GetPositionX(), waitPos.GetPositionY(),
                              waitPos.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_NORMAL);
            }
        }

        // Return to friendly point if on enemy ship or drifted too far
        if (botOnEnemyShip || bot->GetExactDist2d(waitPos) > maxWaitingDistance)
        {
            if (ExitCannonIfSeated())
                return true;

            if (isHorde)
            {
                float const distMiddle = bot->GetExactDist2d(middlePoint);
                // Middle in range: jump middle (next tick stranded override jumps friendly)
                if (distMiddle <= JUMP_GATE)
                {
                    if (UseRocketPack(middlePoint, /*walkIfOutOfRange=*/false))
                        return true;
                    return MoveTo(bot->GetMapId(), middlePoint.GetPositionX(), middlePoint.GetPositionY(),
                                  middlePoint.GetPositionZ(), false, false, false, false,
                                  MovementPriority::MOVEMENT_NORMAL);
                }
                // Middle out of range: walk 10f toward middle
                if (UseRocketPack(middlePoint, /*walkIfOutOfRange=*/true))
                    return true;
                return MoveTo(bot->GetMapId(), middlePoint.GetPositionX(), middlePoint.GetPositionY(),
                              middlePoint.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_NORMAL);
            }

            // Alliance return: ALLY_MIDDLE_POINT -> ALLY_FRIENDLY_POINT
            Position const& allyFriendly = ICC_GUNSHIP_ROCKET_JUMP_ALLY_FRIENDLY_POINT;
            Position const& allyMiddle = ICC_GUNSHIP_ROCKET_JUMP_ALLY_MIDDLE_POINT;
            float const distAllyMid = bot->GetExactDist2d(allyMiddle);
            float const distAllyFriendly = bot->GetExactDist2d(allyFriendly);

            if (distAllyMid <= 10.0f && distAllyFriendly > 5.0f)
            {
                if (UseRocketPack(allyFriendly, /*walkIfOutOfRange=*/true))
                    return true;
                return MoveTo(bot->GetMapId(), allyFriendly.GetPositionX(), allyFriendly.GetPositionY(),
                              allyFriendly.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_NORMAL);
            }
            if (distAllyMid <= JUMP_GATE)
            {
                if (UseRocketPack(allyMiddle, /*walkIfOutOfRange=*/false))
                    return true;
                return MoveTo(bot->GetMapId(), allyMiddle.GetPositionX(), allyMiddle.GetPositionY(),
                              allyMiddle.GetPositionZ(), false, false, false, false,
                              MovementPriority::MOVEMENT_NORMAL);
            }
            if (UseRocketPack(allyMiddle, /*walkIfOutOfRange=*/true))
                return true;
            return MoveTo(bot->GetMapId(), allyMiddle.GetPositionX(), allyMiddle.GetPositionY(),
                          allyMiddle.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_NORMAL);
        }

        // Melee DPS on friendly ship: assist the assist tank on adds it's tanking
        if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot))
        {
            if (Group* group = bot->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (!member || !member->IsAlive())
                        continue;
                    if (!PlayerbotAI::IsAssistTank(member))
                        continue;
                    Unit* assistTarget = member->GetVictim();
                    if (!assistTarget || !assistTarget->IsAlive())
                        break;
                    if (assistTarget->GetExactDist2d(waitPos) > HOLD_RADIUS)
                        break;
                    bot->SetTarget(assistTarget->GetGUID());
                    bot->SetFacingToObject(assistTarget);
                    Attack(assistTarget);
                    return false;
                }
            }
        }

        // Ranged DPS star-mark axethrower/rifleman for weapon focus fire.
        // Handoff uses the previous star's spot as anchor so the marker
        // doesn't jump to whichever add is closest to the picking bot.
        if (PlayerbotAI::IsRangedDps(bot))
        {
            static constexpr uint8 STAR_ICON_INDEX = RtiTargetValue::starIndex;
            static constexpr float ADD_SEARCH_RANGE = 200.0f;
            uint32 const addEntry = (side == GunshipSide::ALLY)
                ? NPC_KOR_KRON_AXETHROWER : NPC_SKYBREAKER_RIFLEMAN;

            Position& lastStarPos = IcecrownHelpers::IccState(bot->GetInstanceId()).gsbLastStarPos;

            if (Group* group = bot->GetGroup())
            {
                Unit* starTarget = botAI->GetUnit(group->GetTargetIcon(STAR_ICON_INDEX));
                bool const validStar = starTarget && starTarget->IsAlive()
                    && starTarget->GetEntry() == addEntry;

                if (validStar)
                {
                    lastStarPos = Position(starTarget->GetPositionX(),
                                           starTarget->GetPositionY(),
                                           starTarget->GetPositionZ());
                }
                else
                {
                    Position const refPos = (lastStarPos.GetPositionX() != 0.0f || lastStarPos.GetPositionY() != 0.0f)
                        ? lastStarPos
                        : Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

                    std::list<Creature*> adds;
                    bot->GetCreatureListWithEntryInGrid(adds, addEntry, ADD_SEARCH_RANGE);

                    Unit* nextAdd = nullptr;
                    float bestDist = FLT_MAX;
                    for (Creature* c : adds)
                    {
                        if (!c || !c->IsAlive() || !c->IsHostileTo(bot))
                            continue;
                        float const d = c->GetExactDist(&refPos);
                        if (d < bestDist)
                        {
                            bestDist = d;
                            nextAdd = c;
                        }
                    }

                    if (nextAdd)
                    {
                        group->SetTargetIcon(STAR_ICON_INDEX, bot->GetGUID(), nextAdd->GetGUID());
                        lastStarPos = Position(nextAdd->GetPositionX(),
                                               nextAdd->GetPositionY(),
                                               nextAdd->GetPositionZ());
                        starTarget = nextAdd;
                    }
                    else
                    {
                        starTarget = nullptr;
                    }
                }

                if (starTarget)
                {
                    context->GetValue<std::string>("rti")->Set("star");
                    if (!bot->GetVehicle())
                        Attack(starTarget);
                }
            }
        }
    }

    return false;
}

bool IccGunshipRocketJumpAction::IsMainTankOnEnemyShip(GunshipSide side) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 const captainEntry = (side == GunshipSide::ALLY) ? NPC_HIGH_OVERLORD_SAURFANG : NPC_MURADIN_BRONZEBEARD;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (!PlayerbotAI::IsMainTank(member))
            continue;
        Unit* captain = member->FindNearestCreature(captainEntry, 15.0f);
        return captain != nullptr;
    }
    return false;
}

bool IccGunshipRocketJumpAction::AnyNonTankAwayFromFriendly(GunshipSide side) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Position const& waitPos = (side == GunshipSide::ALLY) ? ICC_GUNSHIP_ROCKET_JUMP_ALLY2 : ICC_GUNSHIP_ROCKET_JUMP_HORDE2;
    float const threshold = (side == GunshipSide::ALLY) ? 40.0f : 45.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        if (PlayerbotAI::IsMainTank(member) || PlayerbotAI::IsAssistTank(member))
            continue;
        if (member->GetExactDist2d(waitPos) > threshold)
            return true;
    }
    return false;
}

bool IccGunshipRocketJumpAction::ExitCannonIfSeated()
{
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !seat->CanEnterOrExit())
        return false;

    WorldPacket packet;
    bot->GetSession()->HandleRequestVehicleExit(packet);
    return true;
}

Item* IccGunshipRocketJumpAction::FindRocketPack() const
{
    return bot->GetItemByEntry(ITEM_GOBLIN_ROCKET_PACK);
}

bool IccGunshipRocketJumpAction::UseRocketPack(Position const& destination, bool walkIfOutOfRange)
{
    Item* rocketPack = FindRocketPack();
    if (!rocketPack)
        return false;

    if (!bot->HasAura(SPELL_ROCKET_PACK_USEABLE))
        return false;

    if (bot->IsNonMeleeSpellCast(true))
        return false;

    if (bot->CanUseItem(rocketPack) != EQUIP_ERR_OK)
        return false;

    // Throttle rocket pack use to prevent mid-air re-jump spam
    static constexpr uint32 ROCKET_PACK_COOLDOWN_MS = 2500;
    auto& lastRocketPackUse = IcecrownHelpers::IccState(bot->GetInstanceId()).gsbLastRocketPackUse;
    uint32 const now = getMSTime();
    auto const it = lastRocketPackUse.find(bot->GetGUID());
    bool const onCooldown = it != lastRocketPackUse.end() &&
                            getMSTimeDiff(it->second, now) < ROCKET_PACK_COOLDOWN_MS;

    static constexpr float MAX_JUMP_RANGE = 30.0f;
    float const dist = bot->GetExactDist(&destination);

    // On cooldown: consume tick to block MoveTo fallback when target is reachable
    if (onCooldown)
    {
        if (dist <= MAX_JUMP_RANGE)
            return true;
        return false;
    }
    if (dist > MAX_JUMP_RANGE)
    {
        if (!walkIfOutOfRange)
            return false;

        // Walk 10f toward destination, retry jump next tick
        static constexpr float STEP_DISTANCE = 10.0f;
        float const ratio = STEP_DISTANCE / dist;
        float const stepX = bot->GetPositionX() + (destination.GetPositionX() - bot->GetPositionX()) * ratio;
        float const stepY = bot->GetPositionY() + (destination.GetPositionY() - bot->GetPositionY()) * ratio;
        float const stepZ = bot->GetPositionZ() + (destination.GetPositionZ() - bot->GetPositionZ()) * ratio;
        return MoveTo(bot->GetMapId(), stepX, stepY, stepZ, false, false, false, false,
                      MovementPriority::MOVEMENT_NORMAL);
    }

    // Settle bot before firing rocket pack - moving bots tend to misfire mid-stride
    if (bot->isMoving())
    {
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        return true;
    }

    uint8 bagIndex = rocketPack->GetBagSlot();
    uint8 slot = rocketPack->GetSlot();
    uint8 castCount = 1;
    uint32 spellId = SPELL_ROCKET_PACK_USE;
    ObjectGuid itemGuid = rocketPack->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << castCount << spellId << itemGuid << glyphIndex << castFlags;
    packet << uint32(TARGET_FLAG_DEST_LOCATION);
    packet.appendPackGUID(0);
    packet << destination.GetPositionX() << destination.GetPositionY() << destination.GetPositionZ();

    bot->GetSession()->HandleUseItemOpcode(packet);
    lastRocketPackUse[bot->GetGUID()] = now;
    return true;
}

bool IccGunshipRocketJumpAction::RocketPackJumpToward(Position const& target)
{
    static constexpr float JUMP_DISTANCE = 30.0f;
    float const dist = bot->GetExactDist(&target);
    if (dist <= JUMP_DISTANCE)
        return UseRocketPack(target, /*walkIfOutOfRange=*/false);

    float const ratio = JUMP_DISTANCE / dist;
    Position jumpTo(
        bot->GetPositionX() + (target.GetPositionX() - bot->GetPositionX()) * ratio,
        bot->GetPositionY() + (target.GetPositionY() - bot->GetPositionY()) * ratio,
        bot->GetPositionZ() + (target.GetPositionZ() - bot->GetPositionZ()) * ratio);
    return UseRocketPack(jumpTo, /*walkIfOutOfRange=*/false);
}

Unit* IccGunshipRocketJumpAction::FindNearestFriendlyCannon(GunshipSide side) const
{
    if (side == GunshipSide::NONE)
        return nullptr;

    uint32 const cannonEntry = (side == GunshipSide::ALLY) ? NPC_CANNONA : NPC_CANNONH;
    Unit* cannon = bot->FindNearestCreature(cannonEntry, 300.0f);
    if (cannon && cannon->IsFriendlyTo(bot))
        return cannon;
    return nullptr;
}

bool IccGunshipRocketJumpAction::CleanupSkullIcon(uint8 skullIconIndex)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid const currentSkullTarget = group->GetTargetIcon(skullIconIndex);
    if (currentSkullTarget.IsEmpty())
        return false;

    Unit* skullTarget = ObjectAccessor::GetUnit(*bot, currentSkullTarget);
    if (!skullTarget || !skullTarget->IsAlive())
        group->SetTargetIcon(skullIconIndex, bot->GetGUID(), ObjectGuid::Empty);

    return false;
}

bool IccGunshipRocketJumpAction::UpdateBossSkullIcon(Unit* boss, uint8 skullIconIndex)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (group->GetTargetIcon(skullIconIndex) != boss->GetGUID())
        group->SetTargetIcon(skullIconIndex, bot->GetGUID(), boss->GetGUID());

    return false;
}

bool IccGunshipRocketPackSetupAction::Execute(Event /*event*/)
{
    Item* rocketPack = FindRocketPack();
    if (!rocketPack)
        return AcquireRocketPack();
    if (!rocketPack->IsEquipped())
        return EquipRocketPack();
    return false;
}

Item* IccGunshipRocketPackSetupAction::FindRocketPack() const
{
    return bot->GetItemByEntry(ITEM_GOBLIN_ROCKET_PACK);
}

bool IccGunshipRocketPackSetupAction::AcquireRocketPack()
{
    static constexpr float ZAFOD_SEARCH_RADIUS = 150.0f;
    static constexpr uint32 ZAFOD_GOSSIP_MENU_ID = 10885;
    static constexpr uint32 ZAFOD_GOSSIP_OPTION = 0;

    Creature* zafod = bot->FindNearestCreature(NPC_ZAFOD_BOOMBOX, ZAFOD_SEARCH_RADIUS);
    if (!zafod)
        return false;

    if (bot->GetDistance(zafod) > INTERACTION_DISTANCE)
        return MoveTo(bot->GetMapId(), zafod->GetPositionX(), zafod->GetPositionY(), zafod->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_NORMAL);

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    bot->SetFacingToObject(zafod);

    WorldPacket gossipHello(CMSG_GOSSIP_HELLO);
    gossipHello << zafod->GetGUID();
    bot->GetSession()->HandleGossipHelloOpcode(gossipHello);

    WorldPacket gossipSelect(CMSG_GOSSIP_SELECT_OPTION);
    gossipSelect << zafod->GetGUID();
    gossipSelect << uint32(ZAFOD_GOSSIP_MENU_ID) << uint32(ZAFOD_GOSSIP_OPTION);
    bot->GetSession()->HandleGossipSelectOptionOpcode(gossipSelect);
    return true;
}

bool IccGunshipRocketPackSetupAction::EquipRocketPack()
{
    EquipAction* equipAction = dynamic_cast<EquipAction*>(botAI->GetAiObjectContext()->GetAction("equip"));
    if (!equipAction)
        return false;

    ItemIds ids;
    ids.insert(ITEM_GOBLIN_ROCKET_PACK);
    equipAction->EquipItems(ids);
    return true;
}
