/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "Playerbots.h"
#include "SSCHelpers.h"

using namespace SscHelpers;

class LeotherasWhirlwindSpellListenerScript : public AllSpellScript
{
public:
    LeotherasWhirlwindSpellListenerScript()
        : AllSpellScript("LeotherasWhirlwindSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SscSpells::SPELL_LEOTHERAS_WHIRLWIND))
            return;

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || HasInnerDemon(player) ||
                caster->GetExactDist2d(player) >= LEOTHERAS_WHIRLWIND_SAFE_DISTANCE)
            {
                continue;
            }

            if (!player->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
                !player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (botAI && botAI->HasStrategy("ssc", BOT_STATE_COMBAT) &&
                !PlayerbotAI::IsTank(player))
            {
                botAI->RequestSpellInterrupt();
            }
        }
    }
};

// Interrupt a pending cast when a Toxic Spore pool spawns under the bot. Toxic Sporebats cast Toxic
// Spores (38574), which spawn a Spore Drop Trigger npc that spawns the Toxic Spore pool (38575).
class LadyVashjToxicSporesSpellListenerScript : public AllSpellScript
{
public:
    LadyVashjToxicSporesSpellListenerScript()
        : AllSpellScript("LadyVashjToxicSporesSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SscSpells::SPELL_TOXIC_SPORES))
            return;

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() ||
                caster->GetExactDist2d(player) >= TOXIC_SPORES_HIT_RADIUS)
            {
                continue;
            }

            if (!player->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
                !player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (botAI && botAI->HasStrategy("ssc", BOT_STATE_COMBAT))
                botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_SerpentshrineCavernBotScripts()
{
    new LeotherasWhirlwindSpellListenerScript();
    new LadyVashjToxicSporesSpellListenerScript();
}
