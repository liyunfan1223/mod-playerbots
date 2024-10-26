#ifndef _PLAYERBOT_DUNGEONSTRATEGYCONTEXT_H
#define _PLAYERBOT_DUNGEONSTRATEGYCONTEXT_H

#include "Strategy.h"
#include "wotlk/utgardekeep/UtgardeKeepStrategy.h"
#include "wotlk/nexus/NexusStrategy.h"
#include "wotlk/azjolnerub/AzjolNerubStrategy.h"
#include "wotlk/oldkingdom/OldKingdomStrategy.h"
#include "wotlk/draktharonkeep/DrakTharonKeepStrategy.h"
#include "wotlk/violethold/VioletHoldStrategy.h"
#include "wotlk/gundrak/GundrakStrategy.h"
#include "wotlk/hallsofstone/HallsOfStoneStrategy.h"
#include "wotlk/hallsoflightning/HallsOfLightningStrategy.h"
#include "wotlk/oculus/OculusStrategy.h"
#include "wotlk/utgardepinnacle/UtgardePinnacleStrategy.h"
#include "wotlk/cullingofstratholme/CullingOfStratholmeStrategy.h"

/*
Full list/TODO:

Trial of the Champion - ToC
Alliance Champions: Deathstalker Visceri, Eressea Dawnsinger, Mokra the Skullcrusher, Runok Wildmane, Zul'tore
Horde Champions: Ambrose Boltspark, Colosos, Jacob Alerius, Jaelyne Evensong, Lana Stouthammer
Argent Champion: Argent Confessor Paletress/Eadric the Pure
The Black Knight
Halls of Reflection - HoR
Falric, Marwyn, The Lich King
Pit of Saron - PoS
Forgemaster Garfrost, Krick & Ick, Scourgelord Tyrannus
The Forge of Souls - FoS
Bronjahm, Devourer of Souls

*/



class DungeonStrategyContext : public NamedObjectContext<Strategy>
{
    public:
        DungeonStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            // Vanilla
            // ...

            // Burning Crusade
            // ...
            
            // Wrath of the Lich King
            creators["wotlk-uk"] = &DungeonStrategyContext::wotlk_uk;       // Utgarde Keep
            creators["wotlk-nex"] = &DungeonStrategyContext::wotlk_nex;     // The Nexus
            creators["wotlk-an"] = &DungeonStrategyContext::wotlk_an;       // Azjol-Nerub
            creators["wotlk-ok"] = &DungeonStrategyContext::wotlk_ok;       // Ahn'kahet: The Old Kingdom
            creators["wotlk-dtk"] = &DungeonStrategyContext::wotlk_dtk;     // Drak'Tharon Keep
            creators["wotlk-vh"] = &DungeonStrategyContext::wotlk_vh;       // The Violet Hold
            creators["wotlk-gd"] = &DungeonStrategyContext::wotlk_gd;       // Gundrak
            creators["wotlk-hos"] = &DungeonStrategyContext::wotlk_hos;     // Halls of Stone
            creators["wotlk-hol"] = &DungeonStrategyContext::wotlk_hol;     // Halls of Lightning
            creators["wotlk-occ"] = &DungeonStrategyContext::wotlk_occ;     // The Oculus
            creators["wotlk-up"] = &DungeonStrategyContext::wotlk_up;       // Utgarde Pinnacle
            creators["wotlk-cos"] = &DungeonStrategyContext::wotlk_cos;     // The Culling of Stratholme
            creators["wotlk-toc"] = &DungeonStrategyContext::wotlk_toc;     // Trial of the Champion
            creators["wotlk-hor"] = &DungeonStrategyContext::wotlk_hor;     // Halls of Reflection
            creators["wotlk-pos"] = &DungeonStrategyContext::wotlk_pos;     // Pit of Saron
            creators["wotlk-fos"] = &DungeonStrategyContext::wotlk_fos;     // The Forge of Souls
        }
    private:
        static Strategy* wotlk_uk(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
        static Strategy* wotlk_nex(PlayerbotAI* botAI) { return new WotlkDungeonNexStrategy(botAI); }
        static Strategy* wotlk_an(PlayerbotAI* botAI) { return new WotlkDungeonANStrategy(botAI); }
        static Strategy* wotlk_ok(PlayerbotAI* botAI) { return new WotlkDungeonOKStrategy(botAI); }
        static Strategy* wotlk_dtk(PlayerbotAI* botAI) { return new WotlkDungeonDTKStrategy(botAI); }
        static Strategy* wotlk_vh(PlayerbotAI* botAI) { return new WotlkDungeonVHStrategy(botAI); }
        static Strategy* wotlk_gd(PlayerbotAI* botAI) { return new WotlkDungeonGDStrategy(botAI); }
        static Strategy* wotlk_hos(PlayerbotAI* botAI) { return new WotlkDungeonHoSStrategy(botAI); }
        static Strategy* wotlk_hol(PlayerbotAI* botAI) { return new WotlkDungeonHoLStrategy(botAI); }
        static Strategy* wotlk_occ(PlayerbotAI* botAI) { return new WotlkDungeonOccStrategy(botAI); }
        static Strategy* wotlk_up(PlayerbotAI* botAI) { return new WotlkDungeonUPStrategy(botAI); }
        static Strategy* wotlk_cos(PlayerbotAI* botAI) { return new WotlkDungeonCoSStrategy(botAI); }
        // NYI from here down
        static Strategy* wotlk_toc(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
        static Strategy* wotlk_hor(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
        static Strategy* wotlk_pos(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
        static Strategy* wotlk_fos(PlayerbotAI* botAI) { return new WotlkDungeonUKStrategy(botAI); }
};

#endif
