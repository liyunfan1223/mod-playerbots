#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H

#include "RaidGruulsLairActions.h"
#include "NamedObjectContext.h"

class RaidGruulsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidGruulsLairActionContext()
    {
        // High King Maulgar
        creators["high king maulgar maulgar tank"] = &RaidGruulsLairActionContext::high_king_maulgar_maulgar_tank;
        creators["high king maulgar olm tank"] = &RaidGruulsLairActionContext::high_king_maulgar_olm_tank;
        creators["high king maulgar blindeye tank"] = &RaidGruulsLairActionContext::high_king_maulgar_blindeye_tank;
        creators["high king maulgar krosh mage tank"] = &RaidGruulsLairActionContext::high_king_maulgar_krosh_mage_tank;
        creators["high king maulgar kiggler moonkin tank"] = &RaidGruulsLairActionContext::high_king_maulgar_kiggler_moonkin_tank;
        creators["high king maulgar melee dps"] = &RaidGruulsLairActionContext::high_king_maulgar_melee_dps;
        creators["high king maulgar ranged dps"] = &RaidGruulsLairActionContext::high_king_maulgar_ranged_dps;
        creators["high king maulgar healer avoidance"] = &RaidGruulsLairActionContext::high_king_maulgar_healer_avoidance;
        creators["high king maulgar whirlwind run away"] = &RaidGruulsLairActionContext::high_king_maulgar_whirlwind_run_away;
        creators["high king maulgar banish felstalker"] = &RaidGruulsLairActionContext::high_king_maulgar_banish_felstalker;
        creators["high king maulgar hunter misdirection"] = &RaidGruulsLairActionContext::high_king_maulgar_hunter_misdirection;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller position boss"] = &RaidGruulsLairActionContext::gruul_the_dragonkiller_position_boss;
        creators["gruul the dragonkiller spread ranged"] = &RaidGruulsLairActionContext::gruul_the_dragonkiller_spread_ranged;
        creators["gruul the dragonkiller shatter spread"] = &RaidGruulsLairActionContext::gruul_the_dragonkiller_shatter_spread;
    }

private:
    // High King Maulgar
    static Action* high_king_maulgar_maulgar_tank(PlayerbotAI* botAI) { return new HighKingMaulgarMaulgarTankAction(botAI); }
    static Action* high_king_maulgar_olm_tank(PlayerbotAI* botAI) { return new HighKingMaulgarOlmTankAction(botAI); }
    static Action* high_king_maulgar_blindeye_tank(PlayerbotAI* botAI) { return new HighKingMaulgarBlindeyeTankAction(botAI); }
    static Action* high_king_maulgar_krosh_mage_tank(PlayerbotAI* botAI) { return new HighKingMaulgarKroshMageTankAction(botAI); }
    static Action* high_king_maulgar_kiggler_moonkin_tank(PlayerbotAI* botAI) { return new HighKingMaulgarKigglerMoonkinTankAction(botAI); }
    static Action* high_king_maulgar_melee_dps(PlayerbotAI* botAI) { return new HighKingMaulgarMeleeDPSAction(botAI); }
    static Action* high_king_maulgar_ranged_dps(PlayerbotAI* botAI) { return new HighKingMaulgarRangedDPSAction(botAI); }
    static Action* high_king_maulgar_healer_avoidance(PlayerbotAI* botAI) { return new HighKingMaulgarHealerAvoidanceAction(botAI); }
    static Action* high_king_maulgar_whirlwind_run_away(PlayerbotAI* botAI) { return new HighKingMaulgarWhirlwindRunAwayAction(botAI); }
    static Action* high_king_maulgar_banish_felstalker(PlayerbotAI* botAI) { return new HighKingMaulgarBanishFelstalkerAction(botAI); }
    static Action* high_king_maulgar_hunter_misdirection(PlayerbotAI* botAI) { return new HighKingMaulgarHunterMisdirectionAction(botAI); }

    // Gruul the Dragonkiller
    static Action* gruul_the_dragonkiller_position_boss(PlayerbotAI* botAI) { return new GruulTheDragonkillerPositionBossAction(botAI); }
    static Action* gruul_the_dragonkiller_spread_ranged(PlayerbotAI* botAI) { return new GruulTheDragonkillerSpreadRangedAction(botAI); }
    static Action* gruul_the_dragonkiller_shatter_spread(PlayerbotAI* botAI) { return new GruulTheDragonkillerShatterSpreadAction(botAI); }
};

#endif
