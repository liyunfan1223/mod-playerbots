/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDSTRATEGYCONTEXT_H
#define PLAYERBOTS_RAIDSTRATEGYCONTEXT_H

#include "Aq20Strategy.h"
#include "BTStrategy.h"
#include "BWLStrategy.h"
#include "EoEStrategy.h"
#include "GruulStrategy.h"
#include "HyjalStrategy.h"
#include "ICCStrategy.h"
#include "KaraStrategy.h"
#include "MCStrategy.h"
#include "MagStrategy.h"
#include "NaxxStrategy.h"
#include "OSStrategy.h"
#include "OnyStrategy.h"
#include "RSStrategy.h"
#include "SSCStrategy.h"
#include "Strategy.h"
#include "TKStrategy.h"
#include "UldStrategy.h"
#include "VoAStrategy.h"
#include "ZAStrategy.h"

class RaidStrategyContext : public NamedObjectContext<Strategy>
{
public:
    RaidStrategyContext() : NamedObjectContext<Strategy>(false, true)
    {
        creators["aq20"] = &RaidStrategyContext::aq20;
        creators["moltencore"] = &RaidStrategyContext::moltencore;
        creators["bwl"] = &RaidStrategyContext::bwl;
        creators["karazhan"] = &RaidStrategyContext::karazhan;
        creators["gruulslair"] = &RaidStrategyContext::gruulslair;
        creators["magtheridon"] = &RaidStrategyContext::magtheridon;
        creators["naxx"] = &RaidStrategyContext::naxx;
        creators["ssc"] = &RaidStrategyContext::ssc;
        creators["tempestkeep"] = &RaidStrategyContext::tempestkeep;
        creators["hyjal"] = &RaidStrategyContext::hyjal;
        creators["blacktemple"] = &RaidStrategyContext::blacktemple;
        creators["zulaman"] = &RaidStrategyContext::zulaman;
        creators["wotlk-os"] = &RaidStrategyContext::wotlk_os;
        creators["wotlk-eoe"] = &RaidStrategyContext::wotlk_eoe;
        creators["voa"] = &RaidStrategyContext::voa;
        creators["ulduar"] = &RaidStrategyContext::ulduar;
        creators["onyxia"] = &RaidStrategyContext::onyxia;
        creators["icc"] = &RaidStrategyContext::icc;
        creators["rs"] = &RaidStrategyContext::rs;
    }

private:
    static Strategy* aq20(PlayerbotAI* botAI) { return new RaidAq20Strategy(botAI); }
    static Strategy* moltencore(PlayerbotAI* botAI) { return new RaidMcStrategy(botAI); }
    static Strategy* bwl(PlayerbotAI* botAI) { return new RaidBwlStrategy(botAI); }
    static Strategy* karazhan(PlayerbotAI* botAI) { return new RaidKarazhanStrategy(botAI); }
    static Strategy* gruulslair(PlayerbotAI* botAI) { return new RaidGruulsLairStrategy(botAI); }
    static Strategy* magtheridon(PlayerbotAI* botAI) { return new RaidMagtheridonStrategy(botAI); }
    static Strategy* naxx(PlayerbotAI* botAI) { return new RaidNaxxStrategy(botAI); }
    static Strategy* ssc(PlayerbotAI* botAI) { return new RaidSscStrategy(botAI); }
    static Strategy* tempestkeep(PlayerbotAI* botAI) { return new RaidTempestKeepStrategy(botAI); }
    static Strategy* hyjal(PlayerbotAI* botAI) { return new RaidHyjalStrategy(botAI); }
    static Strategy* blacktemple(PlayerbotAI* botAI) { return new RaidBlackTempleStrategy(botAI); }
    static Strategy* zulaman(PlayerbotAI* botAI) { return new RaidZulAmanStrategy(botAI); }
    static Strategy* wotlk_os(PlayerbotAI* botAI) { return new RaidOsStrategy(botAI); }
    static Strategy* wotlk_eoe(PlayerbotAI* botAI) { return new RaidEoEStrategy(botAI); }
    static Strategy* voa(PlayerbotAI* botAI) { return new RaidVoAStrategy(botAI); }
    static Strategy* onyxia(PlayerbotAI* botAI) { return new RaidOnyxiaStrategy(botAI); }
    static Strategy* ulduar(PlayerbotAI* botAI) { return new RaidUlduarStrategy(botAI); }
    static Strategy* icc(PlayerbotAI* botAI) { return new RaidIccStrategy(botAI); }
    static Strategy* rs(PlayerbotAI* botAI) { return new RaidRsStrategy(botAI); }
};

#endif
