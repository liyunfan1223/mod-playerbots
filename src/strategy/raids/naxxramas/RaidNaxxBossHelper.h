#ifndef _PLAYERBOT_RAIDNAXXBOSSHELPER_H
#define _PLAYERBOT_RAIDNAXXBOSSHELPER_H

#include "AiObject.h"
#include "AiObjectContext.h"
#include "EventMap.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ScriptedCreature.h"
#include "RaidNaxxScripts.h"
#include "SharedDefines.h"

#include <string>

const uint32 NAXX_MAP_ID = 533;

template<class BossAiType>
class GenericBossHelper : public AiObject {
    public:
        GenericBossHelper(PlayerbotAI* botAI, std::string name): AiObject(botAI), name_(name) {}
        virtual bool UpdateBossAI() {
            if(unit_ && (!unit_->IsInWorld() || !unit_->IsAlive())) {
                unit_ = nullptr;
            }
            if (!unit_) {
                unit_ = AI_VALUE2(Unit*, "find target", name_);
                if (!unit_) {
                    return false;
                }
                target_ = unit_->ToCreature();
                if (!target_) {
                    return false;
                }
                ai_ = dynamic_cast<BossAiType *>(target_->GetAI());
                if (!ai_) {
                    return false;
                }
                event_map_ = &ai_->events;
                if (!event_map_) {
                    return false;
                }
            }
            timer_ = event_map_->GetTimer();
            return true;
        }
    protected:
        Unit* unit_ = nullptr;
        Creature* target_ = nullptr;
        std::string name_ = nullptr;
        BossAiType *ai_ = nullptr;
        EventMap* event_map_ = nullptr;
        uint32 timer_ = 0;
};

class KelthuzadBossHelper: public GenericBossHelper<boss_kelthuzad::boss_kelthuzadAI> {
    public:
        KelthuzadBossHelper(PlayerbotAI *botAI): GenericBossHelper(botAI, "kel'thuzad") {}
        const std::pair<float, float> center = {3716.19f, -5106.58f};
        const std::pair<float, float> tank_pos = {3709.19f, -5104.86f};
        const std::pair<float, float> assist_tank_pos = {3746.05f, -5112.74f};
        bool IsPhaseOne() {
            return event_map_->GetNextEventTime(KELTHUZAD_EVENT_PHASE_2) != 0;
        }
        bool IsPhaseTwo() {
            return !IsPhaseOne();
        }
        Unit* GetAnyShadowFissure() {
            Unit* shadow_fissure = nullptr;
            GuidVector units = *context->GetValue<GuidVector>("nearest triggers");
            for (auto i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = botAI->GetUnit(*i);
                if (!unit)
                    continue;
                if (botAI->EqualLowercaseName(unit->GetName(), "shadow fissure")) {
                    shadow_fissure = unit;
                }
            }
            return shadow_fissure;
        }
};

class RazuviousBossHelper: public GenericBossHelper<boss_razuvious::boss_razuviousAI> {
    public:
        RazuviousBossHelper(PlayerbotAI *botAI): GenericBossHelper(botAI, "instructor razuvious") {}
};

class SapphironBossHelper: public GenericBossHelper<boss_sapphiron::boss_sapphironAI> {
    public:
        const std::pair<float, float> mainTankPos = {3512.07f, -5274.06f};
        const std::pair<float, float> center = {3517.31f, -5253.74f};
        const float GENERIC_HEIGHT = 137.29f;
        SapphironBossHelper(PlayerbotAI *botAI): GenericBossHelper(botAI, "sapphiron") {}
        bool UpdateBossAI() override {
            if (!GenericBossHelper::UpdateBossAI()) {
                return false;
            }
            uint32 nextEventGround = event_map_->GetNextEventTime(EVENT_GROUND);
            if (nextEventGround && nextEventGround != lastEventGround)
                lastEventGround = nextEventGround;
            return true;
        }
        bool IsPhaseGround() {
            return target_->GetReactState() == REACT_AGGRESSIVE;
        }
        bool IsPhaseFlight() {
            return !IsPhaseGround();
        }
        bool JustLanded() {
            return (event_map_->GetNextEventTime(EVENT_FLIGHT_START) - timer_) >= EVENT_FLIGHT_INTERVAL - POSITION_TIME_AFTER_LANDED;
        }
        bool WaitForExplosion() {
            return event_map_->GetNextEventTime(EVENT_FLIGHT_SPELL_EXPLOSION);
        }
        bool FindPosToAvoidChill(std::vector<float> &dest) {
            Aura* aura = botAI->GetAura("chill", bot);
            if (!aura) {
                return false;
            }
            DynamicObject* dyn_obj = aura->GetDynobjOwner();
            if (!dyn_obj) {
                return false;
            }
            Unit* currentTarget = AI_VALUE(Unit*, "current target");
            float angle = 0;
            uint32 index = botAI->GetGroupSlotIndex(bot);
            if (currentTarget) {
                if (botAI->IsRanged(bot)) {
                    if (bot->GetExactDist2d(currentTarget) <= 45.0f) {
                        angle = bot->GetAngle(dyn_obj) - M_PI + (rand_norm() - 0.5) * M_PI / 2;
                    } else {
                    if (index % 2 == 0) {
                            angle = bot->GetAngle(currentTarget) + M_PI / 2;
                        } else {
                            angle = bot->GetAngle(currentTarget) - M_PI / 2;    
                        }
                    }
                } else {
                    if (index % 3 == 0) {
                        angle = bot->GetAngle(currentTarget);
                    } else if (index % 3 == 1) {
                        angle = bot->GetAngle(currentTarget) + M_PI / 2;
                    } else {
                        angle = bot->GetAngle(currentTarget) - M_PI / 2;
                    }
                }
            } else {
                angle = bot->GetAngle(dyn_obj) - M_PI + (rand_norm() - 0.5) * M_PI / 2;
            }
            dest = {bot->GetPositionX() + cos(angle) * 5.0f, bot->GetPositionY() + sin(angle) * 5.0f, bot->GetPositionZ()};
            return true;
        }
    private:
        const uint32 POSITION_TIME_AFTER_LANDED = 5000;
        const uint32 EVENT_FLIGHT_INTERVAL = 45000;
        uint32 lastEventGround = 0;    
};

class GluthBossHelper: public GenericBossHelper<boss_gluth::boss_gluthAI> {
    public:
        const std::pair<float, float> mainTankPos25 = {3331.48f, -3109.06f};
        const std::pair<float, float> mainTankPos10 = {3278.29f, -3162.06f};
        const std::pair<float, float> beforeDecimatePos = {3267.34f, -3175.68f};
        const std::pair<float, float> leftSlowDownPos = {3290.68f, -3141.65f};
        const std::pair<float, float> rightSlowDownPos = {3300.78f, -3151.98f};
        const std::pair<float, float> rangedPos = {3301.45f, -3139.29f};
        const std::pair<float, float> healPos = {3303.09f, -3135.24f};
        
        const float decimatedZombiePct = 10.0f;
        GluthBossHelper(PlayerbotAI *botAI): GenericBossHelper(botAI, "gluth") {}
        bool BeforeDecimate() {
            uint32 decimate = event_map_->GetNextEventTime(GLUTH_EVENT_DECIMATE);
            return decimate && decimate - timer_ <= 3000;
        }
        bool JustStartCombat() {
            return timer_ < 10000;
        }
};

#endif