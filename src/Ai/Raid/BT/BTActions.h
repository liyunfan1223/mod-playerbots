/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTACTIONS_H
#define PLAYERBOTS_BTACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "BTHelpers.h"
#include "MovementActions.h"

namespace BlackTempleHelpers
{
    struct EyeBlastDangerArea;
}

// General

class BlackTempleEraseTimersAndTrackersAction : public Action
{
public:
    BlackTempleEraseTimersAndTrackersAction(
        PlayerbotAI* botAI) : Action(botAI, "black temple erase timers and trackers") {}
    bool Execute(Event event) override;
};

// High Warlord Naj'entus

class HighWarlordNajentusMisdirectBossToMainTankAction : public AttackAction
{
public:
    HighWarlordNajentusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusTanksPositionBossAction : public AttackAction
{
public:
    HighWarlordNajentusTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "high warlord naj'entus tanks position boss") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusDisperseRangedAction : public MovementAction
{
public:
    HighWarlordNajentusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "naj'entus disperse ranged") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusRemoveImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusRemoveImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus remove impaling spine") {}
    bool Execute(Event event) override;
};

class HighWarlordNajentusThrowImpalingSpineAction : public MovementAction
{
public:
    HighWarlordNajentusThrowImpalingSpineAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "high warlord naj'entus throw impaling spine") {}
    bool Execute(Event event) override;
};

// Supremus

class SupremusMisdirectBossToMainTankAction : public AttackAction
{
public:
    SupremusMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "supremus misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class SupremusDisperseRangedAction : public MovementAction
{
public:
    SupremusDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus disperse ranged") {}
    bool Execute(Event event) override;
};

class SupremusKiteBossAction : public MovementAction
{
public:
    SupremusKiteBossAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus kite boss") {}
    bool Execute(Event event) override;
};

class SupremusMoveAwayFromVolcanosAction : public MovementAction
{
public:
    SupremusMoveAwayFromVolcanosAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "supremus move away from volcanos") {}
    bool Execute(Event event) override;

private:
    Position FindSafestNearbyPosition(
        std::vector<Unit*> const& volcanos, float maxRadius, float hazardRadius);
    bool IsPathSafeFromVolcanos(Position const& start,
        Position const& end, std::vector<Unit*> const& volcanos, float hazardRadius);
    std::vector<Unit*> GetAllSupremusVolcanos();
};

class SupremusManagePhaseTimerAction : public Action
{
public:
    SupremusManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "supremus manage phase timer") {}
    bool Execute(Event event) override;
};

// Shade of Akama

class ShadeOfAkamaMeleeDpsPrioritizeChannelersAction : public AttackAction
{
public:
    ShadeOfAkamaMeleeDpsPrioritizeChannelersAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "shade of akama melee dps prioritize channelers") {}
    bool Execute(Event event) override;
};

// Teron Gorefiend

class TeronGorefiendMisdirectBossToMainTankAction : public AttackAction
{
public:
    TeronGorefiendMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "teron gorefiend misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class TeronGorefiendTanksPositionBossAction : public AttackAction
{
public:
    TeronGorefiendTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "teron gorefiend tanks position boss") {}
    bool Execute(Event event) override;
};

class TeronGorefiendPositionRangedOnBalconyAction : public MovementAction
{
public:
    TeronGorefiendPositionRangedOnBalconyAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend position ranged on balcony") {}
    bool Execute(Event event) override;
};

class TeronGorefiendAvoidShadowOfDeathAction : public Action
{
public:
    TeronGorefiendAvoidShadowOfDeathAction(
        PlayerbotAI* botAI) : Action(botAI, "teron gorefiend avoid shadow of death") {}
    bool Execute(Event event) override;
};

class TeronGorefiendMoveToCornerToDieAction : public MovementAction
{
public:
    TeronGorefiendMoveToCornerToDieAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend move to corner to die") {}
    bool Execute(Event event) override;
};

class TeronGorefiendControlAndDestroyShadowyConstructsAction : public MovementAction
{
public:
    TeronGorefiendControlAndDestroyShadowyConstructsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "teron gorefiend control and destroy shadowy constructs") {}
    bool Execute(Event event) override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilMisdirectBossToMainTankAction : public AttackAction
{
public:
    GurtoggBloodboilMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "gurtogg bloodboil misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilTanksPositionBossAction : public AttackAction
{
public:
    GurtoggBloodboilTanksPositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "gurtogg bloodboil tanks position boss") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilRotateRangedGroupsAction : public MovementAction
{
public:
    GurtoggBloodboilRotateRangedGroupsAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "gurtogg bloodboil rotate ranged groups") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction : public MovementAction
{
public:
    GurtoggBloodboilRangedMoveAwayFromEnragedPlayerAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "gurtogg bloodboil ranged move away from enraged player") {}
    bool Execute(Event event) override;
};

class GurtoggBloodboilManagePhaseTimerAction : public Action
{
public:
    GurtoggBloodboilManagePhaseTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "gurtogg bloodboil manage phase timer") {}
    bool Execute(Event event) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsMisdirectBossToMainTankAction : public AttackAction
{
public:
    ReliquaryOfSoulsMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "reliquary of souls misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsAdjustDistanceFromSufferingAction : public MovementAction
{
public:
    ReliquaryOfSoulsAdjustDistanceFromSufferingAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "reliquary of souls adjust distance from suffering") {}
    bool Execute(Event event) override;

private:
    bool TanksMoveToMinimumRange(Unit* suffering);
    bool MeleeDpsStayAtMaximumRange(Unit* suffering);
    bool RangedMoveAwayFromBoss(Unit* suffering);
};

class ReliquaryOfSoulsHealersDpsSufferingAction : public Action
{
public:
    ReliquaryOfSoulsHealersDpsSufferingAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls healers dps suffering") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellstealRuneShieldAction : public Action
{
public:
    ReliquaryOfSoulsSpellstealRuneShieldAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls spellsteal rune shield") {}
    bool Execute(Event event) override;
};

class ReliquaryOfSoulsSpellReflectDeadenAction : public Action
{
public:
    ReliquaryOfSoulsSpellReflectDeadenAction(
        PlayerbotAI* botAI) : Action(botAI, "reliquary of souls spell reflect deaden") {}
    bool Execute(Event event) override;
};

// Mother Shahraz

class MotherShahrazMisdirectBossToMainTankAction : public AttackAction
{
public:
    MotherShahrazMisdirectBossToMainTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "mother shahraz misdirect boss to main tank") {}
    bool Execute(Event event) override;
};

class MotherShahrazTanksPositionBossUnderPillarAction : public AttackAction
{
public:
    MotherShahrazTanksPositionBossUnderPillarAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "mother shahraz tanks position boss under pillar") {}
    bool Execute(Event event) override;
};

class MotherShahrazMeleeDpsWaitAtSafePositionAction : public MovementAction
{
public:
    MotherShahrazMeleeDpsWaitAtSafePositionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "mother shahraz melee dps wait at safe position") {}
    bool Execute(Event event) override;
};

class MotherShahrazPositionRangedUnderPillarAction : public MovementAction
{
public:
    MotherShahrazPositionRangedUnderPillarAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "mother shahraz position ranged under pillar") {}
    bool Execute(Event event) override;
};

class MotherShahrazRunAwayToBreakFatalAttractionAction : public MovementAction
{
public:
    MotherShahrazRunAwayToBreakFatalAttractionAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "mother shahraz run away to break fatal attraction") {}
    bool Execute(Event event) override;

private:
    std::vector<Player*> GetAttractedPlayers();
};

// Illidari Council

class IllidariCouncilMisdirectBossesToTanksAction : public AttackAction
{
public:
    IllidariCouncilMisdirectBossesToTanksAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council misdirect bosses to tanks") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMainTankPositionGathiosAction : public AttackAction
{
public:
    IllidariCouncilMainTankPositionGathiosAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council main tank position gathios") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMainTankReflectJudgementOfCommandAction : public Action
{
public:
    IllidariCouncilMainTankReflectJudgementOfCommandAction(
        PlayerbotAI* botAI) : Action(botAI, "illidari council main tank reflect judgement of command") {}
    bool Execute(Event event) override;
};

class IllidariCouncilFirstAssistTankFocusMalandeAction : public AttackAction
{
public:
    IllidariCouncilFirstAssistTankFocusMalandeAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council first assist tank focus malande") {}
    bool Execute(Event event) override;
};

class IllidariCouncilSecondAssistTankPositionDarkshadowAction : public AttackAction
{
public:
    IllidariCouncilSecondAssistTankPositionDarkshadowAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council second assist tank position darkshadow") {}
    bool Execute(Event event) override;
};

class IllidariCouncilMageTankPositionZerevorAction : public AttackAction
{
public:
    IllidariCouncilMageTankPositionZerevorAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council mage tank position zerevor") {}
    bool Execute(Event event) override;
};

class IllidariCouncilPositionMageTankHealerAction : public AttackAction
{
public:
    IllidariCouncilPositionMageTankHealerAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council position mage tank healer") {}
    bool Execute(Event event) override;
};

class IllidariCouncilAssignDpsTargetsAction : public AttackAction
{
public:
    IllidariCouncilAssignDpsTargetsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council assign dps targets") {}
    bool Execute(Event event) override;
};

class IllidariCouncilDisperseRangedAction : public MovementAction
{
public:
    IllidariCouncilDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidari council disperse ranged") {}
    bool Execute(Event event) override;
};

class IllidariCouncilCommandPetsToAttackGathiosAction : public AttackAction
{
public:
    IllidariCouncilCommandPetsToAttackGathiosAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidari council command pets to attack gathios") {}
    bool Execute(Event event) override;
};

class IllidariCouncilManageDpsTimerAction : public Action
{
public:
    IllidariCouncilManageDpsTimerAction(
        PlayerbotAI* botAI) : Action(botAI, "illidari council manage dps timer") {}
    bool Execute(Event event) override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageMisdirectToTankAction : public AttackAction
{
public:
    IllidanStormrageMisdirectToTankAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidan stormrage misdirect to tank") {}
    bool Execute(Event event) override;

private:
    bool TryMisdirectToFlameTanks(Group* group);
    bool TryMisdirectToWarlockTank(Unit* illidan);
};

class IllidanStormrageMainTankRepositionBossAction : public AttackAction
{
public:
    IllidanStormrageMainTankRepositionBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidan stormrage main tank reposition boss") {}
    bool Execute(Event event) override;

private:
    bool MoveToShadowTrap(GameObject* trap);
    Position FindSafestNearbyPosition(
        std::vector<Unit*> const& flameCrashes, float maxRadius, float hazardRadius);
    bool IsPathSafeFromFlameCrashes(Position const& start,
        Position const& end, std::vector<Unit*> const& flameCrashes, float hazardRadius);
};

class IllidanStormrageIsolateBotWithParasiteAction : public MovementAction
{
public:
    IllidanStormrageIsolateBotWithParasiteAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidan stormrage isolate bot with parasite") {}
    bool Execute(Event event) override;

private:
    bool InfectedBotMoveFromGroup(Unit* illidan, Position const& targetPos);
    bool FreezeTrapShadowfiend(Player* bot, Unit* illidan, Position const& targetPos);
};

class IllidanStormrageSetEarthbindTotemAction : public Action
{
public:
    IllidanStormrageSetEarthbindTotemAction(
        PlayerbotAI* botAI) : Action(botAI, "illidan stormrage set earthbind totem") {}
    bool Execute(Event event) override;
};

class IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction : public AttackAction
{
public:
    IllidanStormrageAssistTanksHandleFlamesOfAzzinothAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidan stormrage assist tanks handle flames of azzinoth") {}
    bool Execute(Event event) override;

private:
    bool RepositionToAvoidEyeBlast(Unit* illidan, BlackTempleHelpers::EyeBlastDangerArea const& dangerArea);
    bool RepositionToAvoidBlaze(Unit* eastFlame, Unit* westFlame);
};

class IllidanStormrageControlPetAggressionAction : public Action
{
public:
    IllidanStormrageControlPetAggressionAction(
        PlayerbotAI* botAI) : Action(botAI, "illidan stormrage control pet aggression") {}
    bool Execute(Event event) override;
};

class IllidanStormragePositionAboveGrateAction : public MovementAction
{
public:
    IllidanStormragePositionAboveGrateAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidan stormrage position above grate") {}
    bool Execute(Event event) override;
};

class IllidanStormrageRemoveDarkBarrageAction : public Action
{
public:
    IllidanStormrageRemoveDarkBarrageAction(
        PlayerbotAI* botAI) : Action(botAI, "illidan stormrage remove dark barrage") {}
    bool Execute(Event event) override;
};

class IllidanStormrageMoveAwayFromLandingPointAction : public MovementAction
{
public:
    IllidanStormrageMoveAwayFromLandingPointAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidan stormrage move away from landing point") {}
    bool Execute(Event event) override;
};

class IllidanStormrageDisperseRangedAction : public MovementAction
{
public:
    IllidanStormrageDisperseRangedAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidan stormrage disperse ranged") {}
    bool Execute(Event event) override;

private:
    bool FanOutBehindInHumanPhase(Unit* illidan, Group* group);
    bool SpreadInCircleInDemonPhase(Unit* illidan, Group* group);
};

class IllidanStormrageMeleeGoSomewhereToNotDieAction : public MovementAction
{
public:
    IllidanStormrageMeleeGoSomewhereToNotDieAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidan stormrage melee go somewhere to not die") {}
    bool Execute(Event event) override;
};

class IllidanStormrageWarlockTankHandleDemonBossAction : public AttackAction
{
public:
    IllidanStormrageWarlockTankHandleDemonBossAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidan stormrage warlock tank handle demon boss") {}
    bool Execute(Event event) override;
};

class IllidanStormrageDpsPrioritizeAddsAction : public AttackAction
{
public:
    IllidanStormrageDpsPrioritizeAddsAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "illidan stormrage dps prioritize adds") {}
    bool Execute(Event event) override;
};

class IllidanStormrageUseShadowTrapAction : public MovementAction
{
public:
    IllidanStormrageUseShadowTrapAction(
        PlayerbotAI* botAI) : MovementAction(botAI, "illidan stormrage use shadow trap") {}
    bool Execute(Event event) override;
};

class IllidanStormrageManageDpsTimerAndRtiAction : public Action
{
public:
    IllidanStormrageManageDpsTimerAndRtiAction(
        PlayerbotAI* botAI) : Action(botAI, "illidan stormrage manage dps timer and rti") {}
    bool Execute(Event event) override;
};

class IllidanStormrageDestroyHazardsAction : public Action
{
public:
    IllidanStormrageDestroyHazardsAction(
        PlayerbotAI* botAI) : Action(botAI, "illidan stormrage destroy hazards") {}
    bool Execute(Event event) override;
};

class IllidanStormrageHandleAddsCheatAction : public Action
{
public:
    IllidanStormrageHandleAddsCheatAction(
        PlayerbotAI* botAI) : Action(botAI, "illidan stormrage handle adds cheat") {}
    bool Execute(Event event) override;
};

#endif
