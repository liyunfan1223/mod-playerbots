USE acore_playerbots;

INSERT INTO `version_db_playerbots` (`sql_rev`) VALUES ('1651613158228212658');

DELETE FROM `playerbots_text` WHERE `key` = 'suggest_instance';
INSERT INTO `playerbots_text` VALUES

(NULL, 'suggest_dungeon', 'Anyone wants %dungeon?'),
(NULL, 'suggest_dungeon', 'Any groups for %dungeon?'),
(NULL, 'suggest_dungeon', 'Need help for %dungeon?'),
(NULL, 'suggest_dungeon', 'LFD: %dungeon.'),
(NULL, 'suggest_dungeon_role', 'Anyone needs %role for %dungeon?'),
(NULL, 'suggest_dungeon_role', 'Missing %role for %dungeon?'),
(NULL, 'suggest_dungeon_role', 'Can be a %role for %dungeon.'),
(NULL, 'suggest_dungeon', 'Need help with %dungeon?'),
(NULL, 'suggest_dungeon_role', 'Need %role help with %dungeon?'),
(NULL, 'suggest_dungeon', 'Anyone needs gear from %dungeon?'),
(NULL, 'suggest_dungeon', 'A little grind in %dungeon?'),
(NULL, 'suggest_dungeon', 'WTR %dungeon'),
(NULL, 'suggest_dungeon', 'Need help for %dungeon.'),
(NULL, 'suggest_dungeon', 'Wanna run %dungeon.'),
(NULL, 'suggest_dungeon_role', '%role looks for %dungeon.'),
(NULL, 'suggest_dungeon', 'What about %dungeon?'),
(NULL, 'suggest_dungeon', 'Who wants to farm %dungeon?'),
(NULL, 'suggest_dungeon', 'Go in %dungeon?'),
(NULL, 'suggest_dungeon', 'Looking for %dungeon.'),
(NULL, 'suggest_dungeon', 'Need help with %dungeon quests?'),
(NULL, 'suggest_dungeon', 'Wanna quest in %dungeon.'),
(NULL, 'suggest_dungeon', 'Anyone with quests in %dungeon?'),
(NULL, 'suggest_dungeon', 'Could help with quests in %dungeon.'),
(NULL, 'suggest_dungeon_role', '%role: any place in group for %dungeon?'),
(NULL, 'suggest_dungeon', 'Does anybody still run %dungeon this days?'),
(NULL, 'suggest_dungeon_role', '%dungeon: anyone wants to take a %role?'),
(NULL, 'suggest_dungeon_role', 'Is there any point being %role in %dungeon?'),
(NULL, 'suggest_dungeon', 'It is really worth to go to %dungeon?'),
(NULL, 'suggest_dungeon', 'Anybody needs more people for %dungeon?'),
(NULL, 'suggest_dungeon', '%dungeon bosses drop good gear. Wanna run?'),
(NULL, 'suggest_dungeon', 'What about %dungeon?'),
(NULL, 'suggest_dungeon_role', 'Anybody needs %role?'),
(NULL, 'suggest_dungeon_role', 'Anyone needs %role?'),
(NULL, 'suggest_dungeon', 'Who wants %dungeon?'),
(NULL, 'suggest_dungeon', 'Can anybody summon me at %dungeon?'),
(NULL, 'suggest_dungeon', 'Meet me in %dungeon'),
(NULL, 'suggest_dungeon', 'Wanna quick %dungeon run'),
(NULL, 'suggest_dungeon', 'Wanna full %dungeon run'),
(NULL, 'suggest_dungeon', 'How many times were you in %dungeon?'),
(NULL, 'suggest_dungeon', 'Another %dungeon run?'),
(NULL, 'suggest_dungeon', 'Wiped in %dungeon? Take me instead!'),
(NULL, 'suggest_dungeon', 'Take me in %dungeon please.'),
(NULL, 'suggest_dungeon', 'Quick %dungeon run?'),
(NULL, 'suggest_dungeon', 'Full %dungeon run?'),
(NULL, 'suggest_dungeon_role', 'Who can take %role to %dungeon?');

DROP TABLE IF EXISTS `playerbots_dungeon_suggestion_definition`;
CREATE TABLE `playerbots_dungeon_suggestion_definition` (
    `id`         TINYINT(3)   NOT NULL AUTO_INCREMENT UNIQUE,
    `slug`       VARCHAR(255) NOT NULL,
    `name`       VARCHAR(255) NOT NULL,
    `expansion`  TINYINT(1)   NOT NULL,
    `difficulty` TINYINT(1)   NOT NULL,
    `min_level`  TINYINT(3),
    `max_level`  TINYINT(3),
    `comment`    VARCHAR(255),

    PRIMARY KEY (`id`)
)
ENGINE=MyISAM
DEFAULT CHARSET=utf8
ROW_FORMAT=FIXED
COMMENT='Playerbot dungeon suggestion definitions';

INSERT INTO `playerbots_dungeon_suggestion_definition` VALUES

(NULL, 'rfc'          , 'Ragefire Chasm'              , 0, 0, 15, 21, NULL),
(NULL, 'dm'           , 'Deadmines'                   , 0, 0, 15, 23, NULL),
(NULL, 'wc'           , 'Wailing Caverns'             , 0, 0, 17, 27, NULL),
(NULL, 'sfk'          , 'Shadowfang Keep'             , 0, 0, 18, 24, NULL),
(NULL, 'bfd'          , 'Blackfathom Deeps'           , 0, 0, 20, 27, NULL),
(NULL, 'stocks'       , 'Stormwind Stockade'          , 0, 0, 22, 30, NULL),
(NULL, 'gnomer'       , 'Gnomeregan'                  , 0, 0, 26, 36, NULL),
(NULL, 'sm-gy'        , 'Scarlet Monastery: Graveyard', 0, 0, 26, 36, NULL),
(NULL, 'sm-lib'       , 'Scarlet Monastery: Library'  , 0, 0, 29, 39, NULL),
(NULL, 'rfk'          , 'Razorfen Kraul'              , 0, 0, 32, 35, NULL),
(NULL, 'sm-armory'    , 'Scarlet Monastery: Armory'   , 0, 0, 32, 42, NULL),
(NULL, 'sm-cath'      , 'Scarlet Monastery: Cathedral', 0, 0, 35, 45, NULL),
(NULL, 'rfd'          , 'Razorfen Downs'              , 0, 0, 42, 45, NULL),
(NULL, 'ulda'         , 'Uldaman'                     , 0, 0, 42, 52, NULL),
(NULL, 'mara'         , 'Maraudon'                    , 0, 0, 42, 52, NULL),
(NULL, 'zf'           , 'Zul''Farrak'                 , 0, 0, 46, 56, NULL),
(NULL, 'st'           , 'Temple of Atal''Hakkar'      , 0, 0, 50, 60, NULL),
(NULL, 'brd'          , 'Blackrock Depths'            , 0, 0, 52, 60, NULL),
(NULL, 'dm-w'         , 'Dire Maul: West'             , 0, 0, 55, 60, NULL),
(NULL, 'dm-e'         , 'Dire Maul: East'             , 0, 0, 55, 60, NULL),
(NULL, 'dm-n'         , 'Dire Maul: North'            , 0, 0, 55, 60, NULL),
(NULL, 'lbrs'         , 'Lower Blackrock Spire'       , 0, 0, 55, 60, NULL),
(NULL, 'scholo'       , 'Scholomance'                 , 0, 0, 58, 60, NULL),
(NULL, 'strat'        , 'Stratholme'                  , 0, 0, 58, 60, NULL),
(NULL, 'hfr' , 'Hellfire Ramparts'      , 1, 0, 59, 67, NULL),
(NULL, 'bf'  , 'The Blood Furnace'      , 1, 0, 60, 68, NULL),
(NULL, 'sp'  , 'The Slave Pens'         , 1, 0, 61, 69, NULL),
(NULL, 'ub'  , 'The Underbog'           , 1, 0, 62, 70, NULL),
(NULL, 'mt'  , 'Mana-Tombs'             , 1, 0, 63, 70, NULL),
(NULL, 'ac'  , 'Auchenai Crypts'        , 1, 0, 64, 70, NULL),
(NULL, 'seth', 'Sethekk Halls'		    , 1, 0, 66, 70, NULL),
(NULL, 'oh'  , 'Old Hillsbrad Foothills', 1, 0, 66, 70, NULL),
(NULL, 'bm'  , 'The Black Morass'		, 1, 0, 68, 70, NULL),
(NULL, 'mech', 'The Mechanar'			, 1, 0, 68, 70, NULL),
(NULL, 'bot' , 'The Botanica'			, 1, 0, 69, 70, NULL),
(NULL, 'arc' , 'The Arcatraz'			, 1, 0, 69, 70, NULL),
(NULL, 'sh'  , 'The Shattered Halls'	, 1, 0, 69, 70, NULL),
(NULL, 'sv'  , 'The Steamvault'		    , 1, 0, 69, 70, NULL),
(NULL, 'sl'  , 'Shadow Labyrinth'		, 1, 0, 69, 70, NULL),
(NULL, 'mgt' , 'Magister''s Terrace'	, 1, 0, 70, 70, NULL),
(NULL, 'hfr' , 'Hellfire Ramparts'      , 1, 1, 70, 70, NULL),
(NULL, 'bf'  , 'The Blood Furnace'      , 1, 1, 70, 70, NULL),
(NULL, 'sp'  , 'The Slave Pens'         , 1, 1, 70, 70, NULL),
(NULL, 'ub'  , 'The Underbog'           , 1, 1, 70, 70, NULL),
(NULL, 'mt'  , 'Mana-Tombs'             , 1, 1, 70, 70, NULL),
(NULL, 'ac'  , 'Auchenai Crypts'        , 1, 1, 70, 70, NULL),
(NULL, 'seth', 'Sethekk Halls'		    , 1, 1, 70, 70, NULL),
(NULL, 'oh'  , 'Old Hillsbrad Foothills', 1, 1, 70, 70, NULL),
(NULL, 'bm'  , 'The Black Morass'	    , 1, 1, 70, 70, NULL),
(NULL, 'mech', 'The Mechanar'		    , 1, 1, 70, 70, NULL),
(NULL, 'bot' , 'The Botanica'		    , 1, 1, 70, 70, NULL),
(NULL, 'arc' , 'The Arcatraz'		    , 1, 1, 70, 70, NULL),
(NULL, 'sh'  , 'The Shattered Halls'    , 1, 1, 70, 70, NULL),
(NULL, 'sv'  , 'The Steamvault'		    , 1, 1, 70, 70, NULL),
(NULL, 'sl'  , 'Shadow Labyrinth'	    , 1, 1, 70, 70, NULL),
(NULL, 'mgt' , 'Magister''s Terrace'    , 1, 1, 70, 70, NULL),
(NULL, 'uk'    , 'Utgarde Keep'			      , 2, 0, 70, 72, NULL),
(NULL, 'nexus' , 'The Nexus'			      , 2, 0, 71, 73, NULL),
(NULL, 'an'    , 'Azjol-Nerub'			      , 2, 0, 72, 74, NULL),
(NULL, 'ak'    , 'Ahn''kahet: The Old Kingdom', 2, 0, 73, 75, NULL),
(NULL, 'dtk'   , 'Drak''Tharon Keep'		  , 2, 0, 74, 76, NULL),
(NULL, 'vh'    , 'Violet Hold'			      , 2, 0, 75, 77, NULL),
(NULL, 'gd'    , 'Gundrak'			          , 2, 0, 76, 78, NULL),
(NULL, 'hos'   , 'Halls of Stone'			  , 2, 0, 77, 79, NULL),
(NULL, 'hol'   , 'Halls of Lightning'		  , 2, 0, 80, 80, NULL),
(NULL, 'cos'   , 'The Culling of Stratholme'  , 2, 0, 80, 80, NULL),
(NULL, 'oculus', 'The Oculus'			      , 2, 0, 80, 80, NULL),
(NULL, 'up'    , 'Utgarde Pinnacle'		      , 2, 0, 80, 80, NULL),
(NULL, 'toc'   , 'Trial of the Champion'	  , 2, 0, 80, 80, NULL),
(NULL, 'fos'   , 'Forge of Souls'			  , 2, 0, 80, 80, NULL),
(NULL, 'pos'   , 'Pit of Saron'			      , 2, 0, 80, 80, NULL),
(NULL, 'hor'   , 'Halls of Reflection'	      , 2, 0, 80, 80, NULL),
(NULL, 'uk'    , 'Utgarde Keep'			      , 2, 1, 80, 80, NULL),
(NULL, 'nexus' , 'The Nexus'			      , 2, 1, 80, 80, NULL),
(NULL, 'an'    , 'Azjol-Nerub'			      , 2, 1, 80, 80, NULL),
(NULL, 'ak'    , 'Ahn''kahet: The Old Kingdom', 2, 1, 80, 80, NULL),
(NULL, 'dtk'   , 'Drak''Tharon Keep'		  , 2, 1, 80, 80, NULL),
(NULL, 'vh'    , 'Violet Hold'			      , 2, 1, 80, 80, NULL),
(NULL, 'gd'    , 'Gundrak'			          , 2, 1, 80, 80, NULL),
(NULL, 'hos'   , 'Halls of Stone'			  , 2, 1, 80, 80, NULL),
(NULL, 'hol'   , 'Halls of Lightning'		  , 2, 1, 80, 80, NULL),
(NULL, 'cos'   , 'The Culling of Stratholme'  , 2, 1, 80, 80, NULL),
(NULL, 'oculus', 'The Oculus'			      , 2, 1, 80, 80, NULL),
(NULL, 'up'    , 'Utgarde Pinnacle'		      , 2, 1, 80, 80, NULL),
(NULL, 'toc'   , 'Trial of the Champion'	  , 2, 1, 80, 80, NULL),
(NULL, 'fos'   , 'Forge of Souls'			  , 2, 1, 80, 80, NULL),
(NULL, 'pos'   , 'Pit of Saron'			      , 2, 1, 80, 80, NULL),
(NULL, 'hor'   , 'Halls of Reflection'	      , 2, 1, 80, 80, NULL);

CREATE TABLE `playerbots_dungeon_suggestion_abbrevation` (
    `id`           TINYINT(3)   NOT NULL AUTO_INCREMENT UNIQUE,
    `definition_slug` VARCHAR(255) NOT NULL,
    `abbrevation`  VARCHAR(16)  NOT NULL,

    PRIMARY KEY (`id`),
    KEY `definition_slug` (`definition_slug`)
)
ENGINE=MyISAM
DEFAULT CHARSET=utf8
ROW_FORMAT=FIXED
COMMENT='Playerbot dungeon suggestion abbrevations';

CREATE TABLE `playerbots_dungeon_suggestion_strategy` (
    `id`           TINYINT(3)   NOT NULL AUTO_INCREMENT UNIQUE,
    `definition_slug` VARCHAR(255) NOT NULL,
    `strategy`     VARCHAR(255) NOT NULL,
    `difficulty`   TINYINT(1)   NOT NULL,

    PRIMARY KEY (`id`),
    KEY `definition_slug` (`definition_slug`)
)
ENGINE=MyISAM
DEFAULT CHARSET=utf8
ROW_FORMAT=FIXED
COMMENT='Playerbot dungeon suggestion strategies';
