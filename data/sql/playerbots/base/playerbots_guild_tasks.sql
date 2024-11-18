DROP TABLE IF EXISTS `playerbots_guild_tasks`;
CREATE TABLE `playerbots_guild_tasks` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `owner` INT(11) NOT NULL,
  `guildid` INT(11) NOT NULL,
  `time` INT(11) NOT NULL,
  `validIn` INT(11) DEFAULT NULL,
  `type` varchar(45) DEFAULT NULL,
  `value` INT(11) DEFAULT NULL,
  `data` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `owner` (`owner`),
  KEY `guildid` (`guildid`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
