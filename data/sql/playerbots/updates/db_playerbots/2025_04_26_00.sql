-- ##########################################################
-- # Playerbots RandomBots Performance Update
-- # Add missing index to reduce Deadlocks
-- # Author: Raz0r1337 aka St0ny
-- # Date: 2025-04-26
-- ##########################################################

-- Check if the index already exists
SET @index_exists := (
  SELECT COUNT(1)
  FROM INFORMATION_SCHEMA.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'playerbots_random_bots'
    AND INDEX_NAME = 'idx_owner_bot_event'
);

-- Conditionally create the index only if it doesn't exist
SET @ddl := IF(@index_exists = 0,
  'ALTER TABLE `playerbots_random_bots` ADD INDEX `idx_owner_bot_event` (`owner`, `bot`, `event`);',
  'SELECT "Index idx_owner_bot_event already exists.";'
);

PREPARE stmt FROM @ddl;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
