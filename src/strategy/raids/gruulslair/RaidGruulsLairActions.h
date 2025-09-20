#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class HighKingMaulgarRemoveTankAssistAction : public Action
{
public:
    HighKingMaulgarRemoveTankAssistAction(PlayerbotAI* botAI, std::string const name = "high king maulgar remove tank assist") : Action(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMaulgarTankAction : public AttackAction
{
public:
    HighKingMaulgarMaulgarTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar maulgar tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarOlmTankAction : public AttackAction
{
public:
    HighKingMaulgarOlmTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar olm tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarBlindeyeTankAction : public AttackAction
{
public:
    HighKingMaulgarBlindeyeTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar blindeye tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMageTankAction : public AttackAction
{
public:
    HighKingMaulgarMageTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar mage tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMoonkinTankAction : public AttackAction
{
public:
    HighKingMaulgarMoonkinTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar moonkin tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMeleeDPSAction : public AttackAction
{
public:
    HighKingMaulgarMeleeDPSAction(PlayerbotAI* botAI, std::string const name = "high king maulgar melee dps") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarRangedDPSAction : public AttackAction
{
public:
    HighKingMaulgarRangedDPSAction(PlayerbotAI* botAI, std::string const name = "high king maulgar ranged dps") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarHealerAvoidanceAction : public MovementAction
{
public:
    HighKingMaulgarHealerAvoidanceAction(PlayerbotAI* botAI, std::string const name = "high king maulgar healer avoidance") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarBanishFelstalkerAction : public AttackAction
{
public:
    HighKingMaulgarBanishFelstalkerAction(PlayerbotAI* botAI, std::string const name = "high king maulgar banish felstalker") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

class HighKingMaulgarHunterMisdirectionAction : public AttackAction
{
public:
    HighKingMaulgarHunterMisdirectionAction(PlayerbotAI* botAI, std::string const name = "high king maulgar hunter misdirection") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class GruulTheDragonkillerPositionBossAction : public MovementAction
{
public:
    GruulTheDragonkillerPositionBossAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller position boss") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

class GruulTheDragonkillerSpreadRangedAction : public MovementAction
{
public:
    GruulTheDragonkillerSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

class GruulTheDragonkillerShatterSpreadAction : public MovementAction
{
public:
    GruulTheDragonkillerShatterSpreadAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller shatter spread") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful();
};

#endif
