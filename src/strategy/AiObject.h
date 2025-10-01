/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AIOBJECT_H
#define _PLAYERBOT_AIOBJECT_H

#include "Common.h"
#include "PlayerbotAIAware.h"

class AiObjectContext;
class ChatHelper;
class Player;
class PlayerbotAI;

class AiObject : public PlayerbotAIAware
{
public:
    AiObject(PlayerbotAI* botAI);

protected:
    Player* bot;
    Player* GetMaster();
    AiObjectContext* context;
    ChatHelper* chat;
};

class AiNamedObject : public AiObject
{
public:
    AiNamedObject(PlayerbotAI* botAI, std::string const name) : AiObject(botAI), name(name) {}

public:
    virtual std::string const getName() { return name; }

protected:
    std::string const name;
};

//
// TRIGGERS
//

#define NEXT_TRIGGERS(name, relevance) \
    virtual NextAction* getNextAction() { return new NextAction(name, relevance); }

#define BEGIN_TRIGGER(clazz, super)                 \
    class clazz : public super                      \
    {                                               \
    public:                                         \
        clazz(PlayerbotAI* botAI) : super(botAI) {} \
        bool IsActive() override;

#define END_TRIGGER() \
    }                 \
    ;

#define BUFF_TRIGGER(clazz, spell)                               \
    class clazz : public BuffTrigger                             \
    {                                                            \
    public:                                                      \
        clazz(PlayerbotAI* botAI) : BuffTrigger(botAI, spell) {} \
    }

#define BUFF_TRIGGER_A(clazz, spell)                             \
    class clazz : public BuffTrigger                             \
    {                                                            \
    public:                                                      \
        clazz(PlayerbotAI* botAI) : BuffTrigger(botAI, spell) {} \
        bool IsActive() override;                                \
    }

#define BUFF_PARTY_TRIGGER(clazz, spell)                                \
    class clazz : public BuffOnPartyTrigger                             \
    {                                                                   \
    public:                                                             \
        clazz(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, spell) {} \
    }

#define BUFF_PARTY_TRIGGER_A(clazz, spell)                              \
    class clazz : public BuffOnPartyTrigger                             \
    {                                                                   \
    public:                                                             \
        clazz(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, spell) {} \
        bool IsActive() override;                                       \
    }

#define DEBUFF_TRIGGER(clazz, spell)                               \
    class clazz : public DebuffTrigger                             \
    {                                                              \
    public:                                                        \
        clazz(PlayerbotAI* botAI) : DebuffTrigger(botAI, spell) {} \
    }

#define DEBUFF_CHECKISOWNER_TRIGGER(clazz, spell)                           \
    class clazz : public DebuffTrigger                                      \
    {                                                                       \
    public:                                                                 \
        clazz(PlayerbotAI* botAI) : DebuffTrigger(botAI, spell, 1, true) {} \
    }

#define DEBUFF_TRIGGER_A(clazz, spell)                             \
    class clazz : public DebuffTrigger                             \
    {                                                              \
    public:                                                        \
        clazz(PlayerbotAI* botAI) : DebuffTrigger(botAI, spell) {} \
        bool IsActive() override;                                  \
    }

#define DEBUFF_ENEMY_TRIGGER(clazz, spell)                                   \
    class clazz : public DebuffOnAttackerTrigger                             \
    {                                                                        \
    public:                                                                  \
        clazz(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, spell) {} \
    }

#define DEBUFF_ENEMY_TRIGGER_A(clazz, spell)                                 \
    class clazz : public DebuffOnAttackerTrigger                             \
    {                                                                        \
    public:                                                                  \
        clazz(PlayerbotAI* botAI) : DebuffOnAttackerTrigger(botAI, spell) {} \
        bool IsActive() override;                                            \
    }

#define CURE_TRIGGER(clazz, spell, dispel)                                   \
    class clazz : public NeedCureTrigger                                     \
    {                                                                        \
    public:                                                                  \
        clazz(PlayerbotAI* botAI) : NeedCureTrigger(botAI, spell, dispel) {} \
    }

#define CURE_PARTY_TRIGGER(clazz, spell, dispel)                                        \
    class clazz : public PartyMemberNeedCureTrigger                                     \
    {                                                                                   \
    public:                                                                             \
        clazz(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, spell, dispel) {} \
    }

#define CAN_CAST_TRIGGER(clazz, spell)                                     \
    class clazz : public SpellCanBeCastTrigger                             \
    {                                                                      \
    public:                                                                \
        clazz(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, spell) {} \
    }

#define CAN_CAST_TRIGGER_A(clazz, spell)                                   \
    class clazz : public SpellCanBeCastTrigger                             \
    {                                                                      \
    public:                                                                \
        clazz(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, spell) {} \
        bool IsActive() override;                                          \
    }

#define CD_TRIGGER(clazz, spell)                                            \
    class clazz : public SpellNoCooldownTrigger                             \
    {                                                                       \
    public:                                                                 \
        clazz(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, spell) {} \
    }

#define INTERRUPT_TRIGGER(clazz, spell)                                    \
    class clazz : public InterruptSpellTrigger                             \
    {                                                                      \
    public:                                                                \
        clazz(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, spell) {} \
    }

#define INTERRUPT_TRIGGER_A(clazz, spell)                                  \
    class clazz : public InterruptSpellTrigger                             \
    {                                                                      \
    public:                                                                \
        clazz(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, spell) {} \
        bool IsActive() override;                                          \
    }

#define HAS_AURA_TRIGGER(clazz, spell)                              \
    class clazz : public HasAuraTrigger                             \
    {                                                               \
    public:                                                         \
        clazz(PlayerbotAI* botAI) : HasAuraTrigger(botAI, spell) {} \
    }

#define HAS_AURA_TRIGGER_A(clazz, spell)                            \
    class clazz : public HasAuraTrigger                             \
    {                                                               \
    public:                                                         \
        clazz(PlayerbotAI* botAI) : HasAuraTrigger(botAI, spell) {} \
        bool IsActive() override;                                   \
    }

#define SNARE_TRIGGER(clazz, spell)                                     \
    class clazz : public SnareTargetTrigger                             \
    {                                                                   \
    public:                                                             \
        clazz(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, spell) {} \
    }

#define SNARE_TRIGGER_A(clazz, spell)                                   \
    class clazz : public SnareTargetTrigger                             \
    {                                                                   \
    public:                                                             \
        clazz(PlayerbotAI* botAI) : SnareTargetTrigger(botAI, spell) {} \
        bool IsActive() override;                                       \
    }

#define PROTECT_TRIGGER(clazz, spell)                                   \
    class clazz : public ProtectPartyMemberTrigger                      \
    {                                                                   \
    public:                                                             \
        clazz(PlayerbotAI* botAI) : ProtectPartyMemberTrigger(botAI) {} \
    }

#define DEFLECT_TRIGGER(clazz, spell)                                    \
    class clazz : public DeflectSpellTrigger                             \
    {                                                                    \
    public:                                                              \
        clazz(PlayerbotAI* botAI) : DeflectSpellTrigger(botAI, spell) {} \
    }

#define BOOST_TRIGGER(clazz, spell)                               \
    class clazz : public BoostTrigger                             \
    {                                                             \
    public:                                                       \
        clazz(PlayerbotAI* botAI) : BoostTrigger(botAI, spell) {} \
    }

#define BOOST_TRIGGER_A(clazz, spell)                             \
    class clazz : public BoostTrigger                             \
    {                                                             \
    public:                                                       \
        clazz(PlayerbotAI* botAI) : BoostTrigger(botAI, spell) {} \
        bool IsActive() override;                                 \
    }

#define INTERRUPT_HEALER_TRIGGER(clazz, spell)                                   \
    class clazz : public InterruptEnemyHealerTrigger                             \
    {                                                                            \
    public:                                                                      \
        clazz(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, spell) {} \
    }

#define INTERRUPT_HEALER_TRIGGER_A(clazz, spell)                                 \
    class clazz : public InterruptEnemyHealerTrigger                             \
    {                                                                            \
    public:                                                                      \
        clazz(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, spell) {} \
        bool IsActive() override;                                                \
    }

#define CC_TRIGGER(clazz, spell)                                        \
    class clazz : public HasCcTargetTrigger                             \
    {                                                                   \
    public:                                                             \
        clazz(PlayerbotAI* botAI) : HasCcTargetTrigger(botAI, spell) {} \
    }

//
// ACTIONS
//

#define MELEE_ACTION(clazz, spell)                                        \
    class clazz : public CastMeleeSpellAction                             \
    {                                                                     \
    public:                                                               \
        clazz(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, spell) {} \
    }

#define MELEE_ACTION_U(clazz, spell, useful)                              \
    class clazz : public CastMeleeSpellAction                             \
    {                                                                     \
    public:                                                               \
        clazz(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, spell) {} \
        bool isUseful() override { return useful; }                       \
    }

#define SPELL_ACTION(clazz, spell)                                   \
    class clazz : public CastSpellAction                             \
    {                                                                \
    public:                                                          \
        clazz(PlayerbotAI* botAI) : CastSpellAction(botAI, spell) {} \
    }

#define SPELL_ACTION_U(clazz, spell, useful)                         \
    class clazz : public CastSpellAction                             \
    {                                                                \
    public:                                                          \
        clazz(PlayerbotAI* botAI) : CastSpellAction(botAI, spell) {} \
        bool isUseful() override { return useful; }                  \
    }

#define HEAL_ACTION(clazz, spell)                                           \
    class clazz : public CastHealingSpellAction                             \
    {                                                                       \
    public:                                                                 \
        clazz(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, spell) {} \
    }

#define HEAL_ACTION_U(clazz, spell, useful)                                 \
    class clazz : public CastHealingSpellAction                             \
    {                                                                       \
    public:                                                                 \
        clazz(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, spell) {} \
        bool isUseful() override { return useful; }                         \
    }

#define HEAL_PARTY_ACTION(clazz, spell, estAmount, manaEfficiency)                                    \
    class clazz : public HealPartyMemberAction                                                        \
    {                                                                                                 \
    public:                                                                                           \
        clazz(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, spell, estAmount, manaEfficiency) {} \
    }

#define AOE_HEAL_ACTION(clazz, spell, estAmount, manaEfficiency)            \
    class clazz : public CastAoeHealSpellAction                             \
    {                                                                       \
    public:                                                                 \
        clazz(PlayerbotAI* botAI) : CastAoeHealSpellAction(botAI, spell) {} \
    }

#define BUFF_ACTION(clazz, spell)                                        \
    class clazz : public CastBuffSpellAction                             \
    {                                                                    \
    public:                                                              \
        clazz(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, spell) {} \
    }

#define BUFF_ACTION_U(clazz, spell, useful)                              \
    class clazz : public CastBuffSpellAction                             \
    {                                                                    \
    public:                                                              \
        clazz(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, spell) {} \
        bool isUseful() override { return useful; }                      \
    }

#define BUFF_PARTY_ACTION(clazz, spell)                                \
    class clazz : public BuffOnPartyAction                             \
    {                                                                  \
    public:                                                            \
        clazz(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, spell) {} \
    }

#define CURE_ACTION(clazz, spell)                                        \
    class clazz : public CastCureSpellAction                             \
    {                                                                    \
    public:                                                              \
        clazz(PlayerbotAI* botAI) : CastCureSpellAction(botAI, spell) {} \
    }

#define CURE_PARTY_ACTION(clazz, spell, dispel)                                    \
    class clazz : public CurePartyMemberAction                                     \
    {                                                                              \
    public:                                                                        \
        clazz(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, spell, dispel) {} \
    }

#define RESS_ACTION(clazz, spell)                                               \
    class clazz : public ResurrectPartyMemberAction                             \
    {                                                                           \
    public:                                                                     \
        clazz(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, spell) {} \
    }

#define DEBUFF_ACTION(clazz, spell)                                        \
    class clazz : public CastDebuffSpellAction                             \
    {                                                                      \
    public:                                                                \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, spell) {} \
    }

#define DEBUFF_CHECKISOWNER_ACTION(clazz, spell)                                 \
    class clazz : public CastDebuffSpellAction                                   \
    {                                                                            \
    public:                                                                      \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, spell, true) {} \
    }

#define DEBUFF_ACTION_U(clazz, spell, useful)                              \
    class clazz : public CastDebuffSpellAction                             \
    {                                                                      \
    public:                                                                \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, spell) {} \
        bool isUseful() override { return useful; }                        \
    }

#define DEBUFF_ACTION_R(clazz, spell, distance)                                               \
    class clazz : public CastDebuffSpellAction                                                \
    {                                                                                         \
    public:                                                                                   \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, spell) { range = distance; } \
    }

#define DEBUFF_ENEMY_ACTION(clazz, spell)                                            \
    class clazz : public CastDebuffSpellOnAttackerAction                             \
    {                                                                                \
    public:                                                                          \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, spell) {} \
    }

#define REACH_ACTION(clazz, spell, range)                                              \
    class clazz : public CastReachTargetSpellAction                                    \
    {                                                                                  \
    public:                                                                            \
        clazz(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, spell, range) {} \
    }

#define REACH_ACTION_U(clazz, spell, range, useful)                                    \
    class clazz : public CastReachTargetSpellAction                                    \
    {                                                                                  \
    public:                                                                            \
        clazz(PlayerbotAI* botAI) : CastReachTargetSpellAction(botAI, spell, range) {} \
        bool isUseful() override { return useful; }                                    \
    }

#define ENEMY_HEALER_ACTION(clazz, spell)                                         \
    class clazz : public CastSpellOnEnemyHealerAction                             \
    {                                                                             \
    public:                                                                       \
        clazz(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, spell) {} \
    }

#define SNARE_ACTION(clazz, spell)                                        \
    class clazz : public CastSnareSpellAction                             \
    {                                                                     \
    public:                                                               \
        clazz(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, spell) {} \
    }

#define CC_ACTION(clazz, spell)                                                  \
    class clazz : public CastCrowdControlSpellAction                             \
    {                                                                            \
    public:                                                                      \
        clazz(PlayerbotAI* botAI) : CastCrowdControlSpellAction(botAI, spell) {} \
    }

#define PROTECT_ACTION(clazz, spell)                                        \
    class clazz : public CastProtectSpellAction                             \
    {                                                                       \
    public:                                                                 \
        clazz(PlayerbotAI* botAI) : CastProtectSpellAction(botAI, spell) {} \
    }

#define END_RANGED_SPELL_ACTION() \
    }                             \
    ;

#define BEGIN_SPELL_ACTION(clazz, name)  \
    class clazz : public CastSpellAction \
    {                                    \
    public:                              \
        clazz(PlayerbotAI* botAI) : CastSpellAction(botAI, name) {}

#define END_SPELL_ACTION() \
    }                      \
    ;

#define BEGIN_DEBUFF_ACTION(clazz, name)       \
    class clazz : public CastDebuffSpellAction \
    {                                          \
    public:                                    \
        clazz(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, name) {}

#define BEGIN_RANGED_SPELL_ACTION(clazz, name) \
    class clazz : public CastSpellAction       \
    {                                          \
    public:                                    \
        clazz(PlayerbotAI* botAI) : CastSpellAction(botAI, name) {}

#define BEGIN_MELEE_SPELL_ACTION(clazz, name) \
    class clazz : public CastMeleeSpellAction \
    {                                         \
    public:                                   \
        clazz(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, name) {}

#define END_RANGED_SPELL_ACTION() \
    }                             \
    ;

#define BEGIN_BUFF_ON_PARTY_ACTION(clazz, name) \
    class clazz : public BuffOnPartyAction      \
    {                                           \
    public:                                     \
        clazz(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, name) {}

//
// Action node
//

// node_name , action, prerequisite
#define ACTION_NODE_P(name, spell, pre)                                                                       \
    static ActionNode* name([[maybe_unused]] PlayerbotAI* botAI)                                                               \
    {                                                                                                         \
        return new ActionNode(spell, /*P*/ NextAction::array(0, new NextAction(pre), nullptr), /*A*/ nullptr, \
                              /*C*/ nullptr);                                                                 \
    }

// node_name , action, alternative
#define ACTION_NODE_A(name, spell, alt)                                                                       \
    static ActionNode* name([[maybe_unused]] PlayerbotAI* botAI)                                                               \
    {                                                                                                         \
        return new ActionNode(spell, /*P*/ nullptr, /*A*/ NextAction::array(0, new NextAction(alt), nullptr), \
                              /*C*/ nullptr);                                                                 \
    }

// node_name , action, continuer
#define ACTION_NODE_C(name, spell, con)                                                  \
    static ActionNode* name([[maybe_unused]] PlayerbotAI* botAI)                                          \
    {                                                                                    \
        return new ActionNode(spell, /*P*/ nullptr, /*A*/ nullptr,                       \
                              /*C*/ NextAction::array(0, new NextAction(con), nullptr)); \
    }

#endif
