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


const Position ICC_LM_TANK_POSITION = Position(-391.0f, 2259.0f, 42.0f);
const Position ICC_DARK_RECKONING_SAFE_POSITION = Position(-523.33386f, 2211.2031f, 62.823116f);
const Position ICC_ROTTING_FROST_GIANT_TANK_POSITION = Position(-265.90125f, 2209.0605f, 199.97006f);
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
//const Position ICC_PUTRICIDE_TANK_OOZE_POSITION = Position(4362.709f, 3229.1448f, 389.4083f);
//const Position ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION = Position(4397.0386f, 3221.385f, 389.3999f);
//const Position ICC_PUTRICIDE_GAS1_POSITION = Position(4350.772f, 3249.9773f, 389.39508f);
//const Position ICC_PUTRICIDE_GAS2_POSITION = Position(4390.002f, 3204.8855f, 389.39938f);
//const Position ICC_PUTRICIDE_GAS3_POSITION = Position(4367.753f, 3177.5894f, 389.39575f);
//const Position ICC_PUTRICIDE_GAS4_POSITION = Position(4321.8486f, 3206.464f, 389.3982f);
const Position ICC_BPC_OT_POSITION = Position(4649.2236f, 2796.0972f, 361.1815f);
const Position ICC_BPC_MT_POSITION = Position(4648.5674f, 2744.847f, 361.18222f);
const Position ICC_BQL_CENTER_POSITION = Position(4595.0f, 2769.0f, 400.0f);
const Position ICC_BQL_TANK_POSITION = Position(4616.102f, 2768.9167f, 400.13797f);
const Position ICC_SINDRAGOSA_TANK_POSITION = Position(4408.016f, 2508.0647f, 203.37955f);
const Position ICC_SINDRAGOSA_RANGED_POSITION = Position(4373.7686f, 2498.0042f, 203.38176f);
const Position ICC_SINDRAGOSA_MELEE_POSITION = Position(4389.22f, 2499.5237f, 203.38033f);
const Position ICC_SINDRAGOSA_BLISTERING_COLD_POSITION = Position(4345.922f, 2484.708f, 206.22516f);
const Position ICC_SINDRAGOSA_THOMB1_POSITION = Position(4381.819f, 2471.1448f, 203.37704f);       // Westmost position
const Position ICC_SINDRAGOSA_THOMB2_POSITION = Position(4381.819f, 2483.1448f, 203.37704f);       // 12y east from pos1
const Position ICC_SINDRAGOSA_THOMB3_POSITION = Position(4381.819f, 2471.1448f, 203.37704f);       // Same as pos1
const Position ICC_SINDRAGOSA_THOMB4_POSITION = Position(4381.819f, 2483.1448f, 203.37704f);       // Same as pos2
const Position ICC_SINDRAGOSA_THOMB5_POSITION = Position(4381.819f, 2495.1448f, 203.37704f);       // 12y east from pos2/4
const Position ICC_SINDRAGOSA_CENTER_POSITION = Position(4408.0464f, 2484.478f, 203.37529f);           
const Position ICC_SINDRAGOSA_THOMBMB2_POSITION = Position(4382.6113f, 2505.4922f, 203.38197f);       
const Position ICC_SINDRAGOSA_FBOMB_POSITION = Position(4400.031f, 2507.0295f, 203.37929f);      //old 4400.031f, 2507.0295f, 203.37929f  //alternate for 10 man  4366.0225f, 2501.569f, 203.38226f 
const Position ICC_SINDRAGOSA_FBOMB10_POSITION = Position(4366.0225f, 2501.569f, 203.38226f);
const Position ICC_SINDRAGOSA_LOS2_POSITION = Position(4376.0938f, 2511.103f, 203.38303f); 
const Position ICC_LICH_KING_ADDS_POSITION = Position(486.63647f, -2095.7915f, 840.857f);
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
};

class IccSpikeAction : public AttackAction 
{
public:
    IccSpikeAction(PlayerbotAI* botAI) : AttackAction(botAI, "icc spike") {}
    bool Execute(Event event) override;
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
};

class IccAddsLadyDeathwhisperAction : public AttackAction
{
public:
    IccAddsLadyDeathwhisperAction(PlayerbotAI* botAI, std::string const name = "icc adds lady deathwhisper")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
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
};

class IccGunshipEnterCannonAction : public MovementAction
{
public:
    IccGunshipEnterCannonAction(PlayerbotAI* botAI, std::string const name = "icc gunship enter cannon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

class IccGunshipTeleportAllyAction : public AttackAction
{
public:
    IccGunshipTeleportAllyAction(PlayerbotAI* botAI, std::string const name = "icc gunship teleport ally")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccGunshipTeleportHordeAction : public AttackAction
{
public:
    IccGunshipTeleportHordeAction(PlayerbotAI* botAI, std::string const name = "icc gunship teleport horde")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

//DBS
class IccDbsTankPositionAction : public AttackAction
{
public:
    IccDbsTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc dbs tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccAddsDbsAction : public AttackAction
{
public:
    IccAddsDbsAction(PlayerbotAI* botAI, std::string const name = "icc adds dbs")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

//FESTERGUT
class IccFestergutTankPositionAction : public AttackAction
{
public:
    IccFestergutTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc festergut tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccFestergutSporeAction : public AttackAction
{
public:
    IccFestergutSporeAction(PlayerbotAI* botAI, std::string const name = "icc festergut spore")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

//Rotface
class IccRotfaceTankPositionAction : public AttackAction
{
public:
    IccRotfaceTankPositionAction(PlayerbotAI* botAI, std::string const name = "icc rotface tank position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccRotfaceGroupPositionAction : public AttackAction
{
public:
    IccRotfaceGroupPositionAction(PlayerbotAI* botAI, std::string const name = "icc rotface group position")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccRotfaceMoveAwayFromExplosionAction : public MovementAction 
{
    public:
        IccRotfaceMoveAwayFromExplosionAction(PlayerbotAI* botAI, std::string const name = "icc rotface move away from explosion") 
            : MovementAction(botAI, name) {}
        
        bool Execute(Event event) override;
};

//PP

class IccPutricideGrowingOozePuddleAction : public AttackAction
{
public:
    IccPutricideGrowingOozePuddleAction(PlayerbotAI* botAI, std::string const name = "icc putricide growing ooze puddle")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccPutricideVolatileOozeAction : public AttackAction
{
public:
    IccPutricideVolatileOozeAction(PlayerbotAI* botAI, std::string const name = "icc putricide volatile ooze")
        : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class IccPutricideGasCloudAction : public AttackAction
{
    public:
    IccPutricideGasCloudAction(PlayerbotAI* botAI, std::string const name = "icc putricide gas cloud")
        : AttackAction(botAI, name) {}
        bool Execute(Event event) override;
};

class AvoidMalleableGooAction : public MovementAction 
{
    public:
        AvoidMalleableGooAction(PlayerbotAI* botAI, std::string const name = "avoid malleable goo"  ) 
        : MovementAction(botAI, name) {}
        bool Execute(Event event) override;
};

//BPC
class IccBpcKelesethTankAction : public AttackAction
{
public:
    IccBpcKelesethTankAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bpc keleseth tank") {}
    bool Execute(Event event) override;
};

class IccBpcNucleusAction : public AttackAction
{
public:
    IccBpcNucleusAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bpc nucleus") {}
    bool Execute(Event event) override;
};

class IccBpcMainTankAction : public AttackAction
{
public:
    IccBpcMainTankAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bpc main tank") {}
    bool Execute(Event event) override;
};

//BPC Vortex
class IccBpcEmpoweredVortexAction : public MovementAction
{
public:
    IccBpcEmpoweredVortexAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "icc bpc empowered vortex") {}
    bool Execute(Event event) override;
};

//Blood Queen Lana'thel
class IccBqlTankPositionAction : public AttackAction
{
public:
    IccBqlTankPositionAction(PlayerbotAI* botAI) 
        : AttackAction(botAI, "icc bql tank position") {}
    bool Execute(Event event) override;
};  

class IccBqlPactOfDarkfallenAction : public MovementAction
{
public:
    IccBqlPactOfDarkfallenAction(PlayerbotAI* botAI) 
        : MovementAction(botAI, "icc bql pact of darkfallen") {}
    bool Execute(Event event) override;
};

class IccBqlVampiricBiteAction : public AttackAction
{
public:
    IccBqlVampiricBiteAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc bql vampiric bite") {}
    bool Execute(Event event) override;
};

//VDW
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
class IccSindragosaTankPositionAction : public AttackAction
{
public:
    IccSindragosaTankPositionAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc sindragosa tank position") {}
    bool Execute(Event event) override;
};

class IccSindragosaFrostBeaconAction : public MovementAction
{
public:
    IccSindragosaFrostBeaconAction(PlayerbotAI* botAI) 
            : MovementAction(botAI, "icc sindragosa frost beacon") {}
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

class IccSindragosaMysticBuffetAction : public AttackAction
{
public:
    IccSindragosaMysticBuffetAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc sindragosa mystic buffet") {}
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
};

class IccLichKingAddsAction : public AttackAction
{
    public:
        IccLichKingAddsAction(PlayerbotAI* botAI) 
            : AttackAction(botAI, "icc lich king adds") {}
        bool Execute(Event event) override;
};



#endif
