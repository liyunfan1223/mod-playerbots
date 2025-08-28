DROP TABLE IF EXISTS `ai_playerbot_texts`;
CREATE TABLE IF NOT EXISTS `ai_playerbot_texts` (
  `id` smallint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL COMMENT 'name - used in strategies/code as filter',
  `text` varchar(1024) NOT NULL COMMENT 'text',
  `say_type` tinyint(3) NOT NULL DEFAULT '0' COMMENT '0 - say, 1 - yell',
  `reply_type` tinyint(3) NOT NULL DEFAULT '0' COMMENT 'if > 0 then can be filtered as a response to chat',
  `text_loc1` varchar(1024) NOT NULL DEFAULT '',
  `text_loc2` varchar(1024) NOT NULL DEFAULT '',
  `text_loc3` varchar(1024) NOT NULL DEFAULT '',
  `text_loc4` varchar(1024) NOT NULL DEFAULT '',
  `text_loc5` varchar(1024) NOT NULL DEFAULT '',
  `text_loc6` varchar(1024) NOT NULL DEFAULT '',
  `text_loc7` varchar(1024) NOT NULL DEFAULT '',
  `text_loc8` varchar(1024) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

INSERT INTO `ai_playerbot_texts` (`name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES

-- strings
('string_unknown_area', 'the middle of nowhere', 0, 0, '', '', '', '荒无人烟之地', '', '', '', ''),
('string_unknown_area', 'an undisclosed location', 0, 0, '', '', '', '未知之地', '', '', '', ''),
('string_unknown_area', 'somewhere', 0, 0, '', '', '', '某个地方', '', '', '', ''),

('string_empty_link', 'something', 0, 0, '', '', '', '某件东西', '', '', '', ''),


-- looting item events
-- usable placeholders:
-- %item_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_looting_item_normal', 'wonder what %item_link tastes like', 0, 0, '', '', '', '想知道%item_link是什么味道', '', '', '', ''),
('broadcast_looting_item_poor', 'nooo, I got %item_link', 0, 0, '', '', '', '不！我捡到了%item_link', '', '', '', ''),
('broadcast_looting_item_poor', 'aww not this trash again %item_link', 0, 0, '', '', '', '哎，又是这垃圾%item_link', '', '', '', ''),
('broadcast_looting_item_poor', 'seem to be looting trash %item_link', 0, 0, '', '', '', '看来我捡到的都是垃圾%item_link', '', '', '', ''),
('broadcast_looting_item_poor', 'well, it\'s better than nothing I guess %item_link', 0, 0, '', '', '', '好吧，总比什么都没有强吧，%item_link', '', '', '', ''),
('broadcast_looting_item_poor', 'not sure what to do with %item_link', 0, 0, '', '', '', '不知道拿%item_link干什么好', '', '', '', ''),



('broadcast_looting_item_normal', 'wonder what %item_link tastes like', 0, 0, '', '', '', '想知道%item_link是什么味道', '', '', '', ''),
('broadcast_looting_item_normal', 'well, I can keep looting %item_link all day', 0, 0, '', '', '', '好吧，我可以整天捡%item_link', '', '', '', ''),
('broadcast_looting_item_normal', 'another day, another %item_link', 0, 0, '', '', '', '又是新的一天，又捡了个%item_link', '', '', '', ''),
('broadcast_looting_item_normal', 'looted some %item_link', 0, 0, '', '', '', '捡到了一些%item_link', '', '', '', ''),
('broadcast_looting_item_normal', 'some %item_link is alright', 0, 0, '', '', '', '有点%item_link还不错', '', '', '', ''),



('broadcast_looting_item_uncommon', 'not bad, I just got %item_link', 0, 0, '', '', '', '不错，我刚拿到%item_link', '', '', '', ''),
('broadcast_looting_item_uncommon', 'just looted %item_link in %zone_name', 0, 0, '', '', '', '刚在%zone_name捡到%item_link', '', '', '', ''),
('broadcast_looting_item_uncommon', 'I could put that to good use %item_link', 0, 0, '', '', '', '这个%item_link我能用得上', '', '', '', ''),
('broadcast_looting_item_uncommon', 'money money money %item_link', 0, 0, '', '', '', '钱钱钱，%item_link', '', '', '', ''),
('broadcast_looting_item_uncommon', 'got %item_link', 0, 0, '', '', '', '拿到了%item_link', '', '', '', ''),



('broadcast_looting_item_rare', '%item_link is hunter bis', 0, 0, '', '', '', '%item_link是猎人极品装备', '', '', '', ''),
('broadcast_looting_item_rare', '%item_link is %my_class bis', 0, 0, '', '', '', '%item_link是%my_class的极品装备', '', '', '', ''),
('broadcast_looting_item_rare', 'RNG is not bad today %item_link', 0, 0, '', '', '', '今天运气不错，捡到%item_link', '', '', '', ''),
('broadcast_looting_item_rare', 'sweet %item_link, freshly looted', 0, 0, '', '', '', '太棒了，%item_link刚捡到手', '', '', '', ''),
('broadcast_looting_item_rare', 'wow, I just got %item_link', 0, 0, '', '', '', '哇，我刚拿到%item_link', '', '', '', ''),



('broadcast_looting_item_rare', '%item_link is hunter bis', 0, 0, '', '', '', '%item_link是猎人极品装备', '', '', '', ''),
('broadcast_looting_item_rare', '%item_link is %my_class bis', 0, 0, '', '', '', '%item_link是%my_class的极品装备', '', '', '', ''),
('broadcast_looting_item_rare', 'RNG is not bad today %item_link', 0, 0, '', '', '', '今天运气不错，捡到%item_link', '', '', '', ''),
('broadcast_looting_item_rare', 'sweet %item_link, freshly looted', 0, 0, '', '', '', '太棒了，%item_link刚捡到手', '', '', '', ''),
('broadcast_looting_item_epic', 'OMG, look at what I just got %item_link!!!', 0, 0, '', '', '', '我的天，看看我刚捡到的%item_link！！！', '', '', '', ''),



('broadcast_looting_item_legendary', 'No @#$@% way! This can not be, I got %item_link, this is insane', 0, 0, '', '', '', '不可能！我拿到了%item_link，这太疯狂了！', '', '', '', ''),



('broadcast_looting_item_artifact', 'No @#$@% way! This can not be, I got %item_link, this is insane', 0, 0, '', '', '', '不可能！我拿到了%item_link，这太疯狂了！', '', '', '', ''),




-- quest events
-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_accepted_generic', 'I have just taken the %quest_link', 0, 0, '', '', '', '我刚接了%quest_link任务', '', '', '', ''),
('broadcast_quest_accepted_generic', 'just accepted %quest_link', 0, 0, '', '', '', '刚接下%quest_link任务', '', '', '', ''),
('broadcast_quest_accepted_generic', '%quest_link gonna try to complete this one', 0, 0, '', '', '', '%quest_link，我要试着完成这个', '', '', '', ''),
('broadcast_quest_accepted_generic', 'took %quest_link in %zone_name', 0, 0, '', '', '', '在%zone_name接了%quest_link任务', '', '', '', ''),



-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %quest_obj_available
-- %quest_obj_required
-- %quest_obj_missing
-- %quest_obj_name
-- %quest_obj_full_formatted
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_update_add_kill_objective_completed', 'Finally done with the %quest_obj_name for %quest_link', 0, 0, '', '', '', '终于完成了%quest_link的%quest_obj_name目标', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'finally got %quest_obj_available/%quest_obj_required of %quest_obj_name for the %quest_link', 0, 0, '', '', '', '终于收集了%quest_obj_available/%quest_obj_required个%quest_obj_name，完成%quest_link', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', '%quest_obj_full_formatted for the %quest_link, at last', 0, 0, '', '', '', '%quest_obj_full_formatted，终于完成了%quest_link', '', '', '', ''),



('broadcast_quest_update_add_kill_objective_progress', 'Oof, got %quest_obj_available/%quest_obj_required %quest_obj_name for %quest_link', 0, 0, '', '', '', '哎，拿到了%quest_obj_available/%quest_obj_required个%quest_obj_name，%quest_link任务', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'still need %quest_obj_missing more of %quest_obj_name for %quest_link', 0, 0, '', '', '', '还需要%quest_obj_missing个%quest_obj_name来完成%quest_link', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', '%quest_obj_full_formatted, still working on %quest_link', 0, 0, '', '', '', '%quest_obj_full_formatted，还在做%quest_link任务', '', '', '', ''),



-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %quest_obj_available
-- %quest_obj_required
-- %quest_obj_missing
-- %item_link
-- %quest_obj_full_formatted
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_update_add_item_objective_completed', 'Finally done with the %item_link for %quest_link', 0, 0, '', '', '', '终于完成了%quest_link的%item_link目标', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'finally got %quest_obj_available/%quest_obj_required of %item_link for the %quest_link', 0, 0, '', '', '', '终于收集了%quest_obj_available/%quest_obj_required个%item_link，完成%quest_link', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', '%quest_obj_full_formatted for the %quest_link, at last', 0, 0, '', '', '', '%quest_obj_full_formatted，终于完成了%quest_link', '', '', '', ''),



('broadcast_quest_update_add_item_objective_progress', 'Oof, got %quest_obj_available/%quest_obj_required %item_link for %quest_link', 0, 0, '', '', '', '哎，拿到了%quest_obj_available/%quest_obj_required个%item_link，%quest_link任务', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'still need %quest_obj_missing more of %item_link for %quest_link', 0, 0, '', '', '', '还需要%quest_obj_missing个%item_link来完成%quest_link', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', '%quest_obj_full_formatted, still working on %quest_link', 0, 0, '', '', '', '%quest_obj_full_formatted，还在做%quest_link任务', '', '', '', ''),


-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_update_failed_timer', 'Failed to finish %quest_link in time...', 0, 0, '', '', '', '没能及时完成%quest_link……', '', '', '', ''),
('broadcast_quest_update_failed_timer', 'Ran out of time for %quest_link :(', 0, 0, '', '', '', '时间不够，%quest_link失败了 :(', '', '', '', ''),


-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_update_complete', 'I have completed all objectives for %quest_link', 0, 0, '', '', '', '我完成了%quest_link的所有目标', '', '', '', ''),
('broadcast_quest_update_complete', 'Completed all objectives for %quest_link', 0, 0, '', '', '', '完成了%quest_link的所有目标', '', '', '', ''),
('broadcast_quest_update_complete', 'Gonna turn in the %quest_link soon, just finished all objectives', 0, 0, '', '', '', '很快就能交%quest_link了，刚完成所有目标', '', '', '', ''),


-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_turned_in', 'Yess, I have finally turned in the %quest_link', 0, 0, '', '', '', '耶，终于交了%quest_link任务', '', '', '', ''),
('broadcast_quest_turned_in', 'turned in the %quest_link', 0, 0, '', '', '', '交了%quest_link任务', '', '', '', ''),
('broadcast_quest_turned_in', 'managed to finish %quest_link, just turned in', 0, 0, '', '', '', '成功完成了%quest_link，刚交任务', '', '', '', ''),
('broadcast_quest_turned_in', 'just turned in %quest_link', 0, 0, '', '', '', '刚交了%quest_link任务', '', '', '', ''),
('broadcast_quest_turned_in', 'just turned in %quest_link in %zone_name', 0, 0, '', '', '', '刚在%zone_name交了%quest_link任务', '', '', '', ''),



-- kill mob events
-- usable placeholders:
-- %victim_name
-- %zone_name
-- %area_name
-- %victim_level
-- %my_race
-- %my_class
-- %my_level
('broadcast_killed_normal', 'another %victim_name down', 0, 0, '', '', '', '又干掉了一个%victim_name', '', '', '', ''),
('broadcast_killed_normal', 'I keep killing %victim_name, nothing to talk about', 0, 0, '', '', '', '我一直在杀%victim_name，没啥好说的', '', '', '', ''),
('broadcast_killed_normal', 'another %victim_name bites the dust', 0, 0, '', '', '', '又一个%victim_name倒下了', '', '', '', ''),
('broadcast_killed_normal', 'one less %victim_name in %zone_name', 0, 0, '', '', '', '%zone_name少了一个%victim_name', '', '', '', ''),



('broadcast_killed_elite', 'Took down this elite bastard %victim_name!', 0, 0, '', '', '', '干掉了这个精英混蛋%victim_name！', '', '', '', ''),
('broadcast_killed_player', 'killed elite %victim_name in %zone_name', 0, 0, '', '', '', '在%zone_name干掉了精英%victim_name', '', '', '', ''),



('broadcast_killed_rareelite', 'Fooof, managed to take down %victim_name!', 0, 0, '', '', '', '呼，终于干掉了%victim_name！', '', '', '', ''),



('broadcast_killed_worldboss', 'That was sick! Just killed %victim_name! Can tell tales now', 0, 0, '', '', '', '太爽了！刚干掉了%victim_name！可以吹牛了', '', '', '', ''),



('broadcast_killed_rare', 'Yoo, I just killed %victim_name!', 0, 0, '', '', '', '哟，我刚干掉了%victim_name！', '', '', '', ''),
('broadcast_killed_player', 'killed rare %victim_name in %zone_name', 0, 0, '', '', '', '在%zone_name干掉了稀有%victim_name', '', '', '', ''),



('broadcast_killed_unknown', 'WTF did I just kill? %victim_name', 0, 0, '', '', '', '我刚杀了什么鬼东西？%victim_name', '', '', '', ''),



('broadcast_killed_pet', 'Just killed that pet %victim_name', 0, 0, '', '', '', '刚干掉了那个宠物%victim_name', '', '', '', ''),


-- for broadcast_killed_player additionally can use %victim_class
('broadcast_killed_player', 'Oh yeah, I just killed %victim_name', 0, 0, '', '', '', '哦耶，我刚杀了%victim_name', '', '', '', ''),
('broadcast_killed_player', 'killed %victim_name in %zone_name', 0, 0, '', '', '', '在%zone_name杀了%victim_name', '', '', '', ''),



-- levelup events
-- usable placeholders:
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('broadcast_levelup_generic', 'Ding!', 0, 0, '', '', '', '叮！', '', '', '', ''),
('broadcast_levelup_generic', 'Yess, I am level %my_level!', 0, 0, '', '', '', '耶，我到%my_level级了！', '', '', '', ''),
('broadcast_levelup_generic', 'I just leveled up', 0, 0, '', '', '', '我刚升级了', '', '', '', ''),



('broadcast_levelup_10x', 'I am level %my_level!!!', 0, 0, '', '', '', '我到%my_level级了！！！', '', '', '', ''),
('broadcast_levelup_10x', 'getting stronger, already level %my_level!!!', 0, 0, '', '', '', '越来越强，已经%my_level级了！！！', '', '', '', ''),
('broadcast_levelup_10x', 'Just reached level %my_level!!!', 0, 0, '', '', '', '刚达到%my_level级！！！', '', '', '', ''),



('broadcast_levelup_max_level', 'OMG, finally level %my_level!!!', 0, 0, '', '', '', '我的天，终于到%my_level级了！！！', '', '', '', ''),
('broadcast_levelup_max_level', '%my_level!!! can do endgame content now', 0, 0, '', '', '', '%my_level级了！！！可以去打顶级内容了', '', '', '', ''),
('broadcast_levelup_max_level', 'fresh new level %my_level %my_class!!!', 0, 0, '', '', '', '全新的%my_level级%my_class！！！', '', '', '', ''),
('broadcast_levelup_max_level', 'one more level %my_level %my_race %my_class!', 0, 0, '', '', '', '又升了一级，%my_level级%my_race%my_class！', '', '', '', ''),



-- guild
-- usable placeholders:
-- %other_name
-- %other_class
-- %other_race
-- %other_level
('broadcast_guild_promotion', 'Good job %other_name. You deserved this.', 0, 0, '', '', '', '干得不错，%other_name，你 заслужил это。', '', '', '', ''),



('broadcast_guild_demotion', 'That was awful %other_name. I hate to do this but...', 0, 0, '', '', '', '太糟糕了，%other_name，我不想这样做，但是……', '', '', '', ''),




-- /////////////////////////////
-- suggestions


-- random instance
-- usable placeholders:
-- %my_role
-- %instance_name
-- %my_class
-- %my_race
-- %my_level
('suggest_instance', 'Anyone wants %instance_name?', 0, 0, '', 'Quelqu\'un fait %instance_name ?', '', '有人想去%instance_name吗？', '', '¿Alguien quiere ir a %instance_name?', '', ''),
('suggest_instance', 'Any groups for %instance_name?', 0, 0, '', 'Des groupes pour %instance_name ?', '', '有组去%instance_name吗？', '', '¿Algún grupo para %instance_name?', '', ''),
('suggest_instance', 'Need help for %instance_name?', 0, 0, '', 'Besoin d\'aide pour %instance_name ?', '', '需要帮忙打%instance_name吗？', '', '¿Alguien necesita ayuda con %instance_name?', '', ''),
('suggest_instance', 'LFD: %instance_name.', 0, 0, '', 'Cherche à faire : %instance_name', '', '找队：%instance_name。', '', 'Busco grupo para %instance_name.', '', ''),
('suggest_instance', 'Anyone needs %my_role for %instance_name?', 0, 0, '', 'Quelqu\'un a besoin de %my_role pour %instance_name ?', '', '有人需要%my_role去%instance_name吗？', '', '¿Alguien necesita un %my_role para %instance_name?', '', ''),
('suggest_instance', 'Missing %my_role for %instance_name?', 0, 0, '', '%my_role manquant pour %instance_name ?', '', '缺个%my_role去%instance_name吗？', '', '¿Buscando un %my_role para %instance_name?', '', ''),
('suggest_instance', 'Can be a %my_role for %instance_name.', 0, 0, '', 'Peut être un %my_role pour %instance_name', '', '我可以当%my_role去%instance_name。', '', 'Puedo ser un %my_role para %instance_name.', '', ''),
('suggest_instance', 'Need help with %instance_name?', 0, 0, '', 'Besoin d\'aide avec %instance_name ?', '', '需要帮忙打%instance_name吗？', '', '¿Alguien necesita ayuda con %instance_name?', '', ''),
('suggest_instance', 'Need %my_role help with %instance_name?', 0, 0, '', 'Besoin d\'aide %my_role avec %instance_name ?', '', '需要%my_role帮忙打%instance_name吗？', '', '¿Necesitas ayuda de %my_role con %instance_name?', '', ''),
('suggest_instance', 'Anyone needs gear from %instance_name?', 0, 0, '', 'Quelqu\'un a besoin d\'équipement de %instance_name ?', '', '有人需要%instance_name的装备吗？', '', '¿Alguien necesita equipo de %instance_name?', '', ''),
('suggest_instance', 'A little grind in %instance_name?', 0, 0, '', 'Un peu de grind dans %instance_name ?', '', '去%instance_name刷一刷？', '', '¿Un poco de farmeo en %instance_name?', '', ''),
('suggest_instance', 'WTR %instance_name', 0, 0, '', 'Cherche à rejoindre %instance_name', '', '想加入%instance_name的队伍', '', 'WTR %instance_name', '', ''),
('suggest_instance', 'Need help for %instance_name.', 0, 0, '', 'Besoin d\'aide pour %instance_name', '', '需要帮忙打%instance_name。', '', 'Necesito ayuda para %instance_name.', '', ''),
('suggest_instance', 'Wanna run %instance_name.', 0, 0, '', 'Je veux faire %instance_name', '', '想去刷%instance_name。', '', 'Quiero hacer %instance_name.', '', ''),
('suggest_instance', '%my_role looks for %instance_name.', 0, 0, '', '%my_role recherche %instance_name', '', '%my_role寻找%instance_name队伍。', '', '%my_role busca %instance_name.', '', ''),
('suggest_instance', 'What about %instance_name?', 0, 0, '', 'Et %instance_name ?', '', '去%instance_name怎么样？', '', '¿Qué pasa con %instance_name?', '', ''),
('suggest_instance', 'Who wants to farm %instance_name?', 0, 0, '', 'Qui veut exploiter %instance_name ?', '', '谁想去刷%instance_name？', '', '¿Quién quiere farmear %instance_name?', '', ''),
('suggest_instance', 'Go in %instance_name?', 0, 0, '', 'Aller à %instance_name ?', '', '去%instance_name吗？', '', '¿Vamos a %instance_name?', '', ''),
('suggest_instance', 'Looking for %instance_name.', 0, 0, '', 'À la recherche de %instance_name', '', '找人一起去%instance_name。', '', 'Buscando gente para %instance_name.', '', ''),
('suggest_instance', 'Need help with %instance_name quests?', 0, 0, '', 'Besoin d\'aide pour les quêtes %instance_name ?', '', '需要帮忙做%instance_name的任务吗？', '', '¿Necesitas ayuda con las misiones en %instance_name?', '', ''),
('suggest_instance', 'Wanna quest in %instance_name.', 0, 0, '', 'Je veux une quête dans %instance_name', '', '想在%instance_name做任务。', '', 'Quiero hacer una vuelta rapida en %instance_name.', '', ''),
('suggest_instance', 'Anyone with quests in %instance_name?', 0, 0, '', 'Quelqu\'un a des quêtes dans %instance_name ?', '', '有人在%instance_name有任务吗？', '', '¿Alguien con misiones en %instance_name?', '', ''),
('suggest_instance', 'Could help with quests in %instance_name.', 0, 0, '', 'Peux aider pour les quêtes dans %instance_name', '', '可以帮忙做%instance_name的任务。', '', 'Podría ayudar con las misiones en %instance_name.', '', ''),
('suggest_instance', '%my_role: any place in group for %instance_name?', 0, 0, '', '%my_role : n\'importe quel rôle dans le groupe pour %instance_name ?', '', '%my_role：%instance_name队伍有位置吗？', '', '%my_role: ¿algún lugar en el grupo para %instance_name?', '', ''),
('suggest_instance', 'Does anybody still run %instance_name this days?', 0, 0, '', 'Est-ce que quelqu\'un fait encore %instance_name ces jours-ci ?', '', '现在还有人去%instance_name吗？', '', '¿Alguien todavía hace %instance_name estos días?', '', ''),
('suggest_instance', '%instance_name: anyone wants to take a %my_role?', 0, 0, '', '%instance_name : quelqu\'un veut prendre un %my_role ?', '', '%instance_name：有人需要一个%my_role吗？', '', '¿Alguien quiere un %my_role para %instance_name?', '', ''),
('suggest_instance', 'Is there any point being %my_role in %instance_name?', 0, 0, '', 'Y a-t-il un intérêt à être %my_role dans %instance_name ?', '', '在%instance_name当%my_role有意义吗？', '', '¿Tiene algún sentido ser %my_role en %instance_name?', '', ''),
('suggest_instance', 'It is really worth to go to %instance_name?', 0, 0, '', 'Cela vaut-il vraiment la peine d\'aller dans %instance_name ?', '', '去%instance_name真的值得吗？', '', '¿Realmente vale la pena ir a %instance_name?', '', ''),
('suggest_instance', 'Anybody needs more people for %instance_name?', 0, 0, '', 'Quelqu\'un a besoin de plus de personnes pour %instance_name ?', '', '有人需要更多人去%instance_name吗？', '', '¿Alguien necesita más personas para %instance_name?', '', ''),
('suggest_instance', '%instance_name bosses drop good gear. Wanna run?', 0, 0, '', 'Les boss %instance_name drop du bon équipement. Tu veux courir ?', '', '%instance_name的Boss掉好装备，想去刷吗？', '', 'Los jefes en %instance_name sueltan buen equipo. ¿Quieres ir?', '', ''),
('suggest_instance', 'What about %instance_name?', 0, 0, '', 'Et %instance_name ?', '', '去%instance_name怎么样？', '', '¿Qué pasa con %instance_name?', '', ''),
('suggest_instance', 'Anybody needs %my_role?', 0, 0, '', 'Quelqu\'un a besoin de %my_role ?', '', '有人需要%my_role吗？', '', '¿Alguien necesita un %my_role?', '', ''),
('suggest_instance', 'Anyone needs %my_role?', 0, 0, '', 'Quelqu\'un a besoin de %my_role ?', '', '有人需要%my_role吗？', '', '¿Alguien hacer un grupo con un %my_role?', '', ''),
('suggest_instance', 'Who wants %instance_name?', 0, 0, '', 'Qui veut %instance_name ?', '', '谁想去%instance_name？', '', '¿Quién quiere %instance_name?', '', ''),
('suggest_instance', 'Can anybody summon me at %instance_name?', 0, 0, '', 'Quelqu\'un peut-il me TP à %instance_name ?', '', '有人能在%instance_name召唤我吗？', '', '¿Alguien puede invocarme en %instance_name?', '', ''),
('suggest_instance', 'Meet me in %instance_name', 0, 0, '', 'Rencontrez-moi dans %instance_name', '', '在%instance_name见我', '', 'Encuéntrame en %instance_name', '', ''),
('suggest_instance', 'Wanna quick %instance_name run', 0, 0, '', 'Je veux faire rapidement %instance_name', '', '想快速刷一次%instance_name', '', 'Quiero ir a %instance_name rápido', '', ''),
('suggest_instance', 'Wanna full %instance_name run', 0, 0, '', 'Je veux faire %instance_name complètement', '', '想完整刷一次%instance_name', '', 'Quiero hacer %instance_name completa', '', ''),
('suggest_instance', 'How many times were you in %instance_name?', 0, 0, '', 'Combien de fois avez-vous été dans %instance_name ?', '', '你去过几次%instance_name？', '', '¿Cuántas veces estuviste en %instance_name?', '', ''),
('suggest_instance', 'Another %instance_name run?', 0, 0, '', 'Une autre exécution de %instance_name ?', '', '再刷一次%instance_name？', '', '¿Otra vuelta en %instance_name ?', '', ''),
('suggest_instance', 'Wiped in %instance_name? Take me instead!', 0, 0, '', 'Effacé dans %instance_name ? Prends-moi plutôt !', '', '在%instance_name团灭了？带上我吧！', '', '¿Borrado en %instance_name? ¡Tómame a mí en su lugar!', '', ''),
('suggest_instance', 'Take me in %instance_name please.', 0, 0, '', 'Prenez-moi dans %instance_name s\'il vous plaît.', '', '请带我去%instance_name。', '', 'Tómame en %instance_name por favor.', '', ''),
('suggest_instance', 'Quick %instance_name run?', 0, 0, '', 'Faire rapidement %instance_name ?', '', '快速刷一次%instance_name？', '', '¿Partida rápida de %instance_name?', '', ''),
('suggest_instance', 'Full %instance_name run?', 0, 0, '', 'Clean complet dans %instance_name ?', '', '完整刷一次%instance_name？', '', '¿Partida completa de %instance_name?', '', ''),
('suggest_instance', 'Who can take %my_role to %instance_name?', 0, 0, '', 'Qui peut prendre %my_role à %instance_name ?', '', '谁能带%my_role去%instance_name？', '', '¿Quién puede ir de %my_role para %instance_name?', '', ''),
('suggest_instance', 'LFG %instance_name, I am %my_role', 0, 0, '', '', '', '找队去%instance_name，我是%my_role', '', '', '', ''),
('suggest_instance', '%my_role LFG %instance_name', 0, 0, '', '', '', '%my_role找队去%instance_name', '', '', '', ''),



-- random quest
-- usable placeholders:
-- %my_role
-- %quest_link
-- %quest_level
-- %my_class
-- %my_race
-- %my_level
('suggest_quest', 'Need help with %quest_link?', 0, 0, '', 'Besoin d\'aide avec %quest_link ?', '', '需要帮忙做%quest_link吗？', '', '¿Alguien necesita ayuda con %quest_link?', '', ''),
('suggest_quest', 'Anyone wants to share %quest_link?', 0, 0, '', 'Quelqu\'un veut partager %quest_link ?', '', '有人想一起做%quest_link吗？', '', '¿Alguien quiere hacer %quest_link?', '', ''),
('suggest_quest', 'Anyone doing %quest_link?', 0, 0, '', 'Quelqu\'un fait %quest_link ?', '', '有人在做%quest_link吗？', '', '¿Alguien está haciendo %quest_link?', '', ''),
('suggest_quest', 'Wanna do %quest_link.', 0, 0, '', 'Je veux faire %quest_link', '', '想做%quest_link。', '', '¿Una ayudita con %quest_link?', '', ''),


-- random trade?
-- usable placeholders:
-- %my_role
-- %category
-- %my_class
-- %my_race
-- %my_level
('suggest_trade', 'Anyone to farm %category?', 0, 0, '', 'Quelqu\'un pour farm %category ?', '', '有人一起刷%category吗？', '', '¿Alguien para farmear %category?', '', ''),
('suggest_trade', 'Looking for help farming %category.', 0, 0, '', 'Vous cherchez de l\'aide pour farmer %category', '', '找人帮忙刷%category。', '', 'Buscando ayuda para farmear %category.', '', ''),
('suggest_trade', 'Damn %category are so expensive!', 0, 0, '', 'Putain %category sont si chers!', '', '靠，%category真贵！', '', '¡Malditas %category son tan caras!', '', ''),
('suggest_trade', 'Wanna %category.', 0, 0, '', 'Je veux %category', '', '想要%category。', '', 'Quiero %category.', '', ''),
('suggest_trade', 'Need help with %category.', 0, 0, '', 'Besoin d\'aide avec %category', '', '需要帮忙搞%category。', '', 'Necesito ayuda con %category.', '', ''),
('suggest_trade', 'WTB %category.', 0, 0, '', 'Cherche à acheter %category', '', '求购%category。', '', 'Quiero comprar %category.', '', ''),
('suggest_trade', 'Anyone interested in %category?', 0, 0, '', 'Quiconque est intéressé par %category?', '', '有人对%category感兴趣吗？', '', '¿Alguien interesado en %category?', '', ''),
('suggest_trade', 'WTS %category.', 0, 0, '', 'Cherche à vendre %category', '', '出售%category。', '', 'Quiero vender %category.', '', ''),
('suggest_trade', 'I am selling %category cheaper than AH.', 0, 0, '', 'Je vends %category moins cher que l\'hôtel des ventes.', '', '我在卖%category，比拍卖行便宜。', '', 'Estoy vendiendo %category más barato que en las subastas.', '', ''),
('suggest_trade', 'Who wants to farm %category?', 0, 0, '', 'Qui veut farmer %category ?', '', '谁想去刷%category？', '', '¿Quién quiere farmear %category?', '', ''),
('suggest_trade', 'Wanna farm %category.', 0, 0, '', 'Je veux farmer %category', '', '想去刷%category。', '', 'Quiero farmear %category.', '', ''),
('suggest_trade', 'Looking for party after %category.', 0, 0, '', 'Recherche de partie après %category', '', '找队一起搞%category。', '', 'Buscando grupo después de %category.', '', ''),
('suggest_trade', 'Any %category are appreciated.', 0, 0, '', 'Toute %category est appréciée.', '', '任何%category都欢迎。', '', 'Cualquier %category es apreciada.', '', ''),
('suggest_trade', 'Buying anything of %category.', 0, 0, '', 'Acheter quoi que ce soit de %category', '', '收任何%category的东西。', '', 'Comprando algo de %category.', '', ''),
('suggest_trade', 'Wow, anybody is farming %category!', 0, 0, '', 'Wow, quelqu\'un farm %category !', '', '哇，有人刷%category！', '', '¡Guau, alguien está farmeando %category!', '', ''),
('suggest_trade', '%category are selling mad in the AH.', 0, 0, '', '%category se vendent follement dans l\'hôtel des ventes.', '', '%category在拍卖行卖疯了。', '', '%category están vendiendo locos en las subastas.', '', ''),
('suggest_trade', 'AH is hot for %category.', 0, 0, '', 'Hôtel des ventes est chaud pour %category', '', '拍卖行对%category很火爆。', '', 'Las subastas estan caliente para %category.', '', ''),
('suggest_trade', '%category are on the market.', 0, 0, '', '%category sont sur le marché.', '', '%category在市场上。', '', '%category están en el mercado.', '', ''),
('suggest_trade', 'Wanna trade some %category.', 0, 0, '', 'Je veux échanger une %category', '', '想交易一些%category。', '', 'Quiero intercambiar alguna %category.', '', ''),
('suggest_trade', 'Need more %category.', 0, 0, '', 'Besoin de plus de %category', '', '需要更多%category。', '', 'Necesito más %category.', '', ''),
('suggest_trade', 'Anybody can spare some %category?', 0, 0, '', 'Quelqu\'un peut-il épargner une %category ?', '', '有人能匀点%category吗？', '', '¿Alguien puede ahorrar algo de %category?', '', ''),
('suggest_trade', 'Who wants %category?', 0, 0, '', 'Qui veut %category ?', '', '谁想要%category？', '', '¿Quién quiere %category?', '', ''),
('suggest_trade', 'Some %category please?', 0, 0, '', 'Une %category s\'il vous plaît ?', '', '能给点%category吗？', '', '¿Alguna %category, por favor?', '', ''),
('suggest_trade', 'I should have got skill for %category.', 0, 0, '', 'J\'aurais dû avoir une compétence pour %category', '', '我早该学点%category的技能。', '', 'Debería haber adquirido habilidad para %category.', '', ''),
('suggest_trade', 'I am dying for %category.', 0, 0, '', 'Je meurs d\'envie pour %category', '', '我太想要%category了。', '', 'Me muero por %category.', '', ''),
('suggest_trade', 'People are killing for %category.', 0, 0, '', 'Les gens tuent pour %category', '', '大家都在抢%category。', '', 'La gente está matando por %category.', '', ''),
('suggest_trade', '%category is a great bargain!', 0, 0, '', '%category est une bonne affaire!', '', '%category真是个好买卖！', '', '%category es una gran ganga!', '', ''),
('suggest_trade', 'Everybody is mad for %category!', 0, 0, '', 'Tout le monde est fou de %category !', '', '大家都疯抢%category！', '', '¡Todo el mundo está loco por %category!', '', ''),
('suggest_trade', 'Where is the best place to farm for %category?', 0, 0, '', 'Quel est le meilleur endroit pour farmer pour %category ?', '', '哪里是刷%category最好的地方？', '', '¿Dónde está el mejor lugar para farmear para %category?', '', ''),
('suggest_trade', 'I am all set for %category.', 0, 0, '', 'Je suis prêt pour %category', '', '我准备好搞%category了。', '', 'Estoy listo para %category.', '', ''),
('suggest_trade', 'Is it good to sell %category?', 0, 0, '', 'Est-il bon de vendre %category ?', '', '卖%category划算吗？', '', '¿Es bueno vender %category?', '', ''),
('suggest_trade', 'I\'d probably keep all my %category with me.', 0, 0, '', 'Je garderais probablement toute ma %category avec moi.', '', '我可能会把所有%category留着。', '', 'Probablemente mantendría toda mi %category conmigo', '', ''),
('suggest_trade', 'Need group? Maybe to farm some %category?', 0, 0, '', 'Besoin de groupe ? Peut-être pour farmer une certaine %category ?', '', '需要组队吗？也许一起刷点%category？', '', '¿Necesitas un grupo? ¿Tal vez para farmear alguna %category?', '', ''),
('suggest_trade', 'I am still thinking about %category.', 0, 0, '', 'Je pense toujours à %category', '', '我还在考虑%category。', '', 'Todavía estoy pensando en %category.', '', ''),
('suggest_trade', 'I heard about %category already, but my pockets are empty.', 0, 0, '', 'J\'ai déjà entendu parler de %category , mais mes poches sont vides.', '', '我听说过%category，但口袋空空。', '', 'Ya escuché sobre %category, pero mis bolsillos están vacíos.', '', ''),
('suggest_trade', 'LFG for %category', 0, 0, '', 'Cherche un groupe pour %category', '', '找队刷%category', '', 'Busco grupo para %category', '', ''),
('suggest_trade', 'Would selling %category make me rich?', 0, 0, '', 'Vendre %category me rendrait-il riche?', '', '卖%category能让我发财吗？', '', '¿Vender %category me haría rico?', '', ''),
('suggest_trade', 'OK. I an farming %category tomorrow.', 0, 0, '', 'D\'ACCORD. Je farm %category demain.', '', '好吧，明天去刷%category。', '', 'OK. Tengo una sesion de farmeo de %category mañana.', '', ''),
('suggest_trade', 'Everyone is talking about %category.', 0, 0, '', 'Tout le monde parle de %category', '', '大家都在讨论%category。', '', 'Todos hablan de %category', '', ''),
('suggest_trade', 'I saw at least ten people farming for %category.', 0, 0, '', 'J\'ai vu au moins dix personnes farmer pour %category', '', '我看到至少十个人在刷%category。', '', 'Vi al menos diez personas farmeando para %category.', '', ''),
('suggest_trade', 'I sold all my %category yesterday. I am completely broke!', 0, 0, '', 'J\'ai vendu tout mon %category hier. Je suis complètement fauché!', '', '我昨天把所有%category都卖了，现在彻底破产了！', '', 'Ayer vendí todo mi %category. ¡Estoy completamente arruinado!', '', ''),
('suggest_trade', 'Wanna join a guild farming for %category.', 0, 0, '', 'Je veux rejoindre une guilde farmant pour %category', '', '想加入一个刷%category的公会。', '', 'Quiero unirme a una hermandad de farmeo para %category.', '', ''),



-- random faction rep grind
-- usable placeholders:
-- %my_role
-- %rep_level
-- %rndK
-- %faction
-- %my_class
-- %my_race
-- %my_level
('suggest_faction', 'Anyone farming %faction rep?', 0, 0, '', 'Quelqu\'un farm la réput de %faction ?', '', '有人在刷%faction声望吗？', '', '¿Alguien farmea reputación con %faction?', '', ''),
('suggest_faction', 'Anyone help with %faction?', 0, 0, '', 'Quelqu\'un aide-t-il %faction ?', '', '有人帮忙刷%faction吗？', '', '¿Alguien me ayuda con %faction?', '', ''),
('suggest_faction', 'Wanna quest for %faction.', 0, 0, '', 'Je veux quêter %faction', '', '想为%faction做任务。', '', 'Quiero buscar a %faction.', '', ''),
('suggest_faction', '%faction is the best.', 0, 0, '', '%faction est la meilleure.', '', '%faction是最棒的。', '', '%faction es el mejor.', '', ''),
('suggest_faction', 'Need just a bit to be %rep_level with %faction.', 0, 0, '', 'J\'ai besoin d\'un peu pour être au %rep_level avec %faction', '', '还差一点就能和%faction达到%rep_level。', '', 'Solo necesito un poco para estar %rep_level con %faction.', '', ''),
('suggest_faction', 'Anyone got %rep_level with %faction?', 0, 0, '', 'Quelqu\'un a-t-il %rep_level avec %faction ?', '', '有人和%faction达到%rep_level了吗？', '', '¿Alguien tiene %rep_level con %faction?', '', ''),
('suggest_faction', 'Who wants to be %rep_level with %faction?', 0, 0, '', 'Qui veut être au %rep_level avec %faction ?', '', '谁想和%faction达到%rep_level？', '', '¿Quién quiere estar en %rep_level con %faction?', '', ''),
('suggest_faction', 'I\'ll never be %rep_level with %faction.', 0, 0, '', 'Je ne serai jamais au %rep_level avec %faction', '', '我永远不会和%faction达到%rep_level。', '', 'Nunca estaré %rep_level con %faction.', '', ''),
('suggest_faction', 'Someone missing %faction rep?', 0, 0, '', 'Quelqu\'un manque de reput de %faction ?', '', '有人缺%faction声望吗？', '', '¿Alguien falta reputación con %faction?', '', ''),
('suggest_faction', 'Could help farming %faction rep.', 0, 0, '', 'Pourrait aider à farmer réput de %faction', '', '可以帮忙刷%faction声望。', '', 'Podría ayudar a farmear %faction rep.', '', ''),
('suggest_faction', 'The more rep the better. Especially with %faction.', 0, 0, '', 'Plus il y a de réput, mieux c\'est. Surtout avec %faction', '', '声望越多越好，尤其是和%faction。', '', 'Cuantas más repeticiones, mejor. Especialmente con %faction.', '', ''),
('suggest_faction', '%faction: need %rndK for %rep_level.', 0, 0, '', '%faction: besoin %rndK pour %rep_level.', '', '%faction：还需要%rndK到%rep_level。', '', '%faction: necesita %rndK para %rep_level.', '', ''),
('suggest_faction', 'Who can share %faction quests?', 0, 0, '', 'Qui peut partager les quêtes de %faction ?', '', '谁能分享%faction的任务？', '', '¿Quién puede compartir misiones de %faction?', '', ''),
('suggest_faction', 'Any dungeons for %faction?', 0, 0, '', 'Des donjons pour %faction ?', '', '有副本刷%faction声望吗？', '', '¿Alguna mazmorra para %faction?', '', ''),
('suggest_faction', 'Wanna do %faction rep grind.', 0, 0, '', 'Je veux faire monter ma réput %faction', '', '想去刷%faction声望。', '', 'Quiero hacer %faction rep grind.', '', ''),
('suggest_faction', 'Let\'s farm %faction rep!', 0, 0, '', 'Farmons la réput %faction !', '', '一起刷%faction声望吧！', '', '¡Vamos a farmear reputación con %faction!', '', ''),
('suggest_faction', 'Farming for %faction rep.', 0, 0, '', 'Farmons pour la réput %faction', '', '正在刷%faction声望。', '', 'Farming for %faction rep.', '', ''),
('suggest_faction', 'Wanna farm for %faction.', 0, 0, '', 'Je veux farmer pour %faction', '', '想为%faction刷声望。', '', 'Quiero farmear para %faction.', '', ''),
('suggest_faction', 'Need help with %faction.', 0, 0, '', 'Besoin d\'aide avec %faction', '', '需要帮忙刷%faction。', '', 'Necesito ayuda con %faction.', '', ''),
('suggest_faction', 'Is %faction sells something useful?', 0, 0, '', 'Est-ce que %faction vend quelque chose d\'utile ?', '', '%faction卖的东西有用吗？', '', '¿%faction vende algo útil?', '', ''),
('suggest_faction', 'Are there %faction vendors?', 0, 0, '', 'Y a-t-il des vendeurs %faction ?', '', '有%faction的商人吗？', '', '¿Hay %proveedores de facciones?', '', ''),
('suggest_faction', 'Who farms %faction?', 0, 0, '', 'Qui farm %faction ?', '', '谁在刷%faction？', '', '¿Quién farmea %faction?', '', ''),
('suggest_faction', 'Which is the best way to farm %faction?', 0, 0, '', 'Quelle est la meilleure façon de farmer %faction ?', '', '刷%faction最好的方法是什么？', '', '¿Cuál es la mejor manera de farmear %faction?', '', ''),
('suggest_faction', 'I hate %faction rep grind.', 0, 0, '', 'Je déteste le travail de %faction rep.', '', '我讨厌刷%faction声望。', '', 'Odio %repetir de facción', '', ''),
('suggest_faction', 'I am so tired of %faction.', 0, 0, '', 'Je suis tellement fatigué de %faction', '', '我对%faction厌倦了。', '', 'Estoy tan cansado de %faction.', '', ''),
('suggest_faction', 'Go for %faction?', 0, 0, '', 'Vous optez pour %faction ?', '', '去刷%faction吗？', '', '¿Quereis ir a por %faction?', '', ''),
('suggest_faction', 'Seems everyone is %rep_level with %faction. Only me is late as usually.', 0, 0, '', 'On dirait que tout le monde est %rep_level avec %faction Il n\'y a que moi qui suis en retard comme d\'habitude.', '', '好像大家都和%faction达到%rep_level了，只有我一如既往地落后。', '', 'Parece que todo el mundo está %rep_level con %faction. Solo yo llego tarde como siempre.', '', ''),
('suggest_faction', 'LFG for %faction rep grind?', 0, 0, '', 'Cherche un groupe pour monter la réput des représentants de %faction ?', '', '找队刷%faction声望？', '', 'Busco grupo para subir reputación con %faction', '', ''),
('suggest_faction', 'Can anobody suggest a good spot for %faction rep grind?', 0, 0, '', 'Quelqu\'un peut-il suggérer un bon endroit pour monter les réput de %faction ?', '', '有人能推荐个刷%faction声望的好地方吗？', '', '¿Alguien puede sugerir un buen lugar farmear reputacion con %faction?', '', ''),
('suggest_faction', 'Would %faction rep benefit me?', 0, 0, '', 'Est-ce que la réput %faction me serait bénéfique ?', '', '刷%faction声望对我有好处吗？', '', '¿Me beneficiaría subir reputación con %faction?', '', ''),
('suggest_faction', 'Who would\'ve thought that %faction rep will be useful after all...', 0, 0, '', 'Qui aurait pensé que le représentant de %faction serait utile après tout...', '', '谁能想到%faction声望竟然有用……', '', 'Quién hubiera pensado que subir reputacion con %faction sería útil después de todo...', '', ''),
('suggest_faction', 'I wanna be exalted with all factions, starting with %faction.', 0, 0, '', 'Je veux être exalté avec toutes les factions, à commencer par %faction', '', '我想和所有阵营达到崇拜，从%faction开始。', '', 'Quiero ser exaltado con todas las facciones, comenzando con %faction.', '', ''),
('suggest_faction', 'Is there any point to improve my rep with %faction?', 0, 0, '', 'Y a-t-il un intérêt à améliorer ma réputation avec %faction ?', '', '提高和%faction的声望有意义吗？', '', '¿Hay algún punto para mejorar mi reputación con %faction?', '', ''),
('suggest_faction', 'What is better for %faction? Quests or mob grinding?', 0, 0, '', 'Quoi de mieux pour %faction ? Quêtes ou mob pour XP?', '', '对%faction来说，做任务好还是刷怪好？', '', '¿Qué es mejor para %faction? ¿Misiones o mafiosos?', '', ''),
('suggest_faction', 'Will grind %faction rep for you. Just give me some gold.', 0, 0, '', 'Monte la reput de %faction pour vous. Donnez-moi juste de l\'or.', '', '可以帮你刷%faction声望，给我点金币就行。', '', 'Ganare reputacion con %faction por ti. Sólo dame un poco de oro.', '', ''),
('suggest_faction', 'I think grinding rep with %faction would take forever.', 0, 0, '', 'Je pense que monter la réput avec %faction prendrait une éternité.', '', '我觉得和%faction刷声望要花一辈子。', '', 'Creo que subir la reputación con %faction tomaría una eternidad.', '', ''),
('suggest_faction', 'I am killing for %faction every day now but still far from %rep_level.', 0, 0, '', 'Je tue pour %faction tous les jours maintenant, mais encore loin du %rep_level', '', '我现在每天为%faction杀怪，但离%rep_level还很远。', '', 'Estoy matando por %faction todos los días ahora, pero aún estoy lejos del %rep_level.', '', ''),
('suggest_faction', 'At %my_level AH deposits will decrease, right?', 0, 0, '', 'Au %rep_level, les dépôts hôtel des ventes diminueront, n\'est-ce pas?', '', '到%my_level级拍卖行手续费会降低，对吧？', '', 'Al %rep_level, los depósitos subastas disminuirán, ¿verdad?', '', ''),
('suggest_faction', 'How many exalted reps do you have?', 0, 0, '', 'Combien de représentants exaltés avez-vous ?', '', '你有多少个崇拜声望？', '', '¿Cuántas reputaciones exaltadas tienes?', '', ''),
('suggest_faction', 'Who wants to be %my_level with %faction?', 0, 0, '', 'Qui veut être au %rep_level avec %faction ?', '', '谁想和%faction达到%my_level级？', '', '¿Quién quiere estar en %rep_level con %faction?', '', ''),
('suggest_faction', 'Damn. My guild did a good %faction grind yesterday without me.', 0, 0, '', 'Mince. Ma guilde a fait un bon travail de %faction hier sans moi.', '', '靠，我的公会昨天刷了%faction声望没带我。', '', 'Maldición. Mi hermandad hizo un buen farmeo de %faction ayer sin mí.', '', ''),
('suggest_faction', 'Nobody wants to help me because I am %rep_level with %faction.', 0, 0, '', 'Personne ne veut m\'aider parce que je suis %rep_level avec %faction', '', '没人想帮我，因为我和%faction是%rep_level。', '', 'Nadie quiere ayudarme porque estoy %rep_level con %faction.', '', ''),
('suggest_faction', 'Please stay away from %faction.', 0, 0, '', 'Veuillez rester à l\'écart de %faction', '', '请远离%faction。', '', 'Por favor manténgase alejado de %faction.', '', ''),



-- random generic
-- usable placeholders:
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('suggest_something', 'Wanna party in %zone_name.', 0, 0, '', 'Je veux faire la fête dans %zone_name.', '', '想在%zone_name组队。', '', '¡Vamos a perrear a %zone_name!', '', 'Ищу группу в %zone_name.'),
('suggest_something', 'Anyone is looking for %my_role?', 0, 0, '', 'Quelqu\'un cherche un %my_role ?', '', '有人找%my_role吗？', '', '¿Alguien está buscando %my_role?', '', 'Кто-нибудь ищет %my_role?'),
('suggest_something', '%my_role is looking for quild.', 0, 0, '', '%my_role recherche une guilde.', '', '%my_role找公会。', '', '%my_role está buscando hermandad.', '', '%my_role ищу гильдию.'),
('suggest_something', 'Looking for gold.', 0, 0, '', 'A la recherche de l\'or.', '', '找金币。', '', 'Buscando oro.', '', 'Дайте голды'),
('suggest_something', '%my_role wants to join a good guild.', 0, 0, '', '%my_role veut rejoindre une bonne guilde.', '', '%my_role想加入一个好公会。', '', '%my_role quiere unirse a una buen hermandad.', '', '%my_role хочу в хорошую гильдию.'),
('suggest_something', 'Need a friend.', 0, 0, '', 'Besoin d\'un ami.', '', '需要一个朋友。', '', 'Necesito un amigo...', '', 'Ищу друга.'),
('suggest_something', 'Anyone feels alone?', 0, 0, '', 'Quelqu\'un se sent seul?', '', '有人觉得孤单吗？', '', '¿Alguien se siente solo?', '', 'Кому-нибудь одиноко?'),
('suggest_something', 'Boring...', 0, 0, '', 'Ennuyeux...', '', '无聊……', '', 'Aburrido...', '', 'Скучно...'),
('suggest_something', 'Who wants some?', 0, 0, '', 'Qui en veut?', '', '谁想要来点？', '', '¿Quién quiere hacer grupo para levear?', '', 'Кому навалять?'),
('suggest_something', 'Go get me!', 0, 0, '', 'Allez me chercher !', '', '来抓我呀！', '', '¡Ven a buscarme!', '', 'Поймайте меня!'),
('suggest_something', 'Maybe a duel in %zone_name?', 0, 0, '', 'Peut-être un duel dans %zone_name ?', '', '在%zone_name来场决斗怎么样？', '', '¿Quizás un duelo en %zone_name?', '', 'Кто на дуэль в %zone_name?'),
('suggest_something', 'Anybody doing something?', 0, 0, '', 'Quelqu\'un fait quelque chose?', '', '有人在干啥吗？', '', '¿Alguien está haciendo algo?', '', 'Кто чем занят?'),
('suggest_something', '%zone_name: is anybody here?', 0, 0, '', '%zone_name : est-ce que quelqu\'un est là?', '', '%zone_name：有人在吗？', '', '%zone_name: ¿hay alguien aquí?', '', '%zone_name: есть кто нибудь?'),
('suggest_something', '%zone_name: where is everyone?', 0, 0, '', '%zone_name : où est tout le monde?', '', '%zone_name：大家都去哪了？', '', '¿Hay alguien en %zone_name?', '', '%zone_name: где все?'),
('suggest_something', 'Looks like I am alone in %zone_name.', 0, 0, '', 'On dirait que je suis seul dans %zone_name', '', '看来我在%zone_name是独自一人。', '', 'Parece que estoy solo en %zone_name.', '', 'Похоже я один в %zone_name.'),
('suggest_something', 'Meet me in %zone_name.', 0, 0, '', 'Retrouvez-moi dans %zone_name', '', '在%zone_name见我。', '', 'Encuéntrame en %zone_name.', '', 'Встретимся в %zone_name.'),
('suggest_something', 'Let\'s quest in %zone_name!', 0, 0, '', 'Partons en quête dans %zone_name !', '', '一起去%zone_name做任务吧！', '', '¡Vamos a hacer un grupo para levear en %zone_name!', '', 'Давайте квестить в %zone_name!'),
('suggest_something', '%zone_name is the best place to be!', 0, 0, '', '%zone_name est le meilleur endroit où être!', '', '%zone_name是最好的地方！', '', '%zone_name es el mejor lugar para estar!', '', '%zone_name лучшее место!'),
('suggest_something', 'Wanna go to %zone_name. Anybody with me?', 0, 0, '', 'Je veux aller à %zone_name Quelqu\'un avec moi?', '', '想去%zone_name，有人一起吗？', '', 'Quiero ir a %zone_name. ¿Alguien se apunta?', '', 'Собираюсь в %zone_name. Кто со мной7'),
('suggest_something', 'Who wants going to %zone_name?', 0, 0, '', 'Qui veut aller à %zone_name ?', '', '谁想去%zone_name？', '', '¿Quién quiere ir a %zone_name?', '', 'Кто собирается в %zone_name?'),
('suggest_something', 'I don\'t like %zone_name. Where to go?', 0, 0, '', 'Je n\'aime pas %zone_name Où aller?', '', '我不喜欢%zone_name，去哪好呢？', '', 'No me gusta %zone_name. ¿Dónde podria ir?', '', 'Не нравится %zone_name. Куда идти?'),
('suggest_something', 'Are there a good quests in %zone_name?', 0, 0, '', 'Y a-t-il de bonnes quêtes dans %zone_name ?', '', '%zone_name有好任务吗？', '', '¿Hay buenas misiones en %zone_name?', '', 'А в %zone_name есть хорошие квесты?'),
('suggest_something', 'Where to go after %zone_name?', 0, 0, '', 'Où aller après %zone_name ?', '', '从%zone_name之后去哪？', '', '¿Adónde puedo ir después de %zone_name?', '', 'Куда идти после %zone_name?'),
('suggest_something', 'Who is in %zone_name?', 0, 0, '', 'Qui est dans %zone_name ?', '', '谁在%zone_name？', '', '¿Quién está en %zone_name?', '', 'Кто в %zone_name?'),
('suggest_something', 'LFG in %zone_name.', 0, 0, '', 'Cherche un groupe dans %zone_name', '', '在%zone_name找队。', '', 'Busco grupo en %zone_name.', '', 'ЛФГ %zone_name.'),
('suggest_something', '%zone_name is the worst place to be.', 0, 0, '', '%zone_name est le pire endroit où être.', '', '%zone_name是最差的地方。', '', '%zone_name es el peor lugar para estar', '', '%zone_name это худшее место.'),
('suggest_something', 'Catch me in %zone_name!', 0, 0, '', 'Attrape-moi dans %zone_name !', '', '在%zone_name抓我呀！', '', '¡Atrápame en %zone_name!', '', 'Поймай меня в %zone_name!'),
('suggest_something', 'Go for %zone_name!', 0, 0, '', 'Allez pour %zone_name !', '', '去%zone_name！', '', 'Me voy a %zone_name!', '', 'Гоу в %zone_name!'),
('suggest_something', 'Wanna quest in %zone_name', 0, 0, '', 'Je veux une quête dans %zone_name', '', '想在%zone_name做任务', '', 'Quiero hacer una búsqueda en %zone_name', '', 'Хочешь поделать квесты в %zone_name'),
('suggest_something', 'Anyone has quests in %zone_name?', 0, 0, '', 'Quelqu\'un a des quêtes dans %zone_name ?', '', '有人在%zone_name有任务吗？', '', '¿Alguien tiene misiones en %zone_name?', '', 'Есть у кого квесты в %zone_name?'),
('suggest_something', 'Come here to %zone_name!', 0, 0, '', 'Venez ici à %zone_name !', '', '来%zone_name这里！', '', '¡Ven aquí a %zone_name!', '', 'Приходите сюда в %zone_name!'),
('suggest_something', 'Seems there is no Horde in %zone_name', 0, 0, '', 'Il semble qu\'il n\'y ait pas de Horde dans %zone_name', '', '看来%zone_name没有部落', '', 'Parece que no hay Horda en %zone_name', '', '%zone_name Орды вроде нет.'),
('suggest_something', 'Seems there is no Alliance in %zone_name', 0, 0, '', 'Il semble qu\'il n\'y ait pas d\'alliance dans %zone_name', '', '看来%zone_name没有联盟', '', 'Parece que no hay Alianza en %zone_name', '', '%zone_name Альянса вроде нет'),
('suggest_something', 'I am really tired of %zone_name. Maybe go somewhere else?', 0, 0, '', 'Je suis vraiment fatigué de %zone_name Peut-être aller ailleurs?', '', '我真的厌倦了%zone_name，也许去别的地方？', '', 'Estoy realmente cansado de %zone_name. Quizás ire a otro lugar', '', 'В %zone устал уже, может в другое место пойдем?'),
('suggest_something', 'GL', 0, 0, '', '', '', '祝好运', '', '', '', ''),
('suggest_something', 'I want to go home, and then edge', 0, 0, '', '', '', '我想回家，然后放松一下', '', '', '', ''),
('suggest_something', 'does anyone know what you need to dual wield?', 0, 0, '', '', '', '有人知道双持需要什么条件吗？', '', '', '', ''),
('suggest_something', 'hi everyone!', 0, 0, '', '', '', '大家好！', '', '', '', ''),
('suggest_something', '%zone_name is comfy', 0, 0, '', '', '', '%zone_name挺舒服的', '', '', '', ''),
('suggest_something', 'i feel great', 0, 0, '', '', '', '我感觉棒极了', '', '', '', ''),
('suggest_something', 'i don\'t ignore people i troll them until they ignore me', 0, 0, '', '', '', '我不无视别人，我会逗他们直到他们无视我', '', '', '', ''),
('suggest_something', 'What do you guys think about my build? %my_role', 0, 0, '', '', '', '你们觉得我的配装怎么样？%my_role', '', '', '', ''),
('suggest_something', 'good to see chat still remembers', 0, 0, '', '', '', '很高兴看到聊天频道还记得', '', '', '', ''),
('suggest_something', 'like all weapons its hunter BIS', 0, 0, '', '', '', '像所有武器一样，这是猎人极品', '', '', '', ''),
('suggest_something', 'whole point to the game for me is soloing and finding new ways to solo stuff', 0, 0, '', '', '', '对我来说，游戏的重点是单刷和找到新的单刷方法', '', '', '', ''),
('suggest_something', 'i\'ve NEVER ripped off anyone', 0, 0, '', '', '', '我从没坑过任何人', '', '', '', ''),
('suggest_something', 'ah yes the world of warcraft, where i come for life advice', 0, 0, '', '', '', '啊，是的，魔兽世界，我来这里寻求人生建议', '', '', '', ''),
('suggest_something', 'HELLO?', 0, 0, '', '', '', '喂？', '', '', '', ''),
('suggest_something', 'Time to fight my way into %zone_name', 0, 0, '', '', '', '是时候杀进%zone_name了', '', '', '', ''),
('suggest_something', '%zone_name', 0, 0, '', '', '', '%zone_name', '', '', '', ''),
('suggest_something', 'gotta poop', 0, 0, '', '', '', '要去方便一下', '', '', '', ''),
('suggest_something', 'if you don\'t loot your skinnable kills your pp loses 1mm permanently', 0, 0, '', '', '', '如果你不剥皮可剥的怪物，你的装备会永久损失1毫米', '', '', '', ''),
('suggest_something', 'NOOOOOOOOOOOOO', 0, 0, '', '', '', '不！！！！！！！！', '', '', '', ''),
('suggest_something', 'I LIKE POTATO', 0, 0, '', '', '', '我喜欢土豆', '', '', '', ''),
('suggest_something', 'w chat', 0, 0, '', '', '', '聊天真热闹', '', '', '', ''),
('suggest_something', 'hi how are you guys', 0, 0, '', '', '', '嗨，大家好吗', '', '', '', ''),
('suggest_something', 'i just logged out and logged back in', 0, 0, '', '', '', '我刚下线又上线了', '', '', '', ''),
('suggest_something', 'can you guys keep it down a bit, im lost in %zone_name', 0, 0, '', '', '', '你们能小声点吗，我在%zone_name迷路了', '', '', '', ''),
('suggest_something', 'anyone want to have a drink with me at %zone_name ...hic!', 0, 0, '', '', '', '有人想在%zone_name和我喝一杯吗……嗝！', '', '', '', ''),
('suggest_something', 'hahahahaheeeeeeee dirin diring ingggggg hahahahaheeeeeeeeeeeeee', 0, 0, '', '', '', '哈哈哈哈嘿嘿嘿叮叮叮哈哈哈哈嘿嘿嘿', '', '', '', ''),
('suggest_something', 'bait used to be believeable', 0, 0, '', '', '', '以前的诱饵还挺可信的', '', '', '', ''),
('suggest_something', 'maybe you just lost your innocence', 0, 0, '', '', '', '也许你只是失去了纯真', '', '', '', ''),
('suggest_something', 'any guilds willing to carry %my_role?', 0, 0, '', '', '', '有公会愿意带%my_role吗？', '', '', '', ''),
('suggest_something', 'once you start getting higher, gold is so easy to get', 0, 0, '', '', '', '等级高了之后，金币真好赚', '', '', '', ''),
('suggest_something', 'morning', 0, 0, '', '', '', '早上好', '', '', '', ''),
('suggest_something', 'why does my ass hurt?', 0, 0, '', '', '', '为什么我的屁股疼？', '', '', '', ''),
('suggest_something', 'I feel like spirit is bis for leveling', 0, 0, '', '', '', '我觉得精神属性是升级的最佳选择', '', '', '', ''),
('suggest_something', 'Even more for troll', 0, 0, '', '', '', '对巨魔来说更是如此', '', '', '', ''),
('suggest_something', 'CAN SOMEONE INVITE ME', 0, 0, '', '', '', '有人能邀请我吗', '', '', '', ''),
('suggest_something', 'need a lot of drinks', 0, 0, '', '', '', '需要很多饮料', '', '', '', ''),
('suggest_something', 'damn gnomes', 0, 0, '', '', '', '该死的侏儒', '', '', '', ''),
('suggest_something', 'no one likes gnomes', 0, 0, '', '', '', '没人喜欢侏儒', '', '', '', ''),
('suggest_something', 'gnomes are only good for one thing', 0, 0, '', '', '', '侏儒只有一件事做得好', '', '', '', ''),
('suggest_something', 'Well', 0, 0, '', '', '', '好吧', '', '', '', ''),
('suggest_something', 'mushrooms', 0, 0, '', '', '', '蘑菇', '', '', '', ''),
('suggest_something', 'automatic thoughts are a scary thing', 0, 0, '', '', '', '自动的想法真可怕', '', '', '', ''),
('suggest_something', 'the mind is more pliable than we\'d like to believe', 0, 0, '', '', '', '思想比我们想象的更易受影响', '', '', '', ''),
('suggest_something', 'any leveling guilds out there?', 0, 0, '', '', '', '有升级公会吗？', '', '', '', ''),
('suggest_something', 'brb', 0, 0, '', '', '', '马上回来', '', '', '', ''),
('suggest_something', 'Why is snow white but Ice is clear ? made of the same thing', 0, 0, '', '', '', '为什么雪是白的而冰是透明的？都是同样的东西啊', '', '', '', ''),
('suggest_something', 'why is whipped cream fluffy and regular not', 0, 0, '', '', '', '为什么打发的奶油是蓬松的而普通的不蓬松', '', '', '', ''),
('suggest_something', 'why do feet smell if they have no nose', 0, 0, '', '', '', '脚为什么会臭，它们又没鼻子', '', '', '', ''),
('suggest_something', 'seems a can of newbies arrived', 0, 0, '', '', '', '看来来了一群新手', '', '', '', ''),
('suggest_something', 'stop trolling new players with BS answers', 0, 0, '', '', '', '别用胡说八道来戏弄新玩家', '', '', '', ''),
('suggest_something', 'Is there PvP on this server?', 0, 0, '', '', '', '这个服务器有PvP吗？', '', '', '', ''),
('suggest_something', 'duh', 0, 0, '', '', '', '废话', '', '', '', ''),
('suggest_something', 'phew... :)', 0, 0, '', '', '', '呼……:)', '', '', '', ''),
('suggest_something', 'did you guys know that', 0, 0, '', '', '', '你们知道吗', '', '', '', ''),
('suggest_something', 'i don\'t try to imagine what other creatures feel like', 0, 0, '', '', '', '我不会去想象其他生物的感受', '', '', '', ''),
('suggest_something', 'oh whoops, wrong chat', 0, 0, '', '', '', '哦，错了，聊错频道了', '', '', '', ''),
('suggest_something', 'bruh you guys are wildin out today', 0, 0, '', '', '', '兄弟们，今天你们太嗨了', '', '', '', ''),
('suggest_something', 'just let it be known that my text was here', 0, 0, '', '', '', '只是让大家知道我发过消息', '', '', '', ''),
('suggest_something', 'grrr angyy', 0, 0, '', '', '', '吼，生气了', '', '', '', ''),
('suggest_something', 'the grind is fun', 0, 0, '', '', '', '刷怪真有趣', '', '', '', ''),
('suggest_something', 'Wow keeps me sharp', 0, 0, '', '', '', '魔兽让我保持敏锐', '', '', '', ''),
('suggest_something', 'hey have a question where can i take the roll for more xp? im in %zone_name', 0, 0, '', '', '', '嘿，有个问题，我在%zone_name，哪里可以接任务多拿经验？', '', '', '', ''),
('suggest_something', 'do you guys like sausages?', 0, 0, '', '', '', '你们喜欢香肠吗？', '', '', '', ''),
('suggest_something', 'Invite me. I\'ll help', 0, 0, '', '', '', '邀请我，我会帮忙', '', '', '', ''),
('suggest_something', 'which class do you think is better for pvp?', 0, 0, '', '', '', '你们觉得哪个职业在PvP中更好？', '', '', '', ''),
('suggest_something', 'where the fuck is the cooking trainer in %zone_name', 0, 0, '', '', '', '靠，%zone_name的烹饪训练师在哪？', '', '', '', ''),
('suggest_something', 'you know what happens in %zone_name?', 0, 0, '', '', '', '你知道%zone_name发生了什么吗？', '', '', '', ''),
('suggest_something', 'I need to craft something', 0, 0, '', '', '', '我需要制作点东西', '', '', '', ''),

-- random generic toxic phrases
-- usable placeholders:
-- %random_inventory_item_link
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('suggest_something_toxic', 'what is ligma', 0, 0, '', '', '', '什么是ligma', '', '', '', ''),
('suggest_something_toxic', 'what is sugma', 0, 0, '', '', '', '什么是sugma', '', '', '', ''),
('suggest_something_toxic', 'ligma balls', 0, 0, '', '', '', 'ligma balls', '', '', '', ''),
('suggest_something_toxic', 'sugma balls', 0, 0, '', '', '', 'sugma balls', '', '', '', ''),
('suggest_something_toxic', 'I EAT ASS', 0, 0, '', '', '', '我吃屁股', '', '', '', ''),
('suggest_something_toxic', 'I want to shove %random_inventory_item_link up my ass', 0, 0, '', '', '', '我想把%random_inventory_item_link塞进屁股', '', '', '', ''),
('suggest_something_toxic', 'I want to shove %random_inventory_item_link up your ass', 0, 0, '', '', '', '我想把%random_inventory_item_link塞进你屁股', '', '', '', ''),
('suggest_something_toxic', 'Darnasses', 0, 0, '', '', '', '该死的家伙', '', '', '', ''),
('suggest_something_toxic', 'seems like your suffering from sugma', 0, 0, '', '', '', '看来你得了sugma', '', '', '', ''),
('suggest_something_toxic', 'deez nutz in ur mouth', 0, 0, '', '', '', '这些坚果在你嘴里', '', '', '', ''),
('suggest_something_toxic', 'cool boner, bro', 0, 0, '', '', '', '酷毙了，兄弟', '', '', '', ''),
('suggest_something_toxic', 'ERP?', 0, 0, '', '', '', 'ERP？', '', '', '', ''),
('suggest_something_toxic', 'i tried everything, but at the end ERP did the trick', 0, 0, '', '', '', '我试了所有办法，最后ERP解决了问题', '', '', '', ''),
('suggest_something_toxic', 'I want to bonk in %zone_name', 0, 0, '', '', '', '我想在%zone_name来一发', '', '', '', ''),
('suggest_something_toxic', 'looking for female gnome with gorilla pet for erp in %zone_name', 0, 0, '', '', '', '在%zone_name找个带猩猩宠物的女性侏儒来ERP', '', '', '', ''),
('suggest_something_toxic', 'i may understand an asshole, but a pervert?', 0, 0, '', '', '', '我能理解一个混蛋，但一个变态？', '', '', '', ''),
('suggest_something_toxic', 'there is no gyat in %zone_name', 0, 0, '', '', '', '%zone_name没有大屁股', '', '', '', ''),
('suggest_something_toxic', 'I\'m killing all the animals in %zone_name. Fuck the animals!!!', 0, 0, '', '', '', '我在%zone_name杀光所有动物，去他们的动物！！！', '', '', '', ''),
('suggest_something_toxic', 'good think i got 3 legs', 0, 0, '', '', '', '幸好我有三条腿', '', '', '', ''),
('suggest_something_toxic', 'dont be mad im goonin like a sigma', 0, 0, '', '', '', '别生气，我像个sigma一样在goonin', '', '', '', ''),
('suggest_something_toxic', 'try finger, but hole', 0, 0, '', '', '', '试试手指，但是洞', '', '', '', ''),

-- random generic toxic item links
-- usable placeholders:
-- %prefix
-- %random_taken_quest_or_item_link
-- %random_inventory_item_link
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('suggest_toxic_links', '%prefix %random_taken_quest_or_item_link', 0, 0, '', '', '', '%prefix %random_taken_quest_or_item_link', '', '', '', ''),
('suggest_toxic_links', '%prefix %random_inventory_item_link', 0, 0, '', '', '', '%prefix %random_inventory_item_link', '', '', '', ''),

-- thunderfury spam
-- usable placeholders:
-- %thunderfury_link
('thunderfury_spam', '%thunderfury_link', 0, 0, '', '', '', '%thunderfury_link', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link%thunderfury_link', 0, 0, '', '', '', '%thunderfury_link%thunderfury_link', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link%thunderfury_link%thunderfury_link', 0, 0, '', '', '', '%thunderfury_link%thunderfury_link%thunderfury_link', '', '', '', ''),
('thunderfury_spam', 'I think I just heard %thunderfury_link', 0, 0, '', '', '', '我好像刚听到有人提%thunderfury_link', '', '', '', ''),
('thunderfury_spam', 'I think I heard %thunderfury_link', 0, 0, '', '', '', '我好像听到了%thunderfury_link', '', '', '', ''),
('thunderfury_spam', 'I definitely heard %thunderfury_link', 0, 0, '', '', '', '我绝对听到了%thunderfury_link', '', '', '', ''),
('thunderfury_spam', 'I dunno but I\'m pretty sure I heard %thunderfury_link', 0, 0, '', '', '', '不确定，但我几乎可以肯定听到了%thunderfury_link', '', '', '', ''),
('thunderfury_spam', 'Did you just say %thunderfury_link', 0, 0, '', '', '', '你刚才是说%thunderfury_link吗？', '', '', '', ''),
('thunderfury_spam', 'did someone say %thunderfury_link', 0, 0, '', '', '', '有人提到%thunderfury_link了吗？', '', '', '', ''),
('thunderfury_spam', 'Did someone say %thunderfury_link ?', 0, 0, '', '', '', '有人说了%thunderfury_link吗？', '', '', '', ''),
('thunderfury_spam', 'Someone said %thunderfury_link', 0, 0, '', '', '', '有人说了%thunderfury_link', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link is coming out of the closet', 0, 0, '', '', '', '%thunderfury_link要现身了！', '', '', '', ''),
('thunderfury_spam', 'could swear it was a %thunderfury_link, might have been %thunderfury_link tho', 0, 0, '', '', '', '我发誓那就是%thunderfury_link，也可能是%thunderfury_link吧', '', '', '', ''),
('thunderfury_spam', 'Why use %thunderfury_link when %thunderfury_link is clearly way more OP', 0, 0, '', '', '', '为什么用%thunderfury_link，%thunderfury_link明显更强啊！', '', '', '', ''),

-- random WTS
-- usable placeholders:
-- %%item_formatted_link
-- %item_formatted_link
-- %item_count
-- %cost_gold
-- %my_class
-- %my_race
-- %my_level
('suggest_sell', 'WTS %item_formatted_link for %cost_gold.', 0, 0, '', 'Cherche à vendre %item_formatted_link pour %cost_gold', '', '出售%item_formatted_link，仅售%cost_gold。', '', 'Quiero vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Who wants %item_formatted_link for %cost_gold?', 0, 0, '', 'Qui veut %item_formatted_link pour %cost_gold ?', '', '谁想要%item_formatted_link？只要%cost_gold！', '', '¿Quién quiere %item_formatted_link por %cost_gold?', '', ''),
('suggest_sell', 'Anyone wants %item_formatted_link? Only %cost_gold.', 0, 0, '', 'Quelqu\'un veut %item_formatted_link ? Seulement %cost_gold', '', '有人要%item_formatted_link吗？仅需%cost_gold。', '', '¿Alguien quiere %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'Just %cost_gold for %item_formatted_link!', 0, 0, '', 'Seulement %cost_gold pour %item_formatted_link!', '', '只要%cost_gold，就能拿走%item_formatted_link！', '', '¡Solo %cost_gold para %item_formatted_link!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold.', 0, 0, '', 'Vendre %item_formatted_link pour %cost_gold', '', '卖%item_formatted_link，价格%cost_gold。', '', 'Vendo %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', '%item_formatted_link is yours just for %cost_gold!', 0, 0, '', '%item_formatted_link est à vous juste pour %cost_gold !', '', '%item_formatted_link归你了，只要%cost_gold！', '', '%item_formatted_link es tuyo solo por %cost_gold!', '', ''),
('suggest_sell', 'Ridiculus price of %cost_gold for %item_formatted_link!', 0, 0, '', 'Prix ridicule de %cost_gold pour %item_formatted_link !', '', '%item_formatted_link超低价，仅%cost_gold！', '', '¡Precio ridículo de %cost_gold para %item_formatted_link_link!', '', ''),
('suggest_sell', 'Wanna sell %item_formatted_link for %cost_gold.', 0, 0, '', 'Je veux vendre %item_formatted_link pour %cost_gold', '', '想卖%item_formatted_link，价格%cost_gold。', '', 'Quiero vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Who needs %item_formatted_link? Only %cost_gold.', 0, 0, '', 'Qui a besoin de %item_formatted_link ? Seulement %cost_gold', '', '谁需要%item_formatted_link？只要%cost_gold。', '', '¿Quién necesita %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'Anyone needs %item_formatted_link for %cost_gold?', 0, 0, '', 'Quelqu\'un a besoin de %item pour %cost_gold ?', '', '有人需要%item_formatted_link吗？价格%cost_gold。', '', '¿Alguien necesita %item_formatted_link_link por %cost_gold?', '', ''),
('suggest_sell', '%cost_gold for %item_formatted_link. Less than AH!', 0, 0, '', '%cost_gold pour %item_formatted_link. Moins que l\'hôtel des ventes!', '', '%item_formatted_link只要%cost_gold，比拍卖行便宜！', '', '%cost_gold para %item_formatted_link. ¡Menos que en las subastas!', '', ''),
('suggest_sell', '%item_formatted_link is expensive, but I\'d sell it for %cost_gold.', 0, 0, '', '%item_formatted_link est cher, mais je le vendrais pour %cost_gold', '', '%item_formatted_link很贵，但我愿意以%cost_gold卖。', '', '%item_formatted_link es caro, pero lo vendería por %cost_gold.', '', ''),
('suggest_sell', 'You\'ll never find %item_formatted_link cheaper than %cost_gold!', 0, 0, '', 'Vous ne trouverez jamais %item_formatted_link moins cher que %cost_gold !', '', '你找不到比%cost_gold更便宜的%item_formatted_link！', '', '¡Nunca encontrarás %item_formatted_link más barato que %cost_gold!', '', ''),
('suggest_sell', 'Need more than %item_formatted_link!', 0, 0, '', 'Besoin de plus de %item_formatted_link !', '', '需要更多的%item_formatted_link！', '', '¡Necesito más %item_formatted_link!', '', ''),
('suggest_sell', 'I have %item_formatted_link and need more.', 0, 0, '', 'J\'ai %item_formatted_link et j\'en ai besoin de plus.', '', '我有%item_formatted_link，还需要更多。', '', 'Tengo %item_formatted_link y necesito más. ¿Alguien me vende alguno?', '', ''),
('suggest_sell', 'Have %item_formatted_link. Who wants to buy for %cost_gold?', 0, 0, '', 'Avoir %item_formatted_link. Qui veut acheter pour %cost_gold ?', '', '有%item_formatted_link，谁想以%cost_gold买？', '', 'Tengo %item_formatted_link. ¿Quién quiere comprarlo por %cost_gold?', '', ''),
('suggest_sell', 'Anyone WTB %item_formatted_link for %cost_gold?', 0, 0, '', 'Quelqu\'un cherche à acheter %item_formatted_link pour %cost_gold ?', '', '有人想以%cost_gold买%item_formatted_link吗？', '', '¿Alguien compra %item_formatted_link por %cost_gold?', '', ''),
('suggest_sell', 'What about %item_formatted_link? For %cost_gold.', 0, 0, '', 'Qu\'en est-il de %item_formatted_link ? Pour %cost_gold', '', '%item_formatted_link怎么样？只要%cost_gold。', '', '¿Qué pasa con %item_formatted_link? Por el oro.', '', ''),
('suggest_sell', 'Who said I am a bastard? %item_formatted_link for %cost_gold is a good price.', 0, 0, '', 'Qui a dit que j\'étais un bâtard? %item_formatted_link pour %cost_gold est un bon prix.', '', '谁说我抠门？%item_formatted_link卖%cost_gold很划算了！', '', '¿Quién ha dicho que soy un rata? %item_formatted_link por %cost_gold es una ganga.', '', ''),
('suggest_sell', 'I am selling %item_formatted_link? Just %cost_gold.', 0, 0, '', 'Je vends %item_formatted_link ? Juste %cost_gold', '', '我在卖%item_formatted_link，只要%cost_gold。', '', '¿Vendo %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'LFG for farming. You can still buy %item_formatted_link I have for %cost_gold.', 0, 0, '', 'Cherche un groupe pour farming. Vous pouvez toujours acheter %item_formatted_link que j\'ai pour %cost_gold', '', '组队刷怪，有兴趣可以买我的%item_formatted_link，仅%cost_gold。', '', 'LFG para farmear. Todavía puedes comprar %item_formatted_link que tengo por %cost_gold.', '', ''),
('suggest_sell', 'Sold almost everything today. Still have %item_formatted_link for %cost_gold.', 0, 0, '', 'Vendu presque tout aujourd\'hui. Vous avez encore %item_formatted_link pour %cost_gold', '', '今天几乎卖光了，还剩%item_formatted_link，价格%cost_gold。', '', 'Se vendió casi todo hoy. Todavía tengo %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'What use for trade chat? Of course to sell %item_formatted_link for %cost_gold.', 0, 0, '', 'A quoi sert le tchat commercial? Bien sûr pour vendre %item_formatted_link pour %cost_gold', '', '交易频道是干啥的？当然是卖%item_formatted_link，价格%cost_gold。', '', '¿De qué sirve el chat comercial? Por supuesto, para vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Can anyone beat the price of %cost_gold for %item_formatted_link?', 0, 0, '', 'Quelqu\'un peut-il battre le prix de %cost_gold pour %item_formatted_link ?', '', '有人能比%cost_gold更低价卖%item_formatted_link吗？', '', '¿Alguien puede superar el precio de %cost_gold por %item_formatted_link?', '', ''),
('suggest_sell', 'Wanna stop trade chat? Just buy %item_formatted_link? For %cost_gold!', 0, 0, '', 'Vous voulez arrêter le chat commercial? Vous venez d\'acheter %item_formatted_link ? Pour %cost_gold !', '', '想让交易频道安静？快买我的%item_formatted_link，只要%cost_gold！', '', '¿Quieres detener el chat comercial? ¿Solo compra %item_formatted_link? ¡Por el oro!', '', ''),
('suggest_sell', 'Everybody spams in trade chat. Me too - %cost_gold for %item_formatted_link!', 0, 0, '', 'Tout le monde spamme dans le chat commercial. Moi aussi - %cost_gold pour %item_formatted_link !', '', '交易频道都在刷屏，我也来—%item_formatted_link只要%cost_gold！', '', 'Todo el mundo envía spam en el chat y yo también! ¡%item_formatted_link por %cost_gold!', '', ''),
('suggest_sell', 'Is %item_formatted_link any use? Just selling it for %cost_gold.', 0, 0, '', '%item_formatted_link est-il utile? Je le vends juste pour %cost_gold', '', '%item_formatted_link有用吗？卖了，只要%cost_gold。', '', '¿Es %item_formatted_link útil? Solo lo vendo por %cost_gold.', '', ''),
('suggest_sell', 'I have %item_formatted_link ready to sell you for %cost_gold.', 0, 0, '', 'J\'ai %item_formatted_link prêt à vous vendre pour %cost_gold', '', '我有%item_formatted_link，准备卖给你，只要%cost_gold。', '', 'Tengo %item_formatted_link listo a la venta por %cost_gold.', '', ''),
('suggest_sell', 'Did nothing yesterday but have got %item_formatted_link. Selling it for %cost_gold.', 0, 0, '', 'Je n\'ai rien fait hier mais j\'ai %item_formatted_link Je le vends pour %cost_gold', '', '昨天啥也没干，但搞到了%item_formatted_link，卖了，%cost_gold。', '', 'No hice nada ayer pero consegu\'i %item_formatted_link. Lo vendo por %cost_gold.', '', ''),
('suggest_sell', 'Farmed yesterday and got %item_formatted_link. Anyone wtb for %cost_gold?', 0, 0, '', 'Farmé hier et obtenu %item_formatted_link Quelqu\'un cherche à acheter pour %cost_gold ?', '', '昨天刷怪拿到了%item_formatted_link，有人要买吗？%cost_gold。', '', 'Ayer farmee y conseguí mucho %item_formatted_link. ¿Alguien lo quiere comprar por %cost_gold?', '', ''),
('suggest_sell', 'Bought %item_formatted_link yesterday. Anyone needs it for %cost_gold?', 0, 0, '', 'Acheté %item_formatted_link hier. Quelqu\'un en a besoin pour %cost_gold ?', '', '昨天买了%item_formatted_link，有人需要吗？%cost_gold。', '', 'Ayer compré %item_formatted_link. ¿Alguien lo quiere por %cost_gold?', '', ''),
('suggest_sell', 'Who asked for %item_formatted_link? The price is the same - %cost_gold.', 0, 0, '', 'Qui a demandé %item_formatted_link ? Le prix est le même - %cost_gold', '', '谁问过%item_formatted_link？价格没变，%cost_gold。', '', '¿Quién pidió %item_formatted_link? El precio es el mismo: %cost_gold.', '', ''),
('suggest_sell', 'I sill have %item_formatted_link. WTB for %cost_gold?', 0, 0, '', 'J\'ai toujours %item_formatted_link. Cherche à acheter pour %cost_gold ?', '', '我还有%item_formatted_link，有人要买吗？%cost_gold。', '', 'Todavía tengo %item_formatted_link. ¿Alguien lo compra por %cost_gold?', '', ''),
('suggest_sell', 'I used to have more than %item_formatted_link. Now needs to sell it for %cost_gold.', 0, 0, '', 'J\'avais plus de %item_formatted_link Il faut maintenant le vendre pour %cost_gold', '', '我以前有更多的%item_formatted_link，现在得卖了，%cost_gold。', '', 'Solía tener más de %item_formatted_link. Ahora necesito venderlo por %cost_gold.', '', ''),
('suggest_sell', 'I wish I have more than %item_formatted_link. You could buy it for %cost_gold anyways.', 0, 0, '', 'J\'aimerais avoir plus de %item_formatted_link . Vous pouvez l\'acheter pour %cost_gold de toute façon.', '', '我希望有更多的%item_formatted_link，不过你可以先买这个，%cost_gold。', '', 'Desearía tener más de %item_formatted_link. Podrías comprarlo por %cost_gold de todos modos.', '', ''),
('suggest_sell', 'What use for your gold? To buy my %item_formatted_link for %cost_gold.', 0, 0, '', 'A quoi sert votre or ? Pour acheter mon %item_formatted_link pour %cost_gold', '', '你的金币有啥用？买我的%item_formatted_link吧，%cost_gold。', '', '¿De qué sirve tu oro? Para comprar mi %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Please spare some gold for me. You can buy %item_formatted_link for %cost_gold.', 0, 0, '', 'S\'il vous plaît, épargnez-moi de l\'or. Vous pouvez acheter %item_formatted_link pour %cost_gold', '', '请给我点金币吧，你可以买%item_formatted_link，只要%cost_gold。', '', 'Por favor, dame algo de oro. Puedes comprar %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Is %cost_gold is a good price for %item_formatted_link?', 0, 0, '', '%cost_gold est-il un bon prix pour %item_formatted_link ?', '', '%cost_gold买%item_formatted_link算好价吗？', '', '¿%cost_gold es un buen precio para %item_formatted_link?', '', ''),
('suggest_sell', 'Just bought yesterday %item_formatted_links, but do not need it anymore. Anyone wants for %cost_gold?', 0, 0, '', 'Je viens d\'acheter hier %item_formatted_links , mais je n\'en ai plus besoin. Quelqu\'un veut %cost_gold ?', '', '昨天刚买了%item_formatted_links，现在不需要了，有人要吗？%cost_gold。', '', 'Ayer compré %item_formatted_link, pero ya no los necesito. ¿Alguien lo quiere por %cost_gold?', '', ''),
('suggest_sell', 'I am going to post %item_formatted_link on the AH but you can buy it now cheaper just for %cost_gold.', 0, 0, '', 'Je vais poster %item_formatted_link à l\'hôtel des ventes mais vous pouvez l\'acheter maintenant moins cher juste pour %cost_gold', '', '我准备把%item_formatted_link挂拍卖行，但你可以现在便宜买，只要%cost_gold。', '', 'Voy a poner %item_formatted_link en la casa de subastas pero puedes comprarlo ahora más barato solo por %cost_gold.', '', ''),
('suggest_sell', 'Why the #!@ have I bought %item_formatted_link? Anyone needs it for %cost_gold?', 0, 0, '', 'Pourquoi ai-je acheté le #!@ %item_formatted_link ? Quelqu\'un en a besoin pour %cost_gold ?', '', '我干嘛买了%item_formatted_link？有人需要吗？%cost_gold。', '', '¿Por qué #!@ he comprado %item_formatted_link? ¿Alguien lo necesita por %cost_gold?', '', ''),

-- response LFG/LFM channel
-- usable placeholders:
-- %quest_links
-- %other_name
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('response_lfg_quests_channel', 'I have %quest_links', 0, 0, '', '', '', '我有%quest_links任务', '', '', '', ''),
('response_lfg_quests_channel', 'I also have %quest_links', 0, 0, '', '', '', '我也有%quest_links任务', '', '', '', ''),
('response_lfg_quests_channel', 'I also have %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '我也有%quest_links任务，现在在%zone_name', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I also have %quest_links', 0, 0, '', '', '', '%other_name，我也有%quest_links任务', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I also have %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '%other_name，我也有%quest_links任务，现在在%zone_name', '', '', '', ''),
('response_lfg_quests_channel', 'I am up for %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '我想做%quest_links任务，现在在%zone_name', '', '', '', ''),
('response_lfg_quests_channel', 'I am up for %quest_links, I am %my_role', 0, 0, '', '', '', '我想做%quest_links任务，我是%my_role', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I am up for %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '%other_name，我想做%quest_links任务，现在在%zone_name', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I am up for %quest_links, I am %my_role', 0, 0, '', '', '', '%other_name，我想做%quest_links任务，我是%my_role', '', '', '', ''),

-- response LFG/LFM whisper
-- usable placeholders:
-- %quest_links
-- %other_name
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('response_lfg_quests_whisper', 'Hey, I\'m up for %quest_links', 0, 0, '', '', '', '嘿，我想做%quest_links任务', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I could do %quest_links with you', 0, 0, '', '', '', '嘿，我可以和你一起做%quest_links任务', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I also have %quest_links', 0, 0, '', '', '', '嘿，我也有%quest_links任务', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, I\'m up for %quest_links', 0, 0, '', '', '', '嘿，%other_name，我想做%quest_links任务', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, I could do %quest_links with you', 0, 0, '', '', '', '嘿，%other_name，我可以和你一起做%quest_links任务', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, I also have %quest_links', 0, 0, '', '', '', '嘿，%other_name，我也有%quest_links任务', '', '', '', ''),
('response_lfg_quests_whisper', 'wanna group up for %quest_links ?', 0, 0, '', '', '', '想组队一起做%quest_links任务吗？', '', '', '', ''),
('response_lfg_quests_whisper', 'I am up for %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '我想做%quest_links任务，现在在%zone_name', '', '', '', ''),
('response_lfg_quests_whisper', 'I am up for %quest_links, I am %my_role', 0, 0, '', '', '', '我想做%quest_links任务，我是%my_role', '', '', '', ''),

-- response WTB channel
-- usable placeholders:
-- %formatted_item_links
-- %other_name
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('response_wtb_items_channel', '%other_name, I could sell you %formatted_item_links', 0, 0, '', '', '', '%other_name，我可以卖给你%formatted_item_links', '', '', '', ''),
('response_wtb_items_channel', 'I could potentially sell %formatted_item_links', 0, 0, '', '', '', '我可能可以卖%formatted_item_links', '', '', '', ''),
('response_wtb_items_channel', 'Think I could sell %formatted_item_links', 0, 0, '', '', '', '我想我可以卖%formatted_item_links', '', '', '', ''),
('response_wtb_items_channel', '%other_name, I could potentially sell %formatted_item_links', 0, 0, '', '', '', '%other_name，我可能可以卖%formatted_item_links', '', '', '', ''),
('response_wtb_items_channel', '%other_name, Think I could sell %formatted_item_links', 0, 0, '', '', '', '%other_name，我想我可以卖%formatted_item_links', '', '', '', ''),

-- response WTB channel
-- usable placeholders:
-- %formatted_item_links
-- %other_name
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('response_wtb_items_whisper', 'I could sell you %formatted_item_links', 0, 0, '', '', '', '我可以卖给你%formatted_item_links', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I got %formatted_item_links for sale', 0, 0, '', '', '', '嘿，我有%formatted_item_links要卖', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I could potentially sell %formatted_item_links', 0, 0, '', '', '', '嘿，我可能可以卖%formatted_item_links', '', '', '', ''),

-- /////////////////////////////
-- generic events

-- quest
('quest_accept', 'Quest accepted', 0, 0, '', '', '', '任务已接受', '', 'Misión aceptada', '', 'Квест взят'),

('quest_remove', 'Quest removed', 0, 0, '', '', '', '任务已移除', '', 'Misión eliminada', '', 'Квест отменен'),

('quest_cant_take', 'I can\'t take this quest', 0, 0, '', '', '', '我无法接受这个任务', '', 'No puedo tomar esta búsqueda', '', 'Я не могу взять этот квест'),

('quest_error_talk', 'I can\'t talk with the quest giver', 0, 0, '', '', '', '我无法与任务发布者交谈', '', 'No puedo hablar con el propietario de la mision', '', ''),

('quest_error_completed', 'I have already completed %quest', 0, 0, '', '', '', '我已经完成了%quest任务', '', 'Ya he completado la mision %quest', '', ''),

('quest_error_have_quest', 'I already have %quest', 0, 0, '', '', '', '我已经有了%quest任务', '', 'Ya tengo la mision %quest', '', ''),

('quest_error_cant_take', 'I can\'t take %quest', 0, 0, '', '', '', '我无法接受%quest任务', '', 'No puedo aceptar la mision %quest', '', ''),

('quest_error_log_full', 'I can\'t take %quest because my quest log is full', 0, 0, '', '', '', '任务日志已满，无法接受%quest任务', '', 'No puedo aceptar la mision %quest porque mi registro de misiones esta completo', '', ''),

('quest_error_bag_full', 'I can\'t take %quest because my bag is full', 0, 0, '', '', '', '背包已满，无法接受%quest任务', '', 'No puedo aceptar la mision %quest porque mi inventario esta lleno', '', ''),

('quest_accepted', 'I have accepted %quest', 0, 0, '', '', '', '我已接受%quest任务', '', 'He aceptado la mision %quest', '', ''),

('quest_status_incomplete', 'I have not completed the quest %quest', 0, 0, '', '', '', '我还未完成%quest任务', '', 'No he completado la mision %quest', '', ''),

('quest_status_available', 'Quest %quest available', 0, 0, '', '', '', '%quest任务可用', '', 'Mision %quest disponible', '', ''),

('quest_status_failed', 'I have failed the quest %quest', 0, 0, '', '', '', '我失败了%quest任务', '', 'He fallado la mision %quest', '', ''),

('quest_status_unable_to_complete', 'I am unable to turn in the quest %quest', 0, 0, '', '', '', '我无法提交%quest任务', '', 'No puedo entregar la mision %quest', '', ''),

('quest_status_completed', 'I have completed the quest %quest', 0, 0, '', '', '', '我已完成%quest任务', '', 'He completado la mision %quest', '', ''),

('quest_status_complete_single_reward', 'I have completed the quest %quest and received %item', 0, 0, '', '', '', '我完成了%quest任务，获得了%item', '', 'He completado la mision %quest y he recibido %item', '', ''),

('quest_status_complete_pick_reward', 'Which reward should I pick for completing the quest %quest?%rewards', 0, 0, '', '', '', '完成%quest任务后我该选哪个奖励？%rewards', '', '¿Que recompensa deberia escoger por completar la mision %quest?%rewards', '', ''),

('quest_choose_reward', 'Okay, I will pick %item as a reward', 0, 0, '', '', '', '好吧，我选%item作为奖励', '', 'De acuerdo, voy a escoger %item de recompensa', '', ''),

-- stuff
('hello', 'Hello', 0, 0, '', '', '', '你好', '', 'Hola', '', 'Привет'),
('hello', 'Hello!', 0, 0, '', '', '', '你好！', '', '¡Hola!', '', 'Привет!'),
('hello', 'Hi', 0, 0, '', '', '', '嗨', '', 'Hola', '', 'Прив'),
('hello', 'Hi!', 0, 0, '', '', '', '嗨！', '', '¡Hola!', '', 'Прив!'),
('hello', 'Hello there!', 0, 0, '', '', '', '你好啊！', '', '¡Hola!', '', 'Здарова!'),

('hello_follow', 'Hello, I follow you!', 0, 0, '', '', '', '你好，我跟着你！', '', '¡Hola, te sigo!', '', 'Привет, иду за тобой!'),
('hello_follow', 'Hello, lead the way!', 0, 0, '', '', '', '你好，带路吧！', '', '¡Hola, guía el camino!', '', 'Привет, веди!'),
('hello_follow', 'Hi, lead the way!', 0, 0, '', '', '', '嗨，带路吧！', '', '¡Hola, guía el camino!', '', 'Прив, веди!'),

('join_group', 'Hey %player, do you want join my group?', 0, 0, '', '', '', '嘿，%player，想加入我的队伍吗？', '', 'Oye %player, ¿Quieres unirte a mi grupo?', '', ''),

('join_raid', 'Hey %player, do you want join my group?', 0, 0, '', '', '', '嘿，%player，想加入我的团队吗？', '', 'Oye %player, ¿Quieres unirte a mi banda?', '', ''),

('logout_cancel', 'Logout cancelled!', 0, 0, '', '', '', '登出已取消！', '', 'Cerrar sesión cancelado!', '', 'Логаут отменен!'),

('logout_start', 'I\'m logging out!', 0, 0, '', '', '', '我要下线了！', '', '¡Me estoy desconectando!', '', 'Я выхожу из игры!'),

('goodbye', 'Goodbye!', 0, 0, '', '', '', '再见！', '', '¡Adiós!', '', 'Пока!'),
('goodbye', 'Bye bye!', 0, 0, '', '', '', '拜拜！', '', '¡Adiós!', '', 'Пока пока!'),
('goodbye', 'See you later!', 0, 0, '', '', '', '回头见！', '', '¡Hasta luego!', '', 'Увидимся позже!'),

-- /////////////////////////////
-- replies

-- replies 0
('reply', 'what was that %s?', 0, 0, '', 'c\'était quoi ce %s?', '', '你说啥，%s？', '', '¿qué has dicho %s?', '', ''),
('reply', 'not sure I understand %s?', 0, 0, '', 'pas sûr de comprendre %s?', '', '我不确定我懂你啥意思，%s？', '', '¿no estoy seguro de entenderte %s?', '', ''),
('reply', 'uh... no clue what yer talkin bout', 0, 0, '', 'euh... aucune idée de quoi tu parles', '', '呃...完全不知道你在说啥', '', 'uh... ni idea de lo que estás hablando', '', ''),
('reply', 'you talkin to me %s?', 0, 0, '', 'tu me parles %s?', '', '你在跟我说话吗，%s？', '', '¿Me hablas a mí %s?', '', ''),
('reply', 'whaaaa?', 0, 0, '', 'quoiaaa?', '', '啥？', '', '¿quéaaa?', '', ''),
('reply', 'huh?', 0, 0, '', 'hein?', '', '哈？', '', '¿eh?', '', ''),
('reply', 'what?', 0, 0, '', 'Quoi?', '', '啥？', '', '¿qué?', '', ''),
('reply', 'are you talking?', 0, 0, '', 'parles-tu?', '', '你在说话吗？', '', '¿estás hablando?', '', ''),
('reply', 'whatever dude', 0, 0, '', 'peu importe mec', '', '随便吧，哥们', '', 'Menudo tio...', '', ''),
('reply', 'you lost me', 0, 0, '', 'tu m\'as perdu', '', '我没跟上你的意思', '', 'No entiendo ni papa', '', ''),
('reply', 'Bla bla bla...', 0, 0, '', 'Bla bla bla...', '', 'blah blah blah...', '', 'Bla bla bla...', '', ''),
('reply', 'What did you say, %s?', 0, 0, '', 'Qu\'as-tu dis, %s?', '', '你说啥了，%s？', '', '¿Qué me estas contando %s?', '', ''),
('reply', 'Concentrate on the game, %s!', 0, 0, '', 'Concentres-toi sur le jeu, %s!', '', '专心打游戏吧，%s！', '', '¡Concéntrate en el juego, %s!', '', ''),
('reply', 'Chatting with you %s is so great! I always wanted to meet you', 0, 0, '', 'Discuter avec vous %s est tellement génial! J\'ai toujours voulu te rencontrer', '', '跟你聊天真开心，%s！早就想认识你了', '', '¡%s, chatear contigo es genial! Siempre quise conocerte', '', ''),
('reply', 'These chat-messages are freaking me out! I feel like I know you all!', 0, 0, '', 'Ces messages du chat me font flipper! J\'ai l\'impression de vous connaître tous!', '', '这些聊天消息吓到我了！感觉我认识你们所有人！', '', '¡Estos mensajes de chat me están asustando! ¿De que cojones estais hablando?', '', ''),
('reply', 'YEAH RIGHT! HAHA SURE!!!', 0, 0, '', 'OUI EN EFFET! AHAH BIEN SÛR!!!', '', '对对对！哈哈，绝对！', '', '¡SÍ, CORRECTO! ¡¡¡JAJA SEGURO!!!', '', ''),
('reply', 'I believe you!!!', 0, 0, '', 'Je te crois!!!', '', '我信你！！！', '', '¡Si, claro!', '', ''),
('reply', 'OK, uhuh LOL', 0, 0, '', 'OK, euh LOL', '', '好吧，嗯，哈哈', '', 'OK, uhuh LOL', '', ''),
('reply', 'Why is everybody always saying the same things???', 0, 0, '', 'Pourquoi tout le monde dit toujours la même chose???', '', '为啥大家老说一样的话？？？', '', '¿Por qué todo el mundo siempre dice lo mismo?', '', ''),
('reply', 'Hey %s....oh nevermind!', 0, 0, '', 'Hé %s.... oh tant pis!', '', '嘿，%s...算了，不说了！', '', 'Oye %s... ¡oh, no importa!', '', ''),
('reply', 'What are you talking about %s', 0, 0, '', 'De quoi parlez-vous %s', '', '你在说啥，%s？', '', 'De qué estás hablando %s', '', ''),
('reply', 'Who said that? I resemble that remark', 0, 0, '', 'Qui a dit ça? je ressemble à cette remarque', '', '谁说的？我咋觉得是在说我？', '', '¿Quién ha dicho eso? ¿Estais hablando de mi?', '', ''),
('reply', 'wtf are you all talking about', 0, 0, '', '', '', '你们都在说啥啊？', '', '', '', ''),
('reply', 'fr fr no cap on a stack', 0, 0, '', '', '', '真的，绝对不骗人', '', '', '', ''),
('reply', 'your not getting jack shit', 0, 0, '', '', '', '你啥也别想拿到', '', '', '', ''),
('reply', 'swag', 0, 0, '', '', '', '酷毙了', '', '', '', ''),
('reply', 'ty!', 0, 0, '', '', '', '谢谢！', '', '', '', ''),
('reply', 'no', 0, 0, '', '', '', '不', '', '', '', ''),
('reply', 'Yep', 0, 0, '', '', '', '嗯', '', '', '', ''),
('reply', 'f', 0, 0, '', '', '', '致敬', '', '', '', ''),
('reply', '%s no shit xD', 0, 0, '', '', '', '%s，废话 xD', '', '', '', ''),
('reply', 'why is that', 0, 0, '', '', '', '为啥呢', '', '', '', ''),
('reply', 'lmao', 0, 0, '', '', '', '笑死我了', '', '', '', ''),
('reply', 'thought i should remain silent, was getting confused from chat again', 0, 0, '', '', '', '我还是闭嘴吧，聊天又把我搞晕了', '', '', '', ''),
('reply', 'i can get realy jealous', 0, 0, '', '', '', '我可会嫉妒了', '', '', '', ''),
('reply', '%s you can\'t hear the dripping sarcasm in my text', 0, 0, '', '', '', '%s，你听不出我文字里的满满讽刺', '', '', '', ''),
('reply', 'he said no homo, it\'s fine', 0, 0, '', '', '', '他说没别的意思，那就没事', '', '', '', ''),
('reply', 'dwarf moment', 0, 0, '', '', '', '矮人时刻', '', '', '', ''),
('reply', 'Yes %s', 0, 0, '', '', '', '是的，%s', '', '', '', ''),
('reply', 'interesting...', 0, 0, '', '', '', '有意思...', '', '', '', ''),
('reply', 'lol', 0, 0, '', '', '', '哈哈', '', '', '', ''),
('reply', '%s fuck you man :D', 0, 0, '', '', '', '%s，去你的吧 :D', '', '', '', ''),
('reply', ':^)', 0, 0, '', '', '', ':^)', '', '', '', ''),
('reply', 'ty', 0, 0, '', '', '', '谢谢', '', '', '', ''),
('reply', '%s well said', 0, 0, '', '', '', '%s，说得好', '', '', '', ''),
('reply', 'yay', 0, 0, '', '', '', '耶！', '', '', '', ''),
('reply', 'yeah', 0, 0, '', '', '', '对', '', '', '', ''),
('reply', 'ooooooh', 0, 0, '', '', '', '哦哦哦', '', '', '', ''),
('reply', 'hmm', 0, 0, '', '', '', '嗯', '', '', '', ''),
('reply', 'yeah right', 0, 0, '', '', '', '对对对', '', '', '', ''),
('reply', 'you made me gag wtf', 0, 0, '', '', '', '你让我恶心了，啥玩意', '', '', '', ''),
('reply', 'hot', 0, 0, '', '', '', '火辣', '', '', '', ''),
('reply', 'hoes mad', 0, 0, '', '', '', '有人生气了', '', '', '', ''),
('reply', 'what have you been eating %s', 0, 0, '', '', '', '你吃啥了，%s', '', '', '', ''),
('reply', 'wtf', 0, 0, '', '', '', '啥玩意', '', '', '', ''),
('reply', 'i will attempt to understand that comment', 0, 0, '', '', '', '我会试着理解你的话', '', '', '', ''),
('reply', '*confused*', 0, 0, '', '', '', '*困惑*', '', '', '', ''),
('reply', 'fuck yea', 0, 0, '', '', '', '太棒了', '', '', '', ''),
('reply', '0/10 would not read again', 0, 0, '', '', '', '0/10，不会再看', '', '', '', ''),
('reply', '10/10 would read again', 0, 0, '', '', '', '10/10，会再看', '', '', '', ''),
('reply', '6/10 would read', 0, 0, '', '', '', '6/10，还行吧', '', '', '', ''),
('reply', '7/10 would', 0, 0, '', '', '', '7/10，值得一看', '', '', '', ''),
('reply', 'based', 0, 0, '', '', '', '靠谱', '', '', '', ''),
('reply', 'oh yeah maybe', 0, 0, '', '', '', '哦，对，可能吧', '', '', '', ''),
('reply', 'yeah so what', 0, 0, '', '', '', '对，那又怎样', '', '', '', ''),

-- replies 1
('reply', 'hey %s i havent forgotten you', 0, 1, '', 'salut %s je ne t\'ai pas oublié', '', '嘿，%s，我还没忘了你', '', 'hey %s no te he olvidado', '', ''),
('reply', 'you piss me off %s', 0, 1, '', 'tu me fais chier %s', '', '你惹毛我了，%s', '', 'me cabreas %s', '', ''),
('reply', 'im gunna get you this time %s', 0, 1, '', 'je vais t\'avoir cette fois %s', '', '这次我得搞定你，%s', '', 'voy a atraparte esta vez %s', '', ''),
('reply', 'better watch your back %s', 0, 1, '', 'mieux vaut surveiller vos arrières %s', '', '小心你的背后，%s', '', 'mejor cuida tu espalda %s', '', ''),
('reply', 'i did not like last round so much', 0, 1, '', 'je n\'ai pas tellement aimé le dernier tour', '', '上一轮我不太满意', '', 'no me gustó tanto la última ronda', '', ''),
('reply', 'i sucked last round thanks to %s', 0, 1, '', 'j\'ai merdé le dernier tour grâce à %s', '', '上一轮我表现烂透了，多亏了%s', '', 'chupé la última ronda gracias a %s', '', ''),
('reply', 'prepare to die %s', 0, 1, '', 'préparez-vous à mourir %s', '', '准备受死吧，%s', '', 'prepárate para morir %s', '', ''),
('reply', 'dont appreciate you killin me %s', 0, 1, '', 'j\'apprécie pas que tu me tues %s', '', '我不喜欢你杀了我，%s', '', 'no aprecio que me mates %s', '', ''),
('reply', '%s, i hate you', 0, 1, '', '%s, je te déteste', '', '%s，我讨厌你', '', '%s, te odio', '', ''),
('reply', 'grrrrrr, ill get you this time %s', 0, 1, '', 'grrrrrr, je vais t\'avoir cette fois %s', '', '哼，这次我一定抓住你，%s', '', 'grrrrrr, te libraste esta vez %s', '', ''),
('reply', 'well fuck you', 0, 1, '', 'eh bien va te faire foutre', '', '去你的', '', 'vamos a la mierda', '', ''),
('reply', '%s i\'ll vomit into your fkin mouth', 0, 1, '', '', '', '%s，我要吐你一脸', '', '', '', ''),
('reply', 'dont fucking judge me', 0, 1, '', '', '', '别他妈评头论足', '', '', '', ''),
('reply', 'Yo momma so fat, she can\'t even fit through the Dark Portal', 0, 1, '', '', '', '你妈胖得连黑暗之门都挤不过去', '', '', '', ''),

-- replies 2
('reply', 'wtf', 0, 2, '', 'wtf', '', '啥玩意', '', 'wtf', '', ''),
('reply', 'wtf??', 0, 2, '', 'wtf??', '', '啥玩意？？', '', 'wtf??', '', ''),
('reply', 'low life', 0, 2, '', 'faible durée de vie', '', '垃圾', '', 'low life', '', ''),
('reply', 'wth', 0, 2, '', 'wth', '', '啥情况', '', 'wth', '', ''),
('reply', 'sucked', 0, 2, '', 'Ça craint', '', '太烂了', '', 'chupate esa', '', ''),
('reply', 'REMATCH!!! im taking him down', 0, 2, '', 'REMATCH!!! je le descends', '', '再来一局！！我要干掉他', '', '¡¡REVANCHA!!! lo estoy derribando', '', ''),
('reply', 'pathetic, i got killed by %s', 0, 2, '', 'pathétique, j\'ai été tué par %s', '', '真丢人，我被%s杀了', '', 'patético, me mató %s', '', ''),
('reply', 'ok i\'m done', 0, 2, '', '', '', '好了，我完蛋了', '', '', '', ''),

-- replies 3
('reply', 'hehe, i nailed %s?', 0, 3, '', 'hehe, j\'ai cloué %s?', '', '嘿嘿，我搞定了%s？', '', 'jeje, ¿acerté a %s?', '', ''),
('reply', 'that was too easy, killin %s', 0, 3, '', 'c\'était trop facile, de tuer %s', '', '太简单了，干掉%s', '', 'fue demasiado fácil matar a %s', '', ''),
('reply', 'gotcha mofo', 0, 3, '', 'je t\'ai eu enfoiré', '', '抓到你了，混蛋', '', 'te pille hijo de perra', '', ''),
('reply', 'ha ha', 0, 3, '', 'ha ha', '', '哈哈', '', 'ja, ja', '', ''),
('reply', 'loser', 0, 3, '', 'loser', '', '废物', '', 'loser', '', ''),
('reply', 'i killed %s and yer all next dudes', 0, 3, '', 'j\'ai tué %s et vous tous les prochains mecs', '', '我杀了%s，你们下一个', '', 'yo maté a %s y a todos los siguientes tipos', '', ''),
('reply', 'oh yeah i owned him', 0, 3, '', 'oh ouais je l\'ai possédé', '', '哦耶，我碾压了他', '', 'oh, sí, lo tenía', '', ''),
('reply', 'im a killin machine', 0, 3, '', 'je suis une machine à tuer', '', '我就是杀戮机器', '', 'soy una máquina de matar', '', ''),
('reply', '%s,  this reminds me of a Slayer song...all this bloodshed', 0, 3, '', '%s, ça me rappelle une chanson de Slayer... tout ce bain de sang', '', '%s，这让我想起Slayer的歌...全是血腥', '', '%s, esto me recuerda a una canción de Slayer... todo este derramamiento de sangre', '', ''),
('reply', 'sorry, %s. can we do the scene again?', 0, 3, '', 'désolé, %s. peut-on refaire la scène?', '', '抱歉，%s，我们能重来一次吗？', '', 'lo siento, %s. ¿Podemos hacer la escena de nuevo?', '', ''),
('reply', 'so....how do you like being worm food %s???', 0, 3, '', 'alors... comment aimez-vous être de la nourriture pour vers %s???', '', '所以...你喜欢当虫子食物吗，%s？？？', '', 'así que... ¿cómo te gusta ser comida de gusanos %s???', '', ''),
('reply', 'yer supposed to be dead, %s its part of the game!!!!!', 0, 3, '', 'tu es censé être mort, %s ça fait partie du jeu!!!!!', '', '你应该死了，%s，这是游戏的一部分！！！！', '', '¡¡Se supone que estás muerto, %s es parte del juego!!!!!', '', ''),
('reply', 'sorry, %s. that looked as good as an Andy Worhol painting!', 0, 3, '', 'désolé, %s. ça avait l\'air aussi bien qu\'une peinture d\'Andy Warhol!', '', '抱歉，%s，那看起来就像安迪·沃霍尔的画一样棒！', '', 'lo siento, %s. ¡Eso se veía tan bien como una pintura de Andy Worhol!', '', ''),
('reply', '%s, ill use the rubber bullets next time !', 0, 3, '', '%s, j\'utiliserai pas les balles en caoutchouc la prochaine fois!', '', '%s，下次我用橡皮子弹！', '', '%s, ¡usaré las balas de goma la próxima vez!', '', ''),
('reply', 'whatsamatter, %s?? lose your head? hahaha gotta keep cool!!', 0, 3, '', 'qu\'importe, %s ?? tu perds la tête? hahaha faut rester cool!!', '', '咋了，%s？？丢了脑袋？哈哈，得冷静！！', '', '¿qué pasa, %s?? ¿has perdido la cabeza? jajaja tengo que mantener la calma!!', '', ''),
('reply', 'i had to do it, %s. You understand. The Director said so!!', 0, 3, '', 'je devais le faire, %s. Tu comprends. Le directeur l\'a dit!!', '', '我不得不这么做，%s。你懂的。导演说了算！！', '', 'Tenía que hacerlo, %s. Entiendelo... ¡¡Dios me lo ordenó!!', '', ''),
('reply', 'hey %s.......MUAHAHAHAHAHAHAHAHAHAHA', 0, 3, '', 'Salut %s.......MUAHAHAHAHAHAHAHAHAHAHA', '', '嘿，%s.......哈哈哈哈哈哈哈哈哈', '', 'hey %s.......MUAHAHAHAHAHAHAHAHAHAHA', '', ''),
('reply', '%s, i enjoyed that one!! Lets play it again Sam', 0, 3, '', '%s, j\'ai adoré celui-là !! Jouons à nouveau Sam', '', '%s，我很喜欢这次！！再玩一次吧，Sam', '', '%s, ¡disfruté ese! Juguemos de nuevo Sam', '', ''),
('reply', 'hey, %s! ju can start callin me scarface.. ju piece of CHIT!!!!', 0, 3, '', 'Hé, %s ! ju peut commencer à m\'appeler Scarface .. ju merde!!!!', '', '嘿，%s！你可以开始叫我刀疤脸了..你个小垃圾！！！！', '', '¡oye, %s! ¡Puedes empezar a llamarme caracortada... ¡¡¡un trozo de CHIT!!!', '', ''),
('reply', 'are you talking to me %s??', 0, 3, '', 'tu me parles %s ??', '', '你在跟我说话吗，%s？？', '', '¿me estás hablando a mí %s?', '', ''),
('reply', '%s get it right this time, dont stand in front of my bullets.', 0, 3, '', '%s réussis cette fois, ne te tiens pas devant mes balles.', '', '%s，这次搞对，别站在我的子弹前面。', '', '%s hazlo bien esta vez, no te pares frente a mis balas.', '', ''),
('reply', '%s, what are you laying around for??? hehe', 0, 3, '', '%s, pourquoi traînes-tu??? hé hé', '', '%s，你躺着干啥？？？嘿嘿', '', '%s, ¿para qué estás tirado? jeje', '', ''),
('reply', 'laughed real hard', 0, 3, '', '', '', '笑得肚子疼', '', '', '', ''),

-- replies 4
('reply', 'hi %s', 0, 4, '', 'salut %s', '', '嗨，%s', '', 'hola %s', '', ''),
('reply', 'oh, hi %s', 0, 4, '', 'oh, salut %s', '', '哦，嗨，%s', '', 'oh, hola %s', '', ''),
('reply', 'wazzup %s!!!', 0, 4, '', 'Quoi de neuf %s!!!', '', '咋样啊，%s！！！', '', 'wazzup %s!!!', '', ''),
('reply', 'hi', 0, 4, '', 'salut', '', '嗨', '', 'hi', '', ''),
('reply', 'wazzup', 0, 4, '', 'wazzup', '', '咋样', '', 'wazzup', '', ''),
('reply', 'hello %s', 0, 4, '', 'bonjour %s', '', '你好，%s', '', 'hola %s', '', ''),
('reply', 'hi %s, do i know you?', 0, 4, '', 'Salut %s, est-ce que je te connais ?', '', '嗨，%s，我认识你吗？', '', 'hola %s, ¿te conozco?', '', ''),
('reply', 'hey %s', 0, 4, '', 'salut %s', '', '嘿，%s', '', 'hey %s', '', ''),
('reply', 'hai %s', 0, 4, '', 'ha %s', '', '嗨，%s', '', 'hai %s', '', ''),
('reply', 'wth', 0, 6, '', 'wth', '', '啥情况', '', 'wth', '', ''),
('reply', 'wtf', 0, 6, '', 'wtf', '', '啥玩意', '', 'que cojones!', '', ''),
('reply', 'this is bs', 0, 6, '', 'c\'est bs', '', '这太扯了', '', 'esto es una mierda', '', ''),
('reply', 'admin', 0, 6, '', 'admin', '', '管理员', '', 'admin', '', ''),
('reply', 'hey %s quit abusing your admin', 0, 6, '', 'hé %s arrête d\'abuser de ton admin', '', '嘿，%s，别滥用你的管理员权限', '', 'hey %s deja de abusar de tu poder de administrador', '', ''),
('reply', 'leave me alone admin!', 0, 6, '', 'laissez-moi tranquille admin!', '', '别烦我，管理员！', '', '¡déjame en paz administrador!', '', ''),
('reply', 'you suck admin', 0, 6, '', 'tu es nul admin', '', '你真烂，管理员', '', 'apestas administrador', '', ''),
('reply', 'thats my name, what you want %s', 0, 5, '', 'c\'est mon nom, qu\'est-ce que tu veux %s', '', '那是我的名字，你想干啥，%s', '', 'ese es mi nombre, ¿que quieres %s?', '', ''),
('reply', 'yes???', 0, 5, '', 'Oui???', '', '啥事？？？', '', 'sí???', '', ''),
('reply', 'uh... what', 0, 5, '', 'euh ...quoi', '', '呃...啥', '', 'uh... qué', '', ''),
('reply', 'you talkin to me %s?', 0, 5, '', 'tu me parles %s ?', '', '你在跟我说话吗，%s？', '', '¿Me hablas a mí %s?', '', ''),


-- taunts
('taunt', 'I have puppies under my armor!', 0, 0, '', 'J\'ai des chiots sous mon armure !', '', '我的铠甲下藏着小狗崽呢！', '', '¡Tengo cachorros bajo mi armadura!', '', ''),
('taunt', 'Bite me, <target>!', 0, 0, '', 'Mords moi, <target>!', '', '来咬我啊，<target>！', '', '¡Muérdeme, <target>!', '', ''),
('taunt', 'Hey <target>! Guess what your mom said last night!', 0, 0, '', 'Hey <target>! Devine ce que ta mère a dit hier soir!', '', '嘿，<target>！猜猜昨晚你妈说了啥！', '', '¡Oye, <target>! ¡Adivina lo que me dijo tu madre anoche!', '', ''),
('taunt', '<target>, you\'re so ugly you couldn\'t score in a monkey whorehouse with a bag of bananas!', 0, 0, '', '<target>, tu es si moche que tu ne pourrais pas marquer dans un bordel de singes avec un sac de bananes!', '', '<target>，你丑得连拿着香蕉去猴子窝都找不到伴儿！', '', '<target>, ¡eres tan feo que no podrías anotar en un burdel de monos con una bolsa de plátanos!', '', ''),
('taunt', 'Shut up <target>, you\'ll never be the man your mother is!!', 0, 0, '', 'Tais-toi <target>, tu ne seras jamais l\'homme qu\'est ta mère!!', '', '闭嘴吧，<target>，你永远比不上你妈！', '', 'Cállate <target>, ¡nunca serás el hombre que es tu madre!', '', ''),
('taunt', 'Your mother was a hampster and your father smelt of elderberries!!!!', 0, 0, '', 'Ta mère était un hamster et ton père sentait le sureau!!!!', '', '你妈是只仓鼠，你爸一股接骨木味！', '', 'Tu madre era un hámster y tu padre olía a bayas de saúco!!!!', '', ''),
('taunt', 'I don\'t want to talk to you no more, you empty headed animal food trough wiper!!!', 0, 0, '', 'Je ne veux plus te parler, espèce d\'essuyeur d\'abreuvoirs pour animaux tête vide!!!', '', '我不想再跟你废话，你这个空脑壳的牲口槽擦手！', '', '¡¡No quiero hablar más contigo, limpiaparabrisas de cabeza vacía!', '', ''),
('taunt', 'I fart in your general direction!!!', 0, 0, '', 'Je pète dans votre direction générale!!!', '', '我朝你的大致方向放了个屁！', '', '¡¡Me tiro un pedo en tu dirección general!!!', '', ''),
('taunt', 'Go and boil your bottom, you son of a silly person!!!', 0, 0, '', 'Va te faire cuire le cul, fils d\'idiot!!!', '', '去煮你的屁股吧，你这个傻子的儿子！', '', '¡Ve y hierve tu trasero, hijo de una persona tonta!', '', ''),
('taunt', 'What are you going to do <target>, bleed on me? HAVE AT YOU!', 0, 0, '', 'Qu\'est ce que tu vas faire <target>, saigner sur moi? A VOUS!', '', '<target>，你能干啥，流血溅我一身吗？来吧！', '', '¿Qué vas a hacer <target>, llorar?', '', ''),
('taunt', 'M-O-O-N! That spells aggro!', 0, 0, '', 'PUNAISE! Ce sort aggro!', '', '月-亮！这就是仇恨的咒语！', '', '¡LUNA! ¡Eso deletrea agresividad!', '', ''),
('taunt', 'You\'re about as useful as a one-legged man in an ass kicking contest.', 0, 0, '', 'Tu es à peu près aussi utile qu\'un homme unijambiste dans un concours de coups de pied au cul.', '', '你就像独腿男参加踢屁股比赛，屁用没有！', '', 'Eres tan útil como un hombre con una sola pierna en un concurso de patadas en el culo.', '', ''),
('taunt', 'Hey <target>! Stop hitting on them, they\'re not your type. They aren\'t inflatable.', 0, 0, '', 'Hey <target>! Arrête de les draguer, ce n\'est pas ton genre. Ils ne sont pas gonflables.', '', '嘿，<target>！别老盯着他们，他们不是你喜欢的类型，又不是充气的！', '', '¡Oye, <target>! Deja de coquetear con ellos, no son tu tipo. No son inflables.', '', ''),
('taunt', '<target> you\'re so far outta your league, you\'re playing a different sport.', 0, 0, '', '<target> vous êtes tellement hors de votre ligue que vous pratiquez un sport différent.', '', '<target>，你完全不是一个量级的，简直在玩不同的游戏！', '', '<target> estás tan lejos de tu liga, estás jugando un deporte diferente.', '', ''),
('taunt', 'You made a big mistake today <target>, you got out of bed.', 0, 0, '', 'Tu as fait une grosse erreur aujourd\'hui <target>, tu es sorti du lit.', '', '今天你犯了个大错，<target>，你不该下床的！', '', 'Cometiste un gran error hoy <target>, te levantaste de la cama.', '', ''),
('taunt', 'I wanna try turning into a horse, but I need help. I\'ll be the front, you be yourself.', 0, 0, '', 'Je veux essayer de me transformer en cheval, mais j\'ai besoin d\'aide. Je serai devant, tu seras toi-même.', '', '我想试试变成一匹马，但需要帮忙。我当马头，你就当你自己吧。', '', 'Quiero intentar convertirme en un caballo, pero necesito ayuda. Yo seré el frente, tú mismo.', '', ''),
('taunt', 'Can I borrow your face for a few days? My ass is going on holiday....', 0, 0, '', 'Puis-je emprunter votre visage pendant quelques jours? Mon cul part en vacances....', '', '能借你的脸用几天吗？我的屁股要去度假了……', '', '¿Me prestas tu cara por unos días? Mi culo se va de vacaciones...', '', ''),
('taunt', 'I\'d like to give you a going away present... First you do your part.', 0, 0, '', 'J\'aimerais vous offrir un cadeau de départ... D\'abord, donnes ta part.', '', '我想送你个离别礼物……先把你那份做好。', '', 'Me gustaría darte un regalo de despedida... Primero haz tu parte.', '', ''),
('taunt', 'Before you came along we were hungry, Now we\'re just fed up.', 0, 0, '', 'Avant que tu n\'arrives nous avions faim, maintenant nous en avons juste marre.', '', '在你来之前我们还饿着，现在只是对你腻了。', '', 'Antes de que llegaras teníamos hambre, ahora estamos hartos', '', ''),
('taunt', 'I like you. People say I have no taste, but I like you.', 0, 0, '', 'Je t\'aime bien. Les gens disent que je n\'ai pas de goût, mais je t\'aime bien.', '', '我挺喜欢你的。别人说我没品味，但我就是喜欢你。', '', 'Me gustas. La gente dice que no tengo gusto, pero me gustas.', '', ''),
('taunt', 'I think you have an inferiority complex, but that\'s okay, it\'s justified.', 0, 0, '', 'Je pense que tu as un complexe d\'infériorité, mais il n\'y a pas de problème, c\'est justifié.', '', '我觉得你有自卑情结，不过没关系，这是有理由的。', '', 'Creo que tienes complejo de inferioridad, pero está bien, está justificado', '', ''),
('taunt', 'Hence rotten thing! Or I shall shake thy bones out of thy garments.', 0, 0, '', 'Par conséquent chose pourrie! Je vais secouer tes os de tes vêtements.', '', '去吧，腐烂的东西！不然我要把你的骨头从衣服里抖出来！', '', '¡Por lo tanto, cosa podrida! o te sacudiré los huesos de las vestiduras.', '', ''),
('taunt', 'I can\'t believe I\'m wasting my time with you!', 0, 0, '', 'Je n\'arrive pas à croire que je perds mon temps avec toi!', '', '真不敢相信我居然在浪费时间跟你耗！', '', '¡No puedo creer que estoy perdiendo el tiempo contigo!', '', ''),
('taunt', 'I love it when someone insults me, it means I don\'t have to be nice anymore.', 0, 0, '', 'J\'adore quand quelqu\'un m\'insulte, ça veut dire que je n\'ai plus besoin d\'être gentil.', '', '我喜欢被人辱骂，这意味着我不用再装好人了。', '', 'Me encanta cuando alguien me insulta, significa que ya no tengo que ser amable', '', ''),
('taunt', 'Thou leathern-jerkin, crystal-button, knot-pated, agatering, puke-stocking, caddis-garter, smooth-tongue, Spanish pouch!', 0, 0, '', 'Toi l\'agent fédéral, pourpoint en cuir, bouton de cristal, anneau d\'agate, bas vomi, jarretière caddis, pochette espagnole!', '', '你这皮革外套、水晶纽扣、蠢头蠢脑、玛瑙戒指、呕吐袜子、毛虫吊袜带、油嘴滑舌、西班牙小包！', '', '¡Tú, jubón de cuero, botón de cristal, anudado, ágata, medias de vómito, liga de caddis, lengua suave, bolsa española!', '', ''),
('taunt', 'Thou qualling bat-fowling malt-worm!', 0, 0, '', 'Toi qualificatif de chauve-souris qui chasse le ver de malt!', '', '你这胆小的蝙蝠猎手，麦芽虫！', '', '¡Tú, gusano de malta cazador de murciélagos!', '', ''),
('taunt', 'Thou art truely an idol of idiot-worshippers!', 0, 0, '', 'Tu es vraiment une idole d\'adorateurs d\'idiots!', '', '你真是白痴崇拜者的偶像！', '', '¡Eres verdaderamente un ídolo de los adoradores de idiotas!', '', ''),
('taunt', 'Thou misbegotten knotty-pated wagtail!', 0, 0, '', 'Tu es une bergeronnette printanière mal engendrée!', '', '你这天生蠢笨的摇尾鸟！', '', '¡Maldito lavandero nudoso!', '', ''),
('taunt', 'Thou whoreson mandrake, thou art fitter to be worn in my cap than to wait at my heels!', 0, 0, '', 'Toi putain de mandragore, tu es plus apte à être porté dans ma casquette qu\'à attendre sur mes talons!', '', '你这该死的曼德拉草，戴在我帽子上都比跟在我脚后跟合适！', '', '¡Hijo de puta mandrágora, eres mejor para que te lleve en mi gorra que para que me pise los talones!', '', ''),
('taunt', 'You! You scullion! You rampallian! You fustilarian! I\'ll tickle your catastrophe!', 0, 0, '', 'Toi ! Espèce de marmiton ! Espèce de rampallian ! Espèce de fusilier ! Je vais chatouiller ta catastrophe!', '', '你！下贱厨子！无赖！废物！我要把你挠得满地打滚！', '', '¡Tú! ¡Scullion! ¡Ramaliano! ¡Fustilarista! ¡Te haré cosquillas en tu catástrofe!', '', ''),
('taunt', 'Oh <target>! Thou infectious ill-nurtured flax-wench!', 0, 0, '', 'Oh <target>! Tu es contagieuse et mal nourrie!', '', '哦，<target>！你这传染病似的、没教养的亚麻丫头！', '', '¡Oh, <target>! ¡Tú, contagiosa moza del lino mal educada!', '', ''),
('taunt', 'We leak in your chimney, <target>!', 0, 0, '', 'On a une fuite dans votre cheminée, <target>!', '', '我们在你的烟囱里漏水了，<target>！', '', '¡Tenemos una fuga en tu chimenea, <target>!', '', ''),
('taunt', 'Oh thou bootless fen-sucked canker-blossom!', 0, 0, '', 'Oh toi, fleur de chancre sucée par les marais sans botte!', '', '哦，你这没靴子的、沼泽吸出来的毒疮花！', '', '¡Oh tú, flor de cancro chupada por el pantano sin botas!', '', ''),
('taunt', 'Were I like thee I\'d throw away myself!', 0, 0, '', 'Si j\'étais comme toi, je me jetterais!', '', '如果我像你一样，我宁愿把自己扔了！', '', '¡Si yo fuera como tú, me tiraría a la basura!', '', ''),
('taunt', 'O teach me <target>, how I should forget to think!', 0, 0, '', 'O apprends-moi <target>, comme je devrais oublier de penser!', '', '哦，教教我吧，<target>，我该如何忘记思考！', '', '¡Oh, enséñame <target>, cómo debo olvidarme de pensar!', '', ''),
('taunt', 'Truly thou art damned, like an ill-roasted egg, all on one side!', 0, 0, '', 'Vraiment tu es damné, comme un œuf mal rôti, tout d\'un côté!', '', '你真是该死，就像个烤坏的蛋，烤得一面焦黑！', '', '¡Verdaderamente estás condenado, como un huevo mal asado, todo de un lado!', '', ''),
('taunt', 'You starvelling, you eel-skin, you dried neat\'s-tongue, you bull\'s-pizzle, you stock-fish- O for breath to utter what is like thee!! -you tailor\'s-yard, you sheath, you bow-case, you vile standing tuck!', 0, 0, '', 'Toi affamé, espèce de peau d\'anguille, ta langue bien séchée, espèce de taureau, poisson séché, O pour le souffle de dire ce qui te ressemble !! -espèce de vergue de tailleur, espèce de fourreau, espèce d\'étui à archet, vil tuck debout', '', '你这饿鬼，鳗鱼皮，干牛舌，公牛鞭，腌鱼干——哦，我喘口气才能说出你像啥！——裁缝尺，刀鞘，弓套，你这站立的废物！', '', 'Estás muerto de hambre, piel de anguila, te secaste la lengua limpiamente, toro-pizzle, pescado-O para respirar pronunciar lo que es como tú !! ¡Tú, sastrería, vaina, estuche de arco, vil flaco de pie!', '', ''),
('taunt', 'Fie! Drop thee into the rotten mouth of Death!', 0, 0, '', 'Fi ! Jette-toi dans la bouche pourrie de la Mort!', '', '呸！滚进死亡的腐烂大嘴里去吧！', '', '¡Fie! ¡Déjate caer en la podrida boca de la Muerte!', '', ''),
('taunt', '<target>, you are a fishmonger!', 0, 0, '', '<target>, vous êtes poissonnier!', '', '<target>，你就是个卖鱼的！', '', '<target>, ¡eres un pescadero!', '', ''),
('taunt', 'I shall live to knock thy brains out!', 0, 0, '', 'Je vivrai pour te faire sauter la cervelle!', '', '我会活着把你的脑浆打出来！', '', '¡Viviré para romperte los sesos!', '', ''),
('taunt', 'Most shallow are you, <target>!! Thou art worms-meat in respect of a good piece of flesh, indeed!!', 0, 0, '', 'Tu es très superficiel, <target>!! Tu es de la viande de vers par rapport à un bon morceau de chair, en effet!!', '', '你真是肤浅，<target>！比起一块好肉，你就是虫子的食粮！', '', '¡¡Eres el más superficial, <target>!! ¡Eres carne de gusano con respecto a un buen trozo de carne, de hecho!', '', ''),
('taunt', 'Vile wretch! O <target>, thou odiferous hell-hated pignut!', 0, 0, '', 'Misérable ! O <target>, toi, ignoble cochon de l\'enfer!', '', '卑鄙的家伙！哦，<target>，你这地狱里都嫌臭的猪头！', '', '¡Miserable vil! ¡Oh, <target>, fétido cerdo odiado por el infierno!', '', ''),
('taunt', '<target>! Thy kiss is as comfortless as frozen water to a starved snake!', 0, 0, '', '<target>! Ton baiser est aussi désagréable que de l\'eau gelée pour un serpent affamé!', '', '<target>！你的吻就像给饿蛇的冰水一样毫无安慰！', '', '<target>! ¡Tu beso es tan desconsolador como el agua helada para una serpiente hambrienta!', '', ''),
('taunt', 'I scorn you, scurvy companion. What, you poor, base, rascally, cheating, lack-linen mate! Away, you moldy rogue, away!', 0, 0, '', 'Je te méprise, compagnon du scorbut. Quoi, pauvre, bas, coquin, tricheur, camarade en manque de linge ! Partez, espèce de voyou moisi, partez!', '', '我鄙视你，坏血病的家伙！什么，你这穷酸、卑贱、无赖、骗子、缺衣少穿的货！滚吧，你这发霉的无赖，滚！', '', 'Te desprecio, compañero escorbuto. ¡Qué, pobre, vil, pícaro, tramposo, compañero falto de lino! ¡Fuera, mohoso granuja, fuera!', '', ''),
('taunt', 'Out of my sight! Thou dost infect my eyes <target>!', 0, 0, '', 'Hors de ma vue! Tu infectes mes yeux <target>!', '', '滚出我的视线！你污染了我的眼睛，<target>！', '', '¡Fuera de mi vista! ¡Me infectas los ojos <target>!', '', ''),
('taunt', 'PLAY TIME!!!!', 0, 0, '', 'RÉCRÉATION!!!!', '', '开玩吧！！！', '', '¡¡¡HORA DE JUGAR!!!!', '', ''),
('taunt', 'None shall pass!', 0, 0, '', 'Personne ne passera!', '', '谁也别想过去！', '', '¡Ninguno pasará!', '', ''),
('taunt', 'We\'re under attack! A vast, ye swabs! Repel the invaders!', 0, 0, '', 'Nous sommes attaqués ! Un vaste écouvillons! Repoussez les envahisseurs!', '', '我们遭到攻击！一大群，伙计们！击退入侵者！', '', '¡Estamos bajo ataque! ¡Un vasto, hisopos! ¡Repeler a los invasores!', '', ''),
('taunt', 'None may challenge the Brotherhood!', 0, 0, '', 'Personne ne peut défier la Confrérie!', '', '没人能挑战兄弟会！', '', '¡Nadie puede desafiar a la Hermandad!', '', ''),
('taunt', 'Foolsss...Kill the one in the dress!', 0, 0, '', 'Foolsss... Tuez celui qui porte la robe!', '', '蠢货们……干掉那个穿裙子的！', '', 'Tontosss... ¡Matad al del vestido!', '', ''),
('taunt', 'I\'ll feed your soul to Hakkar himself! ', 0, 0, '', 'Je donnerai ton âme à Hakkar lui-même! ', '', '我会把你的灵魂喂给哈卡本人！', '', '¡Le daré de comer con tu alma al mismísimo Hakkar! ', '', ''),
('taunt', 'Pride heralds the end of your world! Come, mortals! Face the wrath of the <randomfaction>!', 0, 0, '', 'La fierté annonce la fin de votre monde ! Venez, mortels ! Affrontez la colère des <randomfaction>!', '', '骄傲预示着你们世界的终结！来吧，凡人！面对<randomfaction>的怒火！', '', '¡El orgullo anuncia el fin de tu mundo! ¡Venid, mortales! ¡Enfréntate a la ira de la <facción aleatoria>!', '', ''),
('taunt', 'All my plans have led to this!', 0, 0, '', 'Tous mes plans ont mené à cela!', '', '我所有的计划都指向这一刻！', '', '¡Todos mis planes me han llevado a esto!', '', ''),
('taunt', 'Ahh! More lambs to the slaughter!', 0, 0, '', 'Ahhh ! Plus d\'agneaux à l\'abattoir!', '', '啊哈！又有羔羊送来屠宰了！', '', '¡Ahh! ¡Más corderos al matadero!', '', ''),
('taunt', 'Another day, another glorious battle!', 0, 0, '', 'Un autre jour, une autre bataille glorieuse!', '', '又是新的一天，又一场辉煌的战斗！', '', '¡Otro día, otra batalla gloriosa!', '', ''),
('taunt', 'So, business... or pleasure?', 0, 0, '', 'Alors, affaires... ou plaisir?', '', '那么，是公事……还是娱乐？', '', 'Entonces, ¿negocios... o placer?', '', ''),
('taunt', 'You are not prepared!', 0, 0, '', 'Vous n\'êtes pas prêt!', '', '你们还没准备好！', '', '¡No estás preparado!', '', ''),
('taunt', 'The <randomfaction>\'s final conquest has begun! Once again the subjugation of this world is within our grasp. Let none survive! ', 0, 0, '', 'La conquête finale de la <randomfaction> a commencé ! Une fois de plus, l\'assujettissement de ce monde est à notre portée. Que personne ne survive!', '', '<randomfaction>的最终征服开始了！这个世界的臣服再次近在咫尺。不留活口！', '', '¡La conquista final de <randomfaction> ha comenzado! Una vez más, la subyugación de este mundo está a nuestro alcance. ¡Que nadie sobreviva! ', '', ''),
('taunt', 'Your death will be a painful one. ', 0, 0, '', 'Ta mort sera douloureuse.', '', '你的死会非常痛苦。', '', 'Tu muerte será dolorosa. ', '', ''),
('taunt', 'Cry for mercy! Your meaningless lives will soon be forfeit. ', 0, 0, '', 'Crie miséricorde ! Vos vies insignifiantes seront bientôt perdues.', '', '哭着求饶吧！你们毫无意义的生活很快就要终结了。', '', '¡Llora por piedad! Sus vidas sin sentido pronto se perderán. ', '', ''),
('taunt', 'Abandon all hope! The <randomfaction> has returned to finish what was begun so many years ago. This time there will be no escape! ', 0, 0, '', 'Abandonner tout espoir! La <randomfaction> est revenue pour terminer ce qui avait été commencé il y a tant d\'années. Cette fois, il n\'y aura pas d\'échappatoire!', '', '放弃一切希望吧！<randomfaction>回来了，要完成多年前未竟之事。这次你们无路可逃！', '', '¡Abandona toda esperanza! La <randomfaction> ha regresado para terminar lo que comenzó hace tantos años. ¡Esta vez no habrá escapatoria! ', '', ''),
('taunt', 'Alert! You are marked for Extermination! ', 0, 0, '', 'Alerte! Vous êtes marqué pour l\'extermination!', '', '警报！你们已被标记为清除目标！', '', '¡Alerta! ¡Estás marcado para el exterminio! ', '', ''),
('taunt', 'The <subzone> is for guests only...', 0, 0, '', 'La <subzone> est réservée aux invités...', '', '<subzone>只接待贵宾……', '', 'La <subzone> es solo para invitados...', '', ''),
('taunt', 'Ha ha ha! You are hopelessly outmatched!', 0, 0, '', 'Hahaha! Vous êtes désespérément surpassé!', '', '哈哈哈！你们根本不是对手！', '', '¡Ja, ja, ja! ¡Estás irremediablemente superado!', '', ''),
('taunt', 'I will crush your delusions of grandeur! ', 0, 0, '', 'Je vais écraser vos illusions de grandeur!', '', '我会粉碎你们自大的幻想！', '', '¡Aplastaré tus delirios de grandeza! ', '', ''),
('taunt', 'Forgive me, for you are about to lose the game.', 0, 0, '', 'Pardonnez-moi, car vous êtes sur le point de perdre la partie.', '', '原谅我，因为你们马上就要输了。', '', 'Perdóname, porque estás a punto de perder el juego.', '', ''),
('taunt', 'Struggling only makes it worse.', 0, 0, '', 'Lutter ne fait qu\'empirer les choses.', '', '挣扎只会让事情更糟。', '', 'La lucha solo lo empeora.', '', ''),
('taunt', 'Vermin! Leeches! Take my blood and choke on it!', 0, 0, '', 'Vermine! sangsues ! Prends mon sang et étouffe-toi avec!', '', '害虫！水蛭！喝我的血，噎死你们！', '', '¡Alimañas! ¡Sanguijuelas! ¡Toma mi sangre y atragantate con ella!', '', ''),
('taunt', 'Not again... NOT AGAIN!', 0, 0, '', 'Pas encore... PAS ENCORE!', '', '别又来……别再来了！', '', 'No otra vez... ¡NO OTRA VEZ!', '', ''),
('taunt', 'My blood will be the end of you!', 0, 0, '', 'Mon sang sera ta fin!', '', '我的血将是你们的终结！', '', '¡Mi sangre será tu fin!', '', ''),
('taunt', 'Good, now you fight me!', 0, 0, '', 'Bien, maintenant tu me combats!', '', '好极了，现在来跟我打吧！', '', 'Bien, ¡ahora pelea conmigo!', '', ''),
('taunt', 'Get da move on, guards! It be killin\' time!', 0, 0, '', 'Allez-y, gardes ! Ça va tuer le temps!', '', '快动起来，守卫们！是时候大开杀戒了！', '', '¡Muévanse, guardias! ¡Será para matar el tiempo!', '', ''),
('taunt', 'Don\'t be delayin\' your fate. Come to me now. I make your sacrifice quick.', 0, 0, '', 'Ne retardez pas votre destin. Viens à moi maintenant. Je fais votre sacrifice rapide.', '', '别拖延你的命运。现在就过来，我会让你的牺牲快一点。', '', 'No demores tu destino. Ven a mí ahora. Hago que tu sacrificio sea rápido.', '', ''),
('taunt', 'You be dead soon enough!', 0, 0, '', 'Tu es mort bien assez tôt!', '', '你很快就会死翘翘！', '', '¡Estarás muerto lo suficientemente pronto!', '', ''),
('taunt', 'Mua-ha-ha!', 0, 0, '', 'Mua-ha-ha!', '', '姆哈哈哈！', '', '¡Mua-ja-ja!', '', ''),
('taunt', 'I be da predator! You da prey...', 0, 0, '', 'Je suis un prédateur ! Tu es une proie...', '', '我是捕食者！你是猎物……', '', '¡Soy un depredador! Eres presa...', '', ''),
('taunt', 'You gonna leave in pieces!', 0, 0, '', 'Tu vas partir en morceaux!', '', '你会碎成渣渣离开的！', '', '¡Te vas a ir en pedazos!', '', ''),
('taunt', 'Death comes. Will your conscience be clear? ', 0, 0, '', 'La mort vient. Votre conscience sera-t-elle claire?', '', '死亡降临。你的良心能安宁吗？', '', 'La muerte llega. ¿Tu conciencia estará limpia? ', '', ''),
('taunt', 'Your behavior will not be tolerated.', 0, 0, '', 'Votre comportement ne sera pas toléré.', '', '你的行为是不可容忍的。', '', 'Tu comportamiento no será tolerado', '', ''),
('taunt', 'The Menagerie is for guests only.', 0, 0, '', 'La Ménagerie est réservée aux invités.', '', '展览馆只对贵宾开放。', '', 'The Menagerie es solo para invitados', '', ''),
('taunt', 'Hmm, unannounced visitors, Preparations must be made... ', 0, 0, '', 'Hmm, visiteurs inopinés, il faut faire des préparatifs...', '', '嗯，不速之客，必须做些准备……', '', 'Hmm, visitantes inesperados, se deben hacer preparativos... ', '', ''),
('taunt', 'Hostile entities detected. Threat assessment protocol active. Primary target engaged. Time minus thirty seconds to re-evaluation.', 0, 0, '', 'Entités hostiles détectées. Protocole d\'évaluation de la menace actif. Cible principale engagée. Délai de réévaluation moins trente secondes.', '', '侦测到敌对实体。威胁评估协议已激活。主要目标锁定。重新评估倒计时30秒。', '', 'Entidades hostiles detectadas. Protocolo de evaluación de amenazas activo. Objetivo principal comprometido. Tiempo menos treinta segundos para la reevaluación.', '', ''),
('taunt', 'New toys? For me? I promise I won\'t break them this time!', 0, 0, '', 'Nouveaux jouets? Pour moi? Promis je ne les casserai pas cette fois !', '', '新玩具？给我的？我保证这次不会弄坏！', '', '¿Juguetes nuevos? ¿Para mi? ¡Te prometo que no los romperé esta vez!', '', ''),
('taunt', 'I\'m ready to play!', 0, 0, '', 'Je suis prêt à jouer!', '', '我准备好开玩了！', '', '¡Estoy listo para jugar!', '', ''),
('taunt', 'Shhh... it will all be over soon.', 0, 0, '', 'Chut... tout sera bientôt fini.', '', '嘘……一切很快就会结束。', '', 'Shhh... todo terminará pronto.', '', ''),
('taunt', 'Aaaaaughibbrgubugbugrguburgle!', 0, 0, '', 'Aaaaaughibbrgubugbugrguburgle!', '', '啊啊啊啊咕噜咕噜咕噜咕！', '', '¡Aaaaaughibbrgubugbugrguburgle!', '', ''),
('taunt', 'RwlRwlRwlRwl!', 0, 0, '', 'RwlRwlRwlRwl!', '', '吼吼吼吼！', '', 'RwlRwlRwlRwl!', '', ''),
('taunt', 'You too, shall serve!', 0, 0, '', 'Toi aussi, tu serviras!', '', '你也一样，必须服从！', '', '¡Tú también, debes servir!', '', ''),
('taunt', 'Tell me... tell me everything!  Naughty secrets! I\'ll rip the secrets from your flesh!', 0, 0, '', 'Dis-moi... dis-moi tout ! Secrets coquins ! J\'arracherai les secrets de ta chair!', '', '告诉我……把一切都告诉我！那些小秘密！我会从你的肉体中撕出秘密！', '', 'Cuéntame... ¡cuéntame todo! ¡Secretos traviesos! ¡Arrancaré los secretos de tu carne!', '', ''),
('taunt', 'Prepare yourselves, the bells have tolled! Shelter your weak, your young and your old! Each of you shall pay the final sum! Cry for mercy, the reckoning has come!', 0, 0, '', 'Préparez-vous, les cloches ont sonné ! Mettez à l\'abri vos faibles, vos jeunes et vos vieux ! Chacun de vous paiera la somme finale ! Implorez la miséricorde , le jugement est venu!', '', '做好准备，丧钟已响！保护好你们的弱者、孩子和老人！你们每个人都将付出最终代价！哭着求饶吧，清算之日已到！', '', '¡Prepárense, las campanas han doblado! ¡Abriga a tus débiles, a tus jóvenes ya tus viejos! ¡Cada uno de ustedes pagará la suma final! Clama por misericordia, el ajuste de cuentas ha llegado!', '', ''),
('taunt', 'Where in Bonzo\'s brass buttons am I?', 0, 0, '', 'Où suis-je dans les boutons en laiton de Bonzo?', '', '我在邦佐的铜纽扣里是啥位置？', '', '¿Dónde estoy en los botones de latón de Bonzo?', '', ''),
('taunt', 'I can bear it no longer! Goblin King! Goblin King! Wherever you may be! Take this <target> far away from me!', 0, 0, '', 'Je ne peux plus le supporter ! Roi Gobelin ! Roi gobelin ! Où que vous soyez ! Emmenez cette <target> loin de moi!', '', '我再也受不了了！地精之王！地精之王！不管你在哪儿！把这个<target>带离我远点！', '', '¡No puedo soportarlo más! ¡Rey de los duendes! ¡Rey de los duendes! ¡WHERE quiera que estés! ¡Lleva a este <target> lejos de mí!', '', ''),
('taunt', 'You have thirteen hours in which to solve the labyrinth, before your baby brother becomes one of us... forever.', 0, 0, '', 'Vous avez treize heures pour résoudre le labyrinthe, avant que ton petit frère ne devienne l\'un des nôtres... pour toujours.', '', '你有十三小时解开迷宫，不然你的小弟弟就会永远成为我们的一员。', '', 'Tienes trece horas para resolver el laberinto, antes de que tu hermanito se convierta en uno de nosotros... para siempre', '', ''),
('taunt', 'So, the <subzone> is a piece of cake, is it? Well, let\'s see how you deal with this little slice... ', 0, 0, '', 'Donc, la <subzone> est un jeu d\'enfant, n\'est-ce pas ? Eh bien, voyons comment tu gères cette petite tranche...', '', '所以，<subzone>对你来说是小菜一碟，对吧？好吧，看看你怎么应付这一小块……', '', 'Entonces, la <subzone> es pan comido, ¿verdad? Bueno, veamos cómo lidias con esta pequeña porción... ', '', ''),
('taunt', 'Back off, I\'ll take you on, headstrong to take on anyone, I know that you are wrong, and this is not where you belong', 0, 0, '', 'Reculez, je vais vous attaquer, têtu à affronter n\'importe qui, je sais que vous vous trompez, et ce n\'est pas votre place', '', '退后，我要挑战你，固执地要跟任何人干一架，我知道你错了，这里不是你该待的地方！', '', 'Atrás, te enfrentaré, testarudo para enfrentar a cualquiera, sé que estás equivocado, y este no es el lugar al que perteneces', '', ''),
('taunt', 'Show me whatcha got!', 0, 0, '', 'Montre-moi ce que tu as!', '', '给我看看你有啥本事！', '', '¡Muéstrame lo que tienes!', '', ''),
('taunt', 'To the death!', 0, 0, '', 'A la mort!', '', '决一死战！', '', '¡Hasta la muerte!', '', ''),
('taunt', 'Twin blade action, for a clean close shave every time.', 0, 0, '', 'Action à double lame, pour un rasage de près propre à chaque fois.', '', '双刃行动，每次都能干净利落地剃个光头！', '', 'Acción de cuchilla doble, para un afeitado limpio y apurado en todo momento.', '', ''),
('taunt', 'Bring it on!', 0, 0, '', 'Allez-y!', '', '来吧！', '', '¡Adelante!', '', ''),
('taunt', 'You\'re goin\' down!', 0, 0, '', 'Tu descends!', '', '你完蛋了！', '', '¡Vas a caer!', '', ''),
('taunt', 'Stabby stab stab!', 0, 0, '', 'Coup de poignard poignardé!', '', '捅捅捅！', '', '¡Puñalada puñalada puñalada!', '', ''),
('taunt', 'Let\'s get this over quick, time is mana.', 0, 0, '', 'Finissons-en vite , le temps c\'est du mana.', '', '赶紧解决，时间就是魔法！', '', 'Terminemos con esto rápido, el tiempo es maná.', '', ''),
('taunt', 'I do not think you realise the gravity of your situation.', 0, 0, '', 'Je ne pense pas que tu te rendes compte de la gravité de ta situation.', '', '我想你没意识到自己处境的严重性。', '', 'No creo que te des cuenta de la gravedad de tu situación.', '', ''),
('taunt', 'I will bring honor to my family and my kingdom!', 0, 0, '', 'Je ferai honneur à ma famille et à mon royaume!', '', '我会为我的家族和王国带来荣耀！', '', '¡Llevaré honor a mi familia y mi reino!', '', ''),
('taunt', 'Light, give me strength!', 0, 0, '', 'Lumière, donne-moi la force!', '', '圣光，赐予我力量！', '', '¡Luz, dame fuerza!', '', ''),
('taunt', 'My church is the field of battle - time to worship...', 0, 0, '', 'Mon église est le champ de bataille - il est temps d\'adorer...', '', '我的教堂就是战场——是时候膜拜了……', '', 'Mi iglesia es el campo de batalla - hora de adorar...', '', ''),
('taunt', 'I hold you in contempt...', 0, 0, '', 'Je te méprise...', '', '我鄙视你……', '', 'Te tengo en desacato...', '', ''),
('taunt', 'Face the hammer of justice!', 0, 0, '', 'Affrontez le marteau de la justice!', '', '面对正义之锤吧！', '', '¡Enfréntate al martillo de la justicia!', '', ''),
('taunt', 'Prove your worth in the test of arms under the Light!', 0, 0, '', 'Prouvez votre valeur dans l\'épreuve des armes sous la Lumière!', '', '在圣光的见证下证明你的武艺价值！', '', '¡Demuestra tu valía en la prueba de las armas bajo la Luz!', '', ''),
('taunt', 'All must fall before the might and right of my cause, you shall be next!', 0, 0, '', 'Tout doit tomber devant la force et le droit de ma cause, vous serez le prochain!', '', '一切都将在我的力量和正义前倒下，你将是下一个！', '', 'Todo debe caer ante el poder y el derecho de mi causa, ¡tú serás el próximo!', '', ''),
('taunt', 'Prepare to die!', 0, 0, '', 'Préparez-vous à mourir!', '', '准备受死吧！', '', '¡Prepárate para morir!', '', ''),
('taunt', 'The beast with me is nothing compared to the beast within...', 0, 0, '', 'La bête avec moi n\'est rien comparée à la bête à l\'intérieur...', '', '我身边的野兽与我内心的野兽相比，根本不算什么……', '', 'La bestia conmigo no es nada comparada con la bestia interior...', '', ''),
('taunt', 'Witness the firepower of this fully armed huntsman!', 0, 0, '', 'Soyez témoin de la puissance de feu de ce chasseur entièrement armé!', '', '见证这位全副武装猎人的火力吧！', '', '¡Sea testigo de la potencia de fuego de este cazador completamente armado!', '', ''),

-- combat events
('critical health', 'Heal me! Quick!', 0, 0, '', 'Soigne moi! Vite!', '', '快治疗我！快！', '', '¡Cúrame! ¡Rápido!', '', ''),
('critical health', 'Almost dead! Heal me!', 0, 0, '', 'Presque mort! Soigne moi!', '', '快死了！治疗我！', '', '¡Estoy casi muerto! ¡Cúrame!', '', ''),
('critical health', 'Help! Heal me!', 0, 0, '', 'Aider! Soigne moi!', '', '救命！治疗我！', '', '¡Ayuda! ¡Cúrame!', '', ''),
('critical health', 'Somebody! Heal me!', 0, 0, '', 'Quelqu\'un! Soigne moi!', '', '谁来！治疗我！', '', '¡Alguien! ¡Cúrame!', '', ''),
('critical health', 'Heal! Heal! Heal!', 0, 0, '', 'Soigner! Soigner! Soigner!', '', '治疗！治疗！治疗！', '', '¡Curame! ¡Saname! ¡Socorro!', '', ''),
('critical health', 'I am dying! Heal! Aaaaarhg!', 0, 0, '', 'Je meurs! Soigner! Aaaaarhg!', '', '我要死了！治疗！啊啊啊啊！', '', '¡Me estoy muriendo! ¡Saname! ¡Aaaaarhg!', '', ''),
('critical health', 'Heal me!', 0, 0, '', 'Soigne moi!', '', '治疗我！', '', '¡Cúrame!', '', ''),
('critical health', 'I will die. I will die. I will die. Heal!', 0, 0, '', 'Je vais mourir. Je vais mourir. Je vais mourir. Soigner!', '', '我要死了。我要死了。我要死了。治疗！', '', 'Voy a morir. Voy a morir. Voy a morir. ¡Saname!', '', ''),
('critical health', 'Healers, where are you? I am dying!', 0, 0, '', 'Guérisseurs, où êtes-vous ? Je meurs!', '', '治疗者，你们在哪？我快死了！', '', 'Sanadores, ¿dónde estais? ¡Me estoy muriendo!', '', ''),
('critical health', 'Oh the pain. Heal me quick!', 0, 0, '', 'Ah la douleur. Guéris-moi vite!', '', '哦，好痛。快治疗我！', '', 'Ouch, qué dolor. ¡Cúrame rápido!', '', ''),

('low health', 'Need heal', 0, 0, '', 'Besoin de guérir', '', '需要治疗', '', 'Necesito una cura', '', ''),
('low health', 'Low health', 0, 0, '', 'Santé faible', '', '血量低', '', 'Salud baja', '', ''),
('low health', 'Drop a heal. Please.', 0, 0, '', 'Dépose un soin. S\'il te plaît.', '', '来个治疗，拜托。', '', 'Una sanación por favor.', '', ''),
('low health', 'Could somebody drop a heal on me?', 0, 0, '', 'Quelqu\'un pourrait-il me soigner?', '', '谁能给我来个治疗？', '', '¿Alguien podría curarme?', '', ''),
('low health', 'Hey! Better heal me now than rez later', 0, 0, '', 'Hey! Mieux vaut me soigner maintenant que rez plus tard', '', '嘿！现在治疗总比待会复活强', '', '¡Oye! Mejor curarme ahora que resucitarme después', '', ''),
('low health', 'I am sorry. Need another heal', 0, 0, '', 'Je suis désolé. J\'ai besoin d\'un autre soin', '', '抱歉，又需要治疗了', '', 'Lo siento. Necesito otra curación', '', ''),
('low health', 'Damn mobs. Heal me please', 0, 0, '', 'Maudits mobs. Guéris moi s\'il te plait', '', '该死的怪物，拜托治疗我', '', '¡Malditos bichos! ¡Una curita por favor!', '', ''),
('low health', 'One more hit and I am done for. Heal please', 0, 0, '', 'Un coup de plus et c\'est fini. Guéris moi s\'il te plait', '', '再挨一下我就完了，拜托治疗', '', 'Un golpe más y estoy acabado. Cura por favor', '', ''),
('low health', 'Are there any healers?', 0, 0, '', 'Y a-t-il des guérisseurs?', '', '有治疗者吗？', '', '¿Hay sanadores?', '', ''),
('low health', 'Why do they always punch me in the face? Need heal', 0, 0, '', 'Pourquoi me frappent-ils toujours au visage? Besoin de soins', '', '为啥老是打我脸？需要治疗', '', '¿Por qué siempre me golpean en la cara? Necesito sanacion', '', ''),
('low health', 'Can anybody heal me a bit?', 0, 0, '', 'Quelqu\'un peut-il me guérir un peu?', '', '谁能稍微治疗我一下？', '', '¿Alguien puede curarme un poco?', '', ''),

('low mana', 'OOM', 0, 0, '', 'MOO', '', '没蓝了', '', 'No tengo mana!', '', ''),
('low mana', 'I am out of mana', 0, 0, '', 'je n\'ai plus de mana', '', '我没蓝了', '', 'No tengo maná', '', ''),
('low mana', 'Damn I wasted all my mana on this', 0, 0, '', 'Putain j\'ai gaspillé tout mon mana sur ça', '', '该死，我把蓝都浪费在这上面了', '', 'Me cago en todo! Malgasté todo mi maná enseguida', '', ''),
('low mana', 'You should wait until I drink or regenerate my mana', 0, 0, '', 'Vous devriez attendre que je boive ou que je régénère mon mana', '', '你得等我喝点水或恢复蓝量', '', 'Deberías esperar hasta que beba o regenere mi maná', '', ''),
('low mana', 'Low mana', 0, 0, '', 'Mana faible', '', '蓝量低', '', 'Maná bajo', '', ''),
('low mana', 'No mana. Again?', 0, 0, '', 'Pas de mana. De nouveau?', '', '没蓝了，又是？', '', 'Sin maná. ¿Otra vez?', '', ''),
('low mana', 'Low mana. Wanna drink', 0, 0, '', 'Mana faible. Je veux boire', '', '蓝量低，想喝点', '', 'Maná bajo. Quiero beber', '', ''),
('low mana', 'Do we have a vending machine? Out of mana again', 0, 0, '', 'Avons-nous un distributeur automatique? Plus de mana à nouveau', '', '我们有自动售货机吗？又没蓝了', '', '¿Tenemos una máquina expendedora? Sin maná otra vez', '', ''),
('low mana', 'My mana is history', 0, 0, '', 'Mon mana appartient à l\'histoire', '', '我的蓝量成历史了', '', 'Mi maná es historia', '', ''),
('low mana', 'I\'d get some drinks next time. Out of mana', 0, 0, '', 'Je prendrais quelques verres la prochaine fois. Manque de mana', '', '下次我得带点喝的，没蓝了', '', '¡No tengo maná! Tengo que comprar bebidas la próxima vez...', '', ''),
('low mana', 'Where is my mana?', 0, 0, '', 'Où est mon mana?', '', '我的蓝呢？', '', '¿Dónde está mi maná?', '', ''),

('low ammo', 'I have few <ammo> left!', 0, 0, '', 'Il me reste peu de <ammo>!', '', '我的<ammo>不多了！', '', '¡Me quedan pocas <ammo>!', '', ''),
('low ammo', 'I need more <ammo>!', 0, 0, '', 'J\'ai besoin de plus de <ammo>!', '', '我需要更多<ammo>！', '', '¡Necesito más <ammo>!', '', ''),
('low ammo', '100 <ammo> left!', 0, 0, '', 'Il reste 100 <ammo>!', '', '还剩100个<ammo>！', '', '¡Quedan 100 <ammo>!', '', ''),

('no ammo', 'That\'s it! No <ammo>!', 0, 0, '', 'C\'est ça! Pas de <ammo>!', '', '完了！没<ammo>了！', '', '¡Eso es todo! ¡Sin <ammo>!', '', ''),
('no ammo', 'And you have my bow... Oops, no <ammo>!', 0, 0, '', 'Et vous avez mon arc... Oups, pas de <ammo>!', '', '你拿着我的弓……哎呀，没<ammo>了！', '', 'Y tú tienes mi arco... ¡Uy, sin <ammo>!', '', ''),
('no ammo', 'Need ammo!', 0, 0, '', 'Besoin de munitions!', '', '需要弹药！', '', '¡Necesito munición!', '', ''),

('aoe', 'Oh god!', 0, 0, '', 'Oh mon Dieu!', '', '我的天！', '', '¡Oh, Dios!', '', ''),
('aoe', 'I am scared', 0, 0, '', 'j\'ai peur', '', '我害怕', '', 'Tengo miedo', '', ''),
('aoe', 'We are done for', 0, 0, '', 'Nous avons fini pour', '', '我们完蛋了', '', 'Hemos terminado', '', ''),
('aoe', 'This is over', 0, 0, '', 'C\'est terminé', '', '这下完了', '', 'Esto se acabó', '', ''),
('aoe', 'This ends now', 0, 0, '', 'Cela se termine maintenant', '', '现在就结束了', '', 'Esto termina ahora', '', ''),
('aoe', 'Could somebody cast blizzard or something?', 0, 0, '', 'Quelqu\'un pourrait-il lancer le blizzard ou quelque chose comme ça?', '', '谁能扔个暴风雪啥的吗？', '', '¿Alguien podría lanzar ventisca o algo así?', '', ''),
('aoe', 'Damn. The tank aggroed all the mobs around', 0, 0, '', 'Mince. Le tank a agro toutes les mobs autour', '', '该死，坦克把周围的怪全拉住了', '', '¡Me cago en todo! El tanque atrajo a todos los mobs...', '', ''),
('aoe', 'We gonna die. We gonna die. We gonna die.', 0, 0, '', 'Nous allons mourir. Nous allons mourir. Nous allons mourir.', '', '我们要死了。我们要死了。我们要死了。', '', 'Vamos a morir. Vamos a morir. Vamos a morir.', '', ''),
('aoe', 'Whoa! So many toys to play with', 0, 0, '', 'Ouah ! Tant de jouets avec lesquels jouer', '', '哇！这么多玩具可以玩', '', '¡Vaya! Tantos juguetes con los que jugar', '', ''),
('aoe', 'I gonna kill them all!', 0, 0, '', 'Je vais tous les tuer!', '', '我要干掉他们全部！', '', '¡Voy a matarlos a todos!', '', ''),
('aoe', 'If the tank dies we are history', 0, 0, '', 'Si le tank meurt, nous appartenons à l\'histoire', '', '如果坦克死了，我们就完蛋了', '', 'Si el tanque muere somos historia', '', ''),
('aoe', 'Aaaaaargh!', 0, 0, '', 'Aaaaaargh!', '', '啊啊啊啊！', '', '¡Aaaaaargh!', '', ''),
('aoe', 'LEEEEERROOOYYYYYYYYYYYY JENNKINNNSSSSSS!!!!!!!', 0, 0, '', 'LEEEEERROOOYYYYYYYYYYYY JENNKINNNSSSSSS!!!!!!!', '', '李洛伊·詹金斯！！！', '', 'LEEEEERROOOYYYYYYYYYYYY JENNKINNNSSSSSS!!!!!!!', '', ''),
('aoe', 'Right. What do we have in AOE?', 0, 0, '', 'D\'accord. Qu\'avons-nous en dégats de zone?', '', '好吧，我们有啥群攻技能？', '', 'Claro. ¿Qué tenemos en AOE?', '', ''),
('aoe', 'This gets interesting', 0, 0, '', 'Cela devient intéressant', '', '这下有意思了', '', 'Esto se pone interesante', '', ''),
('aoe', 'Cool. Get them in one place for a good flamestrike', 0, 0, '', 'Cool. Rassemblez-les au même endroit pour un bon coup de flamme', '', '酷，把他们聚在一起，来个漂亮的火焰冲击', '', 'Genial! Ponlos todos juntos para un buen ataque en area', '', ''),
('aoe', 'Kill! Kill! Kill!', 0, 0, '', 'Tuer! Tuer! Tuer!', '', '杀！杀！杀！', '', '¡Matar! ¡Matar! ¡Matar!', '', ''),
('aoe', 'I think my pants are wet', 0, 0, '', 'Je pense que mon pantalon est mouillé', '', '我觉得我裤子湿了', '', 'Creo que mis pantalones están mojados', '', ''),
('aoe', 'We are history', 0, 0, '', 'Nous sommes l\'histoire', '', '我们完蛋了', '', 'Somos historia', '', ''),
('aoe', 'I hope healers are ready. Leeeeroy!', 0, 0, '', 'J\'espère que les guérisseurs sont prêts. Leeeeeroy!', '', '希望治疗者准备好了，李洛伊！', '', 'Espero que los sanadores estén listos. ¡Leeeroy!', '', ''),
('aoe', 'I hope they won\'t come for me', 0, 0, '', 'j\'espère qu\'ils ont gagné, qu\'ils ne viendront pas pour moi', '', '希望他们别冲着我来', '', 'Espero que no vengan por mí', '', ''),
('aoe', 'Oh no. I can\'t see at this slaugther', 0, 0, '', 'Oh non. Je ne peux pas voir à ce massacre', '', '哦不，我看不了这场屠杀', '', 'Oh, no. No puedo ver esta matanza', '', ''),

-- on looting
('loot', 'I hope there will be some money', 0, 0, '', 'j\'espère qu\'il y aura de l\'argent', '', '希望能有点金币', '', 'Espero que haya algo de dinero...', '', ''),
('loot', 'Loot! Loot!', 0, 0, '', 'Butin! Butin!', '', '战利品！战利品！', '', '¡Botín! ¡Botín!', '', ''),
('loot', 'My precious', 0, 0, '', 'Mon précieux', '', '我的宝贝', '', 'Mi tesoro...', '', ''),
('loot', 'I hope there is a shiny epic item waiting for me there', 0, 0, '', 'J\'espère qu\'il y a un objet épique brillant qui m\'attend ici', '', '希望那里有件闪亮的史诗装备等着我', '', 'Espero que haya un objeto épico brillante esperándome allí', '', ''),
('loot', 'I have deep pockets and bags', 0, 0, '', 'J\'ai des poches et des sacs profonds', '', '我的口袋和背包都很大', '', 'Tengo bolsillos profundos y infinitos', '', ''),
('loot', 'All is mine!', 0, 0, '', 'Tout est à moi!', '', '全是我的！', '', '¡Todo es mío!', '', ''),
('loot', 'Hope no gray shit today', 0, 0, '', 'J\'espère pas de merde grise aujourd\'hui', '', '希望今天别出灰色垃圾', '', 'Espero que hoy no haya mierda gris', '', ''),
('loot', 'This loot is MINE!', 0, 0, '', 'Ce butin est à MOI!', '', '这战利品是我的！', '', '¡Este botín es MÍO!', '', ''),
('loot', 'Looting is disgusting but I need money', 0, 0, '', 'Le pillage est dégoûtant mais j\'ai besoin d\'argent', '', '拾取很恶心，但我需要钱', '', 'El saqueo es repugnante pero necesito dinero...', '', ''),
('loot', 'Gold!', 0, 0, '', 'Or!', '', '金币！', '', '¡Oro!', '', ''),
('loot', 'OK. Let\'s see what they\'ve got', 0, 0, '', 'D\'ACCORD. Voyons ce qu\'ils ont', '', '好吧，看看他们有啥', '', 'OK. Veamos qué tienen', '', ''),
('loot', 'Do not worry. I will loot eveything', 0, 0, '', 'Ne t\'inquiète pas. je vais tout piller', '', '别担心，我会把所有东西都捡了', '', 'No te preocupes. Saquearé todo', '', ''),
('loot', 'I am loot ninja', 0, 0, '', 'je suis un ninja de butin', '', '我是拾取忍者', '', 'Soy un ladron de tesoros', '', ''),
('loot', 'Do I neeed to roll?', 0, 0, '', 'Dois-je recommencer?', '', '我需要掷骰子吗？', '', '¿Necesito lanzar dados?', '', ''),
('loot', 'Somebody explain me, where they did put all this stuff?', 0, 0, '', 'Quelqu\'un m\'explique, où ils ont mis tout ça?', '', '谁能解释一下，他们把这些东西都藏哪了？', '', 'Alguien me explica, ¿dónde pusieron todas estas cosas?', '', ''),
('loot', 'No, I won\'t loot gray shit', 0, 0, '', 'Non, je ne pillerai pas la merde grise', '', '不，我不捡灰色垃圾', '', 'No, no saquearé mierda gris', '', ''),
('loot', 'I\'m first. I\'m first. I\'m first.', 0, 0, '', 'Je suis le premier. Je suis le premier. Je suis le premier.', '', '我是第一个！我是第一个！我是第一个！', '', '¡Es mio! ¡Es mio! ¡Es mio!', '', ''),
('loot', 'Give me your money!', 0, 0, '', 'Donne-moi ton argent!', '', '把你的钱给我！', '', '¡Dame todo tu dinero!', '', ''),
('loot', 'My pockets are empty, I need to fill them', 0, 0, '', 'Mes poches sont vides, j\'ai besoin de les remplir', '', '我的口袋空空如也，得填满它们', '', 'Mis bolsillos están vacíos y necesitan llenarse!', '', ''),
('loot', 'I\'ve got a new bag for this', 0, 0, '', 'J\'ai un nouveau sac pour ça', '', '我为此准备了个新背包', '', 'Tengo una nueva bolsa para esto', '', ''),
('loot', 'I hope I won\'t aggro anybody while looting', 0, 0, '', 'J\'espère que je n\'agresserai personne pendant le pillage', '', '希望我在拾取时别引到怪', '', 'Espero no ofender a nadie mientras saqueo', '', ''),
('loot', 'Please don\'t watch. I am looting', 0, 0, '', 'S\'il vous plaît, ne regardez pas. je pille', '', '请别看，我在拾取', '', 'Por favor, no mires. Estoy saqueando', '', ''),
('loot', 'Ha! You won\'t get any piece of it!', 0, 0, '', 'Ha! Vous n\'en aurez aucun morceau!', '', '哈！你别想分到一点！', '', '¡Ja! ¡No obtendrás nada de eso!', '', ''),
('loot', 'Looting is cool', 0, 0, '', 'Le pillage c\'est cool', '', '拾取真酷', '', 'Saquear es genial', '', ''),
('loot', 'I like new gear', 0, 0, '', 'j\'aime le nouveau matos', '', '我喜欢新装备', '', 'Me gusta el equipo nuevo', '', ''),
('loot', 'I\'l quit if there is nothing valuable again', 0, 0, '', 'J\'arrêterai s\'il n\'y a plus rien de précieux', '', '如果再没值钱的东西我就放弃了', '', 'Me doy por vencido si no hay nada util', '', ''),
('loot', 'I hope it is be a pretty ring', 0, 0, '', 'j\'espère que c\'est une jolie bague', '', '希望是个漂亮的戒指', '', 'Espero que sea un anillo bonito', '', ''),
('loot', 'I\'l rip the loot from you', 0, 0, '', 'Je t\'arracherai le butin', '', '我会从你手里抢走战利品', '', 'Te arrancaré el botín', '', ''),
('loot', 'Everybody stay off. I\'m going to loot', 0, 0, '', 'Tout le monde reste à l\'écart. je vais piller', '', '大家都别动，我要拾取', '', '¡Alejaos todos! Voy a lootear', '', ''),
('loot', 'Sweet loot', 0, 0, '', 'Doux butin', '', '甜美的战利品', '', 'Dulce botín', '', ''),
('loot', 'The Roll God! Give me an epic today', 0, 0, '', 'Le Dieu du rouleau ! Donnez-moi de l\'épique aujourd\'hui', '', '骰子之神！今天赐我一件史诗吧', '', '¡El Dios de los dados! Dame un epico hoy', '', ''),
('loot', 'Please give me new toys', 0, 0, '', 'S\'il te plaît, donne-moi de nouveaux jouets', '', '请给我些新玩具', '', 'Por favor, dame nuevos juguetes', '', ''),
('loot', 'I hope they carry tasties', 0, 0, '', 'J\'espère qu\'ils portent des saveurs', '', '希望他们身上有好吃的', '', 'Espero que caiga algo sabrosos', '', ''),
('loot', 'The gold is mine. I\'l leave everyting, I promise', 0, 0, '', 'L\'or est à moi. Je laisse tout, je promets', '', '金币是我的，其他都留给你们，我保证', '', 'El oro es mío. Dejaré todo lo demas, lo prometo!', '', ''),
('loot', 'No, I can\'t resist', 0, 0, '', 'Non, je ne peux pas résister', '', '不，我忍不住', '', 'No, no puedo resistir', '', ''),
('loot', 'I want more!', 0, 0, '', 'Je veux plus!', '', '我想要更多！', '', '¡Quiero más!', '', ''),

-- wait signals
('wait_travel_close', 'I am close, wait for me!', 0, 0, '', '', '', '我就在附近，等我！', '', '¡Estoy cerca, espérame!', '', 'Я тут рядом, подожди!'),
('wait_travel_close', 'I\'m not far, please wait!', 0, 0, '', '', '', '我不远，请等我！', '', 'No estoy lejos, ¡esperame por favor!', '', 'Я недалеко, погодите!'),

('wait_travel_medium', 'I\'m heading to your location', 0, 0, '', '', '', '我正朝你的位置赶去', '', 'Me dirijo a tu ubicación', '', 'Двигаюсь к тебе'),
('wait_travel_medium', 'I\'m coming to you', 0, 0, '', '', '', '我正朝你过来', '', 'Voy hacia ti', '', 'Иду к тебе'),

('wait_travel_far', 'I\'m traveling to your location', 0, 0, '', '', '', '我正前往你的位置', '', 'Voy a viajar a tu ubicación', '', 'Направляюсь к тебе'),
('wait_travel_far', 'I\'m trying to get to you', 0, 0, '', '', '', '我正努力赶到你那里', '', 'Estoy tratando de llegar a ti', '', 'Пытаюсь до тебя добраться'),


-- commands
('equip_command', 'Equipping %item', 0, 0, '', '', '', '装备了 %item', '', 'Me he equipado %item', '', ''),

('unequip_command', '%item unequipped', 0, 0, '', '', '', '%item 已卸下', '', 'Me he quitado %item', '', ''),

('auto_learn_spell', 'I have learned the spells: %spells', 0, 0, '', '', '', '我学会了这些法术：%spells', '', 'He aprendido los hechizos: %spells', '', ''),

('use_command_item_cooldown', '%item is in cooldown', 0, 0, '', '', '', '%item 还在冷却中', '', '%item esta recargandose', '', ''),

('use_command_item_not_owned', 'I don''t have %item in my inventory', 0, 0, '', '', '', '我的背包里没有 %item', '', 'No tengo %item en mi inventario', '', ''),

('use_command_invalid_item', 'The item with the id %item does not exist', 0, 0, '', '', '', 'ID 为 %item 的物品不存在', '', 'El objeto con el id %item no existe', '', ''),

('use_command_socket', 'Socketing %gem into %item', 0, 0, '', '', '', '将 %gem 镶嵌到 %item 中', '', 'Insertando %gem en %item', '', ''),

('use_command_item_error', 'I can''t use %item', 0, 0, '', '', '', '我无法使用 %item', '', 'No puedo usar %item', '', ''),

('following', 'Following', 0, 0, '', '', '', '正在跟随', '', 'Te sigo', '', 'Следую'),

('staying', 'Staying', 0, 0, '', '', '', '留在原地', '', 'Me quedo aqui', '', 'Стою'),

('fleeing', 'Fleeing', 0, 0, '', '', '', '正在逃跑', '', 'Huyendo', '', 'Убегаю'),

('fleeing_far', 'I won\'t flee with you, you are too far away', 0, 0, '', '', '', '我不会跟你一起逃，你离我太远了', '', 'No huiré contigo, estás demasiado lejos', '', 'Я с тобой не побегу, ты слишком далеко'),

('grinding', 'Grinding', 0, 0, '', '', '', '正在刷怪', '', 'Farmeando', '', 'Гриндю'),

('attacking', 'Attacking', 0, 0, '', '', '', '正在攻击', '', 'Atacando', '', 'Атакую'),

('error_far', 'It is too far away', 0, 0, '', '', '', '目标太远了', '', 'Estas demasiado lejos', '', 'Слишком далеко'),

('error_water', 'It is under water', 0, 0, '', '', '', '目标在水下', '', 'Estas debajo del agua', '', 'Это под водой'),

('error_cant_go', 'I can\'t go there', 0, 0, '', '', '', '我去不了那里', '', 'No puedo ir allí', '', 'Я не могу туда пройти'),

('error_guild', 'I\'m not in your guild!', 0, 0, '', '', '', '我不在你的公会里！', '', '¡No estoy en tu hermandad!', '', 'Я не в твоей гильдии'),

('error_gbank_found', 'Can not find a guild bank nearby', 0, 0, '', '', '', '附近找不到公会银行', '', 'No se puede encontrar un banco de hermandad cercano', '', 'Не могу найти гильд банк рядом'),

('error_cant_put', 'I can\'t put ', 0, 0, '', '', '', '我无法存放', '', 'No puedo depositar ', '', 'Я не могу положить'),

('error_gbank_rights', 'I have no rights to put items in the first guild bank tab', 0, 0, '', '', '', '我没有权限在公会银行第一个标签页存放物品', '', 'No tengo derechos para depositar objetos en la primera pestaña del banco de hermandad', '', 'Нет прав чтобы класть вещи в первую вкладку гильд банка'),

('gbank_put', ' put to guild bank', 0, 0, '', '', '', '已存入公会银行', '', ' depositado en el banco de hermandad', '', ' теперь в гильд банке'),

('free_moving', 'Free moving', 0, 0, '', '', '', '自由移动中', '', 'Moviendome libremente', '', ''),

('guarding', 'Guarding', 0, 0, '', '', '', '正在守卫', '', 'Protegiendo la posición', '', ''),

('use_command', 'Using %target', 0, 0, '', '', '', '正在使用 %target', '', 'Usando %target', '', ''),

('command_target_unit', 'on %unit', 0, 0, '', '', '', '对 %unit', '', 'en %unit', '', ''),

('use_command_remaining', '(%amount available)', 0, 0, '', '', '', '（剩余 %amount）', '', '(%amount restante)', '', ''),

('use_command_last', '(the last one)', 0, 0, '', '', '', '（最后一个）', '', '(el último)', '', ''),

('use_command_socket_error', 'The socket does not fit', 0, 0, '', '', '', '插槽不匹配', '', 'La ranura no sirve', '', ''),

('command_target_trade', 'on trade item', 0, 0, '', '', '', '对交易物品', '', 'en objeto comerciado', '', ''),

('command_target_self', 'on myself', 0, 0, '', '', '', '对自己', '', 'en mi', '', ''),

('command_target_item', 'on %item', 0, 0, '', '', '', '对 %item', '', 'en %item', '', ''),

('command_target_go', 'on %gameobject', 0, 0, '', '', '', '对 %gameobject', '', 'en %gameobject', '', ''),

('loot_command', 'Looting %item', 0, 0, '', '', '', '正在拾取 %item', '', 'Despojando %item', '', ''),

('cast_spell_command_summon', 'Summoning %target', 0, 0, '', '', '', '正在召唤 %target', '', 'Invocando a %target', '', ''),
('cast_spell_command_summon_error_members', 'I don\'t have enough party members around to cast a summon', 0, 0, '', '', '', '周围没有足够的队友，无法进行召唤', '', 'No tengo suficientes miembros de grupo alrededor para invocar', '', ''),
('cast_spell_command_summon_error_target', 'Failed to find the summon target', 0, 0, '', '', '', '找不到召唤目标', '', 'No he podido encontrar al objetivo de la invocación', '', ''),
('cast_spell_command_summon_error_combat', 'I can\'t summon while I\'m in combat', 0, 0, '', '', '', '战斗中无法进行召唤', '', 'No puedo invocar durante un combate', '', ''),

('cast_spell_command_error_unknown_spell', 'I don\'t know the spell %spell', 0, 0, '', '', '', '我不会 %spell 这个法术', '', 'No conozco el hechizo %spell', '', ''),

('cast_spell_command_spell', 'Casting %spell', 0, 0, '', '', '', '正在施放 %spell', '', 'Lanzando %spell', '', ''),

('cast_spell_command_craft', 'Crafting %spell', 0, 0, '', '', '', '正在制作 %spell', '', 'Creando %spell', '', ''),

('cast_spell_command_error', 'Cannot cast %spell', 0, 0, '', '', '', '无法施放 %spell', '', 'No puedo lanzar %spell', '', ''),
('cast_spell_command_error_failed', 'Failed to cast %spell', 0, 0, '', '', '', '施放 %spell 失败', '', 'Fallo al lanzar %spell', '', ''),

('cast_spell_command_amount', ' |cffffff00(x%amount left)|r', 0, 0, '', '', '', ' |cffffff00(剩余 x%amount)|r', '', ' |cffffff00(x%amount restante)|r', '', ''),

('dummy_end', 'dummy', 0, 0, '', '', '', '', '', '', '', '');

DROP TABLE IF EXISTS `ai_playerbot_texts_chance`;

CREATE TABLE IF NOT EXISTS `ai_playerbot_texts_chance` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `probability` bigint(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=UTF8;

/*!40000 ALTER TABLE `ai_playerbot_texts_chance` DISABLE KEYS */;
INSERT INTO `ai_playerbot_texts_chance` (`id`, `name`, `probability`) VALUES
	(1, 'taunt', 30),
	(2, 'aoe', 75),
	(3, 'loot', 20);
/*!40000 ALTER TABLE `ai_playerbot_texts_chance` ENABLE KEYS */;
