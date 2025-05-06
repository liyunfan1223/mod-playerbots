#ifndef _PLAYERBOT_RAIDAQ20UTILS_H
#define _PLAYERBOT_RAIDAQ20UTILS_H

#include "GameObject.h"
#include "Unit.h"

class RaidAq20Utils
{
public:
    static bool IsOssirianBuffActive(Unit* ossirian);
    static int32 GetOssirianDebuffTimeRemaining(Unit* ossirian);
    static GameObject* GetNearestCrystal(Unit* ossirian);
};

#endif
