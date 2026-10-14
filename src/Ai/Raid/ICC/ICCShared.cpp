/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ICCShared.h"
#include "ICCActions.h"
#include "ICCScripts.h"
#include "ICCTriggers.h"
#include "Playerbots.h"
#include "ThreatManager.h"
#include "Timer.h"
#include <algorithm>
#include <map>

namespace
{
    std::vector<Creature*> IccGetCreaturesByEntry(WorldObject* searcher, uint32 entry, float range)
    {
        std::list<Creature*> raw;
        searcher->GetCreatureListWithEntryInGrid(raw, entry, range);

        std::vector<Creature*> out;
        out.reserve(raw.size());
        for (Creature* c : raw)
            if (c && c->IsAlive())
                out.push_back(c);

        return out;
    }
}

std::vector<Creature*> IccGetCreaturesByEntries(WorldObject* searcher, std::initializer_list<uint32> entries,
                                                float range)
{
    std::vector<Creature*> out;
    for (uint32 entry : entries)
    {
        auto part = IccGetCreaturesByEntry(searcher, entry, range);
        out.insert(out.end(), part.begin(), part.end());
    }
    std::sort(out.begin(), out.end(), [](Creature const* a, Creature const* b) { return a->GetGUID() < b->GetGUID(); });
    out.erase(std::unique(out.begin(), out.end()), out.end());

    return out;
}

bool IccCastClassTaunt(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsAlive())
        return false;

    uint32 const now = getMSTime();
    uint32& last = IcecrownHelpers::IccState(bot->GetInstanceId()).lastTauntMs[bot->GetGUID()];
    if (last != 0 && getMSTimeDiff(last, now) < 500)
        return false;
    last = now;

    bool const threatReset = botAI->HasCheat(BotCheatMask::raid);

    if (threatReset && target->GetVictim() != bot)
    {
        ThreatManager& mgr = target->GetThreatMgr();
        mgr.AddThreat(bot, 1000000.0f, nullptr, true, true);
        mgr.FixateTarget(bot);
    }

    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            if (threatReset)
                bot->RemoveSpellCooldown(SPELL_TAUNT_PALADIN, true);
            if (botAI->CastSpell("hand of reckoning", target))
                return true;
            break;
        case CLASS_DEATH_KNIGHT:
            if (threatReset)
                bot->RemoveSpellCooldown(SPELL_TAUNT_DK, true);
            if (botAI->CastSpell("dark command", target))
                return true;
            break;
        case CLASS_DRUID:
            if (threatReset)
                bot->RemoveSpellCooldown(SPELL_TAUNT_DRUID, true);
            if (botAI->CastSpell("growl", target))
                return true;
            break;
        case CLASS_WARRIOR:
            if (threatReset)
                bot->RemoveSpellCooldown(SPELL_TAUNT_WARRIOR, true);
            if (botAI->CastSpell("taunt", target))
                return true;
            break;
        default:
            break;
    }

    if (botAI->CastSpell("shoot", target) || botAI->CastSpell("throw", target))
        return true;

    return false;
}

std::optional<bool> IccTryClassCC(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsAlive())
        return std::nullopt;

    char const* spell = nullptr;
    switch (bot->getClass())
    {
        case CLASS_MAGE:         spell = "Frost Nova"; break;
        case CLASS_DRUID:        spell = "Entangling Roots"; break;
        case CLASS_PALADIN:      spell = "Hammer of Justice"; break;
        case CLASS_WARRIOR:      spell = "Hamstring"; break;
        case CLASS_HUNTER:       spell = "Concussive Shot"; break;
        case CLASS_ROGUE:        spell = "Kidney Shot"; break;
        case CLASS_SHAMAN:       spell = "Frost Shock"; break;
        case CLASS_DEATH_KNIGHT: spell = "Chains of Ice"; break;
        case CLASS_PRIEST:       spell = "Psychic Scream"; break;
        case CLASS_WARLOCK:      spell = "Fear"; break;
        default:                 return std::nullopt;
    }

    if (botAI->HasAura(spell, target))
        return std::nullopt;

    uint32 const now = getMSTime();
    uint32& last = IcecrownHelpers::IccState(bot->GetInstanceId()).lastCcMs[bot->GetGUID()];
    if (last != 0 && getMSTimeDiff(last, now) < 500)
        return std::nullopt;
    last = now;

    return botAI->CastSpell(spell, target);
}

void IccEnsureIconOn(Player* bot, PlayerbotAI* botAI, int8 icon, Unit* target)
{
    if (!target)
        return;

    Group* group = bot->GetGroup();
    if (!group)
        return;

    Unit* current = botAI->GetUnit(group->GetTargetIcon(icon));
    if (!current || !current->IsAlive() || current != target)
        group->SetTargetIcon(icon, bot->GetGUID(), target->GetGUID());
}

void IccApplyHeroicBuffToMember(Player* member, bool applyPainSupp, bool applyNoThreat)
{
    uint32 const now = getMSTime();
    uint32& last = IcecrownHelpers::IccState(member->GetInstanceId()).lastBuffMs[member->GetGUID()];
    if (last != 0 && getMSTimeDiff(last, now) < 500)
        return;
    last = now;

    if (!member->HasAura(SPELL_EXPERIENCED))
        member->AddAura(SPELL_EXPERIENCED, member);

    if (!member->HasAura(SPELL_AGEIS_OF_DALARAN))
        member->AddAura(SPELL_AGEIS_OF_DALARAN, member);

    if (applyPainSupp && !member->HasAura(SPELL_PAIN_SUPPRESION))
        member->AddAura(SPELL_PAIN_SUPPRESION, member);

    if (applyNoThreat && !PlayerbotAI::IsTank(member) && !member->HasAura(SPELL_NO_THREAT))
        member->AddAura(SPELL_NO_THREAT, member);
}

void IccStripExperienced(Player* bot)
{
    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);
}

bool IccBossCastingBlisteringCold(Unit* boss)
{
    return boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                    boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                    boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                    boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4));
}

bool IccBossCastingRemorselessWinter(Unit* boss)
{
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    return boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER1) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER2) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER3) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER4) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER5) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER6) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER7) ||
           boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER8);
}

bool IccBossHasRemorselessWinter(Unit* boss)
{
    if (!boss)
        return false;

    if (boss->HasAura(SPELL_REMORSELESS_WINTER1) || boss->HasAura(SPELL_REMORSELESS_WINTER2) ||
        boss->HasAura(SPELL_REMORSELESS_WINTER3) || boss->HasAura(SPELL_REMORSELESS_WINTER4) ||
        boss->HasAura(SPELL_REMORSELESS_WINTER5) || boss->HasAura(SPELL_REMORSELESS_WINTER6) ||
        boss->HasAura(SPELL_REMORSELESS_WINTER7) || boss->HasAura(SPELL_REMORSELESS_WINTER8))
        return true;

    return IccBossCastingRemorselessWinter(boss);
}

bool IccAnyGroupMemberHasAura(Player* bot, uint32 spellId)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(spellId))
            return true;
    }

    return false;
}

bool IccAnyBloodPrincePresent(PlayerbotAI* botAI)
{
    auto* context = botAI->GetAiObjectContext();
    return AI_VALUE2(Unit*, "find target", "prince valanar") ||
           AI_VALUE2(Unit*, "find target", "prince taldaram") ||
           AI_VALUE2(Unit*, "find target", "prince keleseth");
}

std::optional<bool> IccValithriaShouldHealRaid(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return std::nullopt;

    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> druidGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || IsSelfBot(bot))
            continue;

        if (PlayerbotAI::IsHeal(member) && !IsSelfBot(bot))
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
            if (member->getClass() == CLASS_DRUID)
                druidGuids.push_back(member->GetGUID());
        }
    }

    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(druidGuids.begin(), druidGuids.end());

    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return std::nullopt;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);
    bool isDruid = (bot->getClass() == CLASS_DRUID);

    bool shouldHealRaid = false;
    int druidCount = druidGuids.size();

    if (druidCount > 0)
    {
        if (isDruid)
        {
            int raidHealersNeeded = healerCount > 3 ? 2 : 1;
            int druidIndex =
                std::distance(druidGuids.begin(), std::find(druidGuids.begin(), druidGuids.end(), bot->GetGUID()));
            shouldHealRaid = (druidIndex < raidHealersNeeded);
        }
        else if (healerCount > 3 && druidCount == 1)
        {
            if (healerIndex == (healerCount - 1))
                shouldHealRaid = true;
        }
    }
    else if (healerCount > 3)
        shouldHealRaid = (healerIndex >= (healerCount - 2));
    else
        shouldHealRaid = (healerIndex == (healerCount - 1));

    return shouldHealRaid;
}
