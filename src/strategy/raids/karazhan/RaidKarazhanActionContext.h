#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_CONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_CONTEXT_H

#include "RaidKarazhanActions.h"
#include "NamedObjectContext.h"

class RaidKarazhanActionContext : public NamedObjectContext<Action>
{
public:
    RaidKarazhanActionContext()
    {
        creators["karazhan attumen the huntsman stack behind"] = &RaidKarazhanActionContext::karazhan_attumen_the_huntsman_stack_behind;

        creators["karazhan moroes mark target"] = &RaidKarazhanActionContext::karazhan_moroes_mark_target;

        creators["karazhan maiden of virtue position boss"] = &RaidKarazhanActionContext::karazhan_maiden_of_virtue_position_boss;
        creators["karazhan maiden of virtue position ranged"] = &RaidKarazhanActionContext::karazhan_maiden_of_virtue_position_ranged;

        creators["karazhan big bad wolf position boss"] = &RaidKarazhanActionContext::karazhan_big_bad_wolf_position_boss;
        creators["karazhan big bad wolf run away"] = &RaidKarazhanActionContext::karazhan_big_bad_wolf_run_away;

        creators["karazhan romulo and julianne mark target"] = &RaidKarazhanActionContext::karazhan_romulo_and_julianne_mark_target;

        creators["karazhan wizard of oz mark target"] = &RaidKarazhanActionContext::karazhan_wizard_of_oz_mark_target;
        creators["karazhan wizard of oz scorch strawman"] = &RaidKarazhanActionContext::karazhan_wizard_of_oz_scorch_strawman;

        creators["karazhan the curator mark target"] = &RaidKarazhanActionContext::karazhan_the_curator_mark_target;
        creators["karazhan the curator position boss"] = &RaidKarazhanActionContext::karazhan_the_curator_position_boss;
        creators["karazhan the curator spread ranged"] = &RaidKarazhanActionContext::karazhan_the_curator_spread_ranged;

        creators["karazhan terestian illhoof mark target"] = &RaidKarazhanActionContext::karazhan_terestian_illhoof_mark_target;

        creators["karazhan shade of aran arcane explosion run away"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_arcane_explosion_run_away;
        creators["karazhan shade of aran flame wreath stop movement"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_flame_wreath_stop_movement;
        creators["karazhan shade of aran mark conjured elemental"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_mark_conjured_elemental;
        creators["karazhan shade of aran spread ranged"] = &RaidKarazhanActionContext::karazhan_shade_of_aran_spread_ranged;

        creators["karazhan netherspite block red beam"] = &RaidKarazhanActionContext::karazhan_netherspite_block_red_beam;
        creators["karazhan netherspite block blue beam"] = &RaidKarazhanActionContext::karazhan_netherspite_block_blue_beam;
        creators["karazhan netherspite block green beam"] = &RaidKarazhanActionContext::karazhan_netherspite_block_green_beam;
        creators["karazhan netherspite avoid beam and void zone"] = &RaidKarazhanActionContext::karazhan_netherspite_avoid_beam_and_void_zone;
        creators["karazhan netherspite banish phase avoid void zone"] = &RaidKarazhanActionContext::karazhan_netherspite_banish_phase_avoid_void_zone;

        creators["karazhan prince malchezaar non tank avoid hazard"] = &RaidKarazhanActionContext::karazhan_prince_malchezaar_non_tank_avoid_hazard;
        creators["karazhan prince malchezaar tank avoid hazard"] = &RaidKarazhanActionContext::karazhan_prince_malchezaar_tank_avoid_hazard;
    }

private:
    static Action* karazhan_attumen_the_huntsman_stack_behind(PlayerbotAI* botAI) { return new KarazhanAttumenTheHuntsmanStackBehindAction(botAI); }

    static Action* karazhan_moroes_mark_target(PlayerbotAI* botAI) { return new KarazhanMoroesMarkTargetAction(botAI); }

    static Action* karazhan_maiden_of_virtue_position_boss(PlayerbotAI* botAI) { return new KarazhanMaidenOfVirtuePositionBossAction(botAI); }
    static Action* karazhan_maiden_of_virtue_position_ranged(PlayerbotAI* botAI) { return new KarazhanMaidenOfVirtuePositionRangedAction(botAI); }

    static Action* karazhan_big_bad_wolf_position_boss(PlayerbotAI* botAI) { return new KarazhanBigBadWolfPositionBossAction(botAI); }
    static Action* karazhan_big_bad_wolf_run_away(PlayerbotAI* botAI) { return new KarazhanBigBadWolfRunAwayAction(botAI); }

    static Action* karazhan_romulo_and_julianne_mark_target(PlayerbotAI* botAI) { return new KarazhanRomuloAndJulianneMarkTargetAction(botAI); }

    static Action* karazhan_wizard_of_oz_mark_target(PlayerbotAI* botAI) { return new KarazhanWizardOfOzMarkTargetAction(botAI); }
    static Action* karazhan_wizard_of_oz_scorch_strawman(PlayerbotAI* botAI) { return new KarazhanWizardOfOzScorchStrawmanAction(botAI); }

    static Action* karazhan_the_curator_mark_target(PlayerbotAI* botAI) { return new KarazhanTheCuratorMarkTargetAction(botAI); }
    static Action* karazhan_the_curator_position_boss(PlayerbotAI* botAI) { return new KarazhanTheCuratorPositionBossAction(botAI); }
    static Action* karazhan_the_curator_spread_ranged(PlayerbotAI* botAI) { return new KarazhanTheCuratorSpreadRangedAction(botAI); }

    static Action* karazhan_terestian_illhoof_mark_target(PlayerbotAI* botAI) { return new KarazhanTerestianIllhoofMarkTargetAction(botAI); }

    static Action* karazhan_shade_of_aran_arcane_explosion_run_away(PlayerbotAI* botAI) { return new KarazhanShadeOfAranArcaneExplosionRunAwayAction(botAI); }
    static Action* karazhan_shade_of_aran_flame_wreath_stop_movement(PlayerbotAI* botAI) { return new KarazhanShadeOfAranFlameWreathStopMovementAction(botAI); }
    static Action* karazhan_shade_of_aran_mark_conjured_elemental(PlayerbotAI* botAI) { return new KarazhanShadeOfAranMarkConjuredElementalAction(botAI); }
    static Action* karazhan_shade_of_aran_spread_ranged(PlayerbotAI* botAI) { return new KarazhanShadeOfAranSpreadRangedAction(botAI); }

    static Action* karazhan_netherspite_block_red_beam(PlayerbotAI* botAI) { return new KarazhanNetherspiteBlockRedBeamAction(botAI); }
    static Action* karazhan_netherspite_block_blue_beam(PlayerbotAI* botAI) { return new KarazhanNetherspiteBlockBlueBeamAction(botAI); }
    static Action* karazhan_netherspite_block_green_beam(PlayerbotAI* botAI) { return new KarazhanNetherspiteBlockGreenBeamAction(botAI); }
    static Action* karazhan_netherspite_avoid_beam_and_void_zone(PlayerbotAI* botAI) { return new KarazhanNetherspiteAvoidBeamAndVoidZoneAction(botAI); }
    static Action* karazhan_netherspite_banish_phase_avoid_void_zone(PlayerbotAI* botAI) { return new KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction(botAI); }

    static Action* karazhan_prince_malchezaar_non_tank_avoid_hazard(PlayerbotAI* botAI) { return new KarazhanPrinceMalchezaarNonTankAvoidHazardAction(botAI); }
    static Action* karazhan_prince_malchezaar_tank_avoid_hazard(PlayerbotAI* botAI) { return new KarazhanPrinceMalchezaarTankAvoidHazardAction(botAI); }
};

#endif
