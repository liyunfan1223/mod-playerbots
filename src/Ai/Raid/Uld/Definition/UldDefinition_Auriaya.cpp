/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldDefinitions.h"

#include "Strategy.h"
#include "UldActions_Auriaya.h"
#include "UldEncounterGate.h"
#include "UldEncounter_Auriaya.h"
#include "UldMultipliers_Auriaya.h"
#include "UldTriggers_Auriaya.h"

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
void DefineAuriaya(EncounterBuilder& e)
{
    // No Sonic Screech dodge on purpose: it splits across the cone, so the anchors stack the raid in it.
    // Position goes last: the engine stops at the first success, and killing a Sentry beats standing on a spot.
    e.Node<AuriayaFallFromFloorTrigger, AuriayaFallFromFloorAction>(ACTION_RAID + 4);

    // A loose Sentry is the worst state in the fight, so the taunt outranks the pool dodge, which can wait.
    e.Node<AuriayaSentryTauntTrigger, AuriayaSentryTauntAction>(ACTION_RAID + 3);

    e.Node<AuriayaSeepingEssenceTrigger, AuriayaSeepingEssenceAction>(ACTION_RAID + 2, EncounterRow::Mover);
    e.Node<AuriayaAntiFearTrigger, AuriayaAntiFearAction>(ACTION_RAID + 2);
    e.Node<AuriayaSetDpsPriorityTrigger, AuriayaSetDpsPriorityAction>(ACTION_RAID + 1);
    e.Node<AuriayaRaidPositionTrigger, AuriayaRaidPositionAction>(ACTION_RAID, EncounterRow::Mover);

    // Engaged, not just present: on sight alone bots would sit rooted with no target before the pull.
    // "attack rti target" stays live on purpose, so a mark the player sets still wins.
    e.OwnTargeting("auriaya movement guard multiplier", Role::NonTank, IsAuriayaEngaged, Family::DpsAssist);

    // Anchored roles only, or a generic mover walks the bot off its spot and the anchor oscillates.
    // Keeps Attack, Reach and ReachHeal, so targeting works and healers reach whoever the anchor can't.
    e.OwnMovement("auriaya movement guard multiplier", Role::MainTank | Role::Ranged, IsAuriayaEngaged,
                  Family::Attack | Family::Reach | Family::ReachHeal);

    e.Multiplier<AuriayaAntiFearTotemGuardMultiplier>(Family::Spell);
}
}  // namespace

EncounterDefinition const& UldAuriayaDefinition()
{
    static EncounterDefinition const definition(ULD_BOSS_AURIAYA, &BossStateGateOpen, &DefineAuriaya);
    return definition;
}
