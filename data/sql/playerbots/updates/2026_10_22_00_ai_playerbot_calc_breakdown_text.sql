-- #########################################################
-- Playerbots - Add calc command per-stat breakdown text
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts
WHERE name = 'calc_item_breakdown';

DELETE FROM ai_playerbot_texts_chance
WHERE name = 'calc_item_breakdown';

INSERT INTO `ai_playerbot_texts`
(`id`, `name`, `text`, `say_type`, `reply_type`,
 `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
 `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
1929,
'calc_item_breakdown',
'Breakdown: weighted sum %sum, multipliers x%mult',
0, 0,
'분석: 가중 합계 %sum, 배수 x%mult',
'Détail : somme pondérée %sum, multiplicateurs x%mult',
'Aufschlüsselung: gewichtete Summe %sum, Multiplikatoren x%mult',
'明细：加权总和 %sum，倍率 x%mult',
'明細：加權總和 %sum，倍率 x%mult',
'Desglose: suma ponderada %sum, multiplicadores x%mult',
'Desglose: suma ponderada %sum, multiplicadores x%mult',
'Разбивка: взвешенная сумма %sum, множители x%mult'
);

INSERT INTO ai_playerbot_texts_chance (name, probability)
VALUES ('calc_item_breakdown', 100);
