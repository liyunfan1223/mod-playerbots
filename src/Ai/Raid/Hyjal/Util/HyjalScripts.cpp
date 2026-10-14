/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllCreatureScript.h"
#include "HyjalHelpers.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"

using namespace HyjalHelpers;

namespace
{

Player* GetTargetedPlayer(Spell* spell)
{
    if (!spell)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    return nullptr;
}

bool ShouldInterruptForArchimondeAirBurst(Player* bot, Unit* caster, Player* target)
{
    if (!target)
        return false;

    Unit* activeTank = caster->GetVictim();
    if (!activeTank || activeTank == bot)
        return false;

    if (target != activeTank && target != bot)
        return false;

    float const distanceToActiveTank = bot->GetExactDist2d(activeTank);
    return distanceToActiveTank < AIR_BURST_SAFE_DISTANCE;
}

} // end anonymous namespace

// Inferno summons a Towering Infernal at its target's then-current position after a 3.5s cast.
class AnetheronInfernoSpellListenerScript : public AllSpellScript
{
public:
    AnetheronInfernoSpellListenerScript() :
        AllSpellScript("AnetheronInfernoSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_INFERNO))
            return;

        Player* target = GetTargetedPlayer(spell);
        if (!target)
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);
        if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT))
            return;

        botAI->RequestSpellInterrupt();
    }
};

// Air Burst is a 2s cast that hits all players within 13y of the target.
class ArchimondeAirBurstSpellListenerScript : public AllSpellScript
{
public:
    ArchimondeAirBurstSpellListenerScript() :
        AllSpellScript("ArchimondeAirBurstSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != Id(HyjalSpells::SPELL_AIR_BURST))
            return;

        Player* target = GetTargetedPlayer(spell);
        if (!target)
            return;

        archimondeAirBurstTargets[caster->GetInstanceId()] =
            AirBurstData{ target->GetGUID(), getMSTime() };

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                !ShouldInterruptForArchimondeAirBurst(player, caster, target))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

// Doomfire's mechanic is pretty interesting. A Doomfire Spirit trigger NPC teleports up to 8y
// every 1.6s, and a Doomfire trigger NPC follows it after each teleport and drops the hazards.
// This hook reads the Doomfire NPC since it accompanies the visual fire trail. Real players cannot
// see the spirit so keying off of that would be a cheat.
class ArchimondeDoomfireTrailCreatureScript : public AllCreatureScript
{
public:
    ArchimondeDoomfireTrailCreatureScript()
        : AllCreatureScript("ArchimondeDoomfireTrailCreatureScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (creature->GetEntry() != Id(HyjalNpcs::NPC_DOOMFIRE))
            return;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                creature->GetExactDist2d(player) > DOOMFIRE_DANGER_RADIUS)
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_HyjalBotScripts()
{
    new AnetheronInfernoSpellListenerScript();
    new ArchimondeAirBurstSpellListenerScript();
    new ArchimondeDoomfireTrailCreatureScript();
}
