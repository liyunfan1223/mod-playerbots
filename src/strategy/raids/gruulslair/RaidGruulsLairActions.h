#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class HighKingMaulgarMaulgarTankAction : public AttackAction
{
public:
    HighKingMaulgarMaulgarTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar maulgar tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarOlmTankAction : public AttackAction
{
public:
    HighKingMaulgarOlmTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar olm tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarBlindeyeTankAction : public AttackAction
{
public:
    HighKingMaulgarBlindeyeTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar blindeye tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarKroshMageTankAction : public AttackAction
{
public:
    HighKingMaulgarKroshMageTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar krosh mage tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarKigglerMoonkinTankAction : public AttackAction
{
public:
    HighKingMaulgarKigglerMoonkinTankAction(PlayerbotAI* botAI, std::string const name = "high king maulgar kiggler moonkin tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarMeleeDPSAction : public AttackAction
{
public:
    HighKingMaulgarMeleeDPSAction(PlayerbotAI* botAI, std::string const name = "high king maulgar melee dps") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarRangedDPSAction : public AttackAction
{
public:
    HighKingMaulgarRangedDPSAction(PlayerbotAI* botAI, std::string const name = "high king maulgar ranged dps") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarHealerAvoidanceAction : public MovementAction
{
public:
    HighKingMaulgarHealerAvoidanceAction(PlayerbotAI* botAI, std::string const name = "high king maulgar healer avoidance") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarWhirlwindRunAwayAction : public MovementAction
{
public:
    HighKingMaulgarWhirlwindRunAwayAction(PlayerbotAI* botAI, std::string const name = "high king maulgar whirlwind run away") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarBanishFelstalkerAction : public AttackAction
{
public:
    HighKingMaulgarBanishFelstalkerAction(PlayerbotAI* botAI, std::string const name = "high king maulgar banish felstalker") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class HighKingMaulgarHunterMisdirectionAction : public AttackAction
{
public:
    HighKingMaulgarHunterMisdirectionAction(PlayerbotAI* botAI, std::string const name = "high king maulgar hunter misdirection") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulTheDragonkillerMainTankAction : public AttackAction
{
public:
    GruulTheDragonkillerMainTankAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller main tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulTheDragonkillerSpreadRangedAction : public MovementAction
{
public:
    GruulTheDragonkillerSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class GruulTheDragonkillerShatterSpreadAction : public MovementAction
{
public:
    GruulTheDragonkillerShatterSpreadAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller shatter spread") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
