/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MageActions.h"
#include <cmath>
#include "UseItemAction.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"

Value<Unit*>* CastPolymorphAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool UseManaSapphireAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(33312, false) > 0;  // Mana Sapphire
}

bool UseManaEmeraldAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(22044, false) > 0;  // Mana Emerald
}

bool UseManaRubyAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(8008, false) > 0;  // Mana Ruby
}

bool UseManaCitrineAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(8007, false) > 0;  // Mana Citrine
}

bool UseManaJadeAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(5513, false) > 0;  // Mana Jade
}

bool UseManaAgateAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(5514, false) > 0;  // Mana Agate
}

bool CastFrostNovaAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsInWorld())
        return false;

    if (target->ToCreature() && target->ToCreature()->HasMechanicTemplateImmunity(1 << (MECHANIC_FREEZE - 1)))
        return false;

    if (target->isFrozen())
        return false;

    return sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
}

bool CastConeOfColdAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
    return facingTarget && targetClose;
}

bool CastDragonsBreathAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return facingTarget && targetClose;
}

bool CastBlastWaveAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return targetClose;
}

Unit* CastFocusMagicOnPartyAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* casterDps = nullptr;
    Unit* healer = nullptr;
    Unit* target = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->GetMap() != bot->GetMap() || bot->GetDistance(member) > sPlayerbotAIConfig->spellDistance)
            continue;

        if (member->HasAura(54646))
            continue;

        if (member->getClass() == CLASS_MAGE)
            return member;

        if (!casterDps && botAI->IsCaster(member) && botAI->IsDps(member))
            casterDps = member;

        if (!healer && botAI->IsHeal(member))
            healer = member;

        if (!target)
            target = member;
    }

    if (casterDps)
        return casterDps;

    if (healer)
        return healer;

    return target;
}

bool CastBlinkBackAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    // can cast spell check passed in isUseful()
    bot->SetOrientation(bot->GetAngle(target) + M_PI);
    return CastSpellAction::Execute(event);
}
