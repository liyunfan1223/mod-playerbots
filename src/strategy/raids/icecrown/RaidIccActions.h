#ifndef _PLAYERBOT_RAIDICCACTIONS_H
#define _PLAYERBOT_RAIDICCACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AttackAction.h"
#include "LastMovementValue.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "RaidIccStrategy.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Vehicle.h"
#include "RaidIccTriggers.h"

const Position ICC_LM_TANK_POSITION = Position(-391.0f, 2259.0f, 42.0f);
const Position ICC_DARK_RECKONING_SAFE_POSITION = Position(-523.33386f, 2211.2031f, 62.823116f);
const Position ICC_LDW_TANK_POSTION = Position(-570.1f, 2211.2456f, 49.476616f); //-590.0f
const Position ICC_ROTTING_FROST_GIANT_TANK_POSITION = Position(-328.5085f, 2225.5142f, 199.97298f);
const Position ICC_GUNSHIP_TELEPORT_ALLY = Position (-370.04645f, 1993.3536f, 466.65656f);
const Position ICC_GUNSHIP_TELEPORT_ALLY2 = Position (-392.66208f, 2064.893f, 466.5672f, 5.058196f);
const Position ICC_GUNSHIP_TELEPORT_HORDE = Position (-449.5343f, 2477.2024f, 470.17648f);
const Position ICC_GUNSHIP_TELEPORT_HORDE2 = Position (-429.81586f, 2400.6804f, 471.56537f);
const Position ICC_DBS_TANK_POSITION = Position(-494.26517f, 2211.549f, 541.11414f);
const Position ICC_FESTERGUT_TANK_POSITION = Position(4269.1772f, 3144.7673f, 360.38577f);
const Position ICC_FESTERGUT_RANGED_SPORE = Position(4261.143f, 3109.4146f, 360.38605f);
const Position ICC_FESTERGUT_MELEE_SPORE = Position(4269.1772f, 3144.7673f, 360.38577f);
const Position ICC_ROTFACE_TANK_POSITION = Position(4447.061f, 3150.9758f, 360.38568f);
const Position ICC_ROTFACE_BIG_OOZE_POSITION = Position(4432.687f, 3142.3035f, 360.38623f);
const Position ICC_ROTFACE_SAFE_POSITION = Position(4446.557f, 3065.6594f, 360.51843f);
const Position ICC_ROTFACE_CENTER_POSITION = Position(4446.0547f, 3144.8677f, 360.38593f); //actual center 4.74089 4445.6616f, 3137.1526f, 360.38608
const Position ICC_PUTRICIDE_TANK_POSITION = Position(4373.227f, 3222.058f, 389.4029f);
const Position ICC_PUTRICIDE_GREEN_POSITION = Position(4423.4126f, 3194.2715f, 389.37683f);
const Position ICC_PUTRICIDE_BAD_POSITION = Position(4356.1724f, 3261.5232f, 389.3985f);
//const Position ICC_PUTRICIDE_GAS3_POSITION = Position(4367.753f, 3177.5894f, 389.39575f);
//const Position ICC_PUTRICIDE_GAS4_POSITION = Position(4321.8486f, 3206.464f, 389.3982f);
const Position ICC_BPC_OT_POSITION = Position(4649.2236f, 2796.0972f, 361.1815f);
const Position ICC_BPC_MT_POSITION = Position(4648.5674f, 2744.847f, 361.18222f);
const Position ICC_BQL_CENTER_POSITION = Position(4595.0f, 2769.0f, 400.0f);
const Position ICC_BQL_LWALL1_POSITION = Position(4624.685f, 2789.4895f, 400.13834f);
const Position ICC_BQL_LWALL2_POSITION = Position(4600.749f, 2805.7568f, 400.1374f);
const Position ICC_BQL_LWALL3_POSITION = Position(4572.857f, 2797.3872f, 400.1374f);
const Position ICC_BQL_RWALL1_POSITION = Position(4625.724f, 2748.9917f, 400.13693f);
const Position ICC_BQL_RWALL2_POSITION = Position(4608.3774f, 2735.7466f, 400.13693f);
const Position ICC_BQL_RWALL3_POSITION = Position(4576.813f, 2739.6067f, 400.13693f);
const Position ICC_BQL_LRWALL4_POSITION = Position(4539.345f, 2769.3853f, 403.7267f);
const Position ICC_BQL_TANK_POSITION = Position(4629.746f, 2769.6396f, 401.7479f); //old just in front of stairs 4616.102f, 2768.9167f, 400.13797f
const Position ICC_VDW_HEAL_POSITION = Position(4203.752f, 2483.4343f, 364.87274f);
const Position ICC_VDW_GROUP1_POSITION = Position(4203.585f, 2464.422f, 364.86887f);
const Position ICC_VDW_GROUP2_POSITION = Position(4203.5806f, 2505.2383f, 364.87677f);
const Position ICC_VDW_PORTALSTART_POSITION = Position(4202.637f, 2488.171f, 375.00256f);
const Position ICC_SINDRAGOSA_TANK_POSITION = Position(4408.016f, 2508.0647f, 203.37955f);
const Position ICC_SINDRAGOSA_FLYING_POSITION = Position(4525.6f, 2485.15f, 245.082f);
const Position ICC_SINDRAGOSA_RANGED_POSITION = Position(4441.572f, 2484.482f, 203.37836f);
const Position ICC_SINDRAGOSA_MELEE_POSITION = Position(4423.4546f, 2491.7175f, 203.37686f);
const Position ICC_SINDRAGOSA_BLISTERING_COLD_POSITION = Position(4473.6616f, 2484.8489f, 203.38258f);
const Position ICC_SINDRAGOSA_THOMB1_POSITION = Position(4433.6484f, 2469.4133f, 203.3806f);
const Position ICC_SINDRAGOSA_THOMB2_POSITION = Position(4434.143f, 2486.201f, 203.37473f);
const Position ICC_SINDRAGOSA_THOMB3_POSITION = Position(4436.1147f, 2501.464f, 203.38266f);
const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC1_POSITION = Position(4444.9707f, 2455.7322f, 203.38701f);
const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC2_POSITION = Position(4461.3945f, 2463.5513f, 203.38727f);
const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC3_POSITION = Position(4473.6616f, 2484.8489f, 203.38258f);
const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC4_POSITION = Position(4459.9336f, 2507.409f, 203.38606f);
const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC5_POSITION = Position(4442.3096f, 2512.4688f, 203.38647f);
const Position ICC_SINDRAGOSA_CENTER_POSITION = Position(4408.0464f, 2484.478f, 203.37529f);           
const Position ICC_SINDRAGOSA_THOMBMB2_POSITION = Position(4436.895f, 2498.1401f, 203.38133f);
const Position ICC_SINDRAGOSA_FBOMB_POSITION = Position(4449.3647f, 2486.4524f, 203.379f);
const Position ICC_SINDRAGOSA_FBOMB10_POSITION = Position(4449.3647f, 2486.4524f, 203.379f);
const Position ICC_SINDRAGOSA_LOS2_POSITION = Position(4441.8286f, 2501.946f, 203.38435f);
const Position ICC_LICH_KING_ADDS_POSITION = Position(476.7332f, -2095.3894f, 840.857f);  // old 486.63647f, -2095.7915f, 840.857f
const Position ICC_LICH_KING_MELEE_POSITION = Position(503.5546f, -2106.8213f, 840.857f);
const Position ICC_LICH_KING_RANGED_POSITION = Position(501.3563f, -2085.1816f, 840.857f);
const Position ICC_LICH_KING_ASSISTHC_POSITION = Position(517.2145f, -2125.0674f, 840.857f);
const Position ICC_LK_FROST1_POSITION = Position(503.96548f, -2183.216f, 840.857f);           
const Position ICC_LK_FROST2_POSITION = Position(563.07166f, -2125.7578f, 840.857f);           
const Position ICC_LK_FROST3_POSITION = Position(503.40182f, -2067.3435f, 840.857f); 
const Position ICC_LK_FROSTR1_POSITION = Position(481.168f, -2177.8723f, 840.857f); 
const Position ICC_LK_FROSTR2_POSITION = Position(562.20807f, -2100.2393f, 840.857f); 
const Position ICC_LK_FROSTR3_POSITION = Position(526.35297f, -2071.0317f, 840.857f);

//Lord Marrogwar
class IccLmTankPositionAction : public AttackAction
{
public:
    IccLmTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc lm tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool MoveTowardPosition(const Position& position, float incrementSize);
};

class IccSpikeAction : public AttackAction 
{
public:
    IccSpikeAction(PlayerbotAI* botAI) : AttackAction(botAI, "icc spike") {}
    bool Execute(Event event) override;

    bool HandleSpikeTargeting(Unit* boss);
    bool MoveTowardPosition(const Position& position, float incrementSize);
    void UpdateRaidTargetIcon(Unit* target);
};

//Lady Deathwhisper
class IccDarkReckoningAction : public MovementAction
{
public:
    IccDarkReckoningAction(PlayerbotAI* botAI, std::string const name = "icc dark reckoning")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccRangedPositionLadyDeathwhisperAction : public AttackAction
{
public:
    IccRangedPositionLadyDeathwhisperAction(PlayerbotAI* botAI, std::string const name = "icc ranged position lady deathwhisper")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool MaintainRangedSpacing();
};

class IccAddsLadyDeathwhisperAction : public AttackAction
{
public:
    IccAddsLadyDeathwhisperAction(PlayerbotAI* botAI, std::string const name = "icc adds lady deathwhisper")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool IsTargetedByShade(uint32 shadeEntry);
    bool MoveTowardPosition(const Position& position, float incrementSize);
    bool HandleAddTargeting(Unit* boss);
    void UpdateRaidTargetIcon(Unit* target);

};

class IccShadeLadyDeathwhisperAction : public MovementAction
{
public:
    IccShadeLadyDeathwhisperAction(PlayerbotAI* botAI, std::string const name = "icc shade lady deathwhisper")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

//Gunship Battle
class IccRottingFrostGiantTankPositionAction : public AttackAction
{
public:
    IccRottingFrostGiantTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc rotting frost giant tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccCannonFireAction : public Action
{
public:
    IccCannonFireAction(PlayerbotAI* botAI, std::string const name = "icc cannon fire")
        : Action(botAI, name) {}
    bool Execute(Event event) override;

    Unit* FindValidCannonTarget();
    bool TryCastCannonSpell(uint32 spellId, Unit* target, Unit* vehicleBase);
};

class IccGunshipEnterCannonAction : public MovementAction
{
public:
    IccGunshipEnterCannonAction(PlayerbotAI* botAI, std::string const name = "icc gunship enter cannon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
    Unit* FindBestAvailableCannon();
    bool IsValidCannon(Unit* vehicle, const uint32 validEntries[]);
};

class IccGunshipTeleportAllyAction : public AttackAction
{
public:
    IccGunshipTeleportAllyAction(PlayerbotAI* botAI, std::string const name = "icc gunship teleport ally")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool TeleportTo(const Position& position);
    void CleanupSkullIcon(uint8_t SKULL_ICON_INDEX);
    void UpdateBossSkullIcon(Unit* boss, uint8_t SKULL_ICON_INDEX);
};

class IccGunshipTeleportHordeAction : public AttackAction
{
public:
    IccGunshipTeleportHordeAction(PlayerbotAI* botAI, std::string const name = "icc gunship teleport horde")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool TeleportTo(const Position& position);
    void CleanupSkullIcon(uint8_t SKULL_ICON_INDEX);
    void UpdateBossSkullIcon(Unit* boss, uint8_t SKULL_ICON_INDEX);
};

//DBS
class IccDbsTankPositionAction : public AttackAction
{
public:
    IccDbsTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc dbs tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool CrowdControlBloodBeasts();
    bool EvadeBloodBeasts();
    bool PositionInRangedFormation();
};

class IccAddsDbsAction : public AttackAction
{
public:
    IccAddsDbsAction(PlayerbotAI* botAI, std::string const name = "icc adds dbs")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    Unit* FindPriorityTarget(Unit* boss);
    void UpdateSkullMarker(Unit* priorityTarget);
};

//FESTERGUT
class IccFestergutGroupPositionAction : public AttackAction
{
public:
    IccFestergutGroupPositionAction(PlayerbotAI* botAI, std::string const name = "icc festergut group position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool HasSporesInGroup();
    bool PositionNonTankMembers();
    int CalculatePositionIndex(Group* group);
};

class IccFestergutSporeAction : public AttackAction
{
public:
    IccFestergutSporeAction(PlayerbotAI* botAI, std::string const name = "icc festergut spore")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    Position CalculateSpreadPosition();
    struct SporeInfo
    {
        std::vector<Unit*> sporedPlayers;
        ObjectGuid lowestGuid;
        bool hasLowestGuid = false;
    };
    SporeInfo FindSporedPlayers();
    Position DetermineTargetPosition(bool hasSpore, const SporeInfo& sporeInfo, const Position& spreadRangedPos);
    bool CheckMainTankSpore();
};

//Rotface
class IccRotfaceTankPositionAction : public AttackAction
{
public:
    IccRotfaceTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc rotface tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    void MarkBossWithSkull(Unit* boss);
    bool PositionMainTankAndMelee(Unit *boss);
    bool HandleAssistTankPositioning(Unit* boss);
    bool HandleBigOozePositioning(Unit* boss);
};

class IccRotfaceGroupPositionAction : public AttackAction
{
public:
    IccRotfaceGroupPositionAction(PlayerbotAI* botAI, std::string const name = "icc rotface group position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    //bool MoveAwayFromBigOoze(Unit* bigOoze);
    bool HandlePuddleAvoidance(Unit* boss);
    bool MoveAwayFromPuddle(Unit* boss, Unit* puddle, float puddleDistance);
    bool HandleOozeTargeting();
    bool HandleOozeMemberPositioning();
    bool PositionRangedAndHealers(Unit* boss,Unit* smallOoze);
    bool FindAndMoveFromClosestMember(Unit* boss, Unit* smallOoze);
};

class IccRotfaceMoveAwayFromExplosionAction : public MovementAction 
{
public:
    IccRotfaceMoveAwayFromExplosionAction(PlayerbotAI* botAI, std::string const name = "icc rotface move away from explosion") 
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

    bool MoveToRandomSafeLocation();

};

//PP
class IccPutricideGrowingOozePuddleAction : public AttackAction
{
public:
    IccPutricideGrowingOozePuddleAction(PlayerbotAI* botAI, std::string const name = "icc putricide growing ooze puddle")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    Unit* FindClosestThreateningPuddle();
    Position CalculateSafeMovePosition(Unit* closestPuddle);
    bool IsPositionTooCloseToOtherPuddles(float x, float y, Unit* ignorePuddle);


};

class IccPutricideVolatileOozeAction : public AttackAction
{
public:
    IccPutricideVolatileOozeAction(PlayerbotAI* botAI, std::string const name = "icc putricide volatile ooze")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    void MarkOozeWithSkull(Unit* ooze);
    Unit* FindAuraTarget();
};

class IccPutricideGasCloudAction : public AttackAction
{
public:
    IccPutricideGasCloudAction(PlayerbotAI* botAI, std::string const name = "icc putricide gas cloud")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool HandleGaseousBloatMovement(Unit* gasCloud);
    Position CalculateEmergencyPosition(const Position& botPos, float dx, float dy);
    bool FindSafeMovementPosition(const Position& botPos, const Position& cloudPos, float dx, float dy, int numAngles,
                                  Position& resultPos);
    bool HandleGroupAuraSituation(Unit* gasCloud);
    bool GroupHasGaseousBloat(Group* group);
};

class IccPutricideAvoidMalleableGooAction : public MovementAction
{
public:
    IccPutricideAvoidMalleableGooAction(PlayerbotAI* botAI, std::string const name = "icc putricide avoid malleable goo")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

    bool HandleTankPositioning(Unit* boss);
    bool HandleUnboundPlague(Unit* boss);
    bool HandleBossPositioning(Unit* boss);
    Position CalculateBossPosition(Unit* boss, float distance);
    bool HasObstacleBetween(const Position& from, const Position& to);
    bool IsOnPath(const Position& from, const Position& to, const Position& point, float threshold);
    Position CalculateArcPoint(const Position& current, const Position& target, const Position& center);
    Position CalculateIncrementalMove(const Position& current, const Position& target, float maxDistance);
};

//BPC
class IccBpcKelesethTankAction : public AttackAction
{
public:
    IccBpcKelesethTankAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bpc keleseth tank") {}
    bool Execute(Event event) override;
};

class IccBpcMainTankAction : public AttackAction
{
public:
    IccBpcMainTankAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bpc main tank") {}
    bool Execute(Event event) override;

    void MarkEmpoweredPrince();
};

class IccBpcEmpoweredVortexAction : public MovementAction
{
public:
    IccBpcEmpoweredVortexAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "icc bpc empowered vortex") {}
    bool Execute(Event event) override;

    bool MaintainRangedSpacing();
    bool HandleEmpoweredVortexSpread();
};

class IccBpcKineticBombAction : public AttackAction
{
public:
    IccBpcKineticBombAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bpc kinetic bomb") {}
    bool Execute(Event event) override;

    Unit* FindOptimalKineticBomb();
    bool IsBombAlreadyHandled(Unit* bomb, Group* group);
};

class IccBpcBallOfFlameAction : public MovementAction
{
public:
    IccBpcBallOfFlameAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "icc bpc ball of flame") {}
    bool Execute(Event event) override;
};

//Blood Queen Lana'thel
class IccBqlGroupPositionAction : public AttackAction
{
public:
    IccBqlGroupPositionAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc group tank position") {}
    bool Execute(Event event) override;

    bool HandleTankPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura);
    bool HandleShadowsMovement();
    Position AdjustControlPoint(const Position& wall, const Position& center, float factor);
    Position CalculateBezierPoint(float t, const Position path[4]);
    bool HandleGroupPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura);

private:
    // Evaluate curves
    struct CurveInfo
    {
        Position moveTarget;
        int curveIdx = 0;
        bool foundSafe = false;
        float minDist = 0.0f;
        float score = 0.0f;  
        Position closestPoint;
        float t_closest = 0.0f;
    };
};  

class IccBqlPactOfDarkfallenAction : public MovementAction
{
public:
    IccBqlPactOfDarkfallenAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "icc bql pact of darkfallen") {}
    bool Execute(Event event) override;

    void CalculateCenterPosition(Position& targetPos, const std::vector<Player*>& playersWithAura);
    bool MoveToTargetPosition(const Position& targetPos, int auraCount);
};

class IccBqlVampiricBiteAction : public AttackAction
{
public:
    IccBqlVampiricBiteAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc bql vampiric bite") {}
    bool Execute(Event event) override;

    Player* FindBestBiteTarget(Group* group);
    bool IsInvalidTarget(Player* player);
    bool MoveTowardsTarget(Player* target);
    bool CastVampiricBite(Player* target);
};

// Sister Svalna
class IccValkyreSpearAction : public AttackAction
{
public:
    IccValkyreSpearAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc valkyre spear") {}
    bool Execute(Event event) override;
};
    
class IccSisterSvalnaAction : public AttackAction
{
public:
    IccSisterSvalnaAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc sister svalna") {}
    bool Execute(Event event) override;
};

// Valithria Dreamwalker

class IccValithriaGroupAction : public AttackAction
{
public:
    IccValithriaGroupAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "icc valithria group") {}
    bool Execute(Event event) override;

    bool MoveTowardsPosition(const Position& pos, float increment);
    bool Handle25ManGroupLogic();
    bool HandleMarkingLogic(bool inGroup1, bool inGroup2, const Position& group1Pos, const Position& group2Pos);
    bool Handle10ManGroupLogic();
};

class IccValithriaPortalAction : public MovementAction
{
public:
    IccValithriaPortalAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc valithria portal") {}
    bool Execute(Event event) override;
};

class IccValithriaHealAction : public AttackAction
{
public:
    IccValithriaHealAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc valithria heal") {}
    bool Execute(Event event) override;
};

class IccValithriaDreamCloudAction : public MovementAction
{
public:
    IccValithriaDreamCloudAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc valithria dream cloud") {}
    bool Execute(Event event) override;
};

//Sindragosa
class IccSindragosaGroupPositionAction : public AttackAction
{
public:
    IccSindragosaGroupPositionAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc sindragosa group position") {}
    bool Execute(Event event) override;

    bool HandleTankPositioning(Unit* boss);
    bool HandleNonTankPositioning();
    bool MoveIncrementallyToPosition(const Position& targetPos, float maxStep);
};

class IccSindragosaFrostBeaconAction : public MovementAction
{
public:
    IccSindragosaFrostBeaconAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc sindragosa frost beacon") {}
    bool Execute(Event event) override;

    void HandleSupportActions();
    bool HandleBeaconedPlayer(const Unit* boss);
    bool HandleNonBeaconedPlayer(const Unit* boss);
    bool MoveToPositionIfNeeded(const Position& position, float tolerance);
    bool MoveToPosition(const Position& position);
    bool IsBossFlying(const Unit* boss);

    private:
    static constexpr uint32 FROST_BEACON_AURA_ID = SPELL_FROST_BEACON;
    static constexpr uint32 HAND_OF_FREEDOM_SPELL_ID = 1044;
    static constexpr float POSITION_TOLERANCE = 1.0f;
    static constexpr float TOMB_POSITION_TOLERANCE = 0.5f;
    static constexpr float MIN_SAFE_DISTANCE = 13.0f;
    static constexpr float MOVE_TOLERANCE = 2.0f;
};

class IccSindragosaBlisteringColdAction : public MovementAction
{
public:
    IccSindragosaBlisteringColdAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc sindragosa blistering cold") {}
    bool Execute(Event event) override;
};

class IccSindragosaUnchainedMagicAction : public AttackAction
{
public:
    IccSindragosaUnchainedMagicAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc sindragosa unchained magic") {}
    bool Execute(Event event) override;
};

class IccSindragosaChilledToTheBoneAction : public AttackAction
{
public:
    IccSindragosaChilledToTheBoneAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc sindragosa chilled to the bone") {}
    bool Execute(Event event) override;
}; 

class IccSindragosaMysticBuffetAction : public MovementAction
{
public:
    IccSindragosaMysticBuffetAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc sindragosa mystic buffet") {}
    bool Execute(Event event) override;
};

class IccSindragosaFrostBombAction : public MovementAction
{
public:
    IccSindragosaFrostBombAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc sindragosa frost bomb") {}
    bool Execute(Event event) override;
};

class IccSindragosaTankSwapPositionAction : public AttackAction 
{
    public:
        IccSindragosaTankSwapPositionAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "sindragosa tank swap position") {}
        bool Execute(Event event) override;
};


//LK
class IccLichKingShadowTrapAction : public MovementAction
{
    public:
        IccLichKingShadowTrapAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc lich king shadow trap") {}
        bool Execute(Event event) override;
};

class IccLichKingNecroticPlagueAction : public MovementAction
{
    public:
        IccLichKingNecroticPlagueAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc lich king necrotic plague") {}
        bool Execute(Event event) override;
};

class IccLichKingWinterAction : public AttackAction
{
    public:
        IccLichKingWinterAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc lich king winter") {}
        bool Execute(Event event) override;

        void HandlePositionCorrection();
        bool IsValidCollectibleAdd(Unit* unit);
        bool IsPositionSafeFromDefile(float x, float y, float z, float minSafeDistance);
        void HandleTankPositioning();
        void HandleMeleePositioning();
        void HandleRangedPositioning();
        void HandleMainTankAddManagement(Unit* boss, const Position* tankPos);
        void HandleAssistTankAddManagement(Unit* boss, const Position* tankPos);


    private:
        const Position* GetMainTankPosition();
        const Position* GetMainTankRangedPosition();
        bool TryMoveToPosition(float targetX, float targetY, float targetZ, bool isForced = true);
};

class IccLichKingAddsAction : public AttackAction
{
    public:
        IccLichKingAddsAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc lich king adds") {}
        bool Execute(Event event) override;

        void HandleTeleportationFixes(Difficulty diff, Unit* terenasMenethilHC);
        bool HandleSpiritBombAvoidance(Difficulty diff, Unit* terenasMenethilHC);
        void HandleHeroicNonTankPositioning(Difficulty diff, Unit* terenasMenethilHC);
        void HandleSpiritMarkingAndTargeting(Difficulty diff, Unit* terenasMenethilHC);
        bool HandleQuakeMechanics(Unit* boss);
        void HandleShamblingHorrors(Unit* boss, bool hasPlague);
        bool HandleAssistTankAddManagement(Unit* boss, Difficulty diff);
        void HandleMeleePositioning(Unit* boss, bool hasPlague, Difficulty diff);
        void HandleMainTankTargeting(Unit* boss, Difficulty diff);
        void HandleNonTankHeroicPositioning(Unit* boss, Difficulty diff, bool hasPlague);
        void HandleRangedPositioning(Unit* boss, bool hasPlague, Difficulty diff);
        void HandleDefileMechanics(Unit* boss, Difficulty diff);
        void HandleValkyrMechanics(Difficulty diff);
        std::vector<size_t> CalculateBalancedGroupSizes(size_t totalAssist, size_t numValkyrs);
        size_t GetAssignedValkyrIndex(size_t assistIndex, const std::vector<size_t>& groupSizes);
        std::string GetRTIValueForValkyr(size_t valkyrIndex);
        void HandleValkyrMarking(const std::vector<Unit*>& grabbingValkyrs, Difficulty diff);
        void HandleValkyrAssignment(const std::vector<Unit*>& grabbingValkyrs);
        void ApplyCCToValkyr(Unit* valkyr);
        bool IsValkyr(Unit* unit);
        void HandleVileSpiritMechanics();
};



#endif
