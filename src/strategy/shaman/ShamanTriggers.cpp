/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanTriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "ItemTemplate.h"
#include "Playerbots.h"
#include "TotemsShamanStrategy.h"
#include "InstanceScript.h"
#include "Creature.h"
#include "Unit.h"
#include <ctime>

bool MainHandWeaponNoImbueTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!itemForSpell || itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return false;
    return true;
}

bool OffHandWeaponNoImbueTrigger::IsActive()
{
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!itemForSpell)
        return false;
    uint32 invType = itemForSpell->GetTemplate()->InventoryType;
    bool allowedType = (invType == INVTYPE_WEAPON) || (invType == INVTYPE_WEAPONOFFHAND);
    if (itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) ||
        !allowedType)
        return false;
    return true;
}

bool ShockTrigger::IsActive()
{
    return SpellTrigger::IsActive() && !botAI->HasAura("flame shock", GetTarget(), false, true) &&
           !botAI->HasAura("frost shock", GetTarget(), false, true);
}

// Checks if the target's health is above 25%/1500 hp. Returns false if either are true.
// This logic exists to prevent the use of Earth Shock on bosses as an Elemental Shaman.
bool EarthShockExecuteTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float percent = 100.0f * target->GetHealth() / target->GetMaxHealth();
    if (percent >= 25.0f)
        return false;

    if (target->GetHealth() >= 1500)
        return false;

    return true;
}

bool TotemTrigger::IsActive()
{
    return AI_VALUE(uint8, "attacker count") >= attackerCount && !AI_VALUE2(bool, "has totem", name) &&
           !botAI->HasAura(name, bot);
}

bool WaterWalkingTrigger::IsActive() { return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target"); }

bool WaterBreathingTrigger::IsActive() { return BuffTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target"); }

bool WaterWalkingOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
}

bool WaterBreathingOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && AI_VALUE2(bool, "swimming", "self target");
}

// Checks if Chain Lightning is on Cooldown, and prevents activation if it is.
// This is to ensure that Elemental Mastery is used on Lava Burst (2.0 second cast),
// and not on Chain Lightning (1.5 second cast with talents).
bool ElementalMasteryTrigger::IsActive()
{
    return bot->HasSpellCooldown(421);
}

// Checks if Sprit Wolves are out/if Spirit Walk buff is not on the bot/if the cooldown for Spirit Walk is ready.
// There is custom code for the Spirit Walk cooldown (32 seconds), since no working
// code exists in the AC/Playerbots repo for checking if a guardian's spell is on cooldown.
bool SpiritWalkTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    constexpr uint32 SPIRIT_WOLF = 29264;
    constexpr uint32 SPIRIT_WALK = 58875;
    constexpr int COOLDOWN_SECONDS = 32;

    time_t now = time(nullptr);

    if ((now - lastSpiritWalkTime) < COOLDOWN_SECONDS)
        return false;

    for (Unit* unit : bot->m_Controlled)
    {
        Creature* wolf = dynamic_cast<Creature*>(unit);
        if (wolf && wolf->GetEntry() == SPIRIT_WOLF && wolf->IsAlive())
        {
            if (!bot->HasAura(SPIRIT_WALK))
            {
                lastSpiritWalkTime = now;
                return true;
            }
        }
    }
    return false;
}

// Checks if the bot knows Call of the Elements, and prevents the trigger firing if it doesn't.
// Fires the trigger if at least 2 of the totem slots are empty or out of range.
bool CallOfTheElementsTrigger::IsActive()
{
    if (!botAI->CanCastSpell(SPELL_CALL_OF_THE_ELEMENTS, bot, true))
    {
        return false;
    }

    int emptyCount = 0;
    static const uint8 slots[] = {SUMMON_SLOT_TOTEM_EARTH, SUMMON_SLOT_TOTEM_FIRE, SUMMON_SLOT_TOTEM_WATER,
                                  SUMMON_SLOT_TOTEM_AIR};

    for (uint8 slot : slots)
    {
        ObjectGuid guid = bot->m_SummonSlot[slot];
        bool possibleEmpty = false;

        if (guid.IsEmpty())
        {
            possibleEmpty = true;
        }
        else
        {
            Creature* totem = bot->GetMap()->GetCreature(guid);
            if (!totem || totem->GetDistance(bot) > 30.0f)
            {
                possibleEmpty = true;
            }
        }

        if (!possibleEmpty)
        {
            continue;
        }

        if ((slot == SUMMON_SLOT_TOTEM_EARTH && bot->HasSpell(SPELL_STONESKIN_TOTEM_RANK_1)) ||
            (slot == SUMMON_SLOT_TOTEM_FIRE && bot->HasSpell(SPELL_SEARING_TOTEM_RANK_1)) ||
            (slot == SUMMON_SLOT_TOTEM_WATER && bot->HasSpell(SPELL_HEALING_STREAM_TOTEM_RANK_1)) ||
            (slot == SUMMON_SLOT_TOTEM_AIR && bot->HasSpell(SPELL_NATURE_RESISTANCE_TOTEM_RANK_1)))
        {
            emptyCount++;
        }
    }

    return emptyCount >= 2;
}

// Totemic Recall - Prevents the trigger firing under the following circumstances:
// 1. The bot does not have Totemic Recall (level 30).
// 2. The bot is in an instance and any boss encounter is in progress.
// 3. Any group member or their pet is in combat.
// 4. If Mana Tide Totem or Fire Elemental Totem is active and within 30 yards of the bot.
// 5. Finally, if any totem summon slot is not empty, the trigger will fire.
bool TotemicRecallTrigger::IsActive()
{
    Player* bot = botAI->GetBot();

    if (!bot->HasSpell(SPELL_TOTEMIC_RECALL))
        return false;

    Map* map = bot->GetMap();
    if (map->IsDungeon())
    {
        InstanceScript* instance = ((InstanceMap*)map)->GetInstanceScript();
        if (instance)
        {
            for (uint32 i = 0; i < instance->GetEncounterCount(); ++i)
            {
                if (instance->GetBossState(i) == IN_PROGRESS)
                    return false;
            }
        }
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member)
                continue;
            if (member->IsInCombat())
                return false;
            Pet* pet = member->GetPet();
            if (pet && pet->IsInCombat())
                return false;
        }
    }

    ObjectGuid guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_WATER];
    if (!guid.IsEmpty())
    {
        Creature* totem = bot->GetMap()->GetCreature(guid);
        uint32 currentSpell = 0;
        if (totem)
            currentSpell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);

        for (size_t i = 0; i < MANA_TIDE_TOTEM_COUNT; ++i)
        {
            if (currentSpell == MANA_TIDE_TOTEM[i] && totem && totem->GetDistance(bot) <= 30.0f)
                return false;
        }
    }

    guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_FIRE];
    if (!guid.IsEmpty())
    {
        Creature* totem = bot->GetMap()->GetCreature(guid);
        uint32 currentSpell = 0;
        if (totem)
            currentSpell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);

        for (size_t i = 0; i < FIRE_ELEMENTAL_TOTEM_COUNT; ++i)
        {
            if (currentSpell == FIRE_ELEMENTAL_TOTEM[i] && totem && totem->GetDistance(bot) <= 30.0f)
                return false;
        }
    }

    return !bot->m_SummonSlot[SUMMON_SLOT_TOTEM_EARTH].IsEmpty() ||
           !bot->m_SummonSlot[SUMMON_SLOT_TOTEM_FIRE].IsEmpty() ||
           !bot->m_SummonSlot[SUMMON_SLOT_TOTEM_WATER].IsEmpty() ||
           !bot->m_SummonSlot[SUMMON_SLOT_TOTEM_AIR].IsEmpty();
}

// Find the active totem strategy for this slot, and return the highest-rank spellId the bot knows for it
static uint32 GetRequiredTotemSpellId(PlayerbotAI* ai, const char* strategies[], const uint32* spellList[],
                                      const size_t spellCounts[], size_t numStrategies)
{
    Player* bot = ai->GetBot();
    for (size_t i = 0; i < numStrategies; ++i)
    {
        if (ai->HasStrategy(strategies[i], BOT_STATE_COMBAT))
        {
            // Find the highest-rank spell the bot knows
            for (size_t j = 0; j < spellCounts[i]; ++j)
            {
                if (bot->HasSpell(spellList[i][j]))
                    return spellList[i][j];
            }
        }
    }
    return 0;  // No relevant strategy active, or bot doesn't know any rank
}

// Get the spellId of the currently summoned totem in the slot
static uint32 GetSummonedTotemSpellId(Player* bot, uint8 slot)
{
    ObjectGuid guid = bot->m_SummonSlot[slot];
    if (guid.IsEmpty())
        return 0;
    Creature* totem = bot->GetMap()->GetCreature(guid);
    if (!totem)
        return 0;
    return totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
}

bool NoEarthTotemTrigger::IsActive()
{
    // Check if the bot has Stoneskin Totem (required level 4) and prevents the trigger firing if it doesn't
    if (!bot->HasSpell(SPELL_STONESKIN_TOTEM_RANK_1))
        return false;

    ObjectGuid guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_EARTH];
    Creature* totem = nullptr;
    uint32 currentSpell = 0;
    if (!guid.IsEmpty())
    {
        totem = bot->GetMap()->GetCreature(guid);
        if (totem)
            currentSpell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    }

    // Define supported earth totem strategies for this slot:
    static const char* names[] = {"strength of earth", "stoneskin", "tremor", "earthbind"};
    static const uint32* spells[] = {STRENGTH_OF_EARTH_TOTEM, STONESKIN_TOTEM, TREMOR_TOTEM, EARTHBIND_TOTEM};
    static const size_t counts[] = {STRENGTH_OF_EARTH_TOTEM_COUNT, STONESKIN_TOTEM_COUNT, TREMOR_TOTEM_COUNT,
                                    EARTHBIND_TOTEM_COUNT};

    uint32 requiredSpell = GetRequiredTotemSpellId(botAI, names, spells, counts, 4);

    // EXCEPTION: If Stoneclaw Totem is out and in range, consider the slot "occupied" (do not fire the trigger)
    for (size_t i = 0; i < STONECLAW_TOTEM_COUNT; ++i)
        if (currentSpell == STONECLAW_TOTEM[i] && totem && totem->GetDistance(bot) <= 30.0f)
            return false;

    // If no relevant strategy, only care if the slot is empty or totem is too far away
    if (!requiredSpell)
        return guid.IsEmpty() || !totem || totem->GetDistance(bot) > 30.0f;

    // Fire if slot is empty or wrong totem or totem is too far away
    return !currentSpell || currentSpell != requiredSpell || !totem || totem->GetDistance(bot) > 30.0f;
}

bool NoFireTotemTrigger::IsActive()
{
    // Check if the bot has Searing Totem (required level 10) and prevents the trigger firing if it doesn't
    if (!bot->HasSpell(SPELL_SEARING_TOTEM_RANK_1))
        return false;

    ObjectGuid guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_FIRE];
    Creature* totem = nullptr;
    uint32 currentSpell = 0;
    if (!guid.IsEmpty())
    {
        totem = bot->GetMap()->GetCreature(guid);
        if (totem)
            currentSpell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    }

    // Define supported fire totem strategies for this slot:
    static const char* names[] = {"searing", "magma", "flametongue", "wrath", "frost resistance"};
    static const uint32* spells[] = {SEARING_TOTEM, MAGMA_TOTEM, FLAMETONGUE_TOTEM, TOTEM_OF_WRATH,
                                     FROST_RESISTANCE_TOTEM};
    static const size_t counts[] = {SEARING_TOTEM_COUNT, MAGMA_TOTEM_COUNT, FLAMETONGUE_TOTEM_COUNT, TOTEM_OF_WRATH_COUNT,
                                    FROST_RESISTANCE_TOTEM_COUNT};

    uint32 requiredSpell = GetRequiredTotemSpellId(botAI, names, spells, counts, 5);

    // EXCEPTION: If Fire Elemental is out and in range, consider the slot "occupied" (do not fire the trigger)
    for (size_t i = 0; i < FIRE_ELEMENTAL_TOTEM_COUNT; ++i)
        if (currentSpell == FIRE_ELEMENTAL_TOTEM[i] && totem && totem->GetDistance(bot) <= 30.0f)
            return false;

    // If no relevant strategy, only care if the slot is empty or totem is too far away
    if (!requiredSpell)
        return guid.IsEmpty() || !totem || totem->GetDistance(bot) > 30.0f;

    // Fire if slot is empty or wrong totem or totem is too far away
    return !currentSpell || currentSpell != requiredSpell || !totem || totem->GetDistance(bot) > 30.0f;
}

bool NoWaterTotemTrigger::IsActive()
{
    // Check if the bot has Healing Stream Totem (required level 20) and prevents the trigger firing if it doesn't
    if (!bot->HasSpell(SPELL_HEALING_STREAM_TOTEM_RANK_1))
        return false;

    ObjectGuid guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_WATER];
    Creature* totem = nullptr;
    uint32 currentSpell = 0;
    if (!guid.IsEmpty())
    {
        totem = bot->GetMap()->GetCreature(guid);
        if (totem)
            currentSpell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    }

    // Define supported water totem strategies for this slot:
    static const char* names[] = {"healing stream", "mana spring", "cleansing", "fire resistance"};
    static const uint32* spells[] = {HEALING_STREAM_TOTEM, MANA_SPRING_TOTEM, CLEANSING_TOTEM, FIRE_RESISTANCE_TOTEM};
    static const size_t counts[] = {HEALING_STREAM_TOTEM_COUNT, MANA_SPRING_TOTEM_COUNT, CLEANSING_TOTEM_COUNT,
                                    FIRE_RESISTANCE_TOTEM_COUNT};

    uint32 requiredSpell = GetRequiredTotemSpellId(botAI, names, spells, counts, 4);

    // EXCEPTION: If Mana Tide is out and in range, consider the slot "occupied" (do not fire the trigger)
    for (size_t i = 0; i < MANA_TIDE_TOTEM_COUNT; ++i)
        if (currentSpell == MANA_TIDE_TOTEM[i] && totem && totem->GetDistance(bot) <= 30.0f)
            return false;

    // If no relevant strategy, only care if the slot is empty or totem is too far away
    if (!requiredSpell)
        return guid.IsEmpty() || !totem || totem->GetDistance(bot) > 30.0f;

    // Fire if slot is empty or wrong totem or totem is too far away
    return !currentSpell || currentSpell != requiredSpell || !totem || totem->GetDistance(bot) > 30.0f;
}

bool NoAirTotemTrigger::IsActive()
{
    // Check if the bot has Nature Resistance Totem (required level 30) and prevents the trigger firing if it doesn't
    if (!bot->HasSpell(SPELL_NATURE_RESISTANCE_TOTEM_RANK_1))
        return false;

    ObjectGuid guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_AIR];
    Creature* totem = nullptr;
    uint32 currentSpell = 0;
    if (!guid.IsEmpty())
    {
        totem = bot->GetMap()->GetCreature(guid);
        if (totem)
            currentSpell = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
    }

    // Define supported air totem strategies for this slot:
    static const char* names[] = {"wrath of air", "windfury", "nature resistance", "grounding totem"};
    static const uint32* spells[] = {WRATH_OF_AIR_TOTEM, WINDFURY_TOTEM, NATURE_RESISTANCE_TOTEM, GROUNDING_TOTEM};
    static const size_t counts[] = {WRATH_OF_AIR_TOTEM_COUNT, WINDFURY_TOTEM_COUNT, NATURE_RESISTANCE_TOTEM_COUNT,
                                    GROUNDING_TOTEM_COUNT};

    uint32 requiredSpell = GetRequiredTotemSpellId(botAI, names, spells, counts, 3);

    // If no relevant strategy, only care if the slot is empty or totem is too far away
    if (!requiredSpell)
        return guid.IsEmpty() || !totem || totem->GetDistance(bot) > 30.0f;

    // Fire if slot is empty or wrong totem or totem is too far away
    return !currentSpell || currentSpell != requiredSpell || !totem || totem->GetDistance(bot) > 30.0f;
}

bool SetTotemTrigger::IsActive()
{
    if (!bot->HasSpell(SPELL_CALL_OF_THE_ELEMENTS))
        return false;
    if (!bot->HasSpell(requiredSpellId))
        return false;

    ActionButton const* button = bot->GetActionButton(actionButtonId);
    if (!button || button->GetType() != ACTION_BUTTON_SPELL || button->GetAction() == 0)
        return true;

    size_t totemSpellIdsCount = sizeof(totemSpellIds) / sizeof(uint32);
    for (int i = (int)totemSpellIdsCount - 1; i >= 0; --i)

    {
        if (bot->HasSpell(totemSpellIds[i]))

        {
            return button->GetAction() != totemSpellIds[i];
        }
    }

    return false;
}
