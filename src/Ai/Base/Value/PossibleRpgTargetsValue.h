/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSSIBLERPGTARGETSVALUE_H
#define PLAYERBOTS_POSSIBLERPGTARGETSVALUE_H

#include "NearestGameObjects.h"
#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"
#include "SharedDefines.h"

class PlayerbotAI;

class PossibleRpgTargetsValue : public NearestUnitsValue
{
public:
    PossibleRpgTargetsValue(PlayerbotAI* botAI, float range = 70.0f);

    static const std::vector<uint32> allowedNpcFlags;

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class PossibleNewRpgTargetsValue : public NearestUnitsValue
{
public:
    PossibleNewRpgTargetsValue(PlayerbotAI* botAI);

    // Candidate-cache radius: Playerbots.RpgTargetSearchDistance above 0, otherwise the map visibility range.
    static float GetSearchRange(Player* bot);

    static const std::vector<uint32> allowedNpcFlags;
    GuidVector Calculate() override;
protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class PossibleNewRpgGameObjectsValue : public ObjectGuidListCalculatedValue
{
public:
    PossibleNewRpgGameObjectsValue(PlayerbotAI* botAI, bool ignoreLos = true)
        : ObjectGuidListCalculatedValue(botAI, "possible new rpg game objects"), ignoreLos(ignoreLos)
    {
    }

    static const std::vector<GameobjectTypes> allowedGOFlags;
    GuidVector Calculate() override;

private:
    bool ignoreLos;
};

// GameObjects the "grab" strategy (QuestGrabStrategy) should walk over to and
// use. Unlike PossibleNewRpgGameObjectsValue (which only ever accepts
// GAMEOBJECT_TYPE_QUESTGIVER), this filters with GameObject::ActivateToQuest
// -- the same core-provided check the client itself uses to decide whether an
// object should sparkle for a player's quests, covering CHEST/GOOBER/GENERIC/
// SPELL_FOCUS/QUESTGIVER. Range is measured from the bot's own current
// position, not the leader -- see the comment on Playerbots.QuestGrabDistance.
class PossibleQuestGrabTargetsValue : public ObjectGuidListCalculatedValue
{
public:
    PossibleQuestGrabTargetsValue(PlayerbotAI* botAI, float range = 0.0f)
        : ObjectGuidListCalculatedValue(botAI, "possible quest grab targets"),
          range(range ? range : sPlayerbotAIConfig.QuestGrabDistance)
    {
    }

    GuidVector Calculate() override;

private:
    float range;
};

#endif
