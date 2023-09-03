
#ifndef _PLAYERRBOT_RAIDNAXXMULTIPLIERS_H_
#define _PLAYERRBOT_RAIDNAXXMULTIPLIERS_H_

#include "Multiplier.h"
#include "raids/naxxramas/RaidNaxxBossHelper.h"

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

class SapphironGenericMultiplier : public Multiplier
{
    public:
        SapphironGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "sapphiron generic"), helper(ai) {}

        virtual float GetValue(Action* action);
    private:
        SapphironBossHelper helper;
};

class InstructorRazuviousGenericMultiplier : public Multiplier
{
    public:
        InstructorRazuviousGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "instructor razuvious generic"), helper(ai) {}
        virtual float GetValue(Action* action);
    private:
        RazuviousBossHelper helper;
};

class KelthuzadGenericMultiplier : public Multiplier
{
    public:
        KelthuzadGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "kelthuzad generic"), helper(ai) {}
        virtual float GetValue(Action* action);
    private:
        KelthuzadBossHelper helper;
};

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

class GluthGenericMultiplier : public Multiplier
{
    public:
        GluthGenericMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gluth generic"), helper(ai) {}
        float GetValue(Action* action) override;
    private:
        GluthBossHelper helper;
};

#endif