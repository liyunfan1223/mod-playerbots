-- ##########################################################
-- # Playerbots RandomBots Performance Update
-- # Add missing index to reduce Deadlocks
-- # Author: Raz0r1337 aka St0ny
-- # Date: 2025-04-26
-- ##########################################################

ALTER TABLE `playerbots_random_bots`
ADD INDEX `idx_owner_bot_event` (`owner`, `bot`, `event`);
