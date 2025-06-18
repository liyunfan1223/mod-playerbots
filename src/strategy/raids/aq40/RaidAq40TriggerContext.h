#ifndef _PLAYERBOT_RAIDAQ40TRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDAQ40TRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "RaidAq40Triggers.h"

class RaidAq40TriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidAq40TriggerContext()
    {
        creators["aq40 should use resistance buffs"] = &RaidAq40TriggerContext::should_use_resistance_buffs;
        creators["aq40 has emperor aggro"] = &RaidAq40TriggerContext::has_emperor_aggro;
        creators["aq40 warlock tank emperor"] = &RaidAq40TriggerContext::warlock_tank_emperor;
        creators["aq40 mage frostbolt viscidus"] = &RaidAq40TriggerContext::mage_frostbolt_viscidus;
        creators["aq40 melee viscidus"] = &RaidAq40TriggerContext::melee_viscidus;
        creators["aq40 target emperor vek'lor"] = &RaidAq40TriggerContext::target_emperor_veklor;
        creators["aq40 target emperor vek'nilash"] = &RaidAq40TriggerContext::target_emperor_veknilash;
        creators["aq40 target emperor pests"] = &RaidAq40TriggerContext::target_emperor_pests;
        creators["aq40 approach emperor vek'lor"] = &RaidAq40TriggerContext::approach_emperor_veklor;
        creators["aq40 approach emperor vek'nilash"] = &RaidAq40TriggerContext::approach_emperor_veknilash;
        creators["aq40 ouro burrowed"] = &RaidAq40TriggerContext::ouro_burrowed;
        creators["aq40 cthun1 started"] = &RaidAq40TriggerContext::cthun1_started;
        creators["aq40 cthun2 started"] = &RaidAq40TriggerContext::cthun2_started;
    }

private:
    static Trigger* should_use_resistance_buffs(PlayerbotAI* ai) { return new Aq40ShouldUseResistanceBuffsTrigger(ai); }
    static Trigger* has_emperor_aggro(PlayerbotAI* ai) { return new Aq40HasEmperorAggroTrigger(ai); }
    static Trigger* warlock_tank_emperor(PlayerbotAI* ai) { return new Aq40WarlockTankEmperorTrigger(ai); }
    static Trigger* mage_frostbolt_viscidus(PlayerbotAI* ai) { return new Aq40MageFrostboltViscidusTrigger(ai); }
    static Trigger* melee_viscidus(PlayerbotAI* ai) { return new Aq40MeleeViscidusTrigger(ai); }
    static Trigger* target_emperor_veklor(PlayerbotAI* ai) { return new Aq40TargetEmperorVekLorTrigger(ai); }
    static Trigger* target_emperor_veknilash(PlayerbotAI* ai) { return new Aq40TargetEmperorVekNilashTrigger(ai); }
    static Trigger* target_emperor_pests(PlayerbotAI* ai) { return new Aq40TargetEmperorPestsTrigger(ai); }
    static Trigger* approach_emperor_veklor(PlayerbotAI* ai) { return new Aq40ApproachEmperorVekLorTrigger(ai); }
    static Trigger* approach_emperor_veknilash(PlayerbotAI* ai) { return new Aq40ApproachEmperorVekNilashTrigger(ai); }
    static Trigger* ouro_burrowed(PlayerbotAI* ai) { return new Aq40OuroBurrowedTrigger(ai); }
    static Trigger* cthun1_started(PlayerbotAI* ai) { return new Aq40Cthun1StartedTrigger(ai); }
    static Trigger* cthun2_started(PlayerbotAI* ai) { return new Aq40Cthun2StartedTrigger(ai); }
};

#endif
