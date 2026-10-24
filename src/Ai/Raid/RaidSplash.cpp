/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidSplash.h"

#include "AiObjectContext.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

std::set<std::string> const MULTI_TARGET_ACTIONS = {
    "bladestorm",    "sweeping strikes", "cleave",        "whirlwind",  "thunder clap",
    "shockwave",     "divine storm",     "howling blast", "blood boil", "fan of knives",
    "blade flurry",  "swipe (cat)",      "multi-shot",    "fire nova",  "chain lightning"};

bool SpellSplashes(PlayerbotAI* botAI, Action* action)
{
    // the area is on what these summon, the summon spell itself is single target
    static std::set<std::string> const summoners = {"magma totem", "fire nova", "explosive trap"};
    if (summoners.count(action->getName()))
        return true;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction || dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    uint32 const spellId =
        botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellAction->getSpell())->Get();
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    return spellInfo && (spellInfo->IsTargetingArea() || spellInfo->IsAffectingArea());
}
