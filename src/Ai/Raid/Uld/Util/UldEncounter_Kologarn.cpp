/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldEncounter_Kologarn.h"

#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include "UldScripts.h"
#include "UldTargets.h"
#include "Unit.h"

#include <algorithm>
#include <list>
#include <utility>

using namespace EncounterHelpers;

Unit* GetKologarn(PlayerbotAI* botAI) { return GetUldUnitByEntry(botAI, NPC_KOLOGARN); }

Unit* GetKologarnRightArm(PlayerbotAI* botAI) { return GetUldUnitByEntry(botAI, NPC_RIGHT_ARM); }

bool KologarnEncounterActive(PlayerbotAI* botAI)
{
    // The body is the only authority: the arms and the rubble are its summons, and boss_kologarn
    // never calls SetInCombatWithZone, so this flips exactly when someone engages him.
    Unit* kologarn = GetKologarn(botAI);
    return kologarn && kologarn->IsInCombat();
}

Unit* GetKologarnNearestRubble(PlayerbotAI* botAI, WorldObject const* from)
{
    if (!from)
        return nullptr;

    Unit* best = nullptr;
    float bestDistance = ULDUAR_KOLOGARN_ROOM_SEARCH_RADIUS;

    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RUBBLE)
            continue;

        float const distance = from->GetExactDist2d(unit);
        if (distance > bestDistance)
            continue;

        best = unit;
        bestDistance = distance;
    }

    return best;
}

bool KologarnHasRubble(PlayerbotAI* botAI)
{
    return GetUldUnitByEntry(botAI, NPC_RUBBLE) != nullptr;
}

Unit* GetKologarnLooseRubble(PlayerbotAI* botAI, Player* bot)
{
    if (!bot)
        return nullptr;

    Unit* nearest = nullptr;
    Unit* nearestLoose = nullptr;
    float nearestDistance = ULDUAR_KOLOGARN_ROOM_SEARCH_RADIUS;
    float nearestLooseDistance = ULDUAR_KOLOGARN_ROOM_SEARCH_RADIUS;

    for (ObjectGuid const& guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_RUBBLE)
            continue;

        float const distance = bot->GetExactDist2d(unit);
        if (distance < nearestDistance)
        {
            nearest = unit;
            nearestDistance = distance;
        }

        if (unit->GetVictim() != bot && distance < nearestLooseDistance)
        {
            nearestLoose = unit;
            nearestLooseDistance = distance;
        }
    }

    return nearestLoose ? nearestLoose : nearest;
}

uint8 GetKologarnCrunchArmorStacks(Unit const* unit)
{
    if (!unit)
        return 0;

    Aura* aura = unit->GetAura(SPELL_CRUNCH_ARMOR);
    if (!aura)
        aura = unit->GetAura(SPELL_CRUNCH_ARMOR_ALT);

    return aura ? aura->GetStackAmount() : 0;
}

bool IsKologarnStoneGripped(Unit const* unit)
{
    return unit && (unit->HasAura(SPELL_STONE_GRIP_10) || unit->HasAura(SPELL_STONE_GRIP_25));
}

Player* GetKologarnBodyTank(PlayerbotAI* botAI)
{
    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn)
        return nullptr;

    Unit* victim = kologarn->GetVictim();
    return victim ? victim->ToPlayer() : nullptr;
}

bool IsKologarnBodyTank(PlayerbotAI* botAI, Player* bot) { return bot && GetKologarnBodyTank(botAI) == bot; }

bool IsKologarnOffTank(PlayerbotAI* botAI, Player* bot)
{
    if (!bot || !bot->IsAlive())
        return false;

    if (!PlayerbotAI::IsMainTank(bot) && !PlayerbotAI::IsAssistTankOfIndex(bot, 0))
        return false;

    return !IsKologarnBodyTank(botAI, bot);
}

Unit* GetKologarnDpsTarget(PlayerbotAI* botAI, Player* bot)
{
    if (!bot)
        return nullptr;

    if (PlayerbotAI::IsRangedDps(bot))
    {
        if (Unit* rubble = GetKologarnNearestRubble(botAI, bot))
            return rubble;
    }

    if (Unit* rightArm = GetKologarnRightArm(botAI))
        return rightArm;

    return GetKologarn(botAI);
}

Unit* GetKologarnOffTankTarget(PlayerbotAI* botAI, Player* bot)
{
    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn || !bot)
        return nullptr;

    Unit* rightArm = GetKologarnRightArm(botAI);
    if (rightArm && bot->GetExactDist2d(kologarn) <= ULDUAR_KOLOGARN_TAUNT_RANGE)
        return rightArm;

    return kologarn;
}

Unit* GetKologarnNearestEyebeam(PlayerbotAI* botAI, Player* bot, float radius)
{
    if (!bot)
        return nullptr;

    Unit* best = nullptr;
    float bestDistance = radius;

    for (uint32 entry : {static_cast<uint32>(NPC_KOLOGARN_EYEBEAM_LEFT), static_cast<uint32>(NPC_KOLOGARN_EYEBEAM_RIGHT)})
    {
        std::list<Creature*> eyes;
        bot->GetCreatureListWithEntryInGrid(eyes, entry, radius);

        for (Creature* eye : eyes)
        {
            if (!eye->IsAlive())
                continue;

            float const distance = bot->GetExactDist2d(eye);
            if (distance > bestDistance)
                continue;

            best = eye;
            bestDistance = distance;
        }
    }

    return best;
}

Unit* GetKologarnEyebeamChasing(PlayerbotAI* botAI, Player* bot)
{
    if (!bot)
        return nullptr;

    // The eye MoveChases its summoner, so its victim is the spell target. It can still be closing from
    // past the react radius, so this sweeps the whole room.
    Unit* eye = GetKologarnNearestEyebeam(botAI, bot, ULDUAR_KOLOGARN_ROOM_SEARCH_RADIUS);
    return eye && eye->GetVictim() == bot ? eye : nullptr;
}

Position GetKologarnRubbleHoldSpot(PlayerbotAI* botAI, Unit* rubble)
{
    Player* bot = botAI->GetBot();
    Unit* kologarn = GetKologarn(botAI);

    // Rubble drop at the dead arm's side, so their own Y says which way to drag them out of the raid.
    float const raidY = kologarn ? kologarn->GetPositionY() : bot->GetPositionY();
    float const side = rubble && rubble->GetPositionY() >= raidY ? 1.0f : -1.0f;

    float const y = std::clamp(raidY + side * ULDUAR_KOLOGARN_RUBBLE_HOLD_OFFSET,
                               ULDUAR_KOLOGARN_WALKWAY_Y_MIN, ULDUAR_KOLOGARN_WALKWAY_Y_MAX);
    float const x = std::clamp(bot->GetPositionX(), ULDUAR_KOLOGARN_WALKWAY_X_MIN, ULDUAR_KOLOGARN_WALKWAY_X_MAX);

    return Position(x, y, ULDUAR_KOLOGARN_WALKWAY_Z, bot->GetOrientation());
}

Position GetKologarnEyebeamEscapeStep(Player* bot, Unit* eye)
{
    float const step = ULDUAR_KOLOGARN_EYEBEAM_RUN_STEP;
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    // Toward the entrance first, then along the walkway, and only then back toward the boss. X never
    // reaches the broken span, so even the last resort stays out of the pit.
    float const lateralSign = botY >= (ULDUAR_KOLOGARN_WALKWAY_Y_MIN + ULDUAR_KOLOGARN_WALKWAY_Y_MAX) / 2.0f
                                  ? 1.0f
                                  : -1.0f;
    std::pair<float, float> const candidates[] = {
        {-step, 0.0f}, {0.0f, lateralSign * step}, {0.0f, -lateralSign * step}, {step, 0.0f}};

    Position best(botX, botY, ULDUAR_KOLOGARN_WALKWAY_Z, bot->GetOrientation());
    float bestGain = 0.0f;

    for (auto const& [dx, dy] : candidates)
    {
        float const x = std::clamp(botX + dx, ULDUAR_KOLOGARN_WALKWAY_X_MIN, ULDUAR_KOLOGARN_WALKWAY_X_MAX);
        float const y = std::clamp(botY + dy, ULDUAR_KOLOGARN_WALKWAY_Y_MIN, ULDUAR_KOLOGARN_WALKWAY_Y_MAX);

        Position candidate(x, y, ULDUAR_KOLOGARN_WALKWAY_Z, bot->GetOrientation());

        // Clamped flat against a wall: no ground gained, so no distance from the beam.
        float const moved = candidate.GetExactDist2d(botX, botY);
        if (moved < ULDUAR_KOLOGARN_EYEBEAM_RADIUS)
            continue;

        float const gain = eye ? candidate.GetExactDist2d(eye) - bot->GetExactDist2d(eye) : moved;
        if (gain <= bestGain)
            continue;

        best = candidate;
        bestGain = gain;
    }

    return best;
}
