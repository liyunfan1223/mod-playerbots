#ifndef _PLAYERBOT_RAIDICCTRIGGERS_H
#define _PLAYERBOT_RAIDICCTRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

//Lord Marrowgar
class IccLmTankPositionTrigger : public Trigger
{
public:
    IccLmTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lm tank position") {}
    bool IsActive() override;
};

class IccSpikeNearTrigger : public Trigger
{
public:
    IccSpikeNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc spike near") {}
    bool IsActive() override;
};

//Lady Deathwhisper
class IccDarkReckoningTrigger : public Trigger
{
public:
    IccDarkReckoningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc dark reckoning") {}
    bool IsActive() override;
};

class IccRangedPositionLadyDeathwhisperTrigger : public Trigger
{
public:
    IccRangedPositionLadyDeathwhisperTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc ranged position lady deathwhisper") {}
    bool IsActive() override;
};

class IccAddsLadyDeathwhisperTrigger : public Trigger
{
public:
    IccAddsLadyDeathwhisperTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc adds lady deathwhisper") {}
    bool IsActive() override;
};

class IccShadeLadyDeathwhisperTrigger : public Trigger
{
public:
    IccShadeLadyDeathwhisperTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc shade lady deathwhisper") {}
    bool IsActive() override;
};

//Gunship Battle
class IccRottingFrostGiantTankPositionTrigger : public Trigger
{
public:
    IccRottingFrostGiantTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc rotting frost giant tank position") {}
    bool IsActive() override;
};

class IccInCannonTrigger : public Trigger
{
public:
    IccInCannonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc in cannon") {}
    bool IsActive() override;
};

class IccGunshipCannonNearTrigger : public Trigger
{
public:
    IccGunshipCannonNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc in cannon") {}
    bool IsActive() override;
};

class IccGunshipTeleportAllyTrigger : public Trigger
{
public:
    IccGunshipTeleportAllyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc gunship teleport ally") {}
    bool IsActive() override;
};

class IccGunshipTeleportHordeTrigger : public Trigger
{
public:
    IccGunshipTeleportHordeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc gunship teleport horde") {}
    bool IsActive() override;
};

//DBS
class IccDbsTankPositionTrigger : public Trigger
{
public:
    IccDbsTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc dbs tank position") {}
    bool IsActive() override;
};

class IccDbsMainTankRuneOfBloodTrigger : public Trigger
{
public:
    IccDbsMainTankRuneOfBloodTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc dbs main tank rune of blood") {}
    bool IsActive() override;
};

class IccAddsDbsTrigger : public Trigger
{
public:
    IccAddsDbsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc adds dbs") {}
    bool IsActive() override;
};

//DOGS
class IccStinkyPreciousMainTankMortalWoundTrigger : public Trigger
{
public:
    IccStinkyPreciousMainTankMortalWoundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc stinky precious main tank mortal wound") {}
    bool IsActive() override;
};

//FESTERGUT
class IccFestergutTankPositionTrigger : public Trigger
{
public:
    IccFestergutTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc festergut tank position") {}
    bool IsActive() override;
};

class IccFestergutMainTankGastricBloatTrigger : public Trigger
{
public:
    IccFestergutMainTankGastricBloatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc festergut main tank gastric bloat") {}
    bool IsActive() override;
};

class IccFestergutSporeTrigger : public Trigger
{
public:
    IccFestergutSporeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc festergut spore") {}
    bool IsActive() override;
};

//ROTFACE
class IccRotfaceTankPositionTrigger : public Trigger
{
public:
    IccRotfaceTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc rotface tank position") {}
    bool IsActive() override;
};

class IccRotfaceGroupPositionTrigger : public Trigger
{
public:
    IccRotfaceGroupPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc rotface group position") {}
    bool IsActive() override;
};


class IccRotfaceMoveAwayFromExplosionTrigger : public Trigger
{
public:
    IccRotfaceMoveAwayFromExplosionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc rotface move away from explosion") {}
    bool IsActive() override;
};

//PP
class IccPutricideVolatileOozeTrigger : public Trigger
{
public:
    IccPutricideVolatileOozeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc putricide volatile ooze") {}
    bool IsActive() override;
};

class IccPutricideGasCloudTrigger : public Trigger
{
public:
    IccPutricideGasCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc putricide gas cloud") {}
    bool IsActive() override;
};


class IccPutricideGrowingOozePuddleTrigger : public Trigger
{
public:
    IccPutricideGrowingOozePuddleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc putricide growing ooze puddle") {}
    bool IsActive() override;
};

class IccPutricideMainTankMutatedPlagueTrigger : public Trigger
{
public:
    IccPutricideMainTankMutatedPlagueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc putricide main tank mutated plague") {}
    bool IsActive() override;
};

class IccPutricideMalleableGooTrigger : public Trigger 
{
public:
    IccPutricideMalleableGooTrigger(PlayerbotAI* ai) : Trigger(ai, "icc putricide malleable goo") {}
    bool IsActive() override;
};

//BPC
class IccBpcKelesethTankTrigger : public Trigger 
{
public:
    IccBpcKelesethTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bpc keleseth tank") {}
    bool IsActive() override;
};

class IccBpcNucleusTrigger : public Trigger
{
public:
    IccBpcNucleusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bpc nucleus") {}
    bool IsActive() override;
};

class IccBpcMainTankTrigger : public Trigger
{
public:
    IccBpcMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bpc main tank") {}
    bool IsActive() override;
};

class IccBpcEmpoweredVortexTrigger : public Trigger
{
public:
    IccBpcEmpoweredVortexTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bpc empowered vortex") {}
    bool IsActive() override;
};

//Bql
class IccBqlTankPositionTrigger : public Trigger
{
public:
    IccBqlTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bql tank position") {}
    bool IsActive() override;
};

class IccBqlPactOfDarkfallenTrigger : public Trigger
{
public:
    IccBqlPactOfDarkfallenTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bql pact of darkfallen") {}
    bool IsActive() override;
};

class IccBqlVampiricBiteTrigger : public Trigger
{
public:
    IccBqlVampiricBiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bql vampiric bite") {}
    bool IsActive() override;
};

//VDW
class IccValkyreSpearTrigger : public Trigger
{
public:
    IccValkyreSpearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc valkyre spear") {}
    bool IsActive() override;
};  

class IccSisterSvalnaTrigger : public Trigger
{
public:
    IccSisterSvalnaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sister svalna") {}
    bool IsActive() override;
};

class IccValithriaPortalTrigger : public Trigger
{
public:
    IccValithriaPortalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc valithria portal") {}
    bool IsActive() override;
};

class IccValithriaHealTrigger : public Trigger
{
public:
    IccValithriaHealTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc valithria heal") {}
    bool IsActive() override;
};

class IccValithriaDreamCloudTrigger : public Trigger
{
public:
    IccValithriaDreamCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc valithria dream cloud") {}
    bool IsActive() override;
};


//SINDRAGOSA
class IccSindragosaTankPositionTrigger : public Trigger
{
public:
    IccSindragosaTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa tank position") {}
    bool IsActive() override;
};

class IccSindragosaFrostBeaconTrigger : public Trigger
{
public:
    IccSindragosaFrostBeaconTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa frost beacon") {}
    bool IsActive() override;
};

class IccSindragosaBlisteringColdTrigger : public Trigger
{
public:
    IccSindragosaBlisteringColdTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa blistering cold") {}
    bool IsActive() override;
};

class IccSindragosaUnchainedMagicTrigger : public Trigger   
{
public:
    IccSindragosaUnchainedMagicTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa unchained magic") {}
    bool IsActive() override;
};

class IccSindragosaChilledToTheBoneTrigger : public Trigger
{
public:
    IccSindragosaChilledToTheBoneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa chilled to the bone") {}
    bool IsActive() override;
};

class IccSindragosaMysticBuffetTrigger : public Trigger
{
public:
    IccSindragosaMysticBuffetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa mystic buffet") {}
    bool IsActive() override;
};

class IccSindragosaMainTankMysticBuffetTrigger : public Trigger
{
public:
    IccSindragosaMainTankMysticBuffetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa main tank mystic buffet") {}
    bool IsActive() override;
};

class IccSindragosaTankSwapPositionTrigger : public Trigger
{
public:
    IccSindragosaTankSwapPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa tank swap position") {}
    bool IsActive() override;
};

class IccSindragosaFrostBombTrigger : public Trigger
{
public:
    IccSindragosaFrostBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa frost bomb") {}
    bool IsActive() override;
};


//LICH KING
class IccLichKingShadowTrapTrigger : public Trigger
{
public:
    IccLichKingShadowTrapTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lich king shadow trap") {}
    bool IsActive() override;
};

class IccLichKingNecroticPlagueTrigger : public Trigger 
{
public:
    IccLichKingNecroticPlagueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lich king necrotic plague") {}
    bool IsActive() override;
};

class IccLichKingWinterTrigger : public Trigger
{
public:
    IccLichKingWinterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lich king winter") {}
    bool IsActive() override;
};

class IccLichKingAddsTrigger : public Trigger
{
public:
    IccLichKingAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lich king adds") {}
    bool IsActive() override;
};

#endif
