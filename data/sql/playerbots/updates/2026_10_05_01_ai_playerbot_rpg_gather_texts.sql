-- #########################################################
-- Playerbots - Add rpg status do gather texts
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN (
    'rpg_gather_no_profession_error',
    'rpg_unknown_status_error'
);
DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'rpg_gather_no_profession_error',
    'rpg_unknown_status_error'
);

-- rpg_gather_no_profession_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1928,
    'rpg_gather_no_profession_error',
    'I have neither Herbalism nor Mining, so I can''t gather.',
    0, 0,
    '저는 약초 채집도 채광도 없어서 채집할 수 없습니다.',
    'Je n''ai ni Herboristerie ni Minage, je ne peux donc pas récolter.',
    'Ich habe weder Kräuterkunde noch Bergbau, also kann ich nicht sammeln.',
    '我既没有草药学也没有采矿，所以无法采集。',
    '我既沒有草藥學也沒有採礦，所以無法採集。',
    'No tengo ni Herboristería ni Minería, así que no puedo recolectar.',
    'No tengo ni Herboristería ni Minería, así que no puedo recolectar.',
    'У меня нет ни травничества, ни горного дела, поэтому я не могу собирать.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_gather_no_profession_error', 100);

-- rpg_unknown_status_error: re-inserted with 'do gather' in the option list.
-- Option keywords stay in English because they are the actual command
-- tokens parsed by NewRpgInfo::StatusFromString
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1766,
    'rpg_unknown_status_error',
    'Unknown rpg status. Options: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    0, 0,
    '알 수 없는 RPG 상태입니다. 옵션: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Statut RPG inconnu. Options: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Unbekannter RPG-Status. Optionen: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    '未知的RPG状态。选项: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather。',
    '未知的RPG狀態。選項: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather。',
    'Estado RPG desconocido. Opciones: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Estado RPG desconocido. Opciones: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Неизвестный RPG-статус. Опции: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_unknown_status_error', 100);
