#include "Playerbots.h"
#include "OculusTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "Unit.h"

bool DrakosUnstableSphereTrigger::IsActive()
{
    // Doesn't seem to be much point trying to get melee to dodge this,
    // they get hit anyway and it just causes a lot of running around and chaos
    // if (botAI->IsMelee(bot)) { return false; }
    if (botAI->IsTank(bot)) { return false; }

    GuidVector targets = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_UNSTABLE_SPHERE)
        {
            return true;
        }
    }
    return false;
}

bool DrakeMountTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }
    
    return master->GetVehicleBase() && !bot->GetVehicleBase();
}

bool DrakeDismountTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }

    return !master->GetVehicleBase() && bot->GetVehicleBase();
}

bool GroupFlyingTrigger::IsActive()
{
    Player* master = botAI->GetMaster();
    if (!master) { return false; }

    return master->GetVehicleBase() && bot->GetVehicleBase();
}

bool DrakeCombatTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    if (!vehicleBase) { return false; }

    GuidVector attackers = AI_VALUE(GuidVector, "attackers");
    for (auto& attacker : attackers)
    {
        Unit* target = botAI->GetUnit(attacker);
        if (!target) { continue; }

        return true;
    }
    return false;
}

bool VarosCloudstriderTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "varos cloudstrider");
    if (!boss) { return false; }

    return true;
}

bool UromArcaneExplosionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "mage-lord urom");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_EMPOWERED_ARCANE_EXPLOSION);
}

bool UromTimeBombTrigger::IsActive()
{
    return bot->HasAura(SPELL_TIME_BOMB);
}
