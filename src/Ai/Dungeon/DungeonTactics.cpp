// Engine glue shared by dungeon encounter tactics: action shapes, casts on a target and pets stood down for a mechanic.

#include "DungeonTactics.h"

#include <vector>

#include "AttackAction.h"
#include "CharmInfo.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PetDefines.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "Totem.h"

namespace DungeonTactics
{
namespace
{
constexpr CurrentSpellTypes CastSlots[] = {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL};

// Totems are left out: TotemAI ignores react state and picks its own target
std::vector<Creature*> ControlledCreatures(Player* bot)
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

bool TotemDealsDamage(Totem* totem)
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

std::vector<Totem*> DamagingTotems(Player* bot)
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
}  // namespace

ActionShape ShapeOf(Action* action)
{
    ActionShape shape;
    shape.movement = dynamic_cast<MovementAction*>(action) != nullptr;
    shape.attack = dynamic_cast<AttackAction*>(action) != nullptr;
    shape.avoidAoe = dynamic_cast<AvoidAoeAction*>(action) != nullptr;
    if (CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action))
    {
        bool cure = dynamic_cast<CastCureSpellAction*>(action) || dynamic_cast<CurePartyMemberAction*>(action);
        shape.cast = ClassifyCast(cast->getSpell(), dynamic_cast<CastHealingSpellAction*>(action) != nullptr,
                                  dynamic_cast<CastBuffSpellAction*>(action) != nullptr, cure);
    }
    return shape;
}

bool IsCastingAt(Unit* caster, Unit* target, bool withAutoRepeat)
{
    for (CurrentSpellTypes slot : CastSlots)
    {
        if (slot == CURRENT_AUTOREPEAT_SPELL && !withAutoRepeat)
            continue;

        Spell* spell = caster->GetCurrentSpell(slot);
        if (spell && spell->m_targets.GetUnitTargetGUID() == target->GetGUID())
            return true;
    }
    return false;
}

void StopCastsAt(Player* bot, Unit* target)
{
    for (CurrentSpellTypes slot : CastSlots)
    {
        Spell* spell = bot->GetCurrentSpell(slot);
        if (spell && spell->m_targets.GetUnitTargetGUID() == target->GetGUID())
            bot->InterruptSpell(slot);
    }
}

void SuspendedPets::Remember(Player* bot, Creature* pet)
{
    std::lock_guard<std::mutex> guard(_lock);
    _pets[bot->GetGUID()].emplace(pet->GetGUID(), pet->GetReactState());
}

bool SuspendedPets::Any(Player* bot)
{
    std::lock_guard<std::mutex> guard(_lock);
    auto it = _pets.find(bot->GetGUID());
    return it != _pets.end() && !it->second.empty();
}

std::unordered_map<ObjectGuid, ReactStates> SuspendedPets::Take(Player* bot)
{
    std::lock_guard<std::mutex> guard(_lock);
    auto node = _pets.extract(bot->GetGUID());
    return node.empty() ? std::unordered_map<ObjectGuid, ReactStates>() : std::move(node.mapped());
}

bool PetsOrTotemsFighting(Player* bot, Unit* victim)
{
    if (!DamagingTotems(bot).empty())
        return true;

    for (Creature* pet : ControlledCreatures(bot))
    {
        Unit* petVictim = pet->GetVictim();
        if ((petVictim && (!victim || petVictim == victim)) || pet->GetReactState() != REACT_PASSIVE)
            return true;
    }
    return false;
}

void StandDownPets(Player* bot, SuspendedPets& store)
{
    // The encounter multiplier keeps the shaman from dropping them again until the mechanic ends
    for (Totem* totem : DamagingTotems(bot))
        totem->UnSummon();

    // Passive until the mechanic ends, or the pet's AI picks a target straight back up
    for (Creature* pet : ControlledCreatures(bot))
    {
        store.Remember(bot, pet);
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
}

bool RestorePets(Player* bot, SuspendedPets& store)
{
    bool restored = false;
    for (auto const& [guid, state] : store.Take(bot))
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
}  // namespace DungeonTactics
