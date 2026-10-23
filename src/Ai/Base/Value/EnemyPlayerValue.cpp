/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EnemyPlayerValue.h"
#include "BattlegroundSA.h"
#include "CombatManager.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Vehicle.h"

// Strand of the Ancients helpers. BattlegroundSA does not expose which team attacks (Attackers is private); same
// approach as SaGetAttackerTeam in BattleGroundTactics.cpp: the defender portals have the defenders' faction
static bool SaIsAttackerTeam(Battleground* bg, Player* bot)
{
    GameObject* portal = bg->GetBGObject(BG_SA_PORTAL_DEFFENDER_BLUE);
    if (!portal)
        return false;  // round not set up yet, do not guess

    uint32 const defenderFaction = portal->GetFaction();
    if (defenderFaction == BG_SA_Factions[TEAM_ALLIANCE])
        return bot->GetTeamId() == TEAM_HORDE;
    if (defenderFaction == BG_SA_Factions[TEAM_HORDE])
        return bot->GetTeamId() == TEAM_ALLIANCE;

    return false;
}

// a gate that is still standing within reach of the bot
static bool SaGateInRange(Battleground* bg, Player* bot, float range)
{
    static uint32 const gates[] = {BG_SA_GREEN_GATE, BG_SA_BLUE_GATE,   BG_SA_PURPLE_GATE,
                                   BG_SA_RED_GATE,   BG_SA_YELLOW_GATE, BG_SA_ANCIENT_GATE};
    for (uint32 id : gates)
    {
        GameObject* gate = bg->GetBGObject(id);
        if (gate && gate->GetDestructibleState() != GO_DESTRUCTIBLE_DESTROYED && bot->IsWithinDist(gate, range))
            return true;
    }

    return false;
}

// a gate that is still standing (or the relic) within reach of this unit: the enemy demolisher is hitting it
static bool SaSiegeTargetNear(Battleground* bg, Unit* unit, float range)
{
    static uint32 const gates[] = {BG_SA_GREEN_GATE, BG_SA_BLUE_GATE,   BG_SA_PURPLE_GATE,
                                   BG_SA_RED_GATE,   BG_SA_YELLOW_GATE, BG_SA_ANCIENT_GATE};
    for (uint32 id : gates)
    {
        GameObject* gate = bg->GetBGObject(id);
        if (gate && gate->GetDestructibleState() != GO_DESTRUCTIBLE_DESTROYED && unit->IsWithinDist(gate, range))
            return true;
    }

    GameObject* relic = bg->GetBGObject(BG_SA_TITAN_RELIC);
    return relic && unit->IsWithinDist(relic, range);
}

// nearest enemy player that is attacking this unit directly
static Unit* SaNearestPlayerAttacking(Player* bot, Unit* victim)
{
    Unit* best = nullptr;
    for (Unit* attacker : victim->getAttackers())
    {
        if (!attacker || !attacker->IsPlayer() || !attacker->IsAlive() || !bot->IsHostileTo(attacker) ||
            !bot->CanSeeOrDetect(attacker))
            continue;

        if (!best || bot->GetDistance(attacker) < bot->GetDistance(best))
            best = attacker;
    }

    return best;
}

bool NearestEnemyPlayersValue::AcceptUnit(Unit* unit)
{
    // Apply parent's filtering first (includes level difference checks)
    if (!PossibleTargetsValue::AcceptUnit(unit))
        return false;

    bool inCannon = botAI->IsInVehicle(false, true);
    Player* enemy = dynamic_cast<Player*>(unit);
    if (enemy && botAI->IsOpposing(enemy) && enemy->IsPvP() &&
        !sPlayerbotAIConfig.IsPvpProhibited(enemy->GetZoneId(), enemy->GetAreaId()) &&
        !enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE_2) &&
        ((inCannon || !enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))) &&
        /*!enemy->HasStealthAura() && !enemy->HasInvisibilityAura()*/ (bot->InBattleground() ? bot->CanSeeOrDetect(enemy) : enemy->CanSeeOrDetect(bot)) &&
        !(enemy->HasSpiritOfRedemptionAura()))
    {
        // If with master, only attack if master is PvP flagged
        Player* master = botAI->GetMaster();
        if (master && !master->IsPvP() && !master->IsFFAPvP())
            return false;

        return true;
    }

    return false;
}

Unit* EnemyPlayerValue::Calculate()
{
    bool controllingCannon = false;
    bool controllingVehicle = false;
    bool passenger = false;
    if (Vehicle* vehicle = bot->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!seat || (!seat->CanControl() && !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_ATTACK)))
            return nullptr;  // a passenger seat that can't attack
        if (!seat->CanControl())
            passenger = true;  // fights from its seat: it can't close in
        else if (botAI->IsInVehicle(false, false, false, false, true))
            controllingCannon = true;
        else
            controllingVehicle = true;
    }

    // Strand of the Ancients: the demolishers have to leave the beach, so the attackers do not fight PvP for fun
    Battleground* saBg = bot->GetBattleground();
    bool const saAttacker = saBg && saBg->GetBgTypeID() == BATTLEGROUND_SA && SaIsAttackerTeam(saBg, bot);
    if (saAttacker)
    {
        Vehicle* myVehicle = bot->GetVehicle();
        Unit* myBase = myVehicle ? myVehicle->GetBase() : nullptr;
        if (myBase && myBase->GetEntry() == NPC_DEMOLISHER_SA)
        {
            // inside a demolisher (driver or passenger): break the gates, no PvP while there is a gate in reach.
            // Only an enemy hitting the demolisher itself, with no gate in reach, is answered.
            // TODO: 80 yards is a guess for "gate in reach"; the gate stage logic lives in BGTactics, not here
            if (SaGateInRange(saBg, bot, 80.0f))
                return nullptr;

            // (driver does not attack on foot) players on foot are never picked from a demolisher: only enemy
            // demolishers / towers, otherwise no target at all
            if (Unit* siege = FindEnemySiegeOnFoot())
                return siege;

            return nullptr;
        }
        else if (!myVehicle)
        {
            // on foot, not yet in a demolisher: protect the friendly demolishers against enemy players hitting them
            // TODO: boarding a demolisher is decided by EnterVehicleAction, not here
            if (Unit* vehicleHitter = FindPlayerAttackingFriendlyDemolisher())
                return vehicleHitter;
        }
    }

    // 1. Check units we are currently in PvP combat with.
    std::vector<Unit*> targets;
    Unit* pVictim = bot->GetVictim();
    for (auto const& [guid, combatRef] : bot->GetCombatManager().GetPvPCombatRefs())
    {
        Unit* pTarget = combatRef->GetOther(bot);
        if (!pTarget || pTarget == pVictim || !pTarget->IsPlayer() ||
            !(bot->InBattleground() ? bot->CanSeeOrDetect(pTarget) : pTarget->CanSeeOrDetect(bot)) ||
            !bot->IsWithinDist(pTarget, VISIBILITY_DISTANCE_NORMAL))
            continue;

        if ((bot->GetTeamId() == TEAM_HORDE && pTarget->HasAura(23333)) ||
            (bot->GetTeamId() == TEAM_ALLIANCE && pTarget->HasAura(23335)))
            return pTarget;

        targets.push_back(pTarget);
    }

    if (!targets.empty())
    {
        std::sort(targets.begin(), targets.end(),
                  [&](Unit const* pUnit1, Unit const* pUnit2)
                  { return bot->GetDistance(pUnit1) < bot->GetDistance(pUnit2); });

        return *targets.begin();
    }

    // 1b. SotA: attackers on foot go for the enemy anti-personnel towers / demolishers instead of idle PvP
    // TODO: attackers cannot man the towers (defender faction, EnterVehicleAction refuses them), so "use the enemy
    // tower" is not implemented; they hit it instead. Not confirmed that the attack action accepts a creature from
    // this value when the bot is on foot
    if (saAttacker && !bot->GetVehicle())
        if (Unit* siege = FindEnemySiegeOnFoot())
            return siege;

    // 1c. SotA defenders (on foot or in a tower): the enemy demolisher that is breaking a gate / the relic comes
    // before the players in range. TODO: 80 yards is a guess for "hitting the gate", same as above
    if (saBg && saBg->GetBgTypeID() == BATTLEGROUND_SA && !saAttacker)
        if (Unit* gateBreaker = FindEnemyDemolisher(true))
            return gateBreaker;

    // 1d. SotA tower (anti-personnel cannon): the "nearest enemy players" list stops at the grind distance, far less than
    // the 120 yards of the cannon, so players on foot out of that list were never picked. Search the whole cannon
    // range for the nearest enemy player (on foot or not) in line of sight.
    if (controllingCannon && saBg && saBg->GetBgTypeID() == BATTLEGROUND_SA && !saAttacker)
    {
        float const towerRange = GetMaxAttackDistance();
        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck towerCheck(bot, bot, towerRange);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> towerSearcher(bot, nearby, towerCheck);
        Cell::VisitObjects(bot, towerSearcher, towerRange);

        Unit* nearestPlayer = nullptr;
        for (Unit* unit : nearby)
        {
            if (!unit->IsPlayer() || !unit->IsAlive() || !bot->CanSeeOrDetect(unit) || !bot->IsWithinLOSInMap(unit) ||
                unit->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) || unit->ToPlayer()->HasSpiritOfRedemptionAura())
                continue;

            if (!nearestPlayer || bot->GetDistance(unit) < bot->GetDistance(nearestPlayer))
                nearestPlayer = unit;
        }

        if (nearestPlayer)
            return nearestPlayer;
    }

    // 2. Find enemy player in range.

    GuidVector players = AI_VALUE(GuidVector, "nearest enemy players");
    float const maxAggroDistance = GetMaxAttackDistance();
    for (auto const& gTarget : players)
    {
        Unit* pUnit = botAI->GetUnit(gTarget);
        if (!pUnit)
            continue;

        Player* pTarget = dynamic_cast<Player*>(pUnit);
        if (!pTarget)
            continue;

        if (pTarget == pVictim)
            continue;

        if (bot->GetTeamId() == TEAM_HORDE)
        {
            if (pTarget->HasAura(23333))
                return pTarget;
        }
        else
        {
            if (pTarget->HasAura(23335))
                return pTarget;
        }

        // Aggro weak enemies from further away.
        // If controlling mobile vehicle only agro close enemies (otherwise will never reach objective)
        uint32 const aggroDistance = passenger                                                        ? 30.0f
                                     : controllingVehicle                                             ? 5.0f
                                     : (controllingCannon || bot->GetHealth() > pTarget->GetHealth()) ? maxAggroDistance
                                                                                                      : 20.0f;
        if (!bot->IsWithinDist(pTarget, aggroDistance))
            continue;

        if (bot->IsWithinLOSInMap(pTarget) &&
            (controllingCannon || (fabs(bot->GetPositionZ() - pTarget->GetPositionZ()) < 30.0f)))
            return pTarget;
    }

    // 2b. SotA: a bot manning an anti-personnel tower shoots the enemy demolishers
    // TODO: whether the vehicle spell action takes this unit as "current target" is not confirmed; Rocket Blast
    // damage against demolishers is not confirmed either
    if (Unit* demolisher = FindEnemyDemolisher())
        return demolisher;

    // 3. Check party attackers.

    if (Group* pGroup = bot->GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Unit* pMember = itr->GetSource())
            {
                if (pMember == bot)
                    continue;

                if (ServerFacade::instance().GetDistance2d(bot, pMember) > 30.0f)
                    continue;

                if (Unit* pAttacker = pMember->getAttackerForHelper())
                    if (pAttacker->IsPlayer() && bot->IsWithinDist(pAttacker, maxAggroDistance * 2.0f) &&
                        bot->IsWithinLOSInMap(pAttacker) && pAttacker != pVictim &&
                        (bot->InBattleground() ? bot->CanSeeOrDetect(pAttacker) : pAttacker->CanSeeOrDetect(bot)))
                        return pAttacker;
            }
        }
    }

    return nullptr;
}

Unit* EnemyPlayerValue::FindEnemyDemolisher(bool requireGate)
{
    Battleground* bg = bot->GetBattleground();
    Vehicle* vehicle = bot->GetVehicle();
    bool onFoot = false;
    if (!bg || bg->GetBgTypeID() != BATTLEGROUND_SA)
        return nullptr;

    if (vehicle)
    {
        Unit* base = vehicle->GetBase();
        if (!base || base->GetEntry() != NPC_ANTI_PERSONNAL_CANNON || !botAI->IsInVehicle(false, true))
            return nullptr;
    }
    else
    {
        // on foot: only a defender (the round is set up) and only for a demolisher that is hitting a gate / the relic
        if (!bg->GetBGObject(BG_SA_PORTAL_DEFFENDER_BLUE) || SaIsAttackerTeam(bg, bot))
            return nullptr;

        requireGate = true;
        onFoot = true;
    }

    float range = GetMaxAttackDistance();
    if (onFoot)
        range = std::max(range, 30.0f);  // item O: the 30 yards must be searched
    std::list<Unit*> units;
    Acore::AnyUnfriendlyUnitInObjectRangeCheck check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, units, check);
    Cell::VisitObjects(bot, searcher, range);

    Unit* best = nullptr;
    for (Unit* unit : units)
    {
        if (unit->GetEntry() != NPC_DEMOLISHER_SA || !unit->IsAlive() || unit->IsFriendlyTo(bot) ||
            unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || !bot->IsWithinLOSInMap(unit))
            continue;

        // item O: a defender on foot also hits an enemy demolisher passing within 30 yards of him, even with no
        // standing gate / relic near it (or with the gate already down)
        if (requireGate && !SaSiegeTargetNear(bg, unit, 80.0f) && !(onFoot && bot->IsWithinDist(unit, 30.0f)))
            continue;

        if (!best || bot->GetDistance(unit) < bot->GetDistance(best))
            best = unit;
    }

    return best;
}

Unit* EnemyPlayerValue::FindPlayerAttackingFriendlyDemolisher()
{
    float const range = 40.0f;
    std::list<Unit*> units;
    Acore::AnyFriendlyUnitInObjectRangeCheck check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(bot, units, check);
    Cell::VisitObjects(bot, searcher, range);

    Unit* best = nullptr;
    for (Unit* unit : units)
    {
        if (unit->GetEntry() != NPC_DEMOLISHER_SA || !unit->IsAlive())
            continue;

        Unit* hitter = SaNearestPlayerAttacking(bot, unit);
        if (hitter && bot->IsWithinLOSInMap(hitter) && (!best || bot->GetDistance(hitter) < bot->GetDistance(best)))
            best = hitter;
    }

    return best;
}

Unit* EnemyPlayerValue::FindEnemySiegeOnFoot()
{
    float const range = 40.0f;
    std::list<Unit*> units;
    Acore::AnyUnfriendlyUnitInObjectRangeCheck check(bot, bot, range);
    Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(bot, units, check);
    Cell::VisitObjects(bot, searcher, range);

    Unit* best = nullptr;
    for (Unit* unit : units)
    {
        if ((unit->GetEntry() != NPC_DEMOLISHER_SA && unit->GetEntry() != NPC_ANTI_PERSONNAL_CANNON) ||
            !unit->IsAlive() || unit->IsFriendlyTo(bot) || unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
            !bot->IsWithinLOSInMap(unit))
            continue;

        if (!best || bot->GetDistance(unit) < bot->GetDistance(best))
            best = unit;
    }

    return best;
}

float EnemyPlayerValue::GetMaxAttackDistance()
{
    if (!bot->GetBattleground())
        return 60.0f;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return 40.0f;

    BattlegroundTypeId bgType = bg->GetBgTypeID();
    if (bgType == BATTLEGROUND_RB)
        bgType = bg->GetBgTypeID(true);

    if (bgType == BATTLEGROUND_IC || bgType == BATTLEGROUND_SA)
    {
        if (botAI->IsInVehicle(false, true))
            return 120.0f;
    }

    return 40.0f;
}
