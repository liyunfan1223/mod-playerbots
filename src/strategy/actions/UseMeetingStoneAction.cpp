/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UseMeetingStoneAction.h"

#include "CellImpl.h"
#include "Event.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool UseMeetingStoneAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid guid;
    p >> guid;

    if (master->GetTarget() && master->GetTarget() != bot->GetGUID())
        return false;

    if (!master->GetTarget() && master->GetGroup() != bot->GetGroup())
        return false;

    if (master->IsBeingTeleported())
        return false;

    if (bot->IsInCombat())
    {
        botAI->TellError("I am in combat");
        return false;
    }

    Map* map = master->GetMap();
    if (!map)
        return false;

    GameObject* gameObject = map->GetGameObject(guid);
    if (!gameObject)
        return false;

    GameObjectTemplate const* goInfo = gameObject->GetGOInfo();
    if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SUMMONING_RITUAL)
        return false;

    return Teleport(master, bot);
}

class AnyGameObjectInObjectRangeCheck
{
public:
    AnyGameObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(GameObject* go)
    {
        if (go && i_obj->IsWithinDistInMap(go, i_range) && go->isSpawned() && go->GetGOInfo())
            return true;

        return false;
    }

private:
    WorldObject const* i_obj;
    float i_range;
};

bool SummonAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (Pet* pet = bot->GetPet())
    {
        botAI->PetFollow();
    }

    if (master->GetSession()->GetSecurity() >= SEC_PLAYER)
    {
        // botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({});
        AI_VALUE(std::list<FleeInfo>&, "recently flee info").clear();
        return Teleport(master, bot);
    }

    if (SummonUsingGos(master, bot) || SummonUsingNpcs(master, bot))
    {
        botAI->TellMasterNoFacing("Hello!");
        return true;
    }

    if (SummonUsingGos(bot, master) || SummonUsingNpcs(bot, master))
    {
        botAI->TellMasterNoFacing("Welcome!");
        return true;
    }

    return false;
}

bool SummonAction::SummonUsingGos(Player* summoner, Player* player)
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(summoner, sPlayerbotAIConfig->sightDistance);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(summoner, targets, u_check);
    Cell::VisitAllObjects(summoner, searcher, sPlayerbotAIConfig->sightDistance);

    for (GameObject* go : targets)
    {
        if (go->isSpawned() && go->GetGoType() == GAMEOBJECT_TYPE_MEETINGSTONE)
            return Teleport(summoner, player);
    }

    botAI->TellError(summoner == bot ? "There is no meeting stone nearby" : "There is no meeting stone near you");
    return false;
}

bool SummonAction::SummonUsingNpcs(Player* summoner, Player* player)
{
    if (!sPlayerbotAIConfig->summonAtInnkeepersEnabled)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(summoner, sPlayerbotAIConfig->sightDistance);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(summoner, targets, u_check);
    Cell::VisitAllObjects(summoner, searcher, sPlayerbotAIConfig->sightDistance);

    for (Unit* unit : targets)
    {
        if (unit && unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER))
        {
            if (!player->HasItemCount(6948, 1, false))
            {
                botAI->TellError(player == bot ? "I have no hearthstone" : "You have no hearthstone");
                return false;
            }

            if (player->HasSpellCooldown(8690))
            {
                botAI->TellError(player == bot ? "My hearthstone is not ready" : "Your hearthstone is not ready");
                return false;
            }

            // Trigger cooldown
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(8690);
            if (!spellInfo)
                return false;

            Spell spell(player, spellInfo, TRIGGERED_NONE);
            spell.SendSpellCooldown();

            return Teleport(summoner, player);
        }
    }

    botAI->TellError(summoner == bot ? "There are no innkeepers nearby" : "There are no innkeepers near you");
    return false;
}

bool SummonAction::Teleport(Player* summoner, Player* player)
{
    // Player* master = GetMaster();
    if (!summoner)
        return false;
    
    if (player->GetVehicle())
    {
        botAI->TellError("You cannot summon me while I'm on a vehicle");
        return false;
    }

    if (!summoner->IsBeingTeleported() && !player->IsBeingTeleported())
    {
        float followAngle = GetFollowAngle();
        for (float angle = followAngle - M_PI; angle <= followAngle + M_PI; angle += M_PI / 4)
        {
            uint32 mapId = summoner->GetMapId();
            float x = summoner->GetPositionX() + cos(angle) * sPlayerbotAIConfig->followDistance;
            float y = summoner->GetPositionY() + sin(angle) * sPlayerbotAIConfig->followDistance;
            float z = summoner->GetPositionZ();

            if (summoner->IsWithinLOS(x, y, z))
            {
                if (sPlayerbotAIConfig
                        ->botRepairWhenSummon)  // .conf option to repair bot gear when summoned 0 = off, 1 = on
                    bot->DurabilityRepairAll(false, 1.0f, false);

                if (summoner->IsInCombat() && !sPlayerbotAIConfig->allowSummonInCombat)
                {
                    botAI->TellError("You cannot summon me while you're in combat");
                    return false;
                }

                if (!summoner->IsAlive() && !sPlayerbotAIConfig->allowSummonWhenMasterIsDead)
                {
                    botAI->TellError("You cannot summon me while you're dead");
                    return false;
                }

                if (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST) &&
                    !sPlayerbotAIConfig->allowSummonWhenBotIsDead)
                {
                    botAI->TellError("You cannot summon me while I'm dead, you need to release my spirit first");
                    return false;
                }

                bool revive =
                    sPlayerbotAIConfig->reviveBotWhenSummoned == 2 ||
                    (sPlayerbotAIConfig->reviveBotWhenSummoned == 1 && !summoner->IsInCombat() && summoner->IsAlive());

                if (bot->isDead() && revive)
                {
                    bot->ResurrectPlayer(1.0f, false);
                    botAI->TellMasterNoFacing("I live, again!");
                    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Reset();
                }

                player->GetMotionMaster()->Clear();
                AI_VALUE(LastMovement&, "last movement").clear();
                player->TeleportTo(mapId, x, y, z, 0);
                return true;
            }
        }
    }

    botAI->TellError("Not enough place to summon");
    return false;
}
