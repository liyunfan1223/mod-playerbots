#ifndef _PLAYERBOT_RAIDAQ40ACTIONCONTEXT_H
#define _PLAYERBOT_RAIDAQ40ACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidAq40Actions.h"

class RaidAq40ActionContext : public NamedObjectContext<Action>
{
public:
    RaidAq40ActionContext()
    {
        creators["aq40 use resistance buffs"] = &RaidAq40ActionContext::use_resistance_buffs;
        creators["aq40 move from other emperor"] = &RaidAq40ActionContext::move_from_other_emperor;
        creators["aq40 attack emperor vek'lor"] = &RaidAq40ActionContext::attack_emperor_veklor;
        creators["aq40 attack emperor vek'nilash"] = &RaidAq40ActionContext::attack_emperor_veknilash;
        creators["aq40 attack emperor pests"] = &RaidAq40ActionContext::attack_emperor_pests;
        creators["aq40 move towards emperor vek'lor"] = &RaidAq40ActionContext::move_towards_emperor_veklor;
        creators["aq40 move towards emperor vek'nilash"] = &RaidAq40ActionContext::move_towards_emperor_veknilash;
        creators["searing pain"] = &RaidAq40ActionContext::warlock_searing_pain;
        creators["frostbolt"] = &RaidAq40ActionContext::mage_cast_frostbolt;
        creators["aq40 melee viscidus"] = &RaidAq40ActionContext::melee_viscidus;
        creators["aq40 ouro burrowed flee"] = &RaidAq40ActionContext::ouro_burrowed_flee;
        creators["aq40 cthun1 get positioned"] = &RaidAq40ActionContext::cthun1_get_positioned;
        creators["aq40 cthun2 get positioned"] = &RaidAq40ActionContext::cthun2_get_positioned;
    }

private:
    static Action* use_resistance_buffs(PlayerbotAI *ai) { return new Aq40UseResistanceBuffsAction(ai); }
    static Action* move_from_other_emperor(PlayerbotAI* ai) { return new Aq40MoveFromOtherEmperorAction(ai); }
    static Action* attack_emperor_veklor(PlayerbotAI* ai) { return new Aq40AttackEmperorVekLorAction(ai); }
    static Action* attack_emperor_veknilash(PlayerbotAI* ai) { return new Aq40AttackEmperorVekNilashAction(ai); }
    static Action* attack_emperor_pests(PlayerbotAI* ai) { return new Aq40AttackEmperorPestsAction(ai); }
    static Action* move_towards_emperor_veklor(PlayerbotAI* ai) { return (Action *)(new Aq40MoveTowardsEmperorVekLorAction(ai)); }
    static Action* move_towards_emperor_veknilash(PlayerbotAI* ai) { return (Action *)(new Aq40MoveTowardsEmperorVekNilashAction(ai)); }
    static Action* warlock_searing_pain(PlayerbotAI* ai) { return new Aq40WarlockCastSearingPainAction(ai); }
    static Action* mage_cast_frostbolt(PlayerbotAI* ai) { return new Aq40MageCastFrostboltAction(ai); }
    static Action* melee_viscidus(PlayerbotAI* ai) { return new Aq40MeleeViscidusAction(ai); }
    static Action* ouro_burrowed_flee(PlayerbotAI* ai) { return new Aq40OuroBurrowedFleeAction(ai); }
    static Action* cthun1_get_positioned(PlayerbotAI* ai) { return new Aq40Cthun1PositionAction(ai); }
    static Action* cthun2_get_positioned(PlayerbotAI* ai) { return new Aq40Cthun2PositionAction(ai); }
};

#endif
