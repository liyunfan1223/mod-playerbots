-- #########################################################
-- Playerbots - Add target requester command text
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts
WHERE name = 'target_requester_success';

DELETE FROM ai_playerbot_texts_chance
WHERE name = 'target_requester_success';

INSERT INTO `ai_playerbot_texts`
(`id`, `name`, `text`, `say_type`, `reply_type`,
 `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
 `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
1913,
'target_requester_success',
'Targeting %target',
0, 0,
'%target 님을 대상으로 지정합니다',
'Ciblage de %target',
'%target wird als Ziel ausgewählt',
'正在选择 %target',
'正在選擇 %target',
'Seleccionando a %target',
'Seleccionando a %target',
'Выбираю целью %target'
);

INSERT INTO ai_playerbot_texts_chance (name, probability)
VALUES ('target_requester_success', 100);
