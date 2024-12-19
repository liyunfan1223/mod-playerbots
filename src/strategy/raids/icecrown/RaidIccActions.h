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
const Position ICC_GUNSHIP_TELEPORT_HORDE = Position (-449.5343f, 2477.2024f, 470.17648f); 
const Position ICC_DBS_TANK_POSITION = Position(-494.26517f, 2211.549f, 541.11414f);
const Position ICC_FESTERGUT_TANK_POSITION = Position(4269.1772f, 3144.7673f, 360.38577f);
const Position ICC_FESTERGUT_RANGED_SPORE = Position(4261.143f, 3109.4146f, 360.38605f);
const Position ICC_FESTERGUT_MELEE_SPORE = Position(4269.1772f, 3144.7673f, 360.38577f);
const Position ICC_ROTFACE_TANK_POSITION = Position(4447.061f, 3150.9758f, 360.38568f);
const Position ICC_ROTFACE_SAFE_POSITION = Position(4446.557f, 3065.6594f, 360.51843f);
const Position ICC_PUTRICIDE_TANK_OOZE_POSITION = Position(4362.709f, 3229.1448f, 389.4083f);
const Position ICC_PUTRICIDE_TANK_GAS_CLOUD_POSITION = Position(4397.0386f, 3221.385f, 389.3999f);
const Position ICC_BPC_OT_POSITION = Position(4649.2236f, 2796.0972f, 361.1815f);
const Position ICC_BPC_MT_POSITION = Position(4648.5674f, 2744.847f, 361.18222f);
const Position ICC_BQL_CENTER_POSITION = Position(4595.0f, 2769.0f, 400.0f);
const Position ICC_BQL_TANK_POSITION = Position(4616.102f, 2768.9167f, 400.13797f);

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



#endif
