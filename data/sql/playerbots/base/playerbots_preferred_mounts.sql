DROP TABLE IF EXISTS `playerbots_preferred_mounts`;
CREATE TABLE `playerbots_preferred_mounts` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `guid` INT(11) NOT NULL,
  `type` TINYINT(3) NOT NULL COMMENT '0: Ground, 1: Flying',
  `spellid` INT(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `guid` (`guid`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
