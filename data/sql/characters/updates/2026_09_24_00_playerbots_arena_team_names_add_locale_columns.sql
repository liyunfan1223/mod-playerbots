ALTER TABLE `playerbots_arena_team_names`
    ADD COLUMN `name_koKR` VARCHAR(24) NULL AFTER `name`,
    ADD COLUMN `name_frFR` VARCHAR(24) NULL AFTER `name_koKR`,
    ADD COLUMN `name_deDE` VARCHAR(24) NULL AFTER `name_frFR`,
    ADD COLUMN `name_zhCN` VARCHAR(24) NULL AFTER `name_deDE`,
    ADD COLUMN `name_zhTW` VARCHAR(24) NULL AFTER `name_zhCN`,
    ADD COLUMN `name_esES` VARCHAR(24) NULL AFTER `name_zhTW`,
    ADD COLUMN `name_esMX` VARCHAR(24) NULL AFTER `name_esES`,
    ADD COLUMN `name_ruRU` VARCHAR(24) NULL AFTER `name_esMX`;
