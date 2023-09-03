// /*
//  * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
//  */

#ifndef _PLAYERBOT_RAIDNAXXTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDNAXXTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "RaidNaxxTriggers.h"

class RaidNaxxTriggerContext : public NamedObjectContext<Trigger> 
{
    public:
        RaidNaxxTriggerContext() 
        {
            creators["mutating injection"] = &RaidNaxxTriggerContext::mutating_injection;
            creators["mutating injection removed"] = &RaidNaxxTriggerContext::mutating_injection_removed;
            creators["grobbulus cloud"] = &RaidNaxxTriggerContext::grobbulus_cloud;
            creators["heigan melee"] = &RaidNaxxTriggerContext::heigan_melee;
            creators["heigan ranged"] = &RaidNaxxTriggerContext::heigan_ranged;

            // creators["thaddius phase pet"] = &RaidNaxxTriggerContext::thaddius_phase_pet;
            // creators["thaddius phase pet lose aggro"] = &RaidNaxxTriggerContext::thaddius_phase_pet_lose_aggro;
            // creators["thaddius phase transition"] = &RaidNaxxTriggerContext::thaddius_phase_transition;
            // creators["thaddius phase thaddius"] = &RaidNaxxTriggerContext::thaddius_phase_thaddius;

            creators["razuvious tank"] = &RaidNaxxTriggerContext::razuvious_tank;
            creators["razuvious nontank"] = &RaidNaxxTriggerContext::razuvious_nontank;

            // creators["horseman attractors"] = &RaidNaxxTriggerContext::horseman_attractors;
            // creators["horseman except attractors"] = &RaidNaxxTriggerContext::horseman_except_attractors;

            creators["sapphiron ground"] = &RaidNaxxTriggerContext::sapphiron_ground;
            creators["sapphiron flight"] = &RaidNaxxTriggerContext::sapphiron_flight;
            
            creators["kel'thuzad"] = &RaidNaxxTriggerContext::kelthuzad;
            // creators["kel'thuzad phase two"] = &RaidNaxxTriggerContext::kelthuzad_phase_two;

            creators["anub'rekhan"] = &RaidNaxxTriggerContext::anubrekhan;

            creators["gluth"] = &RaidNaxxTriggerContext::gluth;
            creators["gluth main tank mortal wound"] = &RaidNaxxTriggerContext::gluth_main_tank_mortal_wound;
            
            // creators["loatheb"] = &RaidNaxxTriggerContext::loatheb;
        }
    private:
        static Trigger* mutating_injection(PlayerbotAI* ai) { return new MutatingInjectionTrigger(ai); }
        static Trigger* mutating_injection_removed(PlayerbotAI* ai) { return new MutatingInjectionRemovedTrigger(ai); }
        static Trigger* grobbulus_cloud(PlayerbotAI* ai) { return new GrobbulusCloudTrigger(ai); }
        static Trigger* heigan_melee(PlayerbotAI* ai) { return new HeiganMeleeTrigger(ai); }
        static Trigger* heigan_ranged(PlayerbotAI* ai) { return new HeiganRangedTrigger(ai); }
        // static Trigger* thaddius_phase_pet(PlayerbotAI* ai) { return new ThaddiusPhasePetTrigger(ai); }
        // static Trigger* thaddius_phase_pet_lose_aggro(PlayerbotAI* ai) { return new ThaddiusPhasePetLoseAggroTrigger(ai); }
        // static Trigger* thaddius_phase_transition(PlayerbotAI* ai) { return new ThaddiusPhaseTransitionTrigger(ai); }
        // static Trigger* thaddius_phase_thaddius(PlayerbotAI* ai) { return new ThaddiusPhaseThaddiusTrigger(ai); }
        static Trigger* razuvious_tank(PlayerbotAI* ai) { return new RazuviousTankTrigger(ai); }
        static Trigger* razuvious_nontank(PlayerbotAI* ai) { return new RazuviousNontankTrigger(ai); }
        
        // static Trigger* horseman_attractors(PlayerbotAI* ai) { return new HorsemanAttractorsTrigger(ai); }
        // static Trigger* horseman_except_attractors(PlayerbotAI* ai) { return new HorsemanExceptAttractorsTrigger(ai); }
        static Trigger* sapphiron_ground(PlayerbotAI* ai) { return new SapphironGroundTrigger(ai); }
        static Trigger* sapphiron_flight(PlayerbotAI* ai) { return new SapphironFlightTrigger(ai); }
        // static Trigger* sapphiron_ground_except_main_tank(PlayerbotAI* ai) { return new SapphironGroundExceptMainTankTrigger(ai); }
        // static Trigger* sapphiron_ground_chill(PlayerbotAI* ai) { return new SapphironGroundChillTrigger(ai); }        
        static Trigger* kelthuzad(PlayerbotAI* ai) { return new KelthuzadTrigger(ai); }
        // static Trigger* kelthuzad_phase_two(PlayerbotAI* ai) { return new KelthuzadPhaseTwoTrigger(ai); }
        static Trigger* anubrekhan(PlayerbotAI* ai) { return new AnubrekhanTrigger(ai); }
        static Trigger* gluth(PlayerbotAI* ai) { return new GluthTrigger(ai); }
        static Trigger* gluth_main_tank_mortal_wound(PlayerbotAI* ai) { return new GluthMainTankMortalWoundTrigger(ai); }
        // static Trigger* loatheb(PlayerbotAI* ai) { return new LoathebTrigger(ai); }
};

#endif