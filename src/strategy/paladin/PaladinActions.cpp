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
#include "Log.h"
	
using ai::buff::MakeAuraQualifierForBuff;
using ai::buff::UpgradeToGroupIfAppropriate;

// Paladin — special logic "2 paladins (bstats + bmana)"
namespace {
    inline bool TwoPallyLogicEnabled()
    {
        return sConfigMgr->GetOption<bool>("AiPlayerbot.Paladin.TwoPallyKingsSanctuary", false);
    }

    inline bool IsTwoPaladinsBstatsAndSelfBmana(Player* me)
    {
        Group* g = me->GetGroup();
        if (!g) return false;
        uint32 palCount = 0;
        bool otherHasBstats = false, selfHasBmana = false;
        for (auto const& slot : g->GetMemberSlots())
        {
            Player* p = ObjectAccessor::FindPlayer(slot.guid);
            if (!p || !p->IsInWorld() || p->getClass() != CLASS_PALADIN)
                continue;
            ++palCount;
            if (p == me)
            {
                if (PlayerbotAI* ai = GET_PLAYERBOT_AI(p))
                    selfHasBmana = ai->HasStrategy("bmana", BOT_STATE_NON_COMBAT);
            }
            else if (!otherHasBstats)
            {
                if (PlayerbotAI* ai = GET_PLAYERBOT_AI(p))
                    otherHasBstats = ai->HasStrategy("bstats", BOT_STATE_NON_COMBAT);
            }
        }

    bool ok = palCount == 2 && selfHasBmana && otherHasBstats;
    LOG_DEBUG("playerbots", "[PallyBuff] Check self=bmana/other=bstats -> palCount={}, selfHasBmana={}, otherHasBstats={}, OK={}",
             palCount, selfHasBmana, otherHasBstats, ok);
    return ok;		
    }

    inline bool IsTwoPaladinsSelfBstatsOtherBmana(Player* me)
    {
        Group* g = me->GetGroup();
        if (!g) return false;
        uint32 palCount = 0;
        bool selfHasBstats = false, otherHasBmana = false;
        for (auto const& slot : g->GetMemberSlots())
        {
            Player* p = ObjectAccessor::FindPlayer(slot.guid);
            if (!p || !p->IsInWorld() || p->getClass() != CLASS_PALADIN)
                continue;
            ++palCount;
            if (p == me)
            {
                if (PlayerbotAI* ai = GET_PLAYERBOT_AI(p))
                    selfHasBstats = ai->HasStrategy("bstats", BOT_STATE_NON_COMBAT);
            }
            else if (!otherHasBmana)
            {
                if (PlayerbotAI* ai = GET_PLAYERBOT_AI(p))
                    otherHasBmana = ai->HasStrategy("bmana", BOT_STATE_NON_COMBAT);
            }
        }

    bool ok = palCount == 2 && selfHasBstats && otherHasBmana;
    LOG_DEBUG("playerbots", "[PallyBuff] Check self=bstats/other=bmana -> palCount={}, selfHasBstats={}, otherHasBmana={}, OK={}",
             palCount, selfHasBstats, otherHasBmana, ok);
    return ok;

    }	
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
    // Paladin doesn’t have Sanctuary we do nothing
    if (!bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY))
        return "";

    // Do not buff non-players (pets/vehicles)
    Player* tp = target->ToPlayer();
    if (!tp)
        return "";

    // If the target is the Main Tank, apply Sanctuary
    if (auto* ai = GET_PLAYERBOT_AI(bot))
    {
        if (Unit* mt = ai->GetAiObjectContext()->GetValue<Unit*>("main tank")->Get())
        {
            if (mt == target)
                return "blessing of sanctuary";
        }
    }

    // Otherwise, standard logic: only on tanks
    if (tp->HasTankSpec())
        return "blessing of sanctuary";

    // No tank -> do nothing (Kings will be handled by the Kings action)
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

    // Paladin — special logic "2 paladins (bstats + bmana)"
	if (TwoPallyLogicEnabled() && IsTwoPaladinsBstatsAndSelfBmana(bot))	
    {
        if (Group* g = bot->GetGroup())
        {
            for (auto const& slot : g->GetMemberSlots())
            {
                Player* p = ObjectAccessor::FindPlayer(slot.guid);
                if (!p || !p->IsInWorld()) continue;
                if (!g->IsMember(p->GetGUID())) continue;

                Player* pp = p->ToPlayer();
                if (!pp || !pp->HasTankSpec()) continue;

                bool hasKings = botAI->HasAura("blessing of kings", p) ||
                                botAI->HasAura("greater blessing of kings", p);
                bool hasSanct = botAI->HasAura("blessing of sanctuary", p) ||
                                botAI->HasAura("greater blessing of sanctuary", p);

                // NEVER replace Sanctuary with Kings; and do not duplicate Kings
                if (!hasKings && !hasSanct)
                {
                    LOG_DEBUG("playerbots", "[PallyBuff][Bmana] Cast KING (mono) on tank={} (no Kings & no Sanctuary)",
                             p->GetName());
                    return botAI->CastSpell("blessing of kings", p); // intentional single-target
                }
            }
        }
        if (Player* tp = target->ToPlayer())
        {
            if (tp->HasTankSpec())
            {
                LOG_DEBUG("playerbots", "[PallyBuff][Bmana] Skip buffing tank={} with Might/Wisdom (2-pal setup)",
                         tp->GetName());
                return false;
            }
        }
    }

    std::string castName = GetActualBlessingOfWisdom(target);
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

    // If the initial target is not in MY group, ignore it.
    if (Group* g = bot->GetGroup())
        if (Player* tp = target->ToPlayer())
            if (!g->IsMember(tp->GetGUID()))
            {
                LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Initial target not in group, ignoring");
                target = bot;
            }

    // Auto-buff: if I am the tank and without Sanctuary, target myself first
    if (Player* self = bot->ToPlayer())
    {
        bool selfHasSanct = botAI->HasAura("blessing of sanctuary", self) ||
                            botAI->HasAura("greater blessing of sanctuary", self);
        bool needSelf = self->HasTankSpec() && !selfHasSanct;

        LOG_DEBUG("playerbots", "[PallyBuff][Sanct] {} isTank={} selfHasSanct={} needSelf={}",
                 bot->GetName(), self->HasTankSpec(), selfHasSanct, needSelf);

        if (needSelf)
            target = self;
    }

    // Paladin — special logic "2 paladins (bstats + bmana)"
    if (TwoPallyLogicEnabled() && IsTwoPaladinsSelfBstatsOtherBmana(bot))
    {
        if (auto* ai = GET_PLAYERBOT_AI(bot))
        {
            // Main Tank prioritized if missing Sanctuary (Kings present does not block Sanctuary)
            if (Unit* mt = ai->GetAiObjectContext()->GetValue<Unit*>("main tank")->Get())
            {
                Player* mtp = mt->ToPlayer();
                bool mtHasSanct = botAI->HasAura("blessing of sanctuary", mt) ||
                                  botAI->HasAura("greater blessing of sanctuary", mt);
                bool mtNeed = (mtp && mtp->HasTankSpec() && !mtHasSanct);

                LOG_DEBUG("playerbots", "[PallyBuff][Sanct] MT={} needSanct={}",
                         mtp ? mtp->GetName() : "<null>", mtNeed);

                if (mtNeed)
                    target = mt;
            }

            // Otherwise, look for another tank in the group without Sanctuary
            Player* tp = target->ToPlayer();
            bool targetIsTank = (tp && tp->HasTankSpec());
            bool targetHasSanct = botAI->HasAura("blessing of sanctuary", target) ||
                                  botAI->HasAura("greater blessing of sanctuary", target);

            if (!targetIsTank || targetHasSanct)
            {
                if (Group* g = bot->GetGroup())
                {
                    for (auto const& slot : g->GetMemberSlots())
                    {
                        Player* p = ObjectAccessor::FindPlayer(slot.guid);
                        if (!p || !p->IsInWorld()) continue;
                        if (!g->IsMember(p->GetGUID())) continue;

                        Player* pp = p->ToPlayer();
                        if (!pp || !pp->HasTankSpec()) continue;

                        bool need = !botAI->HasAura("blessing of sanctuary", p) &&
                                    !botAI->HasAura("greater blessing of sanctuary", p);

                        if (need)
                        {
                            LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Fallback tank selected={}", p->GetName());
                            target = p;
                            break;
                        }
                    }
                }
            }
        }
    }

    // LOG: befor cast
    {
        bool hasKings = botAI->HasAura("blessing of kings", target) ||
                        botAI->HasAura("greater blessing of kings", target);
        bool hasSanct = botAI->HasAura("blessing of sanctuary", target) ||
                        botAI->HasAura("greater blessing of sanctuary", target);
        bool knowSanct = bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
        LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Final target={} hasKings={} hasSanct={} knowSanct={}",
                 target->GetName(), hasKings, hasSanct, knowSanct);
    }

    std::string castName = GetActualBlessingOfSanctuary(target, bot);
    if (castName.empty())
        return false;

    auto RP = ai::chat::MakeGroupAnnouncer(bot);
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    bool ok = botAI->CastSpell(castName, target);
    LOG_DEBUG("playerbots", "[PallyBuff][Sanct] Cast {} on {} result={}", castName, target->GetName(), ok);
    return ok;
}

Value<Unit*>* CastBlessingOfKingsOnPartyAction::GetTargetValue()
{
    // Never overwrite Sanctuary: if the target already has Sanctuary (single or greater), skip it
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of kings,greater blessing of kings,blessing of sanctuary,greater blessing of sanctuary"
    );
}

bool CastBlessingOfKingsOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Paladin — special logic "2 paladins (bstats + bmana)"
    if (TwoPallyLogicEnabled() && IsTwoPaladinsSelfBstatsOtherBmana(bot))
    {
        if (Player* tp = target->ToPlayer())
        {
            if (tp->HasTankSpec())
            {
                return false;
            }
        }
    }

    std::string castName = "blessing of kings";
    auto RP = ai::chat::MakeGroupAnnouncer(bot);
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
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
