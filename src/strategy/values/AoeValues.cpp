/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AoeValues.h"

#include "Playerbots.h"
#include "ServerFacade.h"
#include "SpellAuraEffects.h"

GuidVector FindMaxDensity(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    GuidVector units = *botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets");

    std::map<ObjectGuid, GuidVector> groups;
    uint32 maxCount = 0;
    ObjectGuid maxGroup;
    for (GuidVector::iterator i = units.begin(); i != units.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        for (GuidVector::iterator j = units.begin(); j != units.end(); ++j)
        {
            Unit* other = botAI->GetUnit(*j);
            if (!other)
                continue;

            float d = sServerFacade->GetDistance2d(unit, other);
            if (sServerFacade->IsDistanceLessOrEqualThan(d, sPlayerbotAIConfig->aoeRadius * 2))
                groups[*i].push_back(*j);
        }

        if (maxCount < groups[*i].size())
        {
            maxCount = groups[*i].size();
            maxGroup = *i;
        }
    }

    if (!maxCount)
        return GuidVector();

    return groups[maxGroup];
}

WorldLocation AoePositionValue::Calculate()
{
    GuidVector group = FindMaxDensity(bot);
    if (group.empty())
        return WorldLocation();

    // Note: don't know where these values come from or even used.
    float x1 = 0.f;
    float y1 = 0.f;
    float x2 = 0.f;
    float y2 = 0.f;
    for (GuidVector::iterator i = group.begin(); i != group.end(); ++i)
    {
        Unit* unit = GET_PLAYERBOT_AI(bot)->GetUnit(*i);
        if (!unit)
            continue;

        if (i == group.begin() || x1 > unit->GetPositionX())
            x1 = unit->GetPositionX();

        if (i == group.begin() || x2 < unit->GetPositionX())
            x2 = unit->GetPositionX();

        if (i == group.begin() || y1 > unit->GetPositionY())
            y1 = unit->GetPositionY();

        if (i == group.begin() || y2 < unit->GetPositionY())
            y2 = unit->GetPositionY();
    }

    float x = (x1 + x2) / 2;
    float y = (y1 + y2) / 2;
    float z = bot->GetPositionZ() + CONTACT_DISTANCE;
    ;
    bot->UpdateAllowedPositionZ(x, y, z);
    return WorldLocation(bot->GetMapId(), x, y, z, 0);
}

uint8 AoeCountValue::Calculate() { return FindMaxDensity(bot).size(); }

bool HasAreaDebuffValue::Calculate()
{
    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraEffectList const& auras = botAI->GetBot()->GetAuraEffectsByType((AuraType)auraType);

        if (auras.empty())
            continue;

        for (AuraEffect const* aurEff : auras)
        {
            SpellInfo const* proto = aurEff->GetSpellInfo();

            if (!proto)
                continue;

            uint32 trigger_spell_id = proto->Effects[aurEff->GetEffIndex()].TriggerSpell;
            if (trigger_spell_id == 29767)  // Overload
            {
                return true;
            }
            else
            {
                return (!proto->IsPositive() && aurEff->IsPeriodic() && proto->HasAreaAuraEffect());
            }
        }
    }

    return false;
}

Aura* AreaDebuffValue::Calculate()
{
    // Unit::AuraApplicationMap& map = bot->GetAppliedAuras();
    Unit::AuraEffectList const& aurasPeriodicDamage = bot->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
    Unit::AuraEffectList const& aurasPeriodicDamagePercent =
        bot->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
    Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
        bot->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
        bot->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
    Unit::AuraEffectList const& aurasDummy = bot->GetAuraEffectsByType(SPELL_AURA_DUMMY);
    for (const Unit::AuraEffectList& list : {aurasPeriodicDamage, aurasPeriodicDamagePercent, aurasPeriodicTriggerSpell,
                                             aurasPeriodicTriggerWithValueSpell, aurasDummy})
    {
        for (auto i = list.begin(); i != list.end(); ++i)
        {
            AuraEffect* aurEff = *i;
            Aura* aura = aurEff->GetBase();
            AuraObjectType type = aura->GetType();
            bool isPositive = aura->GetSpellInfo()->IsPositive();
            if (type == DYNOBJ_AURA_TYPE && !isPositive)
            {
                DynamicObject* dynOwner = aura->GetDynobjOwner();
                if (!dynOwner)
                {
                    continue;
                }
                // float radius = dynOwner->GetRadius();
                // if (radius > 12.0f)
                //     continue;
                return aura;
            }
        }
    }
    return nullptr;
}