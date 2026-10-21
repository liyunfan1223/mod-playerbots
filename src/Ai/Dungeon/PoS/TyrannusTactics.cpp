// Scourgelord Tyrannus bot tactics: ranged spread out, and a branded player holds damage and heals until it fades.

#include "DungeonTactics.h"
#include "Playerbots.h"
#include "PoSActions.h"
#include "PoSMultipliers.h"
#include "PoSTriggers.h"
#include "Spell.h"
#include "SpellInfo.h"

namespace
{
DungeonTactics::SuspendedPets tyrannusPets;

bool BrandedHealerMayHeal(Player* bot, Unit* boss)
{
    Unit* tank = boss->GetVictim();
    return PitOfSaron::BrandedHealerMayHeal(PlayerbotAI::IsHeal(bot), tank != nullptr,
                                            tank ? tank->GetHealthPct() : 100.0f);
}
}  // namespace

bool TyrannusTrigger::IsActive() { return AI_VALUE2(Unit*, "find target", "scourgelord tyrannus"); }

bool TyrannusOverlordsBrandTrigger::IsActive()
{
    // A branded tank keeps tanking: its damage only lands on itself
    return bot->HasAura(SPELL_OVERLORDS_BRAND) && !PlayerbotAI::IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
}

bool TyrannusOverlordsBrandFadedTrigger::IsActive()
{
    // No boss needed: Tyrannus often dies while someone is branded, and the pets still need their stance back
    return !bot->HasAura(SPELL_OVERLORDS_BRAND) && tyrannusPets.Any(bot);
}

bool TyrannusBrandStopCastTrigger::IsActive()
{
    // A reaction: the main engine waits out a cast in progress, so a heal begun just before the brand would land
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss || !bot->HasAura(SPELL_OVERLORDS_BRAND) || PlayerbotAI::IsTank(bot))
        return false;

    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
    {
        Spell* spell = bot->GetCurrentSpell(slot);
        if (!spell)
            continue;

        SpellInfo const* info = spell->GetSpellInfo();
        bool heal = info->HasEffect(SPELL_EFFECT_HEAL) || info->HasAura(SPELL_AURA_PERIODIC_HEAL);
        if (PitOfSaron::BrandStopsCast(info->IsPositive(), heal, BrandedHealerMayHeal(bot, boss)))
            return true;
    }
    return false;
}

bool TyrannusAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    return boss && PlayerbotAI::IsRanged(bot) && boss->HealthBelowPct(99) && RangedSpread();
}

bool TyrannusAction::RangedSpread()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (ObjectGuid const& guid : members)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || unit == bot)
            continue;

        float step = PitOfSaron::SpreadStep(bot->GetExactDist2d(unit));
        if (step > 0.0f)
            return FleePosition(unit->GetPosition(), step, 250U);
    }
    return false;
}

bool TyrannusBrandStopAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return false;

    // Every heal from the branded player also heals Tyrannus, so any cast counts, not only those on him
    return bot->GetVictim() || (bot->IsNonMeleeSpellCast(true) && !BrandedHealerMayHeal(bot, boss)) ||
           DungeonTactics::PetsOrTotemsFighting(bot, nullptr);
}

bool TyrannusBrandStopAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss || !BrandedHealerMayHeal(bot, boss))
        bot->InterruptNonMeleeSpells(true);

    DungeonTactics::StandDownPets(bot, tyrannusPets);
    return true;
}

bool TyrannusBrandRestorePetsAction::Execute(Event /*event*/) { return DungeonTactics::RestorePets(bot, tyrannusPets); }

float TyrannusBrandMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_OVERLORDS_BRAND) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "scourgelord tyrannus");
    if (!boss)
        return 1.0f;

    DungeonTactics::ActionShape shape = DungeonTactics::ShapeOf(action);
    return PitOfSaron::BrandAllows(shape, BrandedHealerMayHeal(bot, boss)) ? 1.0f : 0.0f;
}
