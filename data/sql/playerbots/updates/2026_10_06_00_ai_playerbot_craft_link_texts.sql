DELETE FROM ai_playerbot_texts WHERE name IN (
    'craft_usage',
    'craft_no_professions'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'craft_usage',
    'craft_no_professions'
);

INSERT INTO ai_playerbot_texts (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8) VALUES
(1830, 'craft_usage', 'Usage: ''craft'', ''craft [item link]'', ''craft ?'' or ''craft reset''', 0, 0, '사용법: ''craft'', ''craft [아이템 링크]'', ''craft ?'' 또는 ''craft reset''', 'Utilisation : ''craft'', ''craft [lien d''objet]'', ''craft ?'' ou ''craft reset''', 'Verwendung: ''craft'', ''craft [Gegenstandslink]'', ''craft ?'' oder ''craft reset''', '用法：''craft''、''craft [物品链接]''、''craft ?'' 或 ''craft reset''', '用法：''craft''、''craft [物品連結]''、''craft ?'' 或 ''craft reset''', 'Uso: ''craft'', ''craft [enlace de objeto]'', ''craft ?'' o ''craft reset''', 'Uso: ''craft'', ''craft [enlace de objeto]'', ''craft ?'' o ''craft reset''', 'Использование: ''craft'', ''craft [ссылка на предмет]'', ''craft ?'' или ''craft reset'''),
(1929, 'craft_no_professions', 'I don''t know any crafting professions', 0, 0, '저는 제작 전문 기술을 하나도 모릅니다', 'Je ne connais aucun métier d''artisanat', 'Ich beherrsche keine Herstellungsberufe', '我不会任何制造专业', '我不會任何製造專業', 'No conozco ninguna profesión de fabricación', 'No conozco ninguna profesión de fabricación', 'Я не владею ни одной ремесленной профессией');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
('craft_usage', 100),
('craft_no_professions', 100);
