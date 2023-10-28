/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TrainerAction.h"
#include "BudgetValues.h"
#include "Event.h"
#include "Playerbots.h"

void TrainerAction::Learn(uint32 cost, TrainerSpell const* tSpell, std::ostringstream& msg)
{
    if (sPlayerbotAIConfig->autoTrainSpells != "free" && !botAI->HasCheat(BotCheatMask::gold))
    {
        if (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells) < cost)
        {
            msg << " - too expensive";
            return;
        }

        bot->ModifyMoney(-int32(cost));
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(tSpell->spell);
    if (!spellInfo)
        return;

    bool learned = false;
    for (uint8 j = 0; j < 3; ++j)
    {
        if (spellInfo->Effects[j].Effect == SPELL_EFFECT_LEARN_SPELL)
        {
            uint32 learnedSpell = spellInfo->Effects[j].TriggerSpell;
            bot->learnSpell(learnedSpell);
            learned = true;
        }
    }

    if (!learned)
        bot->learnSpell(tSpell->spell);

    msg << " - learned";
}

void TrainerAction::Iterate(Creature* creature, TrainerSpellAction action, SpellIds& spells)
{
    TellHeader(creature);

    TrainerSpellData const* trainer_spells = creature->GetTrainerSpells();
    float fDiscountMod =  bot->GetReputationPriceDiscount(creature);
    uint32 totalCost = 0;

    for (TrainerSpellMap::const_iterator itr =  trainer_spells->spellList.begin(); itr !=  trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;
        if (!tSpell)
            continue;

        TrainerSpellState state = bot->GetTrainerSpellState(tSpell);
        if (state != TRAINER_SPELL_GREEN)
            continue;

        uint32 spellId = tSpell->spell;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (!spells.empty() && spells.find(tSpell->spell) == spells.end())
            continue;

        uint32 cost = uint32(floor(tSpell->spellCost *  fDiscountMod));
        totalCost += cost;

        std::ostringstream out;
        out << chat->FormatSpell(spellInfo) << chat->formatMoney(cost);

        if (action)
            (this->*action)(cost, tSpell, out);

        botAI->TellMaster(out);
    }

    TellFooter(totalCost);
}

bool TrainerAction::Execute(Event event)
{
    std::string const text = event.getParam();

    Player* master = GetMaster();

    Creature* creature = botAI->GetCreature(bot->GetTarget());

    if (master) {
        creature = master->GetSelectedUnit() ? master->GetSelectedUnit()->ToCreature() : nullptr;
    }
    // if (AI_VALUE(GuidPosition, "rpg target") != bot->GetTarget())
    //     if (master)
    //         creature = botAI->GetCreature(master->GetTarget());
    //     else
    //         return false;

    if (!creature || !creature->IsTrainer())
        return false;

    if (!creature->IsValidTrainerForPlayer(bot))
    {
        botAI->TellError("This trainer cannot teach me");
        return false;
    }

    // check present spell in trainer spell list
    TrainerSpellData const* cSpells = creature->GetTrainerSpells();
    if (!cSpells)
    {
        botAI->TellError("No spells can be learned from this trainer");
        return false;
    }

    uint32 spell = chat->parseSpell(text);
    SpellIds spells;
    if (spell)
        spells.insert(spell);

    if (text.find("learn") != std::string::npos || sRandomPlayerbotMgr->IsRandomBot(bot) || (sPlayerbotAIConfig->autoTrainSpells != "no" &&
        (creature->GetCreatureTemplate()->trainer_type != TRAINER_TYPE_TRADESKILLS || !botAI->HasActivePlayerMaster()))) //Todo rewrite to only exclude start primary profession skills and make config dependent.
        Iterate(creature, &TrainerAction::Learn, spells);
    else
        Iterate(creature, nullptr, spells);

    return true;
}

void TrainerAction::TellHeader(Creature* creature)
{
    std::ostringstream out;
    out << "--- Can learn from " << creature->GetName() << " ---";
    botAI->TellMaster(out);
}

void TrainerAction::TellFooter(uint32 totalCost)
{
    if (totalCost)
    {
        std::ostringstream out;
        out << "Total cost: " << chat->formatMoney(totalCost);
        botAI->TellMaster(out);
    }
}
