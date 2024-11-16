DROP TABLE IF EXISTS `ai_playerbot_texts_chance`;
CREATE TABLE IF NOT EXISTS `ai_playerbot_texts_chance` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `probability` bigint(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=UTF8;

/*!40000 ALTER TABLE `ai_playerbot_texts_chance` DISABLE KEYS */;
INSERT INTO `ai_playerbot_texts_chance` (`id`, `name`, `probability`) VALUES
	(1, 'taunt', 30),
	(2, 'aoe', 75),
	(3, 'loot', 20);
/*!40000 ALTER TABLE `ai_playerbot_texts_chance` ENABLE KEYS */;
