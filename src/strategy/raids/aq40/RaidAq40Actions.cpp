#include "RaidAq40Actions.h"

#include "Playerbots.h"
#include "AttackAction.h"

bool Aq40UseResistanceBuffsAction::Execute(Event event)
{
    switch(bot->getClass())
    {
        case CLASS_HUNTER:
            {
                bool isnatureboss = false;
                
                Unit* boss;

                if ((boss = AI_VALUE2(Unit*, "find target", "viscidus")) && boss->IsInCombat())
                {
                    isnatureboss = true;
                }
                else if ((boss = AI_VALUE2(Unit*, "find target", "princess huhuran")) && boss->IsInCombat())
                {
                    isnatureboss = true;
                }

                if (isnatureboss)
                {
                    if (!botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT))
                    {
                        botAI->ChangeStrategy("+rnature", BOT_STATE_NON_COMBAT);
                        botAI->ChangeStrategy("+rnature", BOT_STATE_COMBAT);
                        return true;
                    }
                }
                else if (botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT))
                {
                    botAI->ChangeStrategy("-rnature", BOT_STATE_NON_COMBAT);
                    botAI->ChangeStrategy("-rnature", BOT_STATE_COMBAT);
                    return true;
                }
            }
            break;
        case CLASS_PRIEST:
            {
                // paladin aura seems like a waste when priests have buffs that don't have a radius limitation
                if (!botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT))
                {
                    botAI->ChangeStrategy("+rshadow", BOT_STATE_NON_COMBAT);
                    botAI->ChangeStrategy("+rshadow", BOT_STATE_COMBAT);
                    return true;
                }
            }
            break;
    }
        

    return false;
}



// 88072: The Master's Eye for positioning maybe

bool Aq40MoveFromOtherEmperorAction::Execute(Event event)
{
    const float radius = 120.0f;  // emperors' heal range is 60

    if (Unit* boss1 = AI_VALUE2(Unit*, "find target", "emperor vek'lor"))
    if (Unit* boss2 = AI_VALUE2(Unit*, "find target", "emperor vek'nilash"))
    {
        ObjectGuid botguid = bot->GetGUID();
        ObjectGuid petguid = (ObjectGuid)(uint64)0UL;
        if (Pet* pet = bot->GetPet())
        {
            petguid = pet->GetGUID();
        }
        Unit* moveAwayFrom = NULL;

        if (boss1->GetTarget() == botguid || boss1->GetTarget() == petguid)
        {
            moveAwayFrom = boss2;
            if (petguid)
            {
                botAI->PetFollow();
            }
        }
        else if (boss2->GetTarget() == botguid || boss2->GetTarget() == petguid)
        {
            moveAwayFrom = boss1;
            if (petguid)
            {
                botAI->PetFollow();
            }
        }

        if (moveAwayFrom != NULL)
        {
            long distToTravel = radius - bot->GetDistance(moveAwayFrom);

            if (distToTravel > 0)
            {
                return MoveAway(moveAwayFrom, distToTravel); 
            }
        }
    }

    return false;
}

bool Aq40MeleeViscidusAction::Execute(Event event)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "viscidus"))
    {
        if (!bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
        {
            // this might not work properly, didn't have problems with boss anyway
            ObjectGuid guid = boss->GetGUID();

            botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
            bool result = Attack(boss);
            if (result)
            {
                bot->AddUnitState(UNIT_STATE_MELEE_ATTACKING);
                context->GetValue<ObjectGuid>("pull target")->Set(guid);
            }

            return result;
        }
    }

    return false;
}

bool Aq40AttackTargetByNameAction::Execute(Event event)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", WhichEmperor()))
    {
        if (bot->GetTarget() != boss->GetGUID())
        {
            ObjectGuid guid = boss->GetGUID();

            botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
            bool result = Attack(boss);
            if (result)
                context->GetValue<ObjectGuid>("pull target")->Set(guid);

            return result;
        }
    }

    return false;
}

bool Aq40AttackEmperorPestsAction::Execute(Event event)
{
    Unit* current = AI_VALUE(Unit*, "current target");

    if (current && (current->GetName() == "qiraji scarab" || current->GetName() == "qiraji scorpion"))
    {
        return false;
    }

    Unit* pest1 = AI_VALUE2(Unit*, "find target", "qiraji scarab");
    Unit* pest2 = AI_VALUE2(Unit*, "find target", "qiraji scorpion");
    Unit* pest;

    if (pest1 && pest2)
    {
        if (pest1->GetDistance(bot) < pest2->GetDistance(bot))
        {
            pest = pest1;
        }
        else
        {
            pest = pest2;
        }
    }
    else if (pest1)
    {
        pest = pest1;
    }
    else if (pest2)
    {
        pest = pest2;
    }
    else
    {
        return false;
    }

    ObjectGuid guid = pest->GetGUID();
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
    bool result = Attack(pest);
    if (result)
        context->GetValue<ObjectGuid>("pull target")->Set(guid);

    return result;
}

bool Aq40MoveTowardsEmperorAction::Execute(Event event)
{
    const float radius = 20.0f; // assume general healing range of 40, try to keep healers near the tanks/victims

    if (Unit* boss = AI_VALUE2(Unit*, "find target", WhichEmperor()))
    {
        if (ObjectGuid bosstarget = boss->GetTarget())
        {
            Unit* target = botAI->GetUnit(bosstarget);

            long traveltarget = -1;//radius - bot->GetDistance(target);
            long travelboss = radius - bot->GetDistance(boss);

            if (traveltarget > travelboss)
            {
                return MoveTo(target, traveltarget);
            }
            else
            {
                return MoveTo(boss, travelboss);
            }
        }
    }

    return false;
}

bool Aq40OuroBurrowedFleeAction::Execute(Event event)
{
    bool doflee = false;

    if (Unit* boss = AI_VALUE2(Unit*, "find target", "ouro"))
    {
        if (boss->IsInCombat() && (boss->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) == UNIT_FLAG_NOT_SELECTABLE)
        {
            doflee = true;

            // todo: if ankle-biter scarab is near, doflee = false
        }

    }
    /*
    else if (Unit* boss = AI_VALUE2(Unit*, "find target", "eye of c'thun"))
    {
        if (boss->IsInCombat())
        {
            float dist = bot->GetDistance(boss);

            //printf("eyedist %f\n",dist);

            doflee = dist < 13.0;

            if (bot->GetName() == "Snusnu")
            {
                printf("eyedist %f doflee %d\n",dist,doflee);
            }
        }
    }
    */


    if (doflee)
    {
        if (!botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT))
        {
            // add/remove from both for now as it will make it more obvious to
            // player if this strat remains on after fight somehow
            // (which it will in this case, todo: remove after relevant, or just issue 'follow' command to bots)
            botAI->ChangeStrategy("+move from group", BOT_STATE_NON_COMBAT);
            botAI->ChangeStrategy("+move from group", BOT_STATE_COMBAT);
        }
    }
    else if (botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT))
    {
        // add/remove from both for now as it will make it more obvious to
        // player if this strat remains on after fight somehow
        // (which it will in this case, todo: remove after relevant, or just issue 'follow' command to bots)
        botAI->ChangeStrategy("-move from group", BOT_STATE_NON_COMBAT);
        botAI->ChangeStrategy("-move from group", BOT_STATE_COMBAT);
    }

    return true;
}

int Aq40Cthun1PositionAction::wrappingdistancebetween(int first, int second, int scope)
{
    int retval = first - second;
    if (abs(retval) > scope / 2)
    {
        if (retval > scope / 2)
        {
            retval = retval - scope;
        }
        else
        {
            retval = scope + retval;
        }
    }
    return retval;
}

int Aq40Cthun1PositionAction::getnearestpoint(int excludeouter, bool doinner)
{
    int retval = -1;
    float retvaldist = 0.0F;

    for (int n = 0; n < outerpointscount; n++)
    {
        float dist = bot->GetDistance(outerpoints[n]->GetPositionX(), outerpoints[n]->GetPositionY(), outerpoints[n]->GetPositionZ());

        if (retval < 0 || retvaldist > dist)
        {
            if (excludeouter != n)
            {
                retval = n;
                retvaldist = dist;
            }
        }
    }

    if (doinner)
    {
        for (int n = 0; n < innerpointscount; n++)
        {
            float dist = bot->GetDistance(innerpoints[n]->GetPositionX(), innerpoints[n]->GetPositionY(), innerpoints[n]->GetPositionZ());

            if (retval < 0 || retvaldist > dist)
            {
                retval = n + outerpointscount;
                retvaldist = dist;
            }
        }
    }

    return retval;
}

bool Aq40Cthun1PositionAction::Execute(Event event)
{
    // boss_cthun.cpp
    const int SPELL_GREEN_BEAM                            = 26134;
    const int SPELL_RED_COLORATION                        = 22518;        //Probably not the right spell but looks similar


    ObjectGuid botguid = bot->GetGUID();

    if (Unit* boss = AI_VALUE2(Unit*, "find target", "eye of c'thun"))
    {
        Position* point = NULL;

        ObjectGuid bosstargetguid = boss->GetTarget();
        if (boss->HasAura(SPELL_RED_COLORATION))
        {
            int bossd = (int)(boss->GetOrientation() * outerpointscount / (M_PI * 2.0));
            if (bossd < 0)
            {
                bossd = bossd + outerpointscount * ((-bossd) / outerpointscount + 1);
            }
            bossd %= outerpointscount;

            int nearest = getnearestpoint(bossd, false);
            int dist = wrappingdistancebetween(bossd, nearest, outerpointscount);

            if (abs(dist) <= outerpointscount / 6)
            {
                nearest = (nearest + outerpointscount - dist * (outerpointscount / 6) / abs(dist)) % outerpointscount;
            }

            point = outerpoints[nearest];

            if (bot->GetDistance(*point) < 0.5)
            {
                return false;
            }
        }
        else
        {
            if (bosstargetguid == botguid)
            {
                bot->StopMoving();
                return true;
            }
            else
            {
                Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

                if (spell and spell->m_spellInfo->Id == SPELL_GREEN_BEAM)
                {
                    Unit* bosstarget = botAI->GetUnit(bosstargetguid);
                    float bdist = bot->GetDistance(bosstarget);

                    if (bdist < 20.0)
                    {
                        if (bdist <= 10.0)
                        {
                            bot->AttackStop();
                            //printf("%s: still %f away from %s\n",bot->GetName().c_str(),bdist,bosstarget->GetName().c_str());
                        }

                        return MoveAway(bosstarget, 20.0);
                    }
                }
            }
        }

        if (!point)
        {
            Group* group = bot->GetGroup();
            if (bot->GetDistance(boss) < 60.0F && group)
            {
                Player* closest = NULL;
                float closestdist = 0.0F;

                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* member = itr->GetSource();

                    if (member->GetGUID() != botguid && member->IsAlive())
                    {
                        float dist = bot->GetDistance(member);

                        if (dist < 10.0 && (closest == NULL || closestdist < dist))
                        {
                            closest = member;
                            closestdist = dist;
                        }
                    }
                }

                if (closest)
                {
                    return MoveAway(closest, 10.0);
                }
            }

            int nearest = getnearestpoint();

            if (nearest >= outerpointscount)
            {
                point = innerpoints[nearest - outerpointscount];
            }
            else
            {
                point = outerpoints[nearest];
            }

            if (bot->GetDistance(*point) < 0.5)
            {
                return false;
            }
        }

        if (point)
        {
            return MoveTo(bot->GetMapId(), point->GetPositionX(), point->GetPositionY(), point->GetPositionZ(),
                false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}

bool Aq40Cthun2PositionAction::Execute(Event event)
{
    // boss_cthun.cpp
    const int NPC_TRIGGER                                 = 15384;
    const int NPC_EXIT_TRIGGER                            = 15800;

    const int SPELL_DIGESTIVE_ACID                        = 26476;

    // Areatriggers
    const int SPELL_SPIT_OUT                              = 25383;
    const int SPELL_EXIT_STOMACH                          = 26221;
    const int SPELL_RUBBLE_ROCKY                          = 26271;

    const int SPELL_CARAPACE_CTHUN                        = 26156;     // Server-side


    // temple_of_ahn_quiraj.h
    const int NPC_CLAW_TENTACLE       = 15725;
    const int NPC_EYE_TENTACLE        = 15726;
    const int NPC_GIANT_CLAW_TENTACLE = 15728;
    const int NPC_GIANT_EYE_TENTACLE  = 15334;
    const int NPC_FLESH_TENTACLE      = 15802;

    ObjectGuid botguid = bot->GetGUID();

    if (Unit* boss = AI_VALUE2(Unit*, "find target", "c'thun"))
    {
        Position* point = NULL;

        if (bot->GetPositionZ() > 0.0)
        {
            Creature* tentacle1 = bot->FindNearestCreature(NPC_GIANT_EYE_TENTACLE, 100.0f, true);

            if (tentacle1)
            {
                Group* group = bot->GetGroup();
                if (group)
                {
                    Player* closest = NULL;
                    float closestdist = 0.0F;

                    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                    {
                        Player* member = itr->GetSource();

                        if (member->GetGUID() != botguid && member->IsAlive())
                        {
                            float dist = bot->GetDistance(member);

                            if (dist < 11.0 && (closest == NULL || closestdist < dist))
                            {
                                closest = member;
                                closestdist = dist;
                            }
                        }
                    }

                    if (closest)
                    {
                        return MoveAway(closest, 11.0);
                    }
                }
            }

            Unit* attacktarget = NULL;

            if (!boss->HasAura(SPELL_CARAPACE_CTHUN))
            {
                attacktarget = boss;
            }
            else if (tentacle1)
            {
                attacktarget = tentacle1;
            }
            else if (Creature* tentacle3 = bot->FindNearestCreature(NPC_EYE_TENTACLE, 100.0f, true))
            {
                attacktarget = tentacle3;
            }
            else if (Creature* tentacle2 = bot->FindNearestCreature(NPC_GIANT_CLAW_TENTACLE, 100.0f, true))
            {
                attacktarget = tentacle2;
            }
            else if (Creature* tentacle4 = bot->FindNearestCreature(NPC_CLAW_TENTACLE, 100.0f, true))
            {
                attacktarget = tentacle4;
            }

            if (attacktarget)
            {
                ObjectGuid guid = attacktarget->GetGUID();

                botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
                bot->Attack(attacktarget, botAI->IsMelee(bot));

                return false;
            }

            outtriggered = false;
            attackpositioned = false;
        }
        else
        {
            if (bot->GetPositionZ() > -50.0)
            {
                // flying around out of bounds where they shouldn't be...
                // teleport them to splashdown at -8562.1 2037.0 -99.58
                bot->TeleportTo(bot->GetMapId(), -8562.1, 2037.0, -99.58, 5.03);
            }

            bool dogetout = false;

            if (!attackpositioned)
            {
                point = insideattack;

                if (bot->GetDistance(*point) < 1.0)
                {
                    attackpositioned = true;
                }
            }
            else
            {
                int astack = bot->GetAuraCount(SPELL_DIGESTIVE_ACID);
                int adamage = (astack + (astack + 1) + (astack + 2)) * 150; // 150 per stack, 5 second tick

                if (adamage >= bot->GetHealth())
                {
                    point = getout;
                    dogetout = true;
                    //printf("%s: low health, get out\n",bot->GetName().c_str());
                }
                else if (astack < 1) // bugged state check
                {
                    // immediately leave
                    point = getout;
                    dogetout = true;
                    //printf("%s: bugged state, get out\n",bot->GetName().c_str());
                }
                else if (botAI->IsTank(bot, true))
                {
                    // immediately leave
                    point = getout;
                    dogetout = true;
                    //printf("%s: tank, get out\n",bot->GetName().c_str());
                }
                else if (botAI->IsHeal(bot, true))
                {
                    // stack to 5 and leave, lower tolerance for healers
                    if (bot->GetAuraCount(SPELL_DIGESTIVE_ACID) >= 5 || bot->GetHealthPct() <= 50.0)
                    {
                        point = getout;
                        dogetout = true;
                    }
                }
                else
                {
                    // search completely failed with AI_VALUE2 here, but the above AI_VALUE2 started
                    // working after the below statement was used here instead?
                    if (Creature* tboss = bot->FindNearestCreature(NPC_FLESH_TENTACLE, 100.0f, true))
                    {
                        bool hastarget = false;

                        ObjectGuid targetguid = bot->GetTarget();
                        if (Unit* target = botAI->GetUnit(targetguid))
                        {
                            if (target->GetEntry() == NPC_FLESH_TENTACLE)
                            {
                                hastarget = true;
                            }
                        }

                        if (!hastarget)
                        {
                            ObjectGuid guid = tboss->GetGUID();

                            botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
                            bot->SetTarget(guid);

                            //printf("%s: attack flesh tentacle\n",bot->GetName().c_str());

                            if (botAI->IsMelee(bot))
                            {
                                // bots not picking up on these tentacles very effectively
                                return MoveTo(tboss, -1.0);
                            }

                            return false;
                        }
                        else
                        {
                            //printf("%s: tentacle already target\n",bot->GetName().c_str());
                        }
                    }
                    else
                    {
                        //printf("%s: no flesh tentacle, get out\n",bot->GetName().c_str());
                        point = getout;
                        dogetout = true;
                    }
                }
            }

            if (dogetout && !outtriggered)
            {
                if (bot->GetDistance(*point) < 10.0)
                {
                    // at_cthun_stomach_exit::OnTrigger from boss_cthun.cpp
                    Player* player = bot;
                    Unit* cthun = boss;

                    if (Creature* trigger = player->FindNearestCreature(NPC_TRIGGER, 15.0f))
                    {
                        if (!trigger->GetCurrentSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL))
                        {
                            trigger->CastSpell(player, SPELL_EXIT_STOMACH, true);

                            if (Creature* exittrigger = player->FindNearestCreature(NPC_EXIT_TRIGGER, 15.0f))
                            {
                                exittrigger->CastSpell(player, SPELL_RUBBLE_ROCKY, true);
                            }

                            outtriggered = true;
                        }
                    }

                    if (outtriggered)
                    {
                        player->m_Events.AddEventAtOffset([player, cthun]()
                        {
                            if (player->FindNearestCreature(NPC_EXIT_TRIGGER, 10.0f))
                            {
                                player->JumpTo(0.0f, 80.0f, false);

                                player->m_Events.AddEventAtOffset([player, cthun]()
                                {
                                    if (cthun)
                                        player->NearTeleportTo(cthun->GetPositionX(), cthun->GetPositionY(), cthun->GetPositionZ() + 10, float(rand32() % 6));

                                    player->RemoveAurasDueToSpell(SPELL_DIGESTIVE_ACID);
                                }, 1s);
                            }
                            else
                            {
                                player->m_Events.KillAllEvents(false);
                            }
                        }, 3s);
                    }
                }
            }
        }

        if (point)
        {
            return MoveTo(bot->GetMapId(), point->GetPositionX(), point->GetPositionY(), point->GetPositionZ(),
                false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    
    return false;
}
