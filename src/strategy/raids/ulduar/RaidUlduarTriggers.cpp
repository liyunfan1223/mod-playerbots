#include "RaidUlduarTriggers.h"

#include "EventMap.h"
#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidUlduarBossHelper.h"
#include "RaidUlduarScripts.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"

const std::vector<uint32> availableVehicles = {NPC_VEHICLE_CHOPPER, NPC_SALVAGED_DEMOLISHER,
                                               NPC_SALVAGED_DEMOLISHER_TURRET, NPC_SALVAGED_SIEGE_ENGINE,
                                               NPC_SALVAGED_SIEGE_ENGINE_TURRET};

bool FlameLeviathanOnVehicleTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    for (uint32 comp : availableVehicles)
    {
        if (entry == comp)
            return true;
    }
    return false;
}

bool FlameLeviathanVehicleNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Player* master = botAI->GetMaster();
    if (!master)
        return false;

    if (!master->GetVehicle())
        return false;

    return true;
}

bool RazorscaleFlyingAloneTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
    {
        return false;
    }

    // Check if the boss is flying
    if (boss->GetPositionZ() < RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
    {
        return false;
    }

    // Get the list of attackers
    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    if (attackers.empty())
    {
        return true;  // No attackers implies flying alone
    }

    std::vector<Unit*> dark_rune_adds;

    // Loop through attackers to find dark rune adds
    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();

        // Check for valid dark rune entries
        if (entry == RazorscaleBossHelper::UNIT_DARK_RUNE_WATCHER ||
            entry == RazorscaleBossHelper::UNIT_DARK_RUNE_GUARDIAN ||
            entry == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
        {
            dark_rune_adds.push_back(unit);
        }
    }

    // Return whether there are no dark rune adds
    return dark_rune_adds.empty();
}

bool RazorscaleDevouringFlamesTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DEVOURING_FLAME)
        {
            return true;
        }
    }

    return false;
}

bool RazorscaleAvoidSentinelTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
        {
            return true;
        }
    }

    return false;
}

bool RazorscaleAvoidWhirlwindTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL &&
            (unit->HasAura(RazorscaleBossHelper::SPELL_SENTINEL_WHIRLWIND) ||
             unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL)))
        {
            return true;
        }
    }

    return false;
}

bool RazorscaleGroundedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss)
    {
        return false;
    }

    // Check if the boss is flying
    if (boss->GetPositionZ() < RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
    {
        return true;
    }
    return false;
}

bool RazorscaleHarpoonAvailableTrigger::IsActive()
{
    // Get harpoon data from the helper
    const std::vector<RazorscaleBossHelper::HarpoonData>& harpoonData = RazorscaleBossHelper::GetHarpoonData();

    // Get the boss entity
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Update the boss AI context in the helper
    RazorscaleBossHelper razorscaleHelper(botAI);

    if (!razorscaleHelper.UpdateBossAI())
    {
        return false;
    }

    // Check each harpoon entry
    for (const auto& harpoon : harpoonData)
    {
        // Skip harpoons whose chain spell is already active on the boss
        if (razorscaleHelper.IsHarpoonFired(harpoon.chainSpellId))
        {
            continue;
        }

        // Find the nearest harpoon GameObject within 200 yards
        if (GameObject* harpoonGO = bot->FindNearestGameObject(harpoon.gameObjectEntry, 200.0f))
        {
            if (RazorscaleBossHelper::IsHarpoonReady(harpoonGO))
            {
                return true;  // At least one harpoon is available and ready to be fired
            }
        }
    }

    // No harpoons are available or need to be fired
    return false;
}

bool RazorscaleFuseArmorTrigger::IsActive()
{
    // Get the boss entity
    Unit* boss = AI_VALUE2(Unit*, "find target", "razorscale");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Only proceed if this bot can actually tank
    if (!botAI->IsTank(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Iterate through group members to find the main tank with Fuse Armor
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !botAI->IsMainTank(member))
            continue;

        Aura* fuseArmor = member->GetAura(RazorscaleBossHelper::SPELL_FUSEARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= RazorscaleBossHelper::FUSEARMOR_THRESHOLD)
            return true;
    }

    return false;
}

bool IronAssemblyLightningTendrilsTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "stormcaller brundir");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is within 35 yards of the boss
    if (boss->GetDistance(bot) > 35.0f)
        return false;

    // Check if the boss has the Lightning Tendrils aura
    return boss->HasAura(SPELL_LIGHTNING_TENDRILS_10_MAN) || boss->HasAura(SPELL_LIGHTNING_TENDRILS_25_MAN);
}

bool IronAssemblyOverloadTrigger::IsActive()
{
    // Check if bot is tank
    if (botAI->IsTank(bot))
        return false;

    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "stormcaller brundir");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is within 35 yards of the boss
    if (boss->GetDistance(bot) > 35.0f)
        return false;

    // Check if the boss has the Overload aura
    return boss->HasAura(SPELL_OVERLOAD_10_MAN) || boss->HasAura(SPELL_OVERLOAD_25_MAN) ||
           boss->HasAura(SPELL_OVERLOAD_10_MAN_2) || boss->HasAura(SPELL_OVERLOAD_25_MAN_2);
}

bool IronAssemblyRuneOfPowerTrigger::IsActive()
{
    Unit* target = botAI->GetUnit(bot->GetTarget());
    if (!target || !target->IsAlive())
        return false;

    if (!target->HasAura(SPELL_RUNE_OF_POWER))
        return false;

    if (target->GetVictim() != bot)
        return false;

    return botAI->IsTank(bot);
}

bool KologarnMarkDpsTargetTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");
    if (!boss || !boss->IsAlive())
        return false;

    // Only tank bot can mark target
    if (!botAI->IsTank(bot))
        return false;

    // Get current raid dps target
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int8 skullIndex = 7;
    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkullTarget);

    // Check that rubble is marked
    if (currentSkullUnit && currentSkullUnit->IsAlive() && currentSkullUnit->GetEntry() == NPC_RUBBLE)
    {
        return false;  // Skull marker is already set on rubble
    }

    // Check that there is rubble to mark
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target)
            continue;

        uint32 creatureId = target->GetEntry();
        if (target->GetEntry() == NPC_RUBBLE && target->IsAlive())
        {
            return true;  // Found a rubble to mark
        }
    }

    // Check that right arm is marked
    if (currentSkullUnit && currentSkullUnit->IsAlive() && currentSkullUnit->GetEntry() == NPC_RIGHT_ARM)
    {
        return false;  // Skull marker is already set on right arm
    }

    // Check that there is right arm to mark
    Unit* rightArm = AI_VALUE2(Unit*, "find target", "right arm");
    if (rightArm && rightArm->IsAlive())
    {
        return true;  // Found a right arm to mark
    }

    // Check that main body is marked
    if (currentSkullUnit && currentSkullUnit->IsAlive() && currentSkullUnit->GetEntry() == NPC_KOLOGARN)
    {
        return false;  // Skull marker is already set on main body
    }

    // Main body is not marked
    return true;
}

bool KologarnFallFromFloorTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Check if bot is on the floor
    return bot->GetPositionZ() < ULDUAR_KOLOGARN_AXIS_Z_PATHING_ISSUE_DETECT;
}

bool KologarnRubbleSlowdownTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is hunter
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Check that the current skull mark is set on rubble
    int8 skullIndex = 7;
    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkullTarget);
    if (!currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit->GetEntry() != NPC_RUBBLE)
        return false;

    if (bot->HasSpellCooldown(SPELL_FROST_TRAP))
        return false;

    return true;
}

bool KologarnEyebeamTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");

    if (!triggers.empty())
    {
        for (ObjectGuid const guid : triggers)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                std::string triggerName = unit->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale());

                if (triggerName.rfind("Focused Eyebeam", 0) == 0 &&
                    bot->GetDistance2d(unit) < ULDUAR_KOLOGARN_EYEBEAM_RADIUS + 1.0f)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool KologarnAttackDpsTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    // Get bot's current target
    Unit* currentTarget = botAI->GetUnit(bot->GetTarget());
    if (!currentTarget || !currentTarget->IsAlive())
        return false;

    // Get the current raid marker from the group
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    ObjectGuid skullTarget = group->GetTargetIcon(skullIndex);
    ObjectGuid crossTarget = group->GetTargetIcon(crossIndex);

    if (crossTarget && (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)))
    {
        return currentTarget->GetGUID() != crossTarget;
    }
    else
    {
        return currentTarget->GetGUID() != skullTarget;
    }
}

bool KologarnRtiTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    std::string rtiMark = AI_VALUE(std::string, "rti");

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
        return rtiMark != "cross";

    return rtiMark != "skull";
}

bool KologarnCrunchArmorTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "kologarn");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    return bot->HasAura(SPELL_CRUNCH_ARMOR);
}

bool AuriayaFallFromFloorTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "auriaya");
    if (!boss || !boss->IsAlive())
        return false;

    // Check if bot is on the floor
    return bot->GetPositionZ() < ULDUAR_AURIAYA_AXIS_Z_PATHING_ISSUE_DETECT;
}

bool HodirBitingColdTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "hodir");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    Player* master = botAI->GetMaster();
    if (!master || !master->IsAlive())
        return false;

    return botAI->GetAura("biting cold", bot, false, false, 2) &&
           !botAI->GetAura("biting cold", master, false, false, 2);
}

// Snowpacked Icicle Target
bool HodirNearSnowpackedIcicleTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "hodir");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Check if boss is casting Flash Freeze
    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(SPELL_FLASH_FREEZE))
    {
        return false;
    }

    // Find the nearest Snowpacked Icicle Target
    Creature* target = bot->FindNearestCreature(NPC_SNOWPACKED_ICICLE, 100.0f);
    if (!target)
        return false;

    // Check that bot is stacked on Snowpacked Icicle
    if (bot->GetDistance2d(target->GetPositionX(), target->GetPositionY()) <= 5.0f)
    {
        return false;
    }

    return true;
}

bool FreyaNearNatureBombTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Find the nearest Nature Bomb
    GameObject* target = bot->FindNearestGameObject(GOBJECT_NATURE_BOMB, 12.0f);
    return target != nullptr;
}

bool FreyaMarkDpsTargetTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    if (!boss || !boss->IsAlive())
        return false;

    // Only tank bot can mark target
    if (!botAI->IsTank(bot))
        return false;

    // Get current raid dps target
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    int8 skullIndex = 7;
    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    Unit* currentSkullUnit = botAI->GetUnit(currentSkullTarget);

    if (currentSkullUnit && !currentSkullUnit->IsAlive())
    {
        currentSkullUnit = nullptr;
    }

    // Check which adds is up
    Unit* eonarsGift = nullptr;
    Unit* ancientConservator = nullptr;
    Unit* snaplasher = nullptr;
    Unit* ancientWaterSpirit = nullptr;
    Unit* stormLasher = nullptr;
    Unit* firstDetonatingLasher = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        if (target->GetEntry() == NPC_EONARS_GIFT)
        {
            eonarsGift = target;
        }
        else if (target->GetEntry() == NPC_ANCIENT_CONSERVATOR)
        {
            ancientConservator = target;
        }
        else if (target->GetEntry() == NPC_SNAPLASHER)
        {
            snaplasher = target;
        }
        else if (target->GetEntry() == NPC_ANCIENT_WATER_SPIRIT)
        {
            ancientWaterSpirit = target;
        }
        else if (target->GetEntry() == NPC_STORM_LASHER)
        {
            stormLasher = target;
        }
        else if (target->GetEntry() == NPC_DETONATING_LASHER && !firstDetonatingLasher)
        {
            firstDetonatingLasher = target;
        }
    }

    // Check that eonars gift is need to be mark
    if (eonarsGift && (!currentSkullUnit || currentSkullUnit->GetEntry() != eonarsGift->GetEntry()))
    {
        return true;
    }

    // Check that ancient conservator is need to be mark
    if (ancientConservator && (!currentSkullUnit || currentSkullUnit->GetEntry() != ancientConservator->GetEntry()))
    {
        return true;
    }

    // Check that trio of adds is need to be mark
    if (snaplasher || ancientWaterSpirit || stormLasher)
    {
        Unit* highestHealthUnit = nullptr;
        uint32 highestHealth = 0;

        if (snaplasher && snaplasher->GetHealth() > highestHealth)
        {
            highestHealth = snaplasher->GetHealth();
            highestHealthUnit = snaplasher;
        }
        if (ancientWaterSpirit && ancientWaterSpirit->GetHealth() > highestHealth)
        {
            highestHealth = ancientWaterSpirit->GetHealth();
            highestHealthUnit = ancientWaterSpirit;
        }
        if (stormLasher && stormLasher->GetHealth() > highestHealth)
        {
            highestHealthUnit = stormLasher;
        }

        // If the highest health unit is not already marked, mark it
        if (highestHealthUnit && (!currentSkullUnit || currentSkullUnit->GetEntry() != highestHealthUnit->GetEntry()))
        {
            return true;
        }
    }

    // Check that detonating lasher is need to be mark
    if (firstDetonatingLasher &&
        (!currentSkullUnit || currentSkullUnit->GetEntry() != firstDetonatingLasher->GetEntry()))
    {
        Map* map = bot->GetMap();
        if (!map || !map->IsRaid())
            return false;

        uint32 healthThreshold = map->Is25ManRaid() ? 7200 : 4900;  // Detonate maximum damage

        // Check that detonate lasher dont kill raid members
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->GetHealth() < healthThreshold)
                return false;
        }

        return true;
    }

    return false;
}

bool FreyaMoveToHealingSporeTrigger::IsActive()
{
    // Check for the Freya boss
    Unit* boss = AI_VALUE2(Unit*, "find target", "freya");
    if (!boss || !boss->IsAlive())
        return false;

    if (!botAI->IsRanged(bot))
        return false;

    Unit* conservatory = AI_VALUE2(Unit*, "find target", "ancient conservator");
    if (!conservatory || !conservatory->IsAlive())
        return false;

    GuidVector targets = AI_VALUE(GuidVector, "nearest npcs");
    float nearestDistance = std::numeric_limits<float>::max();
    bool foundSpore = false;

    // Iterate through all targets to find healthy spores
    for (const ObjectGuid& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        // Check if the unit is a healthy spore
        if (unit->GetEntry() == NPC_HEALTHY_SPORE)
        {
            foundSpore = true;
            float distance = bot->GetDistance(unit);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
            }
        }
    }

    // If no healthy spores are found, return false
    if (!foundSpore)
        return false;

    // If the nearest spore is farther than 6 yards, a move is required
    return nearestDistance > 6.0f;
}

bool ThorimUnbalancingStrikeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    return bot->HasAura(SPELL_UNBALANCING_STRIKE);
}

bool ThorimMarkDpsTargetTrigger::IsActive()
{
    if (bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > 110.0f)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (botAI->IsMainTank(bot))
    {
        ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
        Unit* currentSkullUnit = botAI->GetUnit(currentSkullTarget);
        if (currentSkullUnit && !currentSkullUnit->IsAlive())
        {
            currentSkullUnit = nullptr;
        }

        Unit* acolyte = AI_VALUE2(Unit*, "find target", "dark rune acolyte");
        Unit* evoker = AI_VALUE2(Unit*, "find target", "dark rune evoker");

        if (acolyte && acolyte->IsAlive() && bot->GetDistance(acolyte) < 50.0f &&
            (!currentSkullUnit || currentSkullUnit->GetEntry() != acolyte->GetEntry()))
            return true;

        if (evoker && evoker->IsAlive() && bot->GetDistance(evoker) < 50.0f &&
            (!currentSkullUnit || currentSkullUnit->GetEntry() != evoker->GetEntry()))
            return true;

        Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");

        // Check boss and it is alive
        if (!boss || !boss->IsAlive())
            return false;


        if (boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD && (!currentSkullUnit || !currentSkullUnit->IsAlive()))
        {
            group->SetTargetIcon(skullIndex, bot->GetGUID(), boss->GetGUID());
            return true;
        }

        return false;
    }
    else if (botAI->IsAssistTankOfIndex(bot, 0))
    {
        Player* mainTank = nullptr;
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }

        if (mainTank && bot->GetDistance(mainTank) < 30.0f)
            return false;

        ObjectGuid currentCrossTarget = group->GetTargetIcon(crossIndex);
        Unit* currentCrossUnit = botAI->GetUnit(currentCrossTarget);
        if (currentCrossUnit && !currentCrossUnit->IsAlive())
        {
            currentCrossUnit = nullptr;
        }

        Unit* acolyte = AI_VALUE2(Unit*, "find target", "dark rune acolyte");
        if (currentCrossUnit && currentCrossUnit->GetEntry() == NPC_DARK_RUNE_ACOLYTE_I)
            return false;

        Unit* runicColossus = AI_VALUE2(Unit*, "find target", "runic colossus");
        Unit* ancientRuneGiant = AI_VALUE2(Unit*, "find target", "ancient rune giant");
        Unit* ironHonorGuard = AI_VALUE2(Unit*, "find target", "iron ring guard");
        Unit* ironRingGuard = AI_VALUE2(Unit*, "find target", "iron honor guard");

        if (acolyte && acolyte->IsAlive() && (!currentCrossUnit || currentCrossUnit->GetEntry() != acolyte->GetEntry()))
            return true;

        if (currentCrossUnit && currentCrossUnit->GetEntry() == NPC_RUNIC_COLOSSUS)
            return false;
        if (runicColossus && runicColossus->IsAlive() &&
            (!currentCrossUnit || currentCrossUnit->GetEntry() != runicColossus->GetEntry()))
            return true;

        if (currentCrossUnit && currentCrossUnit->GetEntry() == NPC_ANCIENT_RUNE_GIANT)
            return false;
        if (ancientRuneGiant && ancientRuneGiant->IsAlive() &&
            (!currentCrossUnit || currentCrossUnit->GetEntry() != ancientRuneGiant->GetEntry()))
            return true;

        return false;
    }

    return false;
}

bool ThorimGauntletPositioningTrigger::IsActive()
{
    if (bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > 110.0f)
        return false;

    Difficulty raidDifficulty = bot->GetRaidDifficulty();

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    uint32 requiredAssistTankQuantity = 1;
    uint32 requiredHealerQuantity = 0;
    uint32 requiredDpsQuantity = 0;

    if (raidDifficulty == Difficulty::RAID_DIFFICULTY_10MAN_NORMAL)
    {
        requiredDpsQuantity = 3;
        requiredHealerQuantity = 1;
    }
    else if (raidDifficulty == Difficulty::RAID_DIFFICULTY_25MAN_NORMAL)
    {
        requiredDpsQuantity = 7;
        requiredHealerQuantity = 2;
    }

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (requiredDpsQuantity > 0 && botAI->IsDps(member))
        {
            requiredDpsQuantity--;
            if (bot->GetGUID() == member->GetGUID())
                break;
        }

        if (requiredAssistTankQuantity > 0 && botAI->IsAssistTankOfIndex(member, 0))
        {
            requiredAssistTankQuantity--;
            if (bot->GetGUID() == member->GetGUID())
                break;
        }

        if (requiredHealerQuantity > 0 && botAI->IsHeal(member))
        {
            requiredHealerQuantity--;
            if (bot->GetGUID() == member->GetGUID())
                break;
        }

        if (requiredDpsQuantity == 0 && requiredAssistTankQuantity == 0 && requiredHealerQuantity == 0)
            return false;
    }

    Unit* master = botAI->GetMaster();
    if (master->GetDistance(ULDUAR_THORIM_NEAR_ENTRANCE_POSITION) < 10.0f && (bot->GetDistance2d(master) > 5.0f))
    {
        return true;
    }

    if ((master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1) < 6.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2) < 6.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1) < 5.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1) < 10.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2) < 10.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3) < 10.0f) &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_1) > 6.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_6_YARDS_2) > 6.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_5_YARDS_1) > 5.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_1) > 10.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_2) > 10.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_LEFT_SIDE_10_YARDS_3) > 10.0f)
    {
        if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
            return false;

        return true;
    }

    if ((master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1) < 6.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2) < 6.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1) < 5.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1) < 10.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2) < 10.0f ||
         master->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3) < 10.0f) &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_1) > 6.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_6_YARDS_2) > 6.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_5_YARDS_1) > 5.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_1) > 10.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_2) > 10.0f &&
        bot->GetDistance(ULDUAR_THORIM_GAUNTLET_RIGHT_SIDE_10_YARDS_3) > 10.0f)
    {
        if (bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
            return false;

        return true;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");
    if (boss && boss->IsAlive() && bot->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD &&
        boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
    {
        return true;
    }

    return false;
}

bool ThorimArenaPositioningTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    if (boss->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    Difficulty raidDifficulty = bot->GetRaidDifficulty();

    Group* group = bot->GetGroup();
    if (!group)
        return false;
    uint32 requiredAssistTankQuantity = 1;
    uint32 requiredHealerQuantity = 0;
    uint32 requiredDpsQuantity = 0;

    if (raidDifficulty == Difficulty::RAID_DIFFICULTY_10MAN_NORMAL)
    {
        requiredDpsQuantity = 3;
        requiredHealerQuantity = 1;
    }
    else if (raidDifficulty == Difficulty::RAID_DIFFICULTY_25MAN_NORMAL)
    {
        requiredDpsQuantity = 7;
        requiredHealerQuantity = 2;
    }

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (requiredDpsQuantity > 0 && botAI->IsDps(member))
        {
            requiredDpsQuantity--;
            if (bot->GetGUID() == member->GetGUID())
                return false;
        }

        if (requiredAssistTankQuantity > 0 && botAI->IsAssistTankOfIndex(member, 0))
        {
            requiredAssistTankQuantity--;
            if (bot->GetGUID() == member->GetGUID())
                return false;
        }

        if (requiredHealerQuantity > 0 && botAI->IsHeal(member))
        {
            requiredHealerQuantity--;
            if (bot->GetGUID() == member->GetGUID())
                return false;
        }

        if (requiredDpsQuantity == 0 && requiredAssistTankQuantity == 0 && requiredHealerQuantity == 0)
            break;
    }

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* target = nullptr;
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        target = botAI->GetUnit(*i);
        if (!target || !target->IsAlive())
            continue;

        uint32 entry = target->GetEntry();

        if (entry == NPC_DARK_RUNE_ACOLYTE_I || entry == NPC_CAPTURED_MERCENARY_SOLDIER_ALLY ||
            entry == NPC_CAPTURED_MERCENARY_SOLDIER_HORDE || entry == NPC_CAPTURED_MERCENARY_CAPTAIN_ALLY ||
            entry == NPC_CAPTURED_MERCENARY_CAPTAIN_HORDE || entry == NPC_JORMUNGAR_BEHEMOT ||
            entry == NPC_DARK_RUNE_WARBRINGER || entry == NPC_DARK_RUNE_EVOKER || entry == NPC_DARK_RUNE_CHAMPION ||
            entry == NPC_DARK_RUNE_COMMONER)
            return false;
    }

    if (bot && bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > 5.0f)
        return true;

    return false;
}

bool ThorimFallFromFloorTrigger::IsActive()
{
    if (bot->GetDistance(ULDUAR_THORIM_NEAR_ARENA_CENTER) > 110.0f)
        return false;

    // Check if bot is on the floor
    return bot->GetPositionZ() < ULDUAR_THORIM_AXIS_Z_PATHING_ISSUE_DETECT;
}

bool ThorimPhase2PositioningTrigger::IsActive()
{
    if (!botAI->IsRanged(bot) && !botAI->IsMainTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "thorim");

    // Check boss and it is alive
    if (!boss || !boss->IsAlive())
        return false;

    if (boss->GetPositionZ() > ULDUAR_THORIM_AXIS_Z_FLOOR_THRESHOLD)
        return false;

    if (botAI->IsMainTank(bot))
    {
        if (bot->GetDistance(ULDUAR_THORIM_PHASE2_TANK_SPOT) > 1.0f && boss->GetVictim() == bot)
            return true;

        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 memberPositionNumber = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (botAI->IsRanged(member))
        {
            if (bot->GetGUID() == member->GetGUID())
                break;

            memberPositionNumber++;

            if (memberPositionNumber == 3)
                memberPositionNumber = 0;
        }
    }

    if (memberPositionNumber == 0 && bot->GetDistance(ULDUAR_THORIM_PHASE2_RANGE1_SPOT) > 1.0f)
        return true;

    if (memberPositionNumber == 1 && bot->GetDistance(ULDUAR_THORIM_PHASE2_RANGE2_SPOT) > 1.0f)
        return true;

    if (memberPositionNumber == 2 && bot->GetDistance(ULDUAR_THORIM_PHASE2_RANGE3_SPOT) > 1.0f)
        return true;

    return false;
}
