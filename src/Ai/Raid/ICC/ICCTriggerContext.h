/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCTRIGGERCONTEXT_H
#define PLAYERBOTS_ICCTRIGGERCONTEXT_H

#include "ICCTriggers.h"
#include "NamedObjectContext.h"

class RaidIccTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidIccTriggerContext()
    {
        creators["icc lm"] = &RaidIccTriggerContext::icc_lm;

        creators["icc dark reckoning"] = &RaidIccTriggerContext::icc_dark_reckoning;
        creators["icc lady deathwhisper"] = &RaidIccTriggerContext::icc_lady_deathwhisper;

        creators["icc rotting frost giant tank position"] = &RaidIccTriggerContext::icc_rotting_frost_giant_tank_position;
        creators["icc in cannon"] = &RaidIccTriggerContext::icc_in_cannon;
        creators["icc gunship cannon near"] = &RaidIccTriggerContext::icc_gunship_cannon_near;
        creators["icc gunship rocket jump"] = &RaidIccTriggerContext::icc_gunship_rocket_jump;
        creators["icc gunship rocket pack setup"] = &RaidIccTriggerContext::icc_gunship_rocket_pack_setup;

        creators["icc dbs"] = &RaidIccTriggerContext::icc_dbs;
        creators["icc dbs main tank rune of blood"] = &RaidIccTriggerContext::icc_dbs_main_tank_rune_of_blood;

        creators["icc dogs"] = &RaidIccTriggerContext::icc_dogs;

        creators["icc festergut group position"] = &RaidIccTriggerContext::icc_festergut_group_position;
        creators["icc festergut spore"] = &RaidIccTriggerContext::icc_festergut_spore;
        creators["icc festergut avoid malleable goo"] = &RaidIccTriggerContext::icc_festergut_avoid_malleable_goo;

        creators["icc rotface tank position"] = &RaidIccTriggerContext::icc_rotface_tank_position;
        creators["icc rotface group position"] = &RaidIccTriggerContext::icc_rotface_group_position;
        creators["icc rotface move away from explosion"] = &RaidIccTriggerContext::icc_rotface_move_away_from_explosion;
        creators["icc rotface avoid vile gas"] = &RaidIccTriggerContext::icc_rotface_avoid_vile_gas;

        creators["icc putricide volatile ooze"] = &RaidIccTriggerContext::icc_putricide_volatile_ooze;
        creators["icc putricide gas cloud"] = &RaidIccTriggerContext::icc_putricide_gas_cloud;
        creators["icc putricide growing ooze puddle"] = &RaidIccTriggerContext::icc_putricide_growing_ooze_puddle;
        creators["icc putricide mutated plague"] = &RaidIccTriggerContext::icc_putricide_mutated_plague;
        creators["icc putricide malleable goo"] = &RaidIccTriggerContext::icc_putricide_malleable_goo;
        creators["icc putricide abomination"] = &RaidIccTriggerContext::icc_putricide_abomination;

        creators["icc bpc keleseth tank"] = &RaidIccTriggerContext::icc_bpc_keleseth_tank;
        creators["icc bpc main tank"] = &RaidIccTriggerContext::icc_bpc_main_tank;
        creators["icc bpc empowered vortex"] = &RaidIccTriggerContext::icc_bpc_empowered_vortex;
        creators["icc bpc kinetic bomb"] = &RaidIccTriggerContext::icc_bpc_kinetic_bomb;
        creators["icc bpc ball of flame"] = &RaidIccTriggerContext::icc_bpc_ball_of_flame;

        creators["icc bql group position"] = &RaidIccTriggerContext::icc_bql_group_position;
        creators["icc bql pact of darkfallen"] = &RaidIccTriggerContext::icc_bql_pact_of_darkfallen;
        creators["icc bql vampiric bite"] = &RaidIccTriggerContext::icc_bql_vampiric_bite;

        creators["icc valkyre spear"] = &RaidIccTriggerContext::icc_valkyre_spear;
        creators["icc sister svalna"] = &RaidIccTriggerContext::icc_sister_svalna;

        creators["icc valithria group"] = &RaidIccTriggerContext::icc_valithria_group;
        creators["icc valithria portal"] = &RaidIccTriggerContext::icc_valithria_portal;
        creators["icc valithria heal"] = &RaidIccTriggerContext::icc_valithria_heal;
        creators["icc valithria dream cloud"] = &RaidIccTriggerContext::icc_valithria_dream_cloud;
        creators["icc valithria zombie kite"] = &RaidIccTriggerContext::icc_valithria_zombie_kite;

        creators["icc sindragosa group position"] = &RaidIccTriggerContext::icc_sindragosa_group_position;
        creators["icc sindragosa frost beacon"] = &RaidIccTriggerContext::icc_sindragosa_frost_beacon;
        creators["icc sindragosa hot"] = &RaidIccTriggerContext::icc_sindragosa_hot;
        creators["icc sindragosa blistering cold"] = &RaidIccTriggerContext::icc_sindragosa_blistering_cold;
        creators["icc sindragosa unchained magic"] = &RaidIccTriggerContext::icc_sindragosa_unchained_magic;
        creators["icc sindragosa chilled to the bone"] = &RaidIccTriggerContext::icc_sindragosa_chilled_to_the_bone;
        creators["icc sindragosa mystic buffet"] = &RaidIccTriggerContext::icc_sindragosa_mystic_buffet;
        creators["icc sindragosa frost bomb"] = &RaidIccTriggerContext::icc_sindragosa_frost_bomb;

        creators["icc lich king shadow trap"] = &RaidIccTriggerContext::icc_lich_king_shadow_trap;
        creators["icc lich king necrotic plague"] = &RaidIccTriggerContext::icc_lich_king_necrotic_plague;
        creators["icc lich king winter"] = &RaidIccTriggerContext::icc_lich_king_winter;
        creators["icc lich king adds"] = &RaidIccTriggerContext::icc_lich_king_adds;
        creators["icc lich king spirit bomb"] = &RaidIccTriggerContext::icc_lich_king_spirit_bomb;
    }

private:
    static Trigger* icc_lm(PlayerbotAI* botAI) { return new IccLmTrigger(botAI); }

    static Trigger* icc_dark_reckoning(PlayerbotAI* botAI) { return new IccDarkReckoningTrigger(botAI); }
    static Trigger* icc_lady_deathwhisper(PlayerbotAI* botAI) { return new IccLadyDeathwhisperTrigger(botAI); }

    static Trigger* icc_rotting_frost_giant_tank_position(PlayerbotAI* botAI) { return new IccRottingFrostGiantTankPositionTrigger(botAI); }
    static Trigger* icc_in_cannon(PlayerbotAI* botAI) { return new IccInCannonTrigger(botAI); }
    static Trigger* icc_gunship_cannon_near(PlayerbotAI* botAI) { return new IccGunshipCannonNearTrigger(botAI); }
    static Trigger* icc_gunship_rocket_jump(PlayerbotAI* botAI) { return new IccGunshipRocketJumpTrigger(botAI); }
    static Trigger* icc_gunship_rocket_pack_setup(PlayerbotAI* botAI) { return new IccGunshipRocketPackSetupTrigger(botAI); }

    static Trigger* icc_dbs(PlayerbotAI* botAI) { return new IccDbsTrigger(botAI); }
    static Trigger* icc_dbs_main_tank_rune_of_blood(PlayerbotAI* botAI) { return new IccDbsMainTankRuneOfBloodTrigger(botAI); }

    static Trigger* icc_dogs(PlayerbotAI* botAI) { return new IccDogsTrigger(botAI); }

    static Trigger* icc_festergut_group_position(PlayerbotAI* botAI) { return new IccFestergutGroupPositionTrigger(botAI); }
    static Trigger* icc_festergut_spore(PlayerbotAI* botAI) { return new IccFestergutSporeTrigger(botAI); }
    static Trigger* icc_festergut_avoid_malleable_goo(PlayerbotAI* botAI) { return new IccFestergutAvoidMalleableGooTrigger(botAI); }

    static Trigger* icc_rotface_tank_position(PlayerbotAI* botAI) { return new IccRotfaceTankPositionTrigger(botAI); }
    static Trigger* icc_rotface_group_position(PlayerbotAI* botAI) { return new IccRotfaceGroupPositionTrigger(botAI); }
    static Trigger* icc_rotface_move_away_from_explosion(PlayerbotAI* botAI) { return new IccRotfaceMoveAwayFromExplosionTrigger(botAI); }
    static Trigger* icc_rotface_avoid_vile_gas(PlayerbotAI* botAI) { return new IccRotfaceAvoidVileGasTrigger(botAI); }

    static Trigger* icc_putricide_volatile_ooze(PlayerbotAI* botAI) { return new IccPutricideVolatileOozeTrigger(botAI); }
    static Trigger* icc_putricide_gas_cloud(PlayerbotAI* botAI) { return new IccPutricideGasCloudTrigger(botAI); }
    static Trigger* icc_putricide_growing_ooze_puddle(PlayerbotAI* botAI) { return new IccPutricideGrowingOozePuddleTrigger(botAI); }
    static Trigger* icc_putricide_mutated_plague(PlayerbotAI* botAI) { return new IccPutricideMutatedPlagueTrigger(botAI); }
    static Trigger* icc_putricide_malleable_goo(PlayerbotAI* botAI) { return new IccPutricideMalleableGooTrigger(botAI); }
    static Trigger* icc_putricide_abomination(PlayerbotAI* botAI) { return new IccPutricideAbominationTrigger(botAI); }

    static Trigger* icc_bpc_keleseth_tank(PlayerbotAI* botAI) { return new IccBpcKelesethTankTrigger(botAI); }
    static Trigger* icc_bpc_main_tank(PlayerbotAI* botAI) { return new IccBpcMainTankTrigger(botAI); }
    static Trigger* icc_bpc_empowered_vortex(PlayerbotAI* botAI) { return new IccBpcEmpoweredVortexTrigger(botAI); }
    static Trigger* icc_bpc_kinetic_bomb(PlayerbotAI* botAI) { return new IccBpcKineticBombTrigger(botAI); }
    static Trigger* icc_bpc_ball_of_flame(PlayerbotAI* botAI) { return new IccBpcBallOfFlameTrigger(botAI); }

    static Trigger* icc_bql_group_position(PlayerbotAI* botAI) { return new IccBqlGroupPositionTrigger(botAI); }
    static Trigger* icc_bql_pact_of_darkfallen(PlayerbotAI* botAI) { return new IccBqlPactOfDarkfallenTrigger(botAI); }
    static Trigger* icc_bql_vampiric_bite(PlayerbotAI* botAI) { return new IccBqlVampiricBiteTrigger(botAI); }

    static Trigger* icc_valkyre_spear(PlayerbotAI* botAI) { return new IccValkyreSpearTrigger(botAI); }
    static Trigger* icc_sister_svalna(PlayerbotAI* botAI) { return new IccSisterSvalnaTrigger(botAI); }

    static Trigger* icc_valithria_group(PlayerbotAI* botAI) { return new IccValithriaGroupTrigger(botAI); }
    static Trigger* icc_valithria_portal(PlayerbotAI* botAI) { return new IccValithriaPortalTrigger(botAI); }
    static Trigger* icc_valithria_heal(PlayerbotAI* botAI) { return new IccValithriaHealTrigger(botAI); }
    static Trigger* icc_valithria_zombie_kite(PlayerbotAI* botAI) { return new IccValithriaZombieKiteTrigger(botAI); }
    static Trigger* icc_valithria_dream_cloud(PlayerbotAI* botAI) { return new IccValithriaDreamCloudTrigger(botAI); }

    static Trigger* icc_sindragosa_group_position(PlayerbotAI* botAI) { return new IccSindragosaGroupPositionTrigger(botAI); }
    static Trigger* icc_sindragosa_frost_beacon(PlayerbotAI* botAI) { return new IccSindragosaFrostBeaconTrigger(botAI); }
    static Trigger* icc_sindragosa_hot(PlayerbotAI* botAI) { return new IccSindragosaHotTrigger(botAI); }
    static Trigger* icc_sindragosa_blistering_cold(PlayerbotAI* botAI) { return new IccSindragosaBlisteringColdTrigger(botAI); }
    static Trigger* icc_sindragosa_unchained_magic(PlayerbotAI* botAI) { return new IccSindragosaUnchainedMagicTrigger(botAI); }
    static Trigger* icc_sindragosa_chilled_to_the_bone(PlayerbotAI* botAI) { return new IccSindragosaChilledToTheBoneTrigger(botAI); }
    static Trigger* icc_sindragosa_mystic_buffet(PlayerbotAI* botAI) { return new IccSindragosaMysticBuffetTrigger(botAI); }
    static Trigger* icc_sindragosa_frost_bomb(PlayerbotAI* botAI) { return new IccSindragosaFrostBombTrigger(botAI); }

    static Trigger* icc_lich_king_shadow_trap(PlayerbotAI* botAI) { return new IccLichKingShadowTrapTrigger(botAI); }
    static Trigger* icc_lich_king_necrotic_plague(PlayerbotAI* botAI) { return new IccLichKingNecroticPlagueTrigger(botAI); }
    static Trigger* icc_lich_king_winter(PlayerbotAI* botAI) { return new IccLichKingWinterTrigger(botAI); }
    static Trigger* icc_lich_king_adds(PlayerbotAI* botAI) { return new IccLichKingAddsTrigger(botAI); }
    static Trigger* icc_lich_king_spirit_bomb(PlayerbotAI* botAI) { return new IccLichKingSpiritBombTrigger(botAI); }

};

#endif
