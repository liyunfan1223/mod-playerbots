#ifndef _PLAYERBOT_RAIDICCTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDICCTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidIccTriggers.h"

class RaidIccTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidIccTriggerContext()
    {
        creators["icc lm tank position"] = &RaidIccTriggerContext::icc_lm_tank_position;
        creators["icc spike near"] = &RaidIccTriggerContext::icc_spike_near;
        creators["icc dark reckoning"] = &RaidIccTriggerContext::icc_dark_reckoning;
        creators["icc ranged position lady deathwhisper"] = &RaidIccTriggerContext::icc_ranged_position_lady_deathwhisper;
        creators["icc adds lady deathwhisper"] = &RaidIccTriggerContext::icc_adds_lady_deathwhisper;
        creators["icc shade lady deathwhisper"] = &RaidIccTriggerContext::icc_shade_lady_deathwhisper;
        creators["icc rotting frost giant tank position"] = &RaidIccTriggerContext::icc_rotting_frost_giant_tank_position;
        creators["icc in cannon"] = &RaidIccTriggerContext::icc_in_cannon;
        creators["icc gunship cannon near"] = &RaidIccTriggerContext::icc_gunship_cannon_near;
        creators["icc gunship teleport ally"] = &RaidIccTriggerContext::icc_gunship_teleport_ally;
        creators["icc gunship teleport horde"] = &RaidIccTriggerContext::icc_gunship_teleport_horde;
        creators["icc dbs tank position"] = &RaidIccTriggerContext::icc_dbs_tank_position;
        creators["icc dbs main tank rune of blood"] = &RaidIccTriggerContext::icc_dbs_main_tank_rune_of_blood;
        creators["icc adds dbs"] = &RaidIccTriggerContext::icc_adds_dbs;
        creators["icc stinky precious main tank mortal wound"] = &RaidIccTriggerContext::icc_stinky_precious_main_tank_mortal_wound;
        creators["icc festergut tank position"] = &RaidIccTriggerContext::icc_festergut_tank_position;
        creators["icc festergut main tank gastric bloat"] = &RaidIccTriggerContext::icc_festergut_main_tank_gastric_bloat;
        creators["icc festergut spore"] = &RaidIccTriggerContext::icc_festergut_spore;
        creators["icc rotface tank position"] = &RaidIccTriggerContext::icc_rotface_tank_position;
        creators["icc rotface group position"] = &RaidIccTriggerContext::icc_rotface_group_position;
        creators["icc rotface move away from explosion"] = &RaidIccTriggerContext::icc_rotface_move_away_from_explosion;
        creators["icc putricide volatile ooze"] = &RaidIccTriggerContext::icc_putricide_volatile_ooze;
        creators["icc putricide gas cloud"] = &RaidIccTriggerContext::icc_putricide_gas_cloud;
        creators["icc putricide growing ooze puddle"] = &RaidIccTriggerContext::icc_putricide_growing_ooze_puddle;
        creators["icc putricide main tank mutated plague"] = &RaidIccTriggerContext::icc_putricide_main_tank_mutated_plague;
        creators["icc bpc keleseth tank"] = &RaidIccTriggerContext::icc_bpc_keleseth_tank;
        creators["icc bpc nucleus"] = &RaidIccTriggerContext::icc_bpc_nucleus;
        creators["icc bpc main tank"] = &RaidIccTriggerContext::icc_bpc_main_tank;
        creators["icc bpc empowered vortex"] = &RaidIccTriggerContext::icc_bpc_empowered_vortex;
        creators["icc bql tank position"] = &RaidIccTriggerContext::icc_bql_tank_position;
        creators["icc bql pact of darkfallen"] = &RaidIccTriggerContext::icc_bql_pact_of_darkfallen;
        creators["icc bql vampiric bite"] = &RaidIccTriggerContext::icc_bql_vampiric_bite;
    }

private:
    static Trigger* icc_lm_tank_position(PlayerbotAI* ai) { return new IccLmTankPositionTrigger(ai); }
    static Trigger* icc_spike_near(PlayerbotAI* ai) { return new IccSpikeNearTrigger(ai); }
    static Trigger* icc_dark_reckoning(PlayerbotAI* ai) { return new IccDarkReckoningTrigger(ai); }
    static Trigger* icc_ranged_position_lady_deathwhisper(PlayerbotAI* ai) { return new IccRangedPositionLadyDeathwhisperTrigger(ai); }
    static Trigger* icc_adds_lady_deathwhisper(PlayerbotAI* ai) { return new IccAddsLadyDeathwhisperTrigger(ai); }
    static Trigger* icc_shade_lady_deathwhisper(PlayerbotAI* ai) { return new IccShadeLadyDeathwhisperTrigger(ai); }
    static Trigger* icc_rotting_frost_giant_tank_position(PlayerbotAI* ai) { return new IccRottingFrostGiantTankPositionTrigger(ai); }
    static Trigger* icc_in_cannon(PlayerbotAI* ai) { return new IccInCannonTrigger(ai); }
    static Trigger* icc_gunship_cannon_near(PlayerbotAI* ai) { return new IccGunshipCannonNearTrigger(ai); }
    static Trigger* icc_gunship_teleport_ally(PlayerbotAI* ai) { return new IccGunshipTeleportAllyTrigger(ai); }
    static Trigger* icc_gunship_teleport_horde(PlayerbotAI* ai) { return new IccGunshipTeleportHordeTrigger(ai); }
    static Trigger* icc_dbs_tank_position(PlayerbotAI* ai) { return new IccDbsTankPositionTrigger(ai); }
    static Trigger* icc_dbs_main_tank_rune_of_blood(PlayerbotAI* ai) { return new IccDbsMainTankRuneOfBloodTrigger(ai); }
    static Trigger* icc_adds_dbs(PlayerbotAI* ai) { return new IccAddsDbsTrigger(ai); }
    static Trigger* icc_stinky_precious_main_tank_mortal_wound(PlayerbotAI* ai) { return new IccStinkyPreciousMainTankMortalWoundTrigger(ai); }
    static Trigger* icc_festergut_tank_position(PlayerbotAI* ai) { return new IccFestergutTankPositionTrigger(ai); }
    static Trigger* icc_festergut_main_tank_gastric_bloat(PlayerbotAI* ai) { return new IccFestergutMainTankGastricBloatTrigger(ai); }
    static Trigger* icc_festergut_spore(PlayerbotAI* ai) { return new IccFestergutSporeTrigger(ai); }
    static Trigger* icc_rotface_tank_position(PlayerbotAI* ai) { return new IccRotfaceTankPositionTrigger(ai); }
    static Trigger* icc_rotface_group_position(PlayerbotAI* ai) { return new IccRotfaceGroupPositionTrigger(ai); }
    static Trigger* icc_rotface_move_away_from_explosion(PlayerbotAI* ai) { return new IccRotfaceMoveAwayFromExplosionTrigger(ai); }
    static Trigger* icc_putricide_volatile_ooze(PlayerbotAI* ai) { return new IccPutricideVolatileOozeTrigger(ai); }   
    static Trigger* icc_putricide_gas_cloud(PlayerbotAI* ai) { return new IccPutricideGasCloudTrigger(ai); }
    static Trigger* icc_putricide_growing_ooze_puddle(PlayerbotAI* ai) { return new IccPutricideGrowingOozePuddleTrigger(ai); }
    static Trigger* icc_putricide_main_tank_mutated_plague(PlayerbotAI* ai) { return new IccPutricideMainTankMutatedPlagueTrigger(ai); }
    static Trigger* icc_bpc_keleseth_tank(PlayerbotAI* ai) { return new IccBpcKelesethTankTrigger(ai); }
    static Trigger* icc_bpc_nucleus(PlayerbotAI* ai) { return new IccBpcNucleusTrigger(ai); }
    static Trigger* icc_bpc_main_tank(PlayerbotAI* ai) { return new IccBpcMainTankTrigger(ai); }
    static Trigger* icc_bpc_empowered_vortex(PlayerbotAI* ai) { return new IccBpcEmpoweredVortexTrigger(ai); }
    static Trigger* icc_bql_tank_position(PlayerbotAI* ai) { return new IccBqlTankPositionTrigger(ai); }
    static Trigger* icc_bql_pact_of_darkfallen(PlayerbotAI* ai) { return new IccBqlPactOfDarkfallenTrigger(ai); }
    static Trigger* icc_bql_vampiric_bite(PlayerbotAI* ai) { return new IccBqlVampiricBiteTrigger(ai); }
};

#endif
