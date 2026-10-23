-- Message a bot whispers when the core refuses its logout request
DELETE FROM ai_playerbot_texts WHERE name IN ('logout_refused');
DELETE FROM ai_playerbot_texts_chance WHERE name IN ('logout_refused');

INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1929,
    'logout_refused',
    "I can't log out right now!",
    0, 0,
    -- koKR
    "지금은 로그아웃할 수 없습니다!",
    -- frFR
    "Je ne peux pas me déconnecter maintenant!",
    -- deDE
    "Ich kann mich gerade nicht ausloggen!",
    -- zhCN
    "我现在无法下线！",
    -- zhTW
    "我現在無法登出！",
    -- esES
    "¡No puedo cerrar sesión ahora mismo!",
    -- esMX
    "¡No puedo cerrar sesión ahora mismo!",
    -- ruRU
    "Сейчас я не могу выйти из игры!");

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('logout_refused', 100);
