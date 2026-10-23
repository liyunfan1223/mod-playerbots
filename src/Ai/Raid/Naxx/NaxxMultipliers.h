/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NAXXMULTIPLIERS_H
#define PLAYERBOTS_NAXXMULTIPLIERS_H

#include "Multiplier.h"
#include "NaxxBossHelper.h"

class GrobbulusMultiplier : public Multiplier
{
public:
    GrobbulusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "grobbulus") {}

public:
    float GetValue(Action* action) override;
};

class HeiganDanceMultiplier : public Multiplier
{
public:
    explicit HeiganDanceMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "heigan dance"), helper(botAI) {}

public:
    float GetValue(Action* action) override;

private:
    HeiganBossHelper helper;
};

class LoathebGenericMultiplier : public Multiplier
{
public:
    LoathebGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "loatheb generic") {}

public:
    float GetValue(Action* action) override;
};

class ThaddiusGenericMultiplier : public Multiplier
{
public:
    ThaddiusGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "thaddius generic"), helper(botAI) {}

public:
    float GetValue(Action* action) override;

private:
    ThaddiusBossHelper helper;
};

class SapphironGenericMultiplier : public Multiplier
{
public:
    SapphironGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sapphiron generic"), helper(botAI) {}

    float GetValue(Action* action) override;

private:
    SapphironBossHelper helper;
};

class InstructorRazuviousGenericMultiplier : public Multiplier
{
public:
    InstructorRazuviousGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "instructor razuvious generic"), helper(botAI) {}
    float GetValue(Action* action) override;

private:
    RazuviousBossHelper helper;
};

class KelthuzadGenericMultiplier : public Multiplier
{
public:
    KelthuzadGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "kelthuzad generic"), helper(botAI) {}
    float GetValue(Action* action) override;

private:
    KelthuzadBossHelper helper;
};

class AnubrekhanGenericMultiplier : public Multiplier
{
public:
    AnubrekhanGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "anubrekhan generic") {}

public:
    float GetValue(Action* action) override;
};

class FourHorsemenGenericMultiplier : public Multiplier
{
public:
    FourHorsemenGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "four horsemen generic") {}

public:
    float GetValue(Action* action) override;
};

// class GothikGenericMultiplier : public Multiplier
// {
// public:
//     GothikGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gothik generic") {}

// public:
//     float GetValue(Action* action) override;
// };

class GluthGenericMultiplier : public Multiplier
{
public:
    GluthGenericMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gluth generic"), helper(botAI) {}
    float GetValue(Action* action) override;

private:
    GluthBossHelper helper;
};

#endif
