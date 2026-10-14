/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"
#include "TKHelpers.h"
#include <algorithm>
#include <list>

using namespace TkHelpers;

class VoidReaverArcaneOrbSpellListenerScript : public AllSpellScript
{
public:
    VoidReaverArcaneOrbSpellListenerScript()
        : AllSpellScript("VoidReaverArcaneOrbSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != Id(TkSpells::SPELL_ARCANE_ORB))
            return;

        std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
        if (targets.empty())
            return;

        Player* target = ObjectAccessor::GetPlayer(*caster, targets.front().targetGUID);
        if (!target)
            return;

        auto& orbs = voidReaverArcaneOrbs[caster->GetInstanceId()];
        uint32 const now = getMSTime();

        ArcaneOrbData orbData;
        orbData.destination = target->GetPosition();
        orbData.castTime = now;

        orbs.push_back(orbData);

        orbs.erase(std::remove_if(orbs.begin(), orbs.end(),
            [now](ArcaneOrbData const& orb) {
                return getMSTimeDiff(orb.castTime, now) > ARCANE_ORB_DURATION_MS;
            }), orbs.end());
    }
};

void AddSC_TempestKeepBotScripts()
{
    new VoidReaverArcaneOrbSpellListenerScript();
}
