/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DebugAction.h"

#include "ChooseTravelTargetAction.h"
#include "MapMgr.h"
#include "Playerbots.h"

bool DebugAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const text = event.getParam();

    if (text == "scan")
    {
        sPlayerbotAIConfig->openLog("scan.csv", "w");

        uint32 i = 0;
        for (auto p : WorldPosition().getCreaturesNear())
        {
            WorldPosition pos(p->mapid, p->posX, p->posY, p->posZ, p->orientation);

            uint32 areaId = 0;
            uint32 zoneId = 0;
            sMapMgr->GetZoneAndAreaId(PHASEMASK_NORMAL, zoneId, areaId, pos.getMapId(), pos.getX(), pos.getY(),
                                      pos.getZ());

            std::ostringstream out;
            out << zoneId << "," << areaId << "," << (pos.getAreaName().empty() ? "none" : pos.getAreaName()) << ",";

            pos.printWKT(out);

            sPlayerbotAIConfig->log("scan.csv", out.str().c_str());

            if (zoneId == 0 && areaId == 0)
            {
                sPlayerbotAIConfig->log("x", out.str().c_str());
            }
            else
            {
                sPlayerbotAIConfig->log("y", out.str().c_str());
            }

            i = zoneId;
        }
        return i == 0;
    }
    else if (text.find("printmap") != std::string::npos)
    {
        sTravelNodeMap->printMap();
        sTravelNodeMap->printNodeStore();
        return true;
    }
    else if (text.find("travel ") != std::string::npos)
    {
        WorldPosition botPos = WorldPosition(bot);

        std::string const destination = text.substr(7);

        TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, destination);
        if (dest)
        {
            std::vector<WorldPosition*> points = dest->nextPoint(&botPos, true);

            if (points.empty())
                return false;

            std::vector<WorldPosition> beginPath, endPath;
            TravelNodeRoute route = sTravelNodeMap->getRoute(botPos, *points.front(), beginPath, bot);

            std::ostringstream out;
            out << "Traveling to " << dest->getTitle() << ": ";

            for (auto node : route.getNodes())
            {
                out << node->getName() << ", ";
            }

            botAI->TellMasterNoFacing(out.str());

            return true;
        }
        else
        {
            botAI->TellMasterNoFacing("Destination " + destination + " not found.");
            return true;
        }
    }
    else if (text.find("quest ") != std::string::npos)
    {
        uint32 questId = stoi(text.substr(6));

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);

        if (!quest)
        {
            botAI->TellMasterNoFacing("Quest " + text.substr(6) + " not found.");
            return false;
        }

        std::ostringstream out;

        out << quest->GetTitle() << ": ";

        QuestContainer* cont = sTravelMgr->quests[questId];

        for (auto g : cont->questGivers)
        {
            out << g->getTitle() << " +" << cont->questGivers.size() << " ";
            break;
        }

        for (auto g : cont->questTakers)
        {
            out << g->getTitle() << " +" << cont->questTakers.size() - 1;
            break;
        }

        for (auto g : cont->questObjectives)
        {
            out << g->getTitle() << " +" << cont->questObjectives.size() - 1;
            break;
        }

        botAI->TellMasterNoFacing(out);

        return true;
    }
    else if (text.find("quest") != std::string::npos)
    {
        std::ostringstream out;
        out << sTravelMgr->quests.size() << " quests ";

        uint32 noT = 0, noG = 0, noO = 0;

        for (auto q : sTravelMgr->quests)
        {
            if (q.second->questGivers.empty())
                noG++;

            if (q.second->questTakers.empty())
                noT++;

            if (q.second->questObjectives.empty())
                noO++;
        }

        out << noG << "|" << noT << "|" << noO << " bad.";

        botAI->TellMasterNoFacing(out);

        return true;
    }
    else if (text.find("bquest") != std::string::npos)
    {
        std::ostringstream out;
        out << "bad quests:";

        uint32 noT = 0, noG = 0, noO = 0;

        for (auto q : sTravelMgr->quests)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(q.first);

            if (!quest)
            {
                out << " " << q.first << " does not exists";
                continue;
            }

            if (q.second->questGivers.empty() || q.second->questTakers.empty() || q.second->questObjectives.empty())
                out << quest->GetTitle() << " ";

            if (q.second->questGivers.empty())
                out << " no G";

            if (q.second->questTakers.empty())
                out << " no T";

            if (q.second->questObjectives.empty())
                out << " no O";
        }
        botAI->TellMasterNoFacing(out);
    }
    else if (text.find("add node") != std::string::npos)
    {
        WorldPosition pos(bot);

        std::string const name = "USER:" + text.substr(9);

        TravelNode* startNode = sTravelNodeMap->addNode(pos, name, false, false);

        for (auto& endNode : sTravelNodeMap->getNodes(pos, 2000))
        {
            endNode->setLinked(false);
        }

        botAI->TellMasterNoFacing("Node " + name + " created.");

        sTravelNodeMap->setHasToGen();

        return true;
    }
    else if (text.find("rem node") != std::string::npos)
    {
        WorldPosition pos(bot);

        TravelNode* startNode = sTravelNodeMap->getNode(pos, nullptr, 50);

        if (!startNode)
            return false;

        if (startNode->isImportant())
        {
            botAI->TellMasterNoFacing("Node can not be removed.");
        }

        sTravelNodeMap->m_nMapMtx.lock();
        sTravelNodeMap->removeNode(startNode);
        botAI->TellMasterNoFacing("Node removed.");
        sTravelNodeMap->m_nMapMtx.unlock();

        sTravelNodeMap->setHasToGen();

        return true;
    }
    else if (text.find("reset node") != std::string::npos)
    {
        for (auto& node : sTravelNodeMap->getNodes())
            node->setLinked(false);
        return true;
    }
    else if (text.find("reset path") != std::string::npos)
    {
        for (auto& node : sTravelNodeMap->getNodes())
            for (auto& path : *node->getLinks())
                node->removeLinkTo(path.first, true);
        return true;
    }
    else if (text.find("gen node") != std::string::npos)
    {
        // Pathfinder
        sTravelNodeMap->generateNodes();
        return true;
    }
    else if (text.find("gen path") != std::string::npos)
    {
        sTravelNodeMap->generatePaths();
        return true;
    }
    else if (text.find("crop path") != std::string::npos)
    {
        sTravelNodeMap->removeUselessPaths();
        return true;
    }
    else if (text.find("save node") != std::string::npos)
    {
        sTravelNodeMap->printNodeStore();
        sTravelNodeMap->saveNodeStore();
        return true;
    }
    else if (text.find("load node") != std::string::npos)
    {
        std::thread t(
            []
            {
                sTravelNodeMap->removeNodes();
                sTravelNodeMap->loadNodeStore();
            });

        t.detach();

        return true;
    }
    else if (text.find("show node") != std::string::npos)
    {
        WorldPosition pos(bot);

        std::vector<TravelNode*> nodes = sTravelNodeMap->getNodes(pos, 500);

        for (auto& node : nodes)
        {
            for (auto& l : *node->getLinks())
            {
                Unit* start = nullptr;
                GuidVector units;

                uint32 time = 60 * IN_MILLISECONDS;

                std::vector<WorldPosition> ppath = l.second->getPath();

                for (auto p : ppath)
                {
                    Creature* wpCreature =
                        bot->SummonCreature(1, p.getX(), p.getY(), p.getZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 20000.0f);
                    // addAura(246, wpCreature);
                    units.push_back(wpCreature->GetGUID());

                    if (!start)
                        start = wpCreature;
                }

                // FakeSpell(1064, bot, start, units.front(), units, {}, pos, pos);
            }
        }
        return true;
    }
    else if (text.find("dspell ") != std::string::npos)
    {
        uint32 spellEffect = stoi(text.substr(7));

        Unit* prev = bot;

        for (float i = 0; i < 60; i++)
        {
            float ang = i / 60 * static_cast<float>(M_PI) * 4;
            float dist = i / 60 * 30;

            WorldPosition botPos(bot);
            WorldPosition botPos1 = botPos;

            botPos.setX(botPos.getX() + cos(ang) * dist);
            botPos.setY(botPos.getY() + sin(ang) * dist);
            botPos.setZ(botPos.getHeight() + 2);

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                       TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

            FakeSpell(spellEffect, wpCreature, wpCreature, prev->GetGUID(), {}, {}, botPos, botPos);

            prev = wpCreature;
        }
        return true;
    }
    else if (text.find("vspell ") != std::string::npos)
    {
        uint32 spellEffect = stoi(text.substr(7));

        Unit* prev = bot;

        for (float i = 0; i < 60; i++)
        {
            float ang = i / 60 * static_cast<float>(M_PI) * 4;
            float dist = i / 60 * 30;

            WorldPosition botPos(bot);
            WorldPosition botPos1 = botPos;

            botPos.setX(botPos.getX() + cos(ang) * dist);
            botPos.setY(botPos.getY() + sin(ang) * dist);
            botPos.setZ(botPos.getHeight() + 2);

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                       TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4);  // visual effect on guid
                data << wpCreature->GetGUID();
                data << uint32(spellEffect);  // index from SpellVisualKit.dbc
                wpCreature->SendMessageToSet(&data, true);
            }
        }
        return true;
    }
    else if (text.find("aspell ") != std::string::npos)
    {
        uint32 spellEffect = stoi(text.substr(7));

        Unit* prev = bot;

        for (float i = 0; i < 60; i++)
        {
            float ang = i / 60 * static_cast<float>(M_PI) * 4;
            float dist = i / 60 * 30;

            WorldPosition botPos(bot);
            WorldPosition botPos1 = botPos;

            botPos.setX(botPos.getX() + cos(ang) * dist);
            botPos.setY(botPos.getY() + sin(ang) * dist);
            botPos.setZ(botPos.getHeight() + 2);

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                       TEMPSUMMON_TIMED_DESPAWN, 5000.0f + i * 100.0f);
            wpCreature->SetObjectScale(0.5f);

            if (wpCreature)
            {
                addAura(spellEffect, wpCreature);
            }
        }
        return true;
    }
    else if (text.find("cspell ") != std::string::npos)
    {
        uint32 spellEffect = stoi(text.substr(7));

        GuidVector units;

        for (float i = 0; i < 60; i++)
        {
            float ang = i / 60 * static_cast<float>(M_PI) * 4;
            float dist = i / 60 * 30;

            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + cos(ang) * dist);
            botPos.setY(botPos.getY() + sin(ang) * dist);
            botPos.setZ(botPos.getHeight() + 2);

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                       TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

            units.push_back(wpCreature->GetGUID());
        }

        WorldPosition botPos(bot);

        if (urand(0, 1))
            FakeSpell(spellEffect, bot, bot, ObjectGuid::Empty, units, {}, botPos, botPos);
        else
            FakeSpell(spellEffect, bot, bot, units.front(), units, {}, botPos, botPos);

        return true;
    }
    else if (text.find("fspell ") != std::string::npos)
    {
        uint32 spellEffect = stoi(text.substr(7));

        {
            WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));

            data.Initialize(SMSG_SPELL_START);
            data << bot->GetPackGUID();
            data << bot->GetPackGUID();
            data << uint32(spellEffect);
            data << uint16(0);
            data << uint32(0);
            data << uint16(2);
            data << master->GetGUID();
            bot->SendMessageToSet(&data, true);
        }

        {
            WorldPacket data(SMSG_SPELL_GO, 53);  // guess size
            data << bot->GetPackGUID();
            data << bot->GetPackGUID();
            data << uint32(spellEffect);   // spellID
            data << uint8(0) << uint8(1);  // flags
            data << uint8(1);              // amount of targets
            data << master->GetGUID();
            data << uint8(0);
            data << uint16(2);
            data << master->GetGUID();
            bot->SendMessageToSet(&data, true);
        }

        return true;
    }
    else if (text.find("spell ") != std::string::npos)
    {
        uint32 spellEffect = stoi(text.substr(6));
        master->SendPlaySpellVisual(bot->GetGUID(), spellEffect);
        return true;
    }
    else if (text.find("tspellmap") != std::string::npos)
    {
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature =
                    bot->SummonCreature((dy == 0 && (dx == 0 || dx == 2)) ? 6 : 2, botPos.getX(), botPos.getY(),
                                        botPos.getZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                if (wpCreature)
                {
                    std::ostringstream out;
                    out << "effect ";
                    out << effect;

                    const std::string& Cname = out.str();

                    wpCreature->Say(Cname.c_str(), LANG_UNIVERSAL, master);
                }
            }
        }
        return true;
    }
    else if (text.find("uspellmap") != std::string::npos)
    {
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature = bot->SummonCreature(effect, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                           TEMPSUMMON_TIMED_DESPAWN, 10000.0f);
            }
        }
        return true;
    }
    else if (text.find("dspellmap") != std::string::npos)
    {
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                FakeSpell(effect, bot, nullptr, ObjectGuid::Empty, {}, {}, botPos, botPos, true);
            }
        }
        return true;
    }
    else if (text.find("vspellmap") != std::string::npos)
    {
        std::vector<WorldPacket> datMap;
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                           TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                if (wpCreature)
                {
                    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4);  // visual effect on guid
                    data << wpCreature->GetGUID();
                    data << uint32(effect);
                    ;  // index from SpellVisualKit.dbc
                    // wpCreature->SendMessageToSet(&data, true);
                    datMap.push_back(data);

                    // wpCreature->MonsterMoveWithSpeed(botPos.getX(), botPos.getY() + 80, botPos.getZ(), 8.0f, true,
                    // true);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (auto dat : datMap)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            bot->SendMessageToSet(&dat, true);
        }
        for (auto dat : datMap)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            bot->SendMessageToSet(&dat, true);
        }

        return true;
    }
    else if (text.find("ispellmap") != std::string::npos)
    {
        std::vector<WorldPacket> datMap;
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature =
                    bot->SummonCreature((dy == 0 && (dx == 0 || dx == 2)) ? 6 : 2, botPos.getX(), botPos.getY(),
                                        botPos.getZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                if (wpCreature)
                {
                    WorldPacket data(SMSG_PLAY_SPELL_IMPACT, 8 + 4);  // visual effect on player
                    data << wpCreature->GetGUID();
                    data << uint32(effect);  // index from SpellVisualKit.dbc
                    // wpCreature->SendMessageToSet(&data, true);
                    datMap.push_back(data);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (auto dat : datMap)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            bot->SendMessageToSet(&dat, true);
        }
        for (auto dat : datMap)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            bot->SendMessageToSet(&dat, true);
        }

        return true;
    }
    else if (text.find("cspellmap") != std::string::npos)
    {
        Creature* wpCreature = nullptr;
        Creature* lCreature = nullptr;
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                wpCreature = bot->SummonCreature((dy == 0 && (dx == 0 || dx == 2)) ? 6 : 2, botPos.getX(),
                                                 botPos.getY(), botPos.getZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                if (wpCreature && lCreature)
                {
                    wpCreature->CastSpell(lCreature, effect, TRIGGERED_FULL_MASK);
                }

                lCreature = wpCreature;
            }
        }
        return true;
    }
    else if (text.find("aspellmap") != std::string::npos)
    {
        Creature* wpCreature = nullptr;

        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                 TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                if (wpCreature)
                {
                    addAura(effect, wpCreature);
                }
            }
        }
        return true;
    }
    else if (text.find("gspellmap") != std::string::npos)
    {
        GuidVector all_targets;  // = { bot->GetGUID(), master->GetGUID() };
        // std::GuidVector all_dummies = { bot->GetGUID(), master->GetGUID() };

        /*GuidVector a_targets = *context->GetValue<GuidVector >("all targets");
        for (auto t : a_targets)
        {
            all_targets.push_back(t);
        }
        */

        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                           TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                all_targets.push_back(wpCreature->GetGUID());
            }
        }

        all_targets.push_back(master->GetGUID());
        all_targets.push_back(bot->GetGUID());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (uint32 i = 0; i < 2; i++)
            for (int32 dx = 0; dx < 10; dx++)
            {
                for (int32 dy = 0; dy < 10; dy++)
                {
                    uint32 spellEffect = stoi(text.substr(10));
                    uint32 effect = dx + dy * 10 + spellEffect * 100;

                    uint32 i = dx + dy * 10;
                    GuidVector hits, miss;

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(effect);
                    if (spellInfo && spellInfo->Speed > 0)
                        for (auto tar : all_targets)
                        {
                            // hits.push_back(tar);

                            switch (urand(0, 10))
                            {
                                case 0:
                                    hits.push_back(tar);
                                    break;
                                case 1:
                                    miss.push_back(tar);
                                    break;
                                case 2:
                                case 3:
                                    break;
                            }
                        }

                    Unit* realCaster = botAI->GetUnit(
                        all_targets[i]);  // botAI->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                    Unit* caster = botAI->GetUnit(
                        all_targets[i]);  // botAI->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                    Unit* target = botAI->GetUnit(all_targets[i + 1]);

                    if (!realCaster)
                        realCaster = bot;

                    if (!caster)
                        caster = bot;

                    if (!target)
                        target = master;

                    FakeSpell(effect, realCaster, caster, target->GetGUID(), hits, miss, WorldPosition(caster),
                              WorldPosition(target));

                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }
        return true;
    }
    else if (text.find("mspellmap") != std::string::npos)
    {
        GuidVector all_targets;

        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0,
                                                           TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                all_targets.push_back(wpCreature->GetGUID());
            }
        }

        all_targets.push_back(master->GetGUID());
        all_targets.push_back(bot->GetGUID());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (uint32 i = 0; i < 2; i++)
            for (int32 dx = 0; dx < 10; dx++)
            {
                for (int32 dy = 0; dy < 10; dy++)
                {
                    uint32 spellEffect = stoi(text.substr(10));
                    uint32 effect = dx + dy * 10 + spellEffect * 100;

                    uint32 i = dx + dy * 10;
                    GuidVector hits, miss;

                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(effect);
                    for (auto tar : all_targets)
                    {
                        // hits.push_back(tar);
                        switch (urand(0, 10))
                        {
                            case 0:
                                hits.push_back(tar);
                                break;
                            case 1:
                                miss.push_back(tar);
                                break;
                            case 2:
                            case 3:
                                break;
                        }
                    }

                    Unit* realCaster = botAI->GetUnit(
                        all_targets[i]);  // botAI->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                    Unit* caster = botAI->GetUnit(
                        all_targets[i]);  // botAI->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                    Unit* target = botAI->GetUnit(all_targets[i + 1]);

                    if (!realCaster)
                        realCaster = bot;

                    if (!caster)
                        caster = bot;

                    if (!target)
                        target = master;

                    master->SendPlaySpellVisual(caster->GetGUID(), 5036);
                    FakeSpell(effect, realCaster, caster, target->GetGUID(), hits, miss, WorldPosition(caster),
                              WorldPosition(target));

                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }

        return true;
    }
    else if (text.find("soundmap") != std::string::npos)
    {
        uint32 soundEffects = stoi(text.substr(9));
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 effect = dx + dy * 10 + soundEffects * 100;
                WorldPosition botPos(bot);

                botPos.setX(botPos.getX() + (dx - 5) * 5);
                botPos.setY(botPos.getY() + (dy - 5) * 5);
                botPos.setZ(botPos.getHeight());

                Creature* wpCreature =
                    bot->SummonCreature((dy == 0 && (dx == 0 || dx == 2)) ? 6 : 2, botPos.getX(), botPos.getY(),
                                        botPos.getZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 10000.0f);

                wpCreature->PlayDistanceSound(effect);
            }
        }
    }
    else if (text.find("sounds") != std::string::npos)
    {
        uint32 soundEffects = stoi(text.substr(6));

        for (uint32 i = 0; i < 100; i++)
        {
            bot->PlayDistanceSound(i + soundEffects * 100);
            bot->Say(std::to_string(i + soundEffects * 100), LANG_UNIVERSAL);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return true;
    }
    else if (text.find("dsound") != std::string::npos)
    {
        uint32 soundEffect = stoi(text.substr(7));
        bot->PlayDirectSound(soundEffect);
        return true;
    }
    else if (text.find("dsound") != std::string::npos)
    {
        uint32 soundEffect = stoi(text.substr(7));
        bot->PlayDirectSound(soundEffect);
        return true;
    }
    else if (text.find("sound") != std::string::npos)
    {
        uint32 soundEffect = stoi(text.substr(6));
        bot->PlayDistanceSound(soundEffect);
        return true;
    }

    std::string const response = botAI->HandleRemoteCommand(text);
    botAI->TellMaster(response);
    return true;
}

void DebugAction::FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target, GuidVector otherTargets,
                            GuidVector missTargets, WorldPosition source, WorldPosition dest, bool forceDest)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    {
        uint32 castFlags = CAST_FLAG_HAS_TRAJECTORY;

        if (spellInfo->HasAttribute(SPELL_ATTR0_USES_RANGED_SLOT) ||
            spellInfo->HasAttribute(SPELL_ATTR0_CU_NEEDS_AMMO_DATA))
            castFlags |= CAST_FLAG_PROJECTILE;

        WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));

        data << truecaster->GetPackGUID();  // truecaster

        if (caster)
            data << caster->GetPackGUID();  // m_caster->GetPackGUID();
        else
            data << ObjectGuid::Empty;

        data << uint32(spellId);    // spellId
        data << uint16(castFlags);  // cast flags
        data << uint32(1000.0f);    // delay?

        SpellCastTargets m_targets;

        m_targets.Write(data);

        // projectile info
        if (castFlags & CAST_FLAG_PROJECTILE)
        {
            data << uint32(5996);
            data << uint32(INVTYPE_AMMO);
        }

        if (caster)
            caster->SendMessageToSet(&data, true);
        else
            truecaster->SendMessageToSet(&data, true);
    }

    {
        SpellCastTargets m_targets;

        if ((spellInfo && spellInfo->Targets & TARGET_FLAG_DEST_LOCATION) || forceDest)
            m_targets.SetDst(dest);

        if ((spellInfo && spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION) || forceDest)
            m_targets.SetSrc(source.getX(), source.getY(), source.getZ());

        if (!forceDest && target)
            if (!spellInfo ||
                !(spellInfo->Targets & TARGET_FLAG_DEST_LOCATION && spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION))
                m_targets.SetUnitTarget(botAI->GetUnit(target));

        uint32 castFlags = CAST_FLAG_UNKNOWN_9;

        if (spellInfo->HasAttribute(SPELL_ATTR0_USES_RANGED_SLOT) ||
            spellInfo->HasAttribute(SPELL_ATTR0_CU_NEEDS_AMMO_DATA))
            castFlags |= CAST_FLAG_PROJECTILE;  // arrows/bullets visual

        if (spellInfo->HasEffect(SPELL_EFFECT_ACTIVATE_RUNE))
            castFlags |= CAST_FLAG_RUNE_LIST;  // rune cooldowns list

        if (m_targets.HasTraj())
            castFlags |= CAST_FLAG_ADJUST_MISSILE;

        if (!spellInfo->StartRecoveryTime)
            castFlags |= CAST_FLAG_NO_GCD;

        WorldPacket data(SMSG_SPELL_GO, 53);  // guess size

        data << truecaster->GetPackGUID();  // truecaster

        if (caster)
            data << caster->GetPackGUID();  // m_caster->GetPackGUID();
        else
            data << ObjectGuid::Empty;

        data << uint32(spellId);    // spellId
        data << uint16(castFlags);  // cast flags

        // WriteSpellGoTargets
        uint32 hits = otherTargets.size() + (target ? 1 : 0);

        data << uint8(hits);  // Hits

        if (target)
            data << target;

        // Hit targets here.
        for (auto otherTarget : otherTargets)
            data << otherTarget;

        data << (uint8)missTargets.size();  // miss

        for (auto missTarget : missTargets)
        {
            data << missTarget;

            data << uint8(SPELL_MISS_RESIST);  // Miss condition
            data << uint8(SPELL_MISS_NONE);    // Miss condition
        }

        m_targets.Write(data);

        // projectile info
        if (castFlags & CAST_FLAG_PROJECTILE)
        {
            data << uint32(5996);
            data << uint32(INVTYPE_AMMO);
        }

        if (caster)
            caster->SendMessageToSet(&data, true);
        else
            truecaster->SendMessageToSet(&data, true);
    }
}

void DebugAction::addAura(uint32 spellId, Unit* target)
{
    if (!target)
        return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return;

    if (!target->IsAlive() && !spellInfo->HasAttribute(SPELL_ATTR0_PASSIVE) &&
        !spellInfo->HasAttribute(SPELL_ATTR2_ALLOW_DEAD_TARGET))
        return;

    target->AddAura(spellInfo, MAX_EFFECT_MASK, target);
}
