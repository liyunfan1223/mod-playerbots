/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Ignis.h"
#include "UldActions_Shared.h"

#include <CombatStrategy.h>
#include <FollowMasterStrategy.h>

#include <cmath>

#include "AiObjectContext.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Group.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "Spell.h"
#include "UldEncounter_Ignis.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include <TankAssistStrategy.h>

bool IgnisScorchedGroundAction::isUseful()
{
    IgnisScorchedGroundTrigger ignisScorchedGroundTrigger(botAI);
    return ignisScorchedGroundTrigger.IsActive();
}

bool IgnisScorchedGroundAction::Execute(Event /*event*/)
{
    // Same grid lookup the trigger uses. Going through "nearest npcs" here instead would leave the
    // trigger hot on a patch the action cannot see, and the bot would stand in it burning.
    Unit* patch = GetIgnisNearestScorchedGround(botAI, bot);
    if (!patch)
        return false;

    return FleePosition(patch->GetPosition(), ULDUAR_IGNIS_SCORCHED_GROUND_AVOID_RADIUS);
}

bool IgnisMainTankPositionAction::isUseful()
{
    IgnisMainTankPositionTrigger ignisMainTankPositionTrigger(botAI);
    return ignisMainTankPositionTrigger.IsActive();
}

bool IgnisMainTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = GetIgnis(botAI);
    if (!boss)
        return false;

    Position const spot = GetIgnisMainTankPosition(botAI, bot, boss);
    float const distance = bot->GetExactDist2d(&spot);

    // Reach then hold. Without the latch the bot re-issues a move on every drift inside the
    // tolerance, and a bot that is sliding on the spot never lands a swing.
    if (_spotReached && distance > ULDUAR_IGNIS_TANK_SPOT_TOLERANCE * 2.0f)
        _spotReached = false;

    if (_spotReached || distance <= ULDUAR_IGNIS_TANK_SPOT_TOLERANCE)
    {
        _spotReached = true;
        return false;
    }

    // No throttle needed, at 10 yd/s to a player's 7 he keeps up. Out of melee means his path broke and walking
    // on loses him for good, unless he's rooted for Scorch.
    if (!bot->IsWithinMeleeRange(boss) && !IsIgnisScorchWindow(boss))
        return false;

    return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool IgnisConstructTankAction::isUseful()
{
    IgnisConstructTankTrigger ignisConstructTankTrigger(botAI);
    return ignisConstructTankTrigger.IsActive();
}

bool IgnisConstructTankAction::Execute(Event event)
{
    int8 const tankIndex = GetIgnisConstructTankIndex(botAI, bot);
    if (tankIndex < 0)
        return false;

    Unit* construct = GetIgnisDrivenConstruct(botAI, bot);
    if (!construct)
        return false;

    if (AI_VALUE(Unit*, "current target") != construct)
        return Attack(construct);

    // Molten resets the construct's threat table, so aggro is re-checked before every leg of the walk
    // or the construct stops following halfway to the water.
    if (construct->GetVictim() != bot)
    {
        if (botAI->DoSpecificAction("taunt spell", event, true))
            return true;

        // Out of taunt range (every taunt is 30 yd), so walk it down. Next tick's taunt ends the chase, nothing
        // else here brings it back.
        return MoveTo(bot->GetMapId(), construct->GetPositionX(), construct->GetPositionY(),
                      construct->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (IsIgnisConstructMolten(construct))
    {
        Position const& pool = GetIgnisAssignedWaterPool(tankIndex);

        // Brittle comes off a once-a-second poll on the construct, so hold still once it is in range
        // rather than walking it back out again.
        if (construct->GetExactDist2d(&pool) <= ULDUAR_IGNIS_WATER_BRITTLE_RADIUS)
            return false;

        Position const spot = GetIgnisWaterApproach(bot, pool);

        return MoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // No lit patch yet: hold the construct where it is instead of dragging it around the room.
    Unit* scorchedGround = GetIgnisAssignedScorchedGround(botAI, construct, tankIndex);
    if (!scorchedGround)
        return false;

    if (bot->GetExactDist2d(scorchedGround) <= ULDUAR_IGNIS_SCORCHED_GROUND_PARK_DISTANCE)
        return false;

    return MoveTo(bot->GetMapId(), scorchedGround->GetPositionX(), scorchedGround->GetPositionY(),
                  scorchedGround->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_FORCED, true,
                  false);
}

bool IgnisAttackBrittleConstructAction::isUseful()
{
    IgnisAttackBrittleConstructTrigger ignisAttackBrittleConstructTrigger(botAI);
    return ignisAttackBrittleConstructTrigger.IsActive();
}

bool IgnisAttackBrittleConstructAction::Execute(Event /*event*/)
{
    Unit* construct = GetIgnisBrittleConstruct(botAI);
    if (!construct)
        return false;

    if (AI_VALUE(Unit*, "current target") != construct)
        return Attack(construct);

    // Rotation filler often misses the single 5000 / 3000 hit Shatter needs. Returns false when none is castable,
    // so the rotation runs instead of waiting on a cooldown.
    static std::unordered_map<uint8, std::vector<std::string>> const burstSpells = {
        {CLASS_MAGE, {"pyroblast", "frostbolt"}},
        {CLASS_HUNTER, {"chimera shot", "aimed shot"}},
        {CLASS_WARLOCK, {"chaos bolt", "shadow bolt"}},
        {CLASS_PRIEST, {"mind blast"}},
        {CLASS_DRUID, {"starfire"}},
        {CLASS_SHAMAN, {"lava burst"}},
    };

    auto const spells = burstSpells.find(bot->getClass());
    if (spells == burstSpells.end())
        return false;

    for (std::string const& spell : spells->second)
    {
        if (botAI->CanCastSpell(spell, construct))
            return botAI->CastSpell(spell, construct);
    }

    return false;
}

bool IgnisAttackBossAction::isUseful()
{
    IgnisAttackBossTrigger ignisAttackBossTrigger(botAI);
    return ignisAttackBossTrigger.IsActive();
}

bool IgnisAttackBossAction::Execute(Event /*event*/)
{
    Unit* boss = GetIgnis(botAI);
    if (!boss)
        return false;

    if (AI_VALUE(Unit*, "current target") != boss)
        return Attack(boss);

    return false;
}

bool IgnisMoltenConstructAvoidAction::isUseful()
{
    IgnisMoltenConstructAvoidTrigger ignisMoltenConstructAvoidTrigger(botAI);
    return ignisMoltenConstructAvoidTrigger.IsActive();
}

bool IgnisMoltenConstructAvoidAction::Execute(Event /*event*/)
{
    // Keyed on the Molten aura rather than the creature entry, so the construct the tank is still
    // walking through the fire does not scatter the raid.
    Unit* molten = GetIgnisNearestMoltenConstruct(botAI, bot);
    if (!molten)
        return false;

    return FleePosition(molten->GetPosition(), ULDUAR_IGNIS_MOLTEN_AVOID_RADIUS);
}

bool IgnisFlameJetsHoldCastAction::isUseful()
{
    IgnisFlameJetsTrigger ignisFlameJetsTrigger(botAI);
    return ignisFlameJetsTrigger.IsActive();
}

bool IgnisFlameJetsHoldCastAction::Execute(Event /*event*/)
{
    Unit* boss = GetIgnis(botAI);
    if (!boss)
        return false;

    Spell* jets = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    Spell* own = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!jets || !own)
        return false;

    // A cast that lands first is worth finishing. Only the ones the knockback would eat get stopped.
    if (own->GetCastTimeRemaining() < jets->GetCastTimeRemaining())
        return false;

    bot->CastStop();

    return true;
}

bool IgnisSlagPotHealAction::isUseful()
{
    IgnisSlagPotHealTrigger ignisSlagPotHealTrigger(botAI);
    return ignisSlagPotHealTrigger.IsActive();
}

bool IgnisSlagPotHealAction::Execute(Event /*event*/)
{
    Player* victim = GetIgnisSlagPotVictim(botAI);
    if (!victim)
        return false;

    // The pot ticks for ten seconds and the victim cannot be moved or shielded out of it, so the only
    // answer is raw throughput.
    static std::vector<std::string> const directHeals = {
        "greater heal",         "flash heal",       "penance",   // priest
        "healing touch",        "nourish",          "regrowth",  // druid
        "holy light",           "flash of light",   "holy shock",  // paladin
        "greater healing wave", "healing wave",     "riptide",  // shaman
    };

    for (std::string const& heal : directHeals)
    {
        if (botAI->CanCastSpell(heal, victim))
            return botAI->CastSpell(heal, victim);
    }

    return false;
}
