/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLTriggers.h"
#include "BWLHelpers.h"
#include "Playerbots.h"

using namespace BlackwingLairHelpers;

// General

bool BwlSuppressionDeviceTrigger::IsActive()
{
    // Until MoP, only rogues could disarm suppression devices.
    // If raid cheats are enabled, any bot can disarm the devices.
    if (botAI->HasCheat(BotCheatMask::raid) || bot->IsClass(CLASS_ROGUE))
    {
        GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
        for (auto i = gos.begin(); i != gos.end(); ++i)
        {
            GameObject const* go = botAI->GetGameObject(*i);
            if (IsActiveSuppressionDeviceInRange(go, bot))
                return true;
        }
    }
    return false;
}

// Razorgore the Untamed

bool BwlRazorgoreNotMindControlledTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "razorgore the untamed"))
        return !boss->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_MINDCONTROL));
    return false;
}

// Vaelastrasz the Corrupt

bool BwlVaelastraszPositioningTrigger::IsActive()
{
    // Prevent non-tanks from rotating the boss while the tanks gain threat.
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "vaelastrasz the corrupt"))
        return boss->GetVictim() != bot;
    return false;
}

bool BwlVaelastraszBurningAdrenalineTrigger::IsActive()
{
    // No check for Vaelastrasz, because bots may still have burning adrenaline even after Vaelastrasz died.
    return bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BURNING_ADRENALINE));
}

// Broodlord Lashlayer

bool BwlBroodlordRangedTooCloseTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (Unit* boss = AI_VALUE2(Unit*, "find target", "broodlord lashlayer"))
    {
        // In case the bot pulled aggro, prevent it from kiting the boss through the room.
        if (boss->GetVictim() != bot)
            return bot->GetDistance2d(boss) < BROODLORD_SAFE_DISTANCE;
    }
    return false;
}

// Firemaw / Ebonroc / Flamegor

bool BwlBlackDrakeNotVictimTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", bossName);
    if (!boss)
        return false;

    // The tank holding the boss stays where it is to avoid rotating the boss.
    return boss->GetVictim() != bot;
}

// Chromaggus

bool BwlAfflictionBronzeTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BROOD_AFFLICTION_BRONZE));
}

// Nefarian

bool BwlNefarianPositioningTrigger::IsActive()
{
    // Prevent non-tanks from rotating the boss while the tanks gain threat.
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "nefarian"))
        return boss->GetVictim() != bot;
    return false;
}

bool BwlNefarianWildMagicTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE &&
        bot->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_WILD_MAGIC));
}

// Trash

bool BwlDeathTalonWyrmguardTankTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "death talon wyrmguard");
}

bool BwlDeathTalonWyrmguardRangedTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "death talon wyrmguard");
}
