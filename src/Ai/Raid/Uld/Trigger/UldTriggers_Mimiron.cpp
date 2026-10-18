/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Mimiron.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldEncounter_Mimiron.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>
#include <RtiTargetValue.h>

using namespace EncounterHelpers;

bool MimironResetEncounterStateTrigger::IsActive()
{
    // -1 is the DisperseDistanceValue default, so anything at or above zero was set by somebody.
    if (bot->GetMapId() != ULDUAR_MAP_ID || AI_VALUE(float, "disperse distance") < 0.0f)
        return false;

    // Same scan the phase 1 node does. Sight distance caps "possible targets" at 100 yd, so at any
    // other boss in here this finds nothing and the value goes back to default before the pull.
    for (auto const& guid : AI_VALUE(GuidVector, "possible targets"))
    {
        Unit* target = botAI->GetUnit(guid);
        if (!target || !target->IsAlive())
            continue;

        uint32 const entry = target->GetEntry();
        if (entry == NPC_LEVIATHAN_MKII || entry == NPC_VX001 || entry == NPC_AERIAL_COMMAND_UNIT)
            return false;
    }

    return true;
}

bool MimironShockBlastTrigger::IsActive()
{
    // By entry, not through "find target": that value walks the bot's own threat list, so it only ever
    // resolves a boss this bot is already on. A bot that has not damaged the mech has to dodge it too.
    Unit* boss = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);

    if (!boss || !boss->IsAlive())
        return false;

    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_SHOCK_BLAST))
        return false;

    if (PlayerbotAI::IsMelee(bot))
        return true;

    // Centre to centre: GetDistance2d takes off the MK II's 8 yd reach and the bot's 1.5 first.
    return bot->GetExactDist2d(boss) < ULDUAR_MIMIRON_SHOCK_BLAST_SAFE_DIST;
}

bool MimironPhase1PositioningTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* leviathanMkII = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (ObjectGuid const& guid : targets)
    {
        Unit* target = botAI->GetUnit(guid);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_LEVIATHAN_MKII)
            leviathanMkII = target;
        else if (target->GetEntry() == NPC_VX001)
            return false;
        else if (target->GetEntry() == NPC_AERIAL_COMMAND_UNIT)
            return false;
    }

    if (!leviathanMkII || !leviathanMkII->IsAlive())
        return false;

    // The movement layer resets "disperse distance" when it rebuilds a formation, so this re-asserts it.
    // Same accessor as the action, never a literal: if the two drift apart this fires every tick.
    return AI_VALUE(float, "disperse distance") != GetMimironPhase1DisperseDistance(botAI);
}

bool MimironP3Wx2LaserBarrageTrigger::IsActive()
{
    // By entry. VX-001 never calls DoZoneInCombat in phase 4, so "find target" leaves any bot that has
    // not damaged it blind to a cone that kills in a single tick.
    Unit* boss = GetUldUnitByEntry(botAI, NPC_VX001);

    if (!boss || !boss->IsAlive())
        return false;

    // Spinning Up is a channel on VX-001, never an aura: 63414 puts its effects on the DB Target and the
    // MK II. Only the barrage itself, 63274/63300, is an aura on VX-001.
    return GetMimironSpinningUpSeconds(boss) >= 0.0f ||
           boss->HasAura(SPELL_P3WX2_LASER_BARRAGE_AURA_1) ||
           boss->HasAura(SPELL_P3WX2_LASER_BARRAGE_AURA_2);
}

bool MimironArcSpreadTrigger::IsActive()
{
    // No "is a mech up" gate of its own. GetMimironSpreadSlot answers false when neither a live nor a
    // staging focus resolves, and that is also what keeps this quiet before the pull and after a wipe.
    Position slot;
    if (!GetMimironSpreadSlot(botAI, bot, slot))
        return false;

    // First, because it is the cheap one and most bots are standing on their slot: everything below
    // gathers the fire field.
    if (bot->GetExactDist2d(slot.GetPositionX(), slot.GetPositionY()) <= ULDUAR_MIMIRON_SPREAD_TOLERANCE)
        return false;

    // Walks during a barrage only inside the safe sector: it's under half a turn wide, so two clear
    // endpoints mean a clear leg. The leg stays at MOVEMENT_COMBAT so a FORCED dodge can still cut it.
    if (Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001))
    {
        MimironBarrageWindow const window = GetMimironBarrageWindow(bot, vx001);
        if (window.valid)
        {
            float const speed = bot->GetSpeed(MOVE_RUN);
            float const travel =
                speed > 0.0f ? bot->GetExactDist2d(slot.GetPositionX(), slot.GetPositionY()) / speed
                             : 0.0f;

            if (!IsMimironSpotBarrageSafe(vx001, window, bot->GetPosition(), 0.0f) ||
                !IsMimironSpotBarrageSafe(vx001, window, slot, travel))
                return false;
        }
    }

    // Screens the slot and the walk, not the bot: a bot that dodged a rocket is clear while its slot may
    // still have the marker. The tank anchor is exempt, the MK II parks it on its own mines.
    if (!IsMimironTankAnchorSlot(botAI, bot) &&
        GetMimironSlotApproaches(botAI, bot, slot, GetMimironFirefighterHazards(botAI)).empty())
        return false;

    // Never walk a bot into a live Rapid Burst: wait the 3 s out wherever it stands.
    if (Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001))
    {
        MimironRapidBurstWindow const burst = GetMimironRapidBurstWindow(botAI, bot, vx001);
        if (!IsMimironSpotRapidBurstSafe(vx001, burst, slot))
            return false;
    }

    return true;
}

bool MimironAerialCommandUnitTrigger::IsActive()
{
    // Phase 3 only: the Aerial Command Unit is up on its own.
    Unit* aerialCommandUnit = GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT);
    if (!aerialCommandUnit || GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) ||
        GetUldUnitByEntry(botAI, NPC_VX001))
        return false;

    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        Unit* assaultBot = GetUldUnitByEntry(botAI, NPC_ASSAULT_BOT);
        Unit* focus = assaultBot ? assaultBot : aerialCommandUnit;

        // Purely so the icon tracks what the raid is actually killing; no bot reads it back.
        return group->GetTargetIcon(RtiTargetValue::skullIndex) != focus->GetGUID();
    }

    // Nothing for anyone else: the phase 3 slots already sit ULDUAR_MIMIRON_PHASE3_SPACING apart, past a
    // Bomb Bot's 5 yd blast.
    return false;
}

bool MimironRocketStrikeTrigger::IsActive()
{
    Unit* boss = GetUldUnitByEntry(botAI, NPC_VX001);

    if (!boss || !boss->IsAlive())
        return false;

    Creature* rocketStrikeN = bot->FindNearestCreature(NPC_ROCKET_STRIKE_N, ULDUAR_MIMIRON_ROOM_SEARCH_RANGE);

    if (!rocketStrikeN)
        return false;

    // Centre to centre against the 3 yd blast: GetDistance2d would take off both object sizes first.
    return bot->GetExactDist2d(rocketStrikeN) <= ULDUAR_MIMIRON_ROCKET_RUN_RADIUS;
}

bool MimironPhase4FocusTrigger::IsActive()
{
    if (!IsMimironPhase4(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
    {
        // Only to switch. Claiming the tick while already on it would starve the taunt.
        Unit* const focus = GetMimironPhase4TankFocus(botAI, bot);
        return focus && AI_VALUE(Unit*, "current target") != focus;
    }

    // Non-tanks only come here to be spread for Hand Pulse; their target belongs to
    // "mimiron set dps priority".
    return AI_VALUE(float, "disperse distance") != ULDUAR_MIMIRON_PHASE4_DISPERSE_DISTANCE;
}

bool MimironProximityMineTrigger::IsActive()
{
    // Melee stand in the mine field, so they take the hits while a mech is up. Between phases they dodge:
    // the mines outlive the MK II, and following the master walks melee across them.
    if (PlayerbotAI::IsMelee(bot) && GetMimironRingFocus(botAI))
        return false;

    // Off during a barrage: the barrage dodge hands a clear bot's tick down, and this node can't see the cone.
    MimironP3Wx2LaserBarrageTrigger barrage(botAI);
    if (barrage.IsActive())
        return false;

    TooCloseToCreatureTrigger tooCloseToProximityMine(botAI);
    return tooCloseToProximityMine.TooCloseToCreature(NPC_PROXIMITY_MINE,
                                                     ULDUAR_MIMIRON_MINE_TRIGGER_RADIUS);
}

bool MimironBombBotTrigger::IsActive()
{
    // The ranged DPS a Bomb Bot is chasing can't outrun it (8.0 yd/s against 7.0), so it stays and shoots.
    // Everyone else steps out of the 5 yd blast.
    if (PlayerbotAI::IsRangedDps(bot) && GetMimironBombBotChasing(botAI, bot))
        return false;

    TooCloseToCreatureTrigger tooCloseToBombBot(botAI);
    return tooCloseToBombBot.TooCloseToCreature(NPC_BOMB_BOT, ULDUAR_MIMIRON_BOMB_BOT_RADIUS);
}

bool MimironApproachTargetTrigger::IsActive()
{
    if (!IsMimironHardModeActive(botAI) || !IsMimironEngaged(botAI))
        return false;

    // The barrage dodge owns positioning for the window, and this node's spots never screen the cone.
    MimironP3Wx2LaserBarrageTrigger barrage(botAI);
    if (barrage.IsActive())
        return false;

    // No live target, nothing to close on. Cheap test first, ahead of the hazard scan below.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return false;

    // The formation places ranged and healers, in range of the boss it is built around. Closing on
    // some other target from there only has the formation walk the bot straight back.
    Position slot;
    if (GetMimironSpreadSlot(botAI, bot, slot))
        return false;

    Position approach;
    return GetMimironTargetApproach(botAI, bot, target, GetMimironApproachRange(botAI, bot),
                                    approach);
}

bool MimironDodgeFlamesTrigger::IsActive()
{
    if (!IsMimironHardModeActive(botAI))
        return false;

    // Stand down for anything that kills outright. All issue at MOVEMENT_FORCED and equal priority blocks,
    // so the fire dodge, which runs far more often, would hold the lock over the escapes.
    MimironShockBlastTrigger shockBlast(botAI);
    if (shockBlast.IsActive())
        return false;

    MimironRocketStrikeTrigger rocketStrike(botAI);
    if (rocketStrike.IsActive())
        return false;

    MimironFrostBombTrigger frostBomb(botAI);
    if (frostBomb.IsActive())
        return false;

    // Only for bots inside the swept band: the barrage dodge takes those and screens every hop. Everyone
    // else keeps dodging fire through the window.
    if (Unit* vx001 = GetUldUnitByEntry(botAI, NPC_VX001))
    {
        MimironBarrageWindow const window = GetMimironBarrageWindow(bot, vx001);
        if (window.valid && !IsMimironSpotBarrageSafe(vx001, window, bot->GetPosition(), 0.0f))
            return false;
    }

    // Off the pass's shared hazard read, which the fire bot, formation and approach triggers ask for too.
    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);

    // Water Spray one-shots, and a fire leg would hold the lock over its escape, so the line comes first.
    if (IsMimironSpotInFireBotSpray(hazards, bot->GetPosition(), ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH))
        return false;

    uint32 nodes = 0;
    for (Position const& node : hazards.flames)
        if (bot->GetExactDist2d(node.GetPositionX(), node.GetPositionY()) < ULDUAR_MIMIRON_FLAMES_RADIUS)
            ++nodes;

    if (!nodes)
        return false;

    bool const lowHealth = bot->GetHealthPct() < ULDUAR_MIMIRON_FLAMES_DODGE_HEALTH_PCT;

    // The core carrier walks through fire to a corpse unless low on health: the corpse lasts only 25 s, and
    // each dodge would undo the walk.
    if (IsMimironCoreFetchDue(botAI, bot))
        return lowHealth;

    // One node (~3.1k a tick) leaves a healthy bot seven ticks, so it stays put. Two halve that to about four
    // seconds, too short to notice and walk out, so they dodge at any health.
    return nodes >= ULDUAR_MIMIRON_FLAMES_DODGE_NODE_OVERRIDE || lowHealth;
}

bool MimironFireBotTrigger::IsActive()
{
    if (!IsMimironHardModeActive(botAI))
        return false;

    // A fire bot that outlived phase 3 can meet the barrage, and that cone kills outright.
    MimironP3Wx2LaserBarrageTrigger barrage(botAI);
    if (barrage.IsActive())
        return false;

    MimironFirefighterHazards const hazards = GetMimironFirefighterHazards(botAI);
    if (IsMimironSpotFireBotSafe(bot, hazards, bot->GetPosition(), ULDUAR_MIMIRON_FIREBOT_SIREN_CLEARANCE,
                                 ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH))
        return false;

    if (IsMimironSpotInFireBotSpray(hazards, bot->GetPosition(), ULDUAR_MIMIRON_FIREBOT_SPRAY_HALF_WIDTH))
        return true;

    // Only the siren left. A bot already walking somewhere clear of it is leaving anyway, and a sidestep
    // would cancel that walk. Walking through a silence costs nothing.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (bot->isMoving() && bot->GetMotionMaster()->GetDestination(x, y, z) &&
        IsMimironSpotFireBotSafe(bot, hazards, Position(x, y, z), ULDUAR_MIMIRON_FIREBOT_SIREN_STAND,
                                 ULDUAR_MIMIRON_FIREBOT_SPRAY_STAND_HALF_WIDTH))
        return false;

    return true;
}

bool MimironFrostBombTrigger::IsActive()
{
    if (!IsMimironHardModeActive(botAI))
        return false;

    TooCloseToCreatureTrigger tooCloseToFrostBomb(botAI);
    return tooCloseToFrostBomb.TooCloseToCreature(NPC_FROST_BOMB, ULDUAR_MIMIRON_FROST_BOMB_RADIUS);
}

bool MimironPlasmaBlastDefensiveTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot))
        return false;

    // Phase 1 only. Nothing later casts it: the cannon is part of the MK II.
    Unit* leviathanMkII = GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII);
    if (!leviathanMkII || GetUldUnitByEntry(botAI, NPC_VX001) ||
        GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        return false;

    // While the cast is up, not between casts: the victim was resolved when it began, and the whole
    // point is having the button down before the first of the six ticks lands.
    Creature* cannon = bot->FindNearestCreature(NPC_LEVIATHAN_MKII_CANNON, ULDUAR_MIMIRON_ROOM_SEARCH_RANGE);
    if (!cannon)
        return false;

    Spell* spell = GetMimironPlasmaBlastCast(cannon);
    if (!spell)
        return false;

    Unit* victim = spell->m_targets.GetUnitTarget();
    return victim && victim->IsAlive() && (victim == bot || PlayerbotAI::IsHeal(bot));
}

bool MimironRedirectThreatTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    // Phase 1 only, and only while somebody else is the main tank.
    if (!GetUldUnitByEntry(botAI, NPC_LEVIATHAN_MKII) ||
        GetUldUnitByEntry(botAI, NPC_VX001) ||
        GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        return false;

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    return mainTank && mainTank != bot && mainTank->IsAlive();
}

bool MimironMagneticCoreTrigger::IsActive()
{
    // One bot per instance owns this, so two carriers cannot burn two cores on the same landing.
    if (GetMimironCoreCarrier(botAI) != bot)
        return false;

    if (!GetUldUnitByEntry(botAI, NPC_AERIAL_COMMAND_UNIT))
        return false;

    // Holding one, the action decides whether to bank a second, hold, walk or spend it.
    if (bot->HasItemCount(ITEM_MIMIRON_MAGNETIC_CORE, 1, false))
        return true;

    // Fetched whatever the unit is doing, since a corpse lasts only 25 s. Corpses lie anywhere, so the
    // search covers the room and the action does the walk.
    return GetMimironCoreCorpse(bot) != nullptr;
}

bool MimironPetControlTrigger::IsActive()
{
    if (!bot->GetGuardianPet())
        return false;

    if (IsMimironPhase4(bot))
        return true;

    // Not by MOVEMENTFLAG_HOVER: it survives the phase 3 defeat and the vehicle boarding.
    return IsMimironPhase3(botAI);
}

bool MimironFrostResistanceTrigger::IsActive()
{
    // Class first: the pick walks the group, and this runs for every bot in the instance.
    if (bot->getClass() != CLASS_PALADIN || botAI->HasAura("frost resistance aura", bot))
        return false;

    return GetMimironFrostResistancePaladin(botAI, bot) == bot;
}

bool MimironSlowBombBotTrigger::IsActive()
{
    std::string const spell = GetMimironBombBotSnare(bot);
    if (spell.empty())
        return false;

    // Reads the Bomb Bot "mimiron set dps priority" already handed out instead of picking one, so the two
    // never fight. Healers never get one, so they never snare.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || target->GetEntry() != NPC_BOMB_BOT)
        return false;

    // Below this it is already on top of somebody and the global is better spent on damage.
    if (GetMimironBombBotApproach(bot, target) < ULDUAR_MIMIRON_BOMB_BOT_SNARE_MIN_APPROACH)
        return false;

    return !botAI->HasAura(spell, target);
}

bool MimironSetDpsPriorityTrigger::IsActive()
{
    // Tanks hold what the tank nodes give them; this one owns "current target" for everybody else.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // Engaged, not present: the action calls Attack() directly, and MK II sits in the room from the
    // moment the raid walks in.
    if (IsMimironEngaged(botAI))
        return true;

    // Phase 4 handover: nothing is engaged, and leftover fire bots walk in spraying. A reset despawns them,
    // so a live one means this handover. Hard mode holds all over Ulduar, hence the room check first.
    if (!IsMimironHardModeActive(botAI) || !IsNearMimironRoom(bot))
        return false;

    return bot->FindNearestCreature(NPC_EMERGENCY_FIRE_BOT, ULDUAR_MIMIRON_STAGING_SEARCH_RANGE) != nullptr;
}
