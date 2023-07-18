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

};

class KelthuzadBossHelper: public AiObject {
    public:
        std::pair<float, float> center = {3716.19f, -5106.58f};
        std::pair<float, float> tank_pos = {3709.19f, -5104.86f};
        std::pair<float, float> assist_tank_pos = {3746.05f, -5112.74f};
        KelthuzadBossHelper(PlayerbotAI *botAI): AiObject(botAI) {}

        bool UpdateBossAI() {
            Unit* target = AI_VALUE2(Unit*, "find target", "kel'thuzad");
            if (!target) {
                return false;
            }
            ai_ = dynamic_cast<boss_kelthuzad::boss_kelthuzadAI *>(target->GetAI());
            if (!ai_) {
                return false;
            }
            event_map_ = &ai_->events;
            if (!event_map_) {
                return false;
            }
            return true;
        }
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
    private:
        boss_kelthuzad::boss_kelthuzadAI *ai_;
        EventMap* event_map_;
};


#endif