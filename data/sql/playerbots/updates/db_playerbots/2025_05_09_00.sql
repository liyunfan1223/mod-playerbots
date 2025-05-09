DROP TABLE IF EXISTS `playerbot_account_keys`;
CREATE TABLE IF NOT EXISTS `playerbots_account_keys` (
    `account_id` INT PRIMARY KEY,
    `security_key` VARCHAR(255) NOT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=INNODB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `playerbot_account_links`;
CREATE TABLE IF NOT EXISTS `playerbots_account_links` (
    `id` INT AUTO_INCREMENT PRIMARY KEY,
    `account_id` INT NOT NULL,
    `linked_account_id` INT NOT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY `account_link` (`account_id`, `linked_account_id`)
) ENGINE=INNODB DEFAULT CHARSET=latin1;
