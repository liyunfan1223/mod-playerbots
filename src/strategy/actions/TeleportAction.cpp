/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TeleportAction.h"

#include "Event.h"
#include "LastMovementValue.h"
#include "Playerbots.h"

bool TeleportAction::Execute(Event event)
{
    // List of allowed portal entries (you can populate this dynamically)
    std::vector<uint32> allowedPortals = { 
        187055, 195142, 195141, 201797, 202079, 194481, 195682, 191164, 176498, 182351, 
        178404, 176497, 181146, 184605, 176499, 195140, 193948, 193427, 193052, 193206, 
        192786, 184594, 183384, 182352, 184604, 189994, 193053, 193207, 193956, 195139, 
        176296, 194011, 194012, 189993, 176500, 176501, 193955, 193425, 193772, 193604, 
        191006, 191007, 191008, 191009, 191013, 191014, 191010, 190960, 191011, 191012, 
        183317, 183321, 183322, 187056, 183323, 183324, 183325, 183326, 183327, 190203, 
        190204, 190205, 190206
    };

    // Try teleporting using allowed portals
    GuidVector closeObjects = *context->GetValue<GuidVector>("nearest game objects no los");
    GameObject* closestPortal = nullptr;
    float closestDistance = 100.0f;

    for (ObjectGuid const& guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        // Check if the game object entry is in the allowed portals list
        if (std::find(allowedPortals.begin(), allowedPortals.end(), go->GetEntry()) != allowedPortals.end())
        {
            float tempDist = bot->GetDistance(go);

            if (tempDist < closestDistance)
            {
                closestDistance = tempDist;
                closestPortal = go;
            }
        }
    }

    // If a nearby portal is found, use it
    if (closestPortal && bot->IsWithinDistInMap(closestPortal, INTERACTION_DISTANCE))
    {
        std::ostringstream out;
        out << "Using portal: " << closestPortal->GetName();
        botAI->TellMasterNoFacing(out.str());

        WorldPacket data(CMSG_GAMEOBJ_USE);
        data << closestPortal->GetGUID();
        bot->GetSession()->HandleGameObjectUseOpcode(data);
        return true;
    }
    // If no portal was found, fallback to spellcaster-type game objects
    GuidVector gos = *context->GetValue<GuidVector>("nearest game objects");
    for (ObjectGuid const guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        GameObjectTemplate const* goInfo = go->GetGOInfo();
        if (goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        uint32 spellId = goInfo->spellcaster.spellId;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_UNITS))
            continue;

        std::ostringstream out;
        out << "Teleporting using " << goInfo->name;
        botAI->TellMasterNoFacing(out.str());

        botAI->ChangeStrategy("-follow,+stay", BOT_STATE_NON_COMBAT);

        Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
        SpellCastTargets targets;
        targets.SetUnitTarget(bot);
        spell->prepare(&targets, nullptr);
        spell->cast(true);
        return true;
    }
    
    // If no game objects were found, try using the last area trigger
    LastMovement& movement = context->GetValue<LastMovement&>("last area trigger")->Get();
    if (movement.lastAreaTrigger)
    {
        WorldPacket p(CMSG_AREATRIGGER);
        p << movement.lastAreaTrigger;
        p.rpos(0);

        bot->GetSession()->HandleAreaTriggerOpcode(p);
        movement.lastAreaTrigger = 0;
        return true;
    }

    // If no teleport option is found
    botAI->TellError("Cannot find any portal to teleport");
    return false;
}
