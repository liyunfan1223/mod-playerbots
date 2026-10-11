/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"

using namespace EncounterHelpers;

namespace BlackTempleHelpers
{

// High Warlord Naj'entus
const Position NAJENTUS_TANK_POSITION = { 438.515f, 772.436f, 11.931f };

// Supremus
std::unordered_map<uint32, time_t> supremusPhaseTimer;

bool HasSupremusVolcanoNearby(PlayerbotAI*, Player* bot)
{
    constexpr float searchRadius = 20.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(
            BlackTempleNpcs::NPC_SUPREMUS_VOLCANO), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            return true;
    }

    return false;
}

// Shade of Akama
const Position AKAMA_CHANNELER_POSITION = { 467.851f, 401.622f, 118.538f };

std::unordered_set<ObjectGuid> hasReachedAkamaChannelerPosition;

// Teron Gorefiend
const Position GOREFIEND_TANK_POSITION  = { 597.653f, 402.284f, 187.090f };
const Position GOREFIEND_DIE_POSITION   = { 525.709f, 377.177f, 193.203f };

// Gurtogg Bloodboil
const Position GURTOGG_TANK_POSITION    = { 735.987f, 272.451f, 063.554f };
const Position GURTOGG_RANGED_POSITION  = { 762.265f, 277.183f, 063.781f };
const Position GURTOGG_SOAKER_POSITION  = { 769.348f, 280.116f, 063.780f };

std::unordered_map<uint32, time_t> gurtoggPhaseTimer;

std::vector<std::vector<Player*>> GetGurtoggRangedRotationGroups(Player* bot)
{
    Group* group = bot->GetGroup();
    std::vector<Player*> rangedMembers;
    std::vector<std::vector<Player*>> groups(3);

    if (!group)
        return groups;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive())
        {
            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (memberAI && PlayerbotAI::IsRanged(member))
                rangedMembers.push_back(member);
        }
    }

    for (size_t i = 0; i < rangedMembers.size(); ++i)
    {
        groups[i / 5].push_back(rangedMembers[i]);
        if (groups[2].size() == 5)
            break;
    }

    return groups;
}

int GetGurtoggActiveRotationGroup(Unit* gurtogg)
{
    if (!gurtogg)
        return -1;

    auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
    if (it == gurtoggPhaseTimer.end())
        return -1;

    const time_t now = std::time(nullptr);
    const time_t elapsed = now - it->second;
    const int groupIndex = (elapsed % 30) / 10; // 3 groups, swapping every 10 seconds

    return groupIndex;
}

// Mother Shahraz
const Position SHAHRAZ_TANK_POSITION       = { 960.438f, 178.989f, 192.826f };
const Position SHAHRAZ_TRANSITION_POSITION = { 951.327f, 179.550f, 192.550f };
const Position SHAHRAZ_RANGED_POSITION     = { 935.267f, 175.459f, 192.821f };
std::unordered_map<ObjectGuid, TankPositionState> shahrazTankStep;

TankPositionState GetShahrazTankPositionState(Player* bot)
{
    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank)
        return TankPositionState::Unknown;

    auto it = shahrazTankStep.find(mainTank->GetGUID());
    if (it != shahrazTankStep.end())
        return it->second;

    return TankPositionState::Unknown;
}

// Illidari Council
const std::array<Position, 4> GATHIOS_TANK_POSITIONS = {{
    { 662.977f, 296.246f, 271.688f },
    { 636.238f, 283.719f, 271.629f },
    { 655.571f, 261.377f, 271.687f },
    { 673.789f, 274.139f, 271.689f }
}};
const Position ZEREVOR_TANK_POSITION = { 686.219f, 377.644f, 271.689f };
const std::array<Position, 2> ZEREVOR_HEALER_POSITIONS = {{
    { 661.385f, 351.219f, 271.690f },
    { 667.003f, 363.768f, 271.690f }
}};
const Position MALANDE_TANK_POSITION = { 690.590f, 299.790f, 277.443f };

std::unordered_map<uint32, time_t> councilDpsWaitTimer;
std::unordered_map<ObjectGuid, uint8> gathiosTankStep;
std::unordered_map<ObjectGuid, uint8> zerevorHealStep;

// (1) First priority is an assistant Mage (real player or bot)
// (2) If no assistant Mage, then look for any Mage bot
Player* GetZerevorMageTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackMage = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
            continue;

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackMage && GET_PLAYERBOT_AI(member))
            fallbackMage = member;
    }

    return fallbackMage;
}

bool HasDangerousCouncilAura(Unit* unit)
{
    static const std::array<uint32, 3> dangerousAuras =
    {
        static_cast<uint32>(BlackTempleSpells::SPELL_CONSECRATION),
        static_cast<uint32>(BlackTempleSpells::SPELL_BLIZZARD),
        static_cast<uint32>(BlackTempleSpells::SPELL_FLAMESTRIKE)
    };

    for (uint32 aura : dangerousAuras)
    {
        if (unit->HasAura(aura))
            return true;
    }

    return false;
}

// Illidan Stormrage <The Betrayer>
const Position ILLIDAN_LANDING_POSITION = { 676.648f, 304.761f, 354.189f };
const Position ILLIDAN_N_GRATE_POSITION = { 682.100f, 306.000f, 353.192f };
const Position ILLIDAN_E_GRATE_POSITION = { 673.500f, 298.500f, 353.192f };
const Position ILLIDAN_W_GRATE_POSITION = { 672.400f, 312.500f, 353.192f };
const std::array<Position, 3> GRATE_POSITIONS = {{
    ILLIDAN_N_GRATE_POSITION,
    ILLIDAN_E_GRATE_POSITION,
    ILLIDAN_W_GRATE_POSITION
}};

const Position ILLIDAN_E_GLAIVE_WAITING_POSITION = { 677.656f, 294.066f, 353.192f };
const std::array<Position, 7> E_GLAIVE_TANK_POSITIONS = {{
    { 683.000f, 295.000f, 354.000f },
    { 696.969f, 300.982f, 354.302f },
    { 691.112f, 287.461f, 354.363f },
    { 676.674f, 280.797f, 354.268f },
    { 664.414f, 284.834f, 354.271f },
    { 656.826f, 295.113f, 354.165f },
    { 665.000f, 304.000f, 354.000f }
}};

const Position ILLIDAN_W_GLAIVE_WAITING_POSITION = { 676.102f, 316.305f, 353.192f };
const std::array<Position, 7> W_GLAIVE_TANK_POSITIONS = {{
    { 697.208f, 313.475f, 354.234f },
    { 681.000f, 318.000f, 354.000f },
    { 664.000f, 307.000f, 354.000f },
    { 656.161f, 314.132f, 354.092f },
    { 665.080f, 326.905f, 354.128f },
    { 678.809f, 329.968f, 354.387f },
    { 690.889f, 324.277f, 354.204f }
}};

std::unordered_map<ObjectGuid, size_t> flameTankWaypointIndex;
std::unordered_map<ObjectGuid, ObjectGuid> illidanShadowTrapGuid;
std::unordered_map<ObjectGuid, Position> illidanShadowTrapDestination;
std::unordered_map<uint32, int> illidanLastPhase;
std::unordered_map<uint32, time_t> illidanBossDpsWaitTimer;
std::unordered_map<uint32, time_t> illidanFlameDpsWaitTimer;
std::unordered_map<uint32, ObjectGuid> eastFlameGuid;
std::unordered_map<uint32, ObjectGuid> westFlameGuid;

int GetIllidanPhase(Unit* illidan)
{
    if (!illidan || illidan->GetHealth() == 1 || illidan->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_SHADOW_PRISON)))
    {
        return -1;
    }

    // Transitioning from Phase 2 to Phase 3
    float x, y, z;
    illidan->GetMotionMaster()->GetDestination(x, y, z);
    Position dest(x, y, z);
    if ((dest.GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f ||
            illidan->GetExactDist2d(ILLIDAN_LANDING_POSITION) < 0.2f) &&
            illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return 0;
    }

    // Phase 2: Flying
    if (illidan->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 2;

    // Phase 1: Health > 65%
    if (illidan->GetHealthPct() > 65.0f)
        return 1;

    // Phase 4: Demon Form
    if (!illidan->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_CAGED)) &&
        (illidan->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_DEMON_FORM)) ||
            illidan->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_DEMON_TRANSFORM_1)) ||
            illidan->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_DEMON_TRANSFORM_2)) ||
            illidan->HasAura(static_cast<uint32>(BlackTempleSpells::SPELL_DEMON_TRANSFORM_3))))
    {
        return 4;
    }

    // Phase 3: Normal (ground, 65-30%, not demon)
    if (illidan->GetHealthPct() > 30.0f)
        return 3;

    // Phase 5: Health <= 30%
    if (illidan->GetHealthPct() <= 30.0f)
        return 5;

    return -1;
}

std::vector<Unit*> GetAllFlameCrashes(Player* bot)
{
    std::vector<Unit*> flameCrashes;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(BlackTempleNpcs::NPC_FLAME_CRASH), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            flameCrashes.push_back(creature);
    }

    return flameCrashes;
}

std::pair<Unit*, Unit*> GetFlamesOfAzzinoth(Player* bot)
{
    Unit* eastFlame = nullptr;
    Unit* westFlame = nullptr;

    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    if (eastFlameGuid.find(instanceId) != eastFlameGuid.end())
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, eastFlameGuid[instanceId]))
        {
            if (unit->IsAlive())
                eastFlame = unit;
        }
    }

    if (westFlameGuid.find(instanceId) != westFlameGuid.end())
    {
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, westFlameGuid[instanceId]))
        {
            if (unit->IsAlive())
                westFlame = unit;
        }
    }

    return { eastFlame, westFlame };
}

// (1) First priority is an assistant Warlock (real player or bot)
// (2) If no assistant Warlock, then look for any Warlock bot
Player* GetIllidanWarlockTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* fallbackWarlock = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
            continue;

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!fallbackWarlock && GET_PLAYERBOT_AI(member))
            fallbackWarlock = member;
    }

    return fallbackWarlock;
}

bool HasParasiticShadowfiend(Player* member)
{
    if (!member)
        return false;

    constexpr uint32 shadowfiendAura1 =
        static_cast<uint32>(BlackTempleSpells::SPELL_PARASITIC_SHADOWFIEND_1);
    constexpr uint32 shadowfiendAura2 =
        static_cast<uint32>(BlackTempleSpells::SPELL_PARASITIC_SHADOWFIEND_2);

    return member->HasAura(shadowfiendAura1) || member->HasAura(shadowfiendAura2);
}

// Get the first bot hunter that doesn't have Parasitic Shadowfiend
Player* GetIllidanTrapperHunter(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
            return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member) && !HasParasiticShadowfiend(member))
        {
            return member;
        }
    }

        return nullptr;
}

Player* GetBotWithParasiticShadowfiend(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) &&
            HasParasiticShadowfiend(member))
        {
            return member;
        }
    }

    return nullptr;
}

EyeBlastDangerArea GetEyeBlastDangerArea(Player* bot)
{
    constexpr float searchRadius = 100.0f;
    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(BlackTempleNpcs::NPC_ILLIDAN_DB_TARGET), searchRadius);

    Creature* eyeBlastTrigger = nullptr;
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
        {
            eyeBlastTrigger = creature;
            break;
        }
    }

    if (!eyeBlastTrigger)
        return {};

    Position startPos = Position(eyeBlastTrigger->GetPositionX(), eyeBlastTrigger->GetPositionY(),
                                    eyeBlastTrigger->GetPositionZ());

    float destX, destY, destZ;
    eyeBlastTrigger->GetMotionMaster()->GetDestination(destX, destY, destZ);
    Position endPos(destX, destY, destZ);

    if (startPos.GetExactDist2d(endPos) < 0.1f)
        return {};

    constexpr float eyeBlastWidth = 9.0f;
    return { startPos, endPos, eyeBlastWidth };
}

bool IsPositionInEyeBlastDangerArea(Position const& pos, EyeBlastDangerArea const& area)
{
    const float dx = area.end.GetPositionX() - area.start.GetPositionX();
    const float dy = area.end.GetPositionY() - area.start.GetPositionY();
    const float length = area.start.GetExactDist2d(area.end.GetPositionX(), area.end.GetPositionY());

    if (length < 0.1f)
        return false;

    const float projectionFactor = (
        (pos.GetPositionX() - area.start.GetPositionX()) * dx + (
            pos.GetPositionY() - area.start.GetPositionY()) * dy) / (length * length);

    const float clampedProjectionFactor = std::clamp(projectionFactor, 0.0f, 1.0f);

    const float closestX = area.start.GetPositionX() + clampedProjectionFactor * dx;
    const float closestY = area.start.GetPositionY() + clampedProjectionFactor * dy;

    const float distToLine = pos.GetExactDist2d(closestX, closestY);

    return distToLine < area.width;
}

GameObject* FindNearestTrap(PlayerbotAI* botAI, Player*)
{
    GuidVector const& gos =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();

    GameObject* nearestTrap = nullptr;
    for (ObjectGuid const& guid : gos)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (go && go->isSpawned() &&
            go->GetEntry() == static_cast<uint32>(BlackTempleObjects::GO_SHADOW_TRAP))
        {
            nearestTrap = go;
            break;
        }
    }

    return nearestTrap;
}

}
