#ifndef _PLAYERBOT_RAIDMAGTHERIDONACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMAGTHERIDONACTIONCONTEXT_H

#include "RaidMagtheridonActions.h"
#include "NamedObjectContext.h"

class RaidMagtheridonActionContext : public NamedObjectContext<Action>
{
public:
    RaidMagtheridonActionContext()
    {
        creators["magtheridon hellfire channeler main tank"] = &RaidMagtheridonActionContext::magtheridon_hellfire_channeler_main_tank;
        creators["magtheridon hellfire channeler northwest channeler tank"] = &RaidMagtheridonActionContext::magtheridon_hellfire_channeler_nw_channeler_tank;
        creators["magtheridon hellfire channeler northeast channeler tank"] = &RaidMagtheridonActionContext::magtheridon_hellfire_channeler_ne_channeler_tank;
        creators["magtheridon hellfire channeler misdirection"] = &RaidMagtheridonActionContext::magtheridon_hellfire_channeler_misdirection;
        creators["magtheridon hellfire channeler dps priority"] = &RaidMagtheridonActionContext::magtheridon_hellfire_channeler_dps_priority;
        creators["magtheridon hellfire channeler warlock"] = &RaidMagtheridonActionContext::magtheridon_hellfire_channeler_warlock;
        creators["magtheridon position boss"] = &RaidMagtheridonActionContext::magtheridon_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagtheridonActionContext::magtheridon_spread_ranged;
        creators["magtheridon spread healer"] = &RaidMagtheridonActionContext::magtheridon_spread_healer;
        creators["magtheridon use manticron cube"] = &RaidMagtheridonActionContext::magtheridon_use_manticron_cube;
    }

private:
    static Action* magtheridon_hellfire_channeler_main_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMainTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_nw_channeler_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerNWChannelerTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_ne_channeler_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerNEChannelerTankAction(botAI); }
    static Action* magtheridon_hellfire_channeler_misdirection(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMisdirectionAction(botAI); }
    static Action* magtheridon_hellfire_channeler_dps_priority(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerDPSPriorityAction(botAI); }
    static Action* magtheridon_hellfire_channeler_warlock(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerWarlockAction(botAI); }
    static Action* magtheridon_position_boss(PlayerbotAI* botAI) { return new MagtheridonPositionBossAction(botAI); }
    static Action* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedAction(botAI); }
    static Action* magtheridon_spread_healer(PlayerbotAI* botAI) { return new MagtheridonSpreadHealerAction(botAI); }
    static Action* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeAction(botAI); }
};

#endif
