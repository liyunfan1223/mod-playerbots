#include "RaidBwlActions.h"

#include "Playerbots.h"

bool BwlOnyxiaScaleCloakAuraCheckAction::Execute(Event [[maybe_unused]] event)
{
    bot->AddAura(22683, bot);
    return true;
}

bool BwlOnyxiaScaleCloakAuraCheckAction::isUseful() { return !bot->HasAura(22683); }

bool BwlTurnOffSuppressionDeviceAction::Execute(Event [[maybe_unused]] event)
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (GuidVector::iterator i = gos.begin(); i != gos.end(); i++)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (!go)
        {
            continue;
        }
        if (go->GetEntry() != 179784 || go->GetDistance(bot) >= 15.0f || go->GetGoState() != GO_STATE_READY)
        {
            continue;
        }
        go->SetGoState(GO_STATE_ACTIVE);
    }
    return true;
}

bool BwlUseHourglassSandAction::Execute(Event [[maybe_unused]] event) { return botAI->CastSpell(23645, bot); }