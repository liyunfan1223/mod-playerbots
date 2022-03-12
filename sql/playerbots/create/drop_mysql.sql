REVOKE ALL PRIVILEGES ON * . * FROM 'acore'@'localhost';

REVOKE ALL PRIVILEGES ON `acore_playerbots` . * FROM 'acore'@'localhost';

REVOKE GRANT OPTION ON `acore_playerbots` . * FROM 'acore'@'localhost';

DROP USER 'acore'@'localhost';

DROP DATABASE IF EXISTS `acore_playerbots`;
