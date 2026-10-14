/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAINTENANCEVALUES_H
#define PLAYERBOTS_MAINTENANCEVALUES_H

#include "Value.h"

class PlayerbotAI;

class CanMoveAroundValue : public BoolCalculatedValue
{
public:
    CanMoveAroundValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can move around", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class ShouldHomeBindValue : public BoolCalculatedValue
{
public:
    ShouldHomeBindValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should home bind", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class ShouldRepairValue : public BoolCalculatedValue
{
public:
    ShouldRepairValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should repair", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class CanRepairValue : public BoolCalculatedValue
{
public:
    CanRepairValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can repair", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class ShouldSellValue : public BoolCalculatedValue
{
public:
    ShouldSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should sell", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class CanSellValue : public BoolCalculatedValue
{
public:
    CanSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can sell", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class CanTrainValue : public BoolCalculatedValue
{
public:
    CanTrainValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can train", 4 * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class CanFightEqualValue : public BoolCalculatedValue
{
public:
    CanFightEqualValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight equal", 4 * IN_MILLISECONDS) {}

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
