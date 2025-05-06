#include "RaidVoAActions.h"
#include "RaidVoATriggers.h"
#include "Define.h"
#include "Event.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Playerbots.h"
#include "Unit.h"

const Position VOA_EMALON_RESTORE_POSITION = Position(-221.8f, -243.8f, 96.8f, 4.7f);

bool EmalonMarkBossAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "emalon the storm watcher");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    bool isMainTank = botAI->IsMainTank(bot);
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a real player
    {
        // Iterate through the first 3 bot tanks to assign the Skull marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group && boss)
                {
                    int8 skullIndex = 7;  // Skull
                    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

                    // If there's no skull set yet, or the skull is on a different target, set boss
                    if (!currentSkullTarget || (boss->GetGUID() != currentSkullTarget))
                    {
                        group->SetTargetIcon(skullIndex, bot->GetGUID(), boss->GetGUID());
                        return true;
                    }
                }
                break;  // Stop after finding the first valid bot tank
            }
        }
    }
    else if (isMainTank)  // Bot is the main tank
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            int8 skullIndex = 7;  // Skull
            ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

            // If there's no skull set yet, or the skull is on a different target, set the Eonar's Gift
            if (!currentSkullTarget || (boss->GetGUID() != currentSkullTarget))
            {
                group->SetTargetIcon(skullIndex, bot->GetGUID(), boss->GetGUID());
                return true;
            }
        }
    }

    return false;
}

bool EmalonMarkBossAction::isUseful()
{
    EmalonMarkBossTrigger emalonMarkBossTrigger(botAI);
    return emalonMarkBossTrigger.IsActive();
}

bool EmalonLightingNovaAction::Execute(Event event)
{
    const float radius = 25.0f;  // 20 yards + 5 yard for safety for 10 man. For 25man there is no maximum range but 25 yards should be ok

    Unit* boss = AI_VALUE2(Unit*, "find target", "emalon the storm watcher");
    if (!boss)
        return false;

    float currentDistance = bot->GetDistance2d(boss);

    if (currentDistance < radius)
    {
        return MoveAway(boss, radius - currentDistance);
    }

    return false;
}

bool EmalonLightingNovaAction::isUseful()
{
    EmalonLightingNovaTrigger emalonLightingNovaTrigger(botAI);
    return emalonLightingNovaTrigger.IsActive();
}

bool EmalonOverchargeAction::Execute(Event event)
{
    // Check if there is any overcharged minion
    Unit* minion = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_TEMPEST_MINION && unit->HasAura(AURA_OVERCHARGE))
        {
            minion = unit;
            break;
        }
    }
    if (!minion)
    {
        return false;
    }

    bool isMainTank = botAI->IsMainTank(bot);
    Unit* mainTankUnit = AI_VALUE(Unit*, "main tank");
    Player* mainTank = mainTankUnit ? mainTankUnit->ToPlayer() : nullptr;

    if (mainTank && !GET_PLAYERBOT_AI(mainTank))  // Main tank is a real player
    {
        // Iterate through the first 3 bot tanks to assign the Skull marker
        for (int i = 0; i < 3; ++i)
        {
            if (botAI->IsAssistTankOfIndex(bot, i) && GET_PLAYERBOT_AI(bot))  // Bot is a valid tank
            {
                Group* group = bot->GetGroup();
                if (group && minion)
                {
                    int8 skullIndex = 7;  // Skull
                    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

                    // If there's no skull set yet, or the skull is on a different target, set Tempest Minion
                    if (!currentSkullTarget || (minion->GetGUID() != currentSkullTarget))
                    {
                        group->SetTargetIcon(skullIndex, bot->GetGUID(), minion->GetGUID());
                        return true;
                    }
                }
                break;  // Stop after finding the first valid bot tank
            }
        }
    }
    else if (isMainTank)  // Bot is the main tank
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            int8 skullIndex = 7;  // Skull
            ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);

            // If there's no skull set yet, or the skull is on a different target, set the Eonar's Gift
            if (!currentSkullTarget || (minion->GetGUID() != currentSkullTarget))
            {
                group->SetTargetIcon(skullIndex, bot->GetGUID(), minion->GetGUID());
                return true;
            }
        }
    }

    return false;
}

bool EmalonOverchargeAction::isUseful()
{
    EmalonOverchargeTrigger emalonOverchargeTrigger(botAI);
    return emalonOverchargeTrigger.IsActive();
}

bool EmalonFallFromFloorAction::Execute(Event event)
{
    return bot->TeleportTo(bot->GetMapId(), VOA_EMALON_RESTORE_POSITION.GetPositionX(),
                           VOA_EMALON_RESTORE_POSITION.GetPositionY(), VOA_EMALON_RESTORE_POSITION.GetPositionZ(),
                           VOA_EMALON_RESTORE_POSITION.GetOrientation());
}

bool EmalonFallFromFloorAction::isUseful()
{
    EmalonFallFromFloorTrigger emalonFallFromFloorTrigger(botAI);
    return emalonFallFromFloorTrigger.IsActive();
}
