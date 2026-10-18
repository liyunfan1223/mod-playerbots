/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Vezax.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Spell.h"
#include "UldData.h"
#include "UldEncounter_Vezax.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "EncounterHelpers.h"
#include "RangeTriggers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>
#include <RtiTargetValue.h>

using namespace EncounterHelpers;

bool VezaxResetEncounterStateTrigger::IsActive()
{
    if (bot->GetMapId() != ULDUAR_MAP_ID)
        return false;

    // Out of combat, not just dead: he's back at full after a wipe, and a dead-boss test would carry every
    // slot into the next pull. Safe mid-fight, slots only get handed out while he's in combat.
    Unit* vezax = GetVezax(botAI);
    if (vezax && vezax->IsInCombat())
        return false;

    return VezaxHasEncounterState(bot);
}

bool VezaxMarkOfTheFacelessTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_MARK_OF_THE_FACELESS))
        return false;

    if (!VezaxEncounterActive(botAI))
        return false;

    Position spot;
    if (!TryGetVezaxMarkSpot(bot, spot))
        return false;

    return bot->GetExactDist2d(spot.GetPositionX(), spot.GetPositionY()) >
           ULDUAR_VEZAX_MARK_SPOT_TOLERANCE;
}

bool VezaxMarkOfTheFacelessBreakTrigger::IsActive()
{
    // Melee and the off-tank only. No mark in the ball reaches the camp at 27+ yd, and moving the camp
    // every 40s costs more cast time than it saves.
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    // Whoever holds the mark is the one person its leech skips, and it has its own node.
    if (bot->HasAura(SPELL_MARK_OF_THE_FACELESS))
        return false;

    if (!VezaxEncounterActive(botAI))
        return false;

    return GetVezaxMarkedAlly(bot) != nullptr;
}

bool VezaxShadowCrashDodgeTrigger::IsActive()
{
    if (!VezaxDodgesShadowCrash(bot))
        return false;

    if (!VezaxFormationActive(botAI))
        return false;

    Position impact;
    if (!TryGetVezaxShadowCrashImpact(botAI, impact))
        return false;

    return bot->GetExactDist2d(impact.GetPositionX(), impact.GetPositionY()) <=
           ULDUAR_VEZAX_SHADOW_CRASH_IMPACT_RADIUS + 1.0f;
}

bool VezaxSearingFlamesInterruptTrigger::IsActive()
{
    Unit* boss = GetVezax(botAI);
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    // Exact spell id, not "the boss is casting something": Vezax also casts Shadow Crash and Surge of
    // Darkness, and neither is worth an interrupt.
    Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id != SPELL_VEZAX_SEARING_FLAMES)
        return false;

    return VezaxIsSearingFlamesInterrupter(bot, boss);
}

bool VezaxSurgeOfDarknessTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = GetVezax(botAI);
    if (!boss || !boss->HasAura(SPELL_VEZAX_SURGE_OF_DARKNESS))
        return false;

    // Only the bot actually being hit for double physical damage spends a cooldown on it.
    return boss->GetVictim() == bot;
}

bool VezaxSaroniteAnimusTrigger::IsActive()
{
    Unit* animus = nullptr;
    if (IsVezaxHardModeActive(botAI) && VezaxEncounterActive(botAI))
        animus = GetUldUnitByEntry(botAI, NPC_VEZAX_SARONITE_ANIMUS);

    if (!animus)
    {
        tankHold.Reset();
        return false;
    }

    // Ahead of the target test: the dwell only counts if it's asked every tick.
    bool const tankHeld = TankHasHeldBoss(bot, animus, tankHold, ULDUAR_VEZAX_ANIMUS_TANK_LEAD_MS);

    // Vezax is invulnerable behind the Saronite Barrier until the Animus dies, so everyone
    // switches to it. Only fire when the bot is not already on it.
    if (AI_VALUE(Unit*, "current target") == animus)
        return false;

    // Tanks build the threat, and a hunter's or rogue's first hits go to the tank through the redirect.
    // On Vezax those hits would use the redirect up, so they can't wait with the rest.
    if (PlayerbotAI::IsTank(bot) || VezaxAnimusRedirectSpell(bot))
        return true;

    // No summon time means the hook never saw it, so nobody gets held on a guess.
    uint32 ageMs = 0;
    return tankHeld || !TryGetVezaxAnimusAge(botAI, ageMs) || ageMs >= ULDUAR_VEZAX_ANIMUS_HOLD_CAP_MS;
}

bool VezaxAnimusRedirectTrigger::IsActive()
{
    char const* redirect = VezaxAnimusRedirectSpell(bot);
    if (!redirect || !IsVezaxHardModeActive(botAI) || !VezaxEncounterActive(botAI))
        return false;

    // Not before the bot is on the Animus: the window opens on its next hit, and a last swing at Vezax
    // would open it there.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetEntry() != NPC_VEZAX_SARONITE_ANIMUS)
        return false;

    uint32 ageMs = 0;
    if (!TryGetVezaxAnimusAge(botAI, ageMs) || ageMs >= ULDUAR_VEZAX_ANIMUS_HOLD_CAP_MS)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || mainTank == bot)
        return false;

    // CanCastSpell passes an out of range cast as castable, so Tricks' reach has to be checked here.
    if (bot->getClass() == CLASS_ROGUE && bot->GetExactDist(mainTank) > ULDUAR_VEZAX_TRICKS_RANGE)
        return false;

    return botAI->CanCastSpell(redirect, mainTank);
}

bool VezaxAnimusBringBackTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || !IsVezaxAnimusOnBot(bot))
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    return mainTank && mainTank != bot &&
           bot->GetExactDist2d(mainTank) > ULDUAR_VEZAX_ANIMUS_BRING_BACK_RADIUS;
}

bool VezaxAnimusMeleeSpotTrigger::IsActive()
{
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || target->GetEntry() != NPC_VEZAX_SARONITE_ANIMUS || !VezaxAnimusPhaseActive(botAI))
        return false;

    // 3D, the same distance his target pick measures.
    Unit* vezax = GetVezax(botAI);
    return vezax && bot->GetExactDist(vezax) > VezaxMeleeHoldDistance(bot, vezax);
}

bool VezaxShadowCrashSoakTrigger::IsActive()
{
    // Cheap tests first: GatherVezaxHazards runs two grid searches.
    if (!VezaxCanSoakShadowCrashField(bot))
        return false;

    // Already inside one, so a move would only re-issue onto where the bot stands.
    if (bot->HasAura(SPELL_VEZAX_SHADOW_CRASH_FIELD))
        return false;

    if (!VezaxEncounterActive(botAI))
        return false;

    std::vector<VezaxHazard> hazards;
    GatherVezaxHazards(bot, hazards, ULDUAR_VEZAX_HAZARD_LOCAL_SEARCH_RADIUS);

    VezaxHazard field;
    if (!TryGetVezaxNearestHazard(bot, hazards, field))
        return false;

    return bot->GetExactDist2d(field.position.GetPositionX(), field.position.GetPositionY()) <=
           ULDUAR_VEZAX_SHADOW_CRASH_SOAK_MAX_TRAVEL;
}

bool VezaxHoldTargetTrigger::IsActive()
{
    if (!VezaxEncounterActive(botAI))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return true;

    // The animus row returns false once the bot is on it and falls through to here, so without this the
    // two trade the target every tick. Entry, not GetVezax: that's a creature lookup per bot per tick.
    return target->GetEntry() != NPC_VEZAX && target->GetEntry() != NPC_VEZAX_SARONITE_ANIMUS;
}

bool VezaxDamageHoldTrigger::IsActive()
{
    if (!IsVezaxHardModeActive(botAI))
        return false;

    if (!VezaxDamageHeld(botAI))
    {
        for (Unit* controlled : bot->m_Controlled)
        {
            Creature* pet = controlled->ToCreature();
            if (pet && pet->HasReactState(REACT_PASSIVE) && VezaxIsHushedPet(pet))
                return true;
        }

        return false;
    }

    Unit* vezax = GetVezax(botAI);
    if (!vezax)
        return false;

    if (bot->GetVictim() == vezax)
        return true;

    for (CurrentSpellTypes const type : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL})
        if (VezaxHarmfulCastOn(bot->GetCurrentSpell(type), vezax))
            return true;

    for (Unit* controlled : bot->m_Controlled)
    {
        Creature* pet = controlled->ToCreature();
        if (pet && !pet->IsTotem() && (!pet->HasReactState(REACT_PASSIVE) || pet->GetVictim() == vezax))
            return true;
    }

    return false;
}

bool VezaxRaidPositionTrigger::IsActive()
{
    // Whoever the Animus is hitting stays with the main tank until he has it back, or the slot would
    // walk it straight back out of his reach.
    if (!PlayerbotAI::IsTank(bot) && IsVezaxAnimusOnBot(bot))
        return false;

    return VezaxFormationActive(botAI);
}
