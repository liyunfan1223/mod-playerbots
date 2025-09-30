/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericBuffUtils.h"
#include "PlayerbotAIConfig.h"

#include <map>

#include "Player.h"
#include "Group.h"
#include "SpellMgr.h"
#include "Chat.h"
#include "PlayerbotAI.h"
#include "ServerFacade.h"
#include "AiObjectContext.h"
#include "Value.h"
#include "Config.h"
#include "PlayerbotTextMgr.h"

namespace ai::buff
{
  std::string MakeAuraQualifierForBuff(std::string const& name)
  {
      // Paladin
      if (name == "blessing of kings")        return "blessing of kings,greater blessing of kings";
      if (name == "blessing of might")        return "blessing of might,greater blessing of might";
      if (name == "blessing of wisdom")       return "blessing of wisdom,greater blessing of wisdom";
      if (name == "blessing of sanctuary")    return "blessing of sanctuary,greater blessing of sanctuary";
      // Druid
      if (name == "mark of the wild")         return "mark of the wild,gift of the wild";
      // Mage
      if (name == "arcane intellect")         return "arcane intellect,arcane brilliance";
      // Priest
      if (name == "power word: fortitude")    return "power word: fortitude,prayer of fortitude";
      return name;
  }

  std::string GroupVariantFor(std::string const& name)
  {
      // Paladin
      if (name == "blessing of kings")        return "greater blessing of kings";
      if (name == "blessing of might")        return "greater blessing of might";
      if (name == "blessing of wisdom")       return "greater blessing of wisdom";
      if (name == "blessing of sanctuary")    return "greater blessing of sanctuary";
      // Druid
      if (name == "mark of the wild")         return "gift of the wild";
      // Mage
      if (name == "arcane intellect")         return "arcane brilliance";
      // Priest
      if (name == "power word: fortitude")    return "prayer of fortitude";

      return std::string();
  }

  bool HasRequiredReagents(Player* bot, uint32 spellId)
  {
      if (!spellId)
          return false;

      if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
      {        for (int i = 0; i < MAX_SPELL_REAGENTS; ++i)
          {
              if (info->Reagent[i] > 0)
              {
                  uint32 const itemId = info->Reagent[i];
                  int32  const need   = info->ReagentCount[i];
                  if ((int32)bot->GetItemCount(itemId, false) < need)
                      return false;
              }
          }
          // No reagent required
          return true;
      }
      return false;
  }

  std::string UpgradeToGroupIfAppropriate(
      Player* bot,
      PlayerbotAI* botAI,
      std::string const& baseName,
      bool announceOnMissing,
      std::function<void(std::string const&)> announce)
  {
     std::string castName = baseName;        Group* g = bot->GetGroup();
     if (!g || g->GetMembersCount() < static_cast<uint32>(sPlayerbotAIConfig->minBotsForGreaterBuff))
         return castName; // Group too small: stay in solo mode

     if (std::string const groupName = GroupVariantFor(baseName); !groupName.empty())
     {
         uint32 const groupVariantSpellId = botAI->GetAiObjectContext()
             ->GetValue<uint32>("spell id", groupName)->Get();

         // We check usefulness on the **basic** buff (not the greater version),
         // because "spell cast useful" may return false for the greater variant.
         bool const usefulBase = botAI->GetAiObjectContext()
             ->GetValue<bool>("spell cast useful", baseName)->Get();

         if (groupVariantSpellId && HasRequiredReagents(bot, groupVariantSpellId))
         {
             // Learned + reagents OK -> switch to greater
             return groupName;
         }

         // Missing reagents -> announce if (a) greater is known, (b) base buff is useful,
         // (c) announce was requested, (d) a callback is provided.
         if (announceOnMissing && groupVariantSpellId && usefulBase && announce)
         {
           static std::map<std::pair<uint32, std::string>, time_t> s_lastWarn; // par bot & par buff
           time_t now = std::time(nullptr);
           uint32 botLow = static_cast<uint32>(bot->GetGUID().GetCounter());
           time_t& last = s_lastWarn[ std::make_pair(botLow, groupName) ];
           if (!last || now - last >= sPlayerbotAIConfig->rpWarningCooldown) // Configurable anti-spam
           {
             // DB Key choice in regard of the buff
             std::string key;
             if (groupName.find("greater blessing") != std::string::npos)
                 key = "rp_missing_reagent_greater_blessing";
             else if (groupName == "gift of the wild")
                 key = "rp_missing_reagent_gift_of_the_wild";
             else if (groupName == "arcane brilliance")
                 key = "rp_missing_reagent_arcane_brilliance";
             else
                 key = "rp_missing_reagent_generic";

             // Placeholders
             std::map<std::string, std::string> placeholders;
             placeholders["%group_spell"] = groupName;
             placeholders["%base_spell"] = baseName;

             std::string announceText = sPlayerbotTextMgr->GetBotTextOrDefault(key,
                 "Out of components for %group_spell. Using %base_spell!", placeholders);

             announce(announceText);
             last = now;
           }
         }
     }
    return castName;
  }
}
