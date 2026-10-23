/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHACTIONCONTEXT_H
#define PLAYERBOTS_SETHACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SethActions.h"

class TbcDungeonSethekkHallsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonSethekkHallsActionContext()
    {
        creators["time-lost controller mark charming totem"] =
            &TbcDungeonSethekkHallsActionContext::time_lost_controller_mark_charming_totem;

        creators["sethekk prophet set tremor totem"] =
            &TbcDungeonSethekkHallsActionContext::sethekk_prophet_set_tremor_totem;

        creators["darkweaver syth mark elementals"] =
            &TbcDungeonSethekkHallsActionContext::darkweaver_syth_mark_elementals;

        creators["anzu alternate marks on boss"] =
            &TbcDungeonSethekkHallsActionContext::anzu_alternate_marks_on_boss;

        creators["anzu cast heal over time spell on bird spirit"] =
            &TbcDungeonSethekkHallsActionContext::anzu_cast_heal_over_time_spell_on_bird_spirit;

        creators["talon king ikiss position tank near pillar"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_position_tank_near_pillar;

        creators["talon king ikiss ranged move near tank"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_ranged_move_near_tank;

        creators["talon king ikiss los arcane explosion"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_los_arcane_explosion;

        creators["talon king ikiss move within los"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_move_within_los;
    }
private:
    static Action* time_lost_controller_mark_charming_totem(PlayerbotAI* botAI)
    {
        return new TimeLostControllerMarkCharmingTotemAction(botAI);
    }
    static Action* sethekk_prophet_set_tremor_totem(PlayerbotAI* botAI)
    {
        return new SethekkProphetSetTremorTotemAction(botAI);
    }
    static Action* darkweaver_syth_mark_elementals(PlayerbotAI* botAI)
    {
        return new DarkweaverSythMarkElementalsAction(botAI);
    }
    static Action* anzu_alternate_marks_on_boss(PlayerbotAI* botAI)
    {
        return new AnzuAlternateMarksOnBossAction(botAI);
    }
    static Action* anzu_cast_heal_over_time_spell_on_bird_spirit(PlayerbotAI* botAI)
    {
        return new AnzuCastHealOverTimeSpellOnBirdSpiritAction(botAI);
    }
    static Action* talon_king_ikiss_position_tank_near_pillar(PlayerbotAI* botAI)
    {
        return new TalonKingIkissPositionTankNearPillarAction(botAI);
    }
    static Action* talon_king_ikiss_ranged_move_near_tank(PlayerbotAI* botAI)
    {
        return new TalonKingIkissRangedMoveNearTankAction(botAI);
    }
    static Action* talon_king_ikiss_los_arcane_explosion(PlayerbotAI* botAI)
    {
        return new TalonKingIkissLosArcaneExplosionAction(botAI);
    }
    static Action* talon_king_ikiss_move_within_los(PlayerbotAI* botAI)
    {
        return new TalonKingIkissMoveWithinLosAction(botAI);
    }
};

#endif
