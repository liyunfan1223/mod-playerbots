#ifndef _PLAYERBOT_RAIDAQ40TRIGGERS_H
#define _PLAYERBOT_RAIDAQ40TRIGGERS_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

class Aq40ShouldUseResistanceBuffsTrigger : public Trigger
{
public:
    Aq40ShouldUseResistanceBuffsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 should use resistance buffs") {}
    bool IsActive() override { return true; }
};

class Aq40HasEmperorAggroTrigger : public Trigger
{
public:
    Aq40HasEmperorAggroTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 has emperor aggro") {}
    bool IsActive() override;
};

class Aq40WarlockTankEmperorTrigger : public Trigger
{
public:
    Aq40WarlockTankEmperorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 warlock tank emperor") {}
    bool IsActive() override;
};

class Aq40MageFrostboltViscidusTrigger : public Trigger
{
public:
    Aq40MageFrostboltViscidusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 mage frostbolt viscidus") {}
    bool IsActive() override;
};

class Aq40MeleeViscidusTrigger : public Trigger
{
public:
    Aq40MeleeViscidusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 melee viscidus") {}
    bool IsActive() override;
};

class Aq40EmperorTrigger : public Trigger
{
public:
    Aq40EmperorTrigger(PlayerbotAI* botAI, std::string const name) : Trigger(botAI, name) {}
    bool IsActive() override;
protected:
    virtual bool IsVekLor() { return false; }
    virtual bool IsVekNilash() { return false; }
    virtual bool IsForHealers() { return false; }
    virtual bool IsPestControl() { return false; }
};

class Aq40TargetEmperorVekLorTrigger : public Aq40EmperorTrigger
{
public:
    Aq40TargetEmperorVekLorTrigger(PlayerbotAI* botAI) : Aq40EmperorTrigger(botAI, "aq40 target emperor vek'lor") {}
protected:
    bool IsVekLor() override { return true; }
    bool IsVekNilash() override { return false; }
    bool IsForHealers() override { return false; }
    bool IsPestControl() override { return false; }
};

class Aq40TargetEmperorVekNilashTrigger : public Aq40EmperorTrigger
{
public:
    Aq40TargetEmperorVekNilashTrigger(PlayerbotAI* botAI) : Aq40EmperorTrigger(botAI, "aq40 target emperor vek'nilash") {}
protected:
    bool IsVekLor() override { return false; }
    bool IsVekNilash() override { return true; }
    bool IsForHealers() override { return false; }
    bool IsPestControl() override { return false; }
};

class Aq40TargetEmperorPestsTrigger : public Aq40EmperorTrigger
{
public:
    Aq40TargetEmperorPestsTrigger(PlayerbotAI* botAI) : Aq40EmperorTrigger(botAI, "aq40 target emperor pests") {}
protected:
    bool IsVekLor() override { return false; }
    bool IsVekNilash() override { return false; }
    bool IsForHealers() override { return false; }
    bool IsPestControl() override { return true; }
};

class Aq40ApproachEmperorVekLorTrigger : public Aq40EmperorTrigger
{
public:
    Aq40ApproachEmperorVekLorTrigger(PlayerbotAI* botAI) : Aq40EmperorTrigger(botAI, "aq40 approach emperor vek'lor") {}
protected:
    bool IsVekLor() override { return true; }
    bool IsVekNilash() override { return false; }
    bool IsForHealers() override { return true; }
    bool IsPestControl() override { return false; }
};

class Aq40ApproachEmperorVekNilashTrigger : public Aq40EmperorTrigger
{
public:
    Aq40ApproachEmperorVekNilashTrigger(PlayerbotAI* botAI) : Aq40EmperorTrigger(botAI, "aq40 approach emperor vek'nilash") {}
protected:
    bool IsVekLor() override { return false; }
    bool IsVekNilash() override { return true; }
    bool IsForHealers() override { return true; }
    bool IsPestControl() override { return false; }
};

class Aq40OuroBurrowedTrigger : public Trigger
{
public:
    Aq40OuroBurrowedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 ouro burrowed") {}
    bool IsActive() override;
};

class Aq40Cthun1StartedTrigger : public Trigger
{
public:
    Aq40Cthun1StartedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun1 started") {}
    bool IsActive() override;
private:
    bool wasactive = false;
};

class Aq40Cthun2StartedTrigger : public Trigger
{
public:
    Aq40Cthun2StartedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun2 started") {}
    bool IsActive() override;
private:
    bool wasactive = false;
};

#endif
