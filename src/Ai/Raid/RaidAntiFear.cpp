/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidAntiFear.h"

#include "EncounterHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "ShamanActions.h"
#include "ShamanTriggers.h"

using namespace EncounterHelpers;

namespace
{
// CanCastSpell treats out of range as castable, so the range check keeps a far away target from being
// picked and never reached
bool CanWardTarget(PlayerbotAI* botAI, Player* bot, Player* target)
{
    return target && target->IsAlive() && !target->HasAura(SPELL_FEAR_WARD) &&
           bot->IsWithinDistInMap(target, sPlayerbotAIConfig.SpellDistance) &&
           botAI->CanCastSpell("fear ward", target);
}
}  // namespace

Player* GetAntiFearWardTarget(PlayerbotAI* botAI, Player* bot)
{
    if (bot->getClass() != CLASS_PRIEST)
        return nullptr;

    // cheap gate before the group walk, each CanCastSpell below runs a full CheckCast
    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", "fear ward")->Get();
    if (!spellId || bot->HasSpellCooldown(spellId))
        return nullptr;

    Player* mainTank = GetGroupMainTank(bot);
    if (CanWardTarget(botAI, bot, mainTank))
        return mainTank;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == mainTank || !PlayerbotAI::IsHeal(member))
            continue;

        if (CanWardTarget(botAI, bot, member))
            return member;
    }

    return nullptr;
}

bool ShouldDropTremorTotem(PlayerbotAI* botAI, Player* bot)
{
    if (bot->getClass() != CLASS_SHAMAN || !bot->HasSpell(SPELL_TREMOR_TOTEM_RANK_1))
        return false;

    if (botAI->GetAiObjectContext()->GetValue<bool>("has totem", "tremor totem")->Get())
        return false;

    return botAI->CanCastSpell("tremor totem", bot);
}

bool RaidAntiFearReady(PlayerbotAI* botAI, Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            return GetAntiFearWardTarget(botAI, bot) != nullptr;
        case CLASS_SHAMAN:
            return ShouldDropTremorTotem(botAI, bot);
        default:
            return false;
    }
}

bool RaidAntiFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    return FearWindowActive() && RaidAntiFearReady(botAI, bot);
}

bool RaidAntiFearAction::Execute(Event /*event*/)
{
    if (bot->getClass() == CLASS_PRIEST)
    {
        Player* target = GetAntiFearWardTarget(botAI, bot);
        return target && botAI->CastSpell("fear ward", target);
    }

    if (bot->getClass() == CLASS_SHAMAN)
        return ShouldDropTremorTotem(botAI, bot) && botAI->CastSpell("tremor totem", bot);

    return false;
}

bool RaidAntiFearAction::isUseful()
{
    if (bot->getClass() != CLASS_PRIEST && bot->getClass() != CLASS_SHAMAN)
        return false;

    return FearWindowActive() && RaidAntiFearReady(botAI, bot);
}

// One earth totem at a time, so Tremor only stays down while the shaman's other earth totems are held
float RaidAntiFearTotemGuardMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    // only the actions that could take the slot pay for the encounter check
    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) && !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) && !dynamic_cast<CastEarthbindTotemAction*>(action))
        return 1.0f;

    return FearWindowActive() ? 0.0f : 1.0f;
}
