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
        creators["icc putricide malleable goo"] = &RaidIccTriggerContext::icc_putricide_malleable_goo;
        creators["icc bpc keleseth tank"] = &RaidIccTriggerContext::icc_bpc_keleseth_tank;
        creators["icc bpc nucleus"] = &RaidIccTriggerContext::icc_bpc_nucleus;
        creators["icc bpc main tank"] = &RaidIccTriggerContext::icc_bpc_main_tank;
        creators["icc bpc empowered vortex"] = &RaidIccTriggerContext::icc_bpc_empowered_vortex;
        creators["icc bql tank position"] = &RaidIccTriggerContext::icc_bql_tank_position;
        creators["icc bql pact of darkfallen"] = &RaidIccTriggerContext::icc_bql_pact_of_darkfallen;
        creators["icc bql vampiric bite"] = &RaidIccTriggerContext::icc_bql_vampiric_bite;
        creators["icc valkyre spear"] = &RaidIccTriggerContext::icc_valkyre_spear;
        creators["icc sister svalna"] = &RaidIccTriggerContext::icc_sister_svalna;
        creators["icc valithria portal"] = &RaidIccTriggerContext::icc_valithria_portal;
        creators["icc valithria heal"] = &RaidIccTriggerContext::icc_valithria_heal;
        creators["icc valithria dream cloud"] = &RaidIccTriggerContext::icc_valithria_dream_cloud;
        creators["icc sindragosa tank position"] = &RaidIccTriggerContext::icc_sindragosa_tank_position;    
        creators["icc sindragosa frost beacon"] = &RaidIccTriggerContext::icc_sindragosa_frost_beacon;
        creators["icc sindragosa blistering cold"] = &RaidIccTriggerContext::icc_sindragosa_blistering_cold;    
        creators["icc sindragosa unchained magic"] = &RaidIccTriggerContext::icc_sindragosa_unchained_magic;
        creators["icc sindragosa chilled to the bone"] = &RaidIccTriggerContext::icc_sindragosa_chilled_to_the_bone;
        creators["icc sindragosa mystic buffet"] = &RaidIccTriggerContext::icc_sindragosa_mystic_buffet;
        creators["icc sindragosa main tank mystic buffet"] = &RaidIccTriggerContext::icc_sindragosa_main_tank_mystic_buffet;
        creators["icc sindragosa frost bomb"] = &RaidIccTriggerContext::icc_sindragosa_frost_bomb;
        creators["icc sindragosa tank swap position"] = &RaidIccTriggerContext::icc_sindragosa_tank_swap_position;
        creators["icc lich king shadow trap"] = &RaidIccTriggerContext::icc_lich_king_shadow_trap;
        creators["icc lich king necrotic plague"] = &RaidIccTriggerContext::icc_lich_king_necrotic_plague;
        creators["icc lich king winter"] = &RaidIccTriggerContext::icc_lich_king_winter;
        creators["icc lich king adds"] = &RaidIccTriggerContext::icc_lich_king_adds;
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
    static Trigger* icc_putricide_malleable_goo(PlayerbotAI* ai) { return new IccPutricideMalleableGooTrigger(ai); }
    static Trigger* icc_bpc_keleseth_tank(PlayerbotAI* ai) { return new IccBpcKelesethTankTrigger(ai); }
    static Trigger* icc_bpc_nucleus(PlayerbotAI* ai) { return new IccBpcNucleusTrigger(ai); }
    static Trigger* icc_bpc_main_tank(PlayerbotAI* ai) { return new IccBpcMainTankTrigger(ai); }
    static Trigger* icc_bpc_empowered_vortex(PlayerbotAI* ai) { return new IccBpcEmpoweredVortexTrigger(ai); }
    static Trigger* icc_bql_tank_position(PlayerbotAI* ai) { return new IccBqlTankPositionTrigger(ai); }
    static Trigger* icc_bql_pact_of_darkfallen(PlayerbotAI* ai) { return new IccBqlPactOfDarkfallenTrigger(ai); }
    static Trigger* icc_bql_vampiric_bite(PlayerbotAI* ai) { return new IccBqlVampiricBiteTrigger(ai); }
    static Trigger* icc_valkyre_spear(PlayerbotAI* ai) { return new IccValkyreSpearTrigger(ai); }
    static Trigger* icc_sister_svalna(PlayerbotAI* ai) { return new IccSisterSvalnaTrigger(ai); }
    static Trigger* icc_valithria_portal(PlayerbotAI* ai) { return new IccValithriaPortalTrigger(ai); }
    static Trigger* icc_valithria_heal(PlayerbotAI* ai) { return new IccValithriaHealTrigger(ai); }
    static Trigger* icc_valithria_dream_cloud(PlayerbotAI* ai) { return new IccValithriaDreamCloudTrigger(ai); }
    static Trigger* icc_sindragosa_tank_position(PlayerbotAI* ai) { return new IccSindragosaTankPositionTrigger(ai); }  
    static Trigger* icc_sindragosa_frost_beacon(PlayerbotAI* ai) { return new IccSindragosaFrostBeaconTrigger(ai); }    
    static Trigger* icc_sindragosa_blistering_cold(PlayerbotAI* ai) { return new IccSindragosaBlisteringColdTrigger(ai); }  
    static Trigger* icc_sindragosa_unchained_magic(PlayerbotAI* ai) { return new IccSindragosaUnchainedMagicTrigger(ai); }
    static Trigger* icc_sindragosa_chilled_to_the_bone(PlayerbotAI* ai) { return new IccSindragosaChilledToTheBoneTrigger(ai); }
    static Trigger* icc_sindragosa_mystic_buffet(PlayerbotAI* ai) { return new IccSindragosaMysticBuffetTrigger(ai); }
    static Trigger* icc_sindragosa_main_tank_mystic_buffet(PlayerbotAI* ai) { return new IccSindragosaMainTankMysticBuffetTrigger(ai); }
    static Trigger* icc_sindragosa_frost_bomb(PlayerbotAI* ai) { return new IccSindragosaFrostBombTrigger(ai); }
    static Trigger* icc_sindragosa_tank_swap_position(PlayerbotAI* ai) { return new IccSindragosaTankSwapPositionTrigger(ai); }
    static Trigger* icc_lich_king_shadow_trap(PlayerbotAI* ai) { return new IccLichKingShadowTrapTrigger(ai); }
    static Trigger* icc_lich_king_necrotic_plague(PlayerbotAI* ai) { return new IccLichKingNecroticPlagueTrigger(ai); }
    static Trigger* icc_lich_king_winter(PlayerbotAI* ai) { return new IccLichKingWinterTrigger(ai); }
    static Trigger* icc_lich_king_adds(PlayerbotAI* ai) { return new IccLichKingAddsTrigger(ai); }
    
};

#endif
