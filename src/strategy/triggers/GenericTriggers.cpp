/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericTriggers.h"

#include <string>

#include "BattlegroundWS.h"
#include "CreatureAI.h"
#include "GameTime.h"
#include "LastSpellCastValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "TemporarySummon.h"
#include "ThreatMgr.h"
#include "Timer.h"

bool LowManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->lowMana;
}

bool MediumManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->mediumMana;
}

bool NoPetTrigger::IsActive()
{
    return (bot->GetMinionGUID().IsEmpty()) && (!AI_VALUE(Unit*, "pet target")) && (!bot->GetGuardianPet()) &&
           (!bot->GetFirstControlled()) && (!AI_VALUE2(bool, "mounted", "self target"));
}

bool HasPetTrigger::IsActive()
{
    return (AI_VALUE(Unit*, "pet target")) && !AI_VALUE2(bool, "mounted", "self target");
    ;
}

bool PetAttackTrigger::IsActive()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
    {
        return false;
    }
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
    {
        return false;
    }
    if (pet->GetVictim() == target && pet->GetCharmInfo()->IsCommandAttack())
    {
        return false;
    }
    if (bot->GetMap()->IsDungeon() && bot->GetGroup() && !target->IsInCombat())
    {
        return false;
    }
    return true;
}

bool HighManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->highMana;
}

bool AlmostFullManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") > 85;
}

bool EnoughManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig->highMana;
}

bool RageAvailable::IsActive() { return AI_VALUE2(uint8, "rage", "self target") >= amount; }

bool EnergyAvailable::IsActive() { return AI_VALUE2(uint8, "energy", "self target") >= amount; }

bool ComboPointsAvailableTrigger::IsActive() { return AI_VALUE2(uint8, "combo", "current target") >= amount; }

bool ComboPointsNotFullTrigger::IsActive() { return AI_VALUE2(uint8, "combo", "current target") < amount; }

bool TargetWithComboPointsLowerHealTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return ComboPointsAvailableTrigger::IsActive() &&
           (target->GetHealth() / AI_VALUE(float, "estimated group dps")) <= lifeTime;
}

bool LoseAggroTrigger::IsActive() { return !AI_VALUE2(bool, "has aggro", "current target"); }

bool HasAggroTrigger::IsActive() { return AI_VALUE2(bool, "has aggro", "current target"); }

bool PanicTrigger::IsActive()
{
    return AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig->criticalHealth &&
           (!AI_VALUE2(bool, "has mana", "self target") ||
            AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->lowMana);
}

bool OutNumberedTrigger::IsActive()
{
    if (bot->GetMap() && (bot->GetMap()->IsDungeon() || bot->GetMap()->IsRaid()))
        return false;

    if (bot->GetGroup() && bot->GetGroup()->isRaidGroup())
        return false;

    int32 botLevel = bot->GetLevel();
    uint32 friendPower = 200;
    uint32 foePower = 0;
    for (auto& attacker : botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get())
    {
        Creature* creature = botAI->GetCreature(attacker);
        if (!creature)
            continue;

        int32 dLevel = creature->GetLevel() - botLevel;
        if (dLevel > -10)
            foePower = std::max(100 + 10 * dLevel, dLevel * 200);
    }

    if (!foePower)
        return false;

    for (auto& helper : botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get())
    {
        Unit* player = botAI->GetUnit(helper);
        if (!player || player == bot)
            continue;

        int32 dLevel = player->GetLevel() - botLevel;

        if (dLevel > -10 && bot->GetDistance(player) < 10.0f)
            friendPower += std::max(200 + 20 * dLevel, dLevel * 200);
    }

    return friendPower < foePower;
}

bool BuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    if (!SpellTrigger::IsActive())
        return false;
    Aura* aura = botAI->GetAura(spell, target, checkIsOwner, checkDuration);
    if (!aura)
        return true;
    if (beforeDuration && aura->GetDuration() < beforeDuration)
        return true;
    return false;
}

Value<Unit*>* BuffOnPartyTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", spell);
}

bool ProtectPartyMemberTrigger::IsActive() { return AI_VALUE(Unit*, "party member to protect"); }

Value<Unit*>* DebuffOnAttackerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("attacker without aura", spell);
}

Value<Unit*>* DebuffOnMeleeAttackerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("melee attacker without aura", spell);
}

bool NoAttackersTrigger::IsActive()
{
    return !AI_VALUE(Unit*, "current target") && AI_VALUE(uint8, "my attacker count") > 0;
}

bool InvalidTargetTrigger::IsActive() { return AI_VALUE2(bool, "invalid target", "current target"); }

bool NoTargetTrigger::IsActive() { return !AI_VALUE(Unit*, "current target"); }

bool MyAttackerCountTrigger::IsActive()
{
    return AI_VALUE2(bool, "combat", "self target") && AI_VALUE(uint8, "my attacker count") >= amount;
}

bool LowTankThreatTrigger::IsActive()
{
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
        return false;

    ThreatMgr& mgr = current_target->GetThreatMgr();
    float threat = mgr.GetThreat(bot);
    float tankThreat = mgr.GetThreat(mt);
    return tankThreat == 0.0f || threat > tankThreat * 0.5f;
}

bool AoeTrigger::IsActive()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
    {
        return false;
    }
    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    int attackers_count = 0;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetExactDist2d(current_target) <= range)
        {
            attackers_count++;
        }
    }
    return attackers_count >= amount;
}

bool NoFoodTrigger::IsActive()
{
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(bot);
    if (isRandomBot && sPlayerbotAIConfig->freeFood)
        return false;

    return AI_VALUE2(std::vector<Item*>, "inventory items", "conjured food").empty();
}

bool NoDrinkTrigger::IsActive()
{
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(bot);
    if (isRandomBot && sPlayerbotAIConfig->freeFood)
        return false;

    return AI_VALUE2(std::vector<Item*>, "inventory items", "conjured water").empty();
}

bool TargetInSightTrigger::IsActive() { return AI_VALUE(Unit*, "grind target"); }

bool DebuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return BuffTrigger::IsActive() && (target->GetHealth() / AI_VALUE(float, "estimated group dps")) >= needLifeTime;
}

bool DebuffOnBossTrigger::IsActive()
{
    if (!DebuffTrigger::IsActive())
    {
        return false;
    }
    Creature* c = GetTarget()->ToCreature();
    return c && ((c->IsDungeonBoss()) || (c->isWorldBoss()));
}

bool SpellTrigger::IsActive() { return GetTarget(); }

bool SpellCanBeCastTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && botAI->CanCastSpell(spell, target);
}

bool SpellNoCooldownTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    if (!spellId)
        return false;

    return !bot->HasSpellCooldown(spellId);
}

bool SpellCooldownTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    if (!spellId)
        return false;

    return bot->HasSpellCooldown(spellId);
}

RandomTrigger::RandomTrigger(PlayerbotAI* botAI, std::string const name, int32 probability)
    : Trigger(botAI, name), probability(probability), lastCheck(getMSTime())
{
}

bool RandomTrigger::IsActive()
{
    if (getMSTime() - lastCheck < sPlayerbotAIConfig->repeatDelay)
        return false;

    lastCheck = getMSTime();
    int32 k = (int32)(probability / sPlayerbotAIConfig->randomChangeMultiplier);
    if (k < 1)
        k = 1;
    return (rand() % k) == 0;
}

bool AndTrigger::IsActive() { return ls && rs && ls->IsActive() && rs->IsActive(); }

std::string const AndTrigger::getName()
{
    std::string name(ls->getName());
    name = name + " and ";
    name = name + rs->getName();
    return name;
}

bool TwoTriggers::IsActive()
{
    if (name1.empty() || name2.empty())
        return false;

    Trigger* trigger1 = botAI->GetAiObjectContext()->GetTrigger(name1);
    Trigger* trigger2 = botAI->GetAiObjectContext()->GetTrigger(name2);

    if (!trigger1 || !trigger2)
        return false;

    return trigger1->IsActive() && trigger2->IsActive();
}

std::string const TwoTriggers::getName()
{
    std::string name;
    name = name1 + " and " + name2;
    return name;
}

bool BoostTrigger::IsActive()
{
    if (!BuffTrigger::IsActive())
        return false;
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->ToPlayer())
        return true;
    return AI_VALUE(uint8, "balance") <= balance;
}

bool GenericBoostTrigger::IsActive() 
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->ToPlayer())
        return true;
    return AI_VALUE(uint8, "balance") <= balance;
}

bool HealerShouldAttackTrigger::IsActive() 
{
    // nobody can help me
    if (botAI->GetNearGroupMemberCount(sPlayerbotAIConfig->sightDistance) <= 1)
        return true;
    
    if (AI_VALUE2(uint8, "health", "party member to heal") < sPlayerbotAIConfig->almostFullHealth)
        return false;
    
    // special check for resto druid (dont remove tree of life frequently)
    if (bot->GetAura(33891))
    {
        LastSpellCast& lastSpell = botAI->GetAiObjectContext()->GetValue<LastSpellCast&>("last spell cast")->Get();
        if (lastSpell.timer + 5 > time(nullptr))
            return false;
    }

    int manaThreshold;
    int balance = AI_VALUE(uint8, "balance");
    // higher threshold in higher pressure
    if (balance <= 50)
        manaThreshold = 85;
    else if (balance <= 100)
        manaThreshold = sPlayerbotAIConfig->highMana;
    else
        manaThreshold = sPlayerbotAIConfig->mediumMana;

    if (AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < manaThreshold)
        return false;

    return true;
}

bool ItemCountTrigger::IsActive() { return AI_VALUE2(uint32, "item count", item) < count; }

bool InterruptSpellTrigger::IsActive()
{
    return SpellTrigger::IsActive() && botAI->IsInterruptableSpellCasting(GetTarget(), getName());
}

bool DeflectSpellTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!target->IsNonMeleeSpellCast(true))
        return false;

    if (target->GetTarget() != bot->GetGUID())
        return false;

    uint32 spellid = context->GetValue<uint32>("spell id", spell)->Get();
    if (!spellid)
        return false;

    SpellInfo const* deflectSpell = sSpellMgr->GetSpellInfo(spellid);
    if (!deflectSpell)
        return false;

    // warrior deflects all
    if (spell == "spell reflection")
        return true;

    // human priest feedback
    if (spell == "feedback")
        return true;

    SpellSchoolMask deflectSchool = SpellSchoolMask(deflectSpell->Effects[EFFECT_0].MiscValue);
    SpellSchoolMask attackSchool = SPELL_SCHOOL_MASK_NONE;

    if (Spell* spell = target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
    {
        if (SpellInfo const* tarSpellInfo = spell->GetSpellInfo())
        {
            attackSchool = tarSpellInfo->GetSchoolMask();
            if (deflectSchool == attackSchool)
                return true;
        }
    }
    return false;
}

bool AttackerCountTrigger::IsActive() { return AI_VALUE(uint8, "attacker count") >= amount; }

bool HasAuraTrigger::IsActive() { return botAI->HasAura(getName(), GetTarget(), false, false, -1, true); }

bool HasAuraStackTrigger::IsActive()
{
    Aura* aura = botAI->GetAura(getName(), GetTarget(), false, true, stack);
    // sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "HasAuraStackTrigger::IsActive %s %d", getName(), aura ?
    // aura->GetStackAmount() : -1);
    return aura;
}

bool TimerTrigger::IsActive()
{
    if (time(nullptr) != lastCheck)
    {
        lastCheck = time(nullptr);
        return true;
    }

    return false;
}

bool HasNoAuraTrigger::IsActive() { return !botAI->HasAura(getName(), GetTarget()); }

bool TankAssistTrigger::IsActive()
{
    if (!AI_VALUE(uint8, "attacker count"))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return true;

    Unit* tankTarget = AI_VALUE(Unit*, "tank target");
    if (!tankTarget || currentTarget == tankTarget)
        return false;

    return AI_VALUE2(bool, "has aggro", "current target");
}

bool IsBehindTargetTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && AI_VALUE2(bool, "behind", "current target");
}

bool IsNotBehindTargetTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !AI_VALUE2(bool, "behind", "current target");
}

bool IsNotFacingTargetTrigger::IsActive() { return !AI_VALUE2(bool, "facing", "current target"); }

bool HasCcTargetTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "cc target", getName()) && !AI_VALUE2(Unit*, "current cc target", getName());
}

bool NoMovementTrigger::IsActive() { return !AI_VALUE2(bool, "moving", "self target"); }

bool NoPossibleTargetsTrigger::IsActive()
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    return !targets.size();
}

bool PossibleAddsTrigger::IsActive() { return AI_VALUE(bool, "possible adds") && !AI_VALUE(ObjectGuid, "pull target"); }

bool NotDpsTargetActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    // do not switch if enemy target
    if (target && target->IsAlive())
    {
        Unit* enemy = AI_VALUE(Unit*, "enemy player target");
        if (target == enemy)
            return false;
    }

    Unit* dps = AI_VALUE(Unit*, "dps target");
    return dps && target != dps;
}

bool NotDpsAoeTargetActiveTrigger::IsActive()
{
    Unit* dps = AI_VALUE(Unit*, "dps aoe target");
    Unit* target = AI_VALUE(Unit*, "current target");
    Unit* enemy = AI_VALUE(Unit*, "enemy player target");

    // do not switch if enemy target
    if (target && target == enemy && target->IsAlive())
        return false;

    return dps && target != dps;
}

bool IsSwimmingTrigger::IsActive() { return AI_VALUE2(bool, "swimming", "self target"); }

bool HasNearestAddsTrigger::IsActive()
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest adds");
    return targets.size();
}

bool HasItemForSpellTrigger::IsActive()
{
    std::string const spell = getName();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && AI_VALUE2(Item*, "item for spell", spellId);
}

bool TargetChangedTrigger::IsActive()
{
    Unit* oldTarget = context->GetValue<Unit*>("old target")->Get();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    return target && oldTarget != target;
}

Value<Unit*>* InterruptEnemyHealerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

bool RandomBotUpdateTrigger::IsActive() { return RandomTrigger::IsActive() && AI_VALUE(bool, "random bot update"); }

bool NoNonBotPlayersAroundTrigger::IsActive()
{
    return !botAI->HasPlayerNearby();
    /*if (!bot->InBattleground())
        return AI_VALUE(GuidVector, "nearest non bot players").empty();

    return false;
    */
}

bool NewPlayerNearbyTrigger::IsActive() { return AI_VALUE(ObjectGuid, "new player nearby"); }

bool CollisionTrigger::IsActive() { return AI_VALUE2(bool, "collision", "self target"); }

bool GiveItemTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "party member without item", item) && AI_VALUE2(uint32, "item count", item);
}

bool GiveFoodTrigger::IsActive()
{
    return AI_VALUE(Unit*, "party member without food") && AI_VALUE2(uint32, "item count", item);
}

bool GiveWaterTrigger::IsActive()
{
    return AI_VALUE(Unit*, "party member without water") && AI_VALUE2(uint32, "item count", item);
}

Value<Unit*>* SnareTargetTrigger::GetTargetValue() { return context->GetValue<Unit*>("snare target", spell); }

bool StayTimeTrigger::IsActive()
{
    time_t stayTime = AI_VALUE(time_t, "stay time");
    time_t now = time(nullptr);
    return delay && stayTime && now > stayTime + 2 * delay / 1000;
}

bool IsMountedTrigger::IsActive() { return AI_VALUE2(bool, "mounted", "self target"); }

bool CorpseNearTrigger::IsActive()
{
    return bot->GetCorpse() && bot->GetCorpse()->IsWithinDistInMap(bot, CORPSE_RECLAIM_RADIUS, true);
}

bool IsFallingTrigger::IsActive() { return bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING); }

bool IsFallingFarTrigger::IsActive() { return bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_FAR); }

bool HasAreaDebuffTrigger::IsActive() { return AI_VALUE2(bool, "has area debuff", "self target"); }

Value<Unit*>* BuffOnMainTankTrigger::GetTargetValue() { return context->GetValue<Unit*>("main tank", spell); }