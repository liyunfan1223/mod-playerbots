DROP TABLE IF EXISTS `playerbots_custom_strategy`;
CREATE TABLE `playerbots_custom_strategy` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `idx` int(11),
  `owner` int(11),
  `action_line` varchar(1024) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DELETE FROM `playerbots_custom_strategy`;
INSERT INTO `playerbots_custom_strategy` VALUES
(NULL, 'say', 1, 0, 'critical health>emote::helpme!99,say::critical health!98'),
(NULL, 'say', 2, 0, 'low health>emote::healme!99,say::low health!98'),
(NULL, 'say', 3, 0, 'low mana>emote::oom!99,say::low mana!98'),
(NULL, 'say', 4, 0, 'tank aoe>charge!99,say::taunt!98'),
(NULL, 'say', 5, 0, 'medium aoe>say::aoe!99'),
(NULL, 'say', 6, 0, 'can loot>say::loot!99'),
(NULL, 'say', 7, 0, 'dps assist>say::taunt!97');
