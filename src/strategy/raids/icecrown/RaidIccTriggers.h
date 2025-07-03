#ifndef _PLAYERBOT_RAIDICCTRIGGERS_H
#define _PLAYERBOT_RAIDICCTRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

enum CreatureIdsICC
{

    // Lord Marrowgar
    NPC_SPIKE1                          = 36619,
    NPC_SPIKE2                          = 38711,
    NPC_SPIKE3                          = 38712,

    // Lady Deathwhisper
    NPC_SHADE                           = 38222,

    // Gunship Battle
    NPC_KOR_KRON_BATTLE_MAGE            = 37117,
    NPC_KOR_KRON_AXETHROWER             = 36968,
    NPC_KOR_KRON_ROCKETEER              = 36982,
    NPC_SKYBREAKER_SORCERER             = 37116,
    NPC_SKYBREAKER_RIFLEMAN             = 36969,
    NPC_SKYBREAKER_MORTAR_SOLDIER       = 36978,
    NPC_IGB_HIGH_OVERLORD_SAURFANG      = 36939,
    NPC_IGB_MURADIN_BRONZEBEARD         = 36948,
    NPC_CANNONA                         = 36838,
    NPC_CANNONH                         = 36839,
    NPC_MURADIN_BRONZEBEARD             = 36948,
    NPC_HIGH_OVERLORD_SAURFANG          = 36939,

    // Deathbringer Saurfang
    NPC_BLOOD_BEAST1                    = 38508,
    NPC_BLOOD_BEAST2                    = 38596,
    NPC_BLOOD_BEAST3                    = 38597,
    NPC_BLOOD_BEAST4                    = 38598,

    // Rotface
    NPC_PUDDLE                          = 37013,
    NPC_BIG_OOZE                        = 36899,

    // Putricide
    NPC_MALLEABLE_OOZE_STALKER          = 38556,
    NPC_GROWING_OOZE_PUDDLE             = 37690,
    NPC_CHOKING_GAS_BOMB                = 38159,

    // Blood Prince Council
    NPC_DARK_NUCLEUS                    = 38369,
    NPC_PRINCE_KELESETH                 = 37972,
    NPC_PRINCE_TALDARAM                 = 37973,
    NPC_PRINCE_VALANAR                  = 37970,
    NPC_KINETIC_BOMB1                   = 38454,
    NPC_KINETIC_BOMB2                   = 38775,
    NPC_KINETIC_BOMB3                   = 38776,
    NPC_KINETIC_BOMB4                   = 38777,
    NPC_BALL_OF_FLAME                   = 38332,
    NPC_BALL_OF_INFERNO_FLAME           = 38451,

    // Blood Queen Lana'thel
    NPC_SWARMING_SHADOWS                = 38163,

    // Sister Svalna
    NPC_SPEAR                           = 38248,
    ITEM_SPEAR                          = 50307,

    // Valithria Dreamwalker
    NPC_VALITHRIA_DREAMWALKER           = 36789,
    NPC_DREAM_PORTAL                    = 37945,
    NPC_NIGHTMARE_PORTAL                = 38430,
    NPC_DREAM_CLOUD                     = 37985,
    NPC_NIGHTMARE_CLOUD                 = 38421,
    NPC_RISEN_ARCHMAGE                  = 37868,
    NPC_BLAZING_SKELETON                = 36791,
    NPC_SUPPRESSER                      = 37863,
    NPC_BLISTERING_ZOMBIE               = 37934,
    NPC_GLUTTONOUS_ABOMINATION          = 37886,
    NPC_ROT_WORM                        = 37907,
    NPC_COLUMN_OF_FROST                 = 37918,
    NPC_MANA_VOID                       = 38068,
    NPC_DREAM_PORTAL_PRE_EFFECT         = 38186,
    NPC_NIGHTMARE_PORTAL_PRE_EFFECT     = 38429,

    // Sindragosa
    NPC_SINDRAGOSA                      = 36853,
    NPC_TOMB1                           = 36980,
    NPC_TOMB2                           = 38320,
    NPC_TOMB3                           = 38321,
    NPC_TOMB4                           = 38322,

    // Lich King
    NPC_THE_LICH_KING                   = 36597,
    NPC_TERENAS_MENETHIL                = 36823,
    NPC_TERENAS_MENETHIL_HC             = 39217,
    NPC_SPIRIT_BOMB                     = 39189,
    NPC_WICKED_SPIRIT1                  = 39190,
    NPC_WICKED_SPIRIT2                  = 39287,
    NPC_WICKED_SPIRIT3                  = 39288,
    NPC_WICKED_SPIRIT4                  = 39289,
    NPC_SHADOW_TRAP                     = 39137,
    NPC_SHAMBLING_HORROR1               = 37698,
    NPC_SHAMBLING_HORROR2               = 39299,
    NPC_SHAMBLING_HORROR3               = 39300,
    NPC_SHAMBLING_HORROR4               = 39301,
    NPC_ICE_SPHERE1                     = 36633,
    NPC_ICE_SPHERE2                     = 39305,
    NPC_ICE_SPHERE3                     = 39306,
    NPC_ICE_SPHERE4                     = 39307,
    NPC_RAGING_SPIRIT1                  = 36701,
    NPC_RAGING_SPIRIT2                  = 39302,
    NPC_RAGING_SPIRIT3                  = 39303,
    NPC_RAGING_SPIRIT4                  = 39304,
    NPC_DRUDGE_GHOUL1                   = 37695,
    NPC_DRUDGE_GHOUL2                   = 39309,
    NPC_DRUDGE_GHOUL3                   = 39310,
    NPC_DRUDGE_GHOUL4                   = 39311,
    NPC_VALKYR_SHADOWGUARD1             = 36609,
    NPC_VALKYR_SHADOWGUARD2             = 39120,
    NPC_VALKYR_SHADOWGUARD3             = 39121,
    NPC_VALKYR_SHADOWGUARD4             = 39122,
    NPC_VILE_SPIRIT1                    = 37799,
    NPC_VILE_SPIRIT2                    = 39284,
    NPC_VILE_SPIRIT3                    = 39285,
    NPC_VILE_SPIRIT4                    = 39286,

};

enum SpellIdsICC
{
    // ICC cheat spells
    SPELL_EMPOWERED_BLOOD               = 70227, //70304 -->50%, 70227 /*100% more dmg, 100% more att speed*/
    SPELL_EXPERIENCED                   = 71188, //dmg 30% 20% att speed
    SPELL_NO_THREAT                     = 70115, //reduce threat
    SPELL_SPITEFULL_FURY                = 36886, //500% more threat
    SPELL_NITRO_BOOSTS                  = 54861, //Speed
    SPELL_PAIN_SUPPRESION               = 69910, //40% dmg reduction
    SPELL_AGEIS_OF_DALARAN              = 71638, //268 all ress
    SPELL_CYCLONE                       = 33786, 
    SPELL_HAMMER_OF_JUSTICE             = 10308, //stun

    // Lady Deathwhisper
    SPELL_DARK_RECKONING                = 69483,

    // Gunship Battle
    SPELL_DEATH_PLAGUE                  = 72865,
    SPELL_BELOW_ZERO                    = 69705,

    // Festergut
    SPELL_GAS_SPORE                     = 69279,

    // Rotface
    SPELL_SLIME_SPRAY                   = 69508,
    SPELL_OOZE_FLOOD                    = 71215,
    SPELL_UNSTABLE_OOZE_EXPLOSION       = 69839,
    SPELL_OOZE_FLOOD_VISUAL             = 69785,

    // Putricide
    SPELL_MALLEABLE_GOO                 = 70852,
    SPELL_GROW_AURA                     = 70347,

    // Blood Prince Council
    SPELL_EMPOWERED_SHOCK_VORTEX1       = 72039,
    SPELL_EMPOWERED_SHOCK_VORTEX2       = 73037,
    SPELL_EMPOWERED_SHOCK_VORTEX3       = 73038,
    SPELL_EMPOWERED_SHOCK_VORTEX4       = 73039,

    // Blood Queen Lana'thel
    SPELL_PACT_OF_THE_DARKFALLEN        = 71340,

    // Sister Svalna
    SPELL_AETHER_SHIELD                 = 71463,

    // Valithria Dreamwalker
    SPELL_DREAM_STATE                   = 70766,
    SPELL_EMERALD_VIGOR                 = 70873,

    // Sindragosa
    SPELL_FROST_BEACON                  = 70126,
    SPELL_ICE_TOMB                      = 70157,
    SPELL_FROST_BOMB_VISUAL             = 70022,
    SPELL_BLISTERING_COLD1              = 70123,
    SPELL_BLISTERING_COLD2              = 71047,
    SPELL_BLISTERING_COLD3              = 71048,
    SPELL_BLISTERING_COLD4              = 71049,

    // The Lich King
    SPELL_HARVEST_SOUL_VALKYR           = 68985,
    SPELL_QUAKE                         = 72262,
    SPELL_REMORSELESS_WINTER1           = 72259,
    SPELL_REMORSELESS_WINTER2           = 74273,
    SPELL_REMORSELESS_WINTER3           = 74274,
    SPELL_REMORSELESS_WINTER4           = 74275,
    SPELL_REMORSELESS_WINTER5           = 68981,
    SPELL_REMORSELESS_WINTER6           = 74270,
    SPELL_REMORSELESS_WINTER7           = 74271,
    SPELL_REMORSELESS_WINTER8           = 74272,
};

const uint32 DEFILE_AURAS[] = {72756, 74162, 74163, 74164};
const uint32 DEFILE_CAST_ID = 72762;
const uint32 DEFILE_NPC_ID = 38757;
const size_t DEFILE_AURA_COUNT = 4;

// All fanatics and adherents entry ids Lady Deathwhisper
static const std::array<uint32, 23> addEntriesLady    = {
                                                        37949, 38394, 38625, 38626, 38010, 38397, 39000, 39001,
                                                        38136, 38396, 38632, 38633, 37890, 38393, 38628, 38629,
                                                        38135, 38395, 38634, 38009, 38398, 38630, 38631};

const std::vector<uint32> spellEntriesFlood           = {
                                                        69782, 69783, 69796, 69797, 69798,
                                                        69799, 69801, 69802, 69795};

const std::vector<uint32> availableTargetsGS          = {
    NPC_KOR_KRON_AXETHROWER, NPC_KOR_KRON_ROCKETEER,        NPC_KOR_KRON_BATTLE_MAGE, NPC_IGB_HIGH_OVERLORD_SAURFANG,
    NPC_SKYBREAKER_RIFLEMAN, NPC_SKYBREAKER_MORTAR_SOLDIER, NPC_SKYBREAKER_SORCERER,  NPC_IGB_MURADIN_BRONZEBEARD};

static std::vector<ObjectGuid> sporeOrder;

//Lord Marrowgar
class IccLmTrigger : public Trigger
{
public:
    IccLmTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lm") {}
    bool IsActive() override;
};

//Lady Deathwhisper
class IccDarkReckoningTrigger : public Trigger
{
public:
    IccDarkReckoningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc dark reckoning") {}
    bool IsActive() override;
};

class IccLadyDeathwhisperTrigger : public Trigger
{
public:
    IccLadyDeathwhisperTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc lady deathwhisper") {}
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
class IccDbsTrigger : public Trigger
{
public:
    IccDbsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc dbs") {}
    bool IsActive() override;
};

class IccDbsMainTankRuneOfBloodTrigger : public Trigger
{
public:
    IccDbsMainTankRuneOfBloodTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc dbs main tank rune of blood") {}
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
class IccFestergutGroupPositionTrigger : public Trigger
{
public:
    IccFestergutGroupPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc festergut group position") {}
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

class IccBpcKineticBombTrigger : public Trigger
{
public:
    IccBpcKineticBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bpc kinetic bomb") {}
    bool IsActive() override;
};

class IccBpcBallOfFlameTrigger : public Trigger
{
public:
    IccBpcBallOfFlameTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bpc ball of flame") {}
    bool IsActive() override;
};

//Bql
class IccBqlGroupPositionTrigger : public Trigger
{
public:
    IccBqlGroupPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc bql tank position") {}
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

// Sister Svalna
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


// Valithria Dreamwalker

class IccValithriaGroupTrigger : public Trigger
{
public:
    IccValithriaGroupTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc valithria group") {}
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
class IccSindragosaGroupPositionTrigger : public Trigger
{
public:
    IccSindragosaGroupPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "icc sindragosa group position") {}
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
