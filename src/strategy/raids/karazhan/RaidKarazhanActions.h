#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_H

#include "AttackAction.h"
#include "RaidKarazhanHelpers.h"

class KarazhanAttumenTheHuntsmanStackBehindAction : public AttackAction
{
public:
    KarazhanAttumenTheHuntsmanStackBehindAction(PlayerbotAI* botAI, std::string const name = "karazhan attumen the huntsman stack behind") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanMoroesMarkTargetAction : public AttackAction
{
public:
    KarazhanMoroesMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan moroes mark target") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanMaidenOfVirtuePositionBossAction : public AttackAction
{
public:
    KarazhanMaidenOfVirtuePositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan maiden of virtue position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanMaidenOfVirtuePositionRangedAction : public AttackAction
{
public:
    KarazhanMaidenOfVirtuePositionRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan maiden of virtue position ranged") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanBigBadWolfRunAwayAction : public AttackAction
{
public:
    KarazhanBigBadWolfRunAwayAction(PlayerbotAI* botAI, std::string const name = "karazhan big bad wolf run away") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    size_t currentIndex = 0;
};

class KarazhanRomuloAndJulianneMarkTargetAction : public AttackAction
{
public:
    KarazhanRomuloAndJulianneMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan romulo and julianne mark target") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanWizardOfOzMarkTargetAction : public AttackAction
{
public:
    KarazhanWizardOfOzMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan wizard of oz mark target") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanWizardOfOzScorchStrawmanAction : public AttackAction
{
public:
    KarazhanWizardOfOzScorchStrawmanAction(PlayerbotAI* botAI, std::string const name = "karazhan wizard of oz scorch strawman") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanTheCuratorMarkTargetAction : public AttackAction
{
public:
    KarazhanTheCuratorMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator mark target") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanTheCuratorPositionBossAction : public AttackAction
{
public:
    KarazhanTheCuratorPositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator position boss") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanTheCuratorSpreadRangedAction : public AttackAction
{
public:
    KarazhanTheCuratorSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator spread ranged") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanTerestianIllhoofMarkTargetAction : public AttackAction
{
public:
    KarazhanTerestianIllhoofMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan terestian illhoof mark target") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanShadeOfAranArcaneExplosionRunAwayAction : public AttackAction
{
public:
    KarazhanShadeOfAranArcaneExplosionRunAwayAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran arcane explosion run away") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanShadeOfAranFlameWreathStopBotAction : public AttackAction
{
public:
    KarazhanShadeOfAranFlameWreathStopBotAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran flame wreath stop bot") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanShadeOfAranMarkConjuredElementalAction : public AttackAction
{
public:
    KarazhanShadeOfAranMarkConjuredElementalAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran mark conjured elemental") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanShadeOfAranSpreadRangedAction : public AttackAction
{
public:
    KarazhanShadeOfAranSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran spread ranged") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBlockRedBeamAction : public AttackAction
{
public:
    KarazhanNetherspiteBlockRedBeamAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite block red beam") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBlockBlueBeamAction : public AttackAction
{
public:
    KarazhanNetherspiteBlockBlueBeamAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite block blue beam") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBlockGreenBeamAction : public AttackAction
{
public:
    KarazhanNetherspiteBlockGreenBeamAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite block green beam") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteAvoidBeamAndVoidZoneAction : public AttackAction
{
public:
    KarazhanNetherspiteAvoidBeamAndVoidZoneAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite avoid beam and void zone") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction : public AttackAction
{
public:
    KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite banish phase avoid void zone") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanPrinceMalchezaarAvoidInfernalAction : public AttackAction
{
public:
    KarazhanPrinceMalchezaarAvoidInfernalAction(PlayerbotAI* botAI, std::string const name = "karazhan prince malchezaar avoid infernal") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanPrinceMalchezaarRunAwayFromShadowNovaAction : public AttackAction
{
public:
    KarazhanPrinceMalchezaarRunAwayFromShadowNovaAction(PlayerbotAI* botAI, std::string const name = "karazhan prince malchezaar run away from shadow nova") : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
