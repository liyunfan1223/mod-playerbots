/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AutoLearnSpellAction.h"
#include "Event.h"
#include "Playerbots.h"

bool AutoLearnSpellAction::Execute(Event event)
{
    std::string const param = event.getParam();

    std::ostringstream out;
    LearnSpells(&out);

    if (!out.str().empty())
    {
        std::string const temp = out.str();
        out.seekp(0);
        out << "Learned spells: ";
        out << temp;
        out.seekp(-2, out.cur);
        out << ".";
        botAI->TellMaster(out);
    }

    return true;
}


void AutoLearnSpellAction::LearnSpells(std::ostringstream* out)
{
    if (sPlayerbotAIConfig->autoLearnTrainerSpells)// || (!botAI->GetMaster() && sRandomPlayerbotMgr->IsRandomBot(bot)))
        LearnTrainerSpells(out);

    if (sPlayerbotAIConfig->autoLearnQuestSpells)// || (!botAI->GetMaster() && sRandomPlayerbotMgr->IsRandomBot(bot)))
        LearnQuestSpells(out);
}

void AutoLearnSpellAction::LearnTrainerSpells(std::ostringstream* out)
{
    bot->LearnDefaultSkills();
    bot->LearnCustomSpells();

    CreatureTemplateContainer const* creatures = sObjectMgr->GetCreatureTemplates();
    for (CreatureTemplateContainer::const_iterator itr = creatures->begin(); itr != creatures->end(); ++itr)
    {
        if (itr->second.trainer_type != TRAINER_TYPE_CLASS && itr->second.trainer_type != TRAINER_TYPE_TRADESKILLS)
            continue;

        if (itr->second.trainer_type == TRAINER_TYPE_CLASS && itr->second.trainer_class != bot->getClass())
            continue;

        TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(itr->second.Entry);
        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator iter = trainer_spells->spellList.begin(); iter != trainer_spells->spellList.end(); ++iter)
        {
            TrainerSpell const* tSpell = &iter->second;
            if (!tSpell)
                continue;

            TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
            if (state != TRAINER_SPELL_GREEN)
                continue;

            if (itr->second.trainer_type == TRAINER_TYPE_TRADESKILLS)
            {
                SpellInfo const* spell = sSpellMgr->GetSpellInfo(tSpell->spell);
                if (spell)
                {
                    std::string const SpellName = spell->SpellName[0];
                    if (spell->Effects[EFFECT_1].Effect == SPELL_EFFECT_SKILL_STEP)
                    {
                        uint32 skill = spell->Effects[EFFECT_1].MiscValue;
                        if (skill)
                        {
                            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
                            if (pSkill)
                            {
                                if (SpellName.find("Apprentice") != std::string::npos && pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY)
                                    continue;
                            }
                        }
                    }
                }
            }

            LearnSpell(tSpell->spell, out);
        }
    }
}

void AutoLearnSpellAction::LearnQuestSpells(std::ostringstream* out)
{
    //CreatureTemplate const* co = sCreatureStorage.LookupEntry<CreatureTemplate>(id);
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 questId = i->first;
        Quest const* quest = i->second;

        if (!quest->GetRequiredClasses() || quest->IsRepeatable() || quest->GetMinLevel() < 10)
            continue;

        if (!bot->SatisfyQuestClass(quest, false) || quest->GetMinLevel() > bot->getLevel() || !bot->SatisfyQuestRace(quest, false))
            continue;

        if (quest->GetRewSpellCast() > 0)
        {
            LearnSpell(quest->GetRewSpellCast(), out);
        }
        else if (quest->GetRewSpell() > 0)
        {
            LearnSpell(quest->GetRewSpell(), out);
        }
    }
}

std::string const FormatSpell(SpellInfo const* sInfo)
{
    std::ostringstream out;
    std::string const rank = sInfo->Rank[0];

    if (rank.empty())
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << "]|h|r";
    else
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << " " << rank << "]|h|r";

    return out.str();
}

void AutoLearnSpellAction::LearnSpell(uint32 spellId, std::ostringstream* out)
{
    SpellInfo const* proto = sSpellMgr->GetSpellInfo(spellId);
    if (!proto)
        return;

    bool learned = false;
    for (uint8 j = 0; j < 3; ++j)
    {
        if (proto->Effects[j].Effect == SPELL_EFFECT_LEARN_SPELL)
        {
            uint32 learnedSpell = proto->Effects[j].TriggerSpell;

            if (!bot->HasSpell(learnedSpell))
            {
                bot->learnSpell(learnedSpell);
                *out << FormatSpell(sSpellMgr->GetSpellInfo(learnedSpell)) << ", ";
            }

            learned = true;
        }
    }

    if (!learned)
    {
        if (!bot->HasSpell(spellId))
        {
            bot->learnSpell(spellId);
            *out << FormatSpell(proto) << ", ";
        }
    }
}
