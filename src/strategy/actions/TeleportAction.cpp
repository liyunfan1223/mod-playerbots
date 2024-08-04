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

    botAI->TellError("Cannot find any portal to teleport");
    return false;
}
