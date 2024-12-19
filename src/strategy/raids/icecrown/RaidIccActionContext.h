#ifndef _PLAYERBOT_RAIDICCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDICCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidIccActions.h"

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
        creators["icc gunship teleport ally"] = &RaidIccActionContext::icc_gunship_teleport_ally;
        creators["icc gunship teleport horde"] = &RaidIccActionContext::icc_gunship_teleport_horde;
        creators["icc dbs tank position"] = &RaidIccActionContext::icc_dbs_tank_position;
        creators["icc adds dbs"] = &RaidIccActionContext::icc_adds_dbs;
        creators["icc festergut tank position"] = &RaidIccActionContext::icc_festergut_tank_position;
        creators["icc festergut spore"] = &RaidIccActionContext::icc_festergut_spore;
        creators["icc rotface tank position"] = &RaidIccActionContext::icc_rotface_tank_position;
        creators["icc rotface group position"] = &RaidIccActionContext::icc_rotface_group_position;
        creators["icc rotface move away from explosion"] = &RaidIccActionContext::icc_rotface_move_away_from_explosion;
        creators["icc putricide volatile ooze"] = &RaidIccActionContext::icc_putricide_volatile_ooze;
        creators["icc putricide gas cloud"] = &RaidIccActionContext::icc_putricide_gas_cloud;
        creators["icc putricide growing ooze puddle"] = &RaidIccActionContext::icc_putricide_growing_ooze_puddle;
        creators["icc bpc keleseth tank"] = &RaidIccActionContext::icc_bpc_keleseth_tank;
        creators["icc bpc nucleus"] = &RaidIccActionContext::icc_bpc_nucleus;
        creators["icc bpc main tank"] = &RaidIccActionContext::icc_bpc_main_tank;
        creators["icc bpc empowered vortex"] = &RaidIccActionContext::icc_bpc_empowered_vortex;
        creators["icc bql tank position"] = &RaidIccActionContext::icc_bql_tank_position;
        creators["icc bql pact of darkfallen"] = &RaidIccActionContext::icc_bql_pact_of_darkfallen;
        creators["icc bql vampiric bite"] = &RaidIccActionContext::icc_bql_vampiric_bite;
    }

private:
    static Action* icc_lm_tank_position(PlayerbotAI* ai) { return new IccLmTankPositionAction(ai); }
    static Action* icc_spike(PlayerbotAI* ai) { return new IccSpikeAction(ai); }
    static Action* icc_dark_reckoning(PlayerbotAI* ai) { return new IccDarkReckoningAction(ai); }
    static Action* icc_ranged_position_lady_deathwhisper(PlayerbotAI* ai) { return new IccRangedPositionLadyDeathwhisperAction(ai); }
    static Action* icc_adds_lady_deathwhisper(PlayerbotAI* ai) { return new IccAddsLadyDeathwhisperAction(ai); }
    static Action* icc_shade_lady_deathwhisper(PlayerbotAI* ai) { return new IccShadeLadyDeathwhisperAction(ai); }
    static Action* icc_rotting_frost_giant_tank_position(PlayerbotAI* ai) { return new IccRottingFrostGiantTankPositionAction(ai); }
    static Action* icc_cannon_fire(PlayerbotAI* ai) { return new IccCannonFireAction(ai); }
    static Action* icc_gunship_enter_cannon(PlayerbotAI* ai) { return new IccGunshipEnterCannonAction(ai); }
    static Action* icc_gunship_teleport_ally(PlayerbotAI* ai) { return new IccGunshipTeleportAllyAction(ai); }
    static Action* icc_gunship_teleport_horde(PlayerbotAI* ai) { return new IccGunshipTeleportHordeAction(ai); } 
    static Action* icc_dbs_tank_position(PlayerbotAI* ai) { return new IccDbsTankPositionAction(ai); }
    static Action* icc_adds_dbs(PlayerbotAI* ai) { return new IccAddsDbsAction(ai); }
    static Action* icc_festergut_tank_position(PlayerbotAI* ai) { return new IccFestergutTankPositionAction(ai); }
    static Action* icc_festergut_spore(PlayerbotAI* ai) { return new IccFestergutSporeAction(ai); }
    static Action* icc_rotface_tank_position(PlayerbotAI* ai) { return new IccRotfaceTankPositionAction(ai); }
    static Action* icc_rotface_group_position(PlayerbotAI* ai) { return new IccRotfaceGroupPositionAction(ai); }
    static Action* icc_rotface_move_away_from_explosion(PlayerbotAI* ai) { return new IccRotfaceMoveAwayFromExplosionAction(ai); } 
    static Action* icc_putricide_volatile_ooze(PlayerbotAI* ai) { return new IccPutricideVolatileOozeAction(ai); }
    static Action* icc_putricide_gas_cloud(PlayerbotAI* ai) { return new IccPutricideGasCloudAction(ai); }
    static Action* icc_putricide_growing_ooze_puddle(PlayerbotAI* ai) { return new IccPutricideGrowingOozePuddleAction(ai); }
    static Action* icc_bpc_keleseth_tank(PlayerbotAI* ai) { return new IccBpcKelesethTankAction(ai); }
    static Action* icc_bpc_nucleus(PlayerbotAI* ai) { return new IccBpcNucleusAction(ai); }
    static Action* icc_bpc_main_tank(PlayerbotAI* ai) { return new IccBpcMainTankAction(ai); }
    static Action* icc_bpc_empowered_vortex(PlayerbotAI* botAI) { return new IccBpcEmpoweredVortexAction(botAI); }
    static Action* icc_bql_tank_position(PlayerbotAI* ai) { return new IccBqlTankPositionAction(ai); }
    static Action* icc_bql_pact_of_darkfallen(PlayerbotAI* ai) { return new IccBqlPactOfDarkfallenAction(ai); }
    static Action* icc_bql_vampiric_bite(PlayerbotAI* ai) { return new IccBqlVampiricBiteAction(ai); }
    
};

#endif
