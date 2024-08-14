/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HELPERS_H
#define _PLAYERBOT_HELPERS_H

#include <stdio.h>
#include <string.h>
#include <charconv>
#include <optional>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <map>
#include <sstream>
#include <vector>
#include <type_traits>
#include <iomanip>
#include <ranges>

#include "Common.h"

// c++ 20 split
inline std::vector<std::string> split(std::string_view s, std::string_view delim)
{
    std::vector<std::string> substrings;
    if (delim.size() == 0ul)
    {
        substrings.emplace_back(s);
        return substrings;
    }

    auto start_pos = s.find_first_not_of(delim);
    auto end_pos = start_pos;
    auto max_length = s.length();

    while (start_pos < max_length)
    {
        end_pos = std::min(max_length, s.find_first_of(delim, start_pos));

        if (end_pos != start_pos)
        {
            substrings.emplace_back(&s[start_pos], end_pos - start_pos);
            start_pos = s.find_first_not_of(delim, end_pos);
        }
    }

    return substrings;
}

inline void split(std::vector<std::string>& dest, const std::string& str, char const* delim)
{
    dest = split(str, delim);
}

inline std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    elems = split(s, &delim);
    return elems;
}

inline std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

// c++ 20 generic converter + concept
template<typename T>
concept is_numeric = std::is_arithmetic_v<T>;

template<is_numeric T>
inline std::optional<T> convert_numeric(std::string_view input)
{
    T out;
    const std::from_chars_result result = std::from_chars(input.data(), input.data() + input.size(), out);
    if(result.ec == std::errc{})
        return out;
    return {};
}

#endif
