/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */
 
#include "GenericBuffUtils.h"

#include "Player.h"
#include "Group.h"
#include "SpellMgr.h"
#include "Chat.h"
#include "PlayerbotAI.h"
#include "ServerFacade.h"
#include "AiObjectContext.h"
#include "Value.h" 


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

        return std::string();
    }

    bool HasRequiredReagents(Player* bot, uint32 spellId)
    {
        if (!spellId)
            return false;

        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
        {
		    for (int i = 0; i < 8; ++i)
            {
                if (info->Reagent[i] > 0)
                {
                    uint32 const itemId = info->Reagent[i];
                    int32  const need   = info->ReagentCount[i];
                    if ((int32)bot->GetItemCount(itemId, false) < need)
                        return false;
                }
            }
            // No composant to buff greater
            return true;
        }
        return false;
    }

    std::string UpgradeToGroupIfAppropriate(
        Player* bot,
        PlayerbotAI* botAI,
        std::string const& baseName,
        bool announceOnMissing,
        std::function<void(std::string const&)> announce
    )
    {
        std::string castName = baseName;

        Group* g = bot->GetGroup();
        if (!g || g->GetMembersCount() <= 2)
            return castName; // duo ou solo: rester en mono pour économiser les composants

        if (std::string const groupName = GroupVariantFor(baseName); !groupName.empty())
        {
         uint32 const groupId = botAI->GetAiObjectContext()
              ->GetValue<uint32>("spell id", groupName)->Get();
          
          // we test the utility of **base** buff (no greater version), because "spell cast useful" may be false for Greater variant.
          bool const usefulBase = botAI->GetAiObjectContext()
              ->GetValue<bool>("spell cast useful", baseName)->Get();
          
          if (groupId && HasRequiredReagents(bot, groupId))
          {
              // spell active + componants OK -> Buff with Greater
              return groupName;
          }
          
          if (announceOnMissing && groupId && usefulBase && announce)
          {
              static time_t s_lastWarn = 0;
              time_t now = std::time(nullptr);
              if (!s_lastWarn || now - s_lastWarn >= 30)
              {
                  std::string rp;
                  if (groupName.find("greater blessing") != std::string::npos)
                      rp = "By the Light... I forgot my Symbols of Kings. We’ll make do with the simple blessings!";
                  else if (groupName == "gift of the wild")
                      rp = "Nature is generous, my bags are not... out of herbs for Gift of the Wild. Take Mark of the Wild for now!";
                  else if (groupName == "arcane brilliance")
                      rp = "Out of Arcane Powder... Brilliance will have to wait. Casting simple Intellect!";
                  else
                      rp = "Oops, I’m out of components for the group version. We’ll go with the single one!";
          
                  announce(rp);
                  s_lastWarn = now;
              }
          }   
        }

        return castName;
    }
}
