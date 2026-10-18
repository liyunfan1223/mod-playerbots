/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_ALGALON_H
#define PLAYERBOTS_ULDTRIGGERS_ALGALON_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "Trigger.h"

// Algalon the Observer. Every trigger reads the per-instance room scan; the Big Bang and Cosmic Smash
// reactions have hard deadlines and run every tick, the rest are paced.

class AlgalonBigBangSoakTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon big bang soak";

    AlgalonBigBangSoakTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonBigBangExternalTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon big bang external";

    AlgalonBigBangExternalTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonSoakerPrepTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon soaker prep";

    AlgalonSoakerPrepTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonSoakerRescueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon soaker rescue";

    AlgalonSoakerRescueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonTankHealCriticalTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon tank heal critical";

    AlgalonTankHealCriticalTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonTankHealTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon tank heal";

    AlgalonTankHealTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonTankBeaconTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon tank beacon";

    AlgalonTankBeaconTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 1000) {}
    bool IsActive() override;
};

class AlgalonTankDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon tank defensive";

    AlgalonTankDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonExplosionCoverTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon explosion cover";

    AlgalonExplosionCoverTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonExplosionPersonalTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon explosion personal";

    AlgalonExplosionPersonalTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonShadowAuraTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon shadow aura";

    AlgalonShadowAuraTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 1000) {}
    bool IsActive() override;
};

class AlgalonBigBangHideTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon big bang hide";

    AlgalonBigBangHideTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonCosmicSmashTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon cosmic smash";

    AlgalonCosmicSmashTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonLeaveBlackHoleTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon leave black hole";

    AlgalonLeaveBlackHoleTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 250) {}
    bool IsActive() override;
};

class AlgalonThreatRedirectTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon threat redirect";

    AlgalonThreatRedirectTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 500) {}
    bool IsActive() override;
};

class AlgalonTankPickupTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon tank pickup";

    AlgalonTankPickupTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AlgalonYieldBossTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon yield boss";

    AlgalonYieldBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 500) {}
    bool IsActive() override;
};

class AlgalonPhasePunchSwapTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon phase punch swap";

    AlgalonPhasePunchSwapTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 2000) {}
    bool IsActive() override;
};

class AlgalonConstellationTauntTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon constellation taunt";

    AlgalonConstellationTauntTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 1000) {}
    bool IsActive() override;
};

class AlgalonDarkMatterTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon dark matter tank";

    AlgalonDarkMatterTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 2000) {}
    bool IsActive() override;
};

class AlgalonConstellationKiteTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon constellation kite";

    AlgalonConstellationKiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 500) {}
    bool IsActive() override;
};

class AlgalonStarTeamTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon star team";

    AlgalonStarTeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 1000) {}
    bool IsActive() override;
};

class AlgalonStarMarkTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon star mark";

    AlgalonStarMarkTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 2000) {}
    bool IsActive() override;
};

class AlgalonRaidPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "algalon raid position";

    AlgalonRaidPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 1000) {}
    bool IsActive() override;
};

#endif
