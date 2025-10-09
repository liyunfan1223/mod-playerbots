#ifndef _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H
#define _PLAYERBOT_RAIDMAGTHERIDONACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class MagtheridonHellfireChannelerMainTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerMainTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler main tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerNWChannelerTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerNWChannelerTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler northwest channeler tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerNEChannelerTankAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerNEChannelerTankAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler northeast channeler tank") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerMisdirectionAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerMisdirectionAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler misdirection") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerDPSPriorityAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler dps priority") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonHellfireChannelerWarlockAction : public AttackAction
{
public:
    MagtheridonHellfireChannelerWarlockAction(PlayerbotAI* botAI, std::string const name = "magtheridon hellfire channeler warlock") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonPositionBossAction : public AttackAction
{
public:
    MagtheridonPositionBossAction(PlayerbotAI* botAI, std::string const name = "magtheridon position boss") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonSpreadRangedAction : public MovementAction
{
public:
    MagtheridonSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "magtheridon spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonSpreadHealerAction : public MovementAction
{
public:
    MagtheridonSpreadHealerAction(PlayerbotAI* botAI, std::string const name = "magtheridon spread healer") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

class MagtheridonUseManticronCubeAction : public MovementAction
{
public:
    MagtheridonUseManticronCubeAction(PlayerbotAI* botAI, std::string const name = "magtheridon use manticron cube") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
