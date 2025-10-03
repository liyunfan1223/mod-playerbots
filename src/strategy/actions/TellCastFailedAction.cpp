/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellCastFailedAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "Playerbots.h"

bool TellCastFailedAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 castCount, result;
    uint32 spellId;
    p >> castCount >> spellId >> result;
    botAI->SpellInterrupted(spellId);

    if (result == SPELL_CAST_OK)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

    std::ostringstream out;
    out << chat->FormatSpell(spellInfo) << ": ";
    switch (result)
    {
        case SPELL_FAILED_NOT_READY:
            out << "not ready";
            break;
        case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
            out << "requires spell focus";
            break;
        case SPELL_FAILED_REQUIRES_AREA:
            out << "cannot cast here";
            break;
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
            out << "requires item";
            break;
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND:
            out << "requires weapon";
            break;
        case SPELL_FAILED_PREVENTED_BY_MECHANIC:
            out << "interrupted";
            break;
        default:
            out << "cannot cast";
    }

    if (spellInfo->CalcCastTime() >= 2000)
        botAI->TellError(out.str());

    return true;
}

bool TellSpellAction::Execute(Event event)
{
    std::string const spell = event.getParam();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    if (!spellId)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    std::ostringstream out;
    out << chat->FormatSpell(spellInfo);
    botAI->TellError(out.str());
    return true;
}
