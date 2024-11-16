DROP TABLE IF EXISTS `playerbots_speech_probability`;
CREATE TABLE `playerbots_speech_probability` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `probability` INT(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `playerbots_speech_probability` VALUES
(NULL, 'taunt', 30),
(NULL, 'aoe', 75),
(NULL, 'loot', 20);
