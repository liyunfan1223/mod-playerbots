/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOCALECOLUMNS_H
#define PLAYERBOTS_LOCALECOLUMNS_H

#include "Common.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

struct LocaleColumn
{
    LocaleConstant Locale;
    std::string Column;
};

// Registered non-enUS locales with a column in the table; the rest fall back to enUS instead of breaking the query.
// Blocking query: call at startup on the world thread and cache the result.
template <class DatabasePool>
std::vector<LocaleColumn> GetLocaleColumns(DatabasePool& db, std::string const& table,
                                           std::function<std::string(LocaleConstant)> const& columnName)
{
    std::unordered_set<std::string> existing;
    if (QueryResult result = db.Query("SELECT `COLUMN_NAME` FROM `information_schema`.`COLUMNS` "
                                      "WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = '{}'",
                                      table))
    {
        do
            existing.insert(result->Fetch()[0].Get<std::string>());
        while (result->NextRow());
    }

    std::vector<LocaleColumn> columns;
    for (uint8 i = LOCALE_koKR; i < TOTAL_LOCALES; ++i)
    {
        LocaleConstant const locale = LocaleConstant(i);
        if (!IsLocaleRegistered(locale))
            continue;

        std::string column = columnName(locale);
        if (existing.contains(column))
            columns.push_back({locale, std::move(column)});
        else
            LOG_WARN("playerbots", "`{}` has no `{}` column, {} falls back to enUS", table, column,
                     localeNames[locale]);
    }

    return columns;
}

#endif
