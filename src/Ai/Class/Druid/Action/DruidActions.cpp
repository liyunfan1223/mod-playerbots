/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DruidActions.h"
#include "AoeValues.h"
#include "Event.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "TargetValue.h"

constexpr uint32 SPELL_ECLIPSE_SOLAR = 48517;
constexpr uint32 SPELL_ECLIPSE_LUNAR = 48518;

namespace
{
    bool PrepareThornsTarget(PlayerbotAI* botAI, Unit* target)
    {
        if (!target)
            return false;

        Aura* existingThorns = botAI->GetAura("thorns", target, true);
        if (!existingThorns)
            return true;

        target->RemoveOwnedAura(existingThorns, AURA_REMOVE_BY_CANCEL);
        return true;
    }
}

std::vector<NextAction> CastAbolishPoisonAction::getAlternatives()
{
    return NextAction::merge({ NextAction("cure poison") },
                             CastSpellAction::getPrerequisites());
}

std::vector<NextAction> CastAbolishPoisonOnPartyAction::getAlternatives()
{
    return NextAction::merge({ NextAction("cure poison on party") },
                             CastSpellAction::getPrerequisites());
}

bool CastLifebloomOnMainTankAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !CastSpellAction::isUseful())
        return false;

    Aura* lifebloom = botAI->GetAura("lifebloom", target, true, true);
    return !lifebloom || lifebloom->GetStackAmount() < 3 || lifebloom->GetDuration() < 2000;
}

bool CastThornsAction::Execute(Event event)
{
    return PrepareThornsTarget(botAI, GetTarget()) && CastBuffSpellAction::Execute(event);
}

bool CastThornsOnPartyAction::Execute(Event event)
{
    return PrepareThornsTarget(botAI, GetTarget()) && BuffOnPartyAction::Execute(event);
}

bool CastThornsOnMainTankAction::Execute(Event event)
{
    return PrepareThornsTarget(botAI, GetTarget()) && BuffOnMainTankAction::Execute(event);
}

bool CastWrathAction::isUseful()
{
    time_t now = time(nullptr);
    time_t solarTime = context->GetValue<time_t>("eclipse solar proc time")->Get();
    time_t lunarTime = context->GetValue<time_t>("eclipse lunar proc time")->Get();

    // --- Update Solar Eclipse tracking ---
    // Wrath is selected during Solar Eclipse (eclipse trigger at 20.0f), so we reliably see it here.
    if (bot->HasAura(SPELL_ECLIPSE_SOLAR) && !solarTime)
        context->GetValue<time_t>("eclipse solar proc time")->Set(now);
    // Lunar procced — Solar fishing window is over, new cycle begins
    if (bot->HasAura(SPELL_ECLIPSE_LUNAR) && solarTime)
        context->GetValue<time_t>("eclipse solar proc time")->Set(0);
    // 30 s cooldown window expired
    if (solarTime && (now - solarTime) >= 30)
        context->GetValue<time_t>("eclipse solar proc time")->Set(0);

    // --- Update Lunar Eclipse tracking (belt-and-suspenders in case Starfire isn't evaluated) ---
    if (bot->HasAura(SPELL_ECLIPSE_LUNAR) && !lunarTime)
        context->GetValue<time_t>("eclipse lunar proc time")->Set(now);
    // Solar procced — Lunar fishing window is over
    if (bot->HasAura(SPELL_ECLIPSE_SOLAR) && lunarTime)
        context->GetValue<time_t>("eclipse lunar proc time")->Set(0);
    if (lunarTime && (now - lunarTime) >= 30)
        context->GetValue<time_t>("eclipse lunar proc time")->Set(0);

    // Block Wrath while in Lunar Eclipse / post-Lunar fishing window
    if (context->GetValue<time_t>("eclipse lunar proc time")->Get())
        return false;

    return CastSpellAction::isUseful();
}

bool CastStarfireAction::isUseful()
{
    time_t now = time(nullptr);
    time_t solarTime = context->GetValue<time_t>("eclipse solar proc time")->Get();
    time_t lunarTime = context->GetValue<time_t>("eclipse lunar proc time")->Get();

    // --- Update Lunar Eclipse tracking ---
    // Starfire is selected during Lunar Eclipse (eclipse trigger at 20.0f), so we reliably see it here.
    if (bot->HasAura(SPELL_ECLIPSE_LUNAR) && !lunarTime)
        context->GetValue<time_t>("eclipse lunar proc time")->Set(now);
    // Solar procced — Lunar fishing window is over, new cycle begins
    if (bot->HasAura(SPELL_ECLIPSE_SOLAR) && lunarTime)
        context->GetValue<time_t>("eclipse lunar proc time")->Set(0);
    // 30 s cooldown window expired
    if (lunarTime && (now - lunarTime) >= 30)
        context->GetValue<time_t>("eclipse lunar proc time")->Set(0);

    // --- Update Solar Eclipse tracking (belt-and-suspenders in case Wrath isn't evaluated) ---
    if (bot->HasAura(SPELL_ECLIPSE_SOLAR) && !solarTime)
        context->GetValue<time_t>("eclipse solar proc time")->Set(now);
    // Lunar procced — Solar fishing window is over
    if (bot->HasAura(SPELL_ECLIPSE_LUNAR) && solarTime)
        context->GetValue<time_t>("eclipse solar proc time")->Set(0);
    if (solarTime && (now - solarTime) >= 30)
        context->GetValue<time_t>("eclipse solar proc time")->Set(0);

    // Block Starfire while in Solar Eclipse / post-Solar fishing window
    if (context->GetValue<time_t>("eclipse solar proc time")->Get())
        return false;

    return CastSpellAction::isUseful();
}

Value<Unit*>* CastEntanglingRootsCcAction::GetTargetValue()
{
    return context->GetValue<Unit*>("rti cc target");
}

Value<Unit*>* CastHibernateCcAction::GetTargetValue() { return context->GetValue<Unit*>("rti cc target"); }

Value<Unit*>* CastCycloneCcAction::GetTargetValue() { return context->GetValue<Unit*>("rti cc target"); }

bool CastTyphoonAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose  = ServerFacade::instance().IsDistanceLessOrEqualThan(
        AI_VALUE2(float, "distance", GetTargetName()), 15.f);
    return facingTarget && targetClose;
}

bool CastStarfallAction::isUseful()
{
    if (!CastSpellAction::isUseful())
        return false;

    // Avoid breaking CC
    WorldLocation aoePos = *context->GetValue<WorldLocation>("aoe position");
    Unit* ccTarget = context->GetValue<Unit*>("current cc target")->Get();
    if (ccTarget && ccTarget->IsAlive())
    {
        float dist2d = ServerFacade::instance().GetDistance2d(ccTarget, aoePos.GetPositionX(), aoePos.GetPositionY());
        if (ServerFacade::instance().IsDistanceLessOrEqualThan(dist2d, sPlayerbotAIConfig.AoeRadius))
            return false;
    }

    // Suppress if any unengaged hostile unit is within 40 yards — Starfall's 36-yard radius would pull them.
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    GuidVector const& nearbyNpcs = AI_VALUE(GuidVector, "possible targets");
    for (ObjectGuid const& guid : nearbyNpcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        // Standard null/world-state guard before touching the unit.
        if (!unit || !unit->IsAlive() || !unit->IsInWorld())
            continue;
        // Already our target — its in-combat flag covers it.
        if (unit == currentTarget)
            continue;
        // Safety net for any hostile-faction trigger creature that carries NON_ATTACKABLE flags.
        if (!bot->IsValidAttackTarget(unit))
            continue;
        // Outside Starfall's actual radius; no pull risk.
        if (ServerFacade::instance().GetDistance2d(bot, unit) > 40.0f)
            continue;
        // Unengaged mob within range — casting would pull it.
        if (!unit->IsInCombat())
            return false;
    }

    return true;
}

std::vector<NextAction> CastReviveAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("caster form") },
                             ResurrectPartyMemberAction::getPrerequisites());
}

std::vector<NextAction> CastRebirthAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("caster form") },
                             ResurrectPartyMemberAction::getPrerequisites());
}

bool CastRebirthAction::isUseful()
{
    return CastSpellAction::isUseful() &&
           AI_VALUE2(float, "distance", GetTargetName()) <= sPlayerbotAIConfig.SpellDistance;
}

bool CastInnervateOnHealerAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target || !target->IsInWorld())
        return false;

    if (botAI->HasAura("innervate", target))
        return false;

    uint32 spellId = AI_VALUE2(uint32, "spell id", "innervate");
    return spellId && !bot->HasSpellCooldown(spellId);
}

std::vector<NextAction> CastInnervateOnHealerAction::getPrerequisites()
{
    return { NextAction("caster form") };
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
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig.SpellDistance)
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

// --- Blanket HoT actions ---

ObjectGuid BlanketHotTargetValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    auto eligible = [&](Player* member) -> bool
    {
        return member && member->IsAlive() &&
               !member->IsGameMaster() &&
               bot->GetDistance2d(member) <= sPlayerbotAIConfig.SpellDistance &&
               !botAI->HasAura(qualifier, member, false, true) &&
               bot->IsWithinLOSInMap(member);
    };

    Player* firstMelee  = nullptr;
    Player* firstRanged = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!eligible(member))
            continue;

        if (PlayerbotAI::IsTank(member))
            return member->GetGUID();
        else if (!firstMelee && PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsTank(member))
            firstMelee = member;
        else if (!firstRanged && PlayerbotAI::IsRanged(member))
            firstRanged = member;

        if (firstMelee && firstRanged)
            break;
    }

    if (firstMelee)
        return firstMelee->GetGUID();

    if (firstRanged)
        return firstRanged->GetGUID();

    return ObjectGuid::Empty;
}

Unit* CastBlanketHotAction::GetTarget() { return botAI->GetUnit(AI_VALUE2(ObjectGuid, "blanket hot target", spell)); }

bool CastBlanketHotAction::isUseful() { return GetTarget() != nullptr; }
