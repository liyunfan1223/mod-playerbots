/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKMULTIPLIERS_H
#define PLAYERBOTS_TKMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "TKHelpers.h"
#include <string>

// General

class TempestKeepEncounterMultiplier : public Multiplier
{
public:
    TempestKeepEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, TkHelpers::TK_MAP_ID)
            ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

// Al'ar <Phoenix God>

class AlarSuppressGapClosersMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarSuppressGapClosersMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "al'ar suppress gap closers") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarControlMovementMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarControlMovementMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "al'ar control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarDisableAutomaticTargetingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarDisableAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "al'ar disable automatic targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarStayAwayFromRebirthMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarStayAwayFromRebirthMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "al'ar stay away from rebirth") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class AlarControlTauntingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    AlarControlTauntingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "al'ar control taunting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Void Reaver

class VoidReaverMaintainPositionsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    VoidReaverMaintainPositionsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "void reaver maintain positions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// High Astromancer Solarian

class HighAstromancerSolarianDisableMeleeTargetingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    HighAstromancerSolarianDisableMeleeTargetingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "high astromancer solarian disable melee targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighAstromancerSolarianWrathStayAwayMultiplier : public TempestKeepEncounterMultiplier
{
public:
    HighAstromancerSolarianWrathStayAwayMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "high astromancer solarian wrath stay away") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kael'thas Sunstrider <Lord of the Blood Elves>

class KaelthasSunstriderWaitForDpsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderWaitForDpsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderKiteThaladredMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderKiteThaladredMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider kiting thaladred") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderControlMisdirectionMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// class KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier : public TempestKeepEncounterMultiplier
// {
// public:
//     KaelthasSunstriderDisableWarlockTankSoulshatterMultiplier(PlayerbotAI* botAI)
//         : TempestKeepEncounterMultiplier(
//             botAI, "kael'thas sunstrider disable warlock tank soulshatter") {}
//
// protected:
//     float GetValueInEncounter(Action* action) override;
// };

class KaelthasSunstriderKeepDistanceFromCapernianMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderKeepDistanceFromCapernianMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider keep distance from capernian") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderManageWeaponTankingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderManageWeaponTankingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider manage weapon tanking") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderSuppressEquipUpgradeMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderSuppressEquipUpgradeMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider suppress equip upgrade") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderManageAutomaticTargetingMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderManageAutomaticTargetingMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider manage automatic targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderDisableDisperseMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderDisableDisperseMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider disable disperse") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderPrepareForPhase3Multiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderPrepareForPhase3Multiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider prepare for phase 3") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderDelayCooldownsMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderDelayCooldownsMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(botAI, "kael'thas sunstrider delay cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier : public TempestKeepEncounterMultiplier
{
public:
    KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(PlayerbotAI* botAI)
        : TempestKeepEncounterMultiplier(
            botAI, "kael'thas sunstrider stay spread during gravity lapse") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
