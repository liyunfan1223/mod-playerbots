/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MGTVALUECONTEXT_H
#define PLAYERBOTS_MGTVALUECONTEXT_H

#include "MgTShared.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Unit.h"
#include "Value.h"

class MgTEscapeSpotsValue : public CalculatedValue<MagistersTerrace::EscapeSpots>
{
public:
    MgTEscapeSpotsValue(PlayerbotAI* botAI, std::string const name)
        : CalculatedValue<MagistersTerrace::EscapeSpots>(botAI, name, 200)
    {
    }
};

class MgTDampeningEscapeValue : public MgTEscapeSpotsValue
{
public:
    MgTDampeningEscapeValue(PlayerbotAI* botAI) : MgTEscapeSpotsValue(botAI, "mgt dampening escape") {}

protected:
    MagistersTerrace::EscapeSpots Calculate() override
    {
        MagistersTerrace::EscapeSpots spots;
        MagistersTerrace::CollectDampeningEscapes(bot, spots);
        return spots;
    }
};

class MgTNovaEscapeValue : public MgTEscapeSpotsValue
{
public:
    MgTNovaEscapeValue(PlayerbotAI* botAI) : MgTEscapeSpotsValue(botAI, "mgt nova escape") {}

protected:
    MagistersTerrace::EscapeSpots Calculate() override
    {
        MagistersTerrace::EscapeSpots spots;
        MagistersTerrace::CollectNovaEscapes(bot, spots);
        return spots;
    }
};

class MgTFlameStrikeEscapeValue : public MgTEscapeSpotsValue
{
public:
    MgTFlameStrikeEscapeValue(PlayerbotAI* botAI) : MgTEscapeSpotsValue(botAI, "mgt flame strike escape") {}

protected:
    MagistersTerrace::EscapeSpots Calculate() override
    {
        MagistersTerrace::EscapeSpots spots;
        MagistersTerrace::CollectFlameStrikeEscapes(bot, spots);
        return spots;
    }
};

class MgTPhoenixEscapeValue : public MgTEscapeSpotsValue
{
public:
    MgTPhoenixEscapeValue(PlayerbotAI* botAI) : MgTEscapeSpotsValue(botAI, "mgt phoenix escape") {}

protected:
    MagistersTerrace::EscapeSpots Calculate() override
    {
        MagistersTerrace::EscapeSpots spots;
        MagistersTerrace::CollectPhoenixEscapes(bot, spots);
        return spots;
    }
};

class MgTPhoenixRingValue : public CalculatedValue<MagistersTerrace::PhoenixRing>
{
public:
    MgTPhoenixRingValue(PlayerbotAI* botAI)
        : CalculatedValue<MagistersTerrace::PhoenixRing>(botAI, "mgt phoenix ring", 200)
    {
    }

protected:
    MagistersTerrace::PhoenixRing Calculate() override { return MagistersTerrace::GetPhoenixRing(bot); }
};

class MgTCrystalTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTCrystalTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mgt crystal target", 200) {}

protected:
    ObjectGuid Calculate() override
    {
        Unit* crystal = MagistersTerrace::GetActiveFelCrystal(bot);
        return crystal ? crystal->GetGUID() : ObjectGuid::Empty;
    }
};

class MgTMageGuardTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTMageGuardTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mgt mage guard target", 200) {}

protected:
    ObjectGuid Calculate() override
    {
        if (!PlayerbotAI::IsTank(bot))
            return ObjectGuid::Empty;

        Unit* guard = MagistersTerrace::GetGlaiveThrowingMageGuard(bot);
        return guard ? guard->GetGUID() : ObjectGuid::Empty;
    }
};

class MgTEnragedWretchedValue : public ObjectGuidCalculatedValue
{
public:
    MgTEnragedWretchedValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mgt enraged wretched", 200) {}

protected:
    ObjectGuid Calculate() override
    {
        if (!PlayerbotAI::IsTank(bot))
            return ObjectGuid::Empty;

        Unit* wretched = MagistersTerrace::GetEnragedWretched(bot);
        return wretched ? wretched->GetGUID() : ObjectGuid::Empty;
    }
};

class MgTInterruptTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTInterruptTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mgt interrupt target", 200) {}

protected:
    ObjectGuid Calculate() override
    {
        Unit* target = MagistersTerrace::GetInterruptTarget(bot);
        return target ? target->GetGUID() : ObjectGuid::Empty;
    }
};

class MgTSunbladeFocusTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTSunbladeFocusTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mgt focus target", 200) {}

protected:
    ObjectGuid Calculate() override
    {
        Unit* target = MagistersTerrace::GetFocusTarget(bot, _latched);
        return target ? target->GetGUID() : ObjectGuid::Empty;
    }

private:
    ObjectGuid _latched;
};

class MgTDelrissaInterruptOrderValue : public ObjectGuidListCalculatedValue
{
public:
    MgTDelrissaInterruptOrderValue(PlayerbotAI* botAI)
        : ObjectGuidListCalculatedValue(botAI, "mgt delrissa interrupt order", 200)
    {
    }

protected:
    GuidVector Calculate() override
    {
        std::vector<Unit*> preference;
        MagistersTerrace::CollectDelrissaInterruptPreference(bot, preference);

        GuidVector order;
        order.reserve(preference.size());
        for (Unit* caster : preference)
            order.push_back(caster->GetGUID());

        return order;
    }
};

class MgTDelrissaFocusTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTDelrissaFocusTargetValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(botAI, "mgt delrissa focus target", 200)
    {
    }

protected:
    ObjectGuid Calculate() override
    {
        Unit* target = MagistersTerrace::GetDelrissaFocusTarget(bot, _latched);
        return target ? target->GetGUID() : ObjectGuid::Empty;
    }

private:
    ObjectGuid _latched;
};

class MgTDelrissaTremorTotemValue : public BoolCalculatedValue
{
public:
    MgTDelrissaTremorTotemValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "mgt delrissa tremor totem", 500) {}

protected:
    bool Calculate() override { return MagistersTerrace::ShouldHoldTremorTotem(bot); }
};

class MgTDelrissaPetsValue : public ObjectGuidListCalculatedValue
{
public:
    MgTDelrissaPetsValue(PlayerbotAI* botAI) : ObjectGuidListCalculatedValue(botAI, "mgt delrissa pets", 500) {}

protected:
    GuidVector Calculate() override
    {
        GuidVector pets;
        MagistersTerrace::CollectDelrissaPets(bot, pets);
        return pets;
    }
};

class MgTKaelFocusTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTKaelFocusTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "mgt kael focus target", 200) {}

protected:
    ObjectGuid Calculate() override
    {
        Unit* target = MagistersTerrace::GetKaelFocusTarget(bot);
        return target ? target->GetGUID() : ObjectGuid::Empty;
    }
};

class MgTGravityLapseValue : public BoolCalculatedValue
{
public:
    MgTGravityLapseValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "mgt gravity lapse", 200) {}

protected:
    bool Calculate() override { return MagistersTerrace::IsGravityLapseActive(bot); }
};

class MgTKaelUnattackableValue : public BoolCalculatedValue
{
public:
    MgTKaelUnattackableValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "mgt kael unattackable", 500) {}

protected:
    bool Calculate() override { return MagistersTerrace::IsKaelUnattackable(bot); }
};

class MgTKaelInterruptTargetValue : public ObjectGuidCalculatedValue
{
public:
    MgTKaelInterruptTargetValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(botAI, "mgt kael interrupt target", 200)
    {
    }

protected:
    ObjectGuid Calculate() override
    {
        Unit* target = MagistersTerrace::GetKaelInterruptTarget(bot);
        return target ? target->GetGUID() : ObjectGuid::Empty;
    }
};

class MgTMeleePhoenixExclusionsValue : public ObjectGuidListCalculatedValue
{
public:
    MgTMeleePhoenixExclusionsValue(PlayerbotAI* botAI)
        : ObjectGuidListCalculatedValue(botAI, "mgt melee phoenix exclusions", 500)
    {
    }

protected:
    GuidVector Calculate() override
    {
        GuidVector phoenixes;
        MagistersTerrace::CollectMeleePhoenixExclusions(bot, phoenixes);
        return phoenixes;
    }
};

class MgTFocusExclusionsValue : public ObjectGuidListCalculatedValue
{
public:
    MgTFocusExclusionsValue(PlayerbotAI* botAI) : ObjectGuidListCalculatedValue(botAI, "mgt focus exclusions", 200) {}

protected:
    GuidVector Calculate() override
    {
        GuidVector exclusions;
        MagistersTerrace::CollectFocusExclusions(bot, exclusions);
        return exclusions;
    }
};

class TbcDungeonMgTValueContext : public NamedObjectContext<UntypedValue>
{
public:
    TbcDungeonMgTValueContext()
    {
        creators["mgt crystal target"] = &TbcDungeonMgTValueContext::mgt_crystal_target;
        creators["mgt dampening escape"] = &TbcDungeonMgTValueContext::mgt_dampening_escape;
        creators["mgt nova escape"] = &TbcDungeonMgTValueContext::mgt_nova_escape;
        creators["mgt mage guard target"] = &TbcDungeonMgTValueContext::mgt_mage_guard_target;
        creators["mgt enraged wretched"] = &TbcDungeonMgTValueContext::mgt_enraged_wretched;
        creators["mgt interrupt target"] = &TbcDungeonMgTValueContext::mgt_interrupt_target;
        creators["mgt focus target"] = &TbcDungeonMgTValueContext::mgt_focus_target;
        creators["mgt delrissa interrupt order"] = &TbcDungeonMgTValueContext::mgt_delrissa_interrupt_order;
        creators["mgt delrissa focus target"] = &TbcDungeonMgTValueContext::mgt_delrissa_focus_target;
        creators["mgt delrissa tremor totem"] = &TbcDungeonMgTValueContext::mgt_delrissa_tremor_totem;
        creators["mgt delrissa pets"] = &TbcDungeonMgTValueContext::mgt_delrissa_pets;
        creators["mgt flame strike escape"] = &TbcDungeonMgTValueContext::mgt_flame_strike_escape;
        creators["mgt phoenix escape"] = &TbcDungeonMgTValueContext::mgt_phoenix_escape;
        creators["mgt phoenix ring"] = &TbcDungeonMgTValueContext::mgt_phoenix_ring;
        creators["mgt kael focus target"] = &TbcDungeonMgTValueContext::mgt_kael_focus_target;
        creators["mgt kael interrupt target"] = &TbcDungeonMgTValueContext::mgt_kael_interrupt_target;
        creators["mgt kael unattackable"] = &TbcDungeonMgTValueContext::mgt_kael_unattackable;
        creators["mgt gravity lapse"] = &TbcDungeonMgTValueContext::mgt_gravity_lapse;
        creators["mgt melee phoenix exclusions"] = &TbcDungeonMgTValueContext::mgt_melee_phoenix_exclusions;
        creators["mgt focus exclusions"] = &TbcDungeonMgTValueContext::mgt_focus_exclusions;
    }

private:
    static UntypedValue* mgt_crystal_target(PlayerbotAI* botAI) { return new MgTCrystalTargetValue(botAI); }
    static UntypedValue* mgt_dampening_escape(PlayerbotAI* botAI) { return new MgTDampeningEscapeValue(botAI); }
    static UntypedValue* mgt_nova_escape(PlayerbotAI* botAI) { return new MgTNovaEscapeValue(botAI); }
    static UntypedValue* mgt_mage_guard_target(PlayerbotAI* botAI) { return new MgTMageGuardTargetValue(botAI); }
    static UntypedValue* mgt_enraged_wretched(PlayerbotAI* botAI) { return new MgTEnragedWretchedValue(botAI); }
    static UntypedValue* mgt_interrupt_target(PlayerbotAI* botAI) { return new MgTInterruptTargetValue(botAI); }
    static UntypedValue* mgt_focus_target(PlayerbotAI* botAI) { return new MgTSunbladeFocusTargetValue(botAI); }
    static UntypedValue* mgt_delrissa_interrupt_order(PlayerbotAI* botAI)
    {
        return new MgTDelrissaInterruptOrderValue(botAI);
    }
    static UntypedValue* mgt_delrissa_focus_target(PlayerbotAI* botAI)
    {
        return new MgTDelrissaFocusTargetValue(botAI);
    }
    static UntypedValue* mgt_delrissa_tremor_totem(PlayerbotAI* botAI)
    {
        return new MgTDelrissaTremorTotemValue(botAI);
    }
    static UntypedValue* mgt_delrissa_pets(PlayerbotAI* botAI) { return new MgTDelrissaPetsValue(botAI); }
    static UntypedValue* mgt_flame_strike_escape(PlayerbotAI* botAI) { return new MgTFlameStrikeEscapeValue(botAI); }
    static UntypedValue* mgt_phoenix_escape(PlayerbotAI* botAI) { return new MgTPhoenixEscapeValue(botAI); }
    static UntypedValue* mgt_phoenix_ring(PlayerbotAI* botAI) { return new MgTPhoenixRingValue(botAI); }
    static UntypedValue* mgt_kael_focus_target(PlayerbotAI* botAI) { return new MgTKaelFocusTargetValue(botAI); }
    static UntypedValue* mgt_kael_interrupt_target(PlayerbotAI* botAI)
    {
        return new MgTKaelInterruptTargetValue(botAI);
    }
    static UntypedValue* mgt_kael_unattackable(PlayerbotAI* botAI) { return new MgTKaelUnattackableValue(botAI); }
    static UntypedValue* mgt_gravity_lapse(PlayerbotAI* botAI) { return new MgTGravityLapseValue(botAI); }
    static UntypedValue* mgt_melee_phoenix_exclusions(PlayerbotAI* botAI)
    {
        return new MgTMeleePhoenixExclusionsValue(botAI);
    }
    static UntypedValue* mgt_focus_exclusions(PlayerbotAI* botAI) { return new MgTFocusExclusionsValue(botAI); }
};

#endif
