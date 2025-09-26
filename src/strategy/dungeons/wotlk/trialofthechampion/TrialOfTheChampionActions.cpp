#include "Playerbots.h"
#include "TrialOfTheChampionActions.h"
#include "TrialOfTheChampionStrategy.h"
#include "strategy/values/NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Timer.h"
#include "Vehicle.h"
#include "GenericSpellActions.h"
#include "GenericActions.h"
#include <fstream>


bool ToCLanceAction::Execute(Event event)
{
    // If already has lance equipped, do nothing
    if (bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return false;

    // Store current mainhand item
    Item* oldWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    // Search inventory for the lance item
    Item* lanceItem = nullptr;

    // Check main inventory
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && item->GetEntry() == ITEM_LANCE)
        {
            lanceItem = item;
            break;
        }
    }

    // Check bags if not found in main inventory
    if (!lanceItem)
    {
        for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; bag++)
        {
            Bag* pBag = bot->GetBagByPos(bag);
            if (pBag)
            {
                for (uint8 slot = 0; slot < pBag->GetBagSize(); slot++)
                {
                    Item* item = bot->GetItemByPos(bag, slot);
                    if (item && item->GetEntry() == ITEM_LANCE)
                    {
                        lanceItem = item;
                        break;
                    }
                }
            }
            if (lanceItem)
                break;
        }
    }

    // If we found the lance, equip it
    if (lanceItem)
    {
        // Store the lance's current position
        uint8 srcBag = lanceItem->GetBagSlot();
        uint8 srcSlot = lanceItem->GetSlot();

        // First unequip current weapon if it exists
        if (oldWeapon)
        {
            bot->SwapItem(oldWeapon->GetPos(), lanceItem->GetPos());
        }
        else
        {
            bot->EquipItem(EQUIPMENT_SLOT_MAINHAND, lanceItem, true);
        }
        return true;
    }

    if (bot->HasItemCount(ITEM_LANCE, 1))
        return false;

    GameObject* lanceRack = bot->FindNearestGameObject(OBJECT_LANCE_RACK, 100.0f);
    if (!lanceRack)
        return false;

    if (!lanceRack->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
        return MoveTo(lanceRack, INTERACTION_DISTANCE);

    botAI->RemoveShapeshift();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    lanceRack->Use(bot);

    return false;
}

bool ToCUELanceAction::Execute(Event event)
{
    if (!bot->HasItemOrGemWithIdEquipped(ITEM_LANCE, 1))
        return false;

    // Call EquipUpgradeAction
    EquipUpgradeAction equipUpgradeAction(botAI);
    if (!equipUpgradeAction.Execute(event))
        return false;

    return false;
}

bool ToCMountedAction::Execute(Event event)
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    GuidVector attackers = AI_VALUE(GuidVector, "possible targets no los");

    Unit* target = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;
        for (uint32 entry : availableTargets)
        {
            if (unit->GetEntry() == entry)
            {
                target = unit;
                break;
            }
        }
        if (target)
            break;
    }

    Aura* defendBot = botAI->GetAura("defend", bot, false, true);
    if (!defendBot || defendBot->GetStackAmount() < 3)
    {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Defend");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    }

    if (!target)
        return false;

    if (target->GetDistance2d(bot) > 5.0f)
    {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Charge");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    }

    Aura* defendTarget = botAI->GetAura("defend", target, false, false);
    if (!defendTarget)
    {}
    else
    {
        uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Shield-Breaker");
        if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
        {
            vehicleBase->AddSpellCooldown(spellId, 0, 1000);
            return true;
        }
    }

    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", "Thrust");
    if (botAI->CanCastVehicleSpell(spellId, target) && botAI->CastVehicleSpell(spellId, target))
    {
        vehicleBase->AddSpellCooldown(spellId, 0, 1000);
        return true;
    }

    return false;
}

bool ToCMountAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    Unit* vehicleToEnter = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (!vehicleBase->IsFriendlyTo(bot))
            continue;

        if (!vehicleBase->GetVehicleKit() || !vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        uint32 entry = vehicleBase->GetEntry();
        if (entry != NPC_ARGENT_BATTLEWORG && entry != NPC_ARGENT_WARHORSE)
            continue;

        if (!vehicleToEnter || bot->GetExactDist(vehicleToEnter) > bot->GetExactDist(vehicleBase))
            vehicleToEnter = vehicleBase;
    }

    if (!vehicleToEnter)
        return false;

    if (EnterVehicle(vehicleToEnter, true))
        return true;

    return false;
}

bool ToCMountAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = bot->GetDistance(vehicleBase);

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    botAI->RemoveShapeshift();

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool ToCEadricAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "eadric the pure");
    if (!boss)
        return false;

    // If Eadric is casting Radiance, face the opposite direction
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_RADIANCE))
    {
        // Calculate the opposite direction
        float angle = bot->GetAngle(boss);
        float newAngle = Position::NormalizeOrientation(angle + M_PI);  // Add 180 degrees (PI radians)

        // Set the bot's orientation to face away from Eadric
        bot->SetFacingTo(newAngle);
        return true;
    }

    return false;
}
