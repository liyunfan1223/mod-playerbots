/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGTRIGGERS_H
#define _PLAYERBOT_RPGTRIGGERS_H

#include "Trigger.h"

class Event;
class GuidPosition;
class PlayerbotAI;

struct CreatureTemplate;

class NoRpgTargetTrigger : public Trigger
{
public:
    NoRpgTargetTrigger(PlayerbotAI* botAI, std::string const name = "no rpg target", int checkInterval = 1)
        : Trigger(botAI, name, checkInterval)
    {
    }

    bool IsActive() override;
};

class HasRpgTargetTrigger : public NoRpgTargetTrigger
{
public:
    HasRpgTargetTrigger(PlayerbotAI* botAI, std::string const name = "has rpg target", int checkInterval = 1)
        : NoRpgTargetTrigger(botAI, name, checkInterval)
    {
    }

    bool IsActive() override;
};

class FarFromRpgTargetTrigger : public NoRpgTargetTrigger
{
public:
    FarFromRpgTargetTrigger(PlayerbotAI* botAI, std::string const name = "far from rpg target", int checkInterval = 1)
        : NoRpgTargetTrigger(botAI, name, checkInterval)
    {
    }

    bool IsActive() override;
};

class NearRpgTargetTrigger : public FarFromRpgTargetTrigger
{
public:
    NearRpgTargetTrigger(PlayerbotAI* botAI, std::string const name = "near rpg target", int checkInterval = 1)
        : FarFromRpgTargetTrigger(botAI, name, checkInterval)
    {
    }

    bool IsActive() override;
};

// Sub actions:
class RpgTrigger : public FarFromRpgTargetTrigger
{
public:
    RpgTrigger(PlayerbotAI* botAI, std::string const name = "sub rpg", int checkInterval = 2)
        : FarFromRpgTargetTrigger(botAI, name, checkInterval)
    {
    }

    GuidPosition getGuidP();

    bool IsActive() override;
    Event Check() override;
};

class RpgTaxiTrigger : public RpgTrigger
{
public:
    RpgTaxiTrigger(PlayerbotAI* botAI, std::string const name = "rpg taxi") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgDiscoverTrigger : public RpgTrigger
{
public:
    RpgDiscoverTrigger(PlayerbotAI* botAI, std::string const name = "rpg discover") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgStartQuestTrigger : public RpgTrigger
{
public:
    RpgStartQuestTrigger(PlayerbotAI* botAI, std::string const name = "rpg start quest") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgEndQuestTrigger : public RpgTrigger
{
public:
    RpgEndQuestTrigger(PlayerbotAI* botAI, std::string const name = "rpg end quest") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgBuyTrigger : public RpgTrigger
{
public:
    RpgBuyTrigger(PlayerbotAI* botAI, std::string const name = "rpg buy") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgSellTrigger : public RpgTrigger
{
public:
    RpgSellTrigger(PlayerbotAI* botAI, std::string const name = "rpg sell") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgRepairTrigger : public RpgTrigger
{
public:
    RpgRepairTrigger(PlayerbotAI* botAI, std::string const name = "rpg repair") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgTrainTrigger : public RpgTrigger
{
public:
    RpgTrainTrigger(PlayerbotAI* botAI, std::string const name = "rpg train") : RpgTrigger(botAI, name) {}

    static bool IsTrainerOf(CreatureTemplate const* cInfo, Player* pPlayer);

    bool IsActive() override;
};

class RpgHealTrigger : public RpgTrigger
{
public:
    RpgHealTrigger(PlayerbotAI* botAI, std::string const name = "rpg heal") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgHomeBindTrigger : public RpgTrigger
{
public:
    RpgHomeBindTrigger(PlayerbotAI* botAI, std::string const name = "rpg home bind") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgQueueBGTrigger : public RpgTrigger
{
public:
    RpgQueueBGTrigger(PlayerbotAI* botAI, std::string const name = "rpg queue bg") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgBuyPetitionTrigger : public RpgTrigger
{
public:
    RpgBuyPetitionTrigger(PlayerbotAI* botAI, std::string const name = "rpg buy petition") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgUseTrigger : public RpgTrigger
{
public:
    RpgUseTrigger(PlayerbotAI* botAI, std::string const name = "rpg use") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgSpellTrigger : public RpgTrigger
{
public:
    RpgSpellTrigger(PlayerbotAI* botAI, std::string const name = "rpg spell") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgCraftTrigger : public RpgTrigger
{
public:
    RpgCraftTrigger(PlayerbotAI* botAI, std::string const name = "rpg craft") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgTradeUsefulTrigger : public RpgTrigger
{
public:
    RpgTradeUsefulTrigger(PlayerbotAI* botAI, std::string const name = "rpg trade useful") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

class RpgDuelTrigger : public RpgTrigger
{
public:
    RpgDuelTrigger(PlayerbotAI* botAI, std::string const name = "rpg duel") : RpgTrigger(botAI, name) {}

    bool IsActive() override;
};

#endif
