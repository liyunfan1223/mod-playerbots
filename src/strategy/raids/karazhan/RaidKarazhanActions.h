#ifndef _PLAYERBOT_RAIDKARAZHANACTIONS_H
#define _PLAYERBOT_RAIDKARAZHANACTIONS_H

#include "Action.h"
#include "MovementActions.h"

class KarazhanAttumenTheHuntsmanStackBehindAction : public MovementAction
{
public:
    KarazhanAttumenTheHuntsmanStackBehindAction(PlayerbotAI* botAI, std::string const name = "karazhan attumen the huntsman stack behind") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanMoroesMarkTargetAction : public Action
{
public:
    KarazhanMoroesMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan moroes mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanMaidenOfVirtuePositionBossAction : public MovementAction
{
public:
    KarazhanMaidenOfVirtuePositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan maiden of virtue position boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanMaidenOfVirtuePositionRangedAction : public MovementAction
{
public:
    KarazhanMaidenOfVirtuePositionRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan maiden of virtue position ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanBigBadWolfPositionBossAction : public MovementAction
{
public:
    KarazhanBigBadWolfPositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan big bad wolf position boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanBigBadWolfRunAwayAction : public MovementAction
{
public:
    KarazhanBigBadWolfRunAwayAction(PlayerbotAI* botAI, std::string const name = "karazhan big bad wolf run away") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    size_t currentIndex = 0;
};

class KarazhanRomuloAndJulianneMarkTargetAction : public Action
{
public:
    KarazhanRomuloAndJulianneMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan romulo and julianne mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanWizardOfOzMarkTargetAction : public Action
{
public:
    KarazhanWizardOfOzMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan wizard of oz mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanWizardOfOzScorchStrawmanAction : public Action
{
public:
    KarazhanWizardOfOzScorchStrawmanAction(PlayerbotAI* botAI, std::string const name = "karazhan wizard of oz scorch strawman") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanTheCuratorMarkTargetAction : public Action
{
public:
    KarazhanTheCuratorMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanTheCuratorPositionBossAction : public MovementAction
{
public:
    KarazhanTheCuratorPositionBossAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator position boss") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanTheCuratorSpreadRangedAction : public MovementAction
{
public:
    KarazhanTheCuratorSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan the curator spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanTerestianIllhoofMarkTargetAction : public Action
{
public:
    KarazhanTerestianIllhoofMarkTargetAction(PlayerbotAI* botAI, std::string const name = "karazhan terestian illhoof mark target") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanShadeOfAranArcaneExplosionRunAwayAction : public MovementAction
{
public:
    KarazhanShadeOfAranArcaneExplosionRunAwayAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran arcane explosion run away") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanShadeOfAranFlameWreathStopMovementAction : public MovementAction
{
public:
    KarazhanShadeOfAranFlameWreathStopMovementAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran flame wreath stop bot") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanShadeOfAranMarkConjuredElementalAction : public Action
{
public:
    KarazhanShadeOfAranMarkConjuredElementalAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran mark conjured elemental") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class KarazhanShadeOfAranSpreadRangedAction : public MovementAction
{
public:
    KarazhanShadeOfAranSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "karazhan shade of aran spread ranged") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBlockRedBeamAction : public MovementAction
{
public:
    KarazhanNetherspiteBlockRedBeamAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite block red beam") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBlockBlueBeamAction : public MovementAction
{
public:
    KarazhanNetherspiteBlockBlueBeamAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite block blue beam") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBlockGreenBeamAction : public MovementAction
{
public:
    KarazhanNetherspiteBlockGreenBeamAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite block green beam") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteAvoidBeamAndVoidZoneAction : public MovementAction
{
public:
    KarazhanNetherspiteAvoidBeamAndVoidZoneAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite avoid beam and void zone") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction : public MovementAction
{
public:
    KarazhanNetherspiteBanishPhaseAvoidVoidZoneAction(PlayerbotAI* botAI, std::string const name = "karazhan netherspite banish phase avoid void zone") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanPrinceMalchezaarNonTankAvoidHazardAction : public MovementAction
{
public:
    KarazhanPrinceMalchezaarNonTankAvoidHazardAction(PlayerbotAI* botAI, std::string const name = "karazhan prince malchezaar non-tank avoid hazard") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class KarazhanPrinceMalchezaarTankAvoidHazardAction : public MovementAction
{
public:
    KarazhanPrinceMalchezaarTankAvoidHazardAction(PlayerbotAI* botAI, std::string const name = "karazhan prince malchezaar tank avoid hazard") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
