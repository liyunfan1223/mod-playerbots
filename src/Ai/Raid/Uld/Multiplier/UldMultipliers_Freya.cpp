/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldMultipliers_Freya.h"

#include "AttackAction.h"
#include "BurstCooldowns.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "UldActions.h"
#include "UldData.h"
#include "UldEncounter_Freya.h"
#include "UldHardMode.h"
#include "UldScripts.h"
#include "UldTriggers.h"
#include "VehicleActions.h"

#include <set>
#include <string>

using namespace EncounterHelpers;

// Freya
float FreyaTrioSyncMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot) || !PlayerbotAI::IsDps(bot))
        return 1.0f;

    // Movement and heals are never held back: a bot waiting out the floor still has to dodge a
    // Nature Bomb and reach a Healthy Spore.
    bool const isDamage = dynamic_cast<MeleeAction*>(action) ||
                          (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action));
    if (!isDamage)
        return 1.0f;

    // Only a trio member is ever held, so the target's entry settles it before the boss lookup and the
    // wave sweep, which every Ulduar fight would otherwise pay on each damage action.
    Unit* const target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    uint32 const entry = target->GetEntry();
    if (entry != NPC_SNAPLASHER && entry != NPC_STORM_LASHER && entry != NPC_ANCIENT_WATER_SPIRIT)
        return 1.0f;

    Unit* freya = GetFreyaScan(botAI).Boss();
    if (!freya || !freya->IsAlive())
        return 1.0f;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    return FreyaTrioSyncSuppress(state, target) ? 0.0f : 1.0f;
}

float FreyaLasherFinishAoeMultiplier::GetValue(Action* action)
{
    if (!action || action->getThreatType() != Action::ActionThreatType::Aoe)
        return 1.0f;

    // Healing AoE is never held: the finish is exactly when the raid is topping up between blasts.
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* freya = GetFreyaScan(botAI).Boss();
    if (!freya || !freya->IsAlive())
        return 1.0f;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    // A Detonate radius rather than the pack's own: a bot at the edge of the pile that keeps casting
    // Blizzard onto it blows the whole pack up at once, which is the thing this exists to stop.
    return GetFreyaFinishingPackNear(botAI, state, ULDUAR_FREYA_LASHER_PACK_CLEAR) ? 0.0f : 1.0f;
}

float FreyaLasherTrapReserveMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastExplosiveTrapAction*>(action) && !dynamic_cast<CastTrapLauncherExplosiveAction*>(action))
        return 1.0f;

    Unit* freya = GetFreyaScan(botAI).Boss();
    if (!freya || !freya->IsAlive())
        return 1.0f;

    FreyaWaveState state;
    GatherFreyaWaveState(botAI, state);

    return state.detonatingLashers.empty() ? 1.0f : 0.0f;
}

float FreyaGroundTremorCastGateMultiplier::GetValue(Action* action)
{
    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction || dynamic_cast<CastMeleeSpellAction*>(action) || bot->GetMapId() != ULDUAR_MAP_ID)
        return 1.0f;

    // The encounter's own hold action is what stops a cast already in flight; this only decides what
    // is allowed to start.
    if (action->getName() == FreyaGroundTremorHoldCastAction::Name)
        return 1.0f;

    uint32 const now = getMSTime();
    if (now != cachedAtMs || !cachedAtMs)
    {
        cachedAtMs = now;
        cachedRemainingMs = EvaluateWindow();
    }

    if (cachedRemainingMs <= 0)
        return 1.0f;

    uint32 const spellId = AI_VALUE2(uint32, "spell id", spellAction->getSpell());
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return 1.0f;

    // Anything that still lands before the tremor is worth starting, so the instants a locked-out bot
    // would fall through to are never held, only the casts the interrupt would eat.
    uint32 const castTime = spellInfo->CalcCastTime();
    if (castTime == 0 && !spellInfo->IsChanneled())
        return 1.0f;

    return static_cast<int32>(castTime) < cachedRemainingMs ? 1.0f : 0.0f;
}

int32 FreyaGroundTremorCastGateMultiplier::EvaluateWindow()
{
    // By entry rather than off the target list: this gate runs for every bot on the Ulduar map, so on
    // trash and in every other fight nothing else there wants that list built.
    Unit* boss = GetFreyaBossByEntry(botAI);
    if (!IsFreyaGroundTremorCasting(boss))
        return 0;

    Spell* tremor = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    return tremor ? tremor->GetCastTimeRemaining() : 0;
}
