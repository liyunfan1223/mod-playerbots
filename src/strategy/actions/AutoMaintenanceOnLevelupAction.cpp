#include "AutoMaintenanceOnLevelupAction.h"

#include "GuildMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "BroadcastHelper.h"

bool AutoMaintenanceOnLevelupAction::Execute(Event event)
{
    AutoPickTalents();
    AutoLearnSpell();
    AutoUpgradeEquip();
    AutoTeleportForLevel();
    return true;
}

void AutoMaintenanceOnLevelupAction::AutoTeleportForLevel()
{
    if (!sPlayerbotAIConfig->autoTeleportForLevel || !sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        return;
    }
    if (botAI->HasRealPlayerMaster())
    {
        return;
    }
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
    return;
}

void AutoMaintenanceOnLevelupAction::AutoPickTalents()
{
    if (!sPlayerbotAIConfig->autoPickTalents || !sRandomPlayerbotMgr->IsRandomBot(bot))
        return;

    if (bot->GetFreeTalentPoints() <= 0)
        return;

    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitTalentsTree(true, true, true);
    factory.InitPetTalents();
}

void AutoMaintenanceOnLevelupAction::AutoLearnSpell()
{
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
    return;
}

void AutoMaintenanceOnLevelupAction::LearnSpells(std::ostringstream* out)
{
    BroadcastHelper::BroadcastLevelup(botAI, bot);
    if (sPlayerbotAIConfig->autoLearnTrainerSpells && sRandomPlayerbotMgr->IsRandomBot(bot))
        LearnTrainerSpells(out);

    if (sPlayerbotAIConfig->autoLearnQuestSpells && sRandomPlayerbotMgr->IsRandomBot(bot))
        LearnQuestSpells(out);
}

void AutoMaintenanceOnLevelupAction::LearnTrainerSpells(std::ostringstream* out)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitClassSpells();
    factory.InitAvailableSpells();
    factory.InitSkills();
    factory.InitPet();
}

void AutoMaintenanceOnLevelupAction::LearnQuestSpells(std::ostringstream* out)
{
    // CreatureTemplate const* co = sCreatureStorage.LookupEntry<CreatureTemplate>(id);
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 questId = i->first;
        Quest const* quest = i->second;

        if (!quest->GetRequiredClasses() || quest->IsRepeatable() || quest->GetMinLevel() < 10)
            continue;

        if (!bot->SatisfyQuestClass(quest, false) || quest->GetMinLevel() > bot->GetLevel() ||
            !bot->SatisfyQuestRace(quest, false))
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

std::string const AutoMaintenanceOnLevelupAction::FormatSpell(SpellInfo const* sInfo)
{
    std::ostringstream out;
    std::string const rank = sInfo->Rank[0];

    if (rank.empty())
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << "]|h|r";
    else
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << " " << rank << "]|h|r";

    return out.str();
}

void AutoMaintenanceOnLevelupAction::LearnSpell(uint32 spellId, std::ostringstream* out)
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

void AutoMaintenanceOnLevelupAction::AutoUpgradeEquip()
{
    if (!sPlayerbotAIConfig->autoUpgradeEquip || !sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        return;
    }
    PlayerbotFactory factory(bot, bot->GetLevel());
    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel)
    {
        factory.InitEquipment(true);
    }
    factory.InitAmmo();
    return;
}

