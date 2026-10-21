/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSActions.h"
#include "CharmInfo.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Totem.h"

#include <algorithm>
#include <limits>
#include <unordered_set>

bool MoveFromBronjahmAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->GetExactDist2d(boss) < 10.0f)
        return FleePosition(boss->GetPosition(), 15.0f, 2000U);
    else
        return true;

    return false;
}

bool AttackCorruptedSoulFragmentAction::Execute(Event /*event*/)
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    // If no valid skull target, search for corrupted soul fragment
    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
                // Mark corrupted soul fragment with skull if in group and not already marked
                if (Group* group = bot->GetGroup())
                {
                    ObjectGuid currentSkullGuid = group->GetTargetIcon(7);
                    if (currentSkullGuid.IsEmpty() || currentSkullGuid != unit->GetGUID())
                    {
                        group->SetTargetIcon(7, bot->GetGUID(), unit->GetGUID());  // 7 = skull
                    }
                }
                break;
        }

    }

    return false;
}

bool BronjahmGroupPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("soulstorm", boss);
    bool hasAura = aura;

    Unit* corruptedSoul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    bool activeSoulExists = corruptedSoul && corruptedSoul->IsAlive();

    if (PlayerbotAI::IsTank(bot) && botAI->HasAggro(boss))
    {
        // If any corrupted soul exists, handle positioning carefully
        if (activeSoulExists)
        {
            GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (auto& npc : npcs)
            {
                Unit* unit = botAI->GetUnit(npc);
                if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
                {
                    // Soul exists - check positions
                    float soulToBossDist = unit->GetExactDist2d(boss);
                    float tankToBossDist = bot->GetExactDist2d(boss);
                    float soulToTankDist = unit->GetExactDist2d(bot);

                    // Handle edge case: if soul is between tank and boss
                    // This happens when: (tankToBossDist > soulToBossDist) AND (soulToTankDist < tankToBossDist)
                    if (tankToBossDist > soulToBossDist && soulToTankDist < tankToBossDist)
                    {
                        // Calculate position 5 yards behind boss from the soul's perspective
                        float angle = boss->GetAngle(unit);                                  // Angle from boss to soul
                        float oppositeAngle = Position::NormalizeOrientation(angle + M_PI);  // Opposite direction

                        // Calculate position 5 yards behind boss
                        float x = boss->GetPositionX() + 5.0f * cos(oppositeAngle);
                        float y = boss->GetPositionY() + 5.0f * sin(oppositeAngle);
                        float z = boss->GetPositionZ();

                        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true,
                                      MovementPriority::MOVEMENT_NORMAL);
                    }

                    // If soul is near boss, flee from boss
                    if (soulToBossDist < 10.0f)
                        return FleePosition(unit->GetPosition(), 13.0f, 1000U);

                    // If soul exists but none of the above conditions, don't move to tank position yet
                    bot->SetFacingToObject(boss);
                    return true;
                }
            }
        }
        else
        {
            // No souls exist, safe to move to tank position
            if (bot->GetExactDist2d(BRONJAHM_TANK_POSITION) > 5.0f)
            {
                return MoveTo(bot->GetMapId(), BRONJAHM_TANK_POSITION.GetPositionX(),
                              BRONJAHM_TANK_POSITION.GetPositionY(), BRONJAHM_TANK_POSITION.GetPositionZ(), false,
                              false, false, true, MovementPriority::MOVEMENT_NORMAL);
            }
        }
    }
    else
    {
        float maxMovement = 10.0f;

        if (bot->GetExactDist2d(boss) > maxMovement && !activeSoulExists && (hasAura || boss->FindCurrentSpellBySpellId(SPELL_SOULSTORM_VISUAL) || boss->FindCurrentSpellBySpellId(SPELL_SOULSTORM_VISUAL2)))
        {
            if (PlayerbotAI::IsRanged(bot))
            {
                return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 6.5f, maxMovement));
            }
            else
            {
                return Move(bot->GetAngle(boss), fmin(bot->GetExactDist2d(boss) - 2.0f, maxMovement));
            }
        }
        else
            return false;
    }

    return false;
}

bool BronjahmGroupPositionAction::isUseful() { return true; }

bool IsInterruptSpell(std::string const& spell)
{
    static std::unordered_set<std::string> const interrupts = {
        "kick", "counterspell", "pummel", "shield bash", "wind shear", "mind freeze", "arcane torrent", "spell lock",
        "silencing shot"};
    return interrupts.count(spell) > 0;
}

bool DevourerWailingSoulsAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    bool inWell = FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS);
    if (boss->isInBack(bot, WAILING_SOULS_SAFE_ARC) && !inWell)
        return false;

    // Melee keeps hitting from behind; ranged keep their distance but move into the same safe wedge
    float distance = PlayerbotAI::IsRanged(bot) ? std::clamp(bot->GetExactDist2d(boss), 8.0f, 20.0f)
                                                : std::max(boss->GetCombatReach(), 2.0f);
    float facing = boss->GetOrientation();
    float relative = Position::NormalizeOrientation(boss->GetAngle(bot) - facing);  // 0..2pi, pi = straight behind
    if (relative < float(M_PI) / 2.0f || relative > 3.0f * float(M_PI) / 2.0f)
    {
        // In front: a straight line behind runs through the cone, so step out to this side first
        float side = relative < float(M_PI) ? 0.6f * float(M_PI) : 1.4f * float(M_PI);
        float d = std::max(bot->GetExactDist2d(boss), distance);
        float x = boss->GetPositionX() + d * std::cos(facing + side);
        float y = boss->GetPositionY() + d * std::sin(facing + side);
        float z = boss->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // A Wailing tick (~4.9k every 0.5 s) hurts more than a well tick (~3k per second): take a well spot if nothing else
    float behind = Position::NormalizeOrientation(facing + float(M_PI));
    float fallbackX = 0.0f, fallbackY = 0.0f;
    bool haveFallback = false;
    for (float offset : {0.0f, 0.25f, -0.25f, 0.45f, -0.45f})
    {
        float angle = Position::NormalizeOrientation(behind + offset);
        float x = boss->GetPositionX() + distance * std::cos(angle);
        float y = boss->GetPositionY() + distance * std::sin(angle);
        if (!haveFallback)
        {
            fallbackX = x;
            fallbackY = y;
            haveFallback = true;
        }
        if (FindWellOfSouls(bot, x, y, WELL_OF_SOULS_AVOID_RADIUS))
            continue;

        float z = boss->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    if (boss->isInBack(bot, WAILING_SOULS_SAFE_ARC))
        return false;  // already behind; only the well is left and moving would not leave it

    float z = boss->GetPositionZ();
    bot->UpdateAllowedPositionZ(fallbackX, fallbackY, z);
    return MoveTo(bot->GetMapId(), fallbackX, fallbackY, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool DevourerWellOfSoulsAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    Unit* well = FindWellOfSouls(bot, bot->GetPositionX(), bot->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS);
    if (!well && boss && PlayerbotAI::IsTank(bot))
    {
        if (IsWailingSoulsActive(boss))
            return false;  // he is rooted for the wail; dragging only walks the tank into the sweep

        // The boss leaps onto its target, so the well is often under him: drag him out so melee can follow
        Unit* bossWell = FindWellOfSouls(bot, boss->GetPositionX(), boss->GetPositionY(), WELL_OF_SOULS_AVOID_RADIUS);
        if (!bossWell)
            return false;

        float angle = bossWell->GetExactDist2d(bot) > 0.5f ? bossWell->GetAngle(bot) : bot->GetOrientation() + float(M_PI);
        float distance = WELL_OF_SOULS_AVOID_RADIUS + boss->GetCombatReach() + 3.0f;
        float x = bossWell->GetPositionX() + distance * std::cos(angle);
        float y = bossWell->GetPositionY() + distance * std::sin(angle);
        float z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        if (bot->GetExactDist2d(x, y) < 1.0f || !bot->IsWithinLOS(x, y, z + 1.0f))
            return false;

        return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    if (!well)
        return false;

    // Melee step out on the side nearest the boss so they can keep hitting; everyone else takes the shortest way out
    bool stayNearBoss = boss && PlayerbotAI::IsMelee(bot) && !PlayerbotAI::IsTank(bot);
    float bestScore = std::numeric_limits<float>::max();
    float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    // Wider rings only when wells cluster: the multiplier blocks every other action while the bot stands in one
    for (float radius : {WELL_OF_SOULS_AVOID_RADIUS + 1.5f, WELL_OF_SOULS_AVOID_RADIUS + 4.0f, WELL_OF_SOULS_AVOID_RADIUS + 7.0f})
    {
        if (bestScore != std::numeric_limits<float>::max())
            break;

        for (int i = 0; i < 16; ++i)
        {
            float angle = float(i) * float(M_PI) / 8.0f;
            float x = well->GetPositionX() + radius * std::cos(angle);
            float y = well->GetPositionY() + radius * std::sin(angle);
            float z = bot->GetPositionZ();
            if (FindWellOfSouls(bot, x, y, WELL_OF_SOULS_AVOID_RADIUS))
                continue;

            // During a wail only spots behind the boss count; stepping out of a well into the sweep is worse
            if (boss && IsWailingSoulsActive(boss))
            {
                float relative = Position::NormalizeOrientation(boss->GetAngle(x, y) - boss->GetOrientation());
                if (std::fabs(relative - float(M_PI)) > WAILING_SOULS_SAFE_ARC / 2.0f)
                    continue;
            }

            bot->UpdateAllowedPositionZ(x, y, z);
            if (std::fabs(z - bot->GetPositionZ()) > 3.0f || !bot->IsWithinLOS(x, y, z + 1.0f))
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

// Totems are left out: TotemAI ignores react state and picks its own target
static std::vector<Creature*> ControlledCreatures(Player* bot)
{
    std::vector<Creature*> pets;
    for (Unit* controlled : bot->m_Controlled)
    {
        Creature* pet = controlled ? controlled->ToCreature() : nullptr;
        if (pet && pet->IsAlive() && !pet->IsTotem())
            pets.push_back(pet);
    }
    return pets;
}

static bool TotemDealsDamage(Totem* totem)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(totem->GetSpell());
    if (!info)
        return false;

    // Searing Totem hits directly; Magma Totem pulses a triggered spell
    for (SpellEffectInfo const& effect : info->Effects)
    {
        if (effect.Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
            return true;
        if (effect.ApplyAuraName == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
            if (SpellInfo const* triggered = sSpellMgr->GetSpellInfo(effect.TriggerSpell))
                if (triggered->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE))
                    return true;
    }
    return false;
}

static std::vector<Totem*> DamagingTotems(Player* bot)
{
    std::vector<Totem*> totems;
    for (Unit* controlled : bot->m_Controlled)
    {
        Totem* totem = controlled && controlled->IsTotem() ? controlled->ToTotem() : nullptr;
        if (totem && totem->IsAlive() && TotemDealsDamage(totem))
            totems.push_back(totem);
    }
    return totems;
}

bool IsDamagingTotemSpell(std::string const& spell)
{
    return spell == "searing totem" || spell == "magma totem" || spell == "fire elemental totem";
}

bool DevourerStopDamageAction::isUseful()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    if (bot->GetVictim() == boss || IsCastingAt(bot, boss) || !DamagingTotems(bot).empty())
        return true;

    for (Creature* pet : ControlledCreatures(bot))
    {
        if (pet->GetVictim() == boss || pet->GetReactState() != REACT_PASSIVE)
            return true;
    }
    return false;
}

bool DevourerStopDamageAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    if (bot->GetVictim() == boss)
        bot->AttackStop();

    // Auto-repeat covers wands: the priest's Shoot kept firing through the whole debuff otherwise
    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL})
    {
        Spell* spell = bot->GetCurrentSpell(slot);
        if (spell && spell->m_targets.GetUnitTargetGUID() == boss->GetGUID())
            bot->InterruptSpell(slot);
    }

    // The multiplier keeps the shaman from dropping them again until the debuff fades
    for (Totem* totem : DamagingTotems(bot))
        totem->UnSummon();

    // Passive until the debuff fades, or the pet's AI picks the boss straight back up
    for (Creature* pet : ControlledCreatures(bot))
    {
        DevourerRememberSuspendedPet(bot, pet);
        pet->SetReactState(REACT_PASSIVE);
        pet->AttackStop();
        pet->CastStop();
        if (CharmInfo* ci = pet->GetCharmInfo())
        {
            ci->SetPlayerReactState(REACT_PASSIVE);
            ci->SetCommandState(COMMAND_FOLLOW);
            ci->SetIsCommandAttack(false);
            ci->SetIsAtStay(false);
            ci->SetIsReturning(true);
            ci->SetIsFollowing(true);
            pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
        }
    }

    return true;
}

bool DevourerRestorePetsAction::Execute(Event /*event*/)
{
    // Only the pets the debuff suspended, back to the stance they had
    bool restored = false;
    for (auto const& [guid, state] : DevourerTakeSuspendedPets(bot))
    {
        Creature* pet = ObjectAccessor::GetCreature(*bot, guid);
        if (!pet || !pet->IsAlive())
            continue;

        pet->SetReactState(state);
        if (CharmInfo* ci = pet->GetCharmInfo())
            ci->SetPlayerReactState(state);
        restored = true;
    }
    return restored;
}

bool DevourerInterruptPhantomBlastAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss || !IsPhantomBlastCasting(boss))
        return false;

    Spell* blast = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    for (char const* name : {"kick", "pummel", "shield bash", "wind shear", "mind freeze", "counterspell", "arcane torrent"})
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", name);
        SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        if (!spellInfo || !botAI->CanCastSpell(name, boss))
            continue;

        // Long-cooldown interrupts wait for the second half of the 2 s cast so a 10 s Kick can take it first
        uint32 cooldown = std::max(spellInfo->RecoveryTime, spellInfo->CategoryRecoveryTime);
        if (cooldown > 15 * IN_MILLISECONDS && blast->GetCastTimeRemaining() > 1000)
            return false;

        return botAI->CastSpell(name, boss);
    }

    return false;
}
