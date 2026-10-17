/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RSTriggers.h"
#include "Playerbots.h"
#include "RSActions.h"
#include "RSScripts.h"
#include "Timer.h"
#include "Trigger.h"
#include <set>

bool RsBaltharusBrandTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_ENERVATING_BRAND))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "baltharus the warborn");
    return boss != nullptr;
}

bool RsBaltharusTankPositionTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "baltharus the warborn");
    return boss != nullptr;
}

bool RsBaltharusHealerPositionTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "baltharus the warborn");
    return boss != nullptr;
}

bool RsBaltharusAvoidFrontTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "baltharus the warborn");
    return boss != nullptr;
}

bool RsSavianaConflagrationTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_FLAME_BEACON))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    return boss != nullptr;
}

bool RsSavianaTankPositionTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    return boss != nullptr;
}

bool RsSavianaAvoidFrontTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    if (!boss)
        return false;

    return !boss->IsLevitating();
}

bool RsSavianaMeleeSpreadTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || (!PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsHeal(bot)))
        return false;

    if (bot->HasAura(SPELL_FLAME_BEACON))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "saviana ragefire");
    if (!boss)
        return false;

    return boss->IsLevitating();
}

bool RsZarithrianAddsTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "general zarithrian");
    if (!boss || !boss->IsAlive())
        return false;

    return true;
}

bool RsZarithrianTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "general zarithrian");
    return boss != nullptr;
}

bool RsHalionTankPositionTrigger::IsActive()
{
    if (RsHalionInTwilight(bot))
        return false;

    Unit* boss = RsHalionAnyPhysicalBoss(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    return RsHalionBossTank(botAI) == bot;
}

bool RsHalionAvoidConesTrigger::IsActive()
{
    if (RsHalionInTwilight(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) && !RsHalionAssistTankAsMelee(botAI))
        return false;

    Unit* boss = RsHalionAnyPhysicalBoss(botAI);
    return boss != nullptr && boss->IsInCombat();
}

bool RsHalionCombustionTrigger::IsActive()
{
    if (RsHalionInTwilight(bot))
        return false;

    Unit* boss = RsHalionAnyPhysicalBoss(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    if (RsHalionBossTank(botAI) == bot)
        return false;

    bool const returning = RsHalionCombustionReturning(bot);
    return RsHalionHasCombustion(bot) || returning;
}

bool RsHalionMeteorTrigger::IsActive()
{
    if (RsHalionInTwilight(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) && !RsHalionAssistTankAsMelee(botAI))
        return false;

    Unit* boss = RsHalionAnyPhysicalBoss(botAI);
    if (!boss || !boss->IsInCombat())
        return false;

    if (RsHalionHasCombustion(bot))
        return false;

    return RsHalionMeteorShouldRally(bot);
}

bool RsHalionAddsTrigger::IsActive()
{
    if (RsHalionInTwilight(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsMelee(bot))
        return false;

    if (RsHalionHasCombustion(bot))
        return false;

    if (!RsHalionEngaged(botAI))
        return false;

    return RsHalionAnyAddAlive(botAI);
}

bool RsHalionAddTankTrigger::IsActive()
{
    if (!RsHalionEngaged(botAI))
        return false;

    return RsHalionTanksAdds(botAI, bot);
}

bool RsHalionStartPositionTrigger::IsActive()
{
    Unit* boss = RsHalionPhase1Boss(botAI);
    if (!boss || !boss->HealthAbovePct(98) || !boss->IsInCombat())
        return false;

    return RsHalionBossTank(botAI) != bot;
}

bool RsHalionEnterPortalTrigger::IsActive()
{
    bool const inTwilight = RsHalionInTwilight(bot);

    if (inTwilight)
        return RsHalionPortalCommit(botAI, bot);

    if (RsHalionEnteringTwilight(botAI, bot))
        return true;

    return !PlayerbotAI::IsMainTank(bot) && RsHalionPortalHeldForAdds(botAI);
}

bool RsHalionP2TankPositionTrigger::IsActive()
{
    if (RsHalionTwilightTank(botAI) != bot || !RsHalionInTwilight(bot))
        return false;

    return RsHalionTwilightBoss(botAI) != nullptr;
}

bool RsHalionP2AvoidConesTrigger::IsActive()
{
    if (!RsHalionInTwilight(bot))
        return false;

    if (RsHalionTwilightTank(botAI) == bot)
        return false;

    return RsHalionTwilightBoss(botAI) != nullptr;
}

bool RsHalionConsumptionTrigger::IsActive()
{
    if (!RsHalionInTwilight(bot))
        return false;

    if (!bot->HasAura(SPELL_MARK_OF_CONSUMPTION) && !bot->HasAura(SPELL_SOUL_CONSUMPTION))
        return false;

    return RsHalionTwilightBoss(botAI) != nullptr;
}

bool RsHalionCutterTrigger::IsActive()
{
    if (!RsHalionInTwilight(bot))
        return false;

    Unit* boss = RsHalionTwilightBoss(botAI);
    if (!boss)
        return false;

    return boss->FindNearestCreature(NPC_ORB_CARRIER, 200.0f) != nullptr;
}

bool RsHalionHealConsumptionTrigger::IsActive()
{
    if (!PlayerbotAI::IsHeal(bot) || !RsHalionInTwilight(bot))
        return false;

    if (!RsHalionIsPhase3(botAI))
        return false;

    return RsHalionConsumptionHealTarget(botAI) != nullptr;
}

bool RsTrashAddsTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;

    return RsTrashActive(botAI, bot);
}

bool RsTrashMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    return RsTrashActive(botAI, bot);
}

bool RsTrashAssistTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsAssistTank(bot))
        return false;

    return RsTrashActive(botAI, bot);
}

bool RsTrashRangedTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (!PlayerbotAI::IsRanged(bot) && !PlayerbotAI::IsHeal(bot))
        return false;

    return RsTrashActive(botAI, bot);
}

bool RsTrashMeleeFlankTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) || !PlayerbotAI::IsMelee(bot))
        return false;

    return RsTrashActive(botAI, bot);
}
