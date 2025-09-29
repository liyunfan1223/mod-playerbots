/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AiFactory.h"

#include "BattlegroundMgr.h"
#include "DKAiObjectContext.h"
#include "DruidAiObjectContext.h"
#include "Engine.h"
#include "Group.h"
#include "HunterAiObjectContext.h"
#include "Item.h"
#include "MageAiObjectContext.h"
#include "PaladinAiObjectContext.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PriestAiObjectContext.h"
#include "RogueAiObjectContext.h"
#include "ShamanAiObjectContext.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "WarlockAiObjectContext.h"
#include "WarriorAiObjectContext.h"

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

    if (bot->GetLevel() >= 10 && ((tabs[0] + tabs[1] + tabs[2]) > 0))
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
                tab = MAGE_TAB_FROST;
                break;
            case CLASS_PALADIN:
                tab = PALADIN_TAB_RETRIBUTION;
                break;
            case CLASS_PRIEST:
                tab = PRIEST_TAB_HOLY;
                break;
            case CLASS_WARLOCK:
                tab = WARLOCK_TAB_DEMONOLOGY;
                break;
            case CLASS_SHAMAN:
                tab = SHAMAN_TAB_ELEMENTAL;
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
        if ((bot->GetActiveSpecMask() & i->second->specMask) == 0)
        {
            continue;
        }
        TalentSpellPos const* talentPos = GetTalentSpellPos(spellId);
        if (!talentPos)
            continue;
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id);
        if (!talentInfo)
            continue;

        uint32 const* talentTabIds = GetTalentTabPages(bot->getClass());

        const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        int rank = spellInfo ? spellInfo->GetRank() : 1;
        if (talentInfo->TalentTab == talentTabIds[0])
            tabs[0] += rank;
        if (talentInfo->TalentTab == talentTabIds[1])
            tabs[1] += rank;
        if (talentInfo->TalentTab == talentTabIds[2])
            tabs[2] += rank;
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
            ;
            break;
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
        engine->addStrategiesNoInit("racials", "chat", "default", "cast time", "potions", "duel", "boost", nullptr);
    }
    if (sPlayerbotAIConfig->autoAvoidAoe && facade->HasRealPlayerMaster())
    {
        engine->addStrategy("avoid aoe", false);
    }
    engine->addStrategy("formation", false);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == 2)
            {
                engine->addStrategiesNoInit("dps", "shadow debuff", "shadow aoe", nullptr);
            }
            else if (tab == PRIEST_TAB_DISCIPLINE)
            {
                engine->addStrategiesNoInit("heal", nullptr);
            }
            else
            {
                engine->addStrategiesNoInit("holy heal", nullptr);
            }

            engine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_MAGE:
            if (tab == 0) // Arcane
                engine->addStrategiesNoInit("arcane", nullptr);
            else if (tab == 1)  // Fire
            {
                if (player->HasSpell(44614) /*Frostfire Bolt*/ && player->HasAura(15047) /*Ice Shards*/)
                {
                    engine->addStrategiesNoInit("frostfire", nullptr);
                }
                else
                {
                    engine->addStrategiesNoInit("fire", nullptr);
                }
            }
            else  // Frost
                engine->addStrategiesNoInit("frost", nullptr);

            engine->addStrategiesNoInit("dps", "dps assist", "cure", "aoe", nullptr);
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                engine->addStrategiesNoInit("tank", "tank assist", "aoe", nullptr);
            else if (tab == 0 || !player->HasSpell(1680)) // Whirlwind
                engine->addStrategiesNoInit("arms", "aoe", "dps assist", /*"behind",*/ nullptr);
            else
                engine->addStrategiesNoInit("fury", "aoe", "dps assist", /*"behind",*/ nullptr);
            break;
        case CLASS_SHAMAN:
            if (tab == 0)  // Elemental
                engine->addStrategiesNoInit("ele", "stoneskin", "wrath", "mana spring", "wrath of air", nullptr);
            else if (tab == 2)  // Restoration
                engine->addStrategiesNoInit("resto", "stoneskin", "flametongue", "mana spring", "wrath of air", nullptr);
            else  // Enhancement
                engine->addStrategiesNoInit("enh", "strength of earth", "magma", "healing stream", "windfury", nullptr);

            engine->addStrategiesNoInit("dps assist", "cure", "aoe", nullptr);
            break;
        case CLASS_PALADIN:
            if (tab == 1)
                engine->addStrategiesNoInit("tank", "tank assist", "bthreat", "barmor", "cure", nullptr);
            else if (tab == 0)
                engine->addStrategiesNoInit("heal", "dps assist", "cure", "bcast", nullptr);
            else
                engine->addStrategiesNoInit("dps", "dps assist", "cure", "baoe", nullptr);

            break;
        case CLASS_DRUID:
            if (tab == 0)
            {
                engine->addStrategiesNoInit("caster", "cure", "caster aoe", "dps assist", nullptr);
                engine->addStrategy("caster debuff", false);
            }
            else if (tab == 2)
                engine->addStrategiesNoInit("heal", "cure", "dps assist", nullptr);
            else
            {
                if (player->HasSpell(768) /*cat form*/&& !player->HasAura(16931) /*thick hide*/)
                {
                    engine->addStrategiesNoInit("cat", "dps assist", nullptr);
                }
                else
                {
                    engine->addStrategiesNoInit("bear", "tank assist", nullptr);
                }
            }
            break;
        case CLASS_HUNTER:
            if (tab == 0)  // Beast Mastery
                engine->addStrategiesNoInit("bm", nullptr);
            else if (tab == 1)  // Marksmanship
                engine->addStrategiesNoInit("mm", nullptr);
            else if (tab == 2)  // Survival
                engine->addStrategiesNoInit("surv", nullptr);

            engine->addStrategiesNoInit("cc", "dps assist", "aoe", nullptr);
            break;
        case CLASS_ROGUE:
            if (tab == ROGUE_TAB_ASSASSINATION || tab == ROGUE_TAB_SUBTLETY)
            {
                engine->addStrategiesNoInit("melee", "dps assist", "aoe", nullptr);
            }
            else
            {
                engine->addStrategiesNoInit("dps", "dps assist", "aoe", nullptr);
            }
            break;
        case CLASS_WARLOCK:
            if (tab == 0)  // Affliction
                engine->addStrategiesNoInit("affli", "curse of agony", nullptr);
            else if (tab == 1)  // Demonology
                engine->addStrategiesNoInit("demo", "curse of agony", "meta melee", nullptr);
            else if (tab == 2)  // Destruction
                engine->addStrategiesNoInit("destro", "curse of elements", nullptr);

            engine->addStrategiesNoInit("cc", "dps assist", "aoe", nullptr);
            break;

        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                engine->addStrategiesNoInit("blood", "tank assist", nullptr);
            else if (tab == 1)
                engine->addStrategiesNoInit("frost", "frost aoe", "dps assist", nullptr);
            else
                engine->addStrategiesNoInit("unholy", "unholy aoe", "dps assist", nullptr);

            break;
    }
    if (PlayerbotAI::IsTank(player, true)) {
        engine->addStrategy("tank face", false);
    }
    if (PlayerbotAI::IsMelee(player, true) && PlayerbotAI::IsDps(player, true)) {
        engine->addStrategy("behind", false);
    }
    if (PlayerbotAI::IsHeal(player, true))
    {
        if (sPlayerbotAIConfig->autoSaveMana)
            engine->addStrategy("save mana", false);
        if (!sPlayerbotAIConfig->IsRestrictedHealerDPSMap(player->GetMapId()))
            engine->addStrategy("healer dps", false);
    }
    if (facade->IsRealPlayer() || sRandomPlayerbotMgr->IsRandomBot(player))
    {
        if (!player->GetGroup())
        {
            // change for heal spec
            engine->addStrategy("boost", false);
            engine->addStrategy("dps assist", false);
            engine->removeStrategy("threat", false);
            // engine-
            switch (player->getClass())
            {
                case CLASS_PRIEST:
                {
                    if (tab != PRIEST_TAB_SHADOW)
                    {
                        engine->addStrategiesNoInit("holy dps", "shadow debuff", "shadow aoe", nullptr);
                    }
                    break;
                }
                case CLASS_DRUID:
                {
                    if (tab == DRUID_TAB_RESTORATION)
                    {
                        engine->addStrategiesNoInit("caster", "caster aoe", nullptr);
                        engine->addStrategy("caster debuff", false);
                    }
                    break;
                }
                case CLASS_SHAMAN:
                {
                    if (tab == SHAMAN_TAB_RESTORATION)
                    {
                        engine->addStrategiesNoInit("caster", "caster aoe", "bmana", nullptr);
                    }
                    break;
                }
                case CLASS_PALADIN:
                {
                    if (tab == PALADIN_TAB_HOLY)
                    {
                        engine->addStrategiesNoInit("dps", "dps assist", "baoe", nullptr);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    if (sRandomPlayerbotMgr->IsRandomBot(player))
    {
        engine->ChangeStrategy(sPlayerbotAIConfig->randomBotCombatStrategies);
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
            engine->addStrategy("warsong", false);

        if (bgType == BATTLEGROUND_AB)
            engine->addStrategy("arathi", false);

        if (bgType == BATTLEGROUND_AV)
            engine->addStrategy("alterac", false);

        if (bgType == BATTLEGROUND_EY)
            engine->addStrategy("eye", false);

        if (bgType == BATTLEGROUND_IC)
            engine->addStrategy("isle", false);

        if (player->InArena())
        {
            engine->addStrategy("arena", false);
        }

        engine->addStrategiesNoInit("boost", "racials", "chat", "default", "aoe", "potions", "cast time", "dps assist",
                              nullptr);
        engine->removeStrategy("custom::say", false);
        engine->removeStrategy("flee", false);
        engine->removeStrategy("threat", false);
        engine->addStrategy("boost", false);

        // if ((player->getClass() == CLASS_DRUID && tab == 2) || (player->getClass() == CLASS_SHAMAN && tab == 2))
        //     engine->addStrategiesNoInit("caster", "caster aoe", nullptr);

        // if (player->getClass() == CLASS_DRUID && tab == 1)
        //     engine->addStrategiesNoInit(/*"behind",*/ "dps", nullptr);

        // if (player->getClass() == CLASS_ROGUE)
        //     engine->addStrategiesNoInit(/*"behind",*/ "stealth", nullptr);
    }
}

Engine* AiFactory::createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext)
{
    Engine* engine = new Engine(facade, aiObjectContext);
    AddDefaultCombatStrategies(player, facade, engine);
    engine->Init();
    return engine;
}

void AiFactory::AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine)
{
    uint8 tab = GetPlayerSpecTab(player);

    switch (player->getClass())
    {
        case CLASS_PRIEST:
            nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_PALADIN:
            if (tab == 1)
            {
                nonCombatEngine->addStrategiesNoInit("bthreat", "tank assist", "barmor", nullptr);
                if (player->GetLevel() >= 20)
                {
                    nonCombatEngine->addStrategy("bhealth", false);
                }
                else
                {
                    nonCombatEngine->addStrategy("bdps", false);
                }
            }
            else if (tab == 0)
                nonCombatEngine->addStrategiesNoInit("dps assist", "bmana", "bcast", nullptr);
            else
                nonCombatEngine->addStrategiesNoInit("dps assist", "bdps", "baoe", nullptr);

            nonCombatEngine->addStrategiesNoInit("cure", nullptr);
            break;
        case CLASS_HUNTER:
            nonCombatEngine->addStrategiesNoInit("bdps", "dps assist", "pet", nullptr);
            break;
        case CLASS_SHAMAN:
            if (tab == 0 || tab == 2)
                nonCombatEngine->addStrategy("bmana", false);
            else
                nonCombatEngine->addStrategy("bdps", false);

            nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_MAGE:
            if (tab == MAGE_TAB_ARCANE || tab == MAGE_TAB_FIRE)
                nonCombatEngine->addStrategy("bdps", false);
            else
                nonCombatEngine->addStrategy("bmana", false);

            nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_DRUID:
            if (tab == 1)
            {
                if (player->GetLevel() >= 20 && !player->HasAura(16931) /*thick hide*/)
                {
                    nonCombatEngine->addStrategy("dps assist", false);
                }
                else
                {
                    nonCombatEngine->addStrategy("tank assist", false);
                }
            }
            else
                nonCombatEngine->addStrategiesNoInit("dps assist", "cure", nullptr);
            break;
        case CLASS_WARRIOR:
            if (tab == 2)
                nonCombatEngine->addStrategy("tank assist", false);
            else
                nonCombatEngine->addStrategy("dps assist", false);
            break;
        case CLASS_WARLOCK:
            if (tab == WARLOCK_TAB_AFFLICTION)
            {
                nonCombatEngine->addStrategiesNoInit("felhunter", "spellstone", nullptr);
            }
            else if (tab == WARLOCK_TAB_DEMONOLOGY)
            {
                nonCombatEngine->addStrategiesNoInit("felguard", "spellstone", nullptr);
            }
            else if (tab == WARLOCK_TAB_DESTRUCTION)
            {
                nonCombatEngine->addStrategiesNoInit("imp", "firestone", nullptr);
            }
            nonCombatEngine->addStrategiesNoInit("dps assist", "ss self", nullptr);
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab == 0)
                nonCombatEngine->addStrategy("tank assist", false);
            else
                nonCombatEngine->addStrategy("dps assist", false);
            break;
        default:
            nonCombatEngine->addStrategy("dps assist", false);
            break;
    }

    if (!player->InBattleground())
    {
        nonCombatEngine->addStrategiesNoInit("nc", "food", "chat", "follow", "default", "quest", "loot",
                                            "gather", "duel", "pvp", "buff", "mount", "emote", nullptr);
    }

    if (sPlayerbotAIConfig->autoSaveMana && PlayerbotAI::IsHeal(player, true))
    {
        nonCombatEngine->addStrategy("save mana", false);
    }

    if ((sRandomPlayerbotMgr->IsRandomBot(player)) && !player->InBattleground())
    {
        Player* master = facade->GetMaster();

        // let 25% of free bots start duels.
        if (!urand(0, 3))
            nonCombatEngine->addStrategy("start duel", false);

        if (sPlayerbotAIConfig->randomBotJoinLfg)
            nonCombatEngine->addStrategy("lfg", false);

        if (!player->GetGroup() || player->GetGroup()->GetLeaderGUID() == player->GetGUID())
        {
            // let 25% of random not grouped (or grp leader) bots help other players
            // if (!urand(0, 3))
            //     nonCombatEngine->addStrategy("attack tagged");

            // nonCombatEngine->addStrategy("pvp", false);
            // nonCombatEngine->addStrategy("collision");
            // nonCombatEngine->addStrategy("group");
            // nonCombatEngine->addStrategy("guild");
            nonCombatEngine->addStrategy("grind", false);

            if (sPlayerbotAIConfig->enableNewRpgStrategy)
            {
                nonCombatEngine->addStrategy("new rpg", false);
            }
            else if (sPlayerbotAIConfig->autoDoQuests)
            {
                // nonCombatEngine->addStrategy("travel");
                nonCombatEngine->addStrategy("rpg", false);
            }
            else
            {
                nonCombatEngine->addStrategy("move random", false);
            }

            if (sPlayerbotAIConfig->randomBotJoinBG)
                nonCombatEngine->addStrategy("bg", false);

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
                        // nonCombatEngine->addStrategy("pvp", false);
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
                    else
                    {
                        // nonCombatEngine->addStrategy("pvp", false);
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
        nonCombatEngine->addStrategiesNoInit("nc", "chat", "default", "buff", "food", "mount", "pvp", "dps assist",
                                       "attack tagged", "emote", nullptr);
        nonCombatEngine->removeStrategy("custom::say", false);
        nonCombatEngine->removeStrategy("travel", false);
        nonCombatEngine->removeStrategy("rpg", false);
        nonCombatEngine->removeStrategy("grind", false);

        BattlegroundTypeId bgType = player->GetBattlegroundTypeId();
        if (bgType == BATTLEGROUND_RB)
            bgType = player->GetBattleground()->GetBgTypeID(true);

        if ((bgType <= BATTLEGROUND_EY || bgType == BATTLEGROUND_IC) &&
            !player->InArena())  // do not add for not supported bg or arena
            nonCombatEngine->addStrategy("battleground", false);

        if (bgType == BATTLEGROUND_WS)
            nonCombatEngine->addStrategy("warsong", false);

        if (bgType == BATTLEGROUND_AV)
            nonCombatEngine->addStrategy("alterac", false);

        if (bgType == BATTLEGROUND_AB)
            nonCombatEngine->addStrategy("arathi", false);

        if (bgType == BATTLEGROUND_EY)
            nonCombatEngine->addStrategy("eye", false);

        if (bgType == BATTLEGROUND_IC)
            nonCombatEngine->addStrategy("isle", false);

        if (player->InArena())
        {
            nonCombatEngine->addStrategy("arena", false);
            nonCombatEngine->removeStrategy("mount", false);
        }
    }
}

Engine* AiFactory::createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* aiObjectContext)
{
    Engine* nonCombatEngine = new Engine(facade, aiObjectContext);

    AddDefaultNonCombatStrategies(player, facade, nonCombatEngine);
    nonCombatEngine->Init();
    return nonCombatEngine;
}

void AiFactory::AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine)
{
    (void)facade;  // unused and remove warning
    deadEngine->addStrategiesNoInit("dead", "stay", "chat", "default", "follow", nullptr);

    if (sRandomPlayerbotMgr->IsRandomBot(player) && !player->GetGroup())
    {
        deadEngine->removeStrategy("follow", false);
    }
}

Engine* AiFactory::createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext)
{
    Engine* deadEngine = new Engine(facade, AiObjectContext);
    AddDefaultDeadStrategies(player, facade, deadEngine);
    deadEngine->Init();
    return deadEngine;
}
