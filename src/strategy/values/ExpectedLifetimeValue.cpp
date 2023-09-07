#include "ExpectedLifetimeValue.h"
#include "Playerbots.h"
#include "SharedDefines.h"

float ExpectedLifetimeValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (!target || !target->IsAlive()) {
        return 0.0f;
    }
    float dps = AI_VALUE(float, "expected group dps");
    float res = target->GetHealth() / dps;
    // bot->Say(target->GetName() + " lifetime: " + std::to_string(res), LANG_UNIVERSAL);
    return res;
}

float ExpectedGroupDpsValue::Calculate()
{
    float dps_num;
    Group* group = bot->GetGroup();
    if (!group) {
        dps_num = 1;
    } else {
        dps_num = group->GetMembersCount() * 0.7;
    }
    // efficiency record based on rare gear level, is there better calculation method?
    float dps_efficiency = 1;
    if (bot->GetLevel() < 30) {
        dps_efficiency = 1.5;
    } else if (bot->GetLevel() < 40) {
        dps_efficiency = 2;
    } else if (bot->GetLevel() < 50) {
        dps_efficiency = 3;
    } else if (bot->GetLevel() < 60) {
        dps_efficiency = 4;
    } else if (bot->GetLevel() < 70) {
        dps_efficiency = 7;
    } else if (bot->GetLevel() < 80) {
        dps_efficiency = 12;
    } else {
        dps_efficiency = 25;
    }
    return dps_num * bot->GetLevel() * dps_efficiency;
}