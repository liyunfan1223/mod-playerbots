/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "JsonUtils.h"
#include "Config.h"
#include "Log.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

std::string JsonEscape(std::string const& value)
{
    std::ostringstream out;
    for (char c : value)
    {
        switch (c)
        {
            case '"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\b':
                out << "\\b";
                break;
            case '\f':
                out << "\\f";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<uint32>(c)
                        << std::dec;
                else
                    out << c;
                break;
        }
    }

    return out.str();
}

std::string JsonNumber(double value)
{
    if (!std::isfinite(value))
        value = 0.0;

    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << value;

    return out.str();
}

std::string JsonRatio(double value, double total)
{
    return JsonNumber(total > 0.0 ? value / total : 0.0);
}

std::string LogsJsonPath(std::string const& filename)
{
    std::string dir = sConfigMgr->GetOption<std::string>("LogsDir", "", false);
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
        dir.push_back('/');

    return dir + filename;
}

bool WriteJsonFile(std::string const& path, std::string const& content, char const* context)
{
    std::string const tempPath = path + ".tmp";

    std::ofstream file(tempPath.c_str(), std::ios::out | std::ios::trunc);
    if (!file)
    {
        LOG_ERROR("playerbots", "{} could not write {}", context, tempPath);
        return false;
    }

    file << content;
    file.close();

    if (!file)
    {
        LOG_ERROR("playerbots", "{} failed to write {}", context, tempPath);

        std::error_code removeError;
        std::filesystem::remove(tempPath, removeError);
        return false;
    }

    std::error_code renameError;
    std::filesystem::rename(tempPath, path, renameError);
    if (renameError)
    {
        LOG_ERROR("playerbots", "{} could not replace {}: {}", context, path, renameError.message());

        std::error_code removeError;
        std::filesystem::remove(tempPath, removeError);
        return false;
    }

    LOG_INFO("playerbots", "{} dump written to {}", context, path);
    return true;
}
