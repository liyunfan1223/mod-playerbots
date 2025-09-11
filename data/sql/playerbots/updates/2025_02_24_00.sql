-- Update max_level for TBC Heroic dungeons in `playerbots_dungeon_suggestion_definition`
UPDATE `playerbots_dungeon_suggestion_definition`
SET `max_level` = 73
WHERE `id` IN (40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56);
