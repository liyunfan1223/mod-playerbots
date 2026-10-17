/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCACTIONCONTEXT_H
#define PLAYERBOTS_ICCACTIONCONTEXT_H

#include "Action.h"
#include "ICCActions.h"
#include "NamedObjectContext.h"

class RaidIccActionContext : public NamedObjectContext<Action>
{
public:
    RaidIccActionContext()
    {
        creators["icc lm tank position"] = &RaidIccActionContext::icc_lm_tank_position;
        creators["icc spike"] = &RaidIccActionContext::icc_spike;

        creators["icc dark reckoning"] = &RaidIccActionContext::icc_dark_reckoning;
        creators["icc ranged position lady deathwhisper"] = &RaidIccActionContext::icc_ranged_position_lady_deathwhisper;
        creators["icc adds lady deathwhisper"] = &RaidIccActionContext::icc_adds_lady_deathwhisper;
        creators["icc shade lady deathwhisper"] = &RaidIccActionContext::icc_shade_lady_deathwhisper;

        creators["icc rotting frost giant tank position"] = &RaidIccActionContext::icc_rotting_frost_giant_tank_position;
        creators["icc cannon fire"] = &RaidIccActionContext::icc_cannon_fire;
        creators["icc gunship enter cannon"] = &RaidIccActionContext::icc_gunship_enter_cannon;
        creators["icc gunship rocket jump"] = &RaidIccActionContext::icc_gunship_rocket_jump;
        creators["icc gunship rocket pack setup"] = &RaidIccActionContext::icc_gunship_rocket_pack_setup;

        creators["icc dbs tank position"] = &RaidIccActionContext::icc_dbs_tank_position;
        creators["icc adds dbs"] = &RaidIccActionContext::icc_adds_dbs;

        creators["icc dogs tank position"] = &RaidIccActionContext::icc_dogs_tank_position;

        creators["icc festergut group position"] = &RaidIccActionContext::icc_festergut_group_position;
        creators["icc festergut spore"] = &RaidIccActionContext::icc_festergut_spore;
        creators["icc festergut avoid malleable goo"] = &RaidIccActionContext::icc_festergut_avoid_malleable_goo;

        creators["icc rotface tank position"] = &RaidIccActionContext::icc_rotface_tank_position;
        creators["icc rotface group position"] = &RaidIccActionContext::icc_rotface_group_position;
        creators["icc rotface move away from explosion"] = &RaidIccActionContext::icc_rotface_move_away_from_explosion;
        creators["icc rotface avoid vile gas"] = &RaidIccActionContext::icc_rotface_avoid_vile_gas;

        creators["icc putricide mutated plague"] = &RaidIccActionContext::icc_putricide_mutated_plague;
        creators["icc putricide volatile ooze"] = &RaidIccActionContext::icc_putricide_volatile_ooze;
        creators["icc putricide gas cloud"] = &RaidIccActionContext::icc_putricide_gas_cloud;
        creators["icc putricide growing ooze puddle"] = &RaidIccActionContext::icc_putricide_growing_ooze_puddle;
        creators["icc putricide avoid malleable goo"] = &RaidIccActionContext::icc_putricide_avoid_malleable_goo;
        creators["icc putricide abomination"] = &RaidIccActionContext::icc_putricide_abomination;

        creators["icc bpc keleseth tank"] = &RaidIccActionContext::icc_bpc_keleseth_tank;
        creators["icc bpc main tank"] = &RaidIccActionContext::icc_bpc_main_tank;
        creators["icc bpc empowered vortex"] = &RaidIccActionContext::icc_bpc_empowered_vortex;
        creators["icc bpc kinetic bomb"] = &RaidIccActionContext::icc_bpc_kinetic_bomb;
        creators["icc bpc ball of flame"] = &RaidIccActionContext::icc_bpc_ball_of_flame;

        creators["icc bql group position"] = &RaidIccActionContext::icc_bql_group_position;
        creators["icc bql pact of darkfallen"] = &RaidIccActionContext::icc_bql_pact_of_darkfallen;
        creators["icc bql vampiric bite"] = &RaidIccActionContext::icc_bql_vampiric_bite;

        creators["icc valkyre spear"] = &RaidIccActionContext::icc_valkyre_spear;
        creators["icc sister svalna"] = &RaidIccActionContext::icc_sister_svalna;

        creators["icc valithria group"] = &RaidIccActionContext::icc_valithria_group;
        creators["icc valithria portal"] = &RaidIccActionContext::icc_valithria_portal;
        creators["icc valithria heal"] = &RaidIccActionContext::icc_valithria_heal;
        creators["icc valithria dream cloud"] = &RaidIccActionContext::icc_valithria_dream_cloud;
        creators["icc valithria zombie kite"] = &RaidIccActionContext::icc_valithria_zombie_kite;

        creators["icc sindragosa group position"] = &RaidIccActionContext::icc_sindragosa_group_position;
        creators["icc sindragosa frost beacon"] = &RaidIccActionContext::icc_sindragosa_frost_beacon;
        creators["icc sindragosa hot"] = &RaidIccActionContext::icc_sindragosa_hot;
        creators["icc sindragosa blistering cold"] = &RaidIccActionContext::icc_sindragosa_blistering_cold;
        creators["icc sindragosa unchained magic"] = &RaidIccActionContext::icc_sindragosa_unchained_magic;
        creators["icc sindragosa chilled to the bone"] = &RaidIccActionContext::icc_sindragosa_chilled_to_the_bone;
        creators["icc sindragosa mystic buffet"] = &RaidIccActionContext::icc_sindragosa_mystic_buffet;
        creators["icc sindragosa frost bomb"] = &RaidIccActionContext::icc_sindragosa_frost_bomb;

        creators["icc lich king shadow trap"] = &RaidIccActionContext::icc_lich_king_shadow_trap;
        creators["icc lich king necrotic plague"] = &RaidIccActionContext::icc_lich_king_necrotic_plague;
        creators["icc lich king winter"] = &RaidIccActionContext::icc_lich_king_winter;
        creators["icc lich king adds"] = &RaidIccActionContext::icc_lich_king_adds;
        creators["icc lich king spirit bomb"] = &RaidIccActionContext::icc_lich_king_spirit_bomb;
    }

private:
    static Action* icc_lm_tank_position(PlayerbotAI* botAI) { return new IccLmTankPositionAction(botAI); }
    static Action* icc_spike(PlayerbotAI* botAI) { return new IccSpikeAction(botAI); }

    static Action* icc_dark_reckoning(PlayerbotAI* botAI) { return new IccDarkReckoningAction(botAI); }
    static Action* icc_ranged_position_lady_deathwhisper(PlayerbotAI* botAI) { return new IccRangedPositionLadyDeathwhisperAction(botAI); }
    static Action* icc_adds_lady_deathwhisper(PlayerbotAI* botAI) { return new IccAddsLadyDeathwhisperAction(botAI); }
    static Action* icc_shade_lady_deathwhisper(PlayerbotAI* botAI) { return new IccShadeLadyDeathwhisperAction(botAI); }

    static Action* icc_rotting_frost_giant_tank_position(PlayerbotAI* botAI) { return new IccRottingFrostGiantTankPositionAction(botAI); }
    static Action* icc_cannon_fire(PlayerbotAI* botAI) { return new IccCannonFireAction(botAI); }
    static Action* icc_gunship_enter_cannon(PlayerbotAI* botAI) { return new IccGunshipEnterCannonAction(botAI); }
    static Action* icc_gunship_rocket_jump(PlayerbotAI* botAI) { return new IccGunshipRocketJumpAction(botAI); }
    static Action* icc_gunship_rocket_pack_setup(PlayerbotAI* botAI) { return new IccGunshipRocketPackSetupAction(botAI); }

    static Action* icc_dbs_tank_position(PlayerbotAI* botAI) { return new IccDbsTankPositionAction(botAI); }
    static Action* icc_adds_dbs(PlayerbotAI* botAI) { return new IccAddsDbsAction(botAI); }

    static Action* icc_dogs_tank_position(PlayerbotAI* botAI) { return new IccDogsTankPositionAction(botAI); }

    static Action* icc_festergut_group_position(PlayerbotAI* botAI) { return new IccFestergutGroupPositionAction(botAI); }
    static Action* icc_festergut_spore(PlayerbotAI* botAI) { return new IccFestergutSporeAction(botAI); }
    static Action* icc_festergut_avoid_malleable_goo(PlayerbotAI* botAI) { return new IccFestergutAvoidMalleableGooAction(botAI); }

    static Action* icc_rotface_tank_position(PlayerbotAI* botAI) { return new IccRotfaceTankPositionAction(botAI); }
    static Action* icc_rotface_group_position(PlayerbotAI* botAI) { return new IccRotfaceGroupPositionAction(botAI); }
    static Action* icc_rotface_move_away_from_explosion(PlayerbotAI* botAI) { return new IccRotfaceMoveAwayFromExplosionAction(botAI); }
    static Action* icc_rotface_avoid_vile_gas(PlayerbotAI* botAI) { return new IccRotfaceAvoidVileGasAction(botAI); }

    static Action* icc_putricide_mutated_plague(PlayerbotAI* botAI) { return new IccPutricideMutatedPlagueAction(botAI); }
    static Action* icc_putricide_volatile_ooze(PlayerbotAI* botAI) { return new IccPutricideVolatileOozeAction(botAI); }
    static Action* icc_putricide_gas_cloud(PlayerbotAI* botAI) { return new IccPutricideGasCloudAction(botAI); }
    static Action* icc_putricide_growing_ooze_puddle(PlayerbotAI* botAI) { return new IccPutricideGrowingOozePuddleAction(botAI); }
    static Action* icc_putricide_avoid_malleable_goo(PlayerbotAI* botAI) { return new IccPutricideAvoidMalleableGooAction(botAI); }
    static Action* icc_putricide_abomination(PlayerbotAI* botAI) { return new IccPutricideAbominationAction(botAI); }

    static Action* icc_bpc_keleseth_tank(PlayerbotAI* botAI) { return new IccBpcKelesethTankAction(botAI); }
    static Action* icc_bpc_main_tank(PlayerbotAI* botAI) { return new IccBpcMainTankAction(botAI); }
    static Action* icc_bpc_empowered_vortex(PlayerbotAI* botAI) { return new IccBpcEmpoweredVortexAction(botAI); }
    static Action* icc_bpc_kinetic_bomb(PlayerbotAI* botAI) { return new IccBpcKineticBombAction(botAI); }
    static Action* icc_bpc_ball_of_flame(PlayerbotAI* botAI) { return new IccBpcBallOfFlameAction(botAI); }

    static Action* icc_bql_group_position(PlayerbotAI* botAI) { return new IccBqlGroupPositionAction(botAI); }
    static Action* icc_bql_pact_of_darkfallen(PlayerbotAI* botAI) { return new IccBqlPactOfDarkfallenAction(botAI); }
    static Action* icc_bql_vampiric_bite(PlayerbotAI* botAI) { return new IccBqlVampiricBiteAction(botAI); }

    static Action* icc_valkyre_spear(PlayerbotAI* botAI) { return new IccValkyreSpearAction(botAI); }
    static Action* icc_sister_svalna(PlayerbotAI* botAI) { return new IccSisterSvalnaAction(botAI); }

    static Action* icc_valithria_group(PlayerbotAI* botAI) { return new IccValithriaGroupAction(botAI); }
    static Action* icc_valithria_portal(PlayerbotAI* botAI) { return new IccValithriaPortalAction(botAI); }
    static Action* icc_valithria_heal(PlayerbotAI* botAI) { return new IccValithriaHealAction(botAI); }
    static Action* icc_valithria_dream_cloud(PlayerbotAI* botAI) { return new IccValithriaDreamCloudAction(botAI); }
    static Action* icc_valithria_zombie_kite(PlayerbotAI* botAI) { return new IccValithriaZombieKiteAction(botAI); }

    static Action* icc_sindragosa_group_position(PlayerbotAI* botAI) { return new IccSindragosaGroupPositionAction(botAI); }
    static Action* icc_sindragosa_frost_beacon(PlayerbotAI* botAI) { return new IccSindragosaFrostBeaconAction(botAI); }
    static Action* icc_sindragosa_hot(PlayerbotAI* botAI) { return new IccSindragosaHotAction(botAI); }
    static Action* icc_sindragosa_blistering_cold(PlayerbotAI* botAI) { return new IccSindragosaBlisteringColdAction(botAI); }
    static Action* icc_sindragosa_unchained_magic(PlayerbotAI* botAI) { return new IccSindragosaUnchainedMagicAction(botAI); }
    static Action* icc_sindragosa_chilled_to_the_bone(PlayerbotAI* botAI) { return new IccSindragosaChilledToTheBoneAction(botAI); }
    static Action* icc_sindragosa_mystic_buffet(PlayerbotAI* botAI) { return new IccSindragosaMysticBuffetAction(botAI); }
    static Action* icc_sindragosa_frost_bomb(PlayerbotAI* botAI) { return new IccSindragosaFrostBombAction(botAI); }

    static Action* icc_lich_king_shadow_trap(PlayerbotAI* botAI) { return new IccLichKingShadowTrapAction(botAI); }
    static Action* icc_lich_king_necrotic_plague(PlayerbotAI* botAI) { return new IccLichKingNecroticPlagueAction(botAI); }
    static Action* icc_lich_king_winter(PlayerbotAI* botAI) { return new IccLichKingWinterAction(botAI); }
    static Action* icc_lich_king_adds(PlayerbotAI* botAI) { return new IccLichKingAddsAction(botAI); }
    static Action* icc_lich_king_spirit_bomb(PlayerbotAI* botAI) { return new IccLichKingSpiritBombAction(botAI); }

};

#endif
