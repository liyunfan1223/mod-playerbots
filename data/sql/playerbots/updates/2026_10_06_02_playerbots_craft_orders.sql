-- Open crafting orders ('craft [item link] [count]'), one per bot, so a server crash does not lose track of a
-- customer's reagents. reagents: space-separated item:perCraft:obtained:own entries.
CREATE TABLE IF NOT EXISTS `playerbots_craft_orders` (
    `bot`          INT UNSIGNED NOT NULL,
    `owner`        INT UNSIGNED NOT NULL,
    `item`         INT UNSIGNED NOT NULL,
    `spell`        INT UNSIGNED NOT NULL,
    `count`        INT UNSIGNED NOT NULL,
    `crafted`      INT UNSIGNED NOT NULL,
    `made`         INT UNSIGNED NOT NULL,
    `delivered`    INT UNSIGNED NOT NULL,
    `own_products` INT UNSIGNED NOT NULL,
    `reagents`     VARCHAR(512) NOT NULL,
    PRIMARY KEY (`bot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
