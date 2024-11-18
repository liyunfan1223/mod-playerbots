DROP TABLE IF EXISTS `playerbots_rarity_cache`;
CREATE TABLE `playerbots_rarity_cache` (
  `id` INT(11) auto_increment,
  `item` INT(11) NOT NULL,
  `rarity` float NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Playerbots Rarity Cache';
