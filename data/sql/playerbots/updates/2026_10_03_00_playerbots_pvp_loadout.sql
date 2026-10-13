-- PvP loadout snapshots: the PvE state of a random bot while it wears an arena loadout (AiPlayerbot.PvpLoadoutSwap).
-- A row exists only while the bot is in PvP state and is deleted once the bot is restored and saved.
-- glyphs: 6 glyph ids, comma-separated. match_items: guids of the items created for the match, to destroy.
CREATE TABLE IF NOT EXISTS `playerbots_pvp_loadout` (
    `guid`              INT UNSIGNED NOT NULL,
    `talent_link`       VARCHAR(128) NOT NULL DEFAULT '',
    `glyphs`            VARCHAR(64)  NOT NULL DEFAULT '',
    `match_items`       TEXT         NOT NULL,
    `arena_instance_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `created_at`        TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- PvP loadout item copies: the PvE items a random bot's arena loadout replaced, kept as the core persists them in
-- item_instance so they can be destroyed for the match and recreated identically at restore (no bag space needed).
-- One row per replaced equipment slot; rows exist only while the playerbots_pvp_loadout row for the bot does.
-- charges: 5 numbers, enchantments: 12 (id duration charges) triples, each followed by a space, as in item_instance.
CREATE TABLE IF NOT EXISTS `playerbots_pvp_loadout_item` (
    `guid`               INT UNSIGNED      NOT NULL,
    `slot`               TINYINT UNSIGNED  NOT NULL,
    `item_guid`          INT UNSIGNED      NOT NULL DEFAULT 0,
    `item_entry`         INT UNSIGNED      NOT NULL DEFAULT 0,
    `creator_guid`       INT UNSIGNED      NOT NULL DEFAULT 0,
    `gift_creator_guid`  INT UNSIGNED      NOT NULL DEFAULT 0,
    `flags`              INT UNSIGNED      NOT NULL DEFAULT 0,
    `duration`           INT UNSIGNED      NOT NULL DEFAULT 0,
    `charges`            TINYTEXT          NOT NULL,
    `enchantments`       TEXT              NOT NULL,
    `random_property_id` SMALLINT          NOT NULL DEFAULT 0,
    `durability`         SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `played_time`        INT UNSIGNED      NOT NULL DEFAULT 0,
    `text`               TEXT              NOT NULL,
    PRIMARY KEY (`guid`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
