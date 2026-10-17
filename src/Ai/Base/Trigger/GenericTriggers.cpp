/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericTriggers.h"
#include "Corpse.h"
#include "CreatureAI.h"
#include "GenericBuffUtils.h"
#include "ItemVisitors.h"
#include "LastSpellCastValue.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "SharedDefines.h"
#include "TargetValue.h"
#include "TemporarySummon.h"
#include "ThreatManager.h"
#include "Timer.h"
#include <string>

bool LowManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig.LowMana;
}

bool MediumManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig.MediumMana;
}

bool LowEnergyTrigger::IsActive()
{
    return AI_VALUE2(uint8, "energy", "self target") < threshold;
}

bool NoPetTrigger::IsActive()
{
    return bot->GetMinionGUID().IsEmpty() && !AI_VALUE(Unit*, "pet target") && !bot->GetGuardianPet() &&
           !bot->GetFirstControlled() && !AI_VALUE2(bool, "mounted", "self target");
}

bool HasPetTrigger::IsActive()
{
    return AI_VALUE(Unit*, "pet target") && !AI_VALUE2(bool, "mounted", "self target");
}

bool PetAttackTrigger::IsActive()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (pet->GetVictim() == target && pet->GetCharmInfo()->IsCommandAttack())
        return false;

    if (bot->GetMap()->IsDungeon() && bot->GetGroup() && !target->IsInCombat())
        return false;

    return true;
}

bool HighManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig.HighMana;
}

bool AlmostFullManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") > 85;
}

bool EnoughManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") &&
           AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.HighMana;
}

bool RageAvailable::IsActive() { return AI_VALUE2(uint8, "rage", "self target") >= amount; }

bool EnergyAvailable::IsActive() { return AI_VALUE2(uint8, "energy", "self target") >= amount; }

bool ComboPointsAvailableTrigger::IsActive() { return AI_VALUE2(uint8, "combo", "current target") >= amount; }

bool ComboPointsNotFullTrigger::IsActive() { return AI_VALUE2(uint8, "combo", "current target") < amount; }

bool TargetWithComboPointsLowerHealTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return ComboPointsAvailableTrigger::IsActive() &&
           (target->GetHealth() / AI_VALUE(float, "estimated group dps")) <= lifeTime;
}

bool LoseAggroTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsPlayer())
        return false;  // players have no threat: taunts only waste the cooldown
    return !AI_VALUE2(bool, "has aggro", "current target");
}

bool HasAggroTrigger::IsActive() { return AI_VALUE2(bool, "has aggro", "current target"); }

bool BuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    Aura* aura = botAI->GetAura(spell, target, checkIsOwner, checkDuration);
    return ai::buff::BuffBelowRefreshTarget(botAI, aura, beforeDuration);
}

Value<Unit*>* BuffOnPartyTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura", ai::buff::MakeAuraQualifierForBuff(spell));
}

bool BuffOnPartyTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (ai::buff::ShouldDeferPartyBuffEvaluationForRecentLogin(bot, target, spell))
        return false;

    return BuffTrigger::IsActive();
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

bool MediumThreatTrigger::IsActive()
{
    if (!AI_VALUE(Unit*, "main tank"))
        return false;

    return MyAttackerCountTrigger::IsActive();
}

bool LowTankThreatTrigger::IsActive()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target || current_target->IsPlayer())
        return false;  // players have no threat: Tricks of the Trade / Misdirection would be wasted

    Unit* mainTank = AI_VALUE(Unit*, "main tank");
    if (!mainTank)
        return false;

    ThreatManager& mgr = current_target->GetThreatMgr();
    float threat = mgr.GetThreat(bot);
    float tankThreat = mgr.GetThreat(mainTank);
    return tankThreat == 0.0f || threat > tankThreat * 0.5f;
}

bool AoeTrigger::IsActive()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
        return false;

    return HasEnoughAoeTargets(botAI, current_target->GetPosition(), range, static_cast<uint32>(amount));
}

bool NoFoodTrigger::IsActive()
{
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot);
    if (isRandomBot && botAI->HasCheat(BotCheatMask::food))
        return false;

    return AI_VALUE2(std::vector<Item*>, "inventory items", "conjured food").empty();
}

bool NoDrinkTrigger::IsActive()
{
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot);
    if (isRandomBot && botAI->HasCheat(BotCheatMask::food))
        return false;

    return AI_VALUE2(std::vector<Item*>, "inventory items", "conjured water").empty();
}

bool TargetInSightTrigger::IsActive() { return AI_VALUE(Unit*, "grind target"); }

bool DebuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return BuffTrigger::IsActive() &&
           (target->GetHealth() / AI_VALUE(float, "estimated group dps")) >= needLifeTime;
}

bool DebuffOnBossTrigger::IsActive()
{
    if (!DebuffTrigger::IsActive())
        return false;

    Creature* creature = GetTarget()->ToCreature();
    return creature && (creature->IsDungeonBoss() || creature->isWorldBoss());
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
    : Trigger(botAI, name), probability(probability), lastCheck(getMSTime()) {}

bool RandomTrigger::IsActive()
{
    if (getMSTime() - lastCheck < sPlayerbotAIConfig.RepeatDelay)
        return false;

    lastCheck = getMSTime();
    int32 k = (int32)(probability / sPlayerbotAIConfig.RandomChangeMultiplier);
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
    if (botAI->GetNearGroupMemberCount(sPlayerbotAIConfig.SightDistance) <= 1)
        return true;

    if (AI_VALUE2(uint8, "health", "party member to heal") < sPlayerbotAIConfig.AlmostFullHealth)
        return false;

    if (bot->GetAura(33891)) // Tree of Life
    {
        LastSpellCast& lastSpell = botAI->GetAiObjectContext()->GetValue<LastSpellCast&>("last spell cast")->Get();
        if (lastSpell.timer + 5 > time(nullptr))
            return false;
    }

    int manaThreshold;
    int balance = AI_VALUE(uint8, "balance");
    if (balance <= 50)
        manaThreshold = 85;
    else if (balance <= 100)
        manaThreshold = sPlayerbotAIConfig.HighMana;
    else
        manaThreshold = sPlayerbotAIConfig.MediumMana;

    if (AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < manaThreshold)
        return false;

    return true;
}

bool ItemCountTrigger::IsActive() { return AI_VALUE2(uint32, "item count", item) < uint32(count); }

bool InterruptSpellTrigger::IsActive()
{
    return SpellTrigger::IsActive() && botAI->IsInterruptableSpellCasting(GetTarget(), getName());
}

bool DeflectSpellTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsNonMeleeSpellCast(true) || target->GetTarget() != bot->GetGUID())
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

bool LossOfControlTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_MOD_STUN) ||
           bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
           bot->HasAuraType(SPELL_AURA_MOD_ROOT) ||
           bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
           bot->HasAuraType(SPELL_AURA_MOD_CHARM);
}

bool FearCharmSleepTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
           bot->HasAuraType(SPELL_AURA_MOD_CHARM) ||
           bot->HasAuraType(SPELL_AURA_AOE_CHARM) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP);
}

bool FearSleepSapTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_MOD_FEAR) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SLEEP) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_SAPPED);
}

bool PoisonDiseaseBleedTrigger::IsActive()
{
    return botAI->HasAuraToDispel(bot, DISPEL_POISON) ||
           botAI->HasAuraToDispel(bot, DISPEL_DISEASE) ||
           bot->HasAuraWithMechanic(1 << MECHANIC_BLEED);
}

bool MovementImpairedTrigger::IsActive()
{
    return botAI->IsMovementImpaired(bot) &&
           !botAI->HasAnyAuraOf(bot, "stealth", "prowl", nullptr);
}

bool HasAuraStackTrigger::IsActive()
{
    return botAI->GetAura(getName(), GetTarget(), false, true, stack);
}

bool TimerTrigger::IsActive()
{
    time_t now = time(nullptr);

    if (now != lastCheck)
    {
        lastCheck = now;
        return true;
    }

    return false;
}

bool TimerBGTrigger::IsActive()
{
    time_t now = time(nullptr);

    if (now - lastCheck >= 60)
    {
        lastCheck = now;
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
    if (botAI->HasStrategy("stay", botAI->GetState()))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !AI_VALUE2(bool, "behind", "current target");
}

bool IsNotFacingTargetTrigger::IsActive()
{
    if (botAI->HasStrategy("stay", botAI->GetState()))
        return false;

    return !AI_VALUE2(bool, "facing", "current target");
}

bool HasCcTargetTrigger::IsActive()
{
    Unit* rtiCcTarget = nullptr;
    if (botAI->IsInNonRaidDungeon())
    {
        rtiCcTarget = AI_VALUE(Unit*, "rti cc target");
        if (!rtiCcTarget)
            return false;
    }

    return IsCcTargetFree(AI_VALUE2(Unit*, "cc target", getName()), rtiCcTarget);
}

bool HasCcTargetTrigger::IsCcTargetFree(Unit* ccTarget, Unit* rtiCcTarget)
{
    if (!ccTarget || (rtiCcTarget && ccTarget != rtiCcTarget))
        return false;

    return !AI_VALUE2(Unit*, "current cc target", getName());
}

bool NoMovementTrigger::IsActive() { return !AI_VALUE2(bool, "moving", "self target"); }

bool NoPossibleTargetsTrigger::IsActive()
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    return !targets.size();
}

bool PossibleAddsTrigger::IsActive()
{
    return AI_VALUE(bool, "possible adds") && !AI_VALUE(ObjectGuid, "pull target");
}

bool NotDpsTargetActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
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

    if (target && target == enemy && target->IsAlive())
        return false;

    return dps && target != dps;
}

bool IsSwimmingTrigger::IsActive() { return AI_VALUE2(bool, "swimming", "self target"); }

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

bool RandomBotUpdateTrigger::IsActive()
{
    return RandomTrigger::IsActive() && AI_VALUE(bool, "random bot update");
}

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

bool ReturnToStayPositionTrigger::IsActive()
{
    PositionInfo stayPosition = AI_VALUE(PositionMap&, "position")["stay"];
    if (stayPosition.isSet())
    {
        const float distance = bot->GetDistance(stayPosition.x, stayPosition.y, stayPosition.z);
        return distance > sPlayerbotAIConfig.FollowDistance;
    }

    return false;
}

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

bool AmmoCountTrigger::IsActive()
{
    if (bot->GetUInt32Value(PLAYER_AMMO_ID) != 0)
        return ItemCountTrigger::IsActive();  // Ammo already equipped

    if (botAI->FindAmmo())
        return true;  // Found ammo in inventory but not equipped

    return ItemCountTrigger::IsActive();
}

bool NewPetTrigger::IsActive()
{
    ObjectGuid currentPetGuid = ObjectGuid::Empty;

    if (Pet* pet = bot->GetPet())
        currentPetGuid = pet->GetGUID();
    else if (Guardian* guardian = bot->GetGuardianPet())
        currentPetGuid = guardian->GetGUID();

    if (currentPetGuid != lastPetGuid)
    {
        triggered = false;
        lastPetGuid = currentPetGuid;
    }

    if (currentPetGuid != ObjectGuid::Empty && !triggered)
    {
        triggered = true;
        return true;
    }

    return false;
}

bool ForceRebuffPendingTrigger::IsActive()
{
    return botAI->forceRebuff.IsPending();
}
