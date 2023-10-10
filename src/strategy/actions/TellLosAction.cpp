/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellLosAction.h"
#include "Event.h"
#include "ChatHelper.h"
#include "Playerbots.h"

bool TellLosAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits("--- Targets ---", *context->GetValue<GuidVector>("possible targets"));
        ListUnits("--- Targets (All) ---", *context->GetValue<GuidVector>("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits("--- NPCs ---", *context->GetValue<GuidVector>("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits("--- Corpses ---", *context->GetValue<GuidVector>("nearest corpses"));
    }

    if (param.empty() || param == "gos" || param == "game objects")
    {
        ListGameObjects("--- Game objects ---", *context->GetValue<GuidVector>("nearest game objects"));
    }

    if (param.empty() || param == "players")
    {
        ListUnits("--- Friendly players ---", *context->GetValue<GuidVector>("nearest friendly players"));
    }

    return true;
}

void TellLosAction::ListUnits(std::string const title, GuidVector units)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : units)
    {
        if (Unit* unit = botAI->GetUnit(guid))
            botAI->TellMaster(unit->GetName());
    }

}
void TellLosAction::ListGameObjects(std::string const title, GuidVector gos)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            botAI->TellMaster(chat->FormatGameobject(go));
    }
}

bool TellAuraAction::Execute(Event event)
{
    botAI->TellMaster("--- Auras ---");
    sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "--- Auras ---");
    Unit::AuraApplicationMap& map = bot->GetAppliedAuras();
	for (Unit::AuraApplicationMap::iterator i = map.begin(); i != map.end(); ++i)
	{
		Aura * aura  = i->second->GetBase();
		if (!aura)
			continue;

		const std::string auraName = aura->GetSpellInfo()->SpellName[0];
        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "Info of Aura - name: " + auraName);
		AuraObjectType type = aura->GetType();
		WorldObject* owner = aura->GetOwner();
        std::string owner_name = owner ? owner->GetName() : "unknown";
        float distance = bot->GetDistance2d(owner);
        Unit* caster = aura->GetCaster();
        std::string caster_name = caster ? caster->GetName() : "unknown";
        bool is_area = aura->IsArea();
        int32 duration = aura->GetDuration();
        const SpellInfo* spellInfo = aura->GetSpellInfo();
        int32 spellId = aura->GetSpellInfo()->Id;
        bool isPositive = aura->GetSpellInfo()->IsPositive();
        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "Info of Aura - name: " + auraName +
                       " caster: " + caster_name + 
                       " type: " + std::to_string(type) + 
                       " owner: " + owner_name + 
                       " distance: " + std::to_string(distance) +
                       " isArea: " + std::to_string(is_area) +
                       " duration: " + std::to_string(duration) +
                       " spellId: " + std::to_string(spellId) +
                       " isPositive: " + std::to_string(isPositive));

		botAI->TellMaster("Info of Aura - name: " + auraName +
                       " caster: " + caster_name + 
                       " type: " + std::to_string(type) + 
                       " owner: " + owner_name + 
                       " distance: " + std::to_string(distance) +
                       " isArea: " + std::to_string(is_area) +
                       " duration: " + std::to_string(duration) +
                       " spellId: " + std::to_string(spellId) +
                       " isPositive: " + std::to_string(isPositive));

        if (type == DYNOBJ_AURA_TYPE) {
            DynamicObject* dyn_owner = aura->GetDynobjOwner();
            float radius = dyn_owner->GetRadius();
            int32 spellId = dyn_owner->GetSpellId();
            int32 duration = dyn_owner->GetDuration();
            sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, std::string("Info of DynamicObject -") +
                           " name: " + dyn_owner->GetName() + 
                           " radius: " + std::to_string(radius) +
                           " spell id: " + std::to_string(spellId) +
                           " duration: " + std::to_string(duration));

            botAI->TellMaster(std::string("Info of DynamicObject -") +
                           " name: " + dyn_owner->GetName() + 
                           " radius: " + std::to_string(radius) +
                           " spell id: " + std::to_string(spellId) +
                           " duration: " + std::to_string(duration));
        }
	}
	return true;
}

bool TellExpectedDpsAction::Execute(Event event)
{
    float dps = AI_VALUE(float, "expected group dps");
    botAI->TellMaster("Expected Group DPS: " + std::to_string(dps));
	return true;
}