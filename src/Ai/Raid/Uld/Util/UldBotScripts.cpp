/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "UldData.h"
#include "UldEncounter_Vezax.h"
#include "UldEncounter_YoggSaron.h"

#include <algorithm>

namespace
{

// A bot part way through a cast can't move, so without this it finishes the cast standing in the
// impact. Runs once, at the instant the missile goes out.
void InterruptVezaxCastersNear(Unit* reference, Position const& hazard, float radius,
                               bool (*needsToLeave)(Player*))
{
    if (!reference || !needsToLeave)
        return;

    Map::PlayerList const& players = reference->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* player = itr->GetSource();
        if (!player || !player->IsAlive())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || !botAI->HasStrategy("ulduar", BOT_STATE_COMBAT))
            continue;

        // Only break the cast of a bot that is about to be told to move. Everyone else either wants
        // to be standing there or is unaffected, and a wasted interrupt is a wasted global.
        if (!needsToLeave(player))
            continue;

        if (player->GetExactDist2d(hazard.GetPositionX(), hazard.GetPositionY()) > radius)
            continue;

        botAI->RequestSpellInterrupt();
    }
}

}  // namespace

class VezaxHazardListenerScript : public AllSpellScript
{
public:
    VezaxHazardListenerScript() : AllSpellScript("VezaxHazardListenerScript") {}

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo,
                     bool /*skipCheck*/) override
    {
        if (!caster || !spellInfo || caster->GetMapId() != ULDUAR_MAP_ID)
            return;

        if (spellInfo->Id == SPELL_VEZAX_SUMMON_SARONITE_VAPORS && caster->GetEntry() == NPC_VEZAX)
        {
            VezaxNoteVaporSummon(caster);
            return;
        }

        if (spellInfo->Id != SPELL_VEZAX_SHADOW_CRASH_CAST)
            return;

        // Both effects are TRIGGER_MISSILE at TARGET_DEST_TARGET_ENEMY, so no unit target. The dest
        // is frozen at cast, so this is where it lands however far the target walks
        Position impact;
        if (WorldLocation const* dst = spell->m_targets.GetDstPos())
            impact.Relocate(dst->GetPositionX(), dst->GetPositionY(), dst->GetPositionZ());
        else if (Unit* target = spell->m_targets.GetUnitTarget())
            impact = target->GetPosition();
        else
            return;

        // Straight from distance over Speed rather than Spell::GetDelayMoment, which the core
        // fills in during the same cast this hook runs inside. Both are milliseconds.
        float const speed = spellInfo->Speed;
        uint32 const flightMs =
            speed > 0.0f ? static_cast<uint32>(caster->GetExactDist(&impact) / speed * IN_MILLISECONDS) : 0;

        VezaxNoteShadowCrash(caster, impact, flightMs);
        InterruptVezaxCastersNear(caster, impact, ULDUAR_VEZAX_SHADOW_CRASH_IMPACT_RADIUS,
                                  &VezaxDodgesShadowCrash);
    }
};

// The boss script keeps its hard mode flag private, so vapor deaths and the Animus summon are read off
// the vapor's self-casts, the first from JustDied. Prepare rather than cast, like the Guardian listener
class VezaxVaporListenerScript : public AllSpellScript
{
public:
    VezaxVaporListenerScript() : AllSpellScript("VezaxVaporListenerScript", {ALLSPELLHOOK_ON_PREPARE}) {}

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || !spellInfo || caster->GetMapId() != ULDUAR_MAP_ID)
            return;

        if (caster->GetEntry() != NPC_VEZAX_SARONITE_VAPORS)
            return;

        if (spellInfo->Id == SPELL_VEZAX_SARONITE_VAPORS_AURA)
            VezaxNoteVaporKilled(caster);
        else if (spellInfo->Id == SPELL_VEZAX_SUMMON_SARONITE_ANIMUS)
            VezaxNoteAnimusSummoned(caster);
    }
};

// Brain Link's partner is private to the aura script and 63803/63804 leave no aura, but one of the two
// is cast at the partner every second, so the cast names the pair
class YoggSaronBrainLinkListenerScript : public AllSpellScript
{
public:
    YoggSaronBrainLinkListenerScript()
        : AllSpellScript("YoggSaronBrainLinkListenerScript", {ALLSPELLHOOK_ON_PREPARE})
    {
    }

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!spell || !caster || !spellInfo || caster->GetMapId() != ULDUAR_MAP_ID)
            return;

        if (spellInfo->Id != SPELL_BRAIN_LINK_DAMAGE && spellInfo->Id != SPELL_BRAIN_LINK_OK)
            return;

        // 63803 goes out twice a tick, once at the partner and once back at the owner. The self-cast
        // is the one that carries no information.
        YoggSaronNoteBrainLinkPair(caster, spell->m_targets.GetUnitTarget());
    }
};

// A Guardian's death reaches no bot on its own: a corpse looks the same a tick or a minute later. Its
// JustDied casts 65719 once, so the cast is the moment the nova went off.
class YoggSaronGuardianDeathListenerScript : public AllSpellScript
{
public:
    YoggSaronGuardianDeathListenerScript()
        : AllSpellScript("YoggSaronGuardianDeathListenerScript", {ALLSPELLHOOK_ON_PREPARE})
    {
    }

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || !spellInfo || caster->GetMapId() != ULDUAR_MAP_ID)
            return;

        if (spellInfo->Id != SPELL_SHADOW_NOVA_SARA || caster->GetEntry() != NPC_GUARDIAN_OF_YS)
            return;

        YoggSaronNoteGuardianDeath(caster);
    }
};

void AddSC_UlduarBotScripts()
{
    new VezaxHazardListenerScript();
    new VezaxVaporListenerScript();
    new YoggSaronBrainLinkListenerScript();
    new YoggSaronGuardianDeathListenerScript();
}
