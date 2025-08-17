/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include <algorithm>
#include <unordered_map>
#include <ctime>
#include <cctype>
 
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
#include "WorldPacket.h"

// Color helpers for chat
inline std::string CC(const char* hex, std::string const& s) { return std::string("|cff") + hex + s + "|r"; }
inline std::string C_GREEN (std::string const& s){ return CC("00ff00", s); }
inline std::string C_RED   (std::string const& s){ return CC("ff3333", s); }
inline std::string C_YELLOW(std::string const& s){ return CC("ffff00", s); }
inline std::string C_CYAN  (std::string const& s){ return CC("00ffff", s); }
inline std::string C_GRAY  (std::string const& s){ return CC("9d9d9d", s); }

namespace {
    enum class StrictSetting { Off = 0, On = 1, Auto = 2 };

    inline StrictSetting ReadStrictSetting()
    {
        std::string v = sConfigMgr->GetOption<std::string>("AiPlayerbot.Paladin.StrictRoleMode", "OFF");
        std::transform(v.begin(), v.end(), v.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        if (v == "on"   || v == "1") return StrictSetting::On;
        if (v == "auto" || v == "2") return StrictSetting::Auto;
        return StrictSetting::Off;
    }

    struct StrictAnnounceState
    {
        bool lastStrict = false;
        time_t lastAnnounce = 0;
    };

    // One entry per group (key = Group address)
    static std::unordered_map<uintptr_t /*groupPtr*/, StrictAnnounceState> s_strictByGroup;

    inline void AnnounceStrict(Player* bot, bool strictOn, StrictSetting setting, uint32 size, uint32 pals)
    {
        // Hot-reload friendly
        bool notify = sConfigMgr->GetOption<bool>("AiPlayerbot.Paladin.StrictNotify", true);
        if (!notify) return;

        Group* g = bot->GetGroup();
        if (!g) return;

        uintptr_t key = reinterpret_cast<uintptr_t>(g);
        StrictAnnounceState& st = s_strictByGroup[key];

        // Only announces on state transition
        if (st.lastStrict == strictOn)
            return;

        time_t now = std::time(nullptr);
        int32 cd = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.StrictNotifyCooldown", 60);
        if (st.lastAnnounce && (now - st.lastAnnounce) < cd)
            return;

        std::string msg;
        switch (setting)
        {
            case StrictSetting::On:
                msg = strictOn ? "[Paladin AI] Strict Mode: ON (FORCED)"
                               : "[Paladin AI] Strict Mode: OFF (FORCED override)";
                break;
            case StrictSetting::Off:
                msg = "[Paladin AI] Strict Mode: OFF";
                break;
            case StrictSetting::Auto:
            default:
            {
                // Threshold values for the message
                int32 reqSize = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictRaidSize", 20);
                int32 reqPals = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictMinPaladins", 3);

                // Colored announcements
                std::string sizeCmp = (size >= static_cast<uint32>(reqSize)) ? C_GREEN("≥") : C_RED("<");
                std::string palCmp  = (pals >= static_cast<uint32>(reqPals)) ? C_GREEN("≥") : C_RED("<");
                std::string sizePart = C_YELLOW("size=")     + std::to_string(size) + sizeCmp + std::to_string(reqSize);
                std::string palPart  = C_YELLOW("paladins=") + std::to_string(pals) + palCmp  + std::to_string(reqPals);

                if (strictOn)
                {
                    msg = C_CYAN("[Paladin AI] ") + "Strict Mode: " + C_GREEN("ON") +
                          " " + C_GRAY("(AUTO)") + " " + sizePart + ", " + palPart;
                }
                else
                {
                    msg = C_CYAN("[Paladin AI] ") + "Strict Mode: " + C_RED("OFF") +
                          " " + C_GRAY("(AUTO)") + " " + sizePart + ", " + palPart +
                          C_GRAY("; reverting to smart per-target");
                }				
                break;
            }
        }

        std::string scopeCfg = sConfigMgr->GetOption<std::string>("AiPlayerbot.Paladin.StrictNotifyScope", "MASTER");
        std::string scope = scopeCfg;
        std::transform(scope.begin(), scope.end(), scope.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (scope == "group")
        {
            ai::chat::MakeGroupAnnouncer(bot)(msg);
        }
        else
        {
            if (auto* ai = GET_PLAYERBOT_AI(bot))
                ai->TellMaster(msg);
        }

        st.lastStrict   = strictOn;
        st.lastAnnounce = now;
    }

    inline uint32 CountPaladinsInGroup(Group* g)
    {
        if (!g) return 0;
        uint32 count = 0;

        Group::MemberSlotList const& slots = g->GetMemberSlots();
        for (auto const& slot : slots)
        {
			Player* p = ObjectAccessor::FindPlayer(slot.guid);
			if (!p || !p->IsInWorld()) continue;

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

        uint32 palCount = CountPaladinsInGroup(g);
        uint32 size = 0;
        for (auto const& slot : g->GetMemberSlots())
        {
            Player* p = ObjectAccessor::FindPlayer(slot.guid);
            if (p && p->IsInWorld())
                ++size;
        }

        switch (ReadStrictSetting())
        {
            case StrictSetting::On:
                AnnounceStrict(bot, /*on=*/true,  StrictSetting::On,  size, palCount);
                return true;

            case StrictSetting::Off:
                AnnounceStrict(bot, /*on=*/false, StrictSetting::Off, size, palCount);
                return false;

            case StrictSetting::Auto:
            default:
            {
                int32 reqSize = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictRaidSize", 20);
                int32 reqPals = sConfigMgr->GetOption<int32>("AiPlayerbot.Paladin.AutoStrictMinPaladins", 3);

                bool on = (size >= static_cast<uint32>(reqSize)) &&
                          (palCount >= static_cast<uint32>(reqPals));

                AnnounceStrict(bot, on, StrictSetting::Auto, size, palCount);
                return on;
            }
        }
    }

    inline bool SanctuaryPriorityOn()
    {
        // 1 = Priority Sanctuary on tanks (fallback Kings), 0 = Priority Kings
        return sConfigMgr->GetOption<bool>("AiPlayerbot.Paladin.SanctuaryPriority", true);
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
    // Policy: if Kings is prioritized, this action does nothing
    if (!SanctuaryPriorityOn())
        return "";

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

    // Non-tank -> do nothing (Kings will be handled by the Kings action)
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

    // Strict mode: Might only for relevant targets (no fallback to Wisdom)
    if (StrictRoleModeEnabledFor(bot))
    {
        std::string desired = GetActualBlessingOfMight(target);
        if (desired != "blessing of might")
            return false; // Ignore irrelevant target (non-physique)
    
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
            return false; // Ignore irrelevant target

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

    // Strict mode: Wisdom only for mana users (no fallback to Might)
    if (StrictRoleModeEnabledFor(bot))
    {
        std::string desired = GetActualBlessingOfWisdom(target);
        if (desired != "blessing of wisdom")
            return false; // Ignore irrelevant target (non-mana)
    
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
    if (SanctuaryPriorityOn())
    {
        // Allow the target even if it has Kings, as long as it does NOT have Sanctuary
        return context->GetValue<Unit*>(
            "party member without aura",
            "blessing of sanctuary,greater blessing of sanctuary"
        );
    }

    // // Only target if it has NEITHER Kings NOR Sanctuary
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

    // Focus MT when Sanctuary is prioritized
    if (SanctuaryPriorityOn())
    {
        Unit* mainTank = AI_VALUE(Unit*, "main tank"); // PartyMemberMainTankValue
        if (mainTank)
        {
            bool hasSanct =
                botAI->HasAura("blessing of sanctuary", mainTank, false, true, -1, true) ||
                botAI->HasAura("greater blessing of sanctuary", mainTank, false, true, -1, true);
            if (!hasSanct)
                target = mainTank; // Force target = MT without Sanctuary
        }
    }

    std::string castName = GetActualBlessingOfSanctuary(target, bot);
    if (castName.empty())
        return false; // Strict action: if not a tank / spell unavailable we do nothing

    auto RP = ai::chat::MakeGroupAnnouncer(bot);
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfKingsOnPartyAction::GetTargetValue()
{
    // Never overwrite Sanctuary: if the target already has Sanctuary (single or greater), skip it
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of kings,greater blessing of kings,blessing of sanctuary,greater blessing of sanctuary"
    );
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
