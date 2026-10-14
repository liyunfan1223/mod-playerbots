/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DRUIDTRIGGERS_H
#define PLAYERBOTS_DRUIDTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RtiTriggers.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include <set>

inline constexpr uint32 AURA_OMEN_OF_CLARITY = 16864;
inline constexpr uint32 SPELL_TREE_OF_LIFE = 33891;

class PlayerbotAI;

class MarkOfTheWildOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    MarkOfTheWildOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "mark of the wild", 8 * IN_MILLISECONDS) {}
};

class MarkOfTheWildTrigger : public BuffTrigger
{
public:
    MarkOfTheWildTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "mark of the wild", 8 * IN_MILLISECONDS) {}

    bool IsActive() override;
};

class ThornsOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    ThornsOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "thorns", 4 * IN_MILLISECONDS) {}

    bool IsActive() override;
};

class ThornsOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    ThornsOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "thorns", false, 4 * IN_MILLISECONDS) {}
};

class ThornsTrigger : public BuffTrigger
{
public:
    ThornsTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "thorns", 4 * IN_MILLISECONDS) {}

    bool IsActive() override;
};

class ClearcastingTrigger : public HasAuraTrigger
{
public:
    ClearcastingTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "clearcasting") {}
};

class PredatorsSwiftnessTrigger : public HasAuraTrigger
{
public:
    PredatorsSwiftnessTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "predator's swiftness") {}
};

class NaturesSwiftnessActiveTrigger : public HasAuraTrigger
{
public:
    NaturesSwiftnessActiveTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "nature's swiftness") {}
    bool IsActive() override { return botAI->HasAura("nature's swiftness", bot); }
};

class RakeTrigger : public DebuffTrigger
{
public:
    RakeTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "rake", 1, true) {}
    bool IsActive() override { return !botAI->HasAura("prowl", bot) && DebuffTrigger::IsActive(); }
};

class InsectSwarmTrigger : public DebuffTrigger
{
public:
    InsectSwarmTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "insect swarm", 1, true) {}
};

class InsectSwarmOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    InsectSwarmOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "insect swarm", true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class MoonfireTrigger : public DebuffTrigger
{
public:
    MoonfireTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "moonfire", 1, true) {}
};

class MoonfireOnAttackerTrigger : public DebuffOnAttackerTrigger
{
public:
    MoonfireOnAttackerTrigger(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, "moonfire", true) {}
    bool IsActive() override { return BuffTrigger::IsActive(); }
};

class FaerieFireTrigger : public DebuffTrigger
{
public:
    FaerieFireTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "faerie fire", 1, false, 25.0f) {}
};

class FaerieFireFeralTrigger : public DebuffTrigger
{
public:
    FaerieFireFeralTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "faerie fire (feral)") {}

    bool IsActive() override
    {
        if (!bot->IsInCombat())
            return false;

        // Bear: every cast generates immediate threat/damage for free — spam it
        if (botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr))
        {
            Unit* target = GetTarget();
            return target && target->IsAlive() && target->IsInWorld();
        }

        if (!botAI->HasAura("cat form", bot))
            return false;

        if (botAI->HasAura("prowl", bot))
            return false;

        // Cat with Omen of Clarity: spam to fish for Clearcasting procs
        if (bot->HasAura(AURA_OMEN_OF_CLARITY))
        {
            Unit* target = GetTarget();
            return target && target->IsAlive() && target->IsInWorld();
        }

        // Cat without Omen of Clarity: apply as a normal debuff, don't reapply
        return DebuffTrigger::IsActive();
    }
};

class BashInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    BashInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "bash") {}
};

class BerserkTrigger : public BoostTrigger
{
public:
    BerserkTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "berserk") {}
};

class BerserkActiveTrigger : public HasAuraTrigger
{
public:
    BerserkActiveTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "berserk") {}
};

class SavageRoarTrigger : public BuffTrigger
{
public:
    SavageRoarTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "savage roar") {}
};

class NaturesGraspTrigger : public BuffTrigger
{
public:
    NaturesGraspTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "nature's grasp") {}
};

class EntanglingRootsTrigger : public RtiCcTrigger
{
public:
    EntanglingRootsTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "entangling roots") {}
};

class EntanglingRootsKiteTrigger : public DebuffTrigger
{
public:
    EntanglingRootsKiteTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "entangling roots") {}

    bool IsActive() override;
};

class HibernateTrigger : public RtiCcTrigger
{
public:
    HibernateTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "hibernate") {}
};

class CycloneTrigger : public RtiCcTrigger
{
public:
    CycloneTrigger(PlayerbotAI* botAI) : RtiCcTrigger(botAI, "cyclone") {}
};

class CurePoisonTrigger : public NeedCureTrigger
{
public:
    CurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cure poison", DISPEL_POISON) {}
};

class PartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCurePoisonTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure poison", DISPEL_POISON)
    {
    }
};

class BearFormTrigger : public BuffTrigger
{
public:
    BearFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "bear form") {}

    bool IsActive() override;
};

class TreeFormTrigger : public BuffTrigger
{
public:
    TreeFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "tree of life") {}

    bool IsActive() override;
};

class CatFormTrigger : public BuffTrigger
{
public:
    CatFormTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "cat form") {}

    bool IsActive() override;
};

class AquaticFormTrigger : public Trigger
{
public:
    AquaticFormTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aquatic form") {}

    bool IsActive() override;
};

class EclipseSolarTrigger : public HasAuraTrigger
{
public:
    EclipseSolarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "eclipse (solar)") {}
};

class EclipseLunarTrigger : public HasAuraTrigger
{
public:
    EclipseLunarTrigger(PlayerbotAI* botAI) : HasAuraTrigger(botAI, "eclipse (lunar)") {}
};

class BashInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
public:
    BashInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "bash") {}
};

class NaturesSwiftnessTrigger : public BuffTrigger
{
public:
    NaturesSwiftnessTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "nature's swiftness") {}
};

class DruidPartyMemberRemoveCurseTrigger : public PartyMemberNeedCureTrigger
{
public:
    DruidPartyMemberRemoveCurseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "druid remove curse", DISPEL_CURSE)
    {
    }
};

class StarfallTrigger : public SpellNoCooldownTrigger
{
public:
    StarfallTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "starfall") {}
};

class ForceOfNatureTrigger : public BoostTrigger
{
public:
    ForceOfNatureTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "force of nature") {}
};

class MangleBearTrigger : public DebuffTrigger
{
public:
    MangleBearTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "mangle (bear)") {}

    bool IsActive() override
    {
        if (!bot->IsInCombat() || !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr))
            return false;
        Unit* target = GetTarget();
        return target && target->IsAlive() && target->IsInWorld();
    }
};

class LacerateTrigger : public DebuffTrigger
{
public:
    LacerateTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "lacerate") {}

    bool IsActive() override
    {
        if (!bot->IsInCombat() || !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr))
            return false;

        Unit* target = GetTarget();
        if (!target || !target->IsAlive() || !target->IsInWorld())
            return false;

        Aura* lacerate = botAI->GetAura("lacerate", target, false, false);
        if (!lacerate)
            return true;

        if (lacerate->GetStackAmount() < 5)
            return true;

        return lacerate->GetDuration() <= 6000;
    }
};

class DemoralizeRoarTrigger : public DebuffTrigger
{
public:
    DemoralizeRoarTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "demoralizing roar") {}

    bool IsActive() override
    {
        return DebuffTrigger::IsActive()
            && !botAI->HasAura("curse of weakness",  GetTarget(), false, false)
            && !botAI->HasAura("demoralizing shout", GetTarget(), false, false)
            && !botAI->HasAura("vindication",        GetTarget(), false, false);
    }
};

class MangleCatTrigger : public DebuffTrigger
{
public:
    MangleCatTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "mangle (cat)", 1, false, 0.0f) {}
    bool IsActive() override
    {
        if (botAI->HasAura("prowl", bot))
            return false;
        return DebuffTrigger::IsActive() && !botAI->HasAura("mangle (bear)", GetTarget(), false, false, -1, true)
            && !botAI->HasAura("trauma", GetTarget(), false, false, -1, true);
    }
};

class FerociousBiteTimeTrigger : public Trigger
{
public:
    FerociousBiteTimeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ferocious bite time") {}
    bool IsActive() override
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target)
            return false;

        uint8 cp = AI_VALUE2(uint8, "combo", "current target");
        if (cp < 5)
            return false;

        Aura* roar = botAI->GetAura("savage roar", bot);
        bool roarCheck = !roar || roar->GetDuration() > 10000;
        if (!roarCheck)
            return false;

        Aura* rip = botAI->GetAura("rip", target, true);
        bool ripCheck = !rip || rip->GetDuration() > 10000;
        if (!ripCheck)
            return false;

        return true;
    }
};

class FerociousBiteExecuteTrigger : public Trigger
{
public:
    FerociousBiteExecuteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ferocious bite execute") {}
    bool IsActive() override
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target || !target->IsAlive())
            return false;

        if (!botAI->HasSpell("ferocious bite"))
            return false;

        if (AI_VALUE2(uint8, "combo", "current target") < 1)
            return false;

        if (target->GetHealthPct() >= 25.0f)
            return false;

        if (target->GetHealth() >= 20000)
            return false;

        return true;
    }
};

class HurricaneChannelCheckTrigger : public Trigger
{
public:
    HurricaneChannelCheckTrigger(PlayerbotAI* botAI, uint32 minEnemies = 3)
        : Trigger(botAI, "hurricane channel check"), minEnemies(minEnemies)
    {
    }

    bool IsActive() override;

protected:
    uint32 minEnemies;
    static const std::set<uint32> HURRICANE_SPELL_IDS;
};

class NoHealerDpsStrategyTrigger : public Trigger
{
public:
    NoHealerDpsStrategyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no healer dps strategy") {}

    bool IsActive() override
    {
        return !botAI->HasStrategy("healer dps", BOT_STATE_COMBAT);
    }
};

// Blanketing and healer dps compete for a druid in tree form, so healer dps does not take effect
// for a grouped resto druid with tree form.
class HealerShouldAttackAndNotBlanketingTrigger : public HealerShouldAttackTrigger
{
public:
    HealerShouldAttackAndNotBlanketingTrigger(PlayerbotAI* botAI) : HealerShouldAttackTrigger(botAI) {}

    std::string const getName() override { return "healer should attack and not blanketing"; }
    bool IsActive() override;
};

class BlanketHotTrigger : public Trigger
{
public:
    BlanketHotTrigger(PlayerbotAI* botAI, std::string const& name, std::string const& spell)
        : Trigger(botAI, name), _spell(spell) {}

    Unit* GetTarget() override;
    bool IsActive() override;

private:
    std::string _spell;
};

class ProwlTrigger : public Trigger
{
public:
    ProwlTrigger(PlayerbotAI* botAI) : Trigger(botAI, "prowl") {}

    bool IsActive() override;
};

#endif
