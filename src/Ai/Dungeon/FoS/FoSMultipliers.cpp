/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSMultipliers.h"
#include "ChooseTargetActions.h"
#include "FoSActions.h"
#include "FoSTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

float BronjahmMultiplier::GetValue(Action* action) {
    Unit* boss = AI_VALUE2(Unit *, "find target", "bronjahm");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (bot->HasAura(SPELL_CORRUPT_SOUL))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromBronjahmAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float AttackFragmentMultiplier::GetValue(Action* action)
{

    Unit* fragment = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            fragment = unit;
            break;
        }
    }

    if (fragment && PlayerbotAI::IsDps(bot) && dynamic_cast<BronjahmGroupPositionAction*>(action))
        return 0.0f;

    return 1.0f;
}

float DevourerOfSoulsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<DevourerWailingSoulsAction*>(action) || dynamic_cast<DevourerWellOfSoulsAction*>(action) ||
        dynamic_cast<DevourerStopDamageAction*>(action) || dynamic_cast<DevourerInterruptPhantomBlastAction*>(action) ||
        dynamic_cast<DevourerStopCastAction*>(action))
        return 1.0f;

    // In a well, leaving comes first: an attack in place cancelled the step out every tick and kept a rogue there 30 s
    if (FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS))
        return 0.0f;

    // Class interrupt triggers fire on any cast: keep them for Phantom Blast (a kicked Wailing Souls leaves the boss rooted)
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    if (cast && IsInterruptSpell(cast->getSpell()))
        return IsPhantomBlastCasting(boss) ? 1.0f : 0.0f;

    // Damage on the boss is mirrored onto the debuffed player: heal, buff and cure only
    if (IsMirroredSoulActive(bot, boss))
    {
        if (dynamic_cast<AttackAction*>(action))
            return 0.0f;

        // Totem drops count as buffs; the damaging ones would hit him the moment they land
        if (cast && IsDamagingTotemSpell(cast->getSpell()))
            return 0.0f;

        if (cast && !dynamic_cast<CastHealingSpellAction*>(action) && !dynamic_cast<CastBuffSpellAction*>(action) &&
            !dynamic_cast<CastCureSpellAction*>(action) && !dynamic_cast<CurePartyMemberAction*>(action))
            return 0.0f;
    }

    // Any other movement (chasing, tank positioning, reaching range) could walk a bot into the sweep
    if (IsWailingSoulsActive(boss) && dynamic_cast<MovementAction*>(action))
        return 0.0f;

    // Once out of a well, stay out: healer repositioning and melee chasing a boss who stands in one walked bots back in
    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action))
    {
        // Inside the ring the well action steps to (7.5 yd), so a bot that stepped out can still chase
        if (FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS + 1.0f))
            return 0.0f;

        if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot) &&
            FindWellOfSouls(bot, boss->GetPositionX(), boss->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS))
            return 0.0f;
    }

    return 1.0f;
}
