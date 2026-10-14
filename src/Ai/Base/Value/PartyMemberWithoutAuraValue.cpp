/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PartyMemberWithoutAuraValue.h"
#include "GenericBuffUtils.h"
#include "Playerbots.h"

extern std::vector<std::string> split(std::string const s, char delim);

class PlayerWithoutAuraPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PlayerWithoutAuraPredicate(PlayerbotAI* botAI, std::string const aura)
        : FindPlayerPredicate(), PlayerbotAIAware(botAI), auras(split(aura, ','))
    {
    }

public:
    bool Check(Unit* unit) override
    {
        if (!unit->IsAlive())
            return false;

        for (std::vector<std::string>::iterator i = auras.begin(); i != auras.end(); ++i)
        {
            if (!ai::buff::BuffBelowRefreshTarget(botAI, botAI->GetAura(*i, unit), 0))
                return false;
        }

        return true;
    }

private:
    std::vector<std::string> auras;
};

Unit* PartyMemberWithoutAuraValue::Calculate()
{
    PlayerWithoutAuraPredicate predicate(botAI, qualifier);
    return FindPartyMember(predicate);
}
