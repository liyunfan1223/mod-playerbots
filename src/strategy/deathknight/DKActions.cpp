/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DKActions.h"
#include "Duration.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

NextAction** CastDeathchillAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("frost presence"), nullptr), CastSpellAction::getPrerequisites());
}

NextAction** CastUnholyMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("unholy presence"), nullptr), CastMeleeSpellAction::getPrerequisites());
}

NextAction** CastFrostMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("frost presence"), nullptr), CastMeleeSpellAction::getPrerequisites());
}

NextAction** CastBloodMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("blood presence"), nullptr), CastMeleeSpellAction::getPrerequisites());
}

bool CastRaiseDeadAction::Execute(Event event)
{
    bool result = CastBuffSpellAction::Execute(event);
    if (!result) {
        return false;
    }
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    // const SpellInfo *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    bot->AddSpellCooldown(spellId, 0, 3 * 60 * 1000);
    return true;
}