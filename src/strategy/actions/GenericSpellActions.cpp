/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericSpellActions.h"
#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"

CastSpellAction::CastSpellAction(PlayerbotAI* botAI, std::string const spell) : Action(botAI, spell), range(botAI->GetRange("spell")), spell(spell)
{
}

bool CastSpellAction::Execute(Event event)
{
    if (spell == "conjure food" || spell == "conjure water")
    {
        //uint32 id = AI_VALUE2(uint32, "spell id", spell);
        //if (!id)
        //    return false;

        uint32 castId = 0;

        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            uint32 spellId = itr->first;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            std::string const namepart = spellInfo->SpellName[0];
            std::wstring wnamepart;
            if (!Utf8toWStr(namepart, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (!Utf8FitTo(spell, wnamepart))
                continue;

            if (spellInfo->Effects[0].Effect != SPELL_EFFECT_CREATE_ITEM)
                continue;

            uint32 itemId = spellInfo->Effects[0].ItemType;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
                continue;

            if (spellInfo->Id > castId)
                castId = spellInfo->Id;
        }

        return botAI->CastSpell(castId, bot);
    }

    return botAI->CastSpell(spell, GetTarget());
}

bool CastSpellAction::isPossible()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true)) {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster())) {
            LOG_DEBUG("playerbots", "Can cast spell failed. Vehicle. - bot name: {}", 
                bot->GetName());
        }
        return false;
    }

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster())) {
            LOG_DEBUG("playerbots", "Can cast spell failed. Mount. - bot name: {}", 
                bot->GetName());
        }
        bot->Dismount();
        return false;
    }

    Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return botAI->CanCastSpell(spell, GetTarget());
}

bool CastSpellAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    if (spell == "mount" && !bot->IsMounted() && !bot->IsInCombat())
        return true;

    if (spell == "mount" && bot->IsInCombat())
    {
        bot->Dismount();
        return false;
    }

    Unit* spellTarget = GetTarget();
    if (!spellTarget)
        return false;

    if (!spellTarget->IsInWorld() || spellTarget->GetMapId() != bot->GetMapId())
        return false;

    // float combatReach = bot->GetCombatReach() + spellTarget->GetCombatReach();
    // if (!botAI->IsRanged(bot))
    //     combatReach += 4.0f / 3.0f;

    return spellTarget && AI_VALUE2(bool, "spell cast useful", spell); // && sServerFacade->GetDistance2d(bot, spellTarget) <= (range + combatReach);
}

CastMeleeSpellAction::CastMeleeSpellAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = ATTACK_DISTANCE;
    // Unit* target = AI_VALUE(Unit*, "current target");
    // if (target)
    //     range = bot->GetMeleeRange(target);

    // range = target->GetCombinedCombatReach();
}

bool CastAuraSpellAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() && !botAI->HasAura(spell, GetTarget(), false, isOwner);
}

CastEnchantItemAction::CastEnchantItemAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = botAI->GetRange("spell");
}

bool CastEnchantItemAction::isPossible()
{
    // if (!CastSpellAction::isPossible()) {
        
    //     botAI->TellMasterNoFacing("Impossible: " + spell);
    //     return false;
    // }

    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    
    // bool ok = AI_VALUE2(Item*, "item for spell", spellId);
    // Item* item = AI_VALUE2(Item*, "item for spell", spellId);
    // botAI->TellMasterNoFacing("spell: " + spell + ", spell id: " + std::to_string(spellId) + " item for spell: " + std::to_string(ok));
    return spellId && AI_VALUE2(Item*, "item for spell", spellId);
}

CastHealingSpellAction::CastHealingSpellAction(PlayerbotAI* botAI, std::string const spell, uint8 estAmount) : CastAuraSpellAction(botAI, spell, true), estAmount(estAmount)
{
    range = botAI->GetRange("heal");
}

bool CastHealingSpellAction::isUseful()
{
	return CastAuraSpellAction::isUseful();
}

bool CastAoeHealSpellAction::isUseful()
{
	return CastSpellAction::isUseful();
}

CastCureSpellAction::CastCureSpellAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell)
{
    range = botAI->GetRange("heal");
}

Value<Unit*>* CurePartyMemberAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member to dispel", dispelType);
}

Value<Unit*>* BuffOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

CastShootAction::CastShootAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shoot")
{
    if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        spell = "shoot";

        switch (pItem->GetTemplate()->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_GUN:
                spell += " gun";
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
                spell += " bow";
                break;
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                spell += " crossbow";
                break;
        }
    }
}

NextAction** CastSpellAction::getPrerequisites()
{
    if (spell == "mount")
        return nullptr;

    if (range > botAI->GetRange("spell"))
        return nullptr;
    else if (range > ATTACK_DISTANCE)
        return NextAction::merge(NextAction::array(0, new NextAction("reach spell"), nullptr), Action::getPrerequisites());
    else
        return NextAction::merge(NextAction::array(0, new NextAction("reach melee"), nullptr), Action::getPrerequisites());
}

Value<Unit*>* CastDebuffSpellOnAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("attacker without aura", spell);
}

Value<Unit*>* CastDebuffSpellOnMeleeAttackerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("melee attacker without aura", spell);
}

CastBuffSpellAction::CastBuffSpellAction(PlayerbotAI* botAI, std::string const spell, bool checkIsOwner) : CastAuraSpellAction(botAI, spell, checkIsOwner)
{
    range = botAI->GetRange("spell");
}

Value<Unit*>* CastSpellOnEnemyHealerAction::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

Value<Unit*>* CastSnareSpellAction::GetTargetValue()
{
    return context->GetValue<Unit*>("snare target", spell);
}

Value<Unit*>* CastCrowdControlSpellAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", getName());
}

bool CastCrowdControlSpellAction::Execute(Event event)
{
    return botAI->CastSpell(getName(), GetTarget());
}

bool CastCrowdControlSpellAction::isPossible()
{
    return botAI->CanCastSpell(getName(), GetTarget());
}

bool CastCrowdControlSpellAction::isUseful()
{
    return true;
}

std::string const CastProtectSpellAction::GetTargetName()
{
    return "party member to protect";
}

bool CastProtectSpellAction::isUseful()
{
    return GetTarget() && !botAI->HasAura(spell, GetTarget());
}

bool CastVehicleSpellAction::isPossible()
{
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CanCastVehicleSpell(spellId, GetTarget());
}

bool CastVehicleSpellAction::isUseful()
{
    return botAI->IsInVehicle(false, true);
}

bool CastVehicleSpellAction::Execute(Event event)
{
    uint32 spellId = AI_VALUE2(uint32, "vehicle spell id", spell);
    return botAI->CastVehicleSpell(spellId, GetTarget());
}

Value<Unit*>* BuffOnMainTankAction::GetTargetValue()
{
    return context->GetValue<Unit*>("main tank", spell);
}

bool CastDebuffSpellAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld()) {
        return false;
    }
    return CastAuraSpellAction::isUseful() && (target->GetHealth() / AI_VALUE(float, "expected group dps")) >= needLifeTime;
}