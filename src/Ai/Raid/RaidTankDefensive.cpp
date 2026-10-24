/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidTankDefensive.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"
#include "SpellAuras.h"

#include <array>

namespace
{
constexpr uint32 SPELL_DIVINE_PROTECTION = 498;
// Divine Shield, Divine Protection, Hand of Protection and a self Lay on Hands each lock the other three
constexpr uint32 SPELL_FORBEARANCE = 25771;
// Avenging Wrath's 30 s marker, which locks Divine Protection the same way
constexpr uint32 SPELL_AVENGING_WRATH_MARKER = 61987;

// Cast and aura go by id: the name lookups behind HasAura and CanCastSpell match the DBC string exactly,
// and "anti magic shell" is not "Anti-Magic Shell". No bubbles, a paladin under Divine Shield holds nothing.
struct TankDefensive
{
    uint8 playerClass;
    char const* castName;  // the class context's action name, which the holds match on
    uint32 spellId;
    bool magicOnly;
    uint32 windowMs;    // the buff's own duration, checked against when the hit lands
    uint8 cutPct;       // 0 when it adds health instead of cutting damage
    bool tank;          // offered by NextTankDefensive
    uint32 auraId = 0;  // the buff when the cast is a dummy: Last Stand 12975 puts up 12976

    uint32 AuraSpell() const { return auraId ? auraId : spellId; }
};

// Shortest cooldown first, so the strongest button stays in hand longest
std::array<TankDefensive, 11> const TANK_DEFENSIVES = { {
    { CLASS_WARRIOR, "last stand", 12975, false, 20000, 0, true, 12976 },         // 180s
    { CLASS_WARRIOR, "shield wall", 871, false, 12000, 60, true },                // 300s
    { CLASS_PALADIN, "divine protection", 498, false, 12000, 50, true },          // 180s
    { CLASS_DRUID, "barkskin", 22812, false, 12000, 20, true },                   //  60s
    { CLASS_DRUID, "frenzied regeneration", 22842, false, 10000, 0, true },       // 180s
    { CLASS_DRUID, "survival instincts", 61336, false, 20000, 0, true },          // 180s
    { CLASS_DEATH_KNIGHT, "anti magic shell", 48707, true, 5000, 75, true },      //  45s
    { CLASS_DEATH_KNIGHT, "vampiric blood", 55233, false, 10000, 0, true },       //  60s
    { CLASS_DEATH_KNIGHT, "icebound fortitude", 48792, false, 12000, 30, true },  // 120s
    { CLASS_SHAMAN, "shamanistic rage", 30823, false, 15000, 30, false },         //  60s
    // Cloak cuts the chance a spell lands by 90% rather than the damage, and area damage rolls to hit
    // too, so it dodges most of a magic AoE and nothing of a physical one
    { CLASS_ROGUE, "cloak of shadows", 31224, true, 5000, 90, false },            //  90s
} };

// The row to cast, or the one already running on the bot (running set), or nullptr
TankDefensive const* PickDefensive(PlayerbotAI* botAI, Player* bot, bool physicalOnly, bool tankOnly,
                                   uint32 needMs, bool& running)
{
    running = false;
    if (!botAI || !bot)
        return nullptr;

    TankDefensive const* weakest = nullptr;
    for (TankDefensive const& entry : TANK_DEFENSIVES)
    {
        if (entry.playerClass != bot->getClass() || (physicalOnly && entry.magicOnly))
            continue;

        if (tankOnly && !entry.tank)
            continue;

        // one at a time, stacking the next one spends two buttons on one window
        if (bot->HasAura(entry.AuraSpell()))
        {
            running = true;
            return &entry;
        }

        if (entry.windowMs < needMs)
            continue;

        if (!weakest && botAI->CanCastSpell(entry.spellId, bot))
            weakest = &entry;
    }

    return weakest;
}

uint32 NextDefensive(PlayerbotAI* botAI, Player* bot, bool physicalOnly, bool tankOnly, uint32 needMs)
{
    bool running = false;
    TankDefensive const* picked = PickDefensive(botAI, bot, physicalOnly, tankOnly, needMs, running);
    return picked && !running ? picked->spellId : 0;
}

// Needs no bot AI, so it answers for a human too
TankDefensive const* FindReadyTankDefensive(Player* bot, bool physicalOnly, uint32 withinMs, uint32 needMs)
{
    if (!bot)
        return nullptr;

    for (TankDefensive const& entry : TANK_DEFENSIVES)
    {
        if (!entry.tank || entry.playerClass != bot->getClass() || (physicalOnly && entry.magicOnly) ||
            entry.windowMs < needMs)
            continue;

        if (entry.spellId == SPELL_DIVINE_PROTECTION)
        {
            bool locked = false;
            for (uint32 const lock : {SPELL_FORBEARANCE, SPELL_AVENGING_WRATH_MARKER})
            {
                Aura const* aura = bot->GetAura(lock);
                locked = locked ||
                         (aura && (aura->GetDuration() < 0 || static_cast<uint32>(aura->GetDuration()) > withinMs));
            }

            if (locked)
                continue;
        }

        if (bot->HasSpell(entry.spellId) && bot->GetSpellCooldownDelay(entry.spellId) <= withinMs)
            return &entry;
    }

    return nullptr;
}
}  // namespace

uint32 NextTankDefensive(PlayerbotAI* botAI, Player* bot, bool physicalOnly)
{
    return NextDefensive(botAI, bot, physicalOnly, true, 0);
}

uint32 NextMagicDefensive(PlayerbotAI* botAI, Player* bot, uint32 needMs)
{
    return NextDefensive(botAI, bot, false, false, needMs);
}

uint32 NextPhysicalDefensive(PlayerbotAI* botAI, Player* bot, uint32 needMs)
{
    return NextDefensive(botAI, bot, true, false, needMs);
}

uint8 BestMagicDefensiveCut(PlayerbotAI* botAI, Player* bot, uint32 needMs)
{
    bool running = false;
    TankDefensive const* picked = PickDefensive(botAI, bot, false, false, needMs, running);

    return picked ? picked->cutPct : 0;
}

char const* ReadyTankDefensive(Player* bot, bool physicalOnly, uint32 withinMs)
{
    TankDefensive const* entry = FindReadyTankDefensive(bot, physicalOnly, withinMs, 0);
    return entry ? entry->castName : nullptr;
}

uint32 ReadyTankDefensiveSpell(Player* bot, bool physicalOnly, uint32 withinMs, uint32 needMs)
{
    TankDefensive const* entry = FindReadyTankDefensive(bot, physicalOnly, withinMs, needMs);
    return entry ? entry->spellId : 0;
}

bool IsMagicOnlyTankDefensive(uint32 spellId)
{
    for (TankDefensive const& entry : TANK_DEFENSIVES)
    {
        if (entry.tank && entry.spellId == spellId)
            return entry.magicOnly;
    }

    return false;
}

char const* RunningTankDefensive(Player* bot, bool physicalOnly, uint32 forMs)
{
    if (!bot)
        return nullptr;

    for (TankDefensive const& entry : TANK_DEFENSIVES)
    {
        if (!entry.tank || entry.playerClass != bot->getClass() || (physicalOnly && entry.magicOnly))
            continue;

        Aura const* aura = bot->GetAura(entry.AuraSpell());
        if (aura && (aura->GetDuration() < 0 || static_cast<uint32>(aura->GetDuration()) >= forMs))
            return entry.castName;
    }

    return nullptr;
}

bool IsHeldTankDefensive(std::string const& actionName)
{
    for (TankDefensive const& entry : TANK_DEFENSIVES)
    {
        if (entry.tank && actionName == entry.castName)
            return true;
    }
    return false;
}

std::vector<std::string> HeldMagicDefensiveNames()
{
    std::vector<std::string> names;
    for (TankDefensive const& entry : TANK_DEFENSIVES)
        names.push_back(entry.castName);

    return names;
}
