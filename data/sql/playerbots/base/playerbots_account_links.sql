DROP TABLE IF EXISTS `playerbots_account_links`;

CREATE TABLE `playerbots_account_links` (
    `id` INT AUTO_INCREMENT PRIMARY KEY,
    `account_id` INT NOT NULL,
    `linked_account_id` INT NOT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY `account_link` (`account_id`, `linked_account_id`)
) ENGINE=INNODB DEFAULT CHARSET=latin1;
