/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MgTShared.h"
#include "AiObjectContext.h"
#include "AttackersValue.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <list>
#include <string>
#include <utility>

using namespace EncounterHelpers;

namespace MagistersTerrace
{
namespace
{
enum MgTNpcs : uint32
{
    NPC_SELIN_FIREHEART         = 24723,
    NPC_FEL_CRYSTAL             = 24722,

    NPC_SUNBLADE_MAGE_GUARD     = 24683,
    NPC_SUNBLADE_BLOOD_KNIGHT   = 24684,
    NPC_SUNBLADE_MAGISTER       = 24685,
    NPC_SUNBLADE_WARLOCK        = 24686,
    NPC_SUNBLADE_PHYSICIAN      = 24687,
    NPC_SUNBLADE_IMP            = 24815,

    NPC_WRETCHED_SKULKER        = 24688,
    NPC_WRETCHED_BRUISER        = 24689,
    NPC_WRETCHED_HUSK           = 24690,

    NPC_COILSKAR_WITCH          = 24696,
    NPC_SISTER_OF_TORMENT       = 24697,
    NPC_ETHEREUM_SMUGGLER       = 24698,
};

enum MgTSpells : uint32
{
    SPELL_MAGIC_DAMPENING_FIELD = 44475,

    SPELL_HOLY_LIGHT            = 44479,
    SPELL_HOLY_LIGHT_H          = 46029,

    SPELL_ARCANE_NOVA           = 44644,
    SPELL_ARCANE_NOVA_H         = 46036,

    SPELL_INCINERATE            = 44519,
    SPELL_INCINERATE_H          = 46043,
    SPELL_FROSTBOLT             = 44606,
    SPELL_FROSTBOLT_H           = 46035,

    SPELL_WRETCHED_FIREBALL     = 44503,
    SPELL_WRETCHED_FROSTBOLT    = 44504,

    SPELL_DEADLY_EMBRACE        = 44547,

    SPELL_FORKED_LIGHTNING      = 20299,
    SPELL_FORKED_LIGHTNING_H    = 46150,

    SPELL_DRINK_FEL_INFUSION    = 44505,
};

enum MgTDelrissaNpcs : uint32
{
    NPC_DELRISSA                = 24560,

    NPC_APOKO                   = 24553,
    NPC_ELLRYS_DUSKHALLOW       = 24558,
    NPC_YAZZAI                  = 24561,
    NPC_ZELFAN                  = 24556,
    NPC_GARAXXAS                = 24555,
    NPC_KAGANI_NIGHTSTRIKE      = 24557,
    NPC_WARLORD_SALARIS         = 24559,
    NPC_ERAMAS_BRIGHTBLAZE      = 24554,

    NPC_HIGH_EXPLOSIVE_SHEEP    = 24715,

    NPC_FIZZLE                  = 24656,
    NPC_SLIVER                  = 24552,
};

enum MgTDelrissaSpells : uint32
{
    SPELL_FLASH_HEAL            = 17843,
    SPELL_LESSER_HEALING_WAVE   = 44256,
    SPELL_SUMMON_IMP            = 44163,
    SPELL_DELRISSA_FEAR         = 38595,
    SPELL_POLYMORPH             = 13323,

    SPELL_TREMOR_TOTEM          = 8143,
};

enum MgTKaelNpcs : uint32
{
    NPC_KAEL_THAS               = 24664,
    NPC_FLAMESTRIKE_TRIGGER     = 24666,
    NPC_PHOENIX                 = 24674,
    NPC_PHOENIX_EGG             = 24675,
    NPC_ARCANE_SPHERE           = 24708,
};

enum MgTKaelSpells : uint32
{
    SPELL_FLAME_STRIKE          = 44190,
    SPELL_FLAME_STRIKE_H        = 46163,

    SPELL_PHOENIX_BURN          = 44197,

    SPELL_SHOCK_BARRIER         = 46165,
    SPELL_PYROBLAST             = 36819,

    SPELL_KAEL_FIREBALL         = 44189,

    SPELL_GRAVITY_LAPSE_INITIAL = 44224,
    SPELL_GRAVITY_LAPSE_DOT     = 44226,
    SPELL_GRAVITY_LAPSE_FLY     = 44227,
    SPELL_POWER_FEEDBACK        = 44233,
};

constexpr char const* SELIN_NAME = "selin fireheart";
constexpr char const* KAEL_NAME  = "kael'thas sunstrider";

constexpr float ROOM_X_MIN = 218.0f;
constexpr float ROOM_X_MAX = 260.0f;
constexpr float ROOM_Y_MIN = -40.0f;
constexpr float ROOM_Y_MAX = 40.0f;

constexpr float CRYSTAL_SCAN_RANGE = 60.0f;

constexpr float DAMPENING_RADIUS = 5.0f;
constexpr float DAMPENING_STAND  = DAMPENING_RADIUS + 1.5f;
constexpr float DAMPENING_CLEAR  = DAMPENING_RADIUS + 4.0f;
constexpr float DAMPENING_SCAN   = 30.0f;

constexpr float TANK_REJOIN_GAP = 5.0f;

constexpr float GLAIVE_MIN_RANGE = 12.0f;
constexpr float GLAIVE_MAX_RANGE = 60.0f;

constexpr float NOVA_CLEAR = 16.0f;

constexpr float INTERRUPT_SCAN = 30.0f;
constexpr float WRETCHED_SCAN  = 40.0f;
constexpr float FOCUS_SCAN     = 40.0f;

constexpr float ESCAPE_LOS_HEIGHT = 2.0f;

constexpr float PARTY_CENTROID_RANGE = 60.0f;

constexpr float ESCAPE_RADII[] = { 7.0f, 10.0f, 13.0f, 16.0f };
constexpr uint8 ESCAPE_BEARINGS = 12;
constexpr float ESCAPE_TRAVEL_WEIGHT = 0.25f;
constexpr uint8 ESCAPE_KEEP = 5;

constexpr float FLAMESTRIKE_STAND = 7.5f;
constexpr float FLAMESTRIKE_CLEAR = 10.0f;
constexpr float FLAMESTRIKE_SCAN  = 40.0f;

constexpr float BURN_DAMAGE  = 8.0f;
constexpr float BURN_STAND   = 9.0f;
constexpr float BURN_CLEAR   = 12.0f;
constexpr float PHOENIX_SCAN = 40.0f;

constexpr float BURN_MELEE_STAND = BURN_DAMAGE + 0.5f;
constexpr float BURN_MELEE_CLEAR = BURN_DAMAGE + 2.5f;
constexpr float BURN_MELEE_RADII[] = { 3.0f, 5.0f, 7.0f, 9.0f, 12.0f };
constexpr uint8 BURN_MELEE_BEARINGS = 16;

constexpr float KAEL_MELEE_RING = 5.0f;

constexpr LapseSpot KAEL_P2      = { 148.54f, 181.13f };

constexpr LapseSpot LAPSE_TANK   = { 148.54f, 177.53f };
constexpr LapseSpot LAPSE_MELEE[] = {
    { 151.30f, 178.82f },
    { 145.78f, 178.82f },
};

constexpr LapseSpot LAPSE_HEALER = { 148.54f, 167.13f };
constexpr LapseSpot LAPSE_RANGED[] = {
    { 137.07f, 164.75f },
    { 148.54f, 155.13f },
    { 164.92f, 169.66f },
};

constexpr float SPHERE_DAMAGE    = 5.0f;
constexpr float SPHERE_CLEAR     = 8.0f;
constexpr float SPHERE_SCAN      = 40.0f;

constexpr float LAPSE_PUSH_MAX   = 8.0f;

constexpr float SPHERE_SPEED     = 3.0f;
constexpr float SPHERE_LEAD      = 2.5f;

constexpr float SPHERE_ALERT     = 9.0f;
constexpr float SPHERE_TRANSIT   = 6.0f;
constexpr float SPHERE_ROUTE_SLACK = 0.5f;

constexpr float SPHERE_DODGE_NEAR_RADIUS = 4.0f;
constexpr float SPHERE_DODGE_RADII[]     = { SPHERE_DODGE_NEAR_RADIUS, 7.0f, 11.0f, 15.0f };
constexpr uint8 SPHERE_DODGE_BEARINGS    = 16;
constexpr float SPHERE_DODGE_TRAVEL_COST = 0.25f;
constexpr float SPHERE_DODGE_MARGIN_CAP    = 6.0f;
constexpr float SPHERE_DODGE_MARGIN_WEIGHT = 0.5f;
constexpr float SPHERE_DODGE_MIN_GAIN      = 2.0f;

constexpr float KITE_LEG          = 12.0f;
constexpr float KITE_TURN_MAGNITUDES[] = { 0.0f, 25.0f, 50.0f, 75.0f,
                                           100.0f, 125.0f, 150.0f, 175.0f };

constexpr float KITE_PROBE_STEP   = 2.0f;
constexpr float KITE_PROBE_MAX    = 24.0f;

constexpr float KITE_PASS_CLEAR   = SPHERE_DAMAGE;
constexpr float KITE_RUNWAY_TIEBREAK = 0.5f;
constexpr float KITE_COMMIT_BONUS = 3.0f;
constexpr float KITE_COMMIT_STRAIGHT = 10.0f;

constexpr uint32 KITE_COMMIT_BREAK_MS = 1500;

constexpr float KITE_NORTH_MIN    = 340.0f;
constexpr float KITE_NORTH_MAX    = 20.0f;
constexpr float KITE_BOWL_MIN     = 100.0f;
constexpr float KITE_BOWL_MAX     = 260.0f;
constexpr float KITE_BOWL_RADIUS  = 22.0f;
constexpr float KITE_SIDE_RADIUS  = 13.0f;

constexpr float LAPSE_TOLERANCE  = 3.0f;

constexpr float LAPSE_BURN_COST = 6.0f;

constexpr float LAPSE_LEG          = 10.0f;
constexpr float LAPSE_PROGRESS_MIN = 1.0f;
constexpr float LAPSE_DETOUR_TURNS[] = { 0.0f, 20.0f, 40.0f, 60.0f, 80.0f, 100.0f };

struct Hazard
{
    float x;
    float y;
    float stand;
    float clear;
};

struct ScoredSpot
{
    float score;
    float x;
    float y;
};

void CollectCreaturesByEntry(Player* bot, std::vector<uint32> const& entries, float radius,
                             std::vector<Unit*>& out)
{
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, entries, radius);

    out.reserve(out.size() + found.size());
    for (Creature* creature : found)
    {
        if (creature->IsAlive())
            out.push_back(creature);
    }
}

void CollectAttackersByEntry(Player* bot, std::vector<uint32> const& entries, float radius,
                             std::vector<Unit*>& out)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    auto const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker)
            continue;

        if (std::find(entries.begin(), entries.end(), attacker->GetEntry()) == entries.end())
            continue;

        if (bot->GetExactDist2d(attacker) > radius)
            continue;

        out.push_back(attacker);
    }
}

bool PartyCentroid(Player* bot, float& cx, float& cy)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float sx = 0.0f, sy = 0.0f;
    int n = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;
        if (bot->GetExactDist2d(member) > PARTY_CENTROID_RANGE)
            continue;

        sx += member->GetPositionX();
        sy += member->GetPositionY();
        ++n;
    }

    if (n == 0)
        return false;

    cx = sx / n;
    cy = sy / n;
    return true;
}

bool SpotClearsHazards(std::vector<Hazard> const& hazards, float x, float y)
{
    for (Hazard const& hazard : hazards)
    {
        float const dx = x - hazard.x;
        float const dy = y - hazard.y;
        if (dx * dx + dy * dy < hazard.clear * hazard.clear)
            return false;
    }

    return true;
}

bool InsideAnyHazard(std::vector<Hazard> const& hazards, float x, float y)
{
    for (Hazard const& hazard : hazards)
    {
        float const dx = x - hazard.x;
        float const dy = y - hazard.y;
        if (dx * dx + dy * dy < hazard.stand * hazard.stand)
            return true;
    }

    return false;
}

void KeepBestReachable(Player* bot, std::vector<ScoredSpot>& scored, EscapeSpots& out)
{
    std::stable_sort(scored.begin(), scored.end(),
                     [](ScoredSpot const& a, ScoredSpot const& b) { return a.score < b.score; });

    float const bz = bot->GetPositionZ();

    uint8 kept = 0;
    for (ScoredSpot const& spot : scored)
    {
        float const z = bot->GetMapHeight(spot.x, spot.y, bz + GROUND_SEARCH_UP, true, GROUND_SEARCH_DOWN);
        if (!(std::fabs(z - bz) <= GROUND_TIER_STEP))
            continue;

        if (!bot->IsWithinLOS(spot.x, spot.y, z + ESCAPE_LOS_HEIGHT))
            continue;

        out.push_back({ spot.x, spot.y, z });

        if (++kept >= ESCAPE_KEEP)
            break;
    }
}

void CollectHazardEscapes(Player* bot, std::vector<Hazard> const& hazards, float anchorGap,
                          EscapeSpots& out)
{
    if (hazards.empty())
        return;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    if (!InsideAnyHazard(hazards, bx, by))
        return;

    float cx = 0.0f, cy = 0.0f;
    bool const haveParty = PartyCentroid(bot, cx, cy);

    std::vector<ScoredSpot> scored;
    for (float radius : ESCAPE_RADII)
    {
        for (int i = 0; i < ESCAPE_BEARINGS; ++i)
        {
            float const angle = (2.0f * static_cast<float>(M_PI) * i) / ESCAPE_BEARINGS;
            float const x = bx + std::cos(angle) * radius;
            float const y = by + std::sin(angle) * radius;
            if (!SpotClearsHazards(hazards, x, y))
                continue;

            float score = radius * ESCAPE_TRAVEL_WEIGHT;
            if (haveParty)
                score += std::fabs(std::hypot(x - cx, y - cy) - anchorGap);

            scored.push_back({ score, x, y });
        }
    }

    KeepBestReachable(bot, scored, out);
}

void CollectRingEscapes(Player* bot, Unit* anchor, std::vector<Hazard> const& hazards, EscapeSpots& out)
{
    if (hazards.empty())
        return;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    if (!InsideAnyHazard(hazards, bx, by))
        return;

    float const ax = anchor->GetPositionX();
    float const ay = anchor->GetPositionY();

    std::vector<ScoredSpot> scored;
    for (float radius : BURN_MELEE_RADII)
    {
        for (int i = 0; i < BURN_MELEE_BEARINGS; ++i)
        {
            float const angle = (2.0f * static_cast<float>(M_PI) * i) / BURN_MELEE_BEARINGS;
            float const x = bx + std::cos(angle) * radius;
            float const y = by + std::sin(angle) * radius;
            if (!SpotClearsHazards(hazards, x, y))
                continue;

            float const score = std::hypot(x - ax, y - ay) + radius * ESCAPE_TRAVEL_WEIGHT;
            scored.push_back({ score, x, y });
        }
    }

    KeepBestReachable(bot, scored, out);
}

uint8 InterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_DEADLY_EMBRACE:
            return 1;
        case SPELL_HOLY_LIGHT:
        case SPELL_HOLY_LIGHT_H:
            return 2;
        case SPELL_ARCANE_NOVA:
        case SPELL_ARCANE_NOVA_H:
            return 3;
        case SPELL_FORKED_LIGHTNING:
        case SPELL_FORKED_LIGHTNING_H:
            return 4;
        case SPELL_INCINERATE:
        case SPELL_INCINERATE_H:
            return 5;
        case SPELL_FROSTBOLT:
        case SPELL_FROSTBOLT_H:
            return 6;
        case SPELL_WRETCHED_FIREBALL:
        case SPELL_WRETCHED_FROSTBOLT:
            return 7;
        default:
            return 0;
    }
}

uint8 CurrentCastPriority(Unit* caster)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 const prio = InterruptPriority(spell->m_spellInfo->Id);
        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}

bool StopsACast(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return false;

    if ((info->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
        info->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
        return true;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; ++i)
    {
        if (info->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
            return true;

        if (info->Effects[i].Effect != SPELL_EFFECT_APPLY_AURA)
            continue;

        if (info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE ||
            info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_STUN)
            return true;
    }

    return false;
}

enum class InterruptKind
{
    Dedicated,
    Control,
};

struct InterruptSpell
{
    char const* name;
    InterruptKind kind;
};

std::vector<InterruptSpell> const& InterruptSpells()
{
    static std::vector<InterruptSpell> const spells = {
        { "kick", InterruptKind::Dedicated },
        { "pummel", InterruptKind::Dedicated },
        { "shield bash", InterruptKind::Dedicated },
        { "wind shear", InterruptKind::Dedicated },
        { "mind freeze", InterruptKind::Dedicated },
        { "counterspell", InterruptKind::Dedicated },
        { "spell lock", InterruptKind::Dedicated },
        { "silencing shot", InterruptKind::Dedicated },
        { "shockwave", InterruptKind::Control },
        { "shadowfury", InterruptKind::Control },
        { "concussion blow", InterruptKind::Control },
        { "hammer of justice", InterruptKind::Control },
        { "maim", InterruptKind::Control },
        { "kidney shot", InterruptKind::Control },
        { "silence", InterruptKind::Control },
        { "bash", InterruptKind::Control },
        { "strangulate", InterruptKind::Control },
        { "arcane torrent", InterruptKind::Control },
    };

    return spells;
}

uint8 TrashFocusRank(uint32 entry)
{
    switch (entry)
    {
        case NPC_SUNBLADE_PHYSICIAN:
            return 1;
        case NPC_SUNBLADE_BLOOD_KNIGHT:
            return 2;
        case NPC_SISTER_OF_TORMENT:
            return 3;
        case NPC_SUNBLADE_WARLOCK:
            return 4;
        case NPC_SUNBLADE_MAGISTER:
            return 5;
        case NPC_ETHEREUM_SMUGGLER:
            return 6;
        case NPC_COILSKAR_WITCH:
            return 7;
        case NPC_SUNBLADE_IMP:
            return 8;
        case NPC_SUNBLADE_MAGE_GUARD:
            return 9;
        default:
            return 0;
    }
}

uint8 RetinueFocusRank(uint32 entry)
{
    switch (entry)
    {
        case NPC_HIGH_EXPLOSIVE_SHEEP:
            return 1;
        case NPC_APOKO:
            return 2;
        case NPC_ELLRYS_DUSKHALLOW:
            return 3;
        case NPC_YAZZAI:
            return 4;
        case NPC_ZELFAN:
            return 5;
        case NPC_WARLORD_SALARIS:
            return 6;
        case NPC_GARAXXAS:
            return 7;
        case NPC_KAGANI_NIGHTSTRIKE:
            return 8;
        case NPC_ERAMAS_BRIGHTBLAZE:
            return 9;
        case NPC_DELRISSA:
            return 10;
        default:
            return 0;
    }
}

Unit* PickRankedFocus(Player* bot, uint8 (*rank)(uint32), std::vector<Unit*> const& extra,
                      ObjectGuid& latched)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    Unit* held = nullptr;
    uint8 heldRank = 0;
    Unit* best = nullptr;
    uint8 bestRank = 0;
    float bestDist = 0.0f;

    auto consider = [&](Unit* candidate)
    {
        uint8 const candidateRank = rank(candidate->GetEntry());
        if (!candidateRank)
            return;

        float const dist = bot->GetExactDist2d(candidate);
        if (dist > FOCUS_SCAN)
            return;

        if (!latched.IsEmpty() && candidate->GetGUID() == latched)
        {
            held = candidate;
            heldRank = candidateRank;
        }

        if (!best || candidateRank < bestRank || (candidateRank == bestRank && dist < bestDist))
        {
            best = candidate;
            bestRank = candidateRank;
            bestDist = dist;
        }
    };

    auto const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        if (Unit* attacker = botAI->GetUnit(guid))
            consider(attacker);
    }

    for (Unit* candidate : extra)
        consider(candidate);

    if (held && heldRank <= bestRank)
        return held;

    latched = best ? best->GetGUID() : ObjectGuid::Empty;
    return best;
}

bool RetinueEngaged(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    auto const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && RetinueFocusRank(attacker->GetEntry()))
            return true;
    }

    return false;
}

uint8 RetinueInterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_DELRISSA_FEAR:
        case SPELL_POLYMORPH:
            return 1;
        case SPELL_FLASH_HEAL:
        case SPELL_LESSER_HEALING_WAVE:
            return 2;
        case SPELL_SUMMON_IMP:
            return 3;
        default:
            return 0;
    }
}

uint8 CurrentRetinueCastPriority(Unit* caster)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 const prio = RetinueInterruptPriority(spell->m_spellInfo->Id);
        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}

int InterruptSlot(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    int slot = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(member))
            continue;

        if (member->GetGUID() < bot->GetGUID())
            ++slot;
    }

    return slot;
}

constexpr FocusOrder FOCUS_ORDER_OWNERS[] = { FocusOrder::Kael, FocusOrder::Delrissa, FocusOrder::Trash };
constexpr char const* FOCUS_ORDER_KEYS[] = { "mgt kael focus target", "mgt delrissa focus target",
                                             "mgt focus target" };

bool LapseActive(Player* bot, Unit* kael)
{
    if (!kael || kael->HasAura(SPELL_POWER_FEEDBACK))
        return false;

    if (bot->HasAura(SPELL_GRAVITY_LAPSE_DOT) || bot->HasAura(SPELL_GRAVITY_LAPSE_FLY))
        return true;

    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = kael->GetCurrentSpell(slot);
        if (spell && spell->m_spellInfo && spell->m_spellInfo->Id == SPELL_GRAVITY_LAPSE_INITIAL)
            return true;
    }

    return false;
}

uint8 KaelInterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_PYROBLAST:
            return 1;
        case SPELL_KAEL_FIREBALL:
            return 2;
        default:
            return 0;
    }
}

uint8 CurrentKaelCastPriority(Unit* kael)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = kael->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 const prio = KaelInterruptPriority(spell->m_spellInfo->Id);
        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}

enum class LapseRole
{
    Tank,
    Healer,
    Melee,
    Ranged
};

LapseRole ClassifyLapseRole(Player* member)
{
    if (PlayerbotAI::IsTank(member))
        return LapseRole::Tank;

    if (PlayerbotAI::IsHeal(member))
        return LapseRole::Healer;

    if (PlayerbotAI::IsMelee(member))
        return LapseRole::Melee;

    return LapseRole::Ranged;
}

int LapseSlot(Player* bot, LapseRole role)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    int slot = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (!GET_PLAYERBOT_AI(member) || ClassifyLapseRole(member) != role)
            continue;

        if (member->GetGUID() < bot->GetGUID())
            ++slot;
    }

    return slot;
}

bool LapseAnchor(Player* bot, LapseSpot& out)
{
    LapseRole const role = ClassifyLapseRole(bot);
    size_t const slot = static_cast<size_t>(LapseSlot(bot, role));

    switch (role)
    {
        case LapseRole::Tank:
            out = LAPSE_TANK;
            break;
        case LapseRole::Healer:
            out = LAPSE_HEALER;
            break;
        case LapseRole::Melee:
            out = LAPSE_MELEE[slot % std::size(LAPSE_MELEE)];
            break;
        default:
            out = LAPSE_RANGED[slot % std::size(LAPSE_RANGED)];
            break;
    }

    return true;
}

float BearingFromKael(float x, float y)
{
    float degrees = std::atan2(x - KAEL_P2.x, y - KAEL_P2.y) * 180.0f / static_cast<float>(M_PI);
    if (degrees < 0.0f)
        degrees += 360.0f;

    return degrees;
}

bool SphereHeading(Unit* sphere, float& hx, float& hy)
{
    if (Unit* victim = sphere->GetVictim())
    {
        float const dx = victim->GetPositionX() - sphere->GetPositionX();
        float const dy = victim->GetPositionY() - sphere->GetPositionY();
        float const len = std::hypot(dx, dy);
        if (len < 0.1f)
            return false;

        hx = dx / len;
        hy = dy / len;
        return true;
    }

    hx = std::cos(sphere->GetOrientation());
    hy = std::sin(sphere->GetOrientation());
    return true;
}

float DistanceToSphereSweep(Unit* sphere, float x, float y)
{
    float const sx = sphere->GetPositionX();
    float const sy = sphere->GetPositionY();

    float hx = 0.0f, hy = 0.0f;
    if (!SphereHeading(sphere, hx, hy))
        return std::hypot(x - sx, y - sy);

    float const reach = SPHERE_SPEED * SPHERE_LEAD;
    float t = (x - sx) * hx + (y - sy) * hy;
    t = std::clamp(t, 0.0f, reach);

    return std::hypot(x - (sx + hx * t), y - (sy + hy * t));
}

float PointToSegment(float px, float py, float ax, float ay, float bx, float by)
{
    float const dx = bx - ax;
    float const dy = by - ay;
    float const len2 = dx * dx + dy * dy;
    if (len2 < 0.0001f)
        return std::hypot(px - ax, py - ay);

    float t = ((px - ax) * dx + (py - ay) * dy) / len2;
    t = std::clamp(t, 0.0f, 1.0f);

    return std::hypot(px - (ax + dx * t), py - (ay + dy * t));
}

float SegmentToSegment(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy)
{
    auto side = [](float ox, float oy, float px, float py, float qx, float qy)
    { return (px - ox) * (qy - oy) - (py - oy) * (qx - ox); };

    float const d1 = side(cx, cy, dx, dy, ax, ay);
    float const d2 = side(cx, cy, dx, dy, bx, by);
    float const d3 = side(ax, ay, bx, by, cx, cy);
    float const d4 = side(ax, ay, bx, by, dx, dy);

    if (((d1 > 0.0f) != (d2 > 0.0f)) && ((d3 > 0.0f) != (d4 > 0.0f)))
        return 0.0f;

    return std::min({ PointToSegment(ax, ay, cx, cy, dx, dy), PointToSegment(bx, by, cx, cy, dx, dy),
                      PointToSegment(cx, cy, ax, ay, bx, by), PointToSegment(dx, dy, ax, ay, bx, by) });
}

float WalkClearanceOfSphere(Unit* sphere, float fx, float fy, float tx, float ty)
{
    float const sx = sphere->GetPositionX();
    float const sy = sphere->GetPositionY();

    float hx = 0.0f, hy = 0.0f;
    if (!SphereHeading(sphere, hx, hy))
        return PointToSegment(sx, sy, fx, fy, tx, ty);

    float const reach = SPHERE_SPEED * SPHERE_LEAD;

    return SegmentToSegment(fx, fy, tx, ty, sx, sy, sx + hx * reach, sy + hy * reach);
}

bool InsideKiteArena(float x, float y, bool healer)
{
    float const bearing = BearingFromKael(x, y);

    if (bearing >= KITE_NORTH_MIN || bearing <= KITE_NORTH_MAX)
        return false;

    bool const bowl = bearing >= KITE_BOWL_MIN && bearing <= KITE_BOWL_MAX;
    if (healer && !bowl)
        return false;

    float const limit = bowl ? KITE_BOWL_RADIUS : KITE_SIDE_RADIUS;

    return std::hypot(x - KAEL_P2.x, y - KAEL_P2.y) <= limit;
}

float KiteRunway(float fx, float fy, float hx, float hy, bool healer, bool outside)
{
    int const steps = static_cast<int>(KITE_PROBE_MAX / KITE_PROBE_STEP);

    bool started = !outside;
    float runway = 0.0f;
    for (int i = 1; i <= steps; ++i)
    {
        float const step = KITE_PROBE_STEP * i;
        bool const clear = InsideKiteArena(fx + hx * step, fy + hy * step, healer);

        if (!started)
        {
            if (clear)
            {
                started = true;
                runway = step;
            }

            continue;
        }

        if (!clear)
            break;

        runway = step;
    }

    return started ? runway : 0.0f;
}

bool IsLapseHealer(Player* bot)
{
    return PlayerbotAI::IsHeal(bot) && !PlayerbotAI::IsTank(bot);
}

bool InsideAnyBurn(std::vector<LapseSpot> const& burns, float x, float y)
{
    for (LapseSpot const& burn : burns)
    {
        if (std::hypot(x - burn.x, y - burn.y) < BURN_DAMAGE)
            return true;
    }

    return false;
}

Unit* GetChasingSphere(Player* bot, LapseWorld const& world)
{
    for (Unit* sphere : world.spheres)
    {
        if (sphere->GetVictim() == bot)
            return sphere;
    }

    return nullptr;
}

float BurningPhoenixKaelGap(Player* bot, Unit* kael)
{
    std::vector<Unit*> phoenixes;
    CollectCreaturesByEntry(bot, { NPC_PHOENIX }, PHOENIX_SCAN, phoenixes);

    float gap = std::numeric_limits<float>::max();
    for (Unit* phoenix : phoenixes)
    {
        if (phoenix->HasAura(SPELL_PHOENIX_BURN))
            gap = std::min(gap, phoenix->GetExactDist2d(kael));
    }

    return gap;
}
}

Unit* GetSelin(Player* bot)
{
    return GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<Unit*>("find target", SELIN_NAME)->Get();
}

Unit* GetKaelthas(Player* bot)
{
    return GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<Unit*>("find target", KAEL_NAME)->Get();
}

Unit* GetActiveFelCrystal(Player* bot)
{
    if (!GetSelin(bot))
        return nullptr;

    std::vector<Unit*> crystals;
    CollectCreaturesByEntry(bot, { NPC_FEL_CRYSTAL }, CRYSTAL_SCAN_RANGE, crystals);

    Unit* best = nullptr;
    float bestDist = CRYSTAL_SCAN_RANGE;
    for (Unit* crystal : crystals)
    {
        if (!AttackersValue::IsValidTarget(crystal, bot))
            continue;

        float const dist = bot->GetExactDist2d(crystal);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = crystal;
        }
    }

    return best;
}

void CollectDampeningEscapes(Player* bot, EscapeSpots& out)
{
    if (!bot->IsInCombat())
        return;

    std::vector<Hazard> fields;
    for (Position const& pos : GetDynamicObjectPositions(bot, DAMPENING_SCAN, SPELL_MAGIC_DAMPENING_FIELD))
        fields.push_back({ pos.GetPositionX(), pos.GetPositionY(), DAMPENING_STAND, DAMPENING_CLEAR });

    CollectHazardEscapes(bot, fields, PlayerbotAI::IsTank(bot) ? TANK_REJOIN_GAP : 0.0f, out);
}

Unit* GetGlaiveThrowingMageGuard(Player* bot)
{
    std::vector<Unit*> guards;
    CollectAttackersByEntry(bot, { NPC_SUNBLADE_MAGE_GUARD }, GLAIVE_MAX_RANGE, guards);

    for (Unit* guard : guards)
    {
        if (guard->GetVictim() != bot)
            continue;

        if (bot->GetExactDist2d(guard) > GLAIVE_MIN_RANGE)
            return guard;
    }

    return nullptr;
}

void CollectNovaEscapes(Player* bot, EscapeSpots& out)
{
    if (PlayerbotAI::IsMelee(bot))
        return;

    std::vector<Unit*> magisters;
    CollectAttackersByEntry(bot, { NPC_SUNBLADE_MAGISTER }, NOVA_CLEAR, magisters);

    std::vector<Hazard> hazards;
    hazards.reserve(magisters.size());
    for (Unit* magister : magisters)
    {
        hazards.push_back({ magister->GetPositionX(), magister->GetPositionY(), NOVA_CLEAR,
                            NOVA_CLEAR + 2.0f });
    }

    CollectHazardEscapes(bot, hazards, 0.0f, out);
}

uint8 GetInterruptUrgency(Unit* caster)
{
    return caster ? CurrentCastPriority(caster) : 0;
}

Unit* GetInterruptTarget(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    Unit* best = nullptr;
    uint8 bestPrio = 0;
    float bestDist = 0.0f;
    auto const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* caster = botAI->GetUnit(guid);
        if (!caster)
            continue;

        float const dist = bot->GetExactDist2d(caster);
        if (dist > INTERRUPT_SCAN)
            continue;

        uint8 const prio = CurrentCastPriority(caster);
        if (!prio)
            continue;

        if (!best || prio < bestPrio || (prio == bestPrio && dist < bestDist))
        {
            best = caster;
            bestPrio = prio;
            bestDist = dist;
        }
    }

    return best;
}

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target, uint8 urgency)
{
    if (!target->IsNonMeleeSpellCast(true))
        return {};

    bool const allowControl = urgency && urgency <= CONTROL_INTERRUPT_URGENCY;

    for (InterruptSpell const& entry : InterruptSpells())
    {
        if (entry.kind == InterruptKind::Control && !allowControl)
            continue;

        std::string const name = entry.name;

        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (!spellId || !StopsACast(spellId))
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

uint8 GetRetinueInterruptUrgency(Unit* caster)
{
    return caster ? CurrentRetinueCastPriority(caster) : 0;
}

Unit* GetFocusTarget(Player* bot, ObjectGuid& latched)
{
    return PickRankedFocus(bot, &TrashFocusRank, {}, latched);
}

ObjectGuid ResolveFocusOrder(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();

    for (char const* key : FOCUS_ORDER_KEYS)
    {
        ObjectGuid const order = context->GetValue<ObjectGuid>(key)->Get();
        if (!order.IsEmpty())
            return order;
    }

    return ObjectGuid::Empty;
}

FocusOrder ResolveFocusOrderOwner(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();

    for (size_t i = 0; i < std::size(FOCUS_ORDER_KEYS); ++i)
    {
        if (!context->GetValue<ObjectGuid>(FOCUS_ORDER_KEYS[i])->Get().IsEmpty())
            return FOCUS_ORDER_OWNERS[i];
    }

    return FocusOrder::None;
}

void CollectFocusExclusions(Player* bot, std::vector<ObjectGuid>& out)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    ObjectGuid const order = ResolveFocusOrder(botAI);
    if (order.IsEmpty())
        return;

    Unit* focus = botAI->GetUnit(order);
    bool const transient = !focus || focus->GetEntry() == NPC_HIGH_EXPLOSIVE_SHEEP ||
                           focus->GetEntry() == NPC_PHOENIX_EGG;

    auto const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    out.reserve(out.size() + attackers.size() + 1);
    for (ObjectGuid const guid : attackers)
    {
        if (guid != order)
            out.push_back(guid);
    }

    if (transient)
        out.push_back(order);
}

Unit* GetEnragedWretched(Player* bot)
{
    std::vector<Unit*> wretched;
    CollectAttackersByEntry(bot, { NPC_WRETCHED_SKULKER, NPC_WRETCHED_BRUISER, NPC_WRETCHED_HUSK },
                            WRETCHED_SCAN, wretched);

    Unit* best = nullptr;
    float bestDist = WRETCHED_SCAN;
    for (Unit* mob : wretched)
    {
        if (!mob->HasAura(SPELL_DRINK_FEL_INFUSION))
            continue;

        Unit* victim = mob->GetVictim();
        if (!victim || victim == bot)
            continue;

        float const dist = bot->GetExactDist2d(mob);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = mob;
        }
    }

    return best;
}

std::vector<std::string> const& TauntSpellNames()
{
    static std::vector<std::string> const names = { "taunt", "growl", "dark command", "hand of reckoning" };

    return names;
}

void ClampIntoRoom(float& x, float& y)
{
    x = std::clamp(x, ROOM_X_MIN, ROOM_X_MAX);
    y = std::clamp(y, ROOM_Y_MIN, ROOM_Y_MAX);
}

bool ShouldHoldTremorTotem(Player* bot)
{
    if (bot->getClass() != CLASS_SHAMAN || !bot->HasSpell(SPELL_TREMOR_TOTEM))
        return false;

    return RetinueEngaged(bot);
}

Unit* GetDelrissaFocusTarget(Player* bot, ObjectGuid& latched)
{
    std::vector<Unit*> sheep;
    if (RetinueEngaged(bot))
    {
        std::vector<Unit*> found;
        CollectCreaturesByEntry(bot, { NPC_HIGH_EXPLOSIVE_SHEEP }, FOCUS_SCAN, found);

        for (Unit* mob : found)
        {
            if (AttackersValue::IsValidTarget(mob, bot))
                sheep.push_back(mob);
        }
    }

    return PickRankedFocus(bot, &RetinueFocusRank, sheep, latched);
}

void CollectDelrissaInterruptPreference(Player* bot, std::vector<Unit*>& out)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);

    std::vector<std::pair<uint8, Unit*>> ranked;
    auto const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* caster = botAI->GetUnit(guid);
        if (!caster || bot->GetExactDist2d(caster) > INTERRUPT_SCAN)
            continue;

        if (!RetinueFocusRank(caster->GetEntry()))
            continue;

        if (uint8 const prio = CurrentRetinueCastPriority(caster))
            ranked.push_back({ prio, caster });
    }

    if (ranked.empty())
        return;

    std::sort(ranked.begin(), ranked.end(), [](auto const& a, auto const& b) {
        if (a.first != b.first)
            return a.first < b.first;

        return a.second->GetGUID() < b.second->GetGUID();
    });

    size_t const slot = static_cast<size_t>(InterruptSlot(bot));
    size_t const index = slot < ranked.size() ? slot : 0;

    out.reserve(out.size() + ranked.size());
    out.push_back(ranked[index].second);

    for (size_t i = 0; i < ranked.size(); ++i)
    {
        if (i != index)
            out.push_back(ranked[i].second);
    }
}

void CollectDelrissaPets(Player* bot, std::vector<ObjectGuid>& out)
{
    std::vector<Unit*> pets;
    CollectAttackersByEntry(bot, { NPC_FIZZLE, NPC_SLIVER }, FOCUS_SCAN, pets);

    out.reserve(out.size() + pets.size());
    for (Unit* pet : pets)
        out.push_back(pet->GetGUID());
}

Unit* GetKaelInterruptTarget(Player* bot)
{
    Unit* kael = GetKaelthas(bot);
    if (!kael || kael->HasAura(SPELL_SHOCK_BARRIER))
        return nullptr;

    return CurrentKaelCastPriority(kael) ? kael : nullptr;
}

bool IsKaelUnattackable(Player* bot)
{
    Unit* kael = GetKaelthas(bot);

    return kael && kael->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
}

void CollectFlameStrikeEscapes(Player* bot, EscapeSpots& out)
{
    if (!GetKaelthas(bot))
        return;

    std::vector<Hazard> hazards;

    std::vector<Unit*> triggers;
    CollectCreaturesByEntry(bot, { NPC_FLAMESTRIKE_TRIGGER }, FLAMESTRIKE_SCAN, triggers);
    for (Unit* trigger : triggers)
    {
        hazards.push_back({ trigger->GetPositionX(), trigger->GetPositionY(), FLAMESTRIKE_STAND,
                            FLAMESTRIKE_CLEAR });
    }

    for (uint32 spellId : { SPELL_FLAME_STRIKE, SPELL_FLAME_STRIKE_H })
    {
        for (Position const& pos : GetDynamicObjectPositions(bot, FLAMESTRIKE_SCAN, spellId))
        {
            hazards.push_back({ pos.GetPositionX(), pos.GetPositionY(), FLAMESTRIKE_STAND,
                                FLAMESTRIKE_CLEAR });
        }
    }

    CollectHazardEscapes(bot, hazards, 0.0f, out);
}

void CollectPhoenixEscapes(Player* bot, EscapeSpots& out)
{
    Unit* kael = GetKaelthas(bot);
    if (!kael || LapseActive(bot, kael))
        return;

    bool const melee = PlayerbotAI::IsMelee(bot);
    float const stand = melee ? BURN_MELEE_STAND : BURN_STAND;
    float const clear = melee ? BURN_MELEE_CLEAR : BURN_CLEAR;

    std::vector<Unit*> phoenixes;
    CollectCreaturesByEntry(bot, { NPC_PHOENIX }, PHOENIX_SCAN, phoenixes);

    std::vector<Hazard> hazards;
    for (Unit* phoenix : phoenixes)
    {
        if (!phoenix->HasAura(SPELL_PHOENIX_BURN))
            continue;

        hazards.push_back({ phoenix->GetPositionX(), phoenix->GetPositionY(), stand, clear });
    }

    if (melee)
        CollectRingEscapes(bot, kael, hazards, out);
    else
        CollectHazardEscapes(bot, hazards, 0.0f, out);
}

PhoenixRing GetPhoenixRing(Player* bot)
{
    if (!PlayerbotAI::IsMelee(bot))
        return PhoenixRing::None;

    Unit* kael = GetKaelthas(bot);
    if (!kael || LapseActive(bot, kael))
        return PhoenixRing::None;

    float const gap = BurningPhoenixKaelGap(bot, kael);
    if (gap >= KAEL_MELEE_RING + BURN_MELEE_CLEAR)
        return PhoenixRing::None;

    if (gap < BURN_MELEE_CLEAR - KAEL_MELEE_RING &&
        bot->GetExactDist2d(kael) <= KAEL_MELEE_RING + BURN_MELEE_CLEAR)
        return PhoenixRing::Covers;

    return PhoenixRing::Threatens;
}

Unit* GetKaelFocusTarget(Player* bot)
{
    Unit* kael = GetKaelthas(bot);
    if (!kael)
        return nullptr;

    if (kael->HasAura(SPELL_SHOCK_BARRIER) && AttackersValue::IsValidTarget(kael, bot))
        return kael;

    std::vector<Unit*> kin;
    CollectCreaturesByEntry(bot, { NPC_PHOENIX_EGG, NPC_PHOENIX }, PHOENIX_SCAN, kin);

    Unit* best = nullptr;
    float bestDist = PHOENIX_SCAN;
    for (Unit* egg : kin)
    {
        if (egg->GetEntry() != NPC_PHOENIX_EGG || !AttackersValue::IsValidTarget(egg, bot))
            continue;

        float const dist = bot->GetExactDist2d(egg);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = egg;
        }
    }

    if (best)
        return best;

    if (PlayerbotAI::IsMelee(bot))
        return nullptr;

    bestDist = PHOENIX_SCAN;
    for (Unit* phoenix : kin)
    {
        if (phoenix->GetEntry() != NPC_PHOENIX || !phoenix->IsInCombat() ||
            !AttackersValue::IsValidTarget(phoenix, bot))
            continue;

        float const dist = bot->GetExactDist2d(phoenix);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = phoenix;
        }
    }

    return best;
}

bool IsGravityLapseActive(Player* bot)
{
    return LapseActive(bot, GetKaelthas(bot));
}

void CollectLapseWorld(Player* bot, LapseWorld& out)
{
    std::vector<Unit*> units;
    CollectCreaturesByEntry(bot, { NPC_ARCANE_SPHERE, NPC_PHOENIX }, std::max(SPHERE_SCAN, PHOENIX_SCAN), units);

    for (Unit* unit : units)
    {
        if (unit->GetEntry() == NPC_ARCANE_SPHERE && bot->GetExactDist2d(unit) <= SPHERE_SCAN)
            out.spheres.push_back(unit);
        else if (unit->GetEntry() == NPC_PHOENIX && unit->HasAura(SPELL_PHOENIX_BURN) &&
                 bot->GetExactDist2d(unit) <= PHOENIX_SCAN)
            out.burns.push_back({ unit->GetPositionX(), unit->GetPositionY() });
    }
}

bool GetGravityLapseKite(Player* bot, LapseWorld const& world, LapseSpot& out, KiteState& state)
{
    Unit* chaser = GetChasingSphere(bot, world);
    if (!chaser)
    {
        state = KiteState{};
        return false;
    }

    if (state.lastMs && GetMSTimeDiffToNow(state.lastMs) > KITE_COMMIT_BREAK_MS)
        state = KiteState{};

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    float ax = bx - chaser->GetPositionX();
    float ay = by - chaser->GetPositionY();
    float away = std::hypot(ax, ay);

    if (away < 0.1f)
    {
        ax = bx - KAEL_P2.x;
        ay = by - KAEL_P2.y;
        away = std::hypot(ax, ay);
        if (away < 0.1f)
            return false;
    }

    ax /= away;
    ay /= away;

    bool const healer = IsLapseHealer(bot);
    bool const outside = !InsideKiteArena(bx, by, healer);

    float here = std::numeric_limits<float>::max();
    for (Unit* other : world.spheres)
        here = std::min(here, DistanceToSphereSweep(other, bx, by));

    float const passBar = std::min(KITE_PASS_CLEAR, here - SPHERE_ROUTE_SLACK);

    bool haveFallback = false;
    float fallbackClear = -1.0f;
    float fallbackTurn = 0.0f;
    LapseSpot fallback = { bx, by };

    for (int pass = 0; pass < 2; ++pass)
    {
        bool const requireFullLeg = pass == 0;

        for (float magnitude : KITE_TURN_MAGNITUDES)
        {
            bool found = false;
            float bestPreference = 0.0f;
            float bestTurn = 0.0f;
            LapseSpot best = { bx, by };

            for (int side = 0; side < 2; ++side)
            {
                float const turn = side ? -magnitude : magnitude;
                if (side && magnitude <= 0.0f)
                    continue;

                float const angle = turn * static_cast<float>(M_PI) / 180.0f;
                float const cosT = std::cos(angle);
                float const sinT = std::sin(angle);

                float const hx = ax * cosT - ay * sinT;
                float const hy = ax * sinT + ay * cosT;

                float const x = bx + hx * KITE_LEG;
                float const y = by + hy * KITE_LEG;

                float const runway = KiteRunway(bx, by, hx, hy, healer, outside);

                if (requireFullLeg ? runway < KITE_LEG : !InsideKiteArena(x, y, healer))
                    continue;

                float clearance = std::numeric_limits<float>::max();
                for (Unit* other : world.spheres)
                    clearance = std::min(clearance, WalkClearanceOfSphere(other, bx, by, x, y));

                if (clearance < passBar)
                {
                    if (clearance > fallbackClear)
                    {
                        haveFallback = true;
                        fallbackClear = clearance;
                        fallbackTurn = turn;
                        fallback = { x, y };
                    }

                    continue;
                }

                float preference = clearance + runway * KITE_RUNWAY_TIEBREAK;

                if (std::fabs(turn) > KITE_COMMIT_STRAIGHT && std::fabs(state.turn) > KITE_COMMIT_STRAIGHT &&
                    (turn > 0.0f) == (state.turn > 0.0f))
                    preference += KITE_COMMIT_BONUS;

                if (InsideAnyBurn(world.burns, x, y))
                    preference -= LAPSE_BURN_COST;

                if (!found || preference > bestPreference)
                {
                    found = true;
                    bestPreference = preference;
                    bestTurn = turn;
                    best = { x, y };
                }
            }

            if (found)
            {
                out = best;
                state.turn = bestTurn;
                state.lastMs = getMSTime();

                return true;
            }
        }
    }

    if (!haveFallback)
        return false;

    out = fallback;
    state.turn = fallbackTurn;
    state.lastMs = getMSTime();

    return true;
}

bool IsLapseMeleeSlot(Player* bot)
{
    LapseRole const role = ClassifyLapseRole(bot);

    return role == LapseRole::Tank || role == LapseRole::Melee;
}

bool GetGravityLapseDodge(Player* bot, LapseWorld const& world, LapseSpot& out)
{
    if (world.spheres.empty())
        return false;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    float here = std::numeric_limits<float>::max();
    for (Unit* sphere : world.spheres)
        here = std::min(here, DistanceToSphereSweep(sphere, bx, by));

    if (here >= SPHERE_ALERT)
        return false;

    LapseSpot anchor = { bx, by };
    LapseAnchor(bot, anchor);

    bool const healer = IsLapseHealer(bot);

    float const routeBar = std::min(SPHERE_TRANSIT, here - SPHERE_ROUTE_SLACK);

    bool found = false;
    float best = 0.0f;

    bool haveFallback = false;
    float fallbackClearance = 0.0f;
    LapseSpot fallback = { bx, by };

    for (float radius : SPHERE_DODGE_RADII)
    {
        for (int i = 0; i < SPHERE_DODGE_BEARINGS; ++i)
        {
            float const angle = (2.0f * static_cast<float>(M_PI) * i) / SPHERE_DODGE_BEARINGS;
            float const x = bx + std::cos(angle) * radius;
            float const y = by + std::sin(angle) * radius;

            if (!InsideKiteArena(x, y, healer))
                continue;

            float clearance = std::numeric_limits<float>::max();
            float route = std::numeric_limits<float>::max();
            for (Unit* sphere : world.spheres)
            {
                clearance = std::min(clearance, DistanceToSphereSweep(sphere, x, y));
                route = std::min(route, WalkClearanceOfSphere(sphere, bx, by, x, y));
            }

            if (route < routeBar)
                continue;

            if (clearance < SPHERE_CLEAR)
            {
                if (clearance > here + SPHERE_DODGE_MIN_GAIN && clearance > fallbackClearance)
                {
                    haveFallback = true;
                    fallbackClearance = clearance;
                    fallback = { x, y };
                }

                continue;
            }

            float const margin = std::min(clearance - SPHERE_CLEAR, SPHERE_DODGE_MARGIN_CAP);
            float const score = std::hypot(x - anchor.x, y - anchor.y) +
                                radius * SPHERE_DODGE_TRAVEL_COST - margin * SPHERE_DODGE_MARGIN_WEIGHT +
                                (InsideAnyBurn(world.burns, x, y) ? LAPSE_BURN_COST : 0.0f);

            if (!found || score < best)
            {
                best = score;
                out = { x, y };
                found = true;
            }
        }
    }

    if (found)
        return true;

    if (haveFallback)
    {
        out = fallback;
        return true;
    }

    return false;
}

bool GetGravityLapseSpot(Player* bot, LapseWorld const& world, LapseSpot& out)
{
    if (!LapseAnchor(bot, out))
        return false;

    LapseSpot const anchor = out;

    for (Unit* sphere : world.spheres)
    {
        float dx = out.x - sphere->GetPositionX();
        float dy = out.y - sphere->GetPositionY();
        float dist = std::hypot(dx, dy);
        if (dist >= SPHERE_CLEAR)
            continue;

        if (dist < 0.1f)
        {
            dx = out.x - KAEL_P2.x;
            dy = out.y - KAEL_P2.y;
            dist = std::hypot(dx, dy);
            if (dist < 0.1f)
                break;
        }

        out.x = sphere->GetPositionX() + (dx / dist) * SPHERE_CLEAR;
        out.y = sphere->GetPositionY() + (dy / dist) * SPHERE_CLEAR;
    }

    float const pushX = out.x - anchor.x;
    float const pushY = out.y - anchor.y;
    float const push = std::hypot(pushX, pushY);
    if (push > LAPSE_PUSH_MAX)
    {
        out.x = anchor.x + (pushX / push) * LAPSE_PUSH_MAX;
        out.y = anchor.y + (pushY / push) * LAPSE_PUSH_MAX;
    }

    bool const healer = IsLapseHealer(bot);
    if (InsideKiteArena(anchor.x, anchor.y, healer) && !InsideKiteArena(out.x, out.y, healer))
        out = anchor;

    return true;
}

bool OnLapseStation(Player* bot, Unit* kael, LapseSpot const& spot)
{
    if (!IsLapseMeleeSlot(bot) || !kael)
        return bot->GetExactDist2d(spot.x, spot.y) <= LAPSE_TOLERANCE;

    return bot->IsWithinCombatRange(kael, sPlayerbotAIConfig.MeleeDistance + CONTACT_DISTANCE);
}

bool LapseThreatened(Player* bot, LapseWorld const& world)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    for (Unit* sphere : world.spheres)
    {
        if (DistanceToSphereSweep(sphere, bx, by) < SPHERE_ALERT)
            return true;
    }

    return false;
}

bool GetLapseApproach(Player* bot, LapseWorld const& world, LapseSpot const& station, bool allowBlocked,
                      LapseSpot& out)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    float const gap = std::hypot(station.x - bx, station.y - by);
    if (gap < 0.1f)
        return false;

    float const leg = std::min(LAPSE_LEG, gap);
    float const progressBar = std::min(LAPSE_PROGRESS_MIN, leg * 0.5f);
    float const dx = (station.x - bx) / gap;
    float const dy = (station.y - by) / gap;

    float here = std::numeric_limits<float>::max();
    for (Unit* sphere : world.spheres)
        here = std::min(here, DistanceToSphereSweep(sphere, bx, by));

    float const routeBar = std::min(SPHERE_TRANSIT, here - SPHERE_ROUTE_SLACK);

    bool haveFallback = false;
    float fallbackRoute = -1.0f;
    LapseSpot fallback = { bx, by };

    for (float magnitude : LAPSE_DETOUR_TURNS)
    {
        bool found = false;
        float bestRoute = 0.0f;
        LapseSpot best = { bx, by };

        for (int side = 0; side < 2; ++side)
        {
            if (side && magnitude <= 0.0f)
                continue;

            float const angle = (side ? -magnitude : magnitude) * static_cast<float>(M_PI) / 180.0f;
            float const cosT = std::cos(angle);
            float const sinT = std::sin(angle);

            float const x = bx + (dx * cosT - dy * sinT) * leg;
            float const y = by + (dx * sinT + dy * cosT) * leg;

            if (gap - std::hypot(station.x - x, station.y - y) < progressBar)
                continue;

            float route = std::numeric_limits<float>::max();
            for (Unit* sphere : world.spheres)
                route = std::min(route, WalkClearanceOfSphere(sphere, bx, by, x, y));

            if (route < routeBar)
            {
                if (route > fallbackRoute)
                {
                    haveFallback = true;
                    fallbackRoute = route;
                    fallback = { x, y };
                }

                continue;
            }

            if (!found || route > bestRoute)
            {
                found = true;
                bestRoute = route;
                best = { x, y };
            }
        }

        if (found)
        {
            out = best;
            return true;
        }
    }

    if (!allowBlocked || !haveFallback)
        return false;

    out = fallback;

    return true;
}

void CollectMeleePhoenixExclusions(Player* bot, std::vector<ObjectGuid>& out)
{
    if (!PlayerbotAI::IsMelee(bot) || !GetKaelthas(bot))
        return;

    std::vector<Unit*> phoenixes;
    CollectCreaturesByEntry(bot, { NPC_PHOENIX }, PHOENIX_SCAN, phoenixes);

    for (Unit* phoenix : phoenixes)
        out.push_back(phoenix->GetGUID());
}
}
