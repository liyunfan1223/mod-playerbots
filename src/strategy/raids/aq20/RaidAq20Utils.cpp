#include "RaidAq20Utils.h"

#include "SpellAuras.h"

uint32 const OSSIRIAN_BUFF = 25176;
uint32 const OSSIRIAN_DEBUFFS[] = {25177, 25178, 25180, 25181, 25183};
uint32 const OSSIRIAN_CRYSTAL_GO_ENTRY = 180619;

bool RaidAq20Utils::IsOssirianBuffActive(Unit* ossirian)
{
    return ossirian && ossirian->HasAura(OSSIRIAN_BUFF);
}

int32 RaidAq20Utils::GetOssirianDebuffTimeRemaining(Unit* ossirian)
{
    int32 retVal = 0xffffff;
    if (ossirian)
    {
        for (uint32 debuff : OSSIRIAN_DEBUFFS)
        {
            if (AuraApplication* auraApplication = ossirian->GetAuraApplication(debuff))
            {
                if (Aura* aura = auraApplication->GetBase())
                {
                    int32 duration = aura->GetDuration();
                    if (retVal > duration)
                        retVal = duration;
                }
            }
        }
    }
    return retVal;
}

GameObject* RaidAq20Utils::GetNearestCrystal(Unit* ossirian)
{
    return ossirian ? ossirian->FindNearestGameObject(OSSIRIAN_CRYSTAL_GO_ENTRY, 200.0f) : nullptr;
}
