/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include <algorithm>
#include <unordered_map>
#include <ctime>
 
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
#include "Playerbots.h"

namespace {
    enum class StrictSetting { Off = 0, On = 1, Auto = 2 };

    inline StrictSetting ReadStrictSetting()
    {
        // Accept "OFF|ON|AUTO" (case-insensitive) OR "0|1|2"
        static std::string value = sConfigMgr->GetOption<std::string>("AiPlayerbot.Paladin.StrictRoleMode", "OFF");
        std::string v = value;
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        if (v == "on" || v == "1")  return StrictSetting::On;
        if (v == "auto" || v == "2") return StrictSetting::Auto;
        return StrictSetting::Off;
    }

    struct StrictAnnounceState
    {
        bool lastStrict = false;
        time_t lastAnnounce = 0;
    };

	static std::unordered_map<uintptr_t /*groupPtr*/, StrictAnnounceState> s_strictByGroup;

    inline void AnnounceStrict(Player* bot, bool strictOn, StrictSetting setting, uint32 size, uint32 pals)
    {
        static bool notify = sConfigMgr->GetOption<bool>("AiPlayerbot.Paladin.StrictNotify", true);
        if (!notify) return;

        Group* g = bot->GetGroup();
        if (!g) return;

		uintptr_t key = reinterpret_cast<uintptr_t>(g);
        StrictAnnounceState& st = s_strictByGroup[key];

        // Only announces when the state changes
        if (st.lastStrict == strictOn)
            return;

        time_t now = std::time(nullptr);
        static int32 cd = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.StrictNotifyCooldown", 60);
        if (st.lastAnnounce && (now - st.lastAnnounce) < cd)
            return;

        std::string msg;
        switch (setting)
        {
            case StrictSetting::On:   msg = strictOn ? "[Paladin AI] Strict Mode: ON (FORCED)" : "[Paladin AI] Strict Mode: OFF (FORCED override)"; break;
            case StrictSetting::Off:  msg = "[Paladin AI] Strict Mode: OFF"; break; // Shouldn’t change dynamically, but safe
            case StrictSetting::Auto:
            default:
                if (strictOn)
                    msg = "[Paladin AI] Strict Mode: ON (AUTO; size≥" + std::to_string(sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictRaidSize", 20)) +
                          ", paladins≥" + std::to_string(sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictMinPaladins", 3)) + ")";
                else
                    msg = "[Paladin AI] Strict Mode: OFF (AUTO; reverting to smart per-target)";
                break;
        }

        std::string scope = sConfigMgr->GetOption<std::string>("AiPlayerbot.Paladin.StrictNotifyScope", "MASTER");
        if (!scope.empty() && (scope == "GROUP" || scope == "group"))
        {
            ai::chat::MakeGroupAnnouncer(bot)(msg);
        }
        else
        {
            if (auto* ai = GET_PLAYERBOT_AI(bot))
                ai->TellMaster(msg);
        }

        st.lastStrict = strictOn;
        st.lastAnnounce = now;
    }

    inline uint32 CountPaladinsInGroup(Group* g)
    {
        if (!g) return 0;
        uint32 count = 0;
    
        Group::MemberSlotList const& slots = g->GetMemberSlots();
        for (auto const& slot : slots)
        {
            Player* p = ObjectAccessor::FindConnectedPlayer(slot.guid);
            if (!p) continue;
    
            if (p->getClass() == CLASS_PALADIN)
                ++count;
        }
        return count;
    }

    inline bool StrictRoleModeEnabledFor(Player* bot)
    {
        Group* g = bot->GetGroup();
        if (!g)
            return false;
		
        int32 raidSizeAuto    = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictRaidSize", 20);
        int32 minPalsAuto     = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictMinPaladins", 3);
        int32 minStrictManual = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.StrictMinGroupSize", 10);
		
        switch (ReadStrictSetting())
        {
            case StrictSetting::On:
            {
                bool on = true;
                AnnounceStrict(bot, on, StrictSetting::On, g->GetMembersCount(), CountPaladinsInGroup(g));
                return on;
            }
			
            case StrictSetting::Off:
            {
                bool on = false;
                AnnounceStrict(bot, on, StrictSetting::Off, g->GetMembersCount(), CountPaladinsInGroup(g));
                return on;
            }

            case StrictSetting::Auto:
            default:
            {
                uint32 size = g->GetMembersCount();
                uint32 palCount = CountPaladinsInGroup(g);
                bool on = (size >= static_cast<uint32>(raidSizeAuto)) &&
                          (palCount >= static_cast<uint32>(minPalsAuto));
                AnnounceStrict(bot, on, StrictSetting::Auto, size, palCount);
                return on;
            }
        }
    }
}
	
using ai::buff::MakeAuraQualifierForBuff;
using ai::buff::UpgradeToGroupIfAppropriate;

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
    Player* targetPlayer = target->ToPlayer();

    if (!targetPlayer)
    {
        return "blessing of sanctuary";
    }

    if (targetPlayer->HasTankSpec() && bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY))
    {
        return "blessing of sanctuary";
    }

    return "blessing of kings";
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

    // Upgrade to Greater if relevant, otherwise single (with RP if reagents missing)
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfMightOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of might,greater blessing of might,blessing of wisdom,greater blessing of wisdom"
    );
}

bool CastBlessingOfMightOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Strict mode: Might only for relevant targets (no fallback to Wisdom)
    if (StrictRoleModeEnabledFor(bot))
    {
        std::string desired = GetActualBlessingOfMight(target);
        if (desired != "blessing of might")
            return false; // on ignore la cible non-pertinente

        std::string castName = "blessing of might";
        auto RP = ai::chat::MakeGroupAnnouncer(bot);
        castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
        return botAI->CastSpell(castName, target);
    }

    std::string castName = GetActualBlessingOfMight(target);

    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    // Upgrade to Greater if relevant, otherwise single (with RP if reagents missing)
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

    // Upgrade to Greater if relevant, otherwise single (with RP if reagents missing)
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfWisdomOnPartyAction::GetTargetValue()
{
	return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of wisdom,greater blessing of wisdom,blessing of might,greater blessing of might"
    );
}

bool CastBlessingOfWisdomOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    // Strict mode: Wisdom only for mana users (no fallback to Might)
    if (StrictRoleModeEnabledFor(bot))
    {
        std::string desired = GetActualBlessingOfWisdom(target);
        if (desired != "blessing of wisdom")
            return false; // on ignore la cible non-pertinente

        std::string castName = "blessing of wisdom";
        auto RP = ai::chat::MakeGroupAnnouncer(bot);
        castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
        return botAI->CastSpell(castName, target);
    }

    std::string castName = GetActualBlessingOfWisdom(target);

    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    // Upgrade to Greater if relevant, otherwise single (with RP if reagents missing)
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfSanctuaryOnPartyAction::GetTargetValue()
{
	return context->GetValue<Unit*>(
    "party member without aura",
    "blessing of sanctuary,greater blessing of sanctuary,blessing of kings,greater blessing of kings"
    );
}

bool CastBlessingOfSanctuaryOnPartyAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfSanctuary(target, bot); // "sanctuary" ou "blessing of kings"


    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    // Upgrade to Greater if relevant, otherwise single (with RP if reagents missing)
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
