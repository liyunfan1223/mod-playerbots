/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CURETRIGGERS_H
#define PLAYERBOTS_CURETRIGGERS_H

#include "GenericTriggers.h"

class PlayerbotAI;
class Unit;

class NeedCureTrigger : public SpellTrigger
{
public:
    NeedCureTrigger(PlayerbotAI* botAI, std::string const spell, uint32 dispelType)
        : SpellTrigger(botAI, spell, 1 * IN_MILLISECONDS), dispelType(dispelType)
    {
    }

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;

protected:
    uint32 dispelType;
};

class TargetAuraDispelTrigger : public NeedCureTrigger
{
public:
    TargetAuraDispelTrigger(PlayerbotAI* botAI, std::string const spell, uint32 dispelType)
        : NeedCureTrigger(botAI, spell, dispelType)
    {
    }

    std::string const GetTargetName() override { return "current target"; }
};

class PartyMemberNeedCureTrigger : public NeedCureTrigger
{
public:
    PartyMemberNeedCureTrigger(PlayerbotAI* botAI, std::string const spell, uint32 dispelType)
        : NeedCureTrigger(botAI, spell, dispelType)
    {
    }

    Value<Unit*>* GetTargetValue() override;
    bool IsActive() override;
};

class NeedWorldBuffTrigger : public Trigger
{
public:
    NeedWorldBuffTrigger(PlayerbotAI* botAI) : Trigger(botAI) {}

    bool IsActive() override;
};

#endif
