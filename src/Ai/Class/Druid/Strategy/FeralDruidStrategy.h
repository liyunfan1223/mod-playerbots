/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FERALDRUIDSTRATEGY_H
#define PLAYERBOTS_FERALDRUIDSTRATEGY_H

#include "Action.h"
#include "GenericDruidStrategy.h"

constexpr uint32 SPELL_CAT_FORM  = 768;
constexpr uint32 AURA_THICK_HIDE = 16931;

class PlayerbotAI;

class ShapeshiftDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ShapeshiftDruidStrategyActionNodeFactory()
    {
        creators["rejuvenation"] = &rejuvenation;
        creators["regrowth"] = &regrowth;
        creators["healing touch"] = &healing_touch;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
        creators["regrowth on party"] = &regrowth_on_party;
        creators["healing touch on party"] = &healing_touch_on_party;
    }

private:
    static ActionNode* regrowth([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ { NextAction("healing touch") },
                              /*C*/ { NextAction("melee", 10.0f) });
    }

    static ActionNode* rejuvenation([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* healing_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ { NextAction("healing touch on party") },
                              /*C*/ { NextAction("melee", 10.0f) });
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch on party",
                              /*P*/ { NextAction("caster form") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

class FeralDruidStrategy : public GenericDruidStrategy
{
protected:
    FeralDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override { return STRATEGY_TYPE_MELEE; }
};

// Optional strategy — enabled by default for cat and bear.
// Registers the "enemy out of melee" → Feral Charge trigger, spec-gated at
// init time so cats get Feral Charge (Cat) and bears get Feral Charge (Bear).
// Disable with: co -feral charge
// Re-enable with: co +feral charge
class FeralChargeDruidStrategy : public CombatStrategy
{
public:
    FeralChargeDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "feral charge"; }
};

#endif
