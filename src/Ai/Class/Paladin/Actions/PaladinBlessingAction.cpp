/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#include "PaladinBlessingAction.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "GenericBuffUtils.h"
#include "PaladinBlessingHelper.h"
#include "Pet.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

using namespace ai::blessing;

namespace ai::blessing
{

RoleProfile ResolveRoleProfile(Player* player)
{
    if (!player)
        return ROLE_CASTER;

    switch (player->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_DEATH_KNIGHT:
            return PlayerbotAI::IsTank(player) ? ROLE_WARRIOR_DK_TANK : ROLE_PHYSICAL_DPS;
        case CLASS_ROGUE:
            return ROLE_PHYSICAL_DPS;
        case CLASS_HUNTER:
            return ROLE_HYBRID_DPS;
        case CLASS_SHAMAN:
            return AiFactory::GetPlayerSpecTab(player) == SHAMAN_TAB_ENHANCEMENT
                ? ROLE_HYBRID_DPS : ROLE_CASTER;
        case CLASS_PALADIN:
            if (PlayerbotAI::IsTank(player))
                return ROLE_PALADIN_TANK;
            return AiFactory::GetPlayerSpecTab(player) == PALADIN_TAB_HOLY
                ? ROLE_CASTER : ROLE_HYBRID_DPS;
        case CLASS_DRUID:
            if (AiFactory::GetPlayerSpecTab(player) == DRUID_TAB_FERAL)
                return PlayerbotAI::IsTank(player) ? ROLE_DRUID_TANK : ROLE_HYBRID_DPS;
            return ROLE_CASTER;
        default:
            return ROLE_CASTER;
    }
}

bool IsEligibleGroupForAutoBlessings(Group const* group)
{
    return ai::buff::IsGroupEligibleForBuffMode(group, sPlayerbotAIConfig.AutoGreaterBlessings);
}

PaladinBlessingPlanner::PaladinBlessingPlanner(PlayerbotAI* botAI)
    : botAI(botAI), bot(botAI->GetBot())
{
}

PendingBlessing PaladinBlessingPlanner::Plan()
{
    if (!bot->IsAlive())
        return {};

    BlessingConstraints c;
    std::unordered_map<uint8, ClassGroup> classGroups;
    bool renewedHopePresent = false;   // a Disc priest's Renewed Hope suppresses Sanctuary
    bool recentLogin = false;          // any member still in post-login grace
    _coverage.clear();
    _humanCaster.clear();

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            ProcessMember(ref->GetSource(), c, classGroups, renewedHopePresent, recentLogin);
    }
    else
    {
        ProcessMember(bot, c, classGroups, renewedHopePresent, recentLogin);
    }

    if (recentLogin)
        return {};

    return SelectCast(c, classGroups, renewedHopePresent);
}

// Provider/role facts and per-class demand are gathered regardless of alive state, so the
// deterministic assignment stays stable across deaths and reses; only alive, assist-valid members
// (and their pets) are scanned into _coverage and become actual blessing targets.
void PaladinBlessingPlanner::ProcessMember(
    Player* m, BlessingConstraints& c,
    std::unordered_map<uint8, ClassGroup>& classGroups, bool& renewedHopePresent, bool& recentLogin)
{
    if (!m || !m->IsInWorld() || m->GetMap() != bot->GetMap())
        return;

    if (ai::buff::IsWithinPostLoginBuffGrace(m))
        recentLogin = true;

    auto addDemand = [&](Unit* u)
    {
        auto& cg = classGroups[u->getClass()];
        cg.cls = u->getClass();
        cg.members.push_back(u);
        if (u->IsAlive() && bot->IsValidAssistTarget(u))
            _coverage[u->GetGUID()] = ai::blessing::scanBlessingAuras(u, _humanCaster);
    };

    if (m->getClass() == CLASS_PRIEST &&
        (m->HasAura(SPELL_RENEWED_HOPE_R1) || m->HasAura(SPELL_RENEWED_HOPE_R2)))
        renewedHopePresent = true;

    if (m->getClass() == CLASS_PALADIN)
    {
        PlayerbotAI* mAI = GET_PLAYERBOT_AI(m);
        PaladinFlags f;
        f.guid           = m->GetGUID();
        f.isHuman        = (mAI == nullptr);
        f.hasKings       = m->HasSpell(SPELL_BLESSING_OF_KINGS);
        f.hasSanctuary   = m->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
        f.mightStrength  = ai::blessing::HighestKnownBlessingStrength(m, BASE_MIGHT);
        f.wisdomStrength = ai::blessing::HighestKnownBlessingStrength(m, BASE_WISDOM);
        c.paladins.push_back(f);

        if (mAI)
        {
            ObjectGuid const g = m->GetGUID();
            if (mAI->HasStrategy("bmight",  BOT_STATE_NON_COMBAT)) c.forced.push_back({ g, BASE_MIGHT });
            if (mAI->HasStrategy("bwisdom", BOT_STATE_NON_COMBAT)) c.forced.push_back({ g, BASE_WISDOM });
            if (mAI->HasStrategy("bkings",  BOT_STATE_NON_COMBAT)) c.forced.push_back({ g, BASE_KINGS });
            if (mAI->HasStrategy("bsanc",   BOT_STATE_NON_COMBAT)) c.forced.push_back({ g, BASE_SANCTUARY });
        }
    }

    addDemand(m);

    if (Pet* pet = m->GetPet();
        pet && pet->IsAlive() && pet->IsInWorld() && pet->GetMap() == bot->GetMap())
        addDemand(pet);
}

PendingBlessing PaladinBlessingPlanner::SelectCast(
    BlessingConstraints const& constraints,
    std::unordered_map<uint8, ClassGroup> const& classGroups, bool renewedHopePresent)
{
    _intended.clear();

    // Bot paladins (the only actors), sorted by GUID; find my own index.
    std::vector<PaladinFlags> bots;
    bots.reserve(constraints.paladins.size());
    for (auto const& p : constraints.paladins)
        if (!p.isHuman)
            bots.push_back(p);
    std::sort(bots.begin(), bots.end(),
        [](PaladinFlags const& l, PaladinFlags const& r) { return l.guid < r.guid; });
    std::optional<std::size_t> selfIdx;
    for (std::size_t i = 0; i < bots.size(); ++i)
        if (bots[i].guid == bot->GetGUID())
        {
            selfIdx = i;
            break;
        }
    if (!selfIdx)
        return {};

    std::unordered_map<uint8, BaseBlessingCategory> classMajority;
    classMajority.reserve(classGroups.size());
    std::vector<BaseBlessingCategory> myCats;
    myCats.reserve(BLESSING_PRIORITY_SLOTS);
    for (auto const& [cls, cg] : classGroups)
        classMajority[cls] = PlanClass(cg, bots, *selfIdx, constraints, renewedHopePresent, myCats);

    // classGroups is unordered; sort so the order I try my owed categories is stable.
    std::sort(myCats.begin(), myCats.end());

    if (myCats.empty())
        return {};

    PendingBlessing const greater = SelectGreater(classGroups, classMajority, myCats);
    if (greater.spellId)
        return greater;
    return SelectSingle(classGroups, classMajority, myCats);
}

BaseBlessingCategory PaladinBlessingPlanner::PlanClass(
    ClassGroup const& group,
    std::vector<PaladinFlags> const& bots, std::size_t selfIdx,
    BlessingConstraints const& constraints, bool renewedHopePresent,
    std::vector<BaseBlessingCategory>& myCats)
{
    std::size_t const M = group.members.size();
    std::size_t const N = bots.size();

    // A human GREATER blessing is class-wide → yield that category for the WHOLE class.  A
    // human SINGLE covers only one character → handled per-member (that member skips it below).
    std::array<bool, BLESSING_CATEGORY_SLOTS> humanGreaterCoversClass{};
    for (Unit* m : group.members)
    {
        auto it = _coverage.find(m->GetGUID());
        if (it == _coverage.end())
            continue;
        for (auto const& mb : it->second)
            if (mb.isGreater && mb.isHumanPaladinCaster)
                humanGreaterCoversClass[mb.category] = true;
    }

    // Each member's top-N castable wants + priority-weighted class demand.
    std::vector<RoleProfile> roles(M);
    std::vector<std::array<bool, BLESSING_CATEGORY_SLOTS>> wantedSet(M);
    std::array<int, BLESSING_CATEGORY_SLOTS> demand{};
    std::array<bool, BLESSING_CATEGORY_SLOTS> botCanCast{};

    for (BaseBlessingCategory c : { BASE_MIGHT, BASE_WISDOM, BASE_KINGS, BASE_SANCTUARY })
        botCanCast[c] = SomeBotCanCast(bots, c);

    // Greaters land class-wide, so a pet sharing its class with a player adds no demand (it would
    // flip the player's blessing) and takes its wants as exception singles; mage-class pets are casters.
    bool const petsFollow = std::any_of(group.members.begin(), group.members.end(),
        [](Unit* u) { return u->IsPlayer(); });

    for (std::size_t k = 0; k < M; ++k)
    {
        Unit* const member = group.members[k];
        RoleProfile const role = member->IsPlayer()
            ? ai::blessing::ResolveRoleProfile(member->ToPlayer())
            : (member->getClass() == CLASS_MAGE ? ROLE_CASTER : ROLE_PHYSICAL_DPS);
        roles[k] = role;
        bool const addsDemand = member->IsPlayer() || !petsFollow;
        std::size_t taken = 0;
        for (BaseBlessingCategory c : BASE_BLESSING_PRIORITIES[role].priorities)
        {
            if (c == BASE_NONE)
                break;
            if (renewedHopePresent && c == BASE_SANCTUARY && role != ROLE_PALADIN_TANK)
                continue;
            if (humanGreaterCoversClass[c])
                continue;
            if (HasHumanBlessing(group.members[k], c, _coverage))
                continue;
            if (!botCanCast[c])
                continue;
            wantedSet[k][c] = true;
            if (addsDemand)
                demand[c] += RoleWeight(role, c);
            if (++taken == N)
                break;
        }
    }

    // Distinct greater category per paladin for this class; mine, plus what every member receives.
    std::vector<BaseBlessingCategory> const greaters =
        AssignClassGreaters(bots, demand, humanGreaterCoversClass, constraints.forced, renewedHopePresent);
    BaseBlessingCategory const myGreater = greaters[selfIdx];

    std::array<bool, BLESSING_CATEGORY_SLOTS> coveredByGreaters{};
    for (BaseBlessingCategory gc : greaters)
        if (gc != BASE_NONE)
            coveredByGreaters[gc] = true;
    for (BaseBlessingCategory c : { BASE_MIGHT, BASE_WISDOM, BASE_KINGS, BASE_SANCTUARY })
        if (humanGreaterCoversClass[c])
            coveredByGreaters[c] = true;

    // What this bot delivers to each member: my greater if it's wanted, else an exception single.
    for (std::size_t k = 0; k < M; ++k)
    {
        BaseBlessingCategory const intended = (myGreater != BASE_NONE && wantedSet[k][myGreater])
            ? myGreater
            : ResolveExceptionCategory(roles[k], wantedSet[k], coveredByGreaters, greaters, bots, selfIdx);

        if (intended != BASE_NONE)
        {
            _intended[group.members[k]->GetGUID()] = intended;
            if (std::find(myCats.begin(), myCats.end(), intended) == myCats.end())
                myCats.push_back(intended);
        }
    }

    return myGreater;
}

bool PaladinBlessingPlanner::CanBlessNow(Unit* m, uint32 spellId) const
{
    if (!m || !m->IsAlive() || !bot->IsValidAssistTarget(m))
        return false;
    // Range gate before the costly CanCastSpell, which reports out-of-range as castable.
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || !bot->IsWithinDistInMap(m, spellInfo->GetMaxRange(true)))
        return false;
    return botAI->CanCastSpell(spellId, m);
}

namespace
{
    // The core refuses our aura under a stronger "highest wins" buff from elsewhere (Battle Shout vs
    // Might), but CheckCast only tests that for single-target spells, not the area greaters.
    bool OutrankedByOtherBuff(Unit* caster, Unit* member, SpellInfo const* spell)
    {
        uint8 auraMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (spell->Effects[i].IsAura())
                auraMask |= 1 << i;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if ((auraMask & (1 << i)) && !member->IsHighestExclusiveAuraEffect(spell,
                    spell->Effects[i].ApplyAuraName, spell->Effects[i].CalcValue(caster), auraMask))
                return true;
        return false;
    }
}

PendingBlessing PaladinBlessingPlanner::SelectGreater(
    std::unordered_map<uint8, ClassGroup> const& groups,
    std::unordered_map<uint8, BaseBlessingCategory> const& classMajority,
    std::vector<BaseBlessingCategory> const& myCats)
{
    ObjectGuid const botGuid = bot->GetGUID();
    for (BaseBlessingCategory const cat : myCats)
    {
        // No id = can't cast this greater (untrained or config-disabled); no reagent = nothing to
        // cast.  Either way SelectSingle handles the category.
        std::optional<uint32> greaterId;
        if (ai::blessing::IsEligibleGroupForAutoBlessings(bot->GetGroup()))
            greaterId = HighestKnownBlessingRank(bot, cat, true);
        if (!greaterId || !ai::buff::HasRequiredReagents(bot, *greaterId))
            continue;
        SpellInfo const* greaterInfo = sSpellMgr->GetSpellInfo(*greaterId);
        if (!greaterInfo)
            continue;

        int32 const myStrength = GetBlessingCastStrength(bot, ToGreaterVariant(cat), *greaterId);

        for (auto const& [cls, cg] : groups)
        {
            // One greater per class: only its majority category (minority specs → SelectSingle).
            auto mIt = classMajority.find(cls);
            if (mIt == classMajority.end() || mIt->second != cat)
                continue;

            // First member who wants `cat` from me, lacks it (not covered by me, a human, or a
            // stronger blessing), and that I can cast on now.  A greater is class-wide, so any works.
            Unit* target = nullptr;
            bool classNeedsGreater = false;
            for (Unit* m : cg.members)
            {
                auto intIt = _intended.find(m->GetGUID());
                if (intIt == _intended.end() || intIt->second != cat)
                    continue;
                if (IsCoveredByMe(botGuid, m, cat, _coverage))
                    continue;
                if (HasHumanBlessing(m, cat, _coverage))
                    continue;
                if (HasStrongerBlessing(m, cat, myStrength, _coverage))
                    continue;
                if (OutrankedByOtherBuff(bot, m, greaterInfo))
                    continue;
                classNeedsGreater = true;
                if (CanBlessNow(m, *greaterId))
                {
                    target = m;
                    break;
                }
            }

            if (!classNeedsGreater || !target)
                continue;

            return { *greaterId, target->GetGUID(), cat, ReagentNoticeKind::Clear };
        }
    }
    return {};
}

PendingBlessing PaladinBlessingPlanner::SelectSingle(
    std::unordered_map<uint8, ClassGroup> const& groups,
    std::unordered_map<uint8, BaseBlessingCategory> const& classMajority,
    std::vector<BaseBlessingCategory> const& myCats)
{
    ObjectGuid const botGuid = bot->GetGUID();
    for (BaseBlessingCategory const cat : myCats)
    {
        std::optional<uint32> const singleId = HighestKnownBlessingRank(bot, cat, false);
        if (!singleId)
            continue;

        std::optional<uint32> greaterId;
        if (ai::blessing::IsEligibleGroupForAutoBlessings(bot->GetGroup()))
            greaterId = HighestKnownBlessingRank(bot, cat, true);
        bool const hasReagent = greaterId && ai::buff::HasRequiredReagents(bot, *greaterId);

        int32 const myStrength = GetBlessingCastStrength(bot, ToSingleVariant(cat), *singleId);

        for (auto const& [cls, cg] : groups)
        {
            // With a reagent, only minority-spec exceptions need a single (the majority gets a
            // greater); without one, every member.
            auto mIt = classMajority.find(cls);
            bool const isException = (mIt == classMajority.end() || cat != mIt->second);
            if (hasReagent && !isException)
                continue;

            for (Unit* m : cg.members)
            {
                auto intIt = _intended.find(m->GetGUID());
                if (intIt == _intended.end() || intIt->second != cat)
                    continue;
                if (IsCoveredByMe(botGuid, m, cat, _coverage))
                    continue;
                if (HasHumanBlessing(m, cat, _coverage))
                    continue;
                if (HasStrongerBlessing(m, cat, myStrength, _coverage))
                    continue;
                if (!CanBlessNow(m, *singleId))
                    continue;

                ReagentNoticeKind notice = ReagentNoticeKind::None;
                if (greaterId)
                    notice = hasReagent ? ReagentNoticeKind::Clear : ReagentNoticeKind::Announce;

                return { *singleId, m->GetGUID(), cat, notice };
            }
        }
    }
    return {};
}

}

Unit* CastBlessingAction::GetTarget()
{
    return botAI->GetUnit(pending.target);
}

bool CastBlessingAction::isUseful()
{
    pending = PaladinBlessingPlanner(botAI).Plan();
    return pending.spellId && GetTarget();
}

bool CastBlessingAction::isPossible() { return true; }

bool CastBlessingAction::Execute(Event /*event*/)
{
    PendingBlessing const pb = pending;
    if (!pb.spellId)
        return false;

    Unit* target = botAI->GetUnit(pb.target);
    if (!target)
        return false;

    bool const cast = botAI->CastSpell(pb.spellId, target);

    if (pb.notice == ReagentNoticeKind::Clear)
        ai::buff::ClearMissingBuffReagentNotice(botAI, BlessingSpellName(ToGreaterVariant(pb.category)));
    else if (pb.notice == ReagentNoticeKind::Announce)
        ai::buff::TryAnnounceMissingBuffReagents(
            botAI, BlessingSpellName(ToSingleVariant(pb.category)), BlessingSpellName(ToGreaterVariant(pb.category)));

    return cast;
}
