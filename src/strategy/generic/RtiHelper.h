/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RTIHELPER_H
#define _PLAYERBOT_RTIHELPER_H

#include <Define.h>

#include <string>

// make static class and method which convert string rti on rti index
class RtiHelper
{
public:
    static int8 GetRtiIndex(std::string const& rti)
    {
        if (rti == "star")
            return 0;
        else if (rti == "circle")
            return 1;
        else if (rti == "diamond")
            return 2;
        else if (rti == "triangle")
            return 3;
        else if (rti == "moon")
            return 4;
        else if (rti == "square")
            return 5;
        else if (rti == "cross")
            return 6;
        else if (rti == "skull")
            return 7;
        return -1;  // Invalid RTI
    }
    static const int8 starIndex = 0;
    static const int8 circleIndex = 1;
    static const int8 diamondIndex = 2;
    static const int8 triangleIndex = 3;
    static const int8 moonIndex = 4;
    static const int8 squareIndex = 5;
    static const int8 crossIndex = 6;
    static const int8 skullIndex = 7;
};

#endif
