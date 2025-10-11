DELETE FROM ai_playerbot_texts WHERE name IN ('no_fishing_pole_error');
DELETE FROM ai_playerbot_texts_chance WHERE name IN ('no_fishing_pole_error');

INSERT INTO ai_playerbot_texts (name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8) VALUES
('no_fishing_pole_error',   "I don't have a Fishing Pole",   0, 0, 
"낚싯대가 없습니다", 
"Je n’ai pas de canne à pêche", 
"Ich habe keine Angelrute", 
"我沒有釣魚竿", 
"我没有钓鱼竿", 
"No tengo una caña de pescar", 
"No tengo una caña de pescar", 
"У меня нет удочки");

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('no_fishing_pole_error',   100);
