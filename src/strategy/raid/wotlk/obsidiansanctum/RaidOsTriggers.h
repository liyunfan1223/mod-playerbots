#ifndef _PLAYERBOT_RAIDOSTRIGGERS_H
#define _PLAYERBOT_RAIDOSTRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

enum ObsidianSanctumIDs
{
    // Bosses
    NPC_SARTHARION                              = 28860,
    NPC_SHADRON                                 = 30451,
    NPC_TENEBRON                                = 30452,
    NPC_VESPERON                                = 30449,

    // Mini-boss shared
    SPELL_SHADOW_BREATH                         = 57570,
    SPELL_SHADOW_FISSURE                        = 57579,
    SPELL_SUMMON_TWILIGHT_WHELP                 = 58035,
    SPELL_GIFT_OF_TWILIGHT_SHADOW               = 57835,
    SPELL_TWILIGHT_TORMENT_VESPERON             = 57935,

    // Sartharion
    SPELL_SARTHARION_CLEAVE                     = 56909,
    SPELL_SARTHARION_FLAME_BREATH               = 56908,
    SPELL_SARTHARION_TAIL_LASH                  = 56910,
    SPELL_CYCLONE_AURA_PERIODIC                 = 57598,
    SPELL_LAVA_STRIKE_DUMMY                     = 57578,
    SPELL_LAVA_STRIKE_DUMMY_TRIGGER             = 57697,
    SPELL_LAVA_STRIKE_SUMMON                    = 57572,
    SPELL_SARTHARION_PYROBUFFET                 = 56916,
    SPELL_SARTHARION_BERSERK                    = 61632,
    SPELL_SARTHARION_TWILIGHT_REVENGE           = 60639,

    // Sartharion with drakes
    SPELL_WILL_OF_SARTHARION                    = 61254,
    SPELL_POWER_OF_TENEBRON                     = 61248,
    SPELL_POWER_OF_VESPERON                     = 61251,
    SPELL_POWER_OF_SHADRON                      = 58105,
    SPELL_GIFT_OF_TWILIGHT_FIRE                 = 58766,

    // Visuals
    SPELL_EGG_MARKER_VISUAL                     = 58547,
    SPELL_FLAME_TSUNAMI_VISUAL                  = 57494,

    // Misc
    SPELL_FADE_ARMOR                            = 60708,
    SPELL_FLAME_TSUNAMI_DAMAGE_AURA             = 57492,
    SPELL_FLAME_TSUNAMI_LEAP                    = 60241,
    SPELL_SARTHARION_PYROBUFFET_TRIGGER         = 57557,

    NPC_TWILIGHT_EGG                            = 30882,
    NPC_TWILIGHT_WHELP                          = 30890,
    NPC_DISCIPLE_OF_SHADRON                     = 30688,
    NPC_DISCIPLE_OF_VESPERON                    = 30858,
    NPC_ACOLYTE_OF_SHADRON                      = 31218,
    NPC_ACOLYTE_OF_VESPERON                     = 31219,

    // Sartharion fight
    NPC_LAVA_BLAZE                              = 30643,
    NPC_FLAME_TSUNAMI                           = 30616,
    NPC_SAFE_AREA_TRIGGER                       = 30494,
    NPC_TWILIGHT_FISSURE                        = 30641,
    GO_TWILIGHT_PORTAL                          = 193988,
    GO_NORMAL_PORTAL                            = 193989,
    SPELL_TWILIGHT_SHIFT                        = 57874,
};

const uint32 OS_MAP_ID = 615;

class SartharionTankTrigger : public Trigger
{
public:
    SartharionTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sartharion tank") {}
    bool IsActive() override;
};

class FlameTsunamiTrigger : public Trigger
{
public:
    FlameTsunamiTrigger(PlayerbotAI* botAI) : Trigger(botAI, "flame tsunami") {}
    bool IsActive() override;
};

class TwilightFissureTrigger : public Trigger
{
public:
    TwilightFissureTrigger(PlayerbotAI* botAI) : Trigger(botAI, "twilight fissure") {}
    bool IsActive() override;
};

class SartharionDpsTrigger : public Trigger
{
public:
    SartharionDpsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sartharion dps") {}
    bool IsActive() override;
};

class SartharionMeleePositioningTrigger : public Trigger
{
public:
    SartharionMeleePositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sartharion melee positioning") {}
    bool IsActive() override;
};

class TwilightPortalEnterTrigger : public Trigger
{
public:
    TwilightPortalEnterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "twilight portal enter") {}
    bool IsActive() override;
};

class TwilightPortalExitTrigger : public Trigger
{
public:
    TwilightPortalExitTrigger(PlayerbotAI* botAI) : Trigger(botAI, "twilight portal exit") {}
    bool IsActive() override;
};

#endif
