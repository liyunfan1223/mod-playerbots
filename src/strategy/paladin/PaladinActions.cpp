/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include <algorithm>

#include "../../../../../src/server/scripts/Spells/spell_generic.cpp"
#include "AiFactory.h"
#include "Config.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using ai::buff::MakeAuraQualifierForBuff;
using ai::buff::UpgradeToGroupIfAppropriate;

// Readable tag to identify the group in the logs based on the leader
static inline std::string MakeGroupTag(Group* g)
{
    if (!g)
        return "g0";
    ObjectGuid leader = g->GetLeaderGUID();
    uint32 low = leader.GetCounter();
    return std::string("g") + std::to_string(low);
}

// Log ONLY bots whose master is the leader of their group/raid == my raid
static inline bool ShouldLogForThisBot(Player* bot)
{
    PlayerbotAI* ai = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
    Player* master = ai ? ai->GetMaster() : nullptr;
    Group* g = bot ? bot->GetGroup() : nullptr;
    return master && g && (g->GetLeaderGUID() == master->GetGUID());
}

// Helper : detect tank role on the target (player bot or not) return true if spec is tank or if the bot have tank
// strategies (bear/tank/tank face).
static inline bool IsTankRole(Player* p)
{
    if (!p)
	{
        return false;
	}
    if (p->HasTankSpec())
	{
        return true;
	}
    if (PlayerbotAI* otherAI = GET_PLAYERBOT_AI(p))
    {
        if (otherAI->HasStrategy("tank", BOT_STATE_NON_COMBAT) || otherAI->HasStrategy("tank", BOT_STATE_COMBAT) ||
            otherAI->HasStrategy("tank face", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("tank face", BOT_STATE_COMBAT) || otherAI->HasStrategy("bear", BOT_STATE_NON_COMBAT) ||
            otherAI->HasStrategy("bear", BOT_STATE_COMBAT))
            return true;
    }
    return false;
}

// Added for solo paladin patch : determine if he's the only paladin on party
static inline bool IsOnlyPaladinInGroup(Player* bot)
{
    // Logging de coordination des rôles
    if (!bot)
	{
        return false;
	}
    Group* g = bot->GetGroup();
    if (!g)
	{
        return true;
	}
    uint32 pals = 0u;
    for (GroupReference* r = g->GetFirstMember(); r; r = r->next())
    {
        Player* p = r->GetSource();
        if (!p || !p->IsInWorld())
		{
            continue;
		}
        if (p->getClass() == CLASS_PALADIN)
		{
            ++pals;
		}
    }
    return pals == 1u;
}

// Number of paladins connected in the group/raid
static inline uint32 CountPaladinsInGroup(Player* bot)
{
    if (!bot)
	{
        return 0u;
	}
	
    Group* g = bot->GetGroup();
    if (!g)
	{
        return (bot->getClass() == CLASS_PALADIN) ? 1u : 0u;
	}
	
    uint32 pals = 0u;
    for (GroupReference* r = g->GetFirstMember(); r; r = r->next())
    {
        Player* p = r->GetSource();
        if (!p || !p->IsInWorld())
		{
            continue;
		}
        if (p->getClass() == CLASS_PALADIN)
		{
            ++pals;
		}
    }
    return pals;
}

// Target eligible for Might: Physical + Hunter + Shaman Enhancement. Tank: Only if >= 3 Paladins (3rd Blessing).
static inline bool ShouldReceiveMight(Player* p, Player* caster)
{
    if (!p)
	{
        return false;
	}
	
    int tab = AiFactory::GetPlayerSpecTab(p);
    switch (p->getClass())
    {
        case CLASS_WARRIOR:
            return true;
        case CLASS_ROGUE:
            return true;
        case CLASS_DEATH_KNIGHT:
            return true;
        case CLASS_HUNTER:
            return true;  // remote physics
        case CLASS_SHAMAN:
            return tab == SHAMAN_TAB_ENHANCEMENT;  // melee
        case CLASS_DRUID:
            return tab == DRUID_TAB_FERAL;  // feral
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_RETRIBUTION)
                return true;
            if (tab == PALADIN_TAB_PROTECTION)
                return CountPaladinsInGroup(caster) >= 3u;
            return false;
        default:
            return false;  // mages/demos/priests/boomkin/resto/elem....
    }
}

// Elect a single paladin per buff role to avoid overwriting each other
static inline bool IsDesignatedRolePaladin(Player* bot, const char* roleName)
{
    if (!bot)
	{
        return true;
	}
	
    Group* g = bot->GetGroup();
    const std::string gtag = MakeGroupTag(g);
    if (!g)
	{
        return true;
	}
	
    const bool doLog = ShouldLogForThisBot(bot);

    // Gather paladins with the same role strategy
    std::vector<ObjectGuid> contenders;
    std::vector<std::string> contenderNames;
    for (GroupReference* r = g->GetFirstMember(); r; r = r->next())
    {
        Player* p = r->GetSource();
        if (!p || !p->IsInWorld() || p->getClass() != CLASS_PALADIN)
		{
            continue;
		}

        if (PlayerbotAI* otherAI = GET_PLAYERBOT_AI(p))
        {
            if (otherAI->HasStrategy(roleName, BOT_STATE_NON_COMBAT))
            {
                contenders.push_back(p->GetGUID());
                contenderNames.emplace_back(p->GetName());
            }
        }
    }

    if (contenders.empty())
    {
        if (doLog)
            LOG_DEBUG("playerbots", "[RoleCoord:{}] role={} -> aucun paladin porteur (proceed)", gtag, roleName);
        return true;
    }
    if (contenders.size() == 1u)
    {
        const bool designated = (bot->GetGUID() == contenders.front());
        if (doLog)
            LOG_DEBUG("playerbots", "[RoleCoord:{}] role={} -> unique porteur: {} | moi={} -> {}", gtag, roleName,
                      contenderNames.front(), bot->GetName(), (designated ? "OUI" : "NON"));
        return designated;  // seul le porteur agit
    }

    // Deterministic winner: smallest GUID
    auto winner = *std::min_element(contenders.begin(), contenders.end());

    std::string winnerName = "?";
    for (GroupReference* r = g->GetFirstMember(); r; r = r->next())
    {
        Player* p = r->GetSource();
        if (p && p->IsInWorld() && p->getClass() == CLASS_PALADIN && p->GetGUID() == winner)
        {
            winnerName = p->GetName();
            break;
        }
    }
    bool designated = (bot->GetGUID() == winner);
    if (doLog)
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role={} -> candidats=[{}] | élu={} | moi={} -> {}", gtag, roleName,
                  fmt::join(contenderNames, ", "), winnerName, bot->GetName(), (designated ? "OUI" : "NON"));
    return designated;
}

static inline bool GroupHasTankOfClass(Group* g, uint8 classId)
{
    return GroupHasTankOfClass(g, static_cast<Classes>(classId));
}

inline std::string const GetActualBlessingOfMight(Unit* target)
{
    std::string result;
    Player* tp = target ? target->ToPlayer() : nullptr;
    if (!tp)
        result = "blessing of might";
    else
    {
        int tab = AiFactory::GetPlayerSpecTab(tp);
        switch (tp->getClass())
        {
            case CLASS_MAGE:
            case CLASS_PRIEST:
            case CLASS_WARLOCK:
                result = "blessing of wisdom";
                break;
            case CLASS_SHAMAN:
                if (tab == SHAMAN_TAB_ELEMENTAL || tab == SHAMAN_TAB_RESTORATION)
                    result = "blessing of wisdom";
                break;
            case CLASS_DRUID:
                if (tab == DRUID_TAB_RESTORATION || tab == DRUID_TAB_BALANCE)
                    result = "blessing of wisdom";
                break;
            case CLASS_PALADIN:
                if (tab == PALADIN_TAB_HOLY)
                    result = "blessing of wisdom";
                break;
            default:
                break;
        }
        if (result.empty())
            result = "blessing of might";
    }

    LOG_DEBUG("playerbots", "[BlessingDecision] Might resolver -> target={} class={} specTab={} => {}",
              (tp ? tp->GetName() : "non-player"), (tp ? int(tp->getClass()) : -1),
              (tp ? AiFactory::GetPlayerSpecTab(tp) : -1), result);
    return result;
}

inline std::string const GetActualBlessingOfWisdom(Unit* target)
{
    std::string result;
    Player* tp = target ? target->ToPlayer() : nullptr;
    if (!tp)
        result = "blessing of might";
    else
    {
        int tab = AiFactory::GetPlayerSpecTab(tp);
        switch (tp->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_ROGUE:
            case CLASS_DEATH_KNIGHT:
            case CLASS_HUNTER:
                result = "blessing of might";
                break;
            case CLASS_SHAMAN:
                if (tab == SHAMAN_TAB_ENHANCEMENT)
                    result = "blessing of might";
                break;
            case CLASS_DRUID:
                if (tab == DRUID_TAB_FERAL)
                    result = "blessing of might";
                break;
            case CLASS_PALADIN:
                if (tab == PALADIN_TAB_PROTECTION || tab == PALADIN_TAB_RETRIBUTION)
                    result = "blessing of might";
                break;
            default:
                break;
        }
        if (result.empty())
            result = "blessing of wisdom";
    }

    LOG_DEBUG("playerbots", "[BlessingDecision] Wisdom resolver -> target={} class={} specTab={} => {}",
              (tp ? tp->GetName() : "non-player"), (tp ? int(tp->getClass()) : -1),
              (tp ? AiFactory::GetPlayerSpecTab(tp) : -1), result);
    return result;
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
    {
        LOG_DEBUG("playerbots", "[BlessingDecision] Sanctuary resolver -> target={} isTankSpec=1 => sanctuary",
                  tp->GetName());
        return "blessing of sanctuary";
    }

    LOG_DEBUG("playerbots", "[BlessingDecision] Sanctuary resolver -> target={} isTankSpec=0 => '' (no sanct)",
              tp->GetName());

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
        "blessing of might,greater blessing of might,blessing of sanctuary,greater blessing of sanctuary");
}

bool CastBlessingOfMightOnPartyAction::Execute(Event event)
{
    // Single-role coordinator: only one paladin with bdps does party-wide buffs
    if (!IsDesignatedRolePaladin(bot, "bdps"))
        return false;
    // Log only my bots
    if (ShouldLogForThisBot(bot))
    {
        const std::string gtag = MakeGroupTag(bot->GetGroup());
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role=bdps -> {} est autorisé à poser Might", gtag, bot->GetName());
    }

    Unit* target = GetTarget();
    if (!target)
        return false;

    // If the current target is not eligible for Might, retarget a party member who is
    Player* tp = target->ToPlayer();
    if (!tp || !ShouldReceiveMight(tp, bot))
    {
        Group* g = bot->GetGroup();
        if (g)
        {
            Unit* best = nullptr;
            for (GroupReference* r = g->GetFirstMember(); r; r = r->next())
            {
                Player* p = r->GetSource();
                if (!p || !p->IsInWorld() || !p->IsAlive())
                    continue;
                if (!ShouldReceiveMight(p, bot))
                    continue;
                // avoid resting Might if already present (greater/mono)
                if (botAI->HasAura("blessing of might", p) || botAI->HasAura("greater blessing of might", p))
                    continue;
                best = p;
                break;
            }
            if (best)
            {
                target = best;
                tp = best->ToPlayer();
                if (ShouldLogForThisBot(bot))
                    LOG_DEBUG("playerbots", "[Might] Re-ciblage vers {}", tp->GetName());
            }
        }
        // if still no valid target -> abandon
        if (!tp || !ShouldReceiveMight(tp, bot))
            return false;
    }

    std::string castName = "blessing of might";
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
        "blessing of wisdom,greater blessing of wisdom,blessing of sanctuary,greater blessing of sanctuary");
}

bool CastBlessingOfWisdomOnPartyAction::Execute(Event event)
{
    // Single-role coordinator: only one paladin with "bmana" does party-wide buffs
    if (!IsDesignatedRolePaladin(bot, "bmana"))
	{	
        return false;
	}

    if (ShouldLogForThisBot(bot))
    {
        const std::string gtag = MakeGroupTag(bot->GetGroup());
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role=bmana -> {} est autorisé à poser Wisdom", gtag, bot->GetName());
    }

    Unit* target = GetTarget();
    if (!target)
	{	
        return false;
	}

    Player* targetPlayer = target->ToPlayer();
	
    if (Group* g = bot->GetGroup(); g && targetPlayer && !g->IsMember(targetPlayer->GetGUID())) {
        return false;
    }	

    if (botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT) && targetPlayer && IsTankRole(targetPlayer))
    {
        LOG_DEBUG("playerbots", "[Wisdom/bmana] Skip tank {} (Kings only)", target->GetName());
        return false;
    }

    // Force Wisdom into party mode (no redirection based on class)
    std::string castName = "blessing of wisdom";
    if (castName.empty())
	{	
        return false;
	}

    auto RP = ai::chat::MakeGroupAnnouncer(bot);
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfSanctuaryOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", "blessing of sanctuary,greater blessing of sanctuary");
}

bool CastBlessingOfSanctuaryOnPartyAction::Execute(Event event)
{
    if (!bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY))
	{
        return false;
	}

    Unit* target = GetTarget();
    if (!target)
    {
        target = bot;
    }

    Player* targetPlayer = target ? target->ToPlayer() : nullptr;

    // Small helpers to check relevant auras
    const auto HasKingsAura = [&](Unit* u) -> bool
    { return botAI->HasAura("blessing of kings", u) || botAI->HasAura("greater blessing of kings", u); };
    const auto HasSanctAura = [&](Unit* u) -> bool
    { return botAI->HasAura("blessing of sanctuary", u) || botAI->HasAura("greater blessing of sanctuary", u); };

    Group* g = bot->GetGroup();
    if (g && targetPlayer && !g->IsMember(targetPlayer->GetGUID())) {
        LOG_DEBUG("playerbots", "[Sanct] Initial target not in group, ignoring");
        target = bot;
        targetPlayer = bot->ToPlayer();
    }

    if (Player* self = bot->ToPlayer())
    {
        bool selfHasSanct = HasSanctAura(self);
        bool needSelf = IsTankRole(self) && !selfHasSanct;

        LOG_DEBUG("playerbots", "[Sanct] {} isTank={} selfHasSanct={} needSelf={}", bot->GetName(), IsTankRole(self),
                  selfHasSanct, needSelf);

        if (needSelf)
        {
            target = self;
            targetPlayer = self;
        }
    }

    // Try to re-target a valid tank in group if needed
    bool targetOk = false;
    if (targetPlayer)
    {
        bool hasSanct = HasSanctAura(targetPlayer);
        targetOk = IsTankRole(targetPlayer) && !hasSanct;
    }

    if (!targetOk)
    {
        if (Group* g = bot->GetGroup())
        {
            for (GroupReference* gref = g->GetFirstMember(); gref; gref = gref->next())
            {
                Player* p = gref->GetSource();
                if (!p)
				{
                    continue;
				}
                if (!p->IsInWorld() || !p->IsAlive())
				{	
                    continue;
				}
                if (!IsTankRole(p))
				{
                    continue;
				}

                bool hasSanct = HasSanctAura(p);
                if (!hasSanct)
                {
                    target = p;  // prioritize this tank
                    targetPlayer = p;
                    targetOk = true;
                    break;
                }
            }
        }
    }

    // If after retargeting we still don't have a valid tank without Sanctuary, we stop and avoid calling the
    // resolver/logger no sanct
    if (!targetOk)
	{
        return false;
	}

    {
        bool hasKings = HasKingsAura(target);
        bool hasSanct = HasSanctAura(target);
        bool knowSanct = bot->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
        LOG_DEBUG("playerbots", "[Sanct] Final target={} hasKings={} hasSanct={} knowSanct={}", target->GetName(),
                  hasKings, hasSanct, knowSanct);
    }

    std::string castName = GetActualBlessingOfSanctuary(target, bot);

    // no point in switching to Greater here, to avoid any collision
    if (castName.empty())
	{
        castName = "blessing of sanctuary";
	}
    // For safety, if ever a non-tank arrives here
    if (targetPlayer && !IsTankRole(targetPlayer))
	{
        return false;
	}
	
    castName = "blessing of sanctuary";

    bool ok = botAI->CastSpell(castName, target);
    LOG_DEBUG("playerbots", "[Sanct] Cast {} on {} result={}", castName, target->GetName(), ok);
    return ok;
}

Value<Unit*>* CastBlessingOfKingsOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", "blessing of kings,greater blessing of kings");
}

bool CastBlessingOfKingsOnPartyAction::Execute(Event event)
{
    // Allow Kings on party if the bot is elected for bstats OR for bmana (bmana places MONO Kings on tanks)
    const bool electedBstats = IsDesignatedRolePaladin(bot, "bstats");
    const bool electedBmana = IsDesignatedRolePaladin(bot, "bmana");
    const bool hasBstats = botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT);
    const bool hasBmana = botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT);
    const bool actAsBstats = electedBstats && hasBstats;
    const bool actAsBmana = electedBmana && hasBmana;
    if (!actAsBstats && !actAsBmana)
	{	
        return false;
	}	
    if (ShouldLogForThisBot(bot))
    {
        const std::string gtag = MakeGroupTag(bot->GetGroup());
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role={} -> {} est autorisé à poser Kings", gtag,
                  (actAsBmana ? "bmana" : "bstats"), bot->GetName());
    }

    Unit* target = GetTarget();
    if (!target)
	{	
        return false;
	}

    Player* targetPlayer = target->ToPlayer();  // <-- déclare targetPlayer ici

    Group* g = bot->GetGroup();
    if (!g)
	{	
        return false;
	}

    // Only one paladin in the party, case with active bstats, avoid self-targeting and ensure that Kings covers the
    // others, Greater if possible.
    if (actAsBstats && IsOnlyPaladinInGroup(bot))
    {
        // If the initial target is not a "non-tank without Kings", we retarget
        auto lacksKings = [&](Unit* u) -> bool
        { return u && !botAI->HasAura("blessing of kings", u) && !botAI->HasAura("greater blessing of kings", u); };
        auto isEligibleNonTank = [&](Player* p) -> bool
        { return p && p->IsInWorld() && p->IsAlive() && !IsTankRole(p); };

        Player* tp0 = target->ToPlayer();
        bool ok0 = tp0 && isEligibleNonTank(tp0) && lacksKings(tp0) && (tp0->GetGUID() != bot->GetGUID());
        if (!ok0)
        {
            Unit* best = nullptr;
            for (GroupReference* r = g->GetFirstMember(); r; r = r->next())
            {
                Player* p = r->GetSource();
                if (!isEligibleNonTank(p))
				{	
                    continue;
				}
                if (!lacksKings(p))
				{	
                    continue;
				}
                if (p->GetGUID() == bot->GetGUID())
				{	
                    continue;  // do not target itself
				}
                best = p;
                break;
            }
            if (best)
            {
                target = best;
                targetPlayer = best->ToPlayer();
                if (ShouldLogForThisBot(bot))
                    LOG_DEBUG("playerbots", "[Kings/bstats-solo] Re-ciblage vers {}", targetPlayer->GetName());
            }
        }
    }

    // Solo paladin, never buff itself to not remove his sanctuary buff
    if (botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT) && IsOnlyPaladinInGroup(bot))
    {
        if (target->GetGUID() == bot->GetGUID())
        {
            LOG_DEBUG("playerbots", "[Kings/bstats-solo] Skip self to keep Sanctuary on {}", bot->GetName());
            return false;
        }
    }

    targetPlayer = target->ToPlayer();
    if (targetPlayer && !g->IsMember(targetPlayer->GetGUID()))
	{	
        return false;
	}

    // If we act in bmana mode => Kings MONO on TANKS only
    if (actAsBmana && (!targetPlayer || !IsTankRole(targetPlayer)))
    {
        LOG_DEBUG("playerbots", "[Kings/bmana] Skip non-tank {}", target->GetName());
        return false;
    }

    if (targetPlayer)
    {
        const bool isTank = IsTankRole(targetPlayer);
        const bool hasSanctFromMe = target->HasAura(SPELL_BLESSING_OF_SANCTUARY, bot->GetGUID()) ||
                                    target->HasAura(SPELL_GREATER_BLESSING_OF_SANCTUARY, bot->GetGUID());
        const bool hasSanctAny =
            botAI->HasAura("blessing of sanctuary", target) || botAI->HasAura("greater blessing of sanctuary", target);

        if (!actAsBmana && isTank && hasSanctFromMe)
        {
            LOG_DEBUG("playerbots", "[Kings] Skip: {} has my Sanctuary and is a tank", target->GetName());
            return false;
        }

        if (actAsBstats && isTank && hasSanctAny)
        {
            LOG_DEBUG("playerbots", "[Kings] Skip (bstats): {} already has Sanctuary and is a tank", target->GetName());
            return false;
        }
    }

    std::string castName = "blessing of kings";

    bool allowGreater = true;

    if (actAsBmana)
	{
        allowGreater = false;
	}

    // In solo-paladin (bstats), we want to favor the Greater to cover the target class.
    if (actAsBstats && IsOnlyPaladinInGroup(bot))
	{
        allowGreater = true;
	}

    if (allowGreater && actAsBstats && targetPlayer && !IsOnlyPaladinInGroup(bot))
    {
        switch (targetPlayer->getClass())
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
