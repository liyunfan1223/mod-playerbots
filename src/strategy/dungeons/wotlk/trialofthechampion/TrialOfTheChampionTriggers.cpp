#include "Playerbots.h"
#include "TrialOfTheChampionTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"


bool ToCLanceTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Unit* mount1 = bot->FindNearestCreature(NPC_ARGENT_WARHORSE, 100.0f);
    if (!mount1)
        return false;

    Unit* mount2 = bot->FindNearestCreature(NPC_ARGENT_BATTLEWORG, 100.0f);
    if (!mount2)
        return false;

    if (!bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return true;

    if (bot->HasItemCount(ITEM_LANCE, 1))
        return false;

    // Find the nearest spear
    GameObject* lanceRack = bot->FindNearestGameObject(OBJECT_LANCE_RACK, 100.0f);
    if (!lanceRack)
        return false;

    return true;
}

bool ToCUELanceTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Unit* mount1 = bot->FindNearestCreature(NPC_ARGENT_WARHORSE, 100.0f);
    if (!mount1 && bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return true;

    Unit* mount2 = bot->FindNearestCreature(NPC_ARGENT_BATTLEWORG, 100.0f);
    if (!mount2 && bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return true;

    return false;
}

bool ToCMountedTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    return entry == NPC_ARGENT_BATTLEWORG || entry == NPC_ARGENT_WARHORSE;
}

bool ToCMountNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Unit* mount1 = bot->FindNearestCreature(NPC_ARGENT_WARHORSE, 100.0f);
    if (!mount1)
        return false;

    Unit* mount2 = bot->FindNearestCreature(NPC_ARGENT_BATTLEWORG, 100.0f);
    if (!mount2)
        return false;

    return true;
}

bool ToCEadricTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "eadric the pure");
    if (!boss)
        return false;

    return true;
}

