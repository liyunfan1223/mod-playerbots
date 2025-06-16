#include "RaidIccMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "RaidIccActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ScriptedCreature.h"
#include "ShamanActions.h"
#include "UseMeetingStoneAction.h"
#include "WarriorActions.h"
#include "PlayerbotAI.h"
#include "RaidIccTriggers.h"

// LK global variables
namespace
{
uint32 g_lastPlagueTime = 0;
bool g_plagueAllowedToCure = false;
std::map<ObjectGuid, uint32> g_plagueTimes;
std::map<ObjectGuid, bool> g_allowCure;
std::mutex g_plagueMutex;  // Lock before accessing shared variables
}

// Lady Deathwhisper
float IccLadyDeathwhisperMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<FleeAction*>(action) || dynamic_cast<FollowAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    static constexpr uint32 VENGEFUL_SHADE_ID = NPC_SHADE;

    // Get the nearest hostile NPCs
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Allow the IccShadeLadyDeathwhisperAction to run
    if (dynamic_cast<IccShadeLadyDeathwhisperAction*>(action))
        return 1.0f;

    for (const auto& npcGuid : npcs)
    {
        Unit* shade = botAI->GetUnit(npcGuid);

        if (!shade || shade->GetEntry() != VENGEFUL_SHADE_ID)
            continue;

        if (!shade->GetVictim() || shade->GetVictim()->GetGUID() != bot->GetGUID())
            continue;

        return 0.0f;  // Cancel all other actions when we need to handle Vengeful Shade
    }

    return 1.0f;
}

// dbs
float IccAddsDbsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
        dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
        dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
        dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (botAI->IsRanged(bot))
        if (dynamic_cast<ReachSpellAction*>(action))
            return 0.0f;

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("rune of blood", bot);
        if (aura)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    return 1.0f;
}

// dogs
float IccDogsMultiplier::GetValue(Action* action)
{
    bool bossPresent = false;
    if (AI_VALUE2(Unit*, "find target", "stinky") || AI_VALUE2(Unit*, "find target", "precious"))
        bossPresent = true;

    if (!bossPresent)
        return 1.0f;

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mortal wound", bot, false, true);
        if (aura && aura->GetStackAmount() >= 8)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }
    return 1.0f;
}

// Festergut
float IccFestergutMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    if (dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("gastric bloat", bot, false, true);
        if (aura && aura->GetStackAmount() >= 6)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    if (dynamic_cast<IccFestergutSporeAction*>(action))
        return 1.0f;

    if (bot->HasAura(SPELL_GAS_SPORE))
        return 0.0f;


    return 1.0f;
}

// Rotface
float IccRotfaceMultiplier::GetValue(Action* action)
{
    Unit* boss1 = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss1)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if (dynamic_cast<FleeAction*>(action) && !(bot->getClass() == CLASS_HUNTER))
        return 0.0f;

    if (dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsAssistTank(bot) && (dynamic_cast<AttackRtiTargetAction*>(action) || dynamic_cast<TankAssistAction*>(action))) 
        return 0.0f; 

    Unit* boss = AI_VALUE2(Unit*, "find target", "big ooze");
    if (!boss)
        return 1.0f;

    static std::map<ObjectGuid, uint32> lastExplosionTimes;
    static std::map<ObjectGuid, bool> hasMoved;
    
    ObjectGuid botGuid = bot->GetGUID();

    // When cast starts, record the time
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_UNSTABLE_OOZE_EXPLOSION))
    {
        if (lastExplosionTimes[botGuid] == 0)  // Only set if not already set
        {
            lastExplosionTimes[botGuid] = time(nullptr);
            hasMoved[botGuid] = false;
        }
    }

    // If explosion cast is no longer active, reset the timers
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_UNSTABLE_OOZE_EXPLOSION))
    {
        if (lastExplosionTimes[botGuid] > 0 && time(nullptr) - lastExplosionTimes[botGuid] >= 16)
        {
            lastExplosionTimes[botGuid] = 0;
            hasMoved[botGuid] = false;
            return 1.0f;  // Allow normal actions to resume
        }
    }

    // If 9 seconds have passed since cast start and we haven't moved yet
    if (lastExplosionTimes[botGuid] > 0 && !hasMoved[botGuid] && time(nullptr) - lastExplosionTimes[botGuid] >= 9)
    {
        if (dynamic_cast<MovementAction*>(action)
            && !dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
        {
            return 0.0f;  // Block other movement actions
        }
        hasMoved[botGuid] = true;  // Mark that we've initiated movement
    }

    // Continue blocking other movements for 7 seconds after moving
    if (hasMoved[botGuid] && time(nullptr) - lastExplosionTimes[botGuid] < 16  // 9 seconds wait + 7 seconds stay
        && dynamic_cast<MovementAction*>(action)
        && !dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
        return 0.0f;

    return 1.0f;
}

// pp
float IccAddsPutricideMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return 1.0f;

    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);
    bool hasUnboundPlague = botAI->HasAura("Unbound Plague", bot);

    if (!(bot->getClass() == CLASS_HUNTER) && dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastDisengageAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mutated plague", bot, false, true);
        if (aura && aura->GetStackAmount() >= 4)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    if (hasGaseousBloat)
    {
        if (dynamic_cast<IccPutricideGasCloudAction*>(action))
            return 1.0f;

        if (dynamic_cast<IccPutricideGrowingOozePuddleAction*>(action))
            return 1.0f;

        if (botAI->IsHeal(bot))
            return 1.0f;
        else
            return 0.0f;  // Cancel all other actions when we need to handle Gaseous Bloat
    }

    if (hasUnboundPlague && boss && !boss->HealthBelowPct(35))
    {
        if (dynamic_cast<IccPutricideAvoidMalleableGooAction*>(action))
            return 1.0f;
        else
            return 0.0f;  // Cancel all other actions when we need to handle Unbound Plague
    }

    if (dynamic_cast<IccPutricideVolatileOozeAction*>(action))
    {
        if (dynamic_cast<IccPutricideAvoidMalleableGooAction*>(action))
            return 0.0f;
        if (dynamic_cast<IccPutricideGrowingOozePuddleAction*>(action) && !botAI->IsMainTank(bot))
            return 0.0f;
        //if (dynamic_cast<IccPutricideGasCloudAction*>(action) && !hasGaseousBloat)
            //return 0.0f;
    }

    return 1.0f;
}

// bpc
float IccBpcAssistMultiplier::GetValue(Action* action)
{
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!keleseth)
        return 1.0f;

    if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
        dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
        dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
        dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura) 
    {
        if (aura->GetStackAmount() > 18 && botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }

        if (aura->GetStackAmount() > 12 && !botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }
    }

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return 1.0f;

    if (valanar && valanar->HasUnitState(UNIT_STATE_CASTING) &&
        (valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX1) ||
         valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX2) ||
         valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX3) ||
         valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX4)))
    {
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<IccBpcEmpoweredVortexAction*>(action))
            return 1.0f;
        else
            return 0.0f;  // Cancel all other actions when we need to handle Empowered Vortex
    }

    Unit* flame1 = bot->FindNearestCreature(NPC_BALL_OF_FLAME, 100.0f);
    Unit* flame2 = bot->FindNearestCreature(NPC_BALL_OF_INFERNO_FLAME, 100.0f);
    bool ballOfFlame = flame1 && flame1->GetVictim() == bot;
    bool infernoFlame = flame2 && flame2->GetVictim() == bot;

    if (flame2)
    {
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<IccBpcKineticBombAction*>(action))
            return 0.0f;

        if (dynamic_cast<IccBpcBallOfFlameAction*>(action))
            return 1.0f;
    }

    if (ballOfFlame || infernoFlame)
    {
        // If bot is tank, do nothing special
        if (dynamic_cast<IccBpcBallOfFlameAction*>(action))
            return 1.0f;
        else
            return 0.0f;  // Cancel all other actions when we need to handle Ball of Flame
    }

    static const std::array<uint32, 4> bombEntries = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2, NPC_KINETIC_BOMB3,
                                                      NPC_KINETIC_BOMB4};
    const GuidVector bombs = AI_VALUE(GuidVector, "possible targets no los");

    bool bombFound = false;

    for (const auto entry : bombEntries)
    {
        for (const auto& guid : bombs)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->GetEntry() == entry)
                {
                    // Check if bomb is within valid Z-axis range
                    if (unit->GetPositionZ() - bot->GetPositionZ() < 25.0f)
                    {
                        bombFound = true;
                        break;
                    }
                }
            }
        }
        if (bombFound)
            break;
    }

    if (bombFound && !(aura && aura->GetStackAmount() > 12) && !botAI->IsTank(bot))
    {
        // If kinetic bomb action is active, disable these actions
        if (dynamic_cast<IccBpcKineticBombAction*>(action))
            return 1.0f;

        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;
    }

    // For assist tank during BPC fight
    if (botAI->IsAssistTank(bot) && !(aura && aura->GetStackAmount() > 18))
    {
        // Allow BPC-specific actions
        if (dynamic_cast<IccBpcKelesethTankAction*>(action))
            return 1.0f;

        // Disable normal assist behavior
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<AttackRtiTargetAction*>(action) ||
            dynamic_cast<CastConsecrationAction*>(action))
            return 0.0f;

    }

    return 1.0f;
}

//BQL
float IccBqlMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return 1.0f;

    Aura* aura2 = botAI->GetAura("Swarming Shadows", bot);
    Aura* aura = botAI->GetAura("Frenzied Bloodthirst", bot);

    if (botAI->IsRanged(bot))
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<CastDisengageAction*>(action))
            return 0.0f;

    // If bot has Pact of Darkfallen aura, return 0 for all other actions
    if (bot->HasAura(SPELL_PACT_OF_THE_DARKFALLEN))
    {
        if (dynamic_cast<IccBqlPactOfDarkfallenAction*>(action))
            return 1.0f;  // Allow Pact of Darkfallen action
        else
            return 0.0f;  // Cancel all other actions when we need to handle Pact of Darkfallen
    }

    if (botAI->IsMelee(bot) && ((boss->GetPositionZ() - ICC_BQL_CENTER_POSITION.GetPositionZ()) > 5.0f) && !aura)
    {
        if (dynamic_cast<IccBqlGroupPositionAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    // If bot has frenzied bloodthirst, allow highest priority for bite action
    if (aura)  // If bot has frenzied bloodthirst
    {
        if (dynamic_cast<IccBqlVampiricBiteAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    if (aura2 && !aura)
    {
        if (dynamic_cast<IccBqlGroupPositionAction*>(action))
            return 1.0f;
        else
            return 0.0f;  // Cancel all other actions when we need to handle Swarming Shadows
    }
    return 1.0f;

    if ((boss->GetExactDist2d(ICC_BQL_TANK_POSITION.GetPositionX(), ICC_BQL_TANK_POSITION.GetPositionY()) > 10.0f) &&
        botAI->IsRanged(bot) && !((boss->GetPositionZ() - bot->GetPositionZ()) > 5.0f))
    {
        if (dynamic_cast<FleeAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

}

//VDW
float IccValithriaDreamCloudMultiplier::GetValue(Action* action)
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);

    Aura* twistedNightmares = botAI->GetAura("Twisted Nightmares", bot);
    Aura* emeraldVigor = botAI->GetAura("Emerald Vigor", bot);


    if (!boss && !bot->HasAura(SPELL_DREAM_STATE))
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if (botAI->IsTank(bot))
    {
        if (dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;
    }

    if (botAI->IsHeal(bot) && (twistedNightmares || emeraldVigor))
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;

    if (bot->HasAura(SPELL_DREAM_STATE) && !bot->HealthBelowPct(50))
    {
        if (dynamic_cast<IccValithriaDreamCloudAction*>(action))
            return 1.0f;  // Allow Dream Cloud action
        else
            return 0.0f;  // Cancel all other actions when we need to handle Dream Cloud
    }

    return 1.0f;

}

//SINDRAGOSA

float IccSindragosaMultiplier::GetValue(Action* action)
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return 1.0f;
    Aura* aura = botAI->GetAura("Unchained Magic", bot, false, true);

    Difficulty diff = bot->GetRaidDifficulty();

    if (boss->HealthBelowPct(95))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action) || dynamic_cast<CastStarfallAction*>(action))
            return 0.0f;
    }

    if (aura && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC) &&
        !dynamic_cast<IccSindragosaFrostBombAction*>(action))
    {
        if (dynamic_cast<MovementAction*>(action) || dynamic_cast<IccSindragosaUnchainedMagicAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    // Check if boss is casting blistering cold (using both normal and heroic spell IDs)
    if (boss->HasUnitState(UNIT_STATE_CASTING) &&
        (boss->FindCurrentSpellBySpellId(70123) || boss->FindCurrentSpellBySpellId(71047) ||
         boss->FindCurrentSpellBySpellId(71048) || boss->FindCurrentSpellBySpellId(71049)))
    {
        // If this is the blistering cold action, give it highest priority
        if (dynamic_cast<IccSindragosaBlisteringColdAction*>(action) ||
            dynamic_cast<HealPartyMemberAction*>(action) ||
            dynamic_cast<ReachPartyMemberToHealAction*>(action) ||
            dynamic_cast<IccSindragosaTankSwapPositionAction*>(action))
            return 1.0f;

        // Disable all other actions while blistering cold is casting
        return 0.0f;
    }

    // Highest priority if we have beacon
    if (bot->HasAura(SPELL_FROST_BEACON))
    {
        if (dynamic_cast<IccSindragosaFrostBeaconAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    Group* group = bot->GetGroup();
    // Check if anyone in group has Frost Beacon (SPELL_FROST_BEACON)
    bool anyoneHasFrostBeacon = false;

    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
            {
                anyoneHasFrostBeacon = true;
                break;
            }
        }
    }

    if (anyoneHasFrostBeacon && boss &&
        boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(),
                             ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 30.0f &&
        !boss->HealthBelowPct(25) && !boss->HealthAbovePct(99))
    {
        if (dynamic_cast<IccSindragosaFrostBeaconAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    if (anyoneHasFrostBeacon && !botAI->IsMainTank(bot))
    {
        if (dynamic_cast<IccSindragosaGroupPositionAction*>(action))
            return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
        if (aura && aura->GetStackAmount() >= 6)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    if (!botAI->IsTank(bot) && boss && boss->HealthBelowPct(35))
    {
        if (dynamic_cast<IccSindragosaGroupPositionAction*>(action))
            return 0.0f;
    }

    if (boss && botAI->IsTank(bot))
    {
        if (boss->HealthBelowPct(35))
        {
            if (dynamic_cast<IccSindragosaTankSwapPositionAction*>(action) || dynamic_cast<TankFaceAction*>(action) ||
                dynamic_cast<AttackAction*>(action) || dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    if (boss && boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 30.0f && !boss->HealthBelowPct(25) && !boss->HealthAbovePct(99))
    {
        if (dynamic_cast<IccSindragosaFrostBombAction*>(action))
            return 1.0f;

        if (dynamic_cast<FollowAction*>(action) || dynamic_cast<IccSindragosaBlisteringColdAction*>(action) ||
            dynamic_cast<IccSindragosaChilledToTheBoneAction*>(action) || dynamic_cast<IccSindragosaMysticBuffetAction*>(action) || 
            dynamic_cast<IccSindragosaFrostBeaconAction*>(action) || dynamic_cast<IccSindragosaUnchainedMagicAction*>(action) || 
            dynamic_cast<FleeAction*>(action) || dynamic_cast<CastDisengageAction*>(action) || dynamic_cast<PetAttackAction*>(action) ||
            dynamic_cast<IccSindragosaGroupPositionAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
            dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
            dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
            dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
            dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastConsecrationAction*>(action) ||
            dynamic_cast<CastFlamestrikeAction*>(action) || dynamic_cast<CastExplosiveTrapAction*>(action) ||
            dynamic_cast<CastExplosiveShotAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float IccLichKingAddsMultiplier::GetValue(Action* action)
{
    Unit* terenasMenethilHC = bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f);

    if (!terenasMenethilHC)
        if (dynamic_cast<CastStarfallAction*>(action))
            return 0.0f;

    if (terenasMenethilHC)
    {
        Unit* mainTank = AI_VALUE(Unit*, "main tank");

        if (!botAI->IsMainTank(bot) && mainTank && bot->GetExactDist2d(mainTank->GetPositionX(), mainTank->GetPositionY()) < 2.0f)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }

        if (botAI->IsMelee(bot) || (bot->getClass() == CLASS_WARLOCK))
        {
            if (dynamic_cast<MovementAction*>(action) || dynamic_cast<IccLichKingAddsAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }

        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) || dynamic_cast<CastChargeAction*>(action) ||
            dynamic_cast<CastFeralChargeBearAction*>(action) || dynamic_cast<CastIceBlockAction*>(action) ||
            dynamic_cast<CastRevivePetAction*>(action) || dynamic_cast<TankAssistAction*>(action))
            return 0.0f;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return 1.0f;

    // Handle cure actions
    if (dynamic_cast<CurePartyMemberAction*>(action) || dynamic_cast<CastCleanseDiseaseAction*>(action) ||
        dynamic_cast<CastCleanseDiseaseOnPartyAction*>(action) ||
        dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) || dynamic_cast<CastCleanseSpiritAction*>(action))
    {
        Group* group = bot->GetGroup();
        if (!group)
            return 1.0f;

        // Check if any bot in the group has plague
        bool anyBotHasPlague = false;
        ObjectGuid plaguedPlayerGuid;  // Track who has plague
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            if (Player* member = ref->GetSource())
            {
                if (botAI->HasAura("Necrotic Plague", member))
                {
                    anyBotHasPlague = true;
                    plaguedPlayerGuid = member->GetGUID();  // Changed from GetObjectGuid()
                    break;
                }
            }
        }

        uint32 currentTime = getMSTime();

        // Reset state if no one has plague
        if (!anyBotHasPlague)
        {
            std::lock_guard<std::mutex> lock(g_plagueMutex);  // Properly initialized
            g_plagueTimes.clear();
            g_allowCure.clear();
            return 1.0f;
        }

        {                                                     // New scope for lock_guard
            std::lock_guard<std::mutex> lock(g_plagueMutex);  // Properly initialized

            // Start timer if this is a new plague
            if (g_plagueTimes.find(plaguedPlayerGuid) == g_plagueTimes.end())
            {
                g_plagueTimes[plaguedPlayerGuid] = currentTime;
                g_allowCure[plaguedPlayerGuid] = false;
                return 0.0f;
            }

            // Once we allow cure, keep allowing it until plague is gone
            if (g_allowCure[plaguedPlayerGuid])
            {
                return 1.0f;
            }

            // Check if enough time has passed (2,5 seconds)
            if (currentTime - g_plagueTimes[plaguedPlayerGuid] >= 2500)
            {
                g_allowCure[plaguedPlayerGuid] = true;
                return 1.0f;
            }
        }  // lock_guard is automatically released here

        return 0.0f;
    }

    if (dynamic_cast<FleeAction*>(action) && (bot->getClass() != CLASS_HUNTER))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action))
        return 0.0f;

    if (boss && !boss->HealthBelowPct(71))
    {
        if (!botAI->IsTank(bot))
            if (dynamic_cast<CastConsecrationAction*>(action))
                return 0.0f;

        if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
            dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
            dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
            dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
            dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastFlamestrikeAction*>(action) ||
            dynamic_cast<CastExplosiveTrapAction*>(action) || dynamic_cast<CastExplosiveShotAction*>(action))
            return 0.0f;
    }

    Unit* currentTarget = AI_VALUE(Unit*, "current target");

    bool hasWinterAura = false;
    if (boss && (boss->HasAura(SPELL_REMORSELESS_WINTER1) || boss->HasAura(SPELL_REMORSELESS_WINTER2) ||
                 boss->HasAura(SPELL_REMORSELESS_WINTER3) || boss->HasAura(SPELL_REMORSELESS_WINTER4)))
        hasWinterAura = true;

    bool hasWinter2Aura = false;
    if (boss && (boss->HasAura(SPELL_REMORSELESS_WINTER5) || boss->HasAura(SPELL_REMORSELESS_WINTER6) ||
                 boss->HasAura(SPELL_REMORSELESS_WINTER7) || boss->HasAura(SPELL_REMORSELESS_WINTER8)))
        hasWinter2Aura = true;

    bool isCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
        isCasting = true;

    bool isWinter = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER1) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER2) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER5) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER6) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER3) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER4) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER7) ||
        boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER8)))
        isWinter = true;

    if (hasWinterAura || hasWinter2Aura || (isCasting && isWinter))
    {
        if (dynamic_cast<IccLichKingWinterAction*>(action) || dynamic_cast<SetFacingTargetAction*>(action))
            return 1.0f;

        if (botAI->IsAssistTank(bot) && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        if (dynamic_cast<IccLichKingAddsAction*>(action))
            return 0.0f;

        if (currentTarget && boss && bot->GetDistance2d(boss->GetPositionX(), boss->GetPositionY()) > 50.0f && currentTarget == boss)
        {
            if (dynamic_cast<AttackRtiTargetAction*>(action) || dynamic_cast<ReachSpellAction*>(action) ||
                dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAssistAction*>(action) ||
                dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }

        if (currentTarget && (currentTarget->GetEntry() == NPC_ICE_SPHERE1 || currentTarget->GetEntry() == NPC_ICE_SPHERE2 ||
            currentTarget->GetEntry() == NPC_ICE_SPHERE3 || currentTarget->GetEntry() == NPC_ICE_SPHERE4))
        {
            if (dynamic_cast<MovementAction*>(action) || dynamic_cast<ReachMeleeAction*>(action) ||
                dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }

    }

    if (botAI->IsRanged(bot) && !botAI->GetAura("Harvest Soul", bot, false, false))
    {
        // Check for defile presence
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        bool defilePresent = false;
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (unit && unit->IsAlive() && unit->GetEntry() == DEFILE_NPC_ID)  // Defile entry
            {
                defilePresent = true;
                break;
            }
        }

        // Only disable movement if defile is present
        if (defilePresent && (
            dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<MoveRandomAction*>(action) ||
            dynamic_cast<MoveFromGroupAction*>(action)))
        {
            return 0.0f;
        }
    }

    if (botAI->IsAssistTank(bot) && boss && !boss->HealthBelowPct(71) && currentTarget == boss)
    {
        if (dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}
