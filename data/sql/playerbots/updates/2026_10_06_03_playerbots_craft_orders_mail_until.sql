-- mail_until: game time until which the bot collects payments for, and returns of, the refunds it mailed; the row
-- outlives the order until then.
ALTER TABLE `playerbots_craft_orders` ADD COLUMN `mail_until` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `reagents`;
