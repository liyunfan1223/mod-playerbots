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

#endif
