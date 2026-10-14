/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "DynamicObject.h"
#include "DynamicObjectScript.h"
#include "MagHelpers.h"
#include "Playerbots.h"

using namespace MagHelpers;

class MagtheridonSpellListenerScript : public AllSpellScript
{
public:
    MagtheridonSpellListenerScript() : AllSpellScript("MagtheridonSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        switch (spellInfo->Id)
        {
            case Id(MagSpells::SPELL_QUAKE):
                HandleQuake(caster);
                break;
            case Id(MagSpells::SPELL_BLAST_NOVA):
                HandleBlastNova(caster);
                break;
            default:
                break;
        }
    }

private:
    // To account for Blast Nova delay caused by Quake's DelayAll.
    void HandleQuake(Unit* caster)
    {
        auto it = blastNovaTimer.find(caster->GetInstanceId());
        if (it != blastNovaTimer.end())
            it->second += QUAKE_DELAY_MS;
    }

    // Cube clickers are permitted to continue casting while in the waiting position, so a spell
    // interrupt request is needed to get them to cancel mid-cast when Blast Nova starts.
    void HandleBlastNova(Unit* caster)
    {
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !IsCubeClicker(player))
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("magtheridon", BOT_STATE_COMBAT))
                continue;

            if (player->HasAura(Id(MagSpells::SPELL_SHADOW_GRASP)))
                continue;

            botAI->RequestSpellInterrupt();
        }
    }
};

// Not to be confused with the 30% ceiling collapse, which is also called "Debris." This is for the
// small patches that fall from time-to-time after the ceiling collapses, which deal 87,500 to
// 112,500 damage on hit (!!!)
class MagtheridonDebrisDynamicObjectScript : public DynamicObjectScript
{
public:
    MagtheridonDebrisDynamicObjectScript() :
        DynamicObjectScript("MagtheridonDebrisDynamicObjectScript") {}

    void OnUpdate(DynamicObject* debris, uint32 /*diff*/) override
    {
        if (debris->GetSpellId() != Id(MagSpells::SPELL_DEBRIS_SPAWN))
            return;

        Map::PlayerList const& players = debris->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("magtheridon", BOT_STATE_COMBAT) ||
                debris->GetExactDist2d(player) > DEBRIS_HAZARD_RADIUS)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonSpellListenerScript();
    new MagtheridonDebrisDynamicObjectScript();
}
