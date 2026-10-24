/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Ignis.h"

#include "GameObject.h"
#include "Group.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "UldEncounter_Ignis.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>

// Ignis the Furnace Master
static int32 GetIgnisBrittleTimeLeft(Unit* construct)
{
    if (!construct)
        return 0;

    if (Aura* brittle = construct->GetAura(SPELL_IGNIS_BRITTLE_10))
        return brittle->GetDuration();

    if (Aura* brittle = construct->GetAura(SPELL_IGNIS_BRITTLE_25))
        return brittle->GetDuration();

    return 0;
}

bool IgnisScorchedGroundTrigger::IsActive()
{
    if (!IsIgnisEngaged(botAI))
        return false;

    // Same grid lookup as the action. If the two disagree, the bot stays burning in a patch it's told to flee.
    Unit* patch = GetIgnisNearestScorchedGround(botAI, bot);

    return patch && bot->GetExactDist2d(patch) <= ULDUAR_IGNIS_SCORCHED_GROUND_AVOID_RADIUS;
}

bool IgnisMainTankPositionTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    // Only once he actually holds the boss. Ignis follows his victim, so a tank without aggro walking
    // to the anchor takes the raid's positioning with him and leaves the boss where it was.
    Unit* boss = GetEngagedIgnis(botAI);
    if (!boss || boss->GetVictim() != bot)
        return false;

    Position const spot = GetIgnisMainTankPosition(bot, boss);

    return bot->GetExactDist2d(&spot) > ULDUAR_IGNIS_TANK_SPOT_TOLERANCE;
}

bool IgnisConstructTankTrigger::IsActive()
{
    // Same answer GetIgnisConstructTankIndex gives a non-tank, for the price of a strategy bit. The
    // full index walks the group, so it waits until the fight is actually on.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!IsIgnisEngaged(botAI))
        return false;

    if (GetIgnisConstructTankIndex(bot) < 0)
        return false;

    // The second tank only ever finds a construct once there is one the first is not already walking,
    // so no extra gate is needed to keep him idle through the single-construct stretches.
    return GetIgnisDrivenConstruct(botAI, bot) != nullptr;
}

bool IgnisAttackBrittleConstructTrigger::IsActive()
{
    // Tanks stay on what they are holding: pulling the main tank off Ignis or a construct tank off a
    // construct whose threat table Molten already wiped costs far more than the one hit it takes.
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!IsIgnisEngaged(botAI))
        return false;

    Unit* construct = GetIgnisBrittleConstruct(botAI);
    if (!construct)
        return false;

    // Shatter deals 18850 in 13 yd, which is inside melee range of the thing they would be swinging
    // at. Melee are only let in at the end of the window, when nobody ranged has closed it.
    if (!PlayerbotAI::IsRanged(bot) &&
        GetIgnisBrittleTimeLeft(construct) > static_cast<int32>(ULDUAR_IGNIS_BRITTLE_MELEE_FALLBACK_MS))
        return false;

    // Stays hot once the target is already current: the action still has the designated burst spell
    // to fire, which is what actually reaches the 5000 / 3000 the shatter needs.
    return true;
}

bool IgnisAttackBossTrigger::IsActive()
{
    Unit* boss = GetEngagedIgnis(botAI);
    if (!boss)
        return false;

    // The generic pickers are off all fight, so this puts everyone on the boss, main tank included. A construct
    // tank drops out only while it holds one, between constructs it has nothing else to do.
    if (GetIgnisHeldConstruct(botAI, bot))
        return false;

    // Checked before the Brittle trigger since it settles most ticks on its own, and that trigger
    // scans every construct in the room.
    if (AI_VALUE(Unit*, "current target") == boss)
        return false;

    IgnisAttackBrittleConstructTrigger brittle(botAI);

    return !brittle.IsActive();
}

bool IgnisMoltenConstructAvoidTrigger::IsActive()
{
    Unit* boss = GetEngagedIgnis(botAI);
    if (!boss)
        return false;

    // The tank holding a construct stays in the pulse, that's his job. One between constructs clears it like
    // anyone else.
    if (GetIgnisHeldConstruct(botAI, bot))
        return false;

    // Ignis' own tank stays put too. He is melee-range of a boss that follows him, so running out of
    // a construct's aura drags Ignis (and his Flame Jets) straight through the raid behind him.
    if (boss->GetVictim() == bot)
        return false;

    Unit* molten = GetIgnisNearestMoltenConstruct(botAI, bot);

    return molten && bot->GetExactDist2d(molten) <= ULDUAR_IGNIS_MOLTEN_AVOID_RADIUS;
}

bool IgnisFlameJetsTrigger::IsActive()
{
    if (!bot->HasUnitState(UNIT_STATE_CASTING))
        return false;

    return GetIgnisIf(botAI, [](Creature const* ignis)
                      { return ignis->IsInCombat() && IsIgnisFlameJetsCasting(ignis); }) != nullptr;
}

bool IgnisSlagPotHealTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    if (!IsIgnisEngaged(botAI))
        return false;

    // Outranks every other heal, so wait for a gap, or the whole team tops off a full-health victim while the
    // tank takes Flame Jets unhealed.
    Player* victim = GetIgnisSlagPotVictim(botAI);

    return victim && victim->GetHealthPct() < ULDUAR_IGNIS_SLAG_POT_HEAL_HP_PCT;
}
