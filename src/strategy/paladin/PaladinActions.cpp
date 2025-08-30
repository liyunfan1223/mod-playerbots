/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include "AiFactory.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "../../../../../src/server/scripts/Spells/spell_generic.cpp"
#include "GenericBuffUtils.h"
#include "Config.h"
#include "Group.h"
#include "ObjectAccessor.h"
	
using ai::buff::MakeAuraQualifierForBuff;
using ai::buff::UpgradeToGroupIfAppropriate;

// Helper : detect tank role on the target (player bot or not) return true if spec is tank or if the bot have tank strategies (bear/tank/tank face).
static inline bool IsTankRole(Player* p)
{
    if (!p) return false;
    if (p->HasTankSpec())
        return true;
    if (PlayerbotAI* otherAI = GET_PLAYERBOT_AI(p))
    {
        if (otherAI->HasStrategy("tank", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("tank", BOT_STATE_COMBAT)     ||
            otherAI->HasStrategy("tank face", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("tank face", BOT_STATE_COMBAT)     ||
            otherAI->HasStrategy("bear", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("bear", BOT_STATE_COMBAT))
            return true;
    }
    return false;
}


static inline bool GroupHasTankOfClass(Group* g, uint8 klass)
{
    return GroupHasTankOfClass(g, static_cast<Classes>(klass));
}

inline std::string const GetActualBlessingOfMight(Unit* target)
{
    if (!target->ToPlayer())
    {
        return "blessing of might";
    }

    int tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return "blessing of wisdom";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ELEMENTAL || tab == SHAMAN_TAB_RESTORATION)
            {
                return "blessing of wisdom";
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_RESTORATION || tab == DRUID_TAB_BALANCE)
            {
                return "blessing of wisdom";
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
            {
                return "blessing of wisdom";
            }
            break;
    }

    return "blessing of might";
}

inline std::string const GetActualBlessingOfWisdom(Unit* target)
{
    if (!target->ToPlayer())
    {
        return "blessing of might";
    }
    int tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        case CLASS_HUNTER:
            return "blessing of might";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
            {
                return "blessing of might";
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
            {
                return "blessing of might";
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_PROTECTION || tab == PALADIN_TAB_RETRIBUTION)
            {
                return "blessing of might";
            }
            break;
    }

    return "blessing of wisdom";
}

inline std::string const GetActualBlessingOfSanctuary(Unit* target, Player* bot)
{
    if (!bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY))
        return "";

    Player* tp = target->ToPlayer();
    if (!tp)
        return "";

    if (auto* ai = GET_PLAYERBOT_AI(bot))
    {
        if (Unit* mt = ai->GetAiObjectContext()->GetValue<Unit*>("main tank")->Get())
        {
            if (mt == target)
                return "blessing of sanctuary";
        }
    }

    if (tp->HasTankSpec())
        return "blessing of sanctuary";

    return "";
}

Value<Unit*>* CastBlessingOnPartyAction::GetTargetValue()
{

	return context->GetValue<Unit*>("party member without aura", MakeAuraQualifierForBuff(spell));
}

bool CastBlessingOfMightAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfMight(target);	
    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfMightOnPartyAction::GetTargetValue()
{
	return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of might,greater blessing of might,blessing of wisdom,greater blessing of wisdom,blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfMightOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfMight(target);
    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

bool CastBlessingOfWisdomAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfWisdom(target);	
    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfWisdomOnPartyAction::GetTargetValue()
{
	return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of wisdom,greater blessing of wisdom,blessing of might,greater blessing of might,blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfWisdomOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (Group* g = bot->GetGroup())
        if (Player* tp = target->ToPlayer())
            if (!g->IsMember(tp->GetGUID()))
                return false;

    if (botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT))
        if (Player* tp = target->ToPlayer())
            // if (tp->HasTankSpec())
			if (IsTankRole(tp))
            {
                LOG_DEBUG("playerbots", "[PallyBuff][Wisdom/bmana] Skip tank {} (Kings only)", target->GetName());
                return false;
            }

    std::string castName = GetActualBlessingOfWisdom(target);
    if (castName.empty())
        return false;

    auto RP = ai::chat::MakeGroupAnnouncer(bot);
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}


Value<Unit*>* CastBlessingOfSanctuaryOnPartyAction::GetTargetValue()
{
	return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfSanctuaryOnPartyAction::Execute(Event event)
{
    if (!bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY)) return false;

    Unit* target = GetTarget();
    if (!target)
    target = bot;

    if (Group* g = bot->GetGroup())
        if (Player* tp = target->ToPlayer())
            if (!g->IsMember(tp->GetGUID()))
            {
                LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Initial target not in group, ignoring");
                target = bot;
            }

    if (Player* self = bot->ToPlayer())
    {
        bool selfHasSanct = botAI->HasAura("blessing of sanctuary", self) ||
                            botAI->HasAura("greater blessing of sanctuary", self);
        // bool needSelf = self->HasTankSpec() && !selfHasSanct;
		bool needSelf = IsTankRole(self) && !selfHasSanct;

        LOG_DEBUG("playerbots", "[PallyBuff][Sanct] {} isTank={} selfHasSanct={} needSelf={}",
                 //bot->GetName(), self->HasTankSpec(), selfHasSanct, needSelf);
				 bot->GetName(), IsTankRole(self), selfHasSanct, needSelf);

        if (needSelf)
            target = self;
    }

    {
        bool targetOk = false;
        if (Player* tp = target->ToPlayer())
        {
            bool hasSanct = botAI->HasAura("blessing of sanctuary", tp) ||
                            botAI->HasAura("greater blessing of sanctuary", tp);
            //targetOk = tp->HasTankSpec() && !hasSanct;
			targetOk = IsTankRole(tp) && !hasSanct;
        }

        if (!targetOk)
        {
            if (Group* g = bot->GetGroup())
            {
                for (auto const& slot : g->GetMemberSlots())
                {
                    if (Player* p = ObjectAccessor::FindPlayer(slot.guid))
                    {
                        if (!g->IsMember(p->GetGUID())) continue;
                        if (!p->IsInWorld() || !p->IsAlive()) continue;
                        // if (!p->HasTankSpec()) continue;
						if (!IsTankRole(p)) continue;

                        bool hasSanct = botAI->HasAura("blessing of sanctuary", p) ||
                                        botAI->HasAura("greater blessing of sanctuary", p);
                        if (!hasSanct)
                        {
                            target = p; // <- buffera ce tank en priorité
                            break;
                        }
                    }
                }
            }
        }
    }

    {
        bool hasKings = botAI->HasAura("blessing of kings", target) ||
                        botAI->HasAura("greater blessing of kings", target);
        bool hasSanct = botAI->HasAura("blessing of sanctuary", target) ||
                        botAI->HasAura("greater blessing of sanctuary", target);
        bool knowSanct = bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
        LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Final target={} hasKings={} hasSanct={} knowSanct={}",
                 target->GetName(), hasKings, hasSanct, knowSanct);
    }

    /*std::string castName = GetActualBlessingOfSanctuary(target, bot);
    if (castName.empty())
        return false;*/

    std::string castName = GetActualBlessingOfSanctuary(target, bot);
    // Si la logique interne ne reconnaît pas le tank (ex: druide ours), on force Sanctuary mono
    if (castName.empty())
    {
        if (Player* tp = target->ToPlayer())
        {
            if (IsTankRole(tp))
                castName = "blessing of sanctuary"; // force mono
            else
                return false;
        }
        else
            return false;
    }

    if (Player* tp = target->ToPlayer())
    {
        // if (tp->HasTankSpec())
		if (IsTankRole(tp))
        {
            castName = "blessing of sanctuary";
        }
        else
        {
            auto RP = ai::chat::MakeGroupAnnouncer(bot);
            castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
        }
    }
    else
    {
        castName = "blessing of sanctuary";
    }
    bool ok = botAI->CastSpell(castName, target);
    LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Cast {} on {} result={}", castName, target->GetName(), ok);
    return ok;
}

Value<Unit*>* CastBlessingOfKingsOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of kings,greater blessing of kings"
    );
}

bool CastBlessingOfKingsOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    Group* g = bot->GetGroup();
    if (!g)
        return false;
    if (Player* tp = target->ToPlayer())
        if (!g->IsMember(tp->GetGUID()))
            return false;
		
    if (botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT))
    {
        Player* tp = target->ToPlayer();
        // if (!tp || !tp->HasTankSpec())
		if (!tp || !IsTankRole(tp))
        {
            LOG_DEBUG("playerbots", "[PallyBuff][Kings/bmana] Skip non-tank {}", target->GetName());
            return false;
        }
    }
 
    if (Player* tp = target->ToPlayer())
    {
        // const bool isTank = tp->HasTankSpec();
		const bool isTank = IsTankRole(tp);
        const bool hasSanctFromMe =
            target->HasAura(SPELL_BLESSING_OF_SANCTUARY, bot->GetGUID()) ||
            target->HasAura(SPELL_GREATER_BLESSING_OF_SANCTUARY, bot->GetGUID());
        const bool hasSanctAny =
            botAI->HasAura("blessing of sanctuary", target) ||
            botAI->HasAura("greater blessing of sanctuary", target);

        if (isTank && hasSanctFromMe)
        {
            LOG_DEBUG("playerbots", "[PallyBuff][Kings] Skip: {} has my Sanctuary and is a tank", target->GetName());
            return false;
        }

        if (botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT) && isTank && hasSanctAny)
        {
            LOG_DEBUG("playerbots", "[PallyBuff][Kings] Skip (bstats): {} already has Sanctuary and is a tank", target->GetName());
            return false;
        }
    }

    std::string castName = "blessing of kings";

    bool allowGreater = true;

    if (botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT))
        allowGreater = false;

    if (allowGreater && botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT))
    {
        if (Player* tp = target->ToPlayer())
        {
            switch (tp->getClass())
            {
                case CLASS_WARRIOR:
                case CLASS_PALADIN:
                case CLASS_DRUID:
                case CLASS_DEATH_KNIGHT:
                    allowGreater = false;
                    break;
                default:
                    break;
            }
        }
    }

    if (allowGreater)
    {
        auto RP = ai::chat::MakeGroupAnnouncer(bot);
        castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    }
	
    return botAI->CastSpell(castName, target);
}

bool CastSealSpellAction::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

Value<Unit*>* CastTurnUndeadAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
    {
        return NULL;
    }
    return current_target->GetVictim();
}


bool CastDivineSacrificeAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() &&
           !botAI->HasAura("divine guardian", GetTarget(), false, false, -1, true);
}

bool CastCancelDivineSacrificeAction::Execute(Event event)
{
    botAI->RemoveAura("divine sacrifice");
    return true;
}

bool CastCancelDivineSacrificeAction::isUseful()
{
    return botAI->HasAura("divine sacrifice", GetTarget(), false, true, -1, true);
}
