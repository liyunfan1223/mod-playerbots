#ifndef _PLAYERBOT_RAIDNAXXBOSSHELPER_H
#define _PLAYERBOT_RAIDNAXXBOSSHELPER_H

#include <string>

#include "AiObject.h"
#include "AiObjectContext.h"
#include "EventMap.h"
#include "Log.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidNaxxScripts.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"

const uint32 NAXX_MAP_ID = 533;

template <class BossAiType>
class GenericBossHelper : public AiObject
{
public:
    GenericBossHelper(PlayerbotAI* botAI, std::string name) : AiObject(botAI), _name(name) {}
    virtual bool UpdateBossAI()
    {
        if (!bot->IsInCombat())
        {
            _unit = nullptr;
        }
        if (_unit && (!_unit->IsInWorld() || !_unit->IsAlive()))
        {
            _unit = nullptr;
        }
        if (!_unit)
        {
            _unit = AI_VALUE2(Unit*, "find target", _name);
            if (!_unit)
            {
                return false;
            }
            _target = _unit->ToCreature();
            if (!_target)
            {
                return false;
            }
            _ai = dynamic_cast<BossAiType*>(_target->GetAI());
            if (!_ai)
            {
                return false;
            }
            _event_map = &_ai->events;
            if (!_event_map)
            {
                return false;
            }
        }
        if (!_event_map)
        {
            return false;
        }
        _timer = _event_map->GetTimer();
        return true;
    }
    virtual void Reset()
    {
        _unit = nullptr;
        _target = nullptr;
        _ai = nullptr;
        _event_map = nullptr;
        _timer = 0;
    }

protected:
    std::string _name;
    Unit* _unit = nullptr;
    Creature* _target = nullptr;
    BossAiType* _ai = nullptr;
    EventMap* _event_map = nullptr;
    uint32 _timer = 0;
};

class KelthuzadBossHelper : public GenericBossHelper<Kelthuzad::boss_kelthuzad::boss_kelthuzadAI>
{
public:
    KelthuzadBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "kel'thuzad") {}
    const std::pair<float, float> center = {3716.19f, -5106.58f};
    const std::pair<float, float> tank_pos = {3709.19f, -5104.86f};
    const std::pair<float, float> assist_tank_pos = {3746.05f, -5112.74f};
    bool IsPhaseOne() { return _event_map->GetNextEventTime(Kelthuzad::EVENT_PHASE_2) != 0; }
    bool IsPhaseTwo() { return !IsPhaseOne(); }
    Unit* GetAnyShadowFissure()
    {
        Unit* shadow_fissure = nullptr;
        GuidVector units = *context->GetValue<GuidVector>("nearest triggers");
        for (auto i = units.begin(); i != units.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (!unit)
                continue;
            if (botAI->EqualLowercaseName(unit->GetName(), "shadow fissure"))
            {
                shadow_fissure = unit;
            }
        }
        return shadow_fissure;
    }
};

class RazuviousBossHelper : public GenericBossHelper<Razuvious::boss_razuvious::boss_razuviousAI>
{
public:
    RazuviousBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "instructor razuvious") {}
};

class SapphironBossHelper : public GenericBossHelper<Sapphiron::boss_sapphiron::boss_sapphironAI>
{
public:
    const std::pair<float, float> mainTankPos = {3512.07f, -5274.06f};
    const std::pair<float, float> center = {3517.31f, -5253.74f};
    const float GENERIC_HEIGHT = 137.29f;
    SapphironBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "sapphiron") {}
    bool UpdateBossAI() override
    {
        if (!GenericBossHelper::UpdateBossAI())
        {
            return false;
        }
        uint32 nextEventGround = _event_map->GetNextEventTime(Sapphiron::EVENT_GROUND);
        if (nextEventGround && nextEventGround != lastEventGround)
            lastEventGround = nextEventGround;
        return true;
    }
    bool IsPhaseGround() { return _target->GetReactState() == REACT_AGGRESSIVE; }
    bool IsPhaseFlight() { return !IsPhaseGround(); }
    bool JustLanded()
    {
        return (_event_map->GetNextEventTime(Sapphiron::EVENT_FLIGHT_START) - _timer) >=
               EVENT_FLIGHT_INTERVAL - POSITION_TIME_AFTER_LANDED;
    }
    bool WaitForExplosion() { return _event_map->GetNextEventTime(Sapphiron::EVENT_FLIGHT_SPELL_EXPLOSION); }
    bool FindPosToAvoidChill(std::vector<float>& dest)
    {
        Aura* aura = botAI->GetAura("chill", bot);
        if (!aura)
        {
            return false;
        }
        DynamicObject* dyn_obj = aura->GetDynobjOwner();
        if (!dyn_obj)
        {
            return false;
        }
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        float angle = 0;
        uint32 index = botAI->GetGroupSlotIndex(bot);
        if (currentTarget)
        {
            if (botAI->IsRanged(bot))
            {
                if (bot->GetExactDist2d(currentTarget) <= 45.0f)
                {
                    angle = bot->GetAngle(dyn_obj) - M_PI + (rand_norm() - 0.5) * M_PI / 2;
                }
                else
                {
                    if (index % 2 == 0)
                    {
                        angle = bot->GetAngle(currentTarget) + M_PI / 2;
                    }
                    else
                    {
                        angle = bot->GetAngle(currentTarget) - M_PI / 2;
                    }
                }
            }
            else
            {
                if (index % 3 == 0)
                {
                    angle = bot->GetAngle(currentTarget);
                }
                else if (index % 3 == 1)
                {
                    angle = bot->GetAngle(currentTarget) + M_PI / 2;
                }
                else
                {
                    angle = bot->GetAngle(currentTarget) - M_PI / 2;
                }
            }
        }
        else
        {
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

class GluthBossHelper : public GenericBossHelper<Gluth::boss_gluth::boss_gluthAI>
{
public:
    const std::pair<float, float> mainTankPos25 = {3331.48f, -3109.06f};
    const std::pair<float, float> mainTankPos10 = {3278.29f, -3162.06f};
    const std::pair<float, float> beforeDecimatePos = {3267.34f, -3175.68f};
    const std::pair<float, float> leftSlowDownPos = {3290.68f, -3141.65f};
    const std::pair<float, float> rightSlowDownPos = {3300.78f, -3151.98f};
    const std::pair<float, float> rangedPos = {3301.45f, -3139.29f};
    const std::pair<float, float> healPos = {3303.09f, -3135.24f};

    const float decimatedZombiePct = 10.0f;
    GluthBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "gluth") {}
    bool BeforeDecimate()
    {
        uint32 decimate = _event_map->GetNextEventTime(Gluth::EVENT_DECIMATE);
        return decimate && decimate - _timer <= 3000;
    }
    bool JustStartCombat() { return _timer < 10000; }
};

class LoathebBossHelper : public GenericBossHelper<Loatheb::boss_loatheb::boss_loathebAI>
{
public:
    const std::pair<float, float> mainTankPos = {2877.57f, -3967.00f};
    const std::pair<float, float> rangePos = {2896.96f, -3980.61f};
    LoathebBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "loatheb") {}
};

class FourhorsemanBossHelper : public GenericBossHelper<FourHorsemen::boss_four_horsemen::boss_four_horsemenAI>
{
public:
    const float posZ = 241.27f;
    const std::pair<float, float> attractPos[2] = {{2502.03f, -2910.90f},
                                                   {2484.61f, -2947.07f}};  // left (sir zeliek), right (lady blaumeux)
    FourhorsemanBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "sir zeliek") {}
    bool UpdateBossAI() override
    {
        if (!GenericBossHelper::UpdateBossAI())
        {
            return false;
        }
        if (!bot->IsInCombat())
        {
            Reset();
        }
        sir = _unit;
        lady = AI_VALUE2(Unit*, "find target", "lady blaumeux");
        if (!lady)
        {
            return true;
        }
        ladyAI = dynamic_cast<FourHorsemen::boss_four_horsemen::boss_four_horsemenAI*>(lady->GetAI());
        if (!ladyAI)
        {
            return true;
        }
        ladyEvent = &ladyAI->events;
        const uint32 voidZone = ladyEvent->GetNextEventTime(FourHorsemen::EVENT_SECONDARY_SPELL);
        if (voidZone && lastEventVoidZone != voidZone)
        {
            voidZoneCounter++;
            voidZoneCounter %= 8;
            lastEventVoidZone = voidZone;
        }
        return true;
    }
    void Reset() override
    {
        GenericBossHelper::Reset();
        sir = nullptr;
        lady = nullptr;
        ladyAI = nullptr;
        ladyEvent = nullptr;
        lastEventVoidZone = 0;
        voidZoneCounter = 0;
        posToGo = 0;
    }
    bool IsAttracter(Player* bot)
    {
        Difficulty diff = bot->GetRaidDifficulty();
        if (diff == RAID_DIFFICULTY_25MAN_NORMAL)
        {
            return botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsHealAssistantOfIndex(bot, 0) ||
                   botAI->IsHealAssistantOfIndex(bot, 1) || botAI->IsHealAssistantOfIndex(bot, 2);
        }
        return botAI->IsRangedDpsAssistantOfIndex(bot, 0) || botAI->IsHealAssistantOfIndex(bot, 0);
    }
    void CalculatePosToGo(Player* bot)
    {
        bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
        if (!lady)
        {
            posToGo = 0;
        }
        else
        {
            // Interval: 24s - 15s - 15s - ...
            posToGo = !(_timer <= 9000 || ((_timer - 9000) / 67500) % 2 == 0);
            if (botAI->IsRangedDpsAssistantOfIndex(bot, 0) || (raid25 && botAI->IsHealAssistantOfIndex(bot, 1)))
            {
                posToGo = 1 - posToGo;
            }
        }
    }
    std::pair<float, float> CurrentAttractPos()
    {
        float posX = attractPos[posToGo].first, posY = attractPos[posToGo].second;
        if (posToGo == 1)
        {
            float offset_x;
            float offset_y;
            if (voidZoneCounter < 4)
            {
                offset_x = voidZoneCounter * (-4.5f);
                offset_y = voidZoneCounter * (4.5f);
            }
            if (voidZoneCounter >= 4)
            {
                offset_x = (7 - voidZoneCounter) * (-4.5f);
                offset_y = (7 - voidZoneCounter) * (4.5f);
                offset_x += 4.5f;
                offset_y += 4.5f;
            }
            posX += offset_x;
            posY += offset_y;
        }
        return {posX, posY};
    }
    Unit* CurrentAttackTarget()
    {
        if (posToGo == 0)
        {
            return sir;
        }
        return lady;
    }

protected:
    Unit* sir = nullptr;
    Unit* lady = nullptr;
    FourHorsemen::boss_four_horsemen::boss_four_horsemenAI* ladyAI = nullptr;
    EventMap* ladyEvent = nullptr;
    uint32 lastEventVoidZone = 0;
    uint32 voidZoneCounter = 0;
    int posToGo = 0;
};
class ThaddiusBossHelper : public GenericBossHelper<Thaddius::boss_thaddius::boss_thaddiusAI>
{
public:
    const std::pair<float, float> tankPosFeugen = {3522.94f, -3002.60f};
    const std::pair<float, float> tankPosStalagg = {3436.14f, -2919.98f};
    const std::pair<float, float> rangedPosFeugen = {3500.45f, -2997.92f};
    const std::pair<float, float> rangedPosStalagg = {3441.01f, -2942.04f};
    const float tankPosZ = 312.61f;
    ThaddiusBossHelper(PlayerbotAI* botAI) : GenericBossHelper(botAI, "thaddius") {}
    bool UpdateBossAI() override
    {
        if (!GenericBossHelper::UpdateBossAI())
        {
            return false;
        }
        feugen = AI_VALUE2(Unit*, "find target", "feugen");
        stalagg = AI_VALUE2(Unit*, "find target", "stalagg");
        return true;
    }
    bool IsPhasePet() { return (feugen && feugen->IsAlive()) || (stalagg && stalagg->IsAlive()); }
    bool IsPhaseTransition()
    {
        if (IsPhasePet())
        {
            return false;
        }
        return _unit->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
    }
    bool IsPhaseThaddius() { return !IsPhasePet() && !IsPhaseTransition(); }
    Unit* GetNearestPet()
    {
        Unit* unit = nullptr;
        if (feugen && feugen->IsAlive())
        {
            unit = feugen;
        }
        if (stalagg && stalagg->IsAlive() && (!feugen || bot->GetDistance(stalagg) < bot->GetDistance(feugen)))
        {
            unit = stalagg;
        }
        return unit;
    }
    std::pair<float, float> PetPhaseGetPosForTank()
    {
        if (GetNearestPet() == feugen)
        {
            return tankPosFeugen;
        }
        return tankPosStalagg;
    }
    std::pair<float, float> PetPhaseGetPosForRanged()
    {
        if (GetNearestPet() == feugen)
        {
            return rangedPosFeugen;
        }
        return rangedPosStalagg;
    }

protected:
    Unit* feugen = nullptr;
    Unit* stalagg = nullptr;
};

#endif
