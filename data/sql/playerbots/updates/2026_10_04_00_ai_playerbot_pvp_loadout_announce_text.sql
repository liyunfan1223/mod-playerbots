-- #########################################################
-- Playerbots - PvP loadout announcement in skirmishes
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts
WHERE name = 'pvp_loadout_announce';

DELETE FROM ai_playerbot_texts_chance
WHERE name = 'pvp_loadout_announce';

INSERT INTO `ai_playerbot_texts`
(`id`, `name`, `text`, `say_type`, `reply_type`,
 `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
 `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
1928,
'pvp_loadout_announce',
'Going in as %spec, average item level %ilvl, geared for rating %rating',
0, 0,
'%spec 특성으로 출전합니다. 평균 아이템 레벨 %ilvl, 평점 %rating 기준 장비',
'J''entre en %spec, niveau d''objet moyen %ilvl, équipé pour une cote de %rating',
'Ich trete als %spec an, durchschnittliche Gegenstandsstufe %ilvl, ausgerüstet für Wertung %rating',
'以%spec出战，平均物品等级%ilvl，按%rating评级配装',
'以%spec出戰，平均物品等級%ilvl，按%rating評級配裝',
'Entro como %spec, nivel de objeto medio %ilvl, equipado para un índice de %rating',
'Entro como %spec, nivel de objeto promedio %ilvl, equipado para un índice de %rating',
'Вступаю в бой как %spec, средний уровень предметов %ilvl, экипировка под рейтинг %rating'
);

INSERT INTO ai_playerbot_texts_chance (name, probability)
VALUES ('pvp_loadout_announce', 100);
