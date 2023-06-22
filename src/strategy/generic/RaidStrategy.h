
#ifndef _PLAYERBOT_RAIDSTRATEGY_H
#define _PLAYERBOT_RAIDSTRATEGY_H

#include "Multiplier.h"
#include "Strategy.h"

class HeiganDanceMultiplier : public Multiplier
{
public:
    HeiganDanceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "helgan dance") {}

public:
    virtual float GetValue(Action* action);
};

// class LoathebGenericMultiplier : public Multiplier
// {
// public:
//     LoathebGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "loatheb generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

// class ThaddiusGenericMultiplier : public Multiplier
// {
// public:
//     ThaddiusGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "thaddius generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

// class SapphironGenericMultiplier : public Multiplier
// {
// public:
//     SapphironGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sapphiron generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

// class InstructorRazuviousGenericMultiplier : public Multiplier
// {
// public:
//     InstructorRazuviousGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "instructor razuvious generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

// class KelthuzadGenericMultiplier : public Multiplier
// {
// public:
//     KelthuzadGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "kelthuzad generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

class AnubrekhanGenericMultiplier : public Multiplier
{
public:
    AnubrekhanGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "anubrekhan generic") {}

public:
    virtual float GetValue(Action* action);
};

// class FourhorsemanGenericMultiplier : public Multiplier
// {
// public:
//     FourhorsemanGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "fourhorseman generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

// class GothikGenericMultiplier : public Multiplier
// {
// public:
//     GothikGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gothik generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

// class GluthGenericMultiplier : public Multiplier
// {
// public:
//     GluthGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gluth generic") {}

// public:
//     virtual float GetValue(Action* action);
// };

class RaidNaxxGenericStrategy : public Strategy
{
public:
    RaidNaxxGenericStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    virtual std::string const getName() override { return "naxx"; }
    virtual void InitTriggers(std::vector<TriggerNode*> &triggers) override;
    virtual void InitMultipliers(std::vector<Multiplier*> &multipliers) override;
};

  
#endif