/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AutoLearnSpellAction.h"
#include "Event.h"
#include "PlayerbotFactory.h"
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
    if (sPlayerbotAIConfig->autoLearnTrainerSpells && sRandomPlayerbotMgr->IsRandomBot(bot))// || (!botAI->GetMaster() && sRandomPlayerbotMgr->IsRandomBot(bot)))
        LearnTrainerSpells(out);

    if (sPlayerbotAIConfig->autoLearnQuestSpells && sRandomPlayerbotMgr->IsRandomBot(bot))// || (!botAI->GetMaster() && sRandomPlayerbotMgr->IsRandomBot(bot)))
        LearnQuestSpells(out);
}

void AutoLearnSpellAction::LearnTrainerSpells(std::ostringstream* out)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitClassSpells();
    factory.InitAvailableSpells();
    factory.InitSkills();
    factory.InitPet();
    // bot->LearnDefaultSkills();

    // CreatureTemplateContainer const* creatureTemplateContainer = sObjectMgr->GetCreatureTemplates();
    // for (CreatureTemplateContainer::const_iterator i = creatureTemplateContainer->begin(); i != creatureTemplateContainer->end(); ++i)
    // {
    //     CreatureTemplate const& co = i->second;
    //     if (co.trainer_type != TRAINER_TYPE_TRADESKILLS && co.trainer_type != TRAINER_TYPE_CLASS)
    //         continue;

    //     if (co.trainer_type == TRAINER_TYPE_CLASS && co.trainer_class != bot->getClass())
    //         continue;

	// 	uint32 trainerId = co.Entry;

	// 	TrainerSpellData const* trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);
    //     if (!trainer_spells)
    //         trainer_spells = sObjectMgr->GetNpcTrainerSpells(trainerId);

    //     if (!trainer_spells)
    //         continue;

    //     for (TrainerSpellMap::const_iterator itr =  trainer_spells->spellList.begin(); itr !=  trainer_spells->spellList.end(); ++itr)
    //     {
    //         TrainerSpell const* tSpell = &itr->second;

    //         if (!tSpell)
    //             continue;

    //         if (!tSpell->learnedSpell[0] && !bot->IsSpellFitByClassAndRace(tSpell->learnedSpell[0]))
    //             continue;

    //         TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
    //         if (state != TRAINER_SPELL_GREEN)
    //             continue;

    //         SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(tSpell->spell);
    //         bool learn = true;
    //         for (uint8 j = 0; j < 3; ++j)
    //         {
    //             if (!tSpell->learnedSpell[j] && !bot->IsSpellFitByClassAndRace(tSpell->learnedSpell[j]))
    //                 continue;

    //             if (spellInfo->Effects[j].Effect == SPELL_EFFECT_PROFICIENCY ||
    //                 spellInfo->Effects[j].Effect == SPELL_EFFECT_SKILL_STEP ||
    //                 spellInfo->Effects[j].Effect == SPELL_EFFECT_DUAL_WIELD)
    //             {
    //                 learn = false;
    //                 break;
    //             }
    //         }
    //         if (!learn) {
    //             continue;
    //         }

    //         if (tSpell->learnedSpell[0]) {
    //             bot->learnSpell(tSpell->learnedSpell[0], false);
    //         }
    //         else {
    //             LOG_INFO("playerbots", "!tSpell->learnedSpell[0] {}", tSpell->spell);
    //             botAI->CastSpell(tSpell->spell, bot);
    //         }
    //     }
    // }
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
