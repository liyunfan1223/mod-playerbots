/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_JSONUTILS_H
#define PLAYERBOTS_JSONUTILS_H

#include <string>

// Escapes a string for use as a JSON string value.
std::string JsonEscape(std::string const& value);

// Formats a number as a JSON value (3 decimals, non-finite values become 0).
std::string JsonNumber(double value);

// Formats value / total as a JSON number; 0 when total is not positive.
std::string JsonRatio(double value, double total);

// Absolute path of `filename` inside the configured LogsDir.
std::string LogsJsonPath(std::string const& filename);

// Writes `content` to `path` atomically (temp file + rename). `context` prefixes the log lines.
bool WriteJsonFile(std::string const& path, std::string const& content, char const* context);

#endif
