/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SethActions.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SethShared.h"
#include <algorithm>
#include <array>
#include <cmath>

using namespace SethShared;
using namespace EncounterHelpers;

bool TimeLostControllerMarkCharmingTotemWithSkullAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 40.0f;
    Unit* totem = bot->FindNearestCreature(Id(SethNpcs::NPC_CHARMING_TOTEM), searchRadius);
    return totem && MarkTargetWithSkull(bot, totem);
}

bool SethekkProphetSetTremorTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(SethSpells::SPELL_TREMOR_TOTEM), bot) &&
        botAI->CastSpell(Id(SethSpells::SPELL_TREMOR_TOTEM), bot);
}

bool DarkweaverSythMarkElementalsWithSkullAction::Execute(Event /*event*/)
{
    static constexpr std::array elementals = {
        "syth frost elemental",
        "syth shadow elemental",
        "syth arcane elemental",
        "syth fire elemental",
    };

    for (auto const& name : elementals)
    {
        if (Unit* elemental = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

bool AnzuAlternateMarksOnBossAction::Execute(Event /*event*/)
{
    Unit* anzu = AI_VALUE2(Unit*, "find target", "anzu");
    if (!anzu)
        return false;

    if (anzu->HasAura(Id(SethSpells::SPELL_BANISH_ANZU)))
        return MarkTargetWithMoon(bot, anzu);

    return MarkTargetWithSkull(bot, anzu);
}

// Priority: Falcon (haste) -> Hawk (damage reduction) -> Eagle (damage all enemies).
bool AnzuCastHealOverTimeSpellOnBirdSpiritAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 60.0f;
    Creature* targetSpirit = nullptr;

    static constexpr std::array spiritEntries = {
        Id(SethNpcs::NPC_FALCON_SPIRIT),
        Id(SethNpcs::NPC_HAWK_SPIRIT),
        Id(SethNpcs::NPC_EAGLE_SPIRIT),
    };

    for (uint32 entry : spiritEntries)
    {
        Creature* spirit = bot->FindNearestCreature(entry, searchRadius);
        if (spirit && !spirit->GetAuraEffect(
                SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, REJUVENATION_SPELL_ICON_ID, 0))
        {
            targetSpirit = spirit;
            break;
        }
    }

    if (!targetSpirit)
        return false;

    if (!botAI->CanCastSpell(Id(SethSpells::SPELL_REJUVENATION_RANK_1), targetSpirit))
        return false;

    return botAI->CastSpell(Id(SethSpells::SPELL_REJUVENATION_RANK_1), targetSpirit);
}

bool TalonKingIkissTankMoveBossToPillarPositionAction::Execute(Event /*event*/)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    if (ikiss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        _hasReachedPillarPosition = false;

    if (_hasReachedPillarPosition == true)
        return false;

    Position const& position = PILLAR_POSITION;
    constexpr float arrivalDist = 2.0f;

    if (bot->GetExactDist2d(position) <= arrivalDist)
    {
        _hasReachedPillarPosition = true;
        return false;
    }

    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(bot, position, arrivalDist, ikiss, moveX, moveY, backwards))
        return false;

    return MoveTo(
        SETH_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool TalonKingIkissRangedStayNearVictimOfBossAction::Execute(Event /*event*/)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss || !ikiss->GetVictim())
        return false;

    Player* victim = ikiss->GetVictim()->ToPlayer();
    if (!victim)
        return false;

    constexpr float targetDist = 10.0f;
    constexpr float tolerance = 5.0f;
    float const distToVictim = bot->GetExactDist2d(victim);
    if (distToVictim <= targetDist + tolerance)
        return false;

    float const angle = victim->GetAngle(bot);
    float const destX = victim->GetPositionX() + std::cos(angle) * targetDist;
    float const destY = victim->GetPositionY() + std::sin(angle) * targetDist;

    return MoveTo(
        SETH_MAP_ID, destX, destY, victim->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool TalonKingIkissLosArcaneExplosionAction::Execute(Event /*event*/)
{
    Position const& pillarCenter = PILLAR_CENTER;
    float const botAngle = pillarCenter.GetAngle(bot);
    float const distToPillar = bot->GetExactDist2d(pillarCenter);

    return MoveToPillar(pillarCenter, botAngle, distToPillar) ||
        MoveAroundPillar(pillarCenter, distToPillar);
}

bool TalonKingIkissLosArcaneExplosionAction::MoveToPillar(
    Position const& pillarCenter, float botAngle, float distToPillar)
{
    constexpr float minRadius = 10.0f;
    constexpr float maxRadius = 13.0f;

    if (distToPillar >= minRadius && distToPillar <= maxRadius)
        return false;

    float const targetRadius = distToPillar < minRadius ? 11.0f : 12.0f;
    float const moveX = pillarCenter.GetPositionX() + targetRadius * cos(botAngle);
    float const moveY = pillarCenter.GetPositionY() + targetRadius * sin(botAngle);

    bot->CastStop();
    return MoveTo(
        SETH_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TalonKingIkissLosArcaneExplosionAction::MoveAroundPillar(
    Position const& pillarCenter, float distToPillar)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    constexpr float angularStep = M_PI / 8.0f;
    constexpr float angularDeadzone = 0.105f;  // ~6°

    float const botAngle = pillarCenter.GetAngle(bot);
    float const targetAngle = pillarCenter.GetAngle(ikiss) + M_PI;
    float delta = Position::NormalizeOrientation(targetAngle - botAngle);
    if (delta > M_PI)
        delta -= 2.0f * M_PI;

    if (fabs(delta) < angularDeadzone)
        return false;

    float const angularDirection = (delta > 0.0f) ? 1.0f : -1.0f;
    float const stepAngle = botAngle + angularDirection * std::min(angularStep, fabs(delta));

    float const moveX = pillarCenter.GetPositionX() + distToPillar * cos(stepAngle);
    float const moveY = pillarCenter.GetPositionY() + distToPillar * sin(stepAngle);

    bot->CastStop();
    return MoveTo(
        SETH_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool TalonKingIkissMoveToWithinLosAction::Execute(Event /*event*/)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return false;

    Position const& pillarCenter = PILLAR_CENTER;
    constexpr float angularStep = M_PI / 8.0f;
    constexpr float angularDeadzone = 0.105f;  // ~6°

    float const botAngle = pillarCenter.GetAngle(bot);
    float const targetAngle = pillarCenter.GetAngle(ikiss);
    float const radius = bot->GetExactDist2d(pillarCenter);
    float delta = Position::NormalizeOrientation(targetAngle - botAngle);
    if (delta > M_PI)
        delta -= 2.0f * M_PI;

    if (fabs(delta) < angularDeadzone)
        return false;

    float const angularDirection = (delta > 0.0f) ? 1.0f : -1.0f;
    float const stepAngle = botAngle + angularDirection * std::min(angularStep, fabs(delta));

    float const moveX = pillarCenter.GetPositionX() + radius * cos(stepAngle);
    float const moveY = pillarCenter.GetPositionY() + radius * sin(stepAngle);

    return MoveTo(
        SETH_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}
