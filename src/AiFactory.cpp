/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AiFactory.h"
#include "BattlegroundMgr.h"
#include "Item.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Engine.h"
#include "Group.h"
#include "DKAiObjectContext.h"
#include "PriestAiObjectContext.h"
#include "MageAiObjectContext.h"
#include "SharedDefines.h"
#include "WarlockAiObjectContext.h"
#include "WarriorAiObjectContext.h"
#include "ShamanAiObjectContext.h"
#include "PaladinAiObjectContext.h"
#include "DruidAiObjectContext.h"
#include "HunterAiObjectContext.h"
#include "RogueAiObjectContext.h"

AiObjectContext* AiFactory::createAiObjectContext(Player* player, PlayerbotAI* botAI)
{
    switch (player->getClass())
    {
        case CLASS_PRIEST:
            return new PriestAiObjectContext(botAI);
        case CLASS_MAGE:
            return new MageAiObjectContext(botAI);
        case CLASS_WARLOCK:
            return new WarlockAiObjectContext(botAI);
        case CLASS_WARRIOR:
            return new WarriorAiObjectContext(botAI);
        case CLASS_SHAMAN:
            return new ShamanAiObjectContext(botAI);
        case CLASS_PALADIN:
            return new PaladinAiObjectContext(botAI);
        case CLASS_DRUID:
            return new DruidAiObjectContext(botAI);
        case CLASS_HUNTER:
            return new HunterAiObjectContext(botAI);
        case CLASS_ROGUE:
            return new RogueAiObjectContext(botAI);
        case CLASS_DEATH_KNIGHT:
            return new DKAiObjectContext(botAI);
    }

    return new AiObjectContext(botAI);
}

uint8 AiFactory::GetPlayerSpecTab(Player* bot)
{
    std::map<uint8, uint32> tabs = GetPlayerSpecTabs(bot);

    if (bot->getLevel() >= 10 && ((tabs[0] + tabs[1] + tabs[2]) > 0))
    {
        int8 tab = -1;
        uint32 max = 0;
        for (uint32 i = 0; i < uint32(3); i++)
        {
            if (tab == -1 || max < tabs[i])
            {
                tab = i;
                max = tabs[i];
            }
        }

        return tab;
    }
    else
    {
        uint8 tab = 0;

        switch (bot->getClass())
        {
            case CLASS_MAGE:
                tab = 1;
                break;
            case CLASS_PALADIN:
                tab = 2;
                break;
            case CLASS_PRIEST:
                tab = 1;
                break;
        }

        return tab;
    }
}

std::map<uint8, uint32> AiFactory::GetPlayerSpecTabs(Player* bot)
{
    std::map<uint8, uint32> tabs = {{0, 0}, {0, 0}, {0, 0}};
    const PlayerTalentMap& talentMap = bot->GetTalentMap();
    for (PlayerTalentMap::const_iterator i = talentMap.begin(); i != talentMap.end(); ++i)
    {
        uint32 spellId = i->first;
        if ((bot->GetActiveSpecMask() & i->second->specMask) == 0) {
            continue;
        }
        TalentSpellPos const* talentPos = GetTalentSpellPos(spellId);
        if(!talentPos)
            continue;
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id);
        if (!talentInfo)
            continue;

        uint32 const* talentTabIds = GetTalentTabPages(bot->getClass());
        if (talentInfo->TalentTab == talentTabIds[0]) tabs[0]++;
        if (talentInfo->TalentTab == talentTabIds[1]) tabs[1]++;
        if (talentInfo->TalentTab == talentTabIds[2]) tabs[2]++;
    }
    return tabs;
}

BotRoles AiFactory::GetPlayerRoles(Player* player)
{
    BotRoles role = BOT_ROLE_NONE;
    uint8 tab = GetPlayerSpecTab(player);

    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
                role = BOT_ROLE_DPS;
            else
                role = BOT_ROLE_HEALER;
            break;
        case CLASS_SHAMAN:
            if (tab == 2)
                role = BOT_ROLE_HEALER;
            else
                role = BOT_ROLE_DPS;
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                role = BOT_ROLE_TANK;
            else
                role = BOT_ROLE_DPS;
            break;
        case CLASS_PALADIN:
            if (tab == 0)
                role = BOT_ROLE_HEALER;
            else if (tab == 1)
                role = BOT_ROLE_TANK;
            else if (tab == 2)
                role = BOT_ROLE_DPS;
            break;
        case CLASS_DRUID:
            if (tab == 0)
                role = BOT_ROLE_DPS;
            else if (tab == 1)
                role = (BotRoles)(BOT_ROLE_TANK | BOT_ROLE_DPS);
            else if (tab == 2)
                role = BOT_ROLE_HEALER;
            break;
        default:
            role = BOT_ROLE_DPS;
            break;
    }

    return role;
}

std::string AiFactory::GetPlayerSpecName(Player* player)
{
    std::string specName;
    int tab = GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
                specName = "shadow";
            else if (tab == 1)
                specName = "holy";
            else
                specName = "disc";
            ;        break;
        case CLASS_SHAMAN:
            if (tab == 2)
                specName = "resto";
            else if (tab == 1)
                specName = "enhance";
            else
                specName = "elem";
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                specName = "prot";
            else if (tab == 1)
                specName = "fury";
            else
                specName = "arms";
            break;
        case CLASS_PALADIN:
            if (tab == 0)
                specName = "holy";
            else if (tab == 1)
                specName = "prot";
            else if (tab == 2)
                specName = "retrib";
            break;
        case CLASS_DRUID:
            if (tab == 0)
                specName = "balance";
            else if (tab == 1)
                specName = "feraldps";
            else if (tab == 2)
                specName = "resto";
            break;
        case CLASS_ROGUE:
            if (tab == 0)
                specName = "assas";
            else if (tab == 1)
                specName = "combat";
            else if (tab == 2)
                specName = "subtle";
            break;
        case CLASS_HUNTER:
            if (tab == 0)
                specName = "beast";
            else if (tab == 1)
                specName = "marks";
            else if (tab == 2)
                specName = "surv";
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                specName = "blooddps";
            else if (tab == 1)
                specName = "frostdps";
            else if (tab == 2)
                specName = "unholydps";
            break;
        case CLASS_MAGE:
            if (tab == 0)
                specName = "arcane";
            else if (tab == 1)
                specName = "fire";
            else if (tab == 2)
                specName = "frost";
            break;
        case CLASS_WARLOCK:
            if (tab == 0)
                specName = "afflic";
            else if (tab == 1)
                specName = "demo";
            else if (tab == 2)
                specName = "destro";
            break;
        default:
            break;
    }

    return specName;
}

void AiFactory::AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* engine)
{
    uint8 tab = GetPlayerSpecTab(player);

    if (!player->InBattleground())
    {
        engine->addStrategies("racials", "chat", "default", "cast time", "duel", "boost", nullptr);
    }

    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
            {
                engine->addStrategies("dps", "shadow debuff", "shadow aoe", "threat", nullptr);
            }
            else
                engine->addStrategies("heal", "threat", nullptr);

            engine->addStrategies("dps assist", "cure", nullptr);
            break;
        case CLASS_MAGE:
            if (tab == 0)
                engine->addStrategies("arcane", "arcane aoe", "threat", nullptr);
            else if (tab == 1)
                engine->addStrategies("fire", "fire aoe", "threat", nullptr);
            else
                engine->addStrategies("frost", "frost aoe", "threat", nullptr);

            engine->addStrategies("dps", "dps assist", "cure", nullptr);
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                engine->addStrategies("tank", "tank assist", "aoe", "mark rti", nullptr);
            else if (player->getLevel() < 36 || tab == 0)
                engine->addStrategies("arms", "aoe", "dps assist", "threat", /*"behind",*/ nullptr);
            else
                engine->addStrategies("fury", "aoe", "dps assist", "threat", /*"behind",*/ nullptr);
            break;
        case CLASS_SHAMAN:
            if (tab == 0)
                engine->addStrategies("caster", "caster aoe", "bmana", "threat", nullptr);
            else if (tab == 2)
                engine->addStrategies("heal", "bmana", nullptr);
            else
                engine->addStrategies("melee", "melee aoe", "bdps", "threat", nullptr);

            engine->addStrategies("dps assist", "cure", "totems", nullptr);
            break;
        case CLASS_PALADIN:
            if (tab == 1)
                engine->addStrategies("tank", "tank assist", "bthreat", "barmor", "cure", nullptr);
            else if (tab == 0)
                engine->addStrategies("heal", "dps assist", "cure", "bcast", nullptr);
            else
                engine->addStrategies("dps", "dps assist", "cure", "baoe", nullptr);

            break;
        case CLASS_DRUID:
            if (tab == 0)
            {
                engine->addStrategies("caster", "cure", "caster aoe", "threat", "dps assist", nullptr);
                engine->addStrategy("caster debuff");
            }
            else if (tab == 2)
                engine->addStrategies("heal", "cure", "dps assist", nullptr);
            else
            {
                engine->removeStrategy("flee");
                engine->addStrategies("bear", "tank assist", nullptr);
            }
            break;
        case CLASS_HUNTER:
            engine->addStrategies("dps", "aoe", "bdps", "threat", "dps assist", nullptr);
            engine->addStrategy("dps debuff");
            break;
        case CLASS_ROGUE:
            if (tab == ROGUE_TAB_ASSASSINATION) {
                engine->addStrategies("melee", "threat", "dps assist", "aoe", /*"behind",*/ nullptr);
            } else {
                engine->addStrategies("dps", "threat", "dps assist", "aoe", /*"behind",*/ nullptr);
            }
            break;
        case CLASS_WARLOCK:
            engine->addStrategies("dps assist", "dps", "dps debuff", "aoe", "threat", nullptr);
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                engine->addStrategies("blood", "tank assist", nullptr);
            else if (tab == 1)
                engine->addStrategies("frost", "frost aoe", "dps assist", "threat", nullptr);
            else
                engine->addStrategies("unholy", "unholy aoe", "dps assist", "threat", nullptr);

            break;
    }

    if (facade->IsRealPlayer() || sRandomPlayerbotMgr->IsRandomBot(player))
    {
        if (!player->GetGroup())
        {
            // change for heal spec
            engine->addStrategy("boost");
            engine->addStrategy("dps assist");
            engine->removeStrategy("threat");
            // engine-
            switch (player->getClass()) {
                case CLASS_PRIEST: {
                    if (tab != PRIEST_TAB_SHADOW) {
                        engine->addStrategies("holy", "shadow debuff", "shadow aoe", nullptr);
                    }
                    break;
                }
                case CLASS_DRUID: {
                    if (tab == DRUID_TAB_RESTORATION) {
                        engine->addStrategies("caster", "caster aoe", nullptr);
                        engine->addStrategy("caster debuff");
                    }
                    break;
                }
                case CLASS_SHAMAN: {
                    if (tab == SHAMAN_TAB_RESTORATION) {
                        engine->addStrategies("caster", "caster aoe", "bmana", nullptr);
                    }
                    break;
                }
                case CLASS_PALADIN: {
                    if (tab == PALADIN_TAB_HOLY) {
                        engine->addStrategies("dps", "dps assist", "baoe", nullptr);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    else
    {
        engine->ChangeStrategy(sPlayerbotAIConfig->combatStrategies);
    }

    // Battleground switch
    if (player->InBattleground() && player->GetBattleground())
    {
        BattlegroundTypeId bgType = player->GetBattlegroundTypeId();
        if (bgType == BATTLEGROUND_RB)
            bgType = player->GetBattleground()->GetBgTypeID(true);

        if (bgType == BATTLEGROUND_WS)
            engine->addStrategy("warsong");

        if (bgType == BATTLEGROUND_AB)
            engine->addStrategy("arathi");

        if (bgType == BATTLEGROUND_AV)
            engine->addStrategy("alterac");

        if (bgType == BATTLEGROUND_EY)
            engine->addStrategy("eye");

        if (bgType == BATTLEGROUND_IC)
            engine->addStrategy("isle");

        if (player->InArena())
        {
            engine->addStrategy("arena");
        }

        engine->addStrategies("boost", "racials", "chat", "default", "aoe", "potions", "conserve mana", "cast time", "dps assist", nullptr);
        engine->removeStrategy("custom::say");
        engine->removeStrategy("flee");
        engine->removeStrategy("threat");
        engine->addStrategy("boost");

        if ((player->getClass() == CLASS_DRUID && tab == 2) || (player->getClass() == CLASS_SHAMAN && tab == 2))
            engine->addStrategies("caster", "caster aoe", nullptr);

        if (player->getClass() == CLASS_DRUID && tab == 1)
            engine->addStrategies(/*"behind",*/ "dps", nullptr);

        if (player->getClass() == CLASS_ROGUE)
            engine->addStrategies(/*"behind",*/ "stealth", nullptr);
    }
}

Engine* AiFactory::createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext)
{
	Engine* engine = new Engine(facade, aiObjectContext);
    AddDefaultCombatStrategies(player, facade, engine);
    return engine;
}

void AiFactory::AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine)
{
    uint8 tab = GetPlayerSpecTab(player);

    switch (player->getClass())
    {
        case CLASS_PRIEST:
            nonCombatEngine->addStrategies("dps assist", "cure", nullptr);
            break;
        case CLASS_PALADIN:
            if (tab == 1)
                nonCombatEngine->addStrategies("bthreat", "tank assist", "bstats", "barmor", nullptr);
            else if (tab == 0)
                nonCombatEngine->addStrategies("dps assist", "bmana", "bcast", nullptr);
            else
                nonCombatEngine->addStrategies("dps assist", "bdps", "baoe",nullptr);

            nonCombatEngine->addStrategies("cure", nullptr);
            break;
        case CLASS_HUNTER:
            nonCombatEngine->addStrategies("bdps", "dps assist", "pet", nullptr);
            break;
        case CLASS_SHAMAN:
            if (tab == 0 || tab == 2)
                nonCombatEngine->addStrategy("bmana");
            else
                nonCombatEngine->addStrategy("bdps");

            nonCombatEngine->addStrategies("dps assist", "cure", nullptr);
            break;
        case CLASS_MAGE:
            if (tab == MAGE_TAB_ARCANE || tab == MAGE_TAB_FIRE)
                nonCombatEngine->addStrategy("bdps");
            else
                nonCombatEngine->addStrategy("bmana");

            nonCombatEngine->addStrategies("dps assist", "cure", nullptr);
            break;
        case CLASS_DRUID:
            if (tab == 1)
                nonCombatEngine->addStrategy("tank assist");
            else
                nonCombatEngine->addStrategies("dps assist", "cure", nullptr);
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                nonCombatEngine->addStrategy("tank assist");
            else
                nonCombatEngine->addStrategy("dps assist");
            break;
        case CLASS_WARLOCK:
            if (tab == WARLOCK_TAB_AFFLICATION) {
                nonCombatEngine->addStrategies("bmana", nullptr);
            } else if (tab == WARLOCK_TAB_DEMONOLOGY) {
                nonCombatEngine->addStrategies("bdps", nullptr);
            } else if (tab == WARLOCK_TAB_DESTRUCTION) {
                nonCombatEngine->addStrategies("bhealth", nullptr);
            }
            nonCombatEngine->addStrategies("dps assist", nullptr);
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                nonCombatEngine->addStrategy("tank assist");
            else
                nonCombatEngine->addStrategy("dps assist");
            break;
        default:
            nonCombatEngine->addStrategy("dps assist");
            break;
    }

    if (!player->InBattleground())
    {
        nonCombatEngine->addStrategies("nc", "food", "chat", "follow",
            "default", "quest", "loot", "gather", "duel", "buff", "mount", nullptr);
    }

    if ((facade->IsRealPlayer() || sRandomPlayerbotMgr->IsRandomBot(player)) && !player->InBattleground())
    {
        Player* master = facade->GetMaster();

         // let 25% of free bots start duels.
        if (!urand(0, 3))
            nonCombatEngine->addStrategy("start duel");

        if (sPlayerbotAIConfig->randomBotJoinLfg)
            nonCombatEngine->addStrategy("lfg");

        if (!player->GetGroup() || player->GetGroup()->GetLeaderGUID() == player->GetGUID())
        {
            // let 25% of random not grouped (or grp leader) bots help other players
            // if (!urand(0, 3))
            //     nonCombatEngine->addStrategy("attack tagged");

            nonCombatEngine->addStrategy("pvp");
            // nonCombatEngine->addStrategy("collision");
            nonCombatEngine->addStrategy("grind");
            // nonCombatEngine->addStrategy("group");
            // nonCombatEngine->addStrategy("guild");

            if (sPlayerbotAIConfig->autoDoQuests)
            {
                // nonCombatEngine->addStrategy("travel");
                nonCombatEngine->addStrategy("rpg");
            } else {
                nonCombatEngine->addStrategy("move random");
            }

            if (sPlayerbotAIConfig->randomBotJoinBG)
                nonCombatEngine->addStrategy("bg");

            // if (!master || GET_PLAYERBOT_AI(master))
            //     nonCombatEngine->addStrategy("maintenance");

            nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->randomBotNonCombatStrategies);
        }
        else
        {
            if (facade)
            {
                if (master)
                {
                    PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
                    if (masterBotAI || sRandomPlayerbotMgr->IsRandomBot(player))
                    {
                        nonCombatEngine->addStrategy("pvp");
                        // nonCombatEngine->addStrategy("collision");
                        // nonCombatEngine->addStrategy("group");
                        // nonCombatEngine->addStrategy("guild");

                        // if (sPlayerbotAIConfig->autoDoQuests)
                        // {
                        //     // nonCombatEngine->addStrategy("travel");
                        //     nonCombatEngine->addStrategy("rpg");
                        // } else {
                        //     nonCombatEngine->addStrategy("move random");
                        // }

                        // if (masterBotAI)
                        //     nonCombatEngine->addStrategy("maintenance");

                        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->randomBotNonCombatStrategies);
                    }
                    else {
                        // nonCombatEngine->addStrategy("pvp");
                        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->nonCombatStrategies);
                    }
                }
            }
        }
    }
    else
    {
        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig->nonCombatStrategies);
    }
    // nonCombatEngine->addStrategy("battleground");
    // nonCombatEngine->addStrategy("warsong");
    // Battleground switch
    if (player->InBattleground() && player->GetBattleground())
    {
        nonCombatEngine->addStrategies("nc", "chat", "default", "buff", "food", "mount", "pvp", "dps assist", "attack tagged", nullptr);
        nonCombatEngine->removeStrategy("custom::say");
        nonCombatEngine->removeStrategy("travel");
        nonCombatEngine->removeStrategy("rpg");
        nonCombatEngine->removeStrategy("grind");

        BattlegroundTypeId bgType = player->GetBattlegroundTypeId();
        if (bgType == BATTLEGROUND_RB)
            bgType = player->GetBattleground()->GetBgTypeID(true);

        if (bgType <= BATTLEGROUND_EY || bgType == BATTLEGROUND_IC) // do not add for not supported bg
            nonCombatEngine->addStrategy("battleground");

        if (bgType == BATTLEGROUND_WS)
            nonCombatEngine->addStrategy("warsong");

        if (bgType == BATTLEGROUND_AV)
            nonCombatEngine->addStrategy("alterac");

        if (bgType == BATTLEGROUND_AB)
            nonCombatEngine->addStrategy("arathi");

        if (bgType == BATTLEGROUND_EY)
            nonCombatEngine->addStrategy("eye");

        if (bgType == BATTLEGROUND_IC)
            nonCombatEngine->addStrategy("isle");

        if (player->InArena())
        {
            nonCombatEngine->addStrategy("arena");
            nonCombatEngine->removeStrategy("mount");
        }
    }
}

Engine* AiFactory::createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext)
{
	Engine* nonCombatEngine = new Engine(facade, aiObjectContext);

    AddDefaultNonCombatStrategies(player, facade, nonCombatEngine);
	return nonCombatEngine;
}

void AiFactory::AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine)
{
    deadEngine->addStrategies("dead", "stay", "chat", "default", "follow", nullptr);

    if (sRandomPlayerbotMgr->IsRandomBot(player) && !player->GetGroup())
    {
        deadEngine->removeStrategy("follow");
    }
}

Engine* AiFactory::createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext)
{
    Engine* deadEngine = new Engine(facade, AiObjectContext);
    AddDefaultDeadStrategies(player, facade, deadEngine);
    return deadEngine;
}
