/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCACTIONS_H
#define PLAYERBOTS_ICCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ICCScripts.h"
#include "ICCShared.h"
#include "ICCStrategy.h"
#include "ICCTriggers.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <set>

inline const Position ICC_LM_TANK_POSITION = Position(-391.0f, 2259.0f, 42.0f);
inline const Position ICC_LM_BONE_STORM_AT_POSITION = Position(-390.02332f, 2179.3481f, 41.96729f);
inline const Position ICC_LM_MID_POSITION = Position(-393.61722f, 2216.335f, 41.99396f);
inline const Position ICC_DARK_RECKONING_SAFE_POSITION = Position(-523.33386f, 2211.2031f, 62.823116f);
inline const Position ICC_LDW_TANK_POSTION = Position(-593.7436f, 2211.298f, 49.476673f); //old closer to stairs -570.1f, 2211.2456f, 49.476616f
inline const Position ICC_LDW_RANGED_POSITION = Position(-653.11096f, 2211.9568f,51.551437f); //old in frint of the boss (-607.0631f, 2212.2488f, 49.47009f)
inline const Position ICC_ROTTING_FROST_GIANT_TANK_POSITION = Position(-328.5085f, 2225.5142f, 199.97298f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_ALLY = Position (-370.04645f, 1993.3536f, 466.65656f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_ALLY2 = Position (-392.66208f, 2064.893f, 466.5672f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_ALLY_MIDDLE_POINT = Position(-383.01166f, 2022.8046f, 467.38193f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_ALLY_FRIENDLY_POINT = Position(-389.26788f, 2045.9136f, 467.56168f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_HORDE = Position (-449.5343f, 2477.2024f, 471.31906f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_HORDE2 = Position (-429.81586f, 2400.6804f, 471.56537f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_HORDE_MIDDLE_POINT = Position(-449.17645f, 2449.2705f, 470.9257f);
inline const Position ICC_GUNSHIP_ROCKET_JUMP_HORDE_FRIENDLY_POINT = Position(-444.74533f, 2420.1208f, 470.78748f);
inline const Position ICC_DBS_TANK_POSITION = Position(-494.26517f, 2211.549f, 541.11414f);
inline const Position ICC_FESTERGUT_TANK_POSITION = Position(4269.1772f, 3144.7673f, 360.38577f);
inline const Position ICC_FESTERGUT_RANGED_SPORE = Position(4264.3623f, 3120.889f, 360.38565f); //old closer to gates 4261.143f, 3109.4146f, 360.38605f
inline const Position ICC_FESTERGUT_RANGED_SPORE_2 = Position(4288.7974f, 3115.6274f, 360.38577f); //new position
inline const Position ICC_FESTERGUT_MELEE_SPORE = Position(4269.1772f, 3144.7673f, 360.38577f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_1 = Position(4453.2085f, 3108.7488f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_2 = Position(4459.4390f, 3118.0210f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_3 = Position(4473.4062f, 3126.2861f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_4 = Position(4469.9370f, 3137.0173f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_5 = Position(4476.7583f, 3145.7160f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_6 = Position(4466.2100f, 3150.0630f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_7 = Position(4464.4297f, 3161.2317f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_8 = Position(4447.2900f, 3163.0322f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_9 = Position(4437.1250f, 3168.4062f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_10 = Position(4417.2217f, 3148.8909f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_11 = Position(4420.9727f, 3137.7700f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_12 = Position(4414.8716f, 3127.9534f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_13 = Position(4425.1553f, 3123.4717f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_14 = Position(4426.3760f, 3113.4153f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_15 = Position(4443.8027f, 3113.9207f, 360.38626f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_16 = Position(4432.477f, 3156.7651f, 360.38568f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_17 = Position(4458.083f, 3132.5842f, 360.38565f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_18 = Position(4457.4565f, 3144.8442f, 360.38565f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_19 = Position(4422.5460f, 3158.0435f, 360.38565f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_20 = Position(4432.1646f, 3142.3418f, 360.38565f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_21 = Position(4436.649f, 3126.1245f, 360.38565f);
inline const Position ICC_ROTFACE_RANGED_POSITION_HC_22 = Position(4450.2363f, 3122.4033f, 360.38565f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_1 = Position(4481.433f, 3137.0117f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_2 = Position(4474.4565f, 3156.9617f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_3 = Position(4452.0586f, 3170.2231f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_4 = Position(4421.2437f, 3161.2275f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_5 = Position(4410.904f, 3136.9976f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_6 = Position(4417.603f, 3119.4712f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_7 = Position(4442.514f, 3105.2234f, 360.38522f);
inline const Position ICC_ROTFACE_EXPLOSION_POSITION_8 = Position(4465.7173f, 3113.6284f, 360.38522f);
inline const Position ICC_ROTFACE_TANK_POSITION = Position(4447.061f, 3150.9758f, 360.38568f);
inline const Position ICC_ROTFACE_BIG_OOZE_POSITION = Position(4432.687f, 3142.3035f, 360.38568f);
inline const Position ICC_ROTFACE_SAFE_POSITION = Position(4446.557f, 3065.6594f, 360.38568f);
inline const Position ICC_ROTFACE_CENTER_POSITION = Position(4446.0547f, 3144.8677f, 360.38568f); //actual center 4.74089 4445.6616f, 3137.1526f, 360.38608
inline const Position ICC_ROTFACE_CENTER_POSITION_BOSS = Position(4445.656f, 3137.1663f, 360.38565f);
inline const Position ICC_PUTRICIDE_TANK_POSITION = Position(4373.227f, 3222.058f, 389.4029f);
inline const Position ICC_PUTRICIDE_GREEN_POSITION = Position(4423.4126f, 3194.2715f, 389.37683f);
inline const Position ICC_PUTRICIDE_GATE_POSITION = Position(4356.3345f, 3167.9407f, 389.39825f);
inline const Position ICC_BPC_OT_POSITION = Position(4649.2236f, 2796.0972f, 361.1815f);
inline const Position ICC_BPC_MT_POSITION = Position(4648.5674f, 2744.847f, 361.18222f);
inline const Position ICC_BPC_CENTER_POSITION = Position(4638.7056f, 2769.3713f, 361.17108f);
inline const Position ICC_BQL_CENTER_POSITION = Position(4595.0f, 2769.0f, 400.0f);
inline const Position ICC_BQL_LWALL1_POSITION = Position(4624.685f, 2789.4895f, 400.13834f);
inline const Position ICC_BQL_LWALL2_POSITION = Position(4600.749f, 2805.7568f, 400.1374f);
inline const Position ICC_BQL_LWALL3_POSITION = Position(4572.857f, 2797.3872f, 400.1374f);
inline const Position ICC_BQL_RWALL1_POSITION = Position(4625.724f, 2748.9917f, 400.13693f);
inline const Position ICC_BQL_RWALL2_POSITION = Position(4608.3774f, 2735.7466f, 400.13693f);
inline const Position ICC_BQL_RWALL3_POSITION = Position(4576.813f, 2739.6067f, 400.13693f);
inline const Position ICC_BQL_LRWALL4_POSITION = Position(4539.345f, 2769.3853f, 403.7267f);
inline const Position ICC_BQL_TANK_POSITION = Position(4633.7964f, 2769.2515f, 401.74777f); //old 4629.746f, 2769.6396f, 401.7479f
inline const Position ICC_VDW_HEAL_POSITION = Position(4203.752f, 2483.4343f, 364.87274f);
inline const Position ICC_VDW_GROUP1_POSITION = Position(4203.585f, 2464.422f, 364.87323f);
inline const Position ICC_VDW_GROUP2_POSITION = Position(4203.5806f, 2505.2383f, 364.87323f);
inline const Position ICC_VDW_PORTALSTART_POSITION = Position(4202.637f, 2488.171f, 375.00256f);
inline const Position ICC_SINDRAGOSA_TANK_POSITION = Position(4408.016f, 2508.0647f, 203.37955f); //X: 4183.136 Y: 2492.9358 Z: 364.87595 Orientation: 5.3975997
inline const Position ICC_SINDRAGOSA_FLYING_POSITION = Position(4525.6f, 2485.15f, 245.082f);
inline const Position ICC_SINDRAGOSA_RANGED_POSITION = Position(4441.572f, 2484.482f, 203.37836f);
inline const Position ICC_SINDRAGOSA_MELEE_POSITION = Position(4423.1646f, 2486.6792f, 203.3749f); //old 4423.4546f, 2491.7175f, 203.37686f
inline const Position ICC_SINDRAGOSA_BLISTERING_COLD_POSITION = Position(4473.6616f, 2484.8489f, 203.38258f);
inline const Position ICC_SINDRAGOSA_THOMB1_POSITION = Position(4433.6484f, 2469.4133f, 203.3806f);
inline const Position ICC_SINDRAGOSA_THOMB2_POSITION = Position(4434.143f, 2486.201f, 203.37473f);
inline const Position ICC_SINDRAGOSA_THOMB3_POSITION = Position(4436.1147f, 2501.464f, 203.38266f);
inline const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC1_POSITION = Position(4444.9707f, 2455.7322f, 203.38701f);
inline const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC2_POSITION = Position(4461.3945f, 2463.5513f, 203.38727f);
inline const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC3_POSITION = Position(4473.6616f, 2484.8489f, 203.38258f);
inline const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC4_POSITION = Position(4459.9336f, 2507.409f, 203.38606f);
inline const Position ICC_SINDRAGOSA_UNCHAINEDMAGIC5_POSITION = Position(4442.3096f, 2512.4688f, 203.38647f);
inline const Position ICC_SINDRAGOSA_CENTER_POSITION = Position(4408.0464f, 2484.478f, 203.37529f);
inline const Position ICC_SINDRAGOSA_THOMBMB2_POSITION = Position(4436.895f, 2498.1401f, 203.38133f);
inline const Position ICC_SINDRAGOSA_FBOMB_POSITION = Position(4453.861f, 2485.524f, 203.3787f);
inline const Position ICC_SINDRAGOSA_FBOMB10_POSITION = Position(4453.861f, 2485.524f, 203.3787f);
inline const Position ICC_SINDRAGOSA_LOS2_POSITION = Position(4441.8286f, 2501.946f, 203.38435f);
inline const Position ICC_LICH_KING_ADDS_POSITION = Position(476.7332f, -2095.3894f, 840.857f);  // old 486.63647f, -2095.7915f, 840.857f
inline const Position ICC_LICH_KING_MELEE_POSITION = Position(503.5546f, -2106.8213f, 840.857f);
inline const Position ICC_LICH_KING_RANGED_POSITION = Position(501.3563f, -2085.1816f, 840.857f);
inline const Position ICC_LICH_KING_ASSISTHC_POSITION = Position(517.2145f, -2125.0674f, 840.857f);
inline const Position ICC_LICH_KING_CENTER_POSITION = Position(503.62036f, -2124.7336f, 840.857f);
inline const Position ICC_LK_FROST1_POSITION = Position(503.96548f, -2183.216f, 840.857f);
inline const Position ICC_LK_FROST2_POSITION = Position(563.07166f, -2125.7578f, 840.857f);
inline const Position ICC_LK_FROST3_POSITION = Position(503.40182f, -2067.3435f, 840.857f);
inline const Position ICC_LK_FROSTR1_POSITION = Position(481.168f, -2177.8723f, 840.857f);
inline const Position ICC_LK_FROSTR2_POSITION = Position(562.20807f, -2100.2393f, 840.857f);
inline const Position ICC_LK_FROSTR3_POSITION = Position(526.35297f, -2071.0317f, 840.857f);
inline const Position ICC_LK_VILE_SPIRIT1_POSITION = Position(505.24002f, -2086.7778f, 840.857f);
inline const Position ICC_LK_VILE_SPIRIT2_POSITION = Position(532.668f, -2122.603f, 840.857f);
inline const Position ICC_LK_VILE_SPIRIT3_POSITION = Position(502.8796f, -2159.7466f, 840.857f);

//Lord Marrogwar
class IccLmTankPositionAction : public AttackAction
{
public:
    IccLmTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc lm tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool MoveTowardPosition(Position const& position, float incrementSize);
};

class IccSpikeAction : public AttackAction
{
public:
    IccSpikeAction(PlayerbotAI* botAI) : AttackAction(botAI, "icc spike") {}
    bool Execute(Event event) override;

    std::vector<Unit*> FindAliveSpikes();
    bool HandleSpikeMarking(std::vector<Unit*> const& spikes, Unit* boss);
    bool HandleNoSpikesMarking(Unit* boss);
    bool HandleSpikeAssignment(std::vector<Unit*> const& spikes, Unit* boss);
    bool MoveTowardPosition(Position const& position, float incrementSize);
    bool IsSpikeInColdFlame(Unit* spike);
    static Player* GetSpikeVictim(Unit* spike);
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
    bool HandleAddTargeting(Unit* boss);
    bool UpdateRaidTargetIcon(Unit* target);
    bool HandleNonTankAddEvasion();
    bool IsAdd(Unit* unit);
    bool IsAssistTankAlive();
    bool ApplyNearbyAddCC();
    bool ApplyCCToAdd(Unit* add);
    bool IsAddsAlive();
    bool EngageBoss();
    Unit* FindAndCollectAdd(Unit* boss);
    Unit* FindAddNearBoss(Unit* boss, float maxDist);

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

// Gunship Battle
class IccCannonFireAction : public Action
{
public:
    IccCannonFireAction(PlayerbotAI* botAI, std::string const name = "icc cannon fire") : Action(botAI, name) {}
    bool Execute(Event event) override;

private:
    Unit* FindValidCannonTarget();
    bool TryCastCannonSpell(uint32 spellId, Unit* target, Unit* vehicleBase);
};

class IccGunshipEnterCannonAction : public MovementAction
{
public:
    IccGunshipEnterCannonAction(PlayerbotAI* botAI, std::string const name = "icc gunship enter cannon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Unit* FindBestAvailableCannon();
    bool IsValidCannon(Unit* vehicle);
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

class IccGunshipRocketJumpAction : public AttackAction
{
public:
    IccGunshipRocketJumpAction(PlayerbotAI* botAI, std::string const name = "icc gunship rocket jump")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    enum class GunshipSide
    {
        NONE,
        ALLY,
        HORDE
    };

    GunshipSide DetectShip() const;
    Item* FindRocketPack() const;
    bool UseRocketPack(Position const& destination, bool walkIfOutOfRange);
    bool RocketPackJumpToward(Position const& target);
    bool ExitCannonIfSeated();
    bool CleanupSkullIcon(uint8 skullIconIndex);
    bool UpdateBossSkullIcon(Unit* boss, uint8 skullIconIndex);
    bool IsMainTankOnEnemyShip(GunshipSide side) const;
    bool AnyNonTankAwayFromFriendly(GunshipSide side) const;
    Unit* FindNearestFriendlyCannon(GunshipSide side) const;
};

class IccGunshipRocketPackSetupAction : public MovementAction
{
public:
    IccGunshipRocketPackSetupAction(PlayerbotAI* botAI, std::string const name = "icc gunship rocket pack setup")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    Item* FindRocketPack() const;
    bool AcquireRocketPack();
    bool EquipRocketPack();
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
    bool UpdateSkullMarker(Unit* priorityTarget);
};

class IccDogsTankPositionAction : public AttackAction
{
public:
    IccDogsTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc dogs tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
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
    Position DetermineTargetPosition(bool hasSpore, SporeInfo const& sporeInfo, Position const& spreadRangedPos);
    bool CheckMainTankSpore();
    bool GooNear(Position const& pos);
};

class IccFestergutAvoidMalleableGooAction : public MovementAction
{
public:
    IccFestergutAvoidMalleableGooAction(PlayerbotAI* botAI,
                                        std::string const name = "icc festergut avoid malleable goo")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

//Rotface
class IccRotfaceTankPositionAction : public AttackAction
{
public:
    IccRotfaceTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc rotface tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool MarkBossWithSkull(Unit* boss);
    bool PositionMainTankAndMelee(Unit* boss, Unit* smallOoze = nullptr);
    bool HandleAssistTankPositioning(Unit* boss);
    Unit* FindAssignedBigOoze(Unit* boss, std::vector<Unit*>& bigOozes);
    bool HandleBigOozeKiting(Unit* bigOoze);
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
    bool HandleOozeMemberPositioning(Unit* mySmallOoze);
    bool PositionRangedAndHealers(Unit* boss,Unit* smallOoze);
    bool PositionHeroicGrid(Unit* boss);
};

class IccRotfaceMoveAwayFromExplosionAction : public MovementAction
{
public:
    IccRotfaceMoveAwayFromExplosionAction(PlayerbotAI* botAI, std::string const name = "icc rotface move away from explosion")
        : MovementAction(botAI, name),
          _escapePosition(0.0f, 0.0f, 0.0f),
          _hasEscape(false),
          _holdUntil(0) {}
    bool Execute(Event event) override;

private:
    Position _escapePosition;
    bool _hasEscape;
    uint32 _holdUntil;
};

class IccRotfaceAvoidVileGasAction : public MovementAction
{
public:
    IccRotfaceAvoidVileGasAction(PlayerbotAI* botAI, std::string const name = "icc rotface avoid vile gas")
        : MovementAction(botAI, name),
          _safeSpot(0.0f, 0.0f, 0.0f),
          _hasSafeSpot(false) {}
    bool Execute(Event event) override;

private:
    Position _safeSpot;
    bool _hasSafeSpot;
};

//PP
class IccPutricideMutatedPlagueAction : public AttackAction
{
public:
    IccPutricideMutatedPlagueAction(PlayerbotAI* botAI, std::string const name = "icc putricide mutated plague")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccPutricideGrowingOozePuddleAction : public AttackAction
{
public:
    IccPutricideGrowingOozePuddleAction(PlayerbotAI* botAI, std::string const name = "icc putricide growing ooze puddle")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    Unit* FindClosestThreateningPuddle();
    Position CalculateSafeMovePosition(Unit* closestPuddle);
    bool IsPositionTooCloseToOtherPuddles(float x, float y, Unit* ignorePuddle);
    bool PathCrossesAnyPuddle(float fromX, float fromY, float toX, float toY, Unit* ignorePuddle);
};

class IccPutricideVolatileOozeAction : public AttackAction
{
public:
    IccPutricideVolatileOozeAction(PlayerbotAI* botAI, std::string const name = "icc putricide volatile ooze")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool MarkOozeWithSkull(Unit* ooze);
    Unit* FindAuraTarget();
};

class IccPutricideGasCloudAction : public AttackAction
{
public:
    IccPutricideGasCloudAction(PlayerbotAI* botAI, std::string const name = "icc putricide gas cloud")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

    bool HandleGaseousBloatMovement(Unit* gasCloud);
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
    bool HasObstacleBetween(Position const& from, Position const& to);
    bool IsOnPath(Position const& from, Position const& to, Position const& point, float threshold);
};

class IccPutricideAbominationAction : public AttackAction
{
public:
    IccPutricideAbominationAction(PlayerbotAI* botAI, std::string const name = "icc putricide abomination")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool BecomeAbomination();
    bool IsSomeoneAlreadyPiloting();
    Unit* FindClosestPuddle(float maxRange);
    Unit* PickSlashTarget(Unit* boss);
    bool TryRegurgitate(Unit* abo, Unit* target);
    bool TryEatOoze(Unit* abo, Unit* puddle);
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

    bool MarkEmpoweredPrince();
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

    Unit* FindNearestBomb();
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
        : AttackAction(botAI, "icc bql group position") {}
    bool Execute(Event event) override;

    bool HandleTankPosition(Unit* boss, Aura* frenzyAura, Aura* shadowAura);
    bool HandleShadowsMovement();
    Position AdjustControlPoint(Position const& wall, Position const& center, float factor);
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

    bool CalculateCenterPosition(Position& targetPos, std::vector<Player*> const& playersWithAura);
    bool MoveToTargetPosition(Position const& targetPos, int auraCount);
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

    bool Handle25ManGroupLogic();
    bool HandleMarkingLogic(bool inGroup1, bool inGroup2, bool singleMarkMode);
    bool Handle10ManGroupLogic();

private:
    bool ApplyCrowdControl(Unit* zombie);
};

class IccValithriaZombieKiteAction : public MovementAction
{
public:
    IccValithriaZombieKiteAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "icc valithria zombie kite") {}
    bool Execute(Event event) override;
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

private:
    std::vector<Creature*> CollectClouds(uint32 entry, Unit* reference);
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
    bool MoveIncrementallyToPosition(Position const& targetPos, float maxStep);
};

class IccSindragosaFrostBeaconAction : public MovementAction
{
public:
    IccSindragosaFrostBeaconAction(PlayerbotAI* botAI)
            : MovementAction(botAI, "icc sindragosa frost beacon") {}
    bool Execute(Event event) override;

    bool HandleSupportActions();
    bool HandleBeaconedPlayer(Unit const* boss);
    bool HandleNonBeaconedPlayer(Unit const* boss);
    bool MoveToPositionIfNeeded(Position const& position, float tolerance);
    bool MoveToPosition(Position const& position);
    bool IsBossFlying(Unit const* boss);
    bool TryDropTombFlares(Unit const* boss);

    private:
    static constexpr float POSITION_TOLERANCE = 1.0f;
    static constexpr float TOMB_POSITION_TOLERANCE = 0.5f;
    static constexpr float MIN_SAFE_DISTANCE = 13.0f;
    static constexpr float MOVE_TOLERANCE = 2.0f;
    static constexpr uint32 FLARE_ITEM_COOLDOWN_MS = 1000;
};

class IccSindragosaHotAction : public Action
{
public:
    IccSindragosaHotAction(PlayerbotAI* botAI) : Action(botAI, "icc sindragosa hot") {}
    bool Execute(Event event) override;
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

private:
    struct FrostBombContext
    {
        Unit* marker = nullptr;
        std::vector<Unit*> tombs;
    };
    bool CollectContext(FrostBombContext& ctx) const;
    int ResolveGroupIndex(Group* group) const;
    void PinGroupToCurrentZone();
    std::vector<Unit*> SelectTombs(std::vector<Unit*> const& tombs, int groupIndex, int groupCount) const;
    Unit* ResolveStickyTomb(std::vector<Unit*> const& myTombs);
    bool HandleRtiMarking(Group* group, int groupIndex, std::vector<Unit*> const& myTombs, Unit* losTomb);
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

    bool IsPositionSafeFromDefile(float x, float y, float z, float minSafeDistance) const;
    bool IsPositionSafeFromShadowTraps(float x, float y) const;
    bool IsValidCollectibleAdd(Unit* unit) const;
    bool HandleTankPositioning();
    bool HandleMeleePositioning();
    bool HandleRangedPositioning();
    bool HandleMainTankAddManagement(Unit* boss, Position const* tankPos);
    bool HandleAssistTankAddManagement(Unit* boss, Position const* tankPos);
    bool HandlePetManagement();

private:
    static constexpr float PLATFORM_Z      = 840.857f;
    static constexpr float BEHIND_DISTANCE = 4.0f;
    bool FixPlatformPosition();
    bool ClearInvalidTarget();
    Position const* GetMainTankPosition();
    Position const* GetMainTankRangedPosition();
    bool TryMoveToPosition(float x, float y, float z, bool forced = true);
};

class IccLichKingAddsAction : public AttackAction
{
public:
    IccLichKingAddsAction(PlayerbotAI* botAI)
        : AttackAction(botAI, "icc lich king adds") {}
    bool Execute(Event event) override;

    bool HandleTeleportationFixes(Difficulty diff, Unit* terenas);
    bool HandleHeroicNonTankPositioning(Difficulty diff, Unit* terenas);
    bool HandleSpiritMarkingAndTargeting(Difficulty diff, Unit* terenas);
    bool HandleQuakeMechanics(Unit* boss);
    bool HandleShamblingHorrors(Unit* boss, bool hasPlague);
    bool HandleRagingSpiritFlanking(Unit* boss, bool hasPlague, Difficulty diff);
    bool HandleAssistTankAddManagement(Unit* boss, Difficulty diff);
    bool HandleMeleePositioning(Unit* boss, bool hasPlague, Difficulty diff);
    bool HandleMainTankTargeting(Unit* boss, Difficulty diff);
    bool HandleNonTankHeroicPositioning(Unit* boss, Difficulty diff, bool hasPlague);
    bool HandleRangedPositioning(Unit* boss, bool hasPlague, Difficulty diff);
    bool HandleDefileMechanics(Unit* boss, Difficulty diff);
    bool HandleCenterStacking(Unit* boss, Difficulty diff);
    bool HandleValkyrMechanics(Difficulty diff);
    bool HandleValkyrMarking(std::vector<Unit*> const& valkyrs, Difficulty diff);
    bool HandleValkyrAssignment(std::vector<Unit*> const& valkyrs);
    bool HandleVileSpiritMechanics();
    bool HandleIceSphereMechanics();
    bool ApplyCCToValkyr(Unit* valkyr);
    bool IsValkyr(Unit* unit);
    std::pair<float, float> DefileAwareStep(float tx, float ty,
                                           std::vector<Unit*> const& defiles,
                                           Difficulty diff);
};

class IccLichKingSpiritBombAction : public MovementAction
{
public:
    IccLichKingSpiritBombAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "icc lich king spirit bomb") {}
    bool Execute(Event event) override;

    static bool IsBombThreatActive(PlayerbotAI* botAI, Player* bot);
};

#endif
