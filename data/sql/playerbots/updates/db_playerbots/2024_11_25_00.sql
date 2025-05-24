UPDATE `updates_include`
SET `path` = '$/data/sql/playerbots/updates'
WHERE `state` = 'RELEASED';

UPDATE `updates_include`
SET `path` = '$/data/sql/playerbots/custom'
WHERE `state` = 'CUSTOM';

UPDATE `updates_include`
SET `path` = '$/data/sql/playerbots/archive'
WHERE `state` = 'ARCHIVED';
