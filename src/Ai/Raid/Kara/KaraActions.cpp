/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraActions.h"
#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <ctime>
#include <limits>
#include <list>
#include <map>
#include <string>

using namespace KaraHelpers;
using namespace EncounterHelpers;

// General

bool KarazhanResetEncounterStatesAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    bool const isMechanicTracker = IsMechanicTrackerBot(bot, KARA_MAP_ID);
    bool reset = false;

    if (isMechanicTracker)
    {
        if (!AI_VALUE2(Unit*, "find target", "midnight") &&
            attumenDpsWaitTimer.erase(instanceId) > 0)
        {
            reset = true;
        }

        if (!AI_VALUE2(Unit*, "find target", "nightbane"))
        {
            if (nightbaneDpsWaitTimer.erase(instanceId) > 0)
                reset = true;

            if (nightbaneFlightPhaseStartTimer.erase(instanceId) > 0)
                reset = true;
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "the big bad wolf"))
    {
        Action* wolfAction = context->GetAction("big bad wolf little red riding hood run away");
        if (wolfAction &&
            static_cast<BigBadWolfLittleRedRidingHoodRunAwayAction*>(wolfAction)->ResetRunIndex())
        {
            reset = true;
        }
    }

    if (!AI_VALUE2(Unit*, "find target", "netherspite"))
    {
        if (isMechanicTracker && netherspiteDpsWaitTimer.erase(instanceId) > 0)
            reset = true;

        Action* redAction = context->GetAction("netherspite block red beam");
        if (redAction &&
            static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
        {
            reset = true;
        }

        Action* blueAction = context->GetAction("netherspite block blue beam");
        if (blueAction &&
            static_cast<NetherspiteBlockBlueBeamAction*>(blueAction)->ResetBlueBeamState())
        {
            reset = true;
        }

        Action* greenAction = context->GetAction("netherspite block green beam");
        if (greenAction &&
            static_cast<NetherspiteBlockGreenBeamAction*>(greenAction)->ResetGreenBeamState())
        {
            reset = true;
        }

        if (currentRedBlocker.erase(instanceId) > 0)
            reset = true;

        if (currentGreenBlocker.erase(instanceId) > 0)
            reset = true;

        if (currentBlueBlocker.erase(instanceId) > 0)
            reset = true;
    }

    return reset;
}

bool KarazhanSetTremorTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(KaraSpells::SPELL_TREMOR_TOTEM), bot) &&
        botAI->CastSpell(Id(KaraSpells::SPELL_TREMOR_TOTEM), bot);
}

// Trash

bool ManaWarpStunCreatureBeforeWarpBreachAction::Execute(Event /*event*/)
{
    Unit* target = nullptr;
    constexpr float searchRadius = 40.0f;
    std::list<Creature*> manaWarps;
    bot->GetCreatureListWithEntryInGrid(manaWarps, Id(KaraNpcs::NPC_MANA_WARP), searchRadius);

    for (Creature* manaWarp : manaWarps)
    {
        if (!manaWarp || !manaWarp->IsAlive() || manaWarp->GetHealthPct() > 15.0f)
            continue;

        if (!target || manaWarp->GetGUID() < target->GetGUID())
            target = manaWarp;
    }

    if (!target)
        return false;

    static constexpr std::array spells = {
        "bash",
        "concussion blow",
        "hammer of justice",
        "kidney shot",
        "maim",
        "shadowfury",
        "shockwave",
    };

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target))
            return true;
    }

    return false;
}

// Attumen the Huntsman

// Midnight's CombatReach is 1.6 yards
// Unmounted Attumen's CombatReach is 1.5 yards
bool AttumenTheHuntsmanHandlePhaseOneAction::Execute(Event /*event*/)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return false;

    if (PlayerbotAI::IsAssistTank(bot))
    {
        Unit* attumen = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
        return attumen && AssistTankMoveAttumenFromGroup(midnight, attumen);
    }

    return AI_VALUE(Unit*, "current target") != midnight && Attack(midnight);
}

bool AttumenTheHuntsmanHandlePhaseOneAction::AssistTankMoveAttumenFromGroup(
    Unit* midnight, Unit* attumen)
{
    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() != bot || midnight->GetVictim() == bot)
        return false;

    constexpr float safeDistance = 8.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
    if (!nearestPlayer || attumen->GetDistance2d(nearestPlayer) >= safeDistance)
        return false;

    return MoveFromGroup(safeDistance);
}

bool AttumenTheHuntsmanHandlePhaseTwoAction::Execute(Event /*event*/)
{
    Unit* attumen = AI_VALUE2(Unit*, "find target", "attumen the huntsman");
    if (!attumen)
        return false;

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (PlayerbotAI::IsTank(bot))
        return CurrentTankPositionAttumen(attumen);

    return StackBehindAttumen(attumen);
}

bool AttumenTheHuntsmanHandlePhaseTwoAction::CurrentTankPositionAttumen(Unit* attumen)
{
    if (attumen->GetVictim() != bot || !bot->IsWithinMeleeRange(attumen))
        return false;

    Position const& position = ATTUMEN_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f || !bot->IsWithinLOS(
            position.GetPositionX(), position.GetPositionY(), position.GetPositionZ()))
    {
        return false;
    }

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    float const toPosX = posX - botX;
    float const toPosY = posY - botY;
    float const toBossX = attumen->GetPositionX() - botX;
    float const toBossY = attumen->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Mounted Attumen's CombatReach is 0 [sic] yards
bool AttumenTheHuntsmanHandlePhaseTwoAction::StackBehindAttumen(Unit* attumen)
{
    float const distanceBehind = bot->getClass() == CLASS_HUNTER? 8.0f : 2.0f;
    float const orientation = attumen->GetOrientation() + M_PI;
    float const rearX = attumen->GetPositionX() + std::cos(orientation) * distanceBehind;
    float const rearY = attumen->GetPositionY() + std::sin(orientation) * distanceBehind;

    if (bot->GetDistance2d(rearX, rearY) <= 0.2f)
        return false;

    return MoveTo(
        KARA_MAP_ID, rearX, rearY, attumen->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool AttumenTheHuntsmanSetDpsTimerAction::Execute(Event /*event*/)
{
    uint32 const instanceId = bot->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    return attumenDpsWaitTimer.try_emplace(instanceId, now).second;
}

// Moroes

bool MoroesMarkTargetAction::Execute(Event /*event*/)
{
    static constexpr std::array moroesGuests = {
        "baroness dorothea millstipe",
        "lady catriona von'indi",
        "lady keira berrybuck",
        "baron rafe dreuger",
        "lord robin daris",
        "lord crispin ference",
    };

    for (char const* name : moroesGuests)
    {
        if (Unit* guest = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, guest);
    }

    return false;
}

// Maiden of Virtue
// CombatReach is 4.8 yards

bool MaidenOfVirtueTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    if (!maiden)
        return false;

    if (AI_VALUE(Unit*, "current target") != maiden)
        return Attack(maiden);

    if (maiden->GetVictim() != bot)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* healer = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsHeal(member) ||
            !member->HasAura(Id(KaraSpells::SPELL_REPENTANCE)))
        {
            continue;
        }

        healer = member;
        break;
    }

    if (healer)
        return MoveBossToStunnedHealer(healer);

    if (!bot->IsWithinMeleeRange(maiden))
        return false;

    Position const& position = MAIDEN_OF_VIRTUE_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = posX - botX;
    float const toPosY = posY - botY;

    float const toBossX = maiden->GetPositionX() - botX;
    float const toBossY = maiden->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool MaidenOfVirtueTankPositionBossAction::MoveBossToStunnedHealer(Player* healer)
{
    constexpr float endDistance = 6.0f;
    float const angle = healer->GetOrientation();
    float const targetX = healer->GetPositionX() + std::cos(angle) * endDistance;
    float const targetY = healer->GetPositionY() + std::sin(angle) * endDistance;

    return MoveTo(
        KARA_MAP_ID, targetX, targetY, healer->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MaidenOfVirtuePositionRangedBetweenPillarsAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    constexpr uint8 maxIndex = 7;
    uint8 index = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !PlayerbotAI::IsRanged(member))
            continue;

        if (member == bot)
            break;

        if (index >= maxIndex)
        {
            index = 0;
            continue;
        }
        index++;
    }

    Position const& position = MAIDEN_OF_VIRTUE_RANGED_POSITIONS[index];
    if (bot->GetExactDist2d(position) <= 2.0f)
        return false;

    return MoveTo(
        KARA_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool MaidenOfVirtueSetGroundingTotemAction::Execute(Event /*event*/)
{
    return botAI->CanCastSpell(Id(KaraSpells::SPELL_GROUNDING_TOTEM), bot) &&
        botAI->CastSpell(Id(KaraSpells::SPELL_GROUNDING_TOTEM), bot);
}

// The Big Bad Wolf
// CombatReach is 0 [sic] yards

bool BigBadWolfPositionBossAction::Execute(Event /*event*/)
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    if (AI_VALUE(Unit*, "current target") != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() != bot || !bot->IsWithinMeleeRange(wolf))
        return false;

    Position const& position = BIG_BAD_WOLF_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = posX - botX;
    float const toPosY = posY - botY;

    float const toBossX = wolf->GetPositionX() - botX;
    float const toBossY = wolf->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Run away, little girl, run away
bool BigBadWolfLittleRedRidingHoodRunAwayAction::Execute(Event /*event*/)
{
    Position const& target = BIG_BAD_WOLF_RUN_POSITIONS[_runIndex];

    if (bot->GetExactDist2d(target.GetPositionX(), target.GetPositionY()) < 1.0f)
        _runIndex = (_runIndex + 1) % 4;

    Position const& position = BIG_BAD_WOLF_RUN_POSITIONS[_runIndex];

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Romulo and Julianne

bool RomuloAndJulianneMarkTargetAction::Execute(Event /*event*/)
{
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    if (!romulo)
        return false;

    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    if (!julianne)
        return false;

    constexpr float maxPctDifference = 10.0f;
    Unit* target = nullptr;

    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() ||
        julianne->GetHealthPct() < 1.0f)
    {
        target = romulo;
    }
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() ||
        romulo->GetHealthPct() < 1.0f)
    {
        target = julianne;
    }
    else
    {
        target = (romulo->GetHealthPct() >= julianne->GetHealthPct()) ? romulo : julianne;
    }

    if (target)
        return MarkTargetWithSkull(bot, target);

    return false;
}

// The Wizard of Oz

bool WizardOfOzMarkTargetAction::Execute(Event /*event*/)
{
    for (char const* name : OZ_TARGETS)
    {
        if (Unit* target = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, target);
    }

    return false;
}

bool WizardOfOzScorchStrawmanAction::Execute(Event /*event*/)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    return strawman &&
        botAI->CanCastSpell("scorch", strawman) &&
        botAI->CastSpell("scorch", strawman);
}

// The Curator

bool TheCuratorMarkAstralFlareAction::Execute(Event /*event*/)
{
    Unit* flare = AI_VALUE2(Unit*, "find target", "astral flare");
    return flare && MarkTargetWithSkull(bot, flare);
}

bool TheCuratorPositionBossAction::Execute(Event /*event*/)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    if (AI_VALUE(Unit*, "current target") != curator)
        return Attack(curator);

    if (curator->GetVictim() != bot || !bot->IsWithinMeleeRange(curator))
        return false;

    Position const& position = THE_CURATOR_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);
    if (distToPosition <= 2.0f)
        return false;

    float const posX = position.GetPositionX();
    float const posY = position.GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = posX - botX;
    float const toPosY = posY - botY;

    float const toBossX = curator->GetPositionX() - botX;
    float const toBossY = curator->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

bool TheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    constexpr float minDistance = 5.0f;
    Player* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);
    return nearestPlayer && FleePosition(nearestPlayer->GetPosition(), minDistance);
}

// Terestian Illhoof

bool TerestianIllhoofMarkTargetAction::Execute(Event /*event*/)
{
    static constexpr std::array illhoofTargets = {
        "demon chains", "kil'rek", "terestian illhoof", };

    for (char const* name : illhoofTargets)
    {
        if (Unit* target = AI_VALUE2(Unit*, "find target", name))
            return MarkTargetWithSkull(bot, target);
    }

    return false;
}

// Shade of Aran

bool ShadeOfAranRunAwayFromArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    constexpr float safeDistance = 20.0f;
    float const currentDistance = bot->GetDistance2d(aran);
    if (currentDistance >= safeDistance)
        return false;

    bot->CastStop();
    return MoveAway(aran, safeDistance - currentDistance);
}

// I will not move when Flame Wreath is cast or the raid blows up
bool ShadeOfAranStopMovingDuringFlameWreathAction::Execute(Event /*event*/)
{
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);
    if (!bot->isMoving())
        return false;

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    return true;
}

bool ShadeOfAranMarkConjuredElementalAction::Execute(Event /*event*/)
{
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 75.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(KaraNpcs::NPC_CONJURED_ELEMENTAL), searchRadius);

    for (Creature* elemental : creatureList)
    {
        if (!elemental || !elemental->IsAlive())
            continue;

        if (!botAI->HasAura("banish", elemental))
            return MarkTargetWithSkull(bot, elemental);
    }

    return false;
}

bool ShadeOfAranRangedMaintainDistanceAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    constexpr float minDistance = 10.0f;
    constexpr float maxDistance = 15.0f;
    float const distanceToBoss = bot->GetDistance2d(aran);

    if (distanceToBoss > maxDistance)
        return MoveTo(aran, maxDistance, MovementPriority::MOVEMENT_COMBAT);

    if (distanceToBoss < minDistance)
        return MoveTo(aran, minDistance, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

// Netherspite
// CombatReach is 18 yards

// The red beam dance (5 seconds in, 5 seconds out)
bool NetherspiteBlockRedBeamAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 150.0f;
    Unit* redPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius);
    if (!redPortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot != redBlocker)
    {
        _wasBlockingRedBeam = false;
        _redBeamTimerWasSet = false;
        return false;
    }

    if (bot->HasAura(Id(KaraSpells::SPELL_RED_BEAM_DEBUFF)))
    {
        if (!_redBeamTimerWasSet)
        {
            _redBeamMoveTimer = std::time(nullptr);
            _redBeamTimerWasSet = true;
        }
    }
    else
    {
        _redBeamTimerWasSet = false;
    }

    if (!_wasBlockingRedBeam)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_red", "%player is moving to block the red beam!",
            placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingRedBeam = true;

    constexpr uint8 intervalSecs = 5;
    if (_redBeamTimerWasSet && std::time(nullptr) - _redBeamMoveTimer >= intervalSecs)
    {
        _lastBeamMoveSideways = !_lastBeamMoveSideways;
        _redBeamMoveTimer = std::time(nullptr);
    }

    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    constexpr float idealDistance = 18.0f;
    std::vector<Unit*> voidZones;
    Position beamPos;
    if (!FindBeamPosition(netherspite, redPortal, voidZones, idealDistance, beamPos))
        return false;

    if (!_lastBeamMoveSideways)
    {
        return MoveTo(
            KARA_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), bot->GetPositionZ(),
            false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    float const length = netherspite->GetExactDist2d(redPortal);
    if (length == 0.0f)
        return false;

    float const dx = (redPortal->GetPositionX() - netherspite->GetPositionX()) / length;
    float const dy = (redPortal->GetPositionY() - netherspite->GetPositionY()) / length;
    float const perpDx = -dy;
    float const perpDy = dx;
    float const sideX = beamPos.GetPositionX() + perpDx * 3.0f;
    float const sideY = beamPos.GetPositionY() + perpDy * 3.0f;

    return MoveTo(
        KARA_MAP_ID, sideX, sideY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

// Two mana-user DPS bots will block the blue beam for each phase (swap at 25 debuff stacks)
bool NetherspiteBlockBlueBeamAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 150.0f;
    Unit* bluePortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius);
    if (!bluePortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot != blueBlocker)
    {
        if (_wasBlockingBlueBeam)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_leaving_blue",
                "%player is leaving the blue beam. Next blocker up!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingBlueBeam = false;
        return false;
    }

    if (!_wasBlockingBlueBeam)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_blue",
            "%player is moving to block the blue beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingBlueBeam = true;

    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    float const idealDistance = bot->getClass() == CLASS_HUNTER ? 25.0f : 18.0f;
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    Position beamPos;

    if (!FindBeamPosition(netherspite, bluePortal, voidZones, idealDistance, beamPos))
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Two healer bots will block the green beam for each phase (swap at 25 debuff stacks)
// OR one non-mana-user dps bot will block the green beam for an entire phase
bool NetherspiteBlockGreenBeamAction::Execute(Event /*event*/)
{
    constexpr float searchRadius = 150.0f;
    Unit* greenPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius);
    if (!greenPortal)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(bot);

    if (bot != greenBlocker)
    {
        if (_wasBlockingGreenBeam)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_leaving_green",
                "%player is leaving the green beam. Next blocker up!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingGreenBeam = false;
        return false;
    }

    if (!_wasBlockingGreenBeam)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_blocking_green",
            "%player is moving to block the green beam!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
    }
    _wasBlockingGreenBeam = true;

    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    constexpr float idealDistance = 18.0f;
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    Position beamPos;

    if (!FindBeamPosition(netherspite, greenPortal, voidZones, idealDistance, beamPos))
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), bot->GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NetherspiteAvoidBeamAndVoidZoneAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(bot);

    constexpr float hazardRadius = 4.0f;
    bool const nearVoidZone = !IsSafePosition(
        bot->GetPositionX(), bot->GetPositionY(), voidZones, hazardRadius);

    constexpr float searchRadius = 150.0f;
    float const bossX = netherspite->GetPositionX();
    float const bossY = netherspite->GetPositionY();
    std::vector<BeamAvoid> beams;

    if (Unit* redPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_RED_PORTAL), searchRadius))
    {
        float const len = netherspite->GetExactDist2d(redPortal);
        beams.push_back({0.0f, len,
            len > 0.0f ? (redPortal->GetPositionX() - bossX) / len : 0.0f,
            len > 0.0f ? (redPortal->GetPositionY() - bossY) / len : 0.0f});
    }

    if (Unit* bluePortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_BLUE_PORTAL), searchRadius))
    {
        float const len = netherspite->GetExactDist2d(bluePortal);
        beams.push_back({0.0f, len,
            len > 0.0f ? (bluePortal->GetPositionX() - bossX) / len : 0.0f,
            len > 0.0f ? (bluePortal->GetPositionY() - bossY) / len : 0.0f});
    }

    if (Unit* greenPortal = bot->FindNearestCreature(Id(KaraNpcs::NPC_GREEN_PORTAL), searchRadius))
    {
        float const len = netherspite->GetExactDist2d(greenPortal);
        beams.push_back({0.0f, len,
            len > 0.0f ? (greenPortal->GetPositionX() - bossX) / len : 0.0f,
            len > 0.0f ? (greenPortal->GetPositionY() - bossY) / len : 0.0f});
    }

    bool const nearBeam = !IsAwayFromBeams(
        bot->GetPositionX(), bot->GetPositionY(), bossX, bossY, beams);

    if (!nearVoidZone && !nearBeam)
        return false;

    constexpr uint8 numAngles = 36;
    constexpr float stepDist = 0.5f;
    constexpr uint8 numSteps = 56;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    Position bestCandidate;
    float bestDistSq = std::numeric_limits<float>::max();
    bool found = false;

    for (uint8 i = 0; i < numAngles; ++i)
    {
        float const angle = i * M_PI / 18.0f;
        for (uint8 j = 0; j <= numSteps; ++j)
        {
            float const dist = 2.0f + j * stepDist;
            float candidateX = botX + std::cos(angle) * dist;
            float candidateY = botY + std::sin(angle) * dist;

            if (!IsSafePosition(candidateX, candidateY, voidZones, hazardRadius) ||
                !IsAwayFromBeams(candidateX, candidateY, bossX, bossY, beams))
            {
                continue;
            }

            float dx = candidateX - botX;
            float dy = candidateY - botY;
            float moveDistSq = dx*dx + dy*dy;

            if (!found || moveDistSq < bestDistSq)
            {
                bestCandidate = Position(candidateX, candidateY, bot->GetPositionZ());
                bestDistSq = moveDistSq;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestCandidate.GetPositionX(), bestCandidate.GetPositionY(),
        bestCandidate.GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool NetherspiteAvoidBeamAndVoidZoneAction::IsAwayFromBeams(
     float x, float y, float botX, float botY, std::vector<BeamAvoid> const& beams)
{
    for (auto const& beam : beams)
    {
        if (beam.maxDist == 0.0f)
            continue;

        float const botdx = x - botX;
        float const botdy = y - botY;
        float const distanceAlongBeam = (botdx * beam.dirX + botdy * beam.dirY);
        float const beamX = botX + beam.dirX * distanceAlongBeam;
        float const beamY = botY + beam.dirY * distanceAlongBeam;
        float const distToBeamSq = (x - beamX) * (x - beamX) + (y - beamY) * (y - beamY);

        constexpr float minDistFromBeamSq = 25.0f;
        if (distToBeamSq < minDistFromBeamSq && distanceAlongBeam > beam.minDist &&
            distanceAlongBeam < beam.maxDist)
        {
            return false;
        }
    }

    return true;
}

bool NetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event /*event*/)
{
    std::vector<Unit*> voidZones = GetAllVoidZones(bot);
    constexpr float safeDistance = 4.0f;

    for (Unit* voidZone : voidZones)
    {
        if (voidZone->GetEntry() == Id(KaraNpcs::NPC_VOID_ZONE) &&
            bot->GetExactDist2d(voidZone) < safeDistance)
        {
            return FleePosition(voidZone->GetPosition(), safeDistance);
        }
    }

    return false;
}

bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    uint32 const instanceId = netherspite->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    bool const isMechanicTracker = IsMechanicTrackerBot(bot, KARA_MAP_ID);
    bool didSomething = false;

    if (IsBanishPhase(netherspite))
    {
        if (isMechanicTracker && netherspiteDpsWaitTimer.erase(instanceId) > 0)
            didSomething = true;

        Action* redAction = context->GetAction("netherspite block red beam");
        if (redAction &&
            static_cast<NetherspiteBlockRedBeamAction*>(redAction)->ResetRedBeamState())
        {
            didSomething = true;
        }

        Action* blueAction = context->GetAction("netherspite block blue beam");
        if (blueAction &&
            static_cast<NetherspiteBlockBlueBeamAction*>(blueAction)->ResetBlueBeamState())
        {
            didSomething = true;
        }

        Action* greenAction = context->GetAction("netherspite block green beam");
        if (greenAction &&
            static_cast<NetherspiteBlockGreenBeamAction*>(greenAction)->ResetGreenBeamState())
        {
            didSomething = true;
        }
    }
    else if (isMechanicTracker && netherspiteDpsWaitTimer.try_emplace(instanceId, now).second)
    {
        didSomething = true;
    }

    return didSomething;
}

// Prince Malchezaar
// CombatReach is 4 yards

bool PrinceMalchezaarEnfeebledBotAvoidHazardAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    constexpr uint8 numAngles = 64;
    constexpr float minSafeBossDistance = 32.0f;
    constexpr float distIncrement = 0.5f;
    constexpr uint8 numSteps = 56;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const bossX = malchezaar->GetPositionX();
    float const bossY = malchezaar->GetPositionY();
    float bestMoveDistSq = std::numeric_limits<float>::max();
    float bestDestX = 0.0f;
    float bestDestY = 0.0f;
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float const angle = (2 * M_PI * i) / numAngles;
        float const dx = std::cos(angle);
        float const dy = std::sin(angle);

        for (uint8 j = 0; j <= numSteps; ++j)
        {
            float const dist = minSafeBossDistance + j * distIncrement;
            float destX = bossX + dx * dist;
            float destY = bossY + dy * dist;
            float destZ = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, botX, botY, destZ, destX, destY, destZ, true))
            {
                continue;
            }

            float ddx = destX - bossX;
            float ddy = destY - bossY;

            constexpr float minSafeBossDistanceSq = minSafeBossDistance * minSafeBossDistance;
            if (ddx*ddx + ddy*ddy < minSafeBossDistanceSq)
                continue;

            float mdx = destX - botX;
            float mdy = destY - botY;
            float moveDistSq = mdx*mdx + mdy*mdy;

            constexpr float safeInfernalDistance = 22.0f;
            if (IsStraightPathSafe(botX, botY, destX, destY, infernals, safeInfernalDistance) &&
                moveDistSq < bestMoveDistSq)
            {
                bestMoveDistSq = moveDistSq;
                bestDestX = destX;
                bestDestY = destY;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool PrinceMalchezaarNonTankAvoidInfernalAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    constexpr float safeInfernalDistance = 22.0f;
    if (!bot->FindNearestCreature(
            Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), safeInfernalDistance, true))
    {
        return false;
    }

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    constexpr float maxDistanceFromBoss = 33.0f;

    float bestDestX = botX;
    float bestDestY = botY;

    bool found = TryFindSafePositionWithSafePath(
        bot, botX, botY, malchezaar->GetPositionX(), malchezaar->GetPositionY(),
        infernals, safeInfernalDistance, maxDistanceFromBoss, bestDestX, bestDestY);

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool PrinceMalchezaarTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    if (AI_VALUE(Unit*, "current target") != malchezaar)
        return Attack(malchezaar);

    if (malchezaar->GetVictim() != bot || !bot->IsWithinMeleeRange(malchezaar))
        return false;

    constexpr float safeInfernalDistance = 30.0f;
    if (!bot->FindNearestCreature(
            Id(KaraNpcs::NPC_NETHERSPITE_INFERNAL), safeInfernalDistance, true))
    {
        return false;
    }

    std::vector<Unit*> infernals = GetSpawnedInfernals(bot);

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    constexpr float maxSampleDist = 75.0f;
    float bestDestX = botX;
    float bestDestY = botY;

    bool found = TryFindSafePositionWithSafePath(
        bot, botX, botY, botX, botY, infernals, safeInfernalDistance,
        maxSampleDist, bestDestX, bestDestY);

    if (!found)
        return false;

    float const toPosX = bestDestX - botX;
    float const toPosY = bestDestY - botY;

    float const toBossX = malchezaar->GetPositionX() - botX;
    float const toBossY = malchezaar->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    return MoveTo(
        KARA_MAP_ID, bestDestX, bestDestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

// Nightbane
// CombatReach is 10.5 yards

bool NightbaneGroundPhaseTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    if (AI_VALUE(Unit*, "current target") != nightbane)
        return Attack(nightbane);

    if (nightbane->GetVictim() != bot || !bot->IsWithinMeleeRange(nightbane))
        return false;

    Position const& domeCenter = TERRACE_DOME_CENTER;
    Position const& eastEnd = TERRACE_EAST_END;
    Position const& westEnd = TERRACE_WEST_END;

    float const radius = domeCenter.GetExactDist2d(eastEnd);
    float const thetaA = atan2(
        eastEnd.GetPositionY() - domeCenter.GetPositionY(),
        eastEnd.GetPositionX() - domeCenter.GetPositionX());
    float const thetaB = atan2(
        westEnd.GetPositionY() - domeCenter.GetPositionY(),
        westEnd.GetPositionX() - domeCenter.GetPositionX());

    float deltaAB = thetaB - thetaA;
    if (deltaAB < 0.0f)
        deltaAB += 2.0f * M_PI;

    float arcStart = 0.0f;
    float arcEnd = 0.0f;
    if (deltaAB < M_PI)
    {
        arcStart = thetaB;
        arcEnd = thetaA + 2.0f * M_PI;
    }
    else
    {
        arcStart = thetaA;
        arcEnd = thetaB;
    }

    float thetaN = atan2(
        nightbane->GetPositionY() - domeCenter.GetPositionY(),
        nightbane->GetPositionX() - domeCenter.GetPositionX());

    if (thetaN < arcStart)
        thetaN += 2.0f * M_PI;
    else if (thetaN >= arcStart + 2.0f * M_PI)
        thetaN -= 2.0f * M_PI;

    float const thetaClamped = std::max(arcStart, std::min(arcEnd, thetaN));
    float const destX = domeCenter.GetPositionX() + radius * cos(thetaClamped);
    float const destY = domeCenter.GetPositionY() + radius * sin(thetaClamped);
    float const distToPosition = bot->GetExactDist2d(destX, destY);

    if (distToPosition <= 0.5f)
        return false;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const toPosX = destX - botX;
    float const toPosY = destY - botY;

    float const toBossX = nightbane->GetPositionX() - botX;
    float const toBossY = nightbane->GetPositionY() - botY;
    bool const backwards = (toPosX * toBossX + toPosY * toBossY) < 0.0f;

    float const maxMoveDist = backwards ? 2.25f : 3.5f;
    float const moveDist = std::min(maxMoveDist, distToPosition);
    float const moveX = botX + (toPosX / distToPosition) * moveDist;
    float const moveY = botY + (toPosY / distToPosition) * moveDist;

    return MoveTo(
        KARA_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, backwards);
}

// Ranged bots will stack on one "ranged leader" that can be designated by the assistant flag
// The ranged leader will lead the ranged group out of Charred Earths
bool NightbaneGroundPhaseCoordinateRangedMovementAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* rangedLeader = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != KARA_MAP_ID ||
            !member->IsAlive() || !PlayerbotAI::IsRanged(member))
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
        {
            rangedLeader = member;
            break;
        }

        if (!rangedLeader || member->GetGUID() < rangedLeader->GetGUID())
            rangedLeader = member;
    }

    if (!rangedLeader)
        return false;

    if (bot == rangedLeader)
        return MoveRangedLeaderToSafeSpot();

    return StackOnRangedLeader(rangedLeader);
}

bool NightbaneGroundPhaseCoordinateRangedMovementAction::MoveRangedLeaderToSafeSpot()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    constexpr float searchRadius = 40.0f;
    constexpr float minBossDist = 15.0f;

    std::vector<Position> charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, Id(KaraSpells::SPELL_CHARRED_EARTH));

    if (charredEarths.empty())
    {
        float const distToBoss = bot->GetExactDist2d(nightbane);
        if (distToBoss < minBossDist)
        {
            bot->CastStop();
            return MoveAway(nightbane, minBossDist - distToBoss, true);
        }

        return false;
    }

    constexpr float safeDistance = 12.0f;
    float const safeDistSq = safeDistance * safeDistance;
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    bool inDanger = false;
    for (auto const& charredEarth : charredEarths)
    {
        float dx = botX - charredEarth.GetPositionX();
        float dy = botY - charredEarth.GetPositionY();
        if (dx * dx + dy * dy < safeDistSq)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    constexpr float maxBossDist = 35.0f;
    constexpr float distStep = 1.0f;
    constexpr uint8 numSteps = 20;
    constexpr uint8 numAngles = 36;

    float const bossX = nightbane->GetPositionX();
    float const bossY = nightbane->GetPositionY();
    float bestDistSq = std::numeric_limits<float>::max();
    float bestX = botX;
    float bestY = botY;
    bool found = false;

    for (uint8 i = 0; i <= numSteps; ++i)
    {
        float const dist = minBossDist + i * distStep;
        for (uint8 j = 0; j < numAngles; ++j)
        {
            float const angle = j * M_PI / 18.0f;
            float candidateX = bossX + cos(angle) * dist;
            float candidateY = bossY + sin(angle) * dist;

            bool safe = true;
            for (auto const& charredEarth : charredEarths)
            {
                float dx = candidateX - charredEarth.GetPositionX();
                float dy = candidateY - charredEarth.GetPositionY();
                if (dx * dx + dy * dy < safeDistSq)
                {
                    safe = false;
                    break;
                }
            }

            if (!safe || nightbane->GetExactDist2d(candidateX, candidateY) > maxBossDist)
                continue;

            float testX = candidateX;
            float testY = candidateY;
            float testZ = bot->GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                    bot, bot->GetPositionX(), bot->GetPositionY(),
                    bot->GetPositionZ(), testX, testY, testZ))
            {
                continue;
            }

            if (!nightbane->IsWithinLOS(candidateX, candidateY, bot->GetPositionZ()))
                continue;

            float dx = candidateX - botX;
            float dy = candidateY - botY;
            float moveDistSq = dx * dx + dy * dy;
            if (moveDistSq < bestDistSq)
            {
                bestDistSq = moveDistSq;
                bestX = candidateX;
                bestY = candidateY;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, bestX, bestY, bot->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneGroundPhaseCoordinateRangedMovementAction::StackOnRangedLeader(Player* rangedLeader)
{
    if (bot->GetExactDist2d(rangedLeader) < 0.5f)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, rangedLeader->GetPositionX(), rangedLeader->GetPositionY(),
        rangedLeader->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool NightbaneControlPetAggressionAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z && pet->GetReactState() == REACT_PASSIVE)
        pet->SetReactState(REACT_DEFENSIVE);

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z && pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }

    return false;
}

// 1. Stack at the "Flight Stack Position" near Nightbane so he doesn't use Fireball Barrage
// 2. Once Rain of Bones hits, the whole party moves to a new stack position
// This lasts for the first 35 seconds of the flight phase, after which Nightbane begins landing
bool NightbaneFlightPhaseStackAndMoveAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    if (AI_VALUE(Unit*, "current target") == nightbane)
    {
        bot->AttackStop();
        bot->CastStop();
    }

    if (bot->HasAura(Id(KaraSpells::SPELL_RAIN_OF_BONES)))
        _rainOfBonesHit = true;

    auto const& posArray = _rainOfBonesHit ?
        NIGHTBANE_RAIN_OF_BONES_POSITIONS : NIGHTBANE_FLIGHT_STACK_POSITIONS;
    Position destPos;
    bool foundSafe = false;

    constexpr float searchRadius = 40.0f;
    constexpr float charredEarthSafeDist = 12.0f;
    std::vector<Position> charredEarths = GetDynamicObjectPositions(
        bot, searchRadius, Id(KaraSpells::SPELL_CHARRED_EARTH));

    for (uint8 i = 0; i < 2; i++)
    {
        destPos = posArray[i];
        bool inCharredEarth = false;

        for (auto const& charredEarth : charredEarths)
        {
            if (charredEarth.GetExactDist2d(destPos) < charredEarthSafeDist)
            {
                inCharredEarth = true;
                break;
            }
        }

        if (!inCharredEarth)
        {
            foundSafe = true;
            break;
        }
    }

    if (!foundSafe)
        return false;

    if (bot->GetExactDist2d(destPos) < 0.5f)
        return false;

    bot->CastStop();
    return MoveTo(
        KARA_MAP_ID, destPos.GetPositionX(), destPos.GetPositionY(), destPos.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Failsafe in case bots fall through the world or off the terrace
bool NightbaneTeleportBackToTerraceAction::Execute(Event /*event*/)
{
    Position const& position = NIGHTBANE_TELEPORT_POSITION;
    return bot->TeleportTo(
        KARA_MAP_ID, position.GetPositionX(), position.GetPositionY(),
        position.GetPositionZ(), bot->GetOrientation());
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    uint32 const instanceId = nightbane->GetMap()->GetInstanceId();
    time_t const now = std::time(nullptr);
    bool const isMechanicTracker = IsMechanicTrackerBot(bot, KARA_MAP_ID);
    bool didSomething = false;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
    {
        Action* action = context->GetAction("nightbane flight phase stack and move");
        if (action &&
            static_cast<NightbaneFlightPhaseStackAndMoveAction*>(action)->ResetRainOfBonesHit())
        {
            didSomething = true;
        }

        if (isMechanicTracker)
        {
            if (nightbaneFlightPhaseStartTimer.erase(instanceId) > 0)
                didSomething = true;

            if (nightbaneDpsWaitTimer.try_emplace(instanceId, now).second)
                didSomething = true;
        }
    }
    else if (isMechanicTracker)
    {
        if (nightbaneDpsWaitTimer.erase(instanceId) > 0)
            didSomething = true;

        if (nightbaneFlightPhaseStartTimer.try_emplace(instanceId, now).second)
            didSomething = true;
    }

    return didSomething;
}
