DROP TABLE IF EXISTS `playerbots_account_keys`;

CREATE TABLE `playerbots_account_keys` (
    `account_id` INT PRIMARY KEY,
    `security_key` VARCHAR(255) NOT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=INNODB DEFAULT CHARSET=latin1;
