/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidEncounter.h"

#include <algorithm>
#include <typeinfo>

#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "InstanceScript.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PetsAction.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include "WarriorActions.h"

namespace Rules = RaidEncounterRules;
namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
    // One bot's trigger checks: opened by the first gate read, closed by the Reset sweep after the
    // checks. Reset isn't guaranteed (a strategy swap rebuilds the list), so the ms stamp also ends it.
    struct GatePass
    {
        PlayerbotAI* botAI = nullptr;
        uint32 atMs = 0;
        uint32 id = 0;
    };

    thread_local GatePass gatePass;  // a bot's whole tick runs on one map thread

    // Kept out of GatePass: ending a pass overwrites it, and an id that restarts would hand a new pass
    // the previous one's cached reads.
    thread_local uint32 gatePassCounter = 0;

    uint32 CurrentGatePass(PlayerbotAI* botAI)
    {
        uint32 const now = getMSTime();
        if (!gatePass.id || gatePass.botAI != botAI || gatePass.atMs != now)
        {
            gatePass.botAI = botAI;
            gatePass.atMs = now;

            // 0 means no pass, so skip it on wrap.
            gatePass.id = ++gatePassCounter;
            if (!gatePass.id)
                gatePass.id = ++gatePassCounter;
        }

        return gatePass.id;
    }

    void EndGatePass(PlayerbotAI* botAI)
    {
        if (gatePass.botAI == botAI)
            gatePass = GatePass();
    }

    // Set only while an inner trigger is being checked.
    struct CheckScope
    {
        PlayerbotAI* botAI = nullptr;
        uint32 passId = 0;
    };

    thread_local CheckScope checkScope;

    struct CheckScopeGuard
    {
        CheckScope saved;

        CheckScopeGuard(PlayerbotAI* botAI, uint32 passId) : saved(checkScope) { checkScope = {botAI, passId}; }
        ~CheckScopeGuard() { checkScope = saved; }
    };

    InstanceScript* InstanceOf(PlayerbotAI* botAI)
    {
        Player* bot = botAI ? botAI->GetBot() : nullptr;
        return bot ? bot->GetInstanceScript() : nullptr;
    }

    Rules::BossStates ReadBossStates(PlayerbotAI* botAI)
    {
        Rules::BossStates states;
        InstanceScript* instance = InstanceOf(botAI);
        if (!instance)
            return states;

        states.inInstance = true;
        states.encounterCount = std::min<uint32>(instance->GetEncounterCount(), Rules::MAX_BOSS_STATES);
        for (uint32 id = 0; id < states.encounterCount; ++id)
        {
            EncounterState const state = instance->GetBossState(id);
            if (state == IN_PROGRESS)
                states.inProgress |= uint64(1) << id;
            else if (state == DONE)
                states.done |= uint64(1) << id;
        }

        return states;
    }

    bool BotHasRole(PlayerbotAI* botAI, Rules::RoleMask role)
    {
        Player* bot = botAI->GetBot();
        switch (role)
        {
            case Role::Tank:
                return PlayerbotAI::IsTank(bot);
            case Role::MainTank:
                return PlayerbotAI::IsMainTank(bot);
            case Role::Heal:
                return PlayerbotAI::IsHeal(bot);
            case Role::Ranged:
                return PlayerbotAI::IsRanged(bot);
            case Role::Melee:
                return PlayerbotAI::IsMelee(bot);
            case Role::NonTank:
                return !PlayerbotAI::IsTank(bot);
            case Role::Dps:
                return PlayerbotAI::IsDps(bot);
            default:
                return false;
        }
    }

    // Actions live as long as the bot's AI and multipliers are rebuilt with every strategy change, so an
    // Action* never goes stale inside one of these.
    class CaresCache
    {
    public:
        template <class Compute>
        bool Get(Action* action, Compute&& compute)
        {
            auto const found = _cares.find(action);
            if (found != _cares.end())
                return found->second;

            bool const result = compute();
            _cares.emplace(action, result);
            return result;
        }

    private:
        std::unordered_map<Action*, bool> _cares;
    };

    class EncounterRuleMultiplier : public Multiplier
    {
    public:
        EncounterRuleMultiplier(PlayerbotAI* botAI, char const* name, std::shared_ptr<Rules::Rule const> rule,
                                Rules::RoleMask roles, EncounterDefinition::Predicate predicate, EncounterGate gate,
                                uint32 encounterId)
            : Multiplier(botAI, name),
              _rule(std::move(rule)),
              _roles(roles),
              _predicate(predicate),
              _gate(gate),
              _encounterId(encounterId)
        {
        }

        float GetValue(Action* action) override
        {
            if (!action)
                return 1.0f;

            bool const cares = _cache.Get(
                action, [&] { return Rules::RuleCares(*_rule, ClassifyAction(action), action->getName()); });

            auto const hasRole = [&](Rules::RoleMask role) { return BotHasRole(botAI, role); };
            return Rules::Evaluate(
                cares, [&] { return _gate(botAI, _encounterId); },
                [&] { return Rules::RoleMatches(_roles, hasRole); }, [&] { return _predicate(botAI); });
        }

    private:
        std::shared_ptr<Rules::Rule const> _rule;
        Rules::RoleMask _roles;
        EncounterDefinition::Predicate _predicate;
        EncounterGate _gate;
        uint32 _encounterId;
        CaresCache _cache;
    };

    class EncounterGatedMultiplier : public Multiplier
    {
    public:
        EncounterGatedMultiplier(PlayerbotAI* botAI, Multiplier* inner, Rules::FamilyMask families, EncounterGate gate,
                                 uint32 encounterId)
            : Multiplier(botAI, inner->getName()),
              _inner(inner),
              _families(families),
              _gate(gate),
              _encounterId(encounterId)
        {
        }

        ~EncounterGatedMultiplier() override { delete _inner; }

        float GetValue(Action* action) override
        {
            if (!action || !_cache.Get(action, [&] { return (ClassifyAction(action) & _families) != 0; }) ||
                !_gate(botAI, _encounterId))
                return 1.0f;

            return _inner->GetValue(action);
        }

    private:
        Multiplier* _inner;
        Rules::FamilyMask _families;
        EncounterGate _gate;
        uint32 _encounterId;
        CaresCache _cache;
    };
}  // namespace

bool BossStateGateOpen(PlayerbotAI* botAI, uint32 encounterId)
{
    if (!botAI)
        return true;

    // Boss state only moves on a pull, a wipe or a kill, so one read of all of them serves every gate the
    // pass asks.
    thread_local uint32 readInPass = 0;
    thread_local Rules::BossStates states;

    uint32 const pass = CurrentGatePass(botAI);
    if (readInPass != pass)
    {
        states = ReadBossStates(botAI);
        readInPass = pass;
    }

    return Rules::GateOpen(states, encounterId);
}

bool BossStateGateLive(PlayerbotAI* botAI, uint32 encounterId)
{
    InstanceScript* instance = InstanceOf(botAI);
    if (!instance || encounterId >= instance->GetEncounterCount())
        return false;

    return instance->GetBossState(encounterId) == IN_PROGRESS;
}

uint32 EncounterTriggerPassId(PlayerbotAI* botAI)
{
    return botAI && checkScope.botAI == botAI ? checkScope.passId : 0;
}

// Name and check interval come off the inner trigger: Engine::ProcessTriggers calls needCheck on this
// object, so the default interval of 1 would quietly run every throttled trigger every tick.
EncounterGatedTrigger::EncounterGatedTrigger(PlayerbotAI* botAI, Trigger* inner, EncounterGate gate,
                                             uint32 encounterId)
    : Trigger(botAI, inner ? inner->getName() : "trigger", inner ? inner->getCheckInterval() : 1),
      _inner(inner),
      _gate(gate),
      _encounterId(encounterId)
{
}

EncounterGatedTrigger::~EncounterGatedTrigger() { delete _inner; }

Event EncounterGatedTrigger::Check()
{
    if (!_inner || !_gate(botAI, _encounterId))
        return Event();

    CheckScopeGuard const scope(botAI, CurrentGatePass(botAI));
    return _inner->Check();
}

bool EncounterGatedTrigger::IsActive() { return _inner && _gate(botAI, _encounterId) && _inner->IsActive(); }

bool EncounterGatedTrigger::IsBuffTrigger() { return _inner && _inner->IsBuffTrigger(); }

bool EncounterGatedTrigger::IsDebuffTrigger() { return _inner && _inner->IsDebuffTrigger(); }

std::vector<NextAction> EncounterGatedTrigger::getHandlers()
{
    return _inner ? _inner->getHandlers() : std::vector<NextAction>();
}

void EncounterGatedTrigger::Reset()
{
    EndGatePass(botAI);

    if (_inner)
        _inner->Reset();
}

Unit* EncounterGatedTrigger::GetTarget() { return _inner ? _inner->GetTarget() : Trigger::GetTarget(); }

Value<Unit*>* EncounterGatedTrigger::GetTargetValue()
{
    return _inner ? _inner->GetTargetValue() : Trigger::GetTargetValue();
}

std::string const EncounterGatedTrigger::GetTargetName()
{
    return _inner ? _inner->GetTargetName() : Trigger::GetTargetName();
}

void EncounterGatedTrigger::ExternalEvent(std::string const param, Player* owner)
{
    if (_inner)
        _inner->ExternalEvent(param, owner);
}

void EncounterGatedTrigger::ExternalEvent(WorldPacket& packet, Player* owner)
{
    if (_inner)
        _inner->ExternalEvent(packet, owner);
}

Rules::FamilyMask ClassifyAction(Action* action)
{
    if (!action)
        return 0;

    Rules::FamilyMask mask = Family::AnyAction;

    if (dynamic_cast<MovementAction*>(action))
    {
        mask |= Family::AnyMovement;

        if (typeid(*action) == typeid(CombatFormationMoveAction))
            mask |= Family::CombatFormationMove;
        else if (dynamic_cast<TankFaceAction*>(action))
            mask |= Family::TankFace;
        else if (dynamic_cast<SetBehindTargetAction*>(action))
            mask |= Family::SetBehind;
        else if (dynamic_cast<RearFlankAction*>(action))
            mask |= Family::RearFlank;
        else if (dynamic_cast<ReachPartyMemberToHealAction*>(action))
            mask |= Family::ReachHeal;
        else if (dynamic_cast<ReachTargetAction*>(action))
            mask |= Family::Reach;
        else if (dynamic_cast<FollowAction*>(action))
            mask |= Family::Follow;
        else if (dynamic_cast<FleeAction*>(action))
            mask |= Family::Flee;
        else if (dynamic_cast<RunAwayAction*>(action))
            mask |= Family::RunAway;
        else if (dynamic_cast<MoveRandomAction*>(action))
            mask |= Family::MoveRandom;
        else if (dynamic_cast<MoveOutOfCollisionAction*>(action))
            mask |= Family::MoveOutOfCollision;
        else if (dynamic_cast<MoveOutOfEnemyContactAction*>(action))
            mask |= Family::MoveOutOfEnemyContact;
        else if (dynamic_cast<AvoidAoeAction*>(action))
            mask |= Family::AvoidAoe;
        else if (dynamic_cast<AttackAction*>(action))
        {
            mask |= Family::Attack;

            if (dynamic_cast<MeleeAction*>(action))
                mask |= Family::Melee;
            else if (dynamic_cast<DpsAssistAction*>(action))
                mask |= Family::DpsAssist;
            else if (dynamic_cast<TankAssistAction*>(action))
                mask |= Family::TankAssist;
            else if (dynamic_cast<DpsAoeAction*>(action))
                mask |= Family::DpsAoe;
            else if (dynamic_cast<AggressiveTargetAction*>(action))
                mask |= Family::AggressiveTarget;
            else if (dynamic_cast<AttackAnythingAction*>(action))
                mask |= Family::AttackAnything;
            else if (dynamic_cast<AttackLeastHpTargetAction*>(action))
                mask |= Family::AttackLeastHp;
            else if (dynamic_cast<AttackRtiTargetAction*>(action))
                mask |= Family::AttackRti;
        }

        return mask;
    }

    if (dynamic_cast<CastSpellAction*>(action))
    {
        mask |= Family::Spell;

        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            mask |= Family::Charge;
        else if (dynamic_cast<CastBlinkBackAction*>(action))
            mask |= Family::Blink;
        else if (dynamic_cast<CastDisengageAction*>(action))
            mask |= Family::Disengage;
        else if (dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
            mask |= Family::DebuffOnAttacker;
        else if (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastTauntOnSnareTargetAction*>(action) ||
                 dynamic_cast<CastChallengingShoutAction*>(action) ||
                 dynamic_cast<CastGrowlAction*>(action) || dynamic_cast<CastChallengingRoarAction*>(action) ||
                 dynamic_cast<CastHandOfReckoningAction*>(action) ||
                 dynamic_cast<CastRighteousDefenseAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
                 dynamic_cast<CastDeathGripAction*>(action) || dynamic_cast<CastMockingBlowAction*>(action))
            mask |= Family::Taunt;

        return mask;
    }

    if (dynamic_cast<DropTargetAction*>(action))
        mask |= Family::DropTarget;
    else if (dynamic_cast<PetAttackAction*>(action))
        mask |= Family::PetAttack;

    return mask;
}

EncounterDefinition::EncounterDefinition(uint32 encounterId, EncounterGate gate, Define define)
    : _encounterId(encounterId), _gate(gate)
{
    EncounterBuilder builder(*this);
    define(builder);

    std::vector<std::string> movers;
    for (Row const& row : _rows)
        if (row.flag == EncounterRow::Mover)
            movers.push_back(row.action);

    for (std::shared_ptr<Rules::Rule> const& rule : builder._moverRules)
        rule->passNames.insert(rule->passNames.end(), movers.begin(), movers.end());
}

void EncounterDefinition::RegisterTriggers(std::unordered_map<std::string, TriggerCreator>& creators) const
{
    for (Row const& row : _rows)
    {
        TriggerCreator inner = row.makeTrigger;
        EncounterGate const gate = _gate;
        uint32 const encounterId = _encounterId;
        creators[row.trigger] = [inner, gate, encounterId](PlayerbotAI* botAI) -> Trigger*
        { return new EncounterGatedTrigger(botAI, inner(botAI), gate, encounterId); };
    }
}

void EncounterDefinition::RegisterActions(std::unordered_map<std::string, ActionCreator>& creators) const
{
    for (Row const& row : _rows)
        if (row.makeAction)
            creators[row.action] = row.makeAction;
}

void EncounterDefinition::AddTriggerNodes(std::vector<TriggerNode*>& triggers) const
{
    // One node per run of rows: ProcessTriggers checks a quiet trigger again for every node naming it.
    for (std::size_t first = 0; first < _rows.size();)
    {
        std::vector<NextAction> actions;
        std::size_t next = first;
        for (; next < _rows.size() && _rows[next].trigger == _rows[first].trigger; ++next)
            actions.push_back(NextAction(_rows[next].action, _rows[next].priority));

        triggers.push_back(new TriggerNode(_rows[first].trigger, std::move(actions)));
        first = next;
    }
}

void EncounterDefinition::AddMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers) const
{
    for (MultiplierCreator const& make : _multipliers)
        multipliers.push_back(make(botAI));
}

void EncounterDefinition::OnTick(PlayerbotAI* botAI) const
{
    if (_tick && _gate(botAI, _encounterId))
        _tick(botAI);
}

void EncounterBuilder::Node(std::string trigger, EncounterDefinition::TriggerCreator makeTrigger, std::string action,
                            EncounterDefinition::ActionCreator makeAction, float priority, EncounterRow flag)
{
    _definition._rows.push_back(
        {std::move(trigger), std::move(makeTrigger), std::move(action), std::move(makeAction), priority, flag});
}

void EncounterBuilder::OwnMovement(char const* name, RoleMask roles, Predicate predicate, FamilyMask keep,
                                   std::vector<std::string> movers)
{
    bool const moverRows = movers.empty();
    AddRule(name, roles, predicate, Rules::OwnMovement(std::move(movers), keep), moverRows);
}

void EncounterBuilder::OwnTargeting(char const* name, RoleMask roles, Predicate predicate, FamilyMask pickers)
{
    AddRule(name, roles, predicate, Rules::OwnTargeting(pickers), false);
}

void EncounterBuilder::Block(char const* name, RoleMask roles, Predicate predicate, FamilyMask families,
                             std::vector<std::string> names)
{
    AddRule(name, roles, predicate, Rules::Block(families, std::move(names)), false);
}

void EncounterBuilder::Exclusive(char const* name, RoleMask roles, Predicate predicate, FamilyMask families,
                                 std::vector<std::string> names)
{
    AddRule(name, roles, predicate, Rules::Exclusive(families, std::move(names)), false);
}

void EncounterBuilder::Tick(EncounterDefinition::TickFn tick) { _definition._tick = tick; }

void EncounterBuilder::AddRule(char const* name, RoleMask roles, Predicate predicate, Rules::Rule rule, bool moverRows)
{
    auto const shared = std::make_shared<Rules::Rule>(std::move(rule));
    if (moverRows)
        _moverRules.push_back(shared);

    EncounterGate const gate = _definition._gate;
    uint32 const encounterId = _definition._encounterId;
    _definition._multipliers.push_back(
        [name, roles, predicate, shared, gate, encounterId](PlayerbotAI* botAI) -> ::Multiplier*
        { return new EncounterRuleMultiplier(botAI, name, shared, roles, predicate, gate, encounterId); });
}

void EncounterBuilder::HandWritten(FamilyMask families, EncounterDefinition::MultiplierCreator make)
{
    EncounterGate const gate = _definition._gate;
    uint32 const encounterId = _definition._encounterId;
    _definition._multipliers.push_back(
        [families, make, gate, encounterId](PlayerbotAI* botAI) -> ::Multiplier*
        { return new EncounterGatedMultiplier(botAI, make(botAI), families, gate, encounterId); });
}
