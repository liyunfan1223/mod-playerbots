/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidActions.h"

#include "Event.h"
#include "Playerbots.h"

NextAction** CastAbolishPoisonAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("cure poison"), nullptr),
                             CastSpellAction::getPrerequisites());
}

NextAction** CastAbolishPoisonOnPartyAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("cure poison on party"), nullptr),
                             CastSpellAction::getPrerequisites());
}

Value<Unit*>* CastEntanglingRootsCcAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", "entangling roots");
}

bool CastEntanglingRootsCcAction::Execute(Event event) { return botAI->CastSpell("entangling roots", GetTarget()); }

Value<Unit*>* CastHibernateCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "hibernate"); }

bool CastHibernateCcAction::Execute(Event event) { return botAI->CastSpell("hibernate", GetTarget()); }

NextAction** CastReviveAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("caster form"), nullptr),
                             ResurrectPartyMemberAction::getPrerequisites());
}

NextAction** CastRebirthAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("caster form"), nullptr),
                             ResurrectPartyMemberAction::getPrerequisites());
}

bool CastRebirthAction::isUseful()
{
    return CastSpellAction::isUseful() &&
           AI_VALUE2(float, "distance", GetTargetName()) <= sPlayerbotAIConfig->spellDistance;
}

Unit* CastRejuvenationOnNotFullAction::GetTarget()
{
    Group* group = bot->GetGroup();
    MinValueCalculator calc(100);
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead() || player->IsFullHealth())
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig->spellDistance)
        {
            continue;
        }
        if (botAI->HasAura("rejuvenation", player))
        {
            continue;
        }
        calc.probe(player->GetHealthPct(), player);
    }
    return (Unit*)calc.param;
}

bool CastRejuvenationOnNotFullAction::isUseful()
{
    return GetTarget();
}