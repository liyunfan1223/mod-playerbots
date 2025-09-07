/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDTRIGGERS_H
#define _PLAYERBOT_DRUIDTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include <set>

class PlayerbotAI;

class MarkOfTheWildOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    MarkOfTheWildOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "mark of the wild", 2 * 2000) {}

    bool IsActive() override;
};

class MarkOfTheWildTrigger : public BuffTrigger
{
public:
    MarkOfTheWildTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "mark of the wild", 2 * 2000) {}

    bool IsActive() override;
};

class ThornsOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    ThornsOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "thorns", 2 * 2000) {}

    bool IsActive() override;
};

class ThornsOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    ThornsOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "thorns", false, 2 * 2000) {}
};

class ThornsTrigger : public BuffTrigger
{
public:
    ThornsTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "thorns", 2 * 2000) {}

    bool IsActive() override;
};

class OmenOfClarityTrigger : public BuffTrigger
{
public:
    OmenOfClarityTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "omen of clarity") {}
};

class RakeTrigger : public DebuffTrigger
{
public:
    RakeTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "rake", 1, true) {}
};

class InsectSwarmTrigger : public DebuffTrigger
{
public:
    InsectSwarmTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "insect swarm", 1, true) {}
};

class MoonfireTrigger : public DebuffTrigger
{
public:
    MoonfireTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "moonfire", 1, true) {}
};

class FaerieFireTrigger : public DebuffTrigger
{
public:
    FaerieFireTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "faerie fire", 1, false, 25.0f) {}
};

class FaerieFireFeralTrigger : public DebuffTrigger
{
public:
    FaerieFireFeralTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "faerie fire (feral)") {}
};

class BashInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    BashInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "bash") {}
};

class TigersFuryTrigger : public BuffTrigger
{
public:
    TigersFuryTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "tiger's fury") {}
};

class BerserkTrigger : public BoostTrigger
{
public:
    BerserkTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "berserk") {}
};

class SavageRoarTrigger : public BuffTrigger
{
public:
    SavageRoarTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "savage roar") {}
};

class NaturesGraspTrigger : public BuffTrigger
{
public:
    NaturesGraspTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "nature's grasp") {}
};

class EntanglingRootsTrigger : public HasCcTargetTrigger
{
public:
    EntanglingRootsTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "entangling roots") {}
};

class EntanglingRootsKiteTrigger : public DebuffTrigger
{
public:
    EntanglingRootsKiteTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "entangling roots") {}

    bool IsActive() override;
};

class HibernateTrigger : public HasCcTargetTrigger
{
public:
    HibernateTrigger(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, "hibernate") {}
};

class CurePoisonTrigger : public NeedCureTrigger
{
public:
    CurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cure poison", DISPEL_POISON) {}
};

class PartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCurePoisonTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure poison", DISPEL_POISON)
    {
    }
};

class BearFormTrigger : public BuffTrigger
{
public:
    BearFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "bear form") {}

    bool IsActive() override;
};

class TreeFormTrigger : public BuffTrigger
{
public:
    TreeFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "tree of life") {}

    bool IsActive() override;
};

class CatFormTrigger : public BuffTrigger
{
public:
    CatFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "cat form") {}

    bool IsActive() override;
};

class EclipseSolarTrigger : public HasAuraTrigger
{
public:
    EclipseSolarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "eclipse (solar)") {}
};

class EclipseLunarTrigger : public HasAuraTrigger
{
public:
    EclipseLunarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "eclipse (lunar)") {}
};

class BashInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
public:
    BashInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "bash") {}
};

class NaturesSwiftnessTrigger : public BuffTrigger
{
public:
    NaturesSwiftnessTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "nature's swiftness") {}
};

class DruidPartyMemberRemoveCurseTrigger : public PartyMemberNeedCureTrigger
{
public:
    DruidPartyMemberRemoveCurseTrigger(PlayerbotAI* ai)
        : PartyMemberNeedCureTrigger(ai, "druid remove curse", DISPEL_CURSE)
    {
    }
};

class EclipseSolarCooldownTrigger : public SpellCooldownTrigger
{
public:
    EclipseSolarCooldownTrigger(PlayerbotAI* ai) : SpellCooldownTrigger(ai, "eclipse (solar)") {}
    bool IsActive() override { return bot->HasSpellCooldown(48517); }
};

class EclipseLunarCooldownTrigger : public SpellCooldownTrigger
{
public:
    EclipseLunarCooldownTrigger(PlayerbotAI* ai) : SpellCooldownTrigger(ai, "eclipse (lunar)") {}
    bool IsActive() override { return bot->HasSpellCooldown(48518); }
};

class MangleCatTrigger : public DebuffTrigger
{
public:
    MangleCatTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "mangle (cat)", 1, false, 0.0f) {}
    bool IsActive() override
    {
        return DebuffTrigger::IsActive() && !botAI->HasAura("mangle (bear)", GetTarget(), false, false, -1, true)
            && !botAI->HasAura("trauma", GetTarget(), false, false, -1, true);
    }
};

class FerociousBiteTimeTrigger : public Trigger
{
public:
    FerociousBiteTimeTrigger(PlayerbotAI* ai) : Trigger(ai, "ferocious bite time") {}
    bool IsActive() override
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target)
            return false;

        uint8 cp = AI_VALUE2(uint8, "combo", "current target");
        if (cp < 5)
            return false;

        Aura* roar = botAI->GetAura("savage roar", bot);
        bool roarCheck = !roar || roar->GetDuration() > 10000;
        if (!roarCheck)
            return false;

        Aura* rip = botAI->GetAura("rip", target, true);
        bool ripCheck = !rip || rip->GetDuration() > 10000;
        if (!ripCheck)
            return false;

        return true;
    }
};

class HurricaneChannelCheckTrigger : public Trigger
{
public:
    HurricaneChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 2)
        : Trigger(botAI, "hurricane channel check"), minEnemies(minEnemies)
    {
    }

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> HURRICANE_SPELL_IDS;
};

#endif
