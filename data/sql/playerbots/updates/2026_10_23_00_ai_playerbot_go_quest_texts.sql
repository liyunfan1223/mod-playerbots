-- #########################################################
-- Playerbots - Add "go quest" texts
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- go_help keeps the command keywords in English, they are
-- the tokens GoAction parses.
-- #########################################################

DELETE FROM ai_playerbot_texts_chance WHERE name IN ('go_quest_nothing_error', 'go_quest_moving', 'go_help');
DELETE FROM ai_playerbot_texts WHERE id IN (1930, 1931, 1932);

INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES
(
    1930,
    'go_quest_nothing_error',
    'Nothing quest-related to grab right now',
    0, 0,
    '지금은 챙길 퀘스트 관련 대상이 없습니다',
    'Rien à récupérer pour une quête pour le moment',
    'Gerade gibt es nichts für eine Quest zu holen',
    '现在附近没有可以处理的任务目标',
    '現在附近沒有可以處理的任務目標',
    'Ahora mismo no hay nada de misiones que recoger',
    'Ahora mismo no hay nada de misiones que recoger',
    'Сейчас рядом нет ничего нужного для заданий'),
(
    1932,
    'go_quest_moving',
    'On my way, whisper ''go quest'' again once I''m there',
    0, 0,
    '가는 중입니다. 도착하면 ''go quest''를 다시 귓속말해 주세요',
    'J''y vais, chuchotez à nouveau ''go quest'' quand j''y serai',
    'Bin unterwegs, flüstere noch einmal ''go quest'', wenn ich da bin',
    '正在过去，到了之后请再密语 ''go quest''',
    '正在過去，到了之後請再密語 ''go quest''',
    'Voy para allá, vuelve a susurrar ''go quest'' cuando llegue',
    'Voy para allá, vuelve a susurrar ''go quest'' cuando llegue',
    'Иду, шепни ''go quest'' ещё раз, когда я буду на месте'),
(
    1931,
    'go_help',
    'Whisper ''go x,y'', ''go [game object]'', ''go unit'', ''go position'' or ''go quest'' and I will go there',
    0, 0,
    '''go x,y'', ''go [game object]'', ''go unit'', ''go position'' 또는 ''go quest''를 귓속말하면 그곳으로 가겠습니다',
    'Chuchotez ''go x,y'', ''go [game object]'', ''go unit'', ''go position'' ou ''go quest'' et j''irai là-bas',
    'Flüstere ''go x,y'', ''go [game object]'', ''go unit'', ''go position'' oder ''go quest'' und ich gehe dorthin',
    '密语 ''go x,y''、''go [game object]''、''go unit''、''go position'' 或 ''go quest''，我就会过去',
    '密語 ''go x,y''、''go [game object]''、''go unit''、''go position'' 或 ''go quest''，我就會過去',
    'Susurra ''go x,y'', ''go [game object]'', ''go unit'', ''go position'' o ''go quest'' e iré allí',
    'Susurra ''go x,y'', ''go [game object]'', ''go unit'', ''go position'' o ''go quest'' e iré allí',
    'Шепни ''go x,y'', ''go [game object]'', ''go unit'', ''go position'' или ''go quest'', и я пойду туда');

INSERT INTO ai_playerbot_texts_chance (name, probability)
SELECT name, 100 FROM ai_playerbot_texts WHERE id IN (1930, 1931, 1932);
