/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAINTANCEVALUE_H
#define _PLAYERBOT_MAINTANCEVALUE_H

#include "Value.h"

class PlayerbotAI;

class CanMoveAroundValue : public BoolCalculatedValue
{
public:
    CanMoveAroundValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can move around", 2 * 2000) {}

    bool Calculate() override;
};

class ShouldHomeBindValue : public BoolCalculatedValue
{
public:
    ShouldHomeBindValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should home bind", 2 * 2000) {}

    bool Calculate() override;
};

class ShouldRepairValue : public BoolCalculatedValue
{
public:
    ShouldRepairValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should repair", 2 * 2000) {}

    bool Calculate() override;
};

class CanRepairValue : public BoolCalculatedValue
{
public:
    CanRepairValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can repair", 2 * 2000) {}

    bool Calculate() override;
};

class ShouldSellValue : public BoolCalculatedValue
{
public:
    ShouldSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should sell", 2 * 2000) {}

    bool Calculate() override;
};

class CanSellValue : public BoolCalculatedValue
{
public:
    CanSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can sell", 2 * 2000) {}

    bool Calculate() override;
};

class CanFightEqualValue : public BoolCalculatedValue
{
public:
    CanFightEqualValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight equal", 2 * 2000) {}

    bool Calculate() override;
};

class CanFightEliteValue : public BoolCalculatedValue
{
public:
    CanFightEliteValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight elite") {}

    bool Calculate() override;
};

class CanFightBossValue : public BoolCalculatedValue
{
public:
    CanFightBossValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight boss") {}

    bool Calculate() override;
};

#endif
