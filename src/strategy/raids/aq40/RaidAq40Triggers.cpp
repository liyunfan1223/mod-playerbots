#include "RaidAq40Triggers.h"

#include "SharedDefines.h"

bool Aq40HasEmperorAggroTrigger::IsActive()
{
    // temple_of_ahnquiraj.h
    const int NPC_VEKLOR              = 15276;
    const int NPC_VEKNILASH           = 15275;

    ObjectGuid botguid = bot->GetGUID();
    ObjectGuid petguid = (ObjectGuid)(uint64)0UL;
    if (Unit* pet = bot->GetPet())
    {
        petguid = pet->GetGUID();
    }

    if (Creature* boss1 = bot->FindNearestCreature(NPC_VEKLOR, 200.0f))
    if (Creature* boss2 = bot->FindNearestCreature(NPC_VEKNILASH, 200.0f))
    {
        ObjectGuid boss1target = boss1->GetTarget();

        if (boss1target && (boss1target == botguid || (petguid && boss1target == petguid)))
        {
            return true;
        }

        ObjectGuid boss2target = boss2->GetTarget();

        if (boss2target && (boss2target == botguid || (petguid && boss2target == petguid)))
        {
            return true;
        }
    }
    return false;
}

bool Aq40WarlockTankEmperorTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARLOCK)
    {
        if (Unit* boss1 = AI_VALUE2(Unit*, "find target", "emperor vek'lor"))
        {
            ObjectGuid bottarget = bot->GetTarget();
            ObjectGuid bossguid = boss1->GetGUID();
            ObjectGuid bosstargetguid = boss1->GetTarget();
            if (bosstargetguid)
            {
                Player* bosstarget = botAI->GetPlayer(bosstargetguid);

                if (!bosstarget || bosstarget->getClass() != CLASS_WARLOCK || !botAI->IsTank(bosstarget, true))
                {
                    //std::ostringstream out;
                    //out << "want tank?  target="<<bottarget.GetRawValue()<<" vek'lor="<<bossguid.GetRawValue();
                    //botAI->TellError(out.str());

                    return bottarget == bossguid && bosstargetguid != bot->GetGUID();
                }
            }
        }
    }

    return false;
}

bool Aq40MageFrostboltViscidusTrigger::IsActive()
{
    if (bot->getClass() == CLASS_MAGE)
    {
        if (Unit* boss1 = AI_VALUE2(Unit*, "find target", "viscidus"))
        {
            ObjectGuid bottarget = bot->GetTarget();
            ObjectGuid bossguid = boss1->GetGUID();

            //std::ostringstream out;
            //out << "want tank?  target="<<bottarget.GetRawValue()<<" vek'lor="<<bossguid.GetRawValue();
            //botAI->TellError(out.str());

            return bottarget == bossguid;
        }
    }

    return false;
}

bool Aq40MeleeViscidusTrigger::IsActive()
{
    // defined in boss_viscidus.cpp
    const int SPELL_VISCIDUS_FREEZE       = 25937;

    if (Unit* boss1 = AI_VALUE2(Unit*, "find target", "viscidus"))
    {
        if (boss1->HasAura(SPELL_VISCIDUS_FREEZE))
        {
            return true;
        }
    }

    return false;
}

bool Aq40EmperorTrigger::IsActive()
{
    Unit* boss1 = AI_VALUE2(Unit*, "find target", "emperor vek'lor");
    Unit* boss2 = AI_VALUE2(Unit*, "find target", "emperor vek'nilash");

    if (boss1 && boss2 && (boss1->IsInCombat() || boss2->IsInCombat()))
    {
        Group* group = bot->GetGroup();
        if (!group)
        {
            return false;
        }

        ObjectGuid self = bot->GetGUID();
        ObjectGuid masterguid = botAI->GetMaster()->GetGUID();

        std::vector<Player*> memberlists[4];
        for (int n = 0; n < 4; n++)
        {
            memberlists[n].reserve(group->GetMembersCount());
        }

        int selftype = -1;

        std::vector<Player*> members;
        members.reserve(group->GetMembersCount());

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            auto memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI)
            {
                Player* submaster = memberAI->GetMaster();
                if (submaster && submaster->GetGUID() == masterguid)
                {
                    int assignedtype;
                    if (member->getClass() == CLASS_WARLOCK)
                    {
                        // lol warlock tanks
                        assignedtype = 0;
                    }
                    else if (memberAI->IsTank(member))
                    {
                        assignedtype = 1;
                    }
                    else if (memberAI->IsHeal(member))
                    {
                        assignedtype = 2;
                    }
                    else
                    {
                        assignedtype = 3;
                    }

                    memberlists[assignedtype].push_back(member);
                    if (member->GetGUID() == self)
                    {
                        selftype = assignedtype;
                    }
                }
            }
        }

        if (selftype < 0 || (IsForHealers() != (selftype == 2)))
        {
            return false;
        }


        int sametypecount = memberlists[selftype].size();

        std::vector<float> dist1(sametypecount);
        std::vector<ObjectGuid> dist1who(sametypecount);
        std::vector<Unit*> dist1unit(sametypecount);

        std::vector<float> dist2(sametypecount);
        std::vector<ObjectGuid> dist2who(sametypecount);
        std::vector<Unit*> dist2unit(sametypecount);

        for (int n = 0; n < sametypecount; n++)
        {
            auto member = memberlists[selftype][n];

            dist1[n] = boss1->GetDistance(member);
            dist2[n] = boss2->GetDistance(member);
            dist1who[n] = member->GetGUID();
            dist2who[n] = member->GetGUID();
            dist1unit[n] = member;
            dist2unit[n] = member;
        }

        for (int n = 0; n < sametypecount; n++)
        {
            for (int subn = n + 1; subn < sametypecount; subn++)
            {
                if (dist1[n] > dist1[subn])
                {
                    float fswap = dist1[n];
                    dist1[n] = dist1[subn];
                    dist1[subn] = fswap;

                    ObjectGuid gswap = dist1who[n];
                    dist1who[n] = dist1who[subn];
                    dist1who[subn] = gswap;

                    Unit* uswap = dist1unit[n];
                    dist1unit[n] = dist1unit[subn];
                    dist1unit[subn] = uswap;
                }

                if (dist2[n] > dist2[subn])
                {
                    float fswap = dist2[n];
                    dist2[n] = dist2[subn];
                    dist2[subn] = fswap;

                    ObjectGuid gswap = dist2who[n];
                    dist2who[n] = dist2who[subn];
                    dist2who[subn] = gswap;

                    Unit* uswap = dist2unit[n];
                    dist2unit[n] = dist2unit[subn];
                    dist2unit[subn] = uswap;
                }
            }
        }

        std::vector<ObjectGuid> boss1assigned;
        int boss1assignedindex = 0;
        std::vector<ObjectGuid> boss2assigned;
        int boss2assignedindex = 0;

        if (selftype == 3)
        {
            sametypecount -= 4;
        }
        else if (selftype == 1)
        {
            if (sametypecount > 3)
            {
                sametypecount -= 2;
            }
            else if (sametypecount > 2)
            {
                sametypecount--;
            }
        }
        else if (selftype == 2)
        {
            if (sametypecount > 4)
            {
                sametypecount = 4;
            }
        }

        int lastcount = 0;
        while (boss1assignedindex + boss2assignedindex < sametypecount)
        {
            for (int n = 0; n < sametypecount; n++)
            {
                if (dist1who[n] && boss1assignedindex <= boss2assignedindex)
                {
                    if (selftype != 3 || (dist1unit[n]->getClass() != CLASS_ROGUE && dist1unit[n]->getClass() != CLASS_WARRIOR && dist1unit[n]->getClass() != CLASS_HUNTER))
                    {
                        boss1assigned[boss1assignedindex] = dist1who[n];
                        boss1assignedindex++;

                        for (int subn = 0; subn < sametypecount; subn++)
                        {
                            if (dist2who[subn] == dist1who[n])
                            {
                                dist2who[subn] = (ObjectGuid)(uint64)0UL;
                                break;
                            }
                        }
                        dist1who[n] = (ObjectGuid)(uint64)0UL;
                    }
                }

                if (boss1assignedindex + boss2assignedindex >= sametypecount)
                {
                    break;
                }

                if (dist2who[n] && boss2assignedindex <= boss1assignedindex)
                {
                    if (selftype != 3 || (dist2unit[n]->getClass() != CLASS_WARLOCK && dist2unit[n]->getClass() != CLASS_MAGE && dist2unit[n]->getClass() != CLASS_PRIEST))
                    {
                        boss2assigned[boss2assignedindex] = dist2who[n];
                        boss2assignedindex++;

                        for (int subn = 0; subn < sametypecount; subn++)
                        {
                            if (dist1who[subn] == dist2who[n])
                            {
                                dist1who[subn] = (ObjectGuid)(uint64)0UL;
                                break;
                            }
                        }
                        dist2who[n] = (ObjectGuid)(uint64)0UL;
                    }
                }
            }

            if (lastcount == boss1assignedindex + boss2assignedindex)
            {
                // failure to keep filling slots, abort
                break;
            }

            lastcount = boss1assignedindex + boss2assignedindex;
        }

        for (int n = 0; n < boss1assignedindex; n++)
        {
            if (boss1assigned[n] == self)
            {
                if (IsVekLor())
                {
                    std::ostringstream out;
                    out << "attack vek'lor";
                    botAI->TellError(out.str());

                    return true;
                }

                return false;
            }
        }

        for (int n = 0; n < boss2assignedindex; n++)
        {
            if (boss2assigned[n] == self)
            {
                if (IsVekNilash())
                {
                    std::ostringstream out;
                    out << "attack vek'nilash";
                    botAI->TellError(out.str());

                    return true;
                }

                return false;
            }
        }

        if (IsPestControl())
        {
            std::ostringstream out;
            out << "attack pests";
            botAI->TellError(out.str());

            return true;
        }
    }

    return false;
}

bool Aq40OuroBurrowedTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "ouro"))
    {
        if (boss->IsInCombat() && (boss->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) == UNIT_FLAG_NOT_SELECTABLE)
        {
            return true;
        }
    }
    /*
    else if (Unit* boss = AI_VALUE2(Unit*, "find target", "eye of c'thun"))
    {
        if (boss->IsInCombat())
        {
            return true;
        }
    }
    */

    return botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT);
}

bool Aq40Cthun1StartedTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "c'thun"))
    {
        if (boss->IsAlive() && boss->IsInCombat() && (boss->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) == 0)
        {
            return false;
        }
    }
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "eye of c'thun"))
    {
        if (boss->IsAlive() && boss->IsInCombat())
        {
            if (!wasactive)
            {
                //printf("start cthun 1\n");
                wasactive = true;
            }
            return true;
        }
    }

    if (wasactive)
    {
        //printf("end cthun 1\n");
        wasactive = false;
        return true;
    }
    return false;
}

bool Aq40Cthun2StartedTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "c'thun"))
    {
        if (boss->IsAlive() && boss->IsInCombat() && (boss->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) == 0)
        {
            if (!wasactive)
            {
                //printf("start cthun 2\n");
                wasactive = true;
            }
            return true;
        }
    }

    if (wasactive)
    {
        //printf("end cthun 2\n");
        wasactive = false;
        return true;
    }
    return false;
}
