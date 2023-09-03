
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RaidNaxxActions.h"
#include "RaidNaxxStrategy.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "RaidNaxxBossHelper.h"

// bool TryToGetBossAIAction::Execute(Event event) {
//     Unit* boss = AI_VALUE(Unit*, "boss target");
//     if (!boss) {
//         return false;
//     }
//     Creature* creature = boss->ToCreature();
//     BossAI* b_ai = dynamic_cast<BossAI*>(boss->GetAI());
//     if (!b_ai) {
//         return false;
//     }
//     bool enabled = boss->IsAIEnabled;
//     EventMap *eventMap = b_botAI->GetEvents();
//     uint8 phase_mask = eventMap->GetPhaseMask();
//     uint32 bossID = b_botAI->GetBossID();
//     bool isDungeonBoss = creature->IsDungeonBoss();
//     // bot->Yell("boss ai detected.", LANG_UNIVERSAL);
//     uint32 next_event_time_1 = eventMap->GetNextEventTime(1);
//     uint32 next_event_time_2 = eventMap->GetNextEventTime(2);
//     uint32 next_event_time_3 = eventMap->GetNextEventTime(3);
//     uint32 next_event_time_4 = eventMap->GetNextEventTime(4);
//     uint32 next_event_time_5 = eventMap->GetNextEventTime(5);
//     uint32 curr_timer = eventMap->GetTimer();
//     bot->Yell("boss ai detected. phase mask: " + std::to_string(phase_mask) + " ai enabled: " + std::to_string(enabled) +
//                 " boss id: " + std::to_string(bossID) + " name: " + boss->GetName() + " entry: " + std::to_string(boss->GetEntry()) +
//                 " guid: " + std::to_string(boss->GetGUID().GetRawValue()) + " isDungeonBoss: " + std::to_string(isDungeonBoss) +
//                 " event1: " + std::to_string(next_event_time_1) + " event2: " + std::to_string(next_event_time_2) +
//                 " event3: " + std::to_string(next_event_time_3) + " event4: " + std::to_string(next_event_time_4) +
//                 " event5: " + std::to_string(next_event_time_5) + " timer: " + std::to_string(curr_timer), LANG_UNIVERSAL); 
//     return true;
    
// }

bool GrobbulusGoBehindAction::Execute(Event event)
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss) {
        return false;
    }
    // Position* pos = boss->GetPosition();
    float orientation = boss->GetOrientation() + M_PI + delta_angle;
    float x = boss->GetPositionX();
    float y = boss->GetPositionY();
    float z = boss->GetPositionZ();
    float rx = x + cos(orientation) * distance;
    float ry = y + sin(orientation) * distance;
    return MoveTo(bot->GetMapId(), rx, ry, z);
}

// bool MoveToPointForceAction::Execute(Event event)
// {
//     return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ(), true);
// }





uint32 RotateAroundTheCenterPointAction::FindNearestWaypoint()
{
    float minDistance = 0;
    int ret = -1;
    for (int i = 0; i < intervals; i++) {
        float w_x = waypoints[i].first, w_y = waypoints[i].second;
        float dis = bot->GetDistance2d(w_x, w_y);
        if (ret == -1 || dis < minDistance) {
            ret = i;
            minDistance = dis;
        }
    }
    return ret;
}

uint32 GrobbulusRotateAction::GetCurrWaypoint()
{
    Unit* boss = AI_VALUE(Unit*, "boss target");
    if (!boss) {
        return false;
    }
    auto* boss_ai = dynamic_cast<boss_grobbulus::boss_grobbulusAI*>(boss->GetAI());
    EventMap* eventMap = &boss_ai->events;
    const uint32 event_time = eventMap->GetNextEventTime(2);
    return (event_time / 15000) % intervals;
}

bool HeiganDanceAction::CalculateSafe() {
    Unit* boss = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!boss) {
        return false;
    }
    boss_heigan::boss_heiganAI* boss_ai = dynamic_cast<boss_heigan::boss_heiganAI*>(boss->GetAI());
    EventMap* eventMap = &boss_ai->events;
    uint32 curr_phase = boss_ai->currentPhase;
    uint32 curr_erupt = eventMap->GetNextEventTime(3);
    uint32 curr_dance = eventMap->GetNextEventTime(4);
    uint32 curr_timer = eventMap->GetTimer();
    if ((curr_phase == 0 && curr_dance - curr_timer >= 85000) || (curr_phase == 1 && curr_dance - curr_timer >= 40000)) {
        ResetSafe();
    } else if (curr_erupt != prev_erupt) {
        NextSafe();
    }
    prev_phase = curr_phase;
    prev_erupt = curr_erupt;
    return true;
}

bool HeiganDanceMeleeAction::Execute(Event event) {
    CalculateSafe();
    if (prev_phase == 0 && botAI->IsMainTank(bot) && !AI_VALUE2(bool, "has aggro", "boss target")) {
        return false;
    }
    assert(curr_safe >= 0 && curr_safe <= 3);
    return MoveInside(bot->GetMapId(), waypoints[curr_safe].first, waypoints[curr_safe].second, bot->GetPositionZ(), botAI->IsMainTank(bot) ? 0 : 0);
}

bool HeiganDanceRangedAction::Execute(Event event) {
    CalculateSafe();
    if (prev_phase != 1) {
        return MoveTo(bot->GetMapId(), platform.first, platform.second, 276.54f);
    }
    botAI->InterruptSpell();
    return MoveInside(bot->GetMapId(), waypoints[curr_safe].first, waypoints[curr_safe].second, bot->GetPositionZ(), 0);
}

// bool ThaddiusAttackNearestPetAction::isUseful()
// {
//     Unit* feugen = AI_VALUE2(Unit*, "find target", "feugen");
//     Unit* stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
//     if (!feugen || !stalagg) {
//         return false;
//     }
//     Unit* result = feugen;
//     if (stalagg && (!result || (bot->GetDistance2d(result) > bot->GetDistance2d(stalagg)))) {
//         result = stalagg;
//     }
//     if (AI_VALUE(Unit*, "current target") == result) {
//         return false;
//     }
//     return true;
// }

// bool ThaddiusAttackNearestPetAction::Execute(Event event) {
//     Unit* feugen = AI_VALUE2(Unit*, "find target", "feugen");
//     Unit* stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
//     Unit* result = feugen;
//     if (stalagg && (!result || (bot->GetDistance2d(result) > bot->GetDistance2d(stalagg)))) {
//         result = stalagg;
//     }
//     // if (result) {
//         // bot->Yell("Goona attack " + result->GetName(), LANG_UNIVERSAL);
//     // }
//     return Attack(result);
// }

// bool ThaddiusMeleeToPlaceAction::isUseful()
// {
//     if (botAI->IsTank(bot)) {
//         Unit* target = AI_VALUE(Unit*, "current target");
//         return target && target->GetVictim() == bot;
//     } else {
//         return !botAI->IsRanged(bot);
//     }
// }

// bool ThaddiusMeleeToPlaceAction::Execute(Event event)
// {
//     Unit* target = AI_VALUE(Unit*, "current target");
//     Unit* feugen = AI_VALUE2(Unit*, "find target", "feugen");
//     Unit* stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
//     // float high_lx = 3455.14f, high_ly = -2927.48f;
//     // float high_rx = 3515.00f, high_ry = -2983.09f;
//     float high_lx = 3436.14f, high_ly = -2919.98f;
//     float high_rx = 3522.94f, high_ry = -3002.60f;
//     float high_z = 312.61f;
//     if (target == feugen) {
//         return MoveTo(bot->GetMapId(), high_rx, high_ry, high_z);
//     } else if (target == stalagg) {
//         return MoveTo(bot->GetMapId(), high_lx, high_ly, high_z);
//     }
//     return false;
// }

// bool ThaddiusRangedToPlaceAction::isUseful()
// {
//     return botAI->IsRanged(bot);
// }

// bool ThaddiusRangedToPlaceAction::Execute(Event event)
// {
//     Unit* target = AI_VALUE(Unit*, "current target");
//     Unit* feugen = AI_VALUE2(Unit*, "find target", "feugen");
//     Unit* stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
//     float high_lx = 3441.01f, high_ly = -2942.04f;
//     float high_rx = 3500.45f, high_ry = -2997.92f;
//     float high_z = 312.61f;
//     if (target == feugen) {
//         return MoveTo(bot->GetMapId(), high_rx, high_ry, high_z);
//     } else if (target == stalagg) {
//         return MoveTo(bot->GetMapId(), high_lx, high_ly, high_z);
//     }
//     return false;
// }

// bool ThaddiusMoveToPlatformAction::isUseful()
// {
//     // float high_z = 312.0f;
//     // if (bot->GetPositionZ() < (high_z - 3.0f)) {
//     //     return false;
//     // }
//     return true;
// }

// bool ThaddiusMoveToPlatformAction::Execute(Event event) {
//     std::vector<std::pair<float, float>> position = {
//         // high left
//         {3462.99f, -2918.90f},
//         // high right
//         {3520.65f, -2976.51f},
//         // low left
//         {3471.36f, -2910.65f},
//         // low right
//         {3528.80f, -2967.04f},
//         // center
//         {3512.19f, -2928.58f},
//     };
//     // float high_lx = 3462.99f, high_ly = -2918.90f;
//     // float high_rx = 3520.65f, high_ly = -2976.51f;
//     // float low_lx = 3471.36f, low_ly = -2910.65f;
//     // float low_rx = 3528.80f, low_ry = -2967.04f;
//     float high_z = 312.00f, low_z = 304.02f;
//     bool is_left = bot->GetDistance2d(position[0].first, position[0].second) < bot->GetDistance2d(position[1].first, position[1].second);
//     if (bot->GetPositionZ() >= (high_z - 3.0f)) {
//         if (is_left) {
//             if (!MoveTo(bot->GetMapId(), position[0].first, position[0].second, high_z)) {
//                 bot->TeleportTo(bot->GetMapId(), position[2].first, position[2].second, low_z, bot->GetOrientation());
//             }
//         } else {
//             if (!MoveTo(bot->GetMapId(), position[1].first, position[1].second, high_z)) {
//                 bot->TeleportTo(bot->GetMapId(), position[3].first, position[3].second, low_z, bot->GetOrientation());
//             }
//         }
//     } else {
//         return MoveTo(bot->GetMapId(), position[4].first, position[4].second, low_z);
//     }
//     return true;
// }

// bool ThaddiusMovePolarityAction::isUseful() {
//     return !botAI->IsMainTank(bot) || AI_VALUE2(bool, "has aggro", "current target");
// }

// bool ThaddiusMovePolarityAction::Execute(Event event) {
//     std::vector<std::pair<float, float>> position = {
//         // left melee
//         {3508.29f, -2920.12f},
//         // left ranged
//         {3501.72f, -2913.36f},
//         // right melee
//         {3519.74f, -2931.69f},
//         // right ranged
//         {3524.32f, -2936.26f},
//         // center melee
//         {3512.19f, -2928.58f},
//         // center ranged
//         {3504.68f, -2936.68f},
//     };
//     uint32 idx;
//     if (botAI->HasAuraWithDuration("negative charge", bot)) {
//         idx = 0;
//     } else if (botAI->HasAuraWithDuration("positive charge", bot)) {
//         idx = 1;
//     } else {
//         idx = 2;
//     }
//     idx = idx * 2 + botAI->IsRanged(bot);
//     return MoveTo(bot->GetMapId(), position[idx].first, position[idx].second, bot->GetPositionZ());
// }

bool RazuviousUseObedienceCrystalAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    // bot->GetCharm
    if (Unit* charm = bot->GetCharm()) {
        Unit* target = AI_VALUE2(Unit*, "find target", "instructor razuvious");
        if (!target) {
            return false;
        }
        if (charm->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_ACTIVE) == NULL_MOTION_TYPE) {
            charm->GetMotionMaster()->Clear();
            charm->GetMotionMaster()->MoveChase(target);
            charm->GetAI()->AttackStart(target);
        }
        Aura* forceObedience = botAI->GetAura("force obedience", charm);
        uint32 duration_time;
        if (!forceObedience) {
            forceObedience = botAI->GetAura("mind control", charm);
            duration_time = 60000;
        } else {
            duration_time = 90000;
        }
        if (!forceObedience) {
            return false;
        }
        if (charm->GetDistance(target) <= 0.51f) {
            // taunt
            bool tauntUseful = true;
            if (forceObedience->GetDuration() <= (duration_time - 5000)) {
                if (target->GetVictim() && botAI->HasAura(29061, target->GetVictim())) {
                    tauntUseful = false;
                }
                if (forceObedience->GetDuration() <= 3000) {
                    tauntUseful = false;
                }
            }
            if (forceObedience->GetDuration() >= (duration_time - 500)) {
                tauntUseful = false;
            }
            if ( tauntUseful && !charm->HasSpellCooldown(29060) ) {
                // shield
                if (!charm->HasSpellCooldown(29061)) {
                    charm->CastSpell(charm, 29061, true);
                    charm->AddSpellCooldown(29061, 0, 30 * 1000);
                }
                charm->CastSpell(target, 29060, true);
                charm->AddSpellCooldown(29060, 0, 20 * 1000);
            }
            // strike
            if (!charm->HasSpellCooldown(61696)) {
                charm->CastSpell(target, 61696, true);
                charm->AddSpellCooldown(61696, 0, 4 * 1000);
            }
        }
    } else {
        Difficulty diff = bot->GetRaidDifficulty();
        if (diff == RAID_DIFFICULTY_10MAN_NORMAL) {
            GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
            for (auto i = npcs.begin(); i != npcs.end(); i++)
            {
                Creature* unit = botAI->GetCreature(*i);
                if (!unit) {
                    continue;
                }
                if (botAI->IsMainTank(bot) && unit->GetSpawnId() != 128352) {
                    continue;
                }
                if (!botAI->IsMainTank(bot) && unit->GetSpawnId() != 128353) {
                    continue;
                }
                if (MoveTo(unit)) {
                    return true;
                }
                Creature *creature = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_SPELLCLICK);
                if (!creature)
                    continue;
                creature->HandleSpellClick(bot);
                return true;
            }
        } else {
            GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
            Unit* target = nullptr;
            for (auto i = attackers.begin(); i != attackers.end(); ++i) {
                Unit* unit = botAI->GetUnit(*i);
                if (!unit)
                    continue;
                if (botAI->EqualLowercaseName(unit->GetName(), "death knight understudy")) {
                    target = unit;
                    break;
                }
            }
            if (target) {
                if (bot->GetDistance2d(target) > sPlayerbotAIConfig->spellDistance) {
                    return MoveNear(target, sPlayerbotAIConfig->spellDistance);
                } else {
                    return botAI->CastSpell("mind control", target);
                }
            }
        }
    }
    return false;
}

bool RazuviousTargetAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    Unit* razuvious = AI_VALUE2(Unit*, "find target", "instructor razuvious");
    Unit* understudy = AI_VALUE2(Unit*, "find target", "death knight understudy");
    Unit* target = nullptr;
    if (botAI->IsTank(bot)) {
        target = understudy;
    } else {
        target = razuvious;
    }
    if (AI_VALUE(Unit*, "current target") == target) {
        return false;
    }
    return Attack(target);
}

// bool HorsemanAttractAlternativelyAction::Execute(Event event)
// {
//     Unit* sir = AI_VALUE2(Unit*, "find target", "sir zeliek");
//     Unit* lady = AI_VALUE2(Unit*, "find target", "lady blaumeux");
//     bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
//     if (!sir) {
//         return false;
//     }
//     std::vector<std::pair<float, float>> position = {
//         // left (sir zeliek)
//         {2502.03f, -2910.90f},
//         // right (lady blaumeux)
//         {2484.61f, -2947.07f},
//     };
//     float pos_z = 241.27f;
//     BossAI* boss_ai = dynamic_cast<BossAI*>(sir->GetAI());
//     EventMap* eventMap = boss_botAI->GetEvents();
//     const uint32 timer = eventMap->GetTimer();
//     if (lady) {
//         BossAI* lady_ai = dynamic_cast<BossAI*>(lady->GetAI());
//         EventMap* ladyEventMap = lady_botAI->GetEvents();
//         const uint32 voidZone = ladyEventMap->GetNextEventTime(5);
//         if (!voidZone) {
//             voidzone_counter = 0;
//         }
//         if (voidZone && last_voidzone != voidZone) {
//             voidzone_counter = (voidzone_counter + 1) % 8;
//         }
//         last_voidzone = voidZone;
//     }
//     int pos_to_go;
//     if (!lady) {
//         pos_to_go = 0;
//     } else {
//         // 24 - 15 - 15 - ...
//         if (timer <= 9000 || ((timer - 9000) / 67500) % 2 == 0) {
//             pos_to_go = 0;
//         } else {
//             pos_to_go = 1;
//         }
//         if (botAI->IsRangedDpsAssistantOfIndex(bot, 0) || (raid25 && botAI->IsHealAssistantOfIndex(bot, 1))) {
//             pos_to_go = 1 - pos_to_go;
//         }
//     }
//     // bot->Yell("pos to go: " + std::to_string(pos_to_go), LANG_UNIVERSAL);
//     float pos_x = position[pos_to_go].first, pos_y = position[pos_to_go].second;
//     if (pos_to_go == 1) {
//         float offset_x;
//         float offset_y;
//         if (voidzone_counter < 4) {
//             offset_x = voidzone_counter * (-4.5f);
//             offset_y = voidzone_counter * (4.5f);
//         }
//         if (voidzone_counter >= 4) {
//             offset_x = (7 - voidzone_counter) * (-4.5f);
//             offset_y = (7 - voidzone_counter) * (4.5f);
//             offset_x += 4.5f;
//             offset_y += 4.5f;
//         }
//         pos_x += offset_x;
//         pos_y += offset_y;
//     }
//     if (MoveTo(bot->GetMapId(), pos_x, pos_y, pos_z)) {
//         return true;
//     }
//     Unit* attackTarget;
//     if (pos_to_go == 0) {
//         attackTarget = sir;
//     } else {
//         attackTarget = lady;
//     }
//     if (context->GetValue<Unit*>("current target")->Get() != attackTarget) {
//         return Attack(attackTarget);
//     }
//     return false;
// }

// bool HorsemanAttactInOrderAction::Execute(Event event)
// {
//     Unit* target = nullptr;
//     Unit* thane = AI_VALUE2(Unit*, "find target", "thane korth'azz");
//     Unit* baron = AI_VALUE2(Unit*, "find target", "baron rivendare");
//     Unit* lady = AI_VALUE2(Unit*, "find target", "lady blaumeux");
//     Unit* sir = AI_VALUE2(Unit*, "find target", "sir zeliek");
//     vector<Unit*> attack_order;
//     if (botAI->IsAssistTank(bot)) {
//         attack_order = {baron, thane, lady, sir};
//     } else {
//         attack_order = {thane, baron, lady, sir};
//     }
//     for (Unit* t : attack_order) {
//         if (t) {
//             target = t;
//             break;
//         }
//     }
//     if (target) {
//         if (context->GetValue<Unit*>("current target")->Get() == target && botAI->GetCurrentState() == BOT_STATE_COMBAT) {
//             return false;
//         }
//         if (!bot->IsWithinLOSInMap(target)) {
//             return MoveNear(target, 10.0f);
//         }
//         return Attack(target);
//     }
//     return false;
// }

// bool SapphironGroundMainTankPositionAction::Execute(Event event)
// {
//     return MoveTo(533, 3512.07f, -5274.06f, 137.29f);
//     // return MoveTo(533, 3498.58f, -5245.35f, 137.29f);
// }

bool SapphironGroundPositionAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    if (botAI->IsMainTank(bot)) {
        if (AI_VALUE2(bool, "has aggro", "current target")) {
            return MoveTo(NAXX_MAP_ID, helper.mainTankPos.first, helper.mainTankPos.second, helper.GENERIC_HEIGHT);
        }
        return false;
    } 
    if (helper.JustLanded()) {
        uint32 index = botAI->GetGroupSlotIndex(bot);
        float start_angle = 0.85 * M_PI;
        float offset_angle = M_PI * 0.02 * index;
        float angle = start_angle + offset_angle;
        float distance;
        if (botAI->IsRanged(bot)) {
            distance = 35.0f;
        } else if (botAI->IsHeal(bot)) {
            distance = 30.0f;
        } else {
            distance = 5.0f;
        }
        return MoveTo(NAXX_MAP_ID, helper.center.first + cos(angle) * distance, helper.center.second + sin(angle) * distance, helper.GENERIC_HEIGHT);
    } else {
        std::vector<float> dest;
        if (helper.FindPosToAvoidChill(dest)) {
            return MoveTo(NAXX_MAP_ID, dest[0], dest[1], dest[2]);
        }
    }
    return false;
}

bool SapphironFlightPositionAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    if (helper.WaitForExplosion()) {
        return MoveToNearestIcebolt();
    } else {
        std::vector<float> dest;
        if (helper.FindPosToAvoidChill(dest)) {
            return MoveTo(NAXX_MAP_ID, dest[0], dest[1], dest[2]);
        }
    }
    return false;
}

bool SapphironFlightPositionAction::MoveToNearestIcebolt()
{
    Group* group = bot->GetGroup();
    if (!group) {
        return false;
    }
    Group::MemberSlotList const& slots = group->GetMemberSlots();
    Player* playerWithIcebolt = nullptr;
    float minDistance;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next()) {
        Player* member = ref->GetSource();
        if (botAI->HasAura("icebolt", member, false, false, -1, true)) {
            if (!playerWithIcebolt || minDistance > bot->GetDistance(member)) {
                playerWithIcebolt = member;
                minDistance = bot->GetDistance(member);
            }
        }
    }
    if (playerWithIcebolt) {
        Unit* boss = AI_VALUE2(Unit*, "find target", "sapphiron");
        if (boss) {
            float angle = boss->GetAngle(playerWithIcebolt);
            return MoveTo(NAXX_MAP_ID, playerWithIcebolt->GetPositionX() + cos(angle) * 3.0f, playerWithIcebolt->GetPositionY() + sin(angle) * 3.0f, helper.GENERIC_HEIGHT);
        }
    }
    return false;
}

// bool SapphironAvoidChillAction::Execute(Event event)
// {
//     Aura* aura = botAI->GetAura("chill", bot);
//     if (!aura) return false;
//     DynamicObject* dyn_obj = aura->GetDynobjOwner();
//     if (!dyn_obj) return false;
//     Unit* currentTarget = AI_VALUE(Unit*, "current target");
//     float angle = 0;
//     uint32 index = botAI->GetGroupSlotIndex(bot);
//     // prevent seg fault
//     if (currentTarget) {
//         if (botAI->IsRanged(bot)) {
//             if (bot->GetExactDist2d(currentTarget) <= 45.0f) {
//                 angle = bot->GetAngle(dyn_obj) - M_PI + (rand_norm() - 0.5) * M_PI / 2;
//             } else {
//             if (index % 2 == 0) {
//                     angle = bot->GetAngle(currentTarget) + M_PI / 2;
//                 } else {
//                     angle = bot->GetAngle(currentTarget) - M_PI / 2;    
//                 }
//             }
//         } else {
//             if (index % 3 == 0) {
//                 angle = bot->GetAngle(currentTarget);
//             } else if (index % 3 == 1) {
//                 angle = bot->GetAngle(currentTarget) + M_PI / 2;
//             } else {
//                 angle = bot->GetAngle(currentTarget) - M_PI / 2;
//             }
//         }
//     } else {
//         angle = bot->GetAngle(dyn_obj) - M_PI + (rand_norm() - 0.5) * M_PI / 2;
//     }
//     // }
//     return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * 5.0f, bot->GetPositionY() + sin(angle) * 5.0f, bot->GetPositionZ());
// }

bool KelthuzadChooseTargetAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    Unit* target = nullptr;
    Unit *target_soldier = nullptr, *target_weaver = nullptr, *target_abomination = nullptr, *target_kelthuzad = nullptr, *target_guardian = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        if (botAI->EqualLowercaseName(unit->GetName(), "guardian of icecrown")) {
            if (!target_guardian) {
                target_guardian = unit;
            } else if (unit->GetVictim() && target_guardian->GetVictim() && 
                       unit->GetVictim()->ToPlayer() && target_guardian->GetVictim()->ToPlayer() && 
                       !botAI->IsAssistTank(unit->GetVictim()->ToPlayer()) && botAI->IsAssistTank(target_guardian->GetVictim()->ToPlayer())) {
                target_guardian = unit;
            } else if (unit->GetVictim() && target_guardian->GetVictim() && 
                       unit->GetVictim()->ToPlayer() && target_guardian->GetVictim()->ToPlayer() && 
                       !botAI->IsAssistTank(unit->GetVictim()->ToPlayer()) && !botAI->IsAssistTank(target_guardian->GetVictim()->ToPlayer()) &&
                       target_guardian->GetDistance2d(helper.center.first, helper.center.second) > bot->GetDistance2d(unit)) {
                target_guardian = unit;
            }
        }
        
        if (unit->GetDistance2d(helper.center.first, helper.center.second) > 30.0f) {
            continue;
        }
        if (bot->GetDistance2d(unit) > sPlayerbotAIConfig->spellDistance) {
            continue;
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "unstoppable abomination")) {
            if (target_abomination == nullptr || 
                target_abomination->GetDistance2d(helper.center.first, helper.center.second) > unit->GetDistance2d(helper.center.first, helper.center.second)) {
                target_abomination = unit;
            }
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "soldier of the frozen wastes")) {
            if (target_soldier == nullptr || 
                target_soldier->GetDistance2d(helper.center.first, helper.center.second) > unit->GetDistance2d(helper.center.first, helper.center.second)) {
                target_soldier = unit;
            }
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "soul weaver")) {
            if (target_weaver == nullptr || 
                target_weaver->GetDistance2d(helper.center.first, helper.center.second) > unit->GetDistance2d(helper.center.first, helper.center.second)) {
                target_weaver = unit;
            }
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "kel'thuzad")) {
            target_kelthuzad = unit;
        }
    }
    std::vector<Unit*> targets;
    if (botAI->IsRanged(bot)) {
        if (botAI->GetRangedDpsIndex(bot) <= 1) {
            targets = {target_soldier, target_weaver, target_abomination, target_kelthuzad};
        } else {
            targets = {target_weaver, target_soldier, target_abomination, target_kelthuzad};
        }
    } else if (botAI->IsAssistTank(bot)) {
        targets = {target_abomination, target_guardian, target_kelthuzad};
    } else {
        targets = {target_abomination, target_kelthuzad};
    }
    for (Unit* t : targets) {
        if (t) {
            target = t;
            break;
        }
    }
    if (context->GetValue<Unit*>("current target")->Get() == target) {
        return false;
    }
    if (target_kelthuzad && target == target_kelthuzad) {
        return Attack(target, true);
    }
    return Attack(target, false);
}

bool KelthuzadPositionAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    if (helper.IsPhaseOne()) {
        if (AI_VALUE(Unit*, "current target") == nullptr) {
            return MoveInside(NAXX_MAP_ID, helper.center.first, helper.center.second, bot->GetPositionZ(), 3.0f);
        }
    } else if (helper.IsPhaseTwo()) {
        Unit* shadow_fissure = helper.GetAnyShadowFissure();
        if (!shadow_fissure || bot->GetDistance2d(shadow_fissure) > 10.0f) {
            float distance, angle;
            if (botAI->IsMainTank(bot)) {
                if (AI_VALUE2(bool, "has aggro", "current target")) {
                    return MoveTo(NAXX_MAP_ID, helper.tank_pos.first, helper.tank_pos.second, bot->GetPositionZ());
                } else {
                    return false;
                }
            } else if (botAI->IsRanged(bot)) {
                uint32 index = botAI->GetRangedIndex(bot);
                if (index < 8) {
                    distance = 20.0f;
                    angle = index * M_PI / 4;
                } else {
                    distance = 32.0f;
                    angle = (index - 8) * M_PI / 4;
                }
                float dx, dy;
                dx = helper.center.first + cos(angle) * distance;
                dy = helper.center.second + sin(angle) * distance;
                return MoveTo(NAXX_MAP_ID, dx, dy, bot->GetPositionZ());
            } else if (botAI->IsTank(bot)) {
                Unit* cur_tar = AI_VALUE(Unit*, "current target");
                if (cur_tar && cur_tar->GetVictim() && cur_tar->GetVictim()->ToPlayer() && 
                    botAI->EqualLowercaseName(cur_tar->GetName(), "guardian of icecrown") && 
                    botAI->IsAssistTank(cur_tar->GetVictim()->ToPlayer()) ) {
                    return MoveTo(NAXX_MAP_ID, helper.assist_tank_pos.first, helper.assist_tank_pos.second, bot->GetPositionZ());
                } else {
                    return false;
                }
            }
        } else {
            float dx, dy;
            float angle;
            if (!botAI->IsRanged(bot)) {
                angle = shadow_fissure->GetAngle(helper.center.first, helper.center.second);
            } else {
                angle = bot->GetAngle(shadow_fissure) + M_PI;
            }
            dx = shadow_fissure->GetPositionX() + cos(angle) * 10.0f;
            dy = shadow_fissure->GetPositionY() + sin(angle) * 10.0f;
            return MoveTo(NAXX_MAP_ID, dx, dy, bot->GetPositionZ());
        }
    }
    return false;
}

bool AnubrekhanChooseTargetAction::Execute(Event event)
{
    GuidVector attackers = context->GetValue<GuidVector >("attackers")->Get();
    Unit* target = nullptr;
    Unit *target_boss = nullptr;
    std::vector<Unit*> target_guards;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
        if (botAI->EqualLowercaseName(unit->GetName(), "crypt guard")) {
            target_guards.push_back(unit);
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "anub'rekhan")) {
            target_boss = unit;
        }
    }
    if (botAI->IsMainTank(bot)) {
        target = target_boss;
    } else {
        if (target_guards.size() == 0) {
            target = target_boss;
        } else {
            if (botAI->IsAssistTank(bot)) {
                for (Unit* t : target_guards) {
                    if (target == nullptr || (target->GetVictim() && target->GetVictim()->ToPlayer() && botAI->IsTank(target->GetVictim()->ToPlayer()))) {
                        target = t;
                    }
                }
            } else {
                for (Unit* t : target_guards) {
                    if (target == nullptr || target->GetHealthPct() > t->GetHealthPct()) {
                        target = t;
                    }
                }
            }
        }
    }
    if (context->GetValue<Unit*>("current target")->Get() == target) {
        return false;
    }
    return Attack(target);
}

bool AnubrekhanPositionAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss) {
        return false;
    }
    boss_anubrekhan::boss_anubrekhanAI* boss_ai = dynamic_cast<boss_anubrekhan::boss_anubrekhanAI*>(boss->GetAI());
    if (!boss_ai) {
        return false;
    }
    EventMap *eventMap = &boss_ai->events;
    uint32 locust = eventMap->GetNextEventTime(2);
    uint32 timer = eventMap->GetTimer();
    bool inPhase = botAI->HasAura("locust swarm", boss) || boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (inPhase || (locust && locust - timer <= 8000)) {
        if (botAI->IsMainTank(bot)) {
            uint32 nearest = FindNearestWaypoint();
            uint32 next_point;
            if (inPhase || (locust && locust - timer <= 3000)) {
                next_point = (nearest + 1) % intervals;
            } else {
                next_point = nearest;
            }
            return MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ());
        } else {
            return MoveInside(533, 3272.49f, -3476.27f, bot->GetPositionZ(), 3.0f);
        }
    }
    return false;
}

bool GluthChooseTargetAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    GuidVector attackers = context->GetValue<GuidVector>("possible targets")->Get();
    Unit* target = nullptr;
    Unit *target_boss = nullptr;
    std::vector<Unit*> target_zombies;
    for (GuidVector::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;
        if (!unit->IsAlive()) {
            continue;
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "zombie chow")) {
            target_zombies.push_back(unit);
        }
        if (botAI->EqualLowercaseName(unit->GetName(), "gluth")) {
            target_boss = unit;
        }
    }
    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) {
        target = target_boss;
    } else if (botAI->IsAssistTankOfIndex(bot, 1)) {
        for (Unit* t : target_zombies) {
            if (t->GetHealthPct() > helper.decimatedZombiePct && t->GetVictim() != bot && t->GetDistance2d(bot) <= 10.0f) {
                if (!target || t->GetDistance2d(bot) < target->GetDistance2d(bot)) {
                    target = t;
                }
            }
        }
    } else if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 0 || botAI->GetClassIndex(bot, CLASS_HUNTER) == 1) {
        // prevent zombie go straight to gluth
        for (Unit* t : target_zombies) {
            if (t->GetHealthPct() > helper.decimatedZombiePct && t->GetVictim() == target_boss && t->GetDistance2d(bot) <= sPlayerbotAIConfig->spellDistance) {
                if (!target || t->GetDistance2d(bot) < target->GetDistance2d(bot)) {
                    target = t;
                }
            }
        }
        if (!target) {
            target = target_boss;
        }
    } else {
        for (Unit* t : target_zombies) {
            if (t->GetHealthPct() <= helper.decimatedZombiePct) {
                if (target == nullptr || 
                    target->GetDistance2d(helper.mainTankPos25.first, helper.mainTankPos25.second) > 
                    t->GetDistance2d(helper.mainTankPos25.first, helper.mainTankPos25.second)) {
                    target = t;
                }
            }
        }
        if (target == nullptr) {
            target = target_boss;
        }
    }
    if (!target || context->GetValue<Unit*>("current target")->Get() == target) {
        return false;
    }
    if (target_boss && target == target_boss)
        return Attack(target, true);
    return Attack(target, false);
    // return Attack(target);
}

bool GluthPositionAction::Execute(Event event)
{
    if (!helper.UpdateBossAI()) {
        return false;
    }
    bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0)) {
        if (AI_VALUE2(bool, "has aggro", "boss target")) {
            if (raid25) {
                return MoveTo(NAXX_MAP_ID, helper.mainTankPos25.first, helper.mainTankPos25.second, bot->GetPositionZ());
            } else {
                return MoveTo(NAXX_MAP_ID, helper.mainTankPos10.first, helper.mainTankPos10.second, bot->GetPositionZ());
            }
        }
    } else if (botAI->IsAssistTankOfIndex(bot, 1)) {
        if (helper.BeforeDecimate()) {
            return MoveTo(bot->GetMapId(), helper.beforeDecimatePos.first, helper.beforeDecimatePos.second, bot->GetPositionZ());
        } else {
            if (AI_VALUE2(bool, "has aggro", "current target")) {
                uint32 nearest = FindNearestWaypoint();
                uint32 next_point = (nearest + 1) % intervals;
                return MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ());
            }
        }
    } else if (botAI->IsRangedDps(bot)) {
        if (raid25) {
            if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 0) {
                return MoveInside(NAXX_MAP_ID, helper.leftSlowDownPos.first, helper.leftSlowDownPos.second, bot->GetPositionZ(), 0.0f);    
            }
            if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 1) {
                return MoveInside(NAXX_MAP_ID, helper.rightSlowDownPos.first, helper.rightSlowDownPos.second, bot->GetPositionZ(), 0.0f);    
            }
        }
        return MoveInside(NAXX_MAP_ID, helper.rangedPos.first, helper.rangedPos.second, bot->GetPositionZ(), 3.0f);
    } else if (botAI->IsHeal(bot)) {
        return MoveInside(NAXX_MAP_ID, helper.healPos.first, helper.healPos.second, bot->GetPositionZ(), 0.0f);
    }
    return false;
}

bool GluthSlowdownAction::Execute(Event event)
{   
    if (!helper.UpdateBossAI()) {
        return false;
    }
    bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
    if (!raid25) {
        return false;
    }
    if (helper.JustStartCombat()) {
        return false;
    }
    switch (bot->getClass()) 
    {
        case CLASS_HUNTER:
            return botAI->CastSpell("frost trap", bot);
            break;
        // case CLASS_MAGE:
        //     return botAI->CastSpell("frost nova", bot);
        //     break;
        default:
            break;
    }
    return false;
}

// bool LoathebPositionAction::Execute(Event event)
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
//     if (!boss) {
//         return false;
//     }
//     if (botAI->IsTank(bot)) {
//         if (AI_VALUE2(bool, "has aggro", "boss target")) {
//             return MoveTo(533, 2877.57, -3967.00, bot->GetPositionZ());
//         }
//     } else if (botAI->IsRanged(bot)) {
//         return MoveInside(533, 2896.96f, -3980.61f, bot->GetPositionZ(), 1.0f);
//     }
//     return false;
// }

// bool LoathebChooseTargetAction::Execute(Event event)
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "loatheb");
//     if (!boss) {
//         return false;
//     }
//     BossAI* boss_ai = dynamic_cast<BossAI*>(boss->GetAI());
//     EventMap* eventMap = boss_botAI->GetEvents();
//     list<ObjectGuid> attackers = context->GetValue<list<ObjectGuid> >("attackers")->Get();
//     Unit* target = nullptr;
//     Unit *target_boss = nullptr;
//     Unit *target_spore = nullptr;
//     for (list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
//     {
//         Unit* unit = botAI->GetUnit(*i);
//         if (!unit)
//             continue;
//         if (!unit->IsAlive()) {
//             continue;
//         }
//         if (botAI->EqualLowercaseName(unit->GetName(), "spore")) {
//             target_spore = unit;
//         }
//         if (botAI->EqualLowercaseName(unit->GetName(), "loatheb")) {
//             target_boss = unit;
//         }
//     }
//     if (target_spore && bot->GetDistance2d(target_spore) <= 1.0f) {
//         target = target_spore;
//     } else {
//         target = target_boss;
//     }
//     if (!target || context->GetValue<Unit*>("current target")->Get() == target) {
//         return false;
//     }
//     return Attack(target);
// }