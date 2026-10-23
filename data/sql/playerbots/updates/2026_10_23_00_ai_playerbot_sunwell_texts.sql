DELETE FROM ai_playerbot_texts WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_below_twenty_percent_health',
    'sathrovarr_health_when_kalecgos_below_twenty_percent_health',
    'felmyst_flight_leader',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'eredar_twins_alythess_tank_main_tank_paladin',
    'eredar_twins_alythess_tank_paladin_tank',
    'eredar_twins_alythess_tank_no_paladin'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_below_twenty_percent_health',
    'sathrovarr_health_when_kalecgos_below_twenty_percent_health',
    'felmyst_flight_leader',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'eredar_twins_alythess_tank_main_tank_paladin',
    'eredar_twins_alythess_tank_paladin_tank',
    'eredar_twins_alythess_tank_no_paladin'
);

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1930, 'kalecgos_tank_sent_to_spectral_realm', 'Tank %tank has been sent to the Spectral Realm. The active Kalecgos tank is %current.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1931, 'kalecgos_tank_should_enter_spectral_realm', 'Tank %tank should enter the Spectral Realm. The active Kalecgos tank is %current.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1932, 'kalecgos_below_twenty_percent_health', 'Kalecgos''s health is at 20%!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1934, 'sathrovarr_health_when_kalecgos_below_twenty_percent_health', 'Sathrovarr''s health is at %sathrovarrHealth%! Don''t forget that we need to defeat them at about the same time!', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1935, 'felmyst_flight_leader', '[NAME] is now the flight phase leader. Everybody needs to stack on [NAME] during the flight phase.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1936, 'kiljaeden_designated_dragon_orb_user', '%bot is the first assistant bot and the designated dragon orb user. If you would like only players to control dragons, please remove assistant flags from all bots.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1937, 'kiljaeden_no_designated_dragon_orb_user', 'No bot has an assistant flag, and therefore a player must control the dragons. If you would like a bot to control the dragons, please set the assistant flag for a bot.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1938, 'eredar_twins_alythess_tank_main_tank_paladin', 'Alythess requires a Paladin tank. %bot is the main tank and a Paladin and is assigned to tank Alythess.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1939, 'eredar_twins_alythess_tank_paladin_tank', 'Alythess requires a Paladin tank. The main tank is not a Paladin. %bot is the best-geared Paladin tank and is assigned to tank Alythess.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1940, 'eredar_twins_alythess_tank_no_paladin', 'Alythess requires a Paladin tank. However, no Paladin tank is present. Therefore, the main tank, %bot, is assigned to tank Alythess.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('kalecgos_tank_sent_to_spectral_realm', 100),
    ('kalecgos_tank_should_enter_spectral_realm', 100),
    ('kalecgos_below_twenty_percent_health', 100),
    ('sathrovarr_health_when_kalecgos_below_twenty_percent_health', 100),
    ('felmyst_flight_leader', 100),
    ('kiljaeden_designated_dragon_orb_user', 100),
    ('kiljaeden_no_designated_dragon_orb_user', 100),
    ('eredar_twins_alythess_tank_main_tank_paladin', 100),
    ('eredar_twins_alythess_tank_paladin_tank', 100),
    ('eredar_twins_alythess_tank_no_paladin', 100);
