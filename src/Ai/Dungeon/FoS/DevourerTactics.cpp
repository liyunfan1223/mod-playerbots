// Devourer of Souls bot tactics: get behind him for Wailing Souls, stay out of wells, hold damage on Mirrored Soul.

#include <limits>
#include <string>

#include "DungeonTactics.h"
#include "FoSActions.h"
#include "FoSMultipliers.h"
#include "FoSPolicy.h"
#include "FoSTriggers.h"
#include "Group.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellMgr.h"

namespace
{
DungeonTactics::SuspendedPets devourerPets;

bool IsWailingSoulsActive(Unit* boss)
{
    return boss->FindCurrentSpellBySpellId(SPELL_WAILING_SOULS) || boss->HasAura(SPELL_WAILING_SOULS_SWEEP) ||
           boss->HasAura(SPELL_WAILING_SOULS_SWEEP_2);
}

bool IsPhantomBlastCasting(Unit* boss)
{
    Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell)
        return false;

    uint32 id = spell->m_spellInfo->Id;
    return id == SPELL_PHANTOM_BLAST || id == SPELL_PHANTOM_BLAST_H;
}

bool IsMirroredSoulActive(Player* bot, Unit* boss)
{
    if (boss->FindCurrentSpellBySpellId(SPELL_MIRRORED_SOUL))
        return true;

    // The debuff sits on the targeted player, not on the boss
    Group* group = bot->GetGroup();
    if (!group)
        return bot->HasAura(SPELL_MIRRORED_SOUL);

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsInWorld() && member->HasAura(SPELL_MIRRORED_SOUL))
            return true;
    }
    return false;
}

Unit* FindWellOfSouls(WorldObject* searcher, float x, float y, float radius = ForgeOfSouls::WellAvoidRadius)
{
    std::list<Creature*> wells;
    searcher->GetCreatureListWithEntryInGrid(wells, uint32(NPC_WELL_OF_SOULS), 60.0f);
    for (Creature* well : wells)
    {
        if (well->IsInWorld() && well->GetExactDist2d(x, y) < radius)
            return well;
    }
    return nullptr;
}

float RelativeToBoss(Unit* boss, float x, float y)
{
    return DungeonTactics::RelativeAngle(boss->GetAngle(x, y), boss->GetOrientation());
}

bool IsDevourerAction(Action* action)
{
    return dynamic_cast<DevourerWailingSoulsAction*>(action) || dynamic_cast<DevourerWellOfSoulsAction*>(action) ||
           dynamic_cast<DevourerStopDamageAction*>(action) ||
           dynamic_cast<DevourerInterruptPhantomBlastAction*>(action) || dynamic_cast<DevourerStopCastAction*>(action);
}
}  // namespace

bool DevourerWailingSoulsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsWailingSoulsActive(boss);
}

bool DevourerWellOfSoulsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    if (FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY()))
        return true;

    return PlayerbotAI::IsTank(bot) && boss->GetVictim() == bot &&
           FindWellOfSouls(bot, boss->GetPositionX(), boss->GetPositionY());
}

bool DevourerMirroredSoulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsMirroredSoulActive(bot, boss);
}

bool DevourerMirroredSoulFadedTrigger::IsActive()
{
    // No boss needed: if he dies or evades mid-debuff the pets still get their stance back
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (boss && IsMirroredSoulActive(bot, boss))
        return false;

    return devourerPets.Any(bot);
}

bool DevourerPhantomBlastTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsPhantomBlastCasting(boss);
}

bool DevourerStopCastTrigger::IsActive()
{
    // A reaction: the main engine waits out a cast in progress, so a bolt begun before the debuff would land
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    return boss && IsMirroredSoulActive(bot, boss) && DungeonTactics::IsCastingAt(bot, boss, false);
}

bool DevourerWailingSoulsAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    float relative = RelativeToBoss(boss, bot->GetPositionX(), bot->GetPositionY());
    if (ForgeOfSouls::InSafeWedge(relative) && !FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    auto moveTo = [&](float x, float y)
    {
        float z = boss->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    };

    float facing = boss->GetOrientation();
    float distance = ForgeOfSouls::WailingStandDistance(PlayerbotAI::IsRanged(bot), bot->GetExactDist2d(boss),
                                                        boss->GetCombatReach());
    if (ForgeOfSouls::InFrontHalf(relative))
    {
        float side = facing + ForgeOfSouls::SideStepAngle(relative);
        float d = std::max(bot->GetExactDist2d(boss), distance);
        return moveTo(boss->GetPositionX() + d * std::cos(side), boss->GetPositionY() + d * std::sin(side));
    }

    // A Wailing tick (~4.9k every 0.5 s) hurts more than a well tick (~3k per second): take a well spot if nothing else
    float behind = facing + float(M_PI);
    for (float offset : ForgeOfSouls::BehindOffsets)
    {
        float x = boss->GetPositionX() + distance * std::cos(behind + offset);
        float y = boss->GetPositionY() + distance * std::sin(behind + offset);
        if (!FindWellOfSouls(bot, x, y))
            return moveTo(x, y);
    }

    if (ForgeOfSouls::InSafeWedge(relative))
        return false;  // already behind; only the well is left and moving would not leave it

    return moveTo(boss->GetPositionX() + distance * std::cos(behind),
                  boss->GetPositionY() + distance * std::sin(behind));
}

bool DevourerWellOfSoulsAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (Unit* well = FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY()))
        return StepOut(boss, well);

    return boss && PlayerbotAI::IsTank(bot) && DragBossClear(boss);
}

bool DevourerWellOfSoulsAction::DragBossClear(Unit* boss)
{
    if (IsWailingSoulsActive(boss))
        return false;  // he is rooted for the wail; dragging only walks the tank into the sweep

    // He leaps onto his target, so the well is often under him: drag him out so melee can follow
    Unit* bossWell = FindWellOfSouls(bot, boss->GetPositionX(), boss->GetPositionY());
    if (!bossWell)
        return false;

    float angle = bossWell->GetExactDist2d(bot) > 0.5f ? bossWell->GetAngle(bot) : bot->GetOrientation() + float(M_PI);
    float distance = ForgeOfSouls::WellDragDistance(boss->GetCombatReach());
    float x = bossWell->GetPositionX() + distance * std::cos(angle);
    float y = bossWell->GetPositionY() + distance * std::sin(angle);
    float z = bot->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);
    if (bot->GetExactDist2d(x, y) < 1.0f || !bot->IsWithinLOS(x, y, z + 1.0f))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool DevourerWellOfSoulsAction::StepOut(Unit* boss, Unit* well)
{
    // Melee step out on the side nearest the boss so they can keep hitting; everyone else takes the shortest way out
    bool stayNearBoss = boss && PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);
    bool wailing = boss && IsWailingSoulsActive(boss);
    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    for (float radius : ForgeOfSouls::WellExitRadii)
    {
        if (bestScore != std::numeric_limits<float>::max())
            break;

        for (int i = 0; i < ForgeOfSouls::WellExitPoints; ++i)
        {
            float angle = float(i) * 2.0f * float(M_PI) / float(ForgeOfSouls::WellExitPoints);
            float x = well->GetPositionX() + radius * std::cos(angle);
            float y = well->GetPositionY() + radius * std::sin(angle);
            if (FindWellOfSouls(bot, x, y))
                continue;

            if (boss && !ForgeOfSouls::WellExitAllowed(wailing, RelativeToBoss(boss, x, y)))
                continue;

            float z = bot->GetPositionZ();
            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > ForgeOfSouls::MaxClimb || !bot->IsWithinLOS(x, y, z + 1.0f))
                continue;

            float score = stayNearBoss ? boss->GetExactDist2d(x, y) : bot->GetExactDist2d(x, y);
            if (score < bestScore)
            {
                bestScore = score;
                bestX = x;
                bestY = y;
                bestZ = z;
            }
        }
    }

    if (bestScore == std::numeric_limits<float>::max())
        return false;

    return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool DevourerStopDamageAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    return bot->GetVictim() == boss || DungeonTactics::IsCastingAt(bot, boss) ||
           DungeonTactics::PetsOrTotemsFighting(bot, boss);
}

bool DevourerStopDamageAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    if (bot->GetVictim() == boss)
        bot->AttackStop();

    DungeonTactics::StopCastsAt(bot, boss);
    DungeonTactics::StandDownPets(bot, devourerPets);
    return true;
}

bool DevourerRestorePetsAction::Execute(Event /*event*/) { return DungeonTactics::RestorePets(bot, devourerPets); }

bool DevourerInterruptPhantomBlastAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss || !IsPhantomBlastCasting(boss))
        return false;

    Spell* blast = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    for (std::string_view interrupt : ForgeOfSouls::PhantomBlastInterrupts)
    {
        std::string name(interrupt);
        uint32 spellId = AI_VALUE2(uint32, "spell id", name);
        SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        if (!spellInfo || !botAI->CanCastSpell(name, boss))
            continue;

        uint32 cooldown = std::max(spellInfo->RecoveryTime, spellInfo->CategoryRecoveryTime);
        if (ForgeOfSouls::HoldForQuickerInterrupt(cooldown, blast->GetCastTimeRemaining()))
            return false;

        return botAI->CastSpell(name, boss);
    }
    return false;
}

float DevourerOfSoulsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss || IsDevourerAction(action))
        return 1.0f;

    // In a well, leaving comes first: an attack in place cancelled the step out every tick and kept a rogue there 30 s
    if (FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY()))
        return 0.0f;

    DungeonTactics::ActionShape shape = DungeonTactics::ShapeOf(action);
    ForgeOfSouls::DevourerFight fight;
    fight.phantomBlast = IsPhantomBlastCasting(boss);
    fight.mirroredSoul = IsMirroredSoulActive(bot, boss);
    fight.wailingSouls = IsWailingSoulsActive(boss);
    if (!ForgeOfSouls::DevourerAllows(shape, fight))
        return 0.0f;

    // Once out of a well, stay out: healer repositioning and melee chasing a boss who stands in one walked bots back in
    if (DungeonTactics::IsRepositioning(shape))
    {
        if (FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY(), ForgeOfSouls::WellGuardRadius))
            return 0.0f;

        if (PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot) &&
            FindWellOfSouls(bot, boss->GetPositionX(), boss->GetPositionY()))
            return 0.0f;
    }

    return 1.0f;
}
