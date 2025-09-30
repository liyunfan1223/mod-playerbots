/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LAZYCALCULATEDVALUE_H
#define _PLAYERBOT_LAZYCALCULATEDVALUE_H

template <class TValue, class TOwner>
class LazyCalculatedValue
{
public:
    typedef TValue (TOwner::*Calculator)();

public:
    LazyCalculatedValue(TOwner* owner, Calculator calculator) : calculator(calculator), owner(owner) { Reset(); }

public:
    TValue GetValue()
    {
        if (!calculated)
        {
            value = (owner->*calculator)();
            calculated = true;
        }

        return value;
    }

    void Reset() { calculated = false; }

protected:
    Calculator calculator;
    TOwner* owner;
    bool calculated;
    TValue value;
};

#endif
