/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDENCOUNTER_H
#define PLAYERBOTS_RAIDENCOUNTER_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Multiplier.h"
#include "RaidEncounterRules.h"
#include "Trigger.h"

class Action;
class PlayerbotAI;

// Says whether a boss's mechanics may run. A raid strategy registers every boss at once, so without a
// gate they'd all run on every pull.
using EncounterGate = bool (*)(PlayerbotAI* botAI, uint32 encounterId);

// Keyed by the InstanceScript boss index, rule in RaidEncounterRules::GateOpen. Gate on the fight, not
// the unit: an add a boss calls in (Sartharion's drakes) never starts an encounter of its own.
bool BossStateGateOpen(PlayerbotAI* botAI, uint32 encounterId);
// The encounter itself is in progress. Unlike the open gate, false while nothing is engaged.
bool BossStateGateLive(PlayerbotAI* botAI, uint32 encounterId);

// Non-zero only inside a gated trigger's Check, unique per trigger pass. Triggers don't change the
// world, so a read cached under one id holds for that pass; actions and multipliers always get 0.
uint32 EncounterTriggerPassId(PlayerbotAI* botAI);

// Wraps rather than subclasses, so the trigger classes and their IsActive bodies stay as they are.
class EncounterGatedTrigger : public Trigger
{
public:
    EncounterGatedTrigger(PlayerbotAI* botAI, Trigger* inner, EncounterGate gate, uint32 encounterId);
    ~EncounterGatedTrigger() override;

    Event Check() override;
    bool IsActive() override;
    bool IsBuffTrigger() override;
    bool IsDebuffTrigger() override;
    std::vector<NextAction> getHandlers() override;
    void Reset() override;
    Unit* GetTarget() override;
    Value<Unit*>* GetTargetValue() override;
    std::string const GetTargetName() override;
    void ExternalEvent(std::string const param, Player* owner = nullptr) override;
    void ExternalEvent(WorldPacket& packet, Player* owner = nullptr) override;

private:
    Trigger* _inner;
    EncounterGate _gate;
    uint32 _encounterId;
};

// The only place an Action is mapped to rule families.
RaidEncounterRules::FamilyMask ClassifyAction(Action* action);

enum class EncounterRow : uint8
{
    Plain,
    Mover  // passes the boss's OwnMovement rules
};

class EncounterBuilder;

// One boss, declared once: its trigger/action rows, rules, hand-written multipliers and tick. The
// raid's contexts and strategy are built from it, and its gate covers all of it.
class EncounterDefinition
{
public:
    using Define = void (*)(EncounterBuilder&);
    using Predicate = bool (*)(PlayerbotAI*);
    using TickFn = void (*)(PlayerbotAI*);
    using TriggerCreator = std::function<Trigger*(PlayerbotAI*)>;
    using ActionCreator = std::function<Action*(PlayerbotAI*)>;
    using MultiplierCreator = std::function<Multiplier*(PlayerbotAI*)>;

    EncounterDefinition(uint32 encounterId, EncounterGate gate, Define define);

    // Register after any other wrapping pass over the same map, or a trigger is gated twice.
    void RegisterTriggers(std::unordered_map<std::string, TriggerCreator>& creators) const;
    void RegisterActions(std::unordered_map<std::string, ActionCreator>& creators) const;
    void AddTriggerNodes(std::vector<TriggerNode*>& triggers) const;
    void AddMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers) const;

    // Housekeeping for Strategy::OnTick. Runs only while the gate is open.
    void OnTick(PlayerbotAI* botAI) const;

private:
    friend class EncounterBuilder;

    struct Row
    {
        std::string trigger;
        TriggerCreator makeTrigger;
        std::string action;
        ActionCreator makeAction;
        float priority;
        EncounterRow flag;
    };

    uint32 _encounterId;
    EncounterGate _gate;
    std::vector<Row> _rows;
    std::vector<MultiplierCreator> _multipliers;  // in declaration order, which is veto order
    TickFn _tick = nullptr;
};

// What a definition function gets. Rules and multipliers keep the order they're declared in: the
// first one to zero an action is the one a trace credits with the veto.
class EncounterBuilder
{
public:
    using FamilyMask = RaidEncounterRules::FamilyMask;
    using RoleMask = RaidEncounterRules::RoleMask;
    using Predicate = EncounterDefinition::Predicate;

    // Both classes carry their own name as a static `Name`, used by their constructors. Rows naming the
    // same trigger back to back share one node.
    template <class T, class A>
    void Node(float priority, EncounterRow flag = EncounterRow::Plain)
    {
        Node(T::Name, [](PlayerbotAI* botAI) -> Trigger* { return new T(botAI); }, A::Name,
             [](PlayerbotAI* botAI) -> Action* { return new A(botAI); }, priority, flag);
    }

    // For an action a shared context already registers, such as "rear flank": the row registers only
    // its trigger.
    template <class T>
    void Node(char const* sharedAction, float priority, EncounterRow flag = EncounterRow::Plain)
    {
        Node(T::Name, [](PlayerbotAI* botAI) -> Trigger* { return new T(botAI); }, sharedAction, nullptr, priority,
             flag);
    }

    // For a class built under several names, which takes them from the row.
    void Node(std::string trigger, EncounterDefinition::TriggerCreator makeTrigger, std::string action,
              EncounterDefinition::ActionCreator makeAction, float priority,
              EncounterRow flag = EncounterRow::Plain);

    // An empty movers list means every Mover row of this definition.
    void OwnMovement(char const* name, RoleMask roles, Predicate predicate,
                     FamilyMask keep = RaidEncounterRules::Family::Attack | RaidEncounterRules::Family::Reach,
                     std::vector<std::string> movers = {});
    void OwnTargeting(char const* name, RoleMask roles, Predicate predicate,
                      FamilyMask pickers = RaidEncounterRules::Family::DpsAssist |
                                           RaidEncounterRules::Family::TankAssist);
    void Block(char const* name, RoleMask roles, Predicate predicate, FamilyMask families,
               std::vector<std::string> names = {});
    void Exclusive(char const* name, RoleMask roles, Predicate predicate, FamilyMask families = 0,
                   std::vector<std::string> names = {});

    // For what no rule kind fits. The multiplier only sees actions in these families, and only while
    // the gate is open. Any args follow the PlayerbotAI into its constructor.
    template <class M, class... Args>
    void Multiplier(FamilyMask families, Args... args)
    {
        HandWritten(families, [args...](PlayerbotAI* botAI) -> ::Multiplier* { return new M(botAI, args...); });
    }

    void Tick(EncounterDefinition::TickFn tick);

private:
    friend class EncounterDefinition;

    explicit EncounterBuilder(EncounterDefinition& definition) : _definition(definition) {}

    void AddRule(char const* name, RoleMask roles, Predicate predicate, RaidEncounterRules::Rule rule,
                 bool moverRows);
    void HandWritten(FamilyMask families, EncounterDefinition::MultiplierCreator make);

    EncounterDefinition& _definition;
    std::vector<std::shared_ptr<RaidEncounterRules::Rule>> _moverRules;  // filled once define returns
};

#endif
