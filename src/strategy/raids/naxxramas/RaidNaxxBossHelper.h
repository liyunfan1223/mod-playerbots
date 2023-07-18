#ifndef _PLAYERBOT_RAIDNAXXBOSSHELPER_H
#define _PLAYERBOT_RAIDNAXXBOSSHELPER_H

#include "AiObject.h"
#include "AiObjectContext.h"
#include "EventMap.h"
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
        bool UpdateBossAI() {
            Unit* target = AI_VALUE2(Unit*, "find target", name_);
            if (!target) {
                return false;
            }
            ai_ = dynamic_cast<BossAiType *>(target->GetAI());
            if (!ai_) {
                return false;
            }
            event_map_ = &ai_->events;
            if (!event_map_) {
                return false;
            }
            return true;
        }
    protected:
        std::string name_;
        BossAiType *ai_;
        EventMap* event_map_;
};

class KelthuzadBossHelper: public GenericBossHelper<boss_kelthuzad::boss_kelthuzadAI> {
    public:
        KelthuzadBossHelper(PlayerbotAI *botAI): GenericBossHelper(botAI, "kel'thuzad") {}
        std::pair<float, float> center = {3716.19f, -5106.58f};
        std::pair<float, float> tank_pos = {3709.19f, -5104.86f};
        std::pair<float, float> assist_tank_pos = {3746.05f, -5112.74f};
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

#endif