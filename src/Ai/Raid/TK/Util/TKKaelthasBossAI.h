/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKKAELTHASBOSSAI_H
#define PLAYERBOTS_TKKAELTHASBOSSAI_H

#include "ScriptedCreature.h"

// This file mirrors the core's boss_kaelthas class so bots can read the encounter phase.
// The core's boss script is at src/server/scripts/Outland/TempestKeep/Eye/boss_kaelthas.cpp.
// IntroduceNewAdvisor and its two enums are not needed and are omitted.

struct boss_kaelthas : public BossAI
{
    boss_kaelthas(Creature* creature);

    void PrepareAdvisors();
    void SetRoomState(GOState state);
    void Reset() override;
    void AttackStart(Unit* who) override;
    void MoveInLineOfSight(Unit* who) override;
    void KilledUnit(Unit* victim) override;
    void JustSummoned(Creature* summon) override;
    void SpellHit(Unit* caster, SpellInfo const* spell) override;
    void MovementInform(uint32 type, uint32 point) override;
    void ExecuteMiddleEvent();
    void PhaseEnchantedWeaponsExecute();
    void PhaseAllAdvisorsExecute();
    void PhaseKaelExecute();
    void UpdateAI(uint32 diff) override;
    bool CheckEvadeIfOutOfCombatArea() const override;
    void JustDied(Unit* killer) override;

    uint32 GetPhase() const { return _phase; } // This is the only addition to the class.

private:
    uint32 _phase;
    uint8 _advisorsAlive;
    bool _transitionSceneReached = false;
};

#endif
