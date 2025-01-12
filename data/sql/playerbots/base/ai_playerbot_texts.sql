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

-- Updated Chatter Text over 900 lines added by https://github.com/AzoraNova on 12 jan 2025

-- strings
('string_unknown_area', 'the middle of nowhere', 0, 0, '', '', '', '', '', '', '', ''),
('string_unknown_area', 'an undisclosed location', 0, 0, '', '', '', '', '', '', '', ''),
('string_unknown_area', 'somewhere', 0, 0, '', '', '', '', '', '', '', ''),
('string_empty_link', 'something', 0, 0, '', '', '', '', '', '', '', ''),


-- looting item events
-- usable placeholders:
-- %item_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_looting_item_normal', 'wonder what %item_link tastes like', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'nooo, I got %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'aww not this trash again %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'seem to be looting trash %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'well, it’s better than nothing I guess %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'not sure what to do with %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'wonder what %item_link tastes like', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'well, I can keep looting %item_link all day', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'another day, another %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'looted some %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'some %item_link is alright', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'not bad, I just got %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'just looted %item_link in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'I could put that to good use %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'money money money %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'got %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', '%item_link is hunter bis', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', '%item_link is %my_class bis', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', 'RNG is not bad today %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', 'sweet %item_link, freshly looted', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', 'wow, I just got %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', '%item_link is hunter bis', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', '%item_link is %my_class bis', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', 'RNG is not bad today %item_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_rare', 'sweet %item_link, freshly looted', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_epic', 'OMG, look at what I just got %item_link!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_legendary', 'No @#$@% way! This can not be, I got %item_link, this is insane', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_artifact', 'No @#$@% way! This can not be, I got %item_link, this is insane', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Seriously? Another %item_link? My luck is broken.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'This %item_link is not even worth the pixels it’s printed on.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I got %item_link... Ugh, here we go again.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I don’t even know why I’m picking up %item_link, it’s useless.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Look at this shiny %item_link... too bad it’s garbage.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'This %item_link isn’t even worth the paper it’s written on.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I guess %item_link is better than nothing? Barely.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Not sure what to do with %item_link, maybe I’ll just sell it for 1 silver.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Another %item_link... well, at least I’m getting something.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I hope %item_link is actually worth something because it looks awful.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Is %item_link a joke? Someone’s laughing at me.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Why do I keep getting %item_link... did I anger RNG?', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'It’s like %item_link is following me around. I don’t want it!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'There’s %item_link, again. What a surprise.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I guess %item_link is good for vendor trash?', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Noooo, not %item_link again! RNG, please!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Well, at least %item_link isn’t completely useless, right?', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Another %item_link? RNG, do you even care?', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Maybe %item_link will be worth something in 10 years.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I guess I’ll just admire how %item_link looks... and throw it away.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Who needs %item_link? Not me, that’s for sure.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Maybe %item_link will be useful... just kidding, it’s trash.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'The universe has given me %item_link. Great.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Well, I have %item_link. Can’t say I’m thrilled.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'What am I even going to do with %item_link? Can’t even sell it.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Let’s add %item_link to my growing pile of disappointment.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Thanks for %item_link... it’s exactly what I didn’t need.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Another %item_link... RNG, we need to talk.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I’ve looted so many %item_link, I could open a store with them.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'A shiny %item_link... maybe for the trash pile.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'It’s %item_link, everybody. Don’t get too excited.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Well, at least it’s %item_link... could be worse.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'I’ll take %item_link over nothing.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Another %item_link. Not the worst.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_poor', 'Hey, %item_link... it’s okay, I suppose.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Hey, %item_link. Not bad, not great, but I’ll take it.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Could be worse... I just looted %item_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'I guess %item_link will come in handy for something.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Another day, another %item_link. Can’t complain.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Well, %item_link is alright, I guess.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Just got %item_link. It’s not the best, but it’s something.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Could be worse, at least it’s %item_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', '%item_link is good enough, I’m not mad.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'It’s no legendary, but %item_link will do.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Not a bad drop... %item_link is alright.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'I’ll take %item_link over nothing.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Not the best, but %item_link is a nice addition.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Getting %item_link again. I guess it’s better than nothing.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', '%item_link again, huh? I’m not thrilled, but I’ll live.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Another %item_link? I’m okay with that.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Could’ve gotten worse, but %item_link isn’t so bad.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'It’s %item_link... but at least it’s something.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'I’ll take %item_link, could be worse.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Looted %item_link, nothing special but okay.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Not what I wanted, but %item_link will do for now.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Another %item_link. Not amazing, but it’ll do.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Looted %item_link, could be worse.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'Well, %item_link is better than a poke in the eye.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_normal', 'I’ll take %item_link and move on.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Nice! A solid %item_link for the collection.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'I’ll take that %item_link any day.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Not bad, %item_link might come in handy.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Got myself a %item_link, now we’re talking.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'That %item_link is better than most, I’ll take it.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'I could use %item_link for something good.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Not bad, %item_link will work just fine.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Another %item_link, at least it’s not useless.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'That %item_link might make a decent profit.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'A solid %item_link, now the day is looking better.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Well, I didn’t expect %item_link, but I’ll take it.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Not sure what to do with %item_link, but I’m happy to have it.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Not my first choice, but I’ll happily accept %item_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'That %item_link is exactly what I needed today.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'I’ll never say no to a nice %item_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'Another %item_link. I’ll take it, no complaints here.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', 'At least %item_link has some use. Not the worst loot.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_looting_item_uncommon', '%item_link might just be the one thing I need for the day.', 0, 0, '', '', '', '', '', '', '', ''),



-- quest events
-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_accepted_generic', 'I have just taken the %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'just accepted %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', '%quest_link gonna try to complete this one', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'took %quest_link in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I just picked up %quest_link, time to get to work!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Got %quest_link, let’s see what this one is about.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Just grabbed %quest_link, let’s do this!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Another quest, another %quest_link. Let’s go!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Looks like it’s %quest_link time, wish me luck.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Accepted %quest_link, let’s see how tough this one is.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Picked up %quest_link, time to go make some progress.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Just took %quest_link. Let’s see how hard this one is.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Accepted %quest_link, let’s hope there’s no surprise boss at the end.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'On my way to complete %quest_link. Here we go!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I guess I’m doing %quest_link now. Let’s hope it’s worth it.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Picked up %quest_link. Let’s get moving!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I’ve just taken %quest_link. This should be a piece of cake.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I accepted %quest_link, now to knock it out of the park.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Took %quest_link in %zone_name. Let’s do this!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'On my way to finish %quest_link. Let’s make it quick!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Accepted %quest_link. Wonder what awaits me in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Got %quest_link, hope it doesn’t take forever.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Just grabbed %quest_link. Here we go again.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Looks like %quest_link is the next one on my list.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Off to complete %quest_link. I hope this one is fun.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I’m diving into %quest_link. This will be an adventure!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Accepted %quest_link in %zone_name. Let’s see what this brings!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Just took %quest_link. Time to see what I’m up against.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Another %quest_link. Let’s get this done quickly!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I just took %quest_link. This will be a fun one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Taking on %quest_link in %zone_name. Let’s go!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Grabbed %quest_link. Can’t wait to see what happens next.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Took %quest_link. Time to dive in and get it done.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Alright, %quest_link is on. Let’s get to work!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'On to %quest_link. Let’s hope it’s not too hard.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Just grabbed %quest_link. Should be an easy one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'I’ve accepted %quest_link. Let’s make it quick!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Took %quest_link, now to see what it’s all about.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_accepted_generic', 'Just accepted %quest_link, let’s get moving!', 0, 0, '', '', '', '', '', '', '', ''),


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
('broadcast_quest_update_add_kill_objective_completed', 'Finally done with the %quest_obj_name for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'finally got %quest_obj_available/%quest_obj_required of %quest_obj_name for the %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', '%quest_obj_full_formatted for the %quest_link, at last', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Oof, got %quest_obj_available/%quest_obj_required %quest_obj_name for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'still need %quest_obj_missing more of %quest_obj_name for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', '%quest_obj_full_formatted, still working on %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', '%quest_obj_name complete for %quest_link, what a grind!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Getting close to completing %quest_obj_name for %quest_link, just need %quest_obj_missing more.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', '%quest_obj_name is almost done, need just %quest_obj_missing more for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'So far, %quest_obj_available/%quest_obj_required %quest_obj_name completed for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'Finally finished %quest_obj_name for %quest_link! That took a while!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Still going strong, got %quest_obj_available/%quest_obj_required of %quest_obj_name for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'Yes! Completed %quest_obj_name for %quest_link. On to the next one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Made progress on %quest_obj_name, %quest_obj_available/%quest_obj_required completed for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', '%quest_obj_full_formatted... almost there for %quest_link!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Just need %quest_obj_missing more %quest_obj_name for %quest_link, almost done!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'Finally done with %quest_obj_name for %quest_link, what a relief!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Got %quest_obj_available/%quest_obj_required of %quest_obj_name for %quest_link, making progress!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'The %quest_obj_name for %quest_link is complete! Now for the next task.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', '100% of %quest_obj_name done for %quest_link, now onto the rewards!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Still need %quest_obj_missing more of %quest_obj_name for %quest_link, almost done!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Working on %quest_obj_name for %quest_link, currently %quest_obj_available/%quest_obj_required done.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', '%quest_obj_full_formatted for %quest_link. That was a long one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Just need a few more of %quest_obj_name for %quest_link, and I’m done!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'Another objective completed! %quest_obj_full_formatted for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_progress', 'Still grinding on %quest_obj_name for %quest_link, almost there with %quest_obj_available/%quest_obj_required.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_kill_objective_completed', 'Objective done! %quest_obj_full_formatted for %quest_link. On to the next one!', 0, 0, '', '', '', '', '', '', '', ''),



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
('broadcast_quest_update_add_item_objective_completed', 'Finally done with the %item_link for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'finally got %quest_obj_available/%quest_obj_required of %item_link for the %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', '%quest_obj_full_formatted for the %quest_link, at last', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Oof, got %quest_obj_available/%quest_obj_required %item_link for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'still need %quest_obj_missing more of %item_link for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', '%quest_obj_full_formatted, still working on %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Completed the %item_link objective for %quest_link. Finally!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Making progress! %quest_obj_available/%quest_obj_required %item_link for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', '%quest_obj_name is halfway done, need %quest_obj_missing more %item_link for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Still need %quest_obj_missing of %item_link for %quest_link, but getting closer!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Finally finished %item_link for %quest_link, now time for the next task!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Almost there! Got %quest_obj_available/%quest_obj_required %item_link for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Yes! Completed %item_link for %quest_link. Now moving on to the next objective.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Still need %quest_obj_missing more %item_link for %quest_link, but progress is being made!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Made good progress on %item_link for %quest_link, need %quest_obj_missing more.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', '100% of %item_link done for %quest_link! Time to move forward.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Got %quest_obj_available/%quest_obj_required %item_link for %quest_link. I can feel the finish line!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'That’s it, %item_link complete for %quest_link. What a relief!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Working hard on %item_link for %quest_link, still need %quest_obj_missing more.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Almost finished with %item_link for %quest_link, just %quest_obj_missing more left.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Completed %item_link for %quest_link, the grind is real!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Still chipping away at %item_link for %quest_link, %quest_obj_available/%quest_obj_required done so far.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', '%item_link for %quest_link is getting closer to completion, %quest_obj_available/%quest_obj_required done.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Yes! %item_link finished for %quest_link! That one took a bit longer.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'I have %quest_obj_available/%quest_obj_required of %item_link for %quest_link, still going strong!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Done with %item_link for %quest_link. On to the next one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_completed', 'Another one down! %item_link is complete for %quest_link!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Still grinding on %item_link for %quest_link, got %quest_obj_available/%quest_obj_required done.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_add_item_objective_progress', 'Getting there! Need %quest_obj_missing more %item_link for %quest_link.', 0, 0, '', '', '', '', '', '', '', ''),


-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_update_failed_timer', 'Failed to finish %quest_link in time...', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_failed_timer', 'Ran out of time for %quest_link :(', 0, 0, '', '', '', '', '', '', '', ''),


-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_update_complete', 'I have completed all objectives for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_complete', 'Completed all objectives for %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_update_complete', 'Gonna turn in the %quest_link soon, just finished all objectives', 0, 0, '', '', '', '', '', '', '', ''),


-- usable placeholders:
-- %quest_link
-- %zone_name
-- %area_name
-- %my_race
-- %my_class
-- %my_level
('broadcast_quest_turned_in', 'Yess, I have finally turned in the %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'turned in the %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'managed to finish %quest_link, just turned in', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'just turned in %quest_link', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'just turned in %quest_link in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Another quest completed! %quest_link turned in', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Mission accomplished! %quest_link is turned in', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Finally, %quest_link turned in! That was a journey!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Just completed %quest_link and turned it in, feeling good!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Turned in %quest_link in %zone_name. Now, on to the next challenge!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'I have turned in %quest_link. One more down!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Successfully turned in %quest_link, that took a while!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Completed and turned in %quest_link in %zone_name. Feels great!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Quest complete! %quest_link turned in and ready for the next one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Finally turned in %quest_link in %zone_name. That one was tough!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'That’s it, %quest_link turned in! Now to collect the reward!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Finished %quest_link! Now time to turn it in and move on.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Just turned in %quest_link, feeling accomplished!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'I’ve turned in %quest_link. Let’s see what’s next!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Just wrapped up %quest_link and turned it in. On to the next one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Another one in the books, %quest_link turned in', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Turned in %quest_link! Time to collect my rewards.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Completed and turned in %quest_link, what a relief!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'That was quick! %quest_link turned in already!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Just turned in %quest_link in %zone_name. Ready for the next adventure!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Successfully turned in %quest_link. Feels like progress!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'Mission complete! %quest_link is turned in!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_quest_turned_in', 'I just finished %quest_link and turned it in, on to the next one!', 0, 0, '', '', '', '', '', '', '', ''),



-- kill mob events
-- usable placeholders:
-- %victim_name
-- %zone_name
-- %area_name
-- %victim_level
-- %my_race
-- %my_class
-- %my_level
('broadcast_killed_normal', 'another %victim_name down', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'I keep killing %victim_name, nothing to talk about', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'another %victim_name bites the dust', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'one less %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_elite', 'Took down this elite bastard %victim_name!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'killed elite %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rareelite', 'Fooof, managed to take down %victim_name!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_worldboss', 'That was sick! Just killed %victim_name! Can tell tales now', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rare', 'Yoo, I just killed %victim_name!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'killed rare %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_unknown', 'WTF did I just kill? %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_pet', 'Just killed that pet %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Oh yeah, I just killed %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'killed %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'another %victim_name down', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'I keep killing %victim_name, nothing to talk about', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'another %victim_name bites the dust', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'one less %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'Just took down %victim_name like it’s nothing', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', '%victim_name didn’t stand a chance', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'One more %victim_name out of the way', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', '%victim_name was too easy, on to the next one', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'Another %victim_name defeated. Easy peasy', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', '%victim_name wasn’t tough at all', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'Down goes %victim_name, nothing special', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', 'That was too easy, %victim_name down', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_normal', '%victim_name dropped so fast, blink and it’s over', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_elite', 'Took down this elite %victim_name!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_elite', 'Elite %victim_name is no more', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_elite', 'Took down the elite %victim_name like a champ', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_elite', 'Elite monster %victim_name, no match for me!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_elite', 'Finally took down the mighty %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'killed elite %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Just got the elite %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Killed the elite %victim_name. Too easy', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Took down %victim_name in %zone_name, what a fight', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Killed elite %victim_name. Guess I’m unstoppable', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rareelite', 'Fooof, managed to take down %victim_name!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rareelite', 'Rare elite %victim_name has fallen', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rareelite', '%victim_name is no more. Another rare elite slain', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rareelite', 'Rare elite %victim_name down. I’m on fire!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rareelite', 'I just killed the rare elite %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_worldboss', 'That was awesome! Just killed %victim_name! What a battle', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_worldboss', 'World boss %victim_name is down! That was epic', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_worldboss', 'I’ve slain the world boss %victim_name. Let’s celebrate!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_worldboss', 'Killed %victim_name! That world boss was no joke!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_worldboss', 'Just killed %victim_name, that was legendary', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rare', 'Yoo, I just killed %victim_name!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rare', 'Rare %victim_name down. That was a fun fight', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rare', 'I just killed the rare %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rare', 'Rare %victim_name defeated in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_rare', 'Just killed rare %victim_name in %zone_name. That was quick!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'killed rare %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Got the rare %victim_name in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'I killed %victim_name in %zone_name. That felt good', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Took down %victim_name in %zone_name. They didn’t stand a chance', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_player', 'Just killed %victim_name in %zone_name, easy', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_unknown', 'What did I just kill? %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_unknown', 'What kind of %victim_name is that? Never seen anything like it', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_unknown', 'Just killed something strange, %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_unknown', '%victim_name was weird, but down they go', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_pet', 'Just defeated %victim_name', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_pet', 'Goodbye, %victim_name. You were just a pet', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_pet', 'Just took down the pet %victim_name. That was unexpected', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_killed_pet', 'Down goes %victim_name. Better luck next time, pet', 0, 0, '', '', '', '', '', '', '', ''),



-- levelup events
-- usable placeholders:
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('broadcast_levelup_generic', '%my_level Ding!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Yess, I am level %my_level!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'I just leveled up', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'I am level %my_level!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'getting stronger, already level %my_level!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Just reached level %my_level!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'OMG, finally level %my_level!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', '%my_level!!! can do endgame content now', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'fresh new level %my_level %my_class!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'one more level %my_level %my_race %my_class!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Ding! Another level up!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'I’m officially level %my_level now!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Level %my_level, feeling stronger already!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Up I go! Level %my_level reached', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'That was quick, level %my_level already!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Just hit level %my_level, let’s do this!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Level %my_level, on to bigger things!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Level %my_level, I’m on fire now!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'Level %my_level complete, onward!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_generic', 'I’m now level %my_level, what’s next?', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Level %my_level!!! It’s happening!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Just hit level %my_level, this feels amazing!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Level %my_level and I’m only getting started!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Wow, level %my_level already, I’m unstoppable!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Reaching new heights at level %my_level!!!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Level %my_level and still pushing forward!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Level %my_level, I’m too strong now!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Hitting %my_level, it feels so good!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'I’m level %my_level now! Let’s go!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_10x', 'Level %my_level, still a lot to go!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Finally hit level %my_level, I can do anything!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Max level %my_level, endgame here I come!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level! Ready for the endgame!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Endgame unlocked at level %my_level, bring it on!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'I’ve reached level %my_level, time to shine!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, let’s do some endgame raiding!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'At level %my_level, nothing can stop me now!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, all systems go for high-level content!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Yes! Level %my_level, %my_class ready for everything!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, it’s time to take on the world!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Reached level %my_level, let’s take on those elite bosses!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Now I’m %my_level, the real adventure begins!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level! Time for some serious action!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level and still growing stronger!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Finally at level %my_level! Let’s show off what I can do!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level is here! I’m ready for the big leagues!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Just hit %my_level, get ready for the real challenges!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level complete, it’s time for the epic content!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Fresh out of %my_level! Ready for endgame dungeons!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Now that I’m level %my_level, nothing is beyond my reach!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'It’s official! Level %my_level, time to dominate!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Made it to level %my_level! Time to make my mark on the world!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level unlocked! The real fun begins now!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, and the world is my playground!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, ready to show the world my power!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Just hit %my_level, I’m going all in now!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, I can finally take on everything!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Level %my_level, I’m feeling unstoppable!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_levelup_max_level', 'Just reached level %my_level, can’t wait to dive into the endgame!', 0, 0, '', '', '', '', '', '', '', ''),


-- guild
-- usable placeholders:
-- %other_name
-- %other_class
-- %other_race
-- %other_level
('broadcast_guild_promotion', 'Good job %other_name. You deserved this.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'That was awful %other_name. I hate to do this but...', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Congrats %other_name, you’ve earned this promotion!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Well deserved %other_name, welcome to the next level!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', '%other_name is leveling up! Congratulations on the promotion!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Promotion time! %other_name, you’ve earned this one!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'A new role for %other_name! Well done, keep it up!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Huge congrats to %other_name, you’ve earned a well-deserved promotion!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Let’s all give a round of applause to %other_name, for the promotion!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Cheers to %other_name on the promotion! Keep shining!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Congrats %other_name, you’re moving up! Well done!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_promotion', 'Another step up for %other_name! Congratulations!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'Tough decision, but %other_name, you’ve been demoted. Time to step up!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'Sorry %other_name, but it’s time for a demotion. Hope you come back stronger!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'Unfortunately, %other_name, it’s a demotion this time. You’ll get ''em next time!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'A hard decision, but %other_name, you’ve been demoted. Time to grind back up!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'We hate to see it, but %other_name, it’s a demotion for now.', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'Sorry %other_name, your rank has been lowered. Let’s work harder together!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'A demotion for %other_name... Time to focus and rise back up!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'Not what we wanted, but %other_name, you’ve been demoted. Learn and come back stronger!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'A tough moment for %other_name, but we know you can bounce back from this demotion!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'Sorry %other_name, but a demotion has been applied. It’s time for improvement!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_demotion', 'We didn’t want this for %other_name, but a demotion has occurred. Time to get better!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_recruitment', 'Looking for fresh blood! Join the guild, be part of something great!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_recruitment', 'Recruiting members! If you’re ready to join us, the guild door is open!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_recruitment', 'The guild is looking for skilled adventurers. Join us and thrive!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_recruitment', 'Want to join a great guild? We’re looking for new members! Come on in!', 0, 0, '', '', '', '', '', '', '', ''),
('broadcast_guild_recruitment', 'The guild is growing! Join us for exciting adventures and rewards!', 0, 0, '', '', '', '', '', '', '', ''),



-- /////////////////////////////
-- suggestions


-- random instance
-- usable placeholders:
-- %my_role
-- %instance_name
-- %my_class
-- %my_race
-- %my_level
('suggest_instance', 'Anyone wants %instance_name?', 0, 0, '', 'Quelqu’un fait %instance_name ?', '', '', '', '¿Alguien quiere ir a %instance_name?', '', ''),
('suggest_instance', 'Any groups for %instance_name?', 0, 0, '', 'Des groupes pour %instance_name ?', '', '', '', '¿Algún grupo para %instance_name?', '', ''),
('suggest_instance', 'Need help for %instance_name?', 0, 0, '', 'Besoin d’aide pour %instance_name ?', '', '', '', '¿Alguien necesita ayuda con %instance_name?', '', ''),
('suggest_instance', 'LFD: %instance_name.', 0, 0, '', 'Cherche à faire : %instance_name', '', '', '', 'Busco grupo para %instance_name.', '', ''),
('suggest_instance', 'Anyone needs %my_role for %instance_name?', 0, 0, '', 'Quelqu’un a besoin de %my_role pour %instance_name ?', '', '', '', '¿Alguien necesita un %my_role para %instance_name?', '', ''),
('suggest_instance', 'Missing %my_role for %instance_name?', 0, 0, '', '%my_role manquant pour %instance_name ?', '', '', '', '¿Buscando un %my_role para %instance_name?', '', ''),
('suggest_instance', 'Can be a %my_role for %instance_name.', 0, 0, '', 'Peut être un %my_role pour %instance_name', '', '', '', 'Puedo ser un %my_role para %instance_name.', '', ''),
('suggest_instance', 'Need help with %instance_name?', 0, 0, '', 'Besoin d’aide avec %instance_name ?', '', '', '', '¿Alguien necesita ayuda con %instance_name?', '', ''),
('suggest_instance', 'Need %my_role help with %instance_name?', 0, 0, '', 'Besoin d’aide %my_role avec %instance_name ?', '', '', '', '¿Necesitas ayuda de %my_role con %instance_name?', '', ''),
('suggest_instance', 'Anyone needs gear from %instance_name?', 0, 0, '', 'Quelqu’un a besoin d’équipement de %instance_name ?', '', '', '', '¿Alguien necesita equipo de %instance_name?', '', ''),
('suggest_instance', 'A little grind in %instance_name?', 0, 0, '', 'Un peu de grind dans %instance_name ?', '', '', '', '¿Un poco de farmeo en %instance_name?', '', ''),
('suggest_instance', 'WTR %instance_name', 0, 0, '', 'Cherche à rejoindre %instance_name', '', '', '', 'WTR %instance_name', '', ''),
('suggest_instance', 'Need help for %instance_name.', 0, 0, '', 'Besoin d’aide pour %instance_name', '', '', '', 'Necesito ayuda para %instance_name.', '', ''),
('suggest_instance', 'Wanna run %instance_name.', 0, 0, '', 'Je veux faire %instance_name', '', '', '', 'Quiero hacer %instance_name.', '', ''),
('suggest_instance', '%my_role looks for %instance_name.', 0, 0, '', '%my_role recherche %instance_name', '', '', '', '%my_role busca %instance_name.', '', ''),
('suggest_instance', 'What about %instance_name?', 0, 0, '', 'Et %instance_name ?', '', '', '', '¿Qué pasa con %instance_name?', '', ''),
('suggest_instance', 'Who wants to farm %instance_name?', 0, 0, '', 'Qui veut exploiter %instance_name ?', '', '', '', '¿Quién quiere farmear %instance_name?', '', ''),
('suggest_instance', 'Go in %instance_name?', 0, 0, '', 'Aller à %instance_name ?', '', '', '', '¿Vamos a %instance_name?', '', ''),
('suggest_instance', 'Looking for %instance_name.', 0, 0, '', 'À la recherche de %instance_name', '', '', '', 'Buscando gente para %instance_name.', '', ''),
('suggest_instance', 'Need help with %instance_name quests?', 0, 0, '', 'Besoin d’aide pour les quêtes %instance_name ?', '', '', '', '¿Necesitas ayuda con las misiones en %instance_name?', '', ''),
('suggest_instance', 'Wanna quest in %instance_name.', 0, 0, '', 'Je veux une quête dans %instance_name', '', '', '', 'Quiero hacer una vuelta rapida en %instance_name.', '', ''),
('suggest_instance', 'Anyone with quests in %instance_name?', 0, 0, '', 'Quelqu’un a des quêtes dans %instance_name ?', '', '', '', '¿Alguien con misiones en %instance_name?', '', ''),
('suggest_instance', 'Could help with quests in %instance_name.', 0, 0, '', 'Peux aider pour les quêtes dans %instance_name', '', '', '', 'Podría ayudar con las misiones en %instance_name.', '', ''),
('suggest_instance', '%my_role: any place in group for %instance_name?', 0, 0, '', '%my_role : n’importe quel rôle dans le groupe pour %instance_name ?', '', '', '', '%my_role: ¿algún lugar en el grupo para %instance_name?', '', ''),
('suggest_instance', 'Does anybody still run %instance_name this days?', 0, 0, '', 'Est-ce que quelqu’un fait encore %instance_name ces jours-ci ?', '', '', '', '¿Alguien todavía hace %instance_name estos días?', '', ''),
('suggest_instance', '%instance_name: anyone wants to take a %my_role?', 0, 0, '', '%instance_name : quelqu’un veut prendre un %my_role ?', '', '', '', '¿Alguien quiere un %my_role para %instance_name?', '', ''),
('suggest_instance', 'Is there any point being %my_role in %instance_name?', 0, 0, '', 'Y a-t-il un intérêt à être %my_role dans %instance_name ?', '', '', '', '¿Tiene algún sentido ser %my_role en %instance_name?', '', ''),
('suggest_instance', 'It is really worth to go to %instance_name?', 0, 0, '', 'Cela vaut-il vraiment la peine d’aller dans %instance_name ?', '', '', '', '¿Realmente vale la pena ir a %instance_name?', '', ''),
('suggest_instance', 'Anybody needs more people for %instance_name?', 0, 0, '', 'Quelqu’un a besoin de plus de personnes pour %instance_name ?', '', '', '', '¿Alguien necesita más personas para %instance_name?', '', ''),
('suggest_instance', '%instance_name bosses drop good gear. Wanna run?', 0, 0, '', 'Les boss %instance_name drop du bon équipement. Tu veux courir ?', '', '', '', 'Los jefes en %instance_name sueltan buen equipo. ¿Quieres ir?', '', ''),
('suggest_instance', 'What about %instance_name?', 0, 0, '', 'Et %instance_name ?', '', '', '', '¿Qué pasa con %instance_name?', '', ''),
('suggest_instance', 'Anybody needs %my_role?', 0, 0, '', 'Quelqu’un a besoin de %my_role ?', '', '', '', '¿Alguien necesita un %my_role?', '', ''),
('suggest_instance', 'Anyone needs %my_role?', 0, 0, '', 'Quelqu’un a besoin de %my_role ?', '', '', '', '¿Alguien hacer un grupo con un %my_role?', '', ''),
('suggest_instance', 'Who wants %instance_name?', 0, 0, '', 'Qui veut %instance_name ?', '', '', '', '¿Quién quiere %instance_name?', '', ''),
('suggest_instance', 'Can anybody summon me at %instance_name?', 0, 0, '', 'Quelqu’un peut-il me TP à %instance_name ?', '', '', '', '¿Alguien puede invocarme en %instance_name?', '', ''),
('suggest_instance', 'Meet me in %instance_name', 0, 0, '', 'Rencontrez-moi dans %instance_name', '', '', '', 'Encuéntrame en %instance_name', '', ''),
('suggest_instance', 'Wanna quick %instance_name run', 0, 0, '', 'Je veux faire rapidement %instance_name', '', '', '', 'Quiero ir a %instance_name rápido', '', ''),
('suggest_instance', 'Wanna full %instance_name run', 0, 0, '', 'Je veux faire %instance_name complètement', '', '', '', 'Quiero hacer %instance_name completa', '', ''),
('suggest_instance', 'How many times were you in %instance_name?', 0, 0, '', 'Combien de fois avez-vous été dans %instance_name ?', '', '', '', '¿Cuántas veces estuviste en %instance_name?', '', ''),
('suggest_instance', 'Another %instance_name run?', 0, 0, '', 'Une autre exécution de %instance_name ?', '', '', '', '¿Otra vuelta en %instance_name ?', '', ''),
('suggest_instance', 'Wiped in %instance_name? Take me instead!', 0, 0, '', 'Effacé dans %instance_name ? Prends-moi plutôt !', '', '', '', '¿Borrado en %instance_name? ¡Tómame a mí en su lugar!', '', ''),
('suggest_instance', 'Take me in %instance_name please.', 0, 0, '', 'Prenez-moi dans %instance_name s''il vous plaît.', '', '', '', 'Tómame en %instance_name por favor.', '', ''),
('suggest_instance', 'Quick %instance_name run?', 0, 0, '', 'Faire rapidement %instance_name ?', '', '', '', '¿Partida rápida de %instance_name?', '', ''),
('suggest_instance', 'Full %instance_name run?', 0, 0, '', 'Clean complet dans %instance_name ?', '', '', '', '¿Partida completa de %instance_name?', '', ''),
('suggest_instance', 'Who can take %my_role to %instance_name?', 0, 0, '', 'Qui peut prendre %my_role à %instance_name ?', '', '', '', '¿Quién puede ir de %my_role para %instance_name?', '', ''),
('suggest_instance', 'LFG %instance_name, I am %my_role', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_instance', '%my_role LFG %instance_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_instance', 'Anyone looking for a group for %instance_name?', 0, 0, '', 'Quelqu’un cherche un groupe pour %instance_name ?', '', '', '', '¿Alguien busca grupo para %instance_name?', '', ''),
('suggest_instance', 'Need a DPS for %instance_name', 0, 0, '', 'Besoin d’un DPS pour %instance_name', '', '', '', 'Necesito un DPS para %instance_name', '', ''),
('suggest_instance', 'Looking for a healer for %instance_name', 0, 0, '', 'Cherche un soigneur pour %instance_name', '', '', '', 'Busco un sanador para %instance_name', '', ''),
('suggest_instance', 'Anyone need a tank for %instance_name?', 0, 0, '', 'Quelqu’un a besoin d’un tank pour %instance_name ?', '', '', '', '¿Alguien necesita un tanque para %instance_name?', '', ''),
('suggest_instance', 'Trying to do %instance_name. Need a group!', 0, 0, '', 'Essayer de faire %instance_name. Besoin d’un groupe !', '', '', '', 'Intentando hacer %instance_name. ¡Necesito un grupo!', '', ''),
('suggest_instance', 'Anyone down for %instance_name?', 0, 0, '', 'Quelqu’un part pour %instance_name ?', '', '', '', '¿Alguien se apunta para %instance_name?', '', ''),
('suggest_instance', 'Looking for 1 more for %instance_name', 0, 0, '', 'Cherche 1 de plus pour %instance_name', '', '', '', 'Busco 1 más para %instance_name', '', ''),
('suggest_instance', 'Anyone wanna join for %instance_name?', 0, 0, '', 'Quelqu’un veut rejoindre pour %instance_name ?', '', '', '', '¿Alguien quiere unirse para %instance_name?', '', ''),
('suggest_instance', 'Anyone interested in %instance_name?', 0, 0, '', 'Quelqu’un intéressé par %instance_name ?', '', '', '', '¿Alguien interesado en %instance_name?', '', ''),
('suggest_instance', 'Looking for a quick run through %instance_name', 0, 0, '', 'Cherche une course rapide dans %instance_name', '', '', '', 'Buscando una corrida rápida en %instance_name', '', ''),
('suggest_instance', 'Who’s up for %instance_name?', 0, 0, '', 'Qui est partant pour %instance_name ?', '', '', '', '¿Quién está listo para %instance_name?', '', ''),
('suggest_instance', 'Need help to clear %instance_name', 0, 0, '', 'Besoin d’aide pour nettoyer %instance_name', '', '', '', 'Necesito ayuda para limpiar %instance_name', '', ''),
('suggest_instance', 'Want to farm %instance_name?', 0, 0, '', 'Envie de farmer %instance_name ?', '', '', '', '¿Quieres farmear %instance_name?', '', ''),
('suggest_instance', 'Looking to complete %instance_name', 0, 0, '', 'Cherche à compléter %instance_name', '', '', '', 'Buscando completar %instance_name', '', ''),
('suggest_instance', 'Anyone needs %instance_name items?', 0, 0, '', 'Quelqu’un a besoin d’objets de %instance_name ?', '', '', '', '¿Alguien necesita objetos de %instance_name?', '', ''),
('suggest_instance', 'Looking for a carry through %instance_name', 0, 0, '', 'Cherche un carry pour %instance_name', '', '', '', 'Buscando un carry para %instance_name', '', ''),
('suggest_instance', 'Anyone want to run %instance_name?', 0, 0, '', 'Quelqu’un veut faire %instance_name ?', '', '', '', '¿Alguien quiere hacer %instance_name?', '', ''),
('suggest_instance', 'Running %instance_name, looking for more', 0, 0, '', 'En train de faire %instance_name, cherche des membres', '', '', '', 'Haciendo %instance_name, buscando más personas', '', ''),
('suggest_instance', 'Anyone available for %instance_name?', 0, 0, '', 'Quelqu’un disponible pour %instance_name ?', '', '', '', '¿Alguien disponible para %instance_name?', '', ''),
('suggest_instance', 'Looking for someone to help with %instance_name', 0, 0, '', 'Cherche quelqu’un pour aider avec %instance_name', '', '', '', 'Buscando a alguien para ayudar con %instance_name', '', ''),
('suggest_instance', 'Need %my_role for %instance_name!', 0, 0, '', 'Besoin de %my_role pour %instance_name !', '', '', '', '¡Necesito un %my_role para %instance_name!', '', ''),
('suggest_instance', 'Want to do a %instance_name run?', 0, 0, '', 'Envie de faire %instance_name ?', '', '', '', '¿Quieres hacer una corrida de %instance_name?', '', ''),
('suggest_instance', 'Need %my_role for %instance_name run', 0, 0, '', 'Besoin de %my_role pour la course de %instance_name', '', '', '', 'Necesito %my_role para la corrida de %instance_name', '', ''),
('suggest_instance', 'Anyone want to farm %instance_name gear?', 0, 0, '', 'Quelqu’un veut farmer du matériel de %instance_name ?', '', '', '', '¿Alguien quiere farmear equipo de %instance_name?', '', ''),
('suggest_instance', 'Looking for a group to speedrun %instance_name', 0, 0, '', 'Cherche un groupe pour faire %instance_name en speedrun', '', '', '', 'Buscando un grupo para speedrun de %instance_name', '', ''),
('suggest_instance', 'Need a group to clear %instance_name', 0, 0, '', 'Besoin d’un groupe pour nettoyer %instance_name', '', '', '', 'Necesito un grupo para limpiar %instance_name', '', ''),
('suggest_instance', 'Anyone interested in farming %instance_name?', 0, 0, '', 'Quelqu’un intéressé par le farming de %instance_name ?', '', '', '', '¿Alguien interesado en farmear %instance_name?', '', ''),
('suggest_instance', 'Looking for some gear from %instance_name', 0, 0, '', 'Cherche du matériel de %instance_name', '', '', '', 'Buscando equipo de %instance_name', '', ''),
('suggest_instance', 'Anyone for a quick %instance_name run?', 0, 0, '', 'Quelqu’un pour une course rapide dans %instance_name ?', '', '', '', '¿Alguien para una corrida rápida de %instance_name?', '', ''),
('suggest_instance', 'Anyone want to try %instance_name together?', 0, 0, '', 'Quelqu’un veut essayer %instance_name ensemble ?', '', '', '', '¿Alguien quiere intentar %instance_name juntos?', '', ''),
('suggest_instance', 'Looking for people to complete %instance_name', 0, 0, '', 'Cherche des gens pour compléter %instance_name', '', '', '', 'Buscando personas para completar %instance_name', '', ''),
('suggest_instance', 'Can we get a group for %instance_name?', 0, 0, '', 'Peut-on avoir un groupe pour %instance_name ?', '', '', '', '¿Podemos formar un grupo para %instance_name?', '', ''),
('suggest_instance', 'Need a quick run for %instance_name', 0, 0, '', 'Besoin d’une course rapide pour %instance_name', '', '', '', 'Necesito una corrida rápida para %instance_name', '', ''),
('suggest_instance', 'Anyone down to farm %instance_name together?', 0, 0, '', 'Quelqu’un partant pour farmer %instance_name ensemble ?', '', '', '', '¿Alguien quiere farmear %instance_name juntos?', '', ''),
('suggest_instance', 'Looking for a few more for %instance_name', 0, 0, '', 'Cherche quelques personnes pour %instance_name', '', '', '', 'Buscando algunos más para %instance_name', '', ''),
('suggest_instance', 'Anyone need help with %instance_name mechanics?', 0, 0, '', 'Quelqu’un a besoin d’aide avec les mécaniques de %instance_name ?', '', '', '', '¿Alguien necesita ayuda con las mecánicas de %instance_name?', '', ''),
('suggest_instance', 'Need a group for %instance_name fast', 0, 0, '', 'Besoin d’un groupe pour %instance_name rapidement', '', '', '', 'Necesito un grupo rápido para %instance_name', '', ''),
('suggest_instance', 'Anyone want to speedrun %instance_name?', 0, 0, '', 'Envie de faire %instance_name en speedrun ?', '', '', '', '¿Alguien quiere hacer %instance_name en speedrun?', '', ''),
('suggest_instance', 'Looking for a quick %instance_name completion', 0, 0, '', 'Cherche à compléter %instance_name rapidement', '', '', '', 'Buscando completar %instance_name rápido', '', ''),
('suggest_instance', 'Looking for anyone to join %instance_name', 0, 0, '', 'Cherche quelqu’un pour rejoindre %instance_name', '', '', '', 'Buscando a alguien para unirse a %instance_name', '', ''),
('suggest_instance', 'Anyone down to clear %instance_name?', 0, 0, '', 'Quelqu’un pour nettoyer %instance_name ?', '', '', '', '¿Alguien para limpiar %instance_name?', '', ''),
('suggest_instance', 'Need help with the final boss of %instance_name', 0, 0, '', 'Besoin d’aide avec le dernier boss de %instance_name', '', '', '', 'Necesito ayuda con el último jefe de %instance_name', '', ''),
('suggest_instance', 'Anyone interested in running %instance_name today?', 0, 0, '', 'Quelqu’un intéressé par faire %instance_name aujourd’hui ?', '', '', '', '¿Alguien interesado en hacer %instance_name hoy?', '', ''),
('suggest_instance', 'Looking for a quick run of %instance_name to farm some gear', 0, 0, '', 'Cherche une course rapide de %instance_name pour farmer du matériel', '', '', '', 'Buscando una corrida rápida de %instance_name para farmear equipo', '', ''),
('suggest_instance', 'Anyone looking to farm materials from %instance_name?', 0, 0, '', 'Quelqu’un cherche à farmer des matériaux de %instance_name ?', '', '', '', '¿Alguien quiere farmear materiales de %instance_name?', '', ''),
('suggest_instance', 'Let’s get a group for %instance_name', 0, 0, '', 'Formons un groupe pour %instance_name', '', '', '', 'Formemos un grupo para %instance_name', '', ''),


-- random quest
-- usable placeholders:
-- %my_role
-- %quest_link
-- %quest_level
-- %my_class
-- %my_race
-- %my_level
('suggest_quest', 'Need help with %quest_link?', 0, 0, '', 'Besoin d’aide avec %quest_link ?', '', '', '', '¿Alguien necesita ayuda con %quest_link?', '', ''),
('suggest_quest', 'Anyone wants to share %quest_link?', 0, 0, '', 'Quelqu’un veut partager %quest_link ?', '', '', '', '¿Alguien quiere hacer %quest_link?', '', ''),
('suggest_quest', 'Anyone doing %quest_link?', 0, 0, '', 'Quelqu’un fait %quest_link ?', '', '', '', '¿Alguien está haciendo %quest_link?', '', ''),
('suggest_quest', 'Wanna do %quest_link.', 0, 0, '', 'Je veux faire %quest_link', '', '', '', '¿Una ayudita con %quest_link?', '', ''),
('suggest_quest', 'Looking to group for %quest_link.', 0, 0, '', 'Cherche un groupe pour %quest_link.', '', '', '', 'Buscando un grupo para %quest_link.', '', ''),
('suggest_quest', 'Who wants to help with %quest_link?', 0, 0, '', 'Qui veut aider avec %quest_link ?', '', '', '', '¿Quién quiere ayudar con %quest_link?', '', ''),
('suggest_quest', 'Need assistance with %quest_link?', 0, 0, '', 'Besoin d’aide pour %quest_link ?', '', '', '', '¿Alguien necesita ayuda con %quest_link?', '', ''),
('suggest_quest', 'Anyone interested in %quest_link?', 0, 0, '', 'Quelqu’un intéressé par %quest_link ?', '', '', '', '¿Alguien interesado en %quest_link?', '', ''),
('suggest_quest', 'Anyone wants to finish %quest_link?', 0, 0, '', 'Quelqu’un veut finir %quest_link ?', '', '', '', '¿Alguien quiere terminar %quest_link?', '', ''),
('suggest_quest', 'Need a team for %quest_link?', 0, 0, '', 'Besoin d’un groupe pour %quest_link ?', '', '', '', '¿Necesito un equipo para %quest_link?', '', ''),
('suggest_quest', 'Any takers for %quest_link?', 0, 0, '', 'Des volontaires pour %quest_link ?', '', '', '', '¿Alguien se apunta a %quest_link?', '', ''),
('suggest_quest', 'Let’s do %quest_link together.', 0, 0, '', 'Faisons %quest_link ensemble.', '', '', '', 'Vamos a hacer %quest_link juntos.', '', ''),
('suggest_quest', 'Need someone for %quest_link.', 0, 0, '', 'Besoin de quelqu’un pour %quest_link.', '', '', '', '¿Necesito a alguien para %quest_link?', '', ''),
('suggest_quest', 'Anyone needs help with %quest_link?', 0, 0, '', 'Quelqu’un a besoin d’aide pour %quest_link ?', '', '', '', '¿Alguien necesita ayuda con %quest_link?', '', ''),
('suggest_quest', 'Need help with %quest_link? I got you.', 0, 0, '', 'Besoin d’aide avec %quest_link ? Je suis là.', '', '', '', '¿Necesitas ayuda con %quest_link? Yo te ayudo.', '', ''),
('suggest_quest', 'Looking to do %quest_link. Anyone in?', 0, 0, '', 'Je cherche à faire %quest_link. Qui est partant ?', '', '', '', 'Busco hacer %quest_link. ¿Alguien se une?', '', ''),
('suggest_quest', 'Help wanted for %quest_link.', 0, 0, '', 'Aide recherchée pour %quest_link.', '', '', '', 'Se busca ayuda para %quest_link.', '', ''),
('suggest_quest', 'Who is up for %quest_link?', 0, 0, '', 'Qui est partant pour %quest_link ?', '', '', '', '¿Quién se apunta a %quest_link?', '', ''),
('suggest_quest', 'Who wants to join for %quest_link?', 0, 0, '', 'Qui veut rejoindre pour %quest_link ?', '', '', '', '¿Quién quiere unirse a %quest_link?', '', ''),
('suggest_quest', 'Anyone up for %quest_link?', 0, 0, '', 'Quelqu’un partant pour %quest_link ?', '', '', '', '¿Alguien quiere hacer %quest_link?', '', ''),
('suggest_quest', 'Need some help with %quest_link here.', 0, 0, '', 'Besoin d’aide pour %quest_link ici.', '', '', '', 'Necesito ayuda con %quest_link.', '', ''),
('suggest_quest', 'Anyone for %quest_link?', 0, 0, '', 'Quelqu’un pour %quest_link ?', '', '', '', '¿Alguien para %quest_link?', '', ''),
('suggest_quest', 'Who is ready to do %quest_link?', 0, 0, '', 'Qui est prêt à faire %quest_link ?', '', '', '', '¿Quién está listo para %quest_link?', '', ''),
('suggest_quest', 'Looking for some %quest_link help.', 0, 0, '', 'Je cherche un peu d’aide pour %quest_link.', '', '', '', 'Buscando ayuda para %quest_link.', '', ''),
('suggest_quest', 'Anyone need %quest_link help?', 0, 0, '', 'Quelqu’un a besoin d’aide pour %quest_link ?', '', '', '', '¿Alguien necesita ayuda con %quest_link?', '', ''),
('suggest_quest', 'Who is down to complete %quest_link?', 0, 0, '', 'Qui est partant pour terminer %quest_link ?', '', '', '', '¿Quién se apunta a completar %quest_link?', '', ''),
('suggest_quest', 'Anyone need assistance for %quest_link?', 0, 0, '', 'Besoin d’assistance pour %quest_link ?', '', '', '', '¿Alguien necesita asistencia con %quest_link?', '', ''),
('suggest_quest', 'Who can help with %quest_link?', 0, 0, '', 'Qui peut m’aider avec %quest_link ?', '', '', '', '¿Quién puede ayudar con %quest_link?', '', ''),
('suggest_quest', 'Looking for a hand with %quest_link.', 0, 0, '', 'Je cherche une main pour %quest_link.', '', '', '', 'Buscando ayuda con %quest_link.', '', ''),
('suggest_quest', 'Anyone ready for %quest_link?', 0, 0, '', 'Quelqu’un est prêt pour %quest_link ?', '', '', '', '¿Alguien listo para %quest_link?', '', ''),
('suggest_quest', 'Can anyone join me for %quest_link?', 0, 0, '', 'Quelqu’un peut-il me rejoindre pour %quest_link ?', '', '', '', '¿Alguien puede unirse para %quest_link?', '', ''),
('suggest_quest', 'Looking for some partners for %quest_link.', 0, 0, '', 'Je cherche des partenaires pour %quest_link.', '', '', '', 'Buscando compañeros para %quest_link.', '', ''),
('suggest_quest', 'Anyone want to team up for %quest_link?', 0, 0, '', 'Qui veut se regrouper pour %quest_link ?', '', '', '', '¿Alguien quiere formar un equipo para %quest_link?', '', ''),
('suggest_quest', 'Who is willing to do %quest_link?', 0, 0, '', 'Qui est prêt à faire %quest_link ?', '', '', '', '¿Quién está dispuesto a hacer %quest_link?', '', ''),
('suggest_quest', 'Who needs %quest_link to be completed?', 0, 0, '', 'Qui a besoin de terminer %quest_link ?', '', '', '', '¿Quién necesita completar %quest_link?', '', ''),
('suggest_quest', 'Looking to complete %quest_link.', 0, 0, '', 'Cherche à compléter %quest_link.', '', '', '', 'Buscando completar %quest_link.', '', ''),
('suggest_quest', 'Who can join me for %quest_link?', 0, 0, '', 'Qui peut me rejoindre pour %quest_link ?', '', '', '', '¿Quién puede unirse a %quest_link?', '', ''),
('suggest_quest', 'Anyone interested in completing %quest_link?', 0, 0, '', 'Quelqu’un intéressé à compléter %quest_link ?', '', '', '', '¿Alguien interesado en completar %quest_link?', '', ''),
('suggest_quest', 'Wanna help with %quest_link?', 0, 0, '', 'Tu veux aider avec %quest_link ?', '', '', '', '¿Quieres ayudar con %quest_link?', '', ''),
('suggest_quest', 'Anyone willing to help with %quest_link?', 0, 0, '', 'Quelqu’un prêt à aider avec %quest_link ?', '', '', '', '¿Alguien dispuesto a ayudar con %quest_link?', '', ''),
('suggest_quest', 'Need someone to finish %quest_link.', 0, 0, '', 'Besoin de quelqu’un pour finir %quest_link.', '', '', '', 'Necesito a alguien para terminar %quest_link.', '', ''),
('suggest_quest', 'Anyone want to finish %quest_link?', 0, 0, '', 'Quelqu’un veut finir %quest_link ?', '', '', '', '¿Alguien quiere terminar %quest_link?', '', ''),
('suggest_quest', 'Any help with %quest_link?', 0, 0, '', 'Besoin d’aide avec %quest_link ?', '', '', '', '¿Necesito ayuda con %quest_link?', '', ''),
('suggest_quest', 'Anyone up for %quest_link?', 0, 0, '', 'Quelqu’un partant pour %quest_link ?', '', '', '', '¿Alguien se apunta a %quest_link?', '', ''),
('suggest_quest', 'Looking for %quest_link completion partner.', 0, 0, '', 'Cherche un partenaire pour compléter %quest_link.', '', '', '', 'Buscando compañero para completar %quest_link.', '', ''),
('suggest_quest', 'Who is down to finish %quest_link?', 0, 0, '', 'Qui est partant pour finir %quest_link ?', '', '', '', '¿Quién se apunta a terminar %quest_link?', '', ''),
('suggest_quest', 'Help wanted for %quest_link.', 0, 0, '', 'Aide recherchée pour %quest_link.', '', '', '', 'Se busca ayuda para %quest_link.', '', ''),
('suggest_quest', 'Anyone interested in %quest_link today?', 0, 0, '', 'Quelqu’un intéressé par %quest_link aujourd’hui ?', '', '', '', '¿Alguien interesado en %quest_link hoy?', '', ''),
('suggest_quest', 'Anyone doing %quest_link now?', 0, 0, '', 'Quelqu’un fait %quest_link maintenant ?', '', '', '', '¿Alguien haciendo %quest_link ahora?', '', ''),
('suggest_quest', 'Any takers for %quest_link now?', 0, 0, '', 'Des volontaires pour %quest_link maintenant ?', '', '', '', '¿Alguien se apunta a %quest_link ahora?', '', ''),
('suggest_quest', 'Anyone wants to start %quest_link?', 0, 0, '', 'Quelqu’un veut commencer %quest_link ?', '', '', '', '¿Alguien quiere empezar %quest_link?', '', ''),
('suggest_quest', 'Need help with %quest_link please.', 0, 0, '', 'Besoin d’aide avec %quest_link s''il vous plaît.', '', '', '', 'Necesito ayuda con %quest_link, por favor.', '', ''),
('suggest_quest', 'Who can help me with %quest_link?', 0, 0, '', 'Qui peut m’aider avec %quest_link ?', '', '', '', '¿Quién puede ayudarme con %quest_link?', '', ''),
('suggest_quest', 'Looking to group for %quest_link.', 0, 0, '', 'Cherche un groupe pour %quest_link.', '', '', '', 'Buscando un grupo para %quest_link.', '', ''),


-- random trade?
-- usable placeholders:
-- %my_role
-- %category
-- %my_class
-- %my_race
-- %my_level
('suggest_trade', 'Anyone to farm %category?', 0, 0, '', 'Quelqu’un pour farm %category ?', '', '', '', '¿Alguien para farmear %category?', '', ''),
('suggest_trade', 'Looking for help farming %category.', 0, 0, '', 'Vous cherchez de l’aide pour farmer %category', '', '', '', 'Buscando ayuda para farmear %category.', '', ''),
('suggest_trade', 'Damn %category are so expensive!', 0, 0, '', 'Putain %category sont si chers!', '', '', '', '¡Malditas %category son tan caras!', '', ''),
('suggest_trade', 'Wanna %category.', 0, 0, '', 'Je veux %category', '', '', '', 'Quiero %category.', '', ''),
('suggest_trade', 'Need help with %category.', 0, 0, '', 'Besoin d’aide avec %category', '', '', '', 'Necesito ayuda con %category.', '', ''),
('suggest_trade', 'WTB %category.', 0, 0, '', 'Cherche à acheter %category', '', '', '', 'Quiero comprar %category.', '', ''),
('suggest_trade', 'Anyone interested in %category?', 0, 0, '', 'Quiconque est intéressé par %category?', '', '', '', '¿Alguien interesado en %category?', '', ''),
('suggest_trade', 'WTS %category.', 0, 0, '', 'Cherche à vendre %category', '', '', '', 'Quiero vender %category.', '', ''),
('suggest_trade', 'I am selling %category cheaper than AH.', 0, 0, '', 'Je vends %category moins cher que l’hôtel des ventes.', '', '', '', 'Estoy vendiendo %category más barato que en las subastas.', '', ''),
('suggest_trade', 'Who wants to farm %category?', 0, 0, '', 'Qui veut farmer %category ?', '', '', '', '¿Quién quiere farmear %category?', '', ''),
('suggest_trade', 'Wanna farm %category.', 0, 0, '', 'Je veux farmer %category', '', '', '', 'Quiero farmear %category.', '', ''),
('suggest_trade', 'Looking for party after %category.', 0, 0, '', 'Recherche de partie après %category', '', '', '', 'Buscando grupo después de %category.', '', ''),
('suggest_trade', 'Any %category are appreciated.', 0, 0, '', 'Toute %category est appréciée.', '', '', '', 'Cualquier %category es apreciada.', '', ''),
('suggest_trade', 'Buying anything of %category.', 0, 0, '', 'Acheter quoi que ce soit de %category', '', '', '', 'Comprando algo de %category.', '', ''),
('suggest_trade', 'Wow, anybody is farming %category!', 0, 0, '', 'Wow, quelqu’un farm %category !', '', '', '', '¡Guau, alguien está farmeando %category!', '', ''),
('suggest_trade', '%category are selling mad in the AH.', 0, 0, '', '%category se vendent follement dans l’hôtel des ventes.', '', '', '', '%category están vendiendo locos en las subastas.', '', ''),
('suggest_trade', 'AH is hot for %category.', 0, 0, '', 'Hôtel des ventes est chaud pour %category', '', '', '', 'Las subastas estan caliente para %category.', '', ''),
('suggest_trade', '%category are on the market.', 0, 0, '', '%category sont sur le marché.', '', '', '', '%category están en el mercado.', '', ''),
('suggest_trade', 'Wanna trade some %category.', 0, 0, '', 'Je veux échanger une %category', '', '', '', 'Quiero intercambiar alguna %category.', '', ''),
('suggest_trade', 'Need more %category.', 0, 0, '', 'Besoin de plus de %category', '', '', '', 'Necesito más %category.', '', ''),
('suggest_trade', 'Anybody can spare some %category?', 0, 0, '', 'Quelqu’un peut-il épargner une %category ?', '', '', '', '¿Alguien puede ahorrar algo de %category?', '', ''),
('suggest_trade', 'Who wants %category?', 0, 0, '', 'Qui veut %category ?', '', '', '', '¿Quién quiere %category?', '', ''),
('suggest_trade', 'Some %category please?', 0, 0, '', 'Une %category s''il vous plaît ?', '', '', '', '¿Alguna %category, por favor?', '', ''),
('suggest_trade', 'I should have got skill for %category.', 0, 0, '', 'j’aurais dû avoir une compétence pour %category', '', '', '', 'Debería haber adquirido habilidad para %category.', '', ''),
('suggest_trade', 'I am dying for %category.', 0, 0, '', 'Je meurs d’envie pour %category', '', '', '', 'Me muero por %category.', '', ''),
('suggest_trade', 'People are killing for %category.', 0, 0, '', 'Les gens tuent pour %category', '', '', '', 'La gente está matando por %category.', '', ''),
('suggest_trade', '%category is a great bargain!', 0, 0, '', '%category est une bonne affaire!', '', '', '', '%category es una gran ganga!', '', ''),
('suggest_trade', 'Everybody is mad for %category!', 0, 0, '', 'Tout le monde est fou de %category !', '', '', '', '¡Todo el mundo está loco por %category!', '', ''),
('suggest_trade', 'Where is the best place to farm for %category?', 0, 0, '', 'Quel est le meilleur endroit pour farmer pour %category ?', '', '', '', '¿Dónde está el mejor lugar para farmear para %category?', '', ''),
('suggest_trade', 'I am all set for %category.', 0, 0, '', 'Je suis prêt pour %category', '', '', '', 'Estoy listo para %category.', '', ''),
('suggest_trade', 'Is it good to sell %category?', 0, 0, '', 'Est-il bon de vendre %category ?', '', '', '', '¿Es bueno vender %category?', '', ''),
('suggest_trade', 'I’d probably keep all my %category with me.', 0, 0, '', 'Je garderais probablement toute ma %category avec moi.', '', '', '', 'Probablemente mantendría toda mi %category conmigo', '', ''),
('suggest_trade', 'Need group? Maybe to farm some %category?', 0, 0, '', 'Besoin de groupe ? Peut-être pour farmer une certaine %category ?', '', '', '', '¿Necesitas un grupo? ¿Tal vez para farmear alguna %category?', '', ''),
('suggest_trade', 'I am still thinking about %category.', 0, 0, '', 'Je pense toujours à %category', '', '', '', 'Todavía estoy pensando en %category.', '', ''),
('suggest_trade', 'I heard about %category already, but my pockets are empty.', 0, 0, '', 'j’ai déjà entendu parler de %category , mais mes poches sont vides.', '', '', '', 'Ya escuché sobre %category, pero mis bolsillos están vacíos.', '', ''),
('suggest_trade', 'LFG for %category', 0, 0, '', 'Cherche un groupe pour %category', '', '', '', 'Busco grupo para %category', '', ''),
('suggest_trade', 'Would selling %category make me rich?', 0, 0, '', 'Vendre %category me rendrait-il riche?', '', '', '', '¿Vender %category me haría rico?', '', ''),
('suggest_trade', 'OK. I an farming %category tomorrow.', 0, 0, '', 'd’ACCORD. Je farm %category demain.', '', '', '', 'OK. Tengo una sesion de farmeo de %category mañana.', '', ''),
('suggest_trade', 'Everyone is talking about %category.', 0, 0, '', 'Tout le monde parle de %category', '', '', '', 'Todos hablan de %category', '', ''),
('suggest_trade', 'I saw at least ten people farming for %category.', 0, 0, '', 'j’ai vu au moins dix personnes farmer pour %category', '', '', '', 'Vi al menos diez personas farmeando para %category.', '', ''),
('suggest_trade', 'I sold all my %category yesterday. I am completely broke!', 0, 0, '', 'j’ai vendu tout mon %category hier. Je suis complètement fauché!', '', '', '', 'Ayer vendí todo mi %category. ¡Estoy completamente arruinado!', '', ''),
('suggest_trade', 'Wanna join a guild farming for %category.', 0, 0, '', 'Je veux rejoindre une guilde farmant pour %category', '', '', '', 'Quiero unirme a una hermandad de farmeo para %category.', '', ''),
('suggest_trade', 'I am selling %category cheaper than AH.', 0, 0, '', 'Je vends %category moins cher que l’hôtel des ventes.', '', '', '', 'Estoy vendiendo %category más barato que en las subastas.', '', ''),
('suggest_trade', 'Who wants to farm %category?', 0, 0, '', 'Qui veut farmer %category ?', '', '', '', '¿Quién quiere farmear %category?', '', ''),
('suggest_trade', 'Selling %category, anyone interested?', 0, 0, '', 'Je vends %category, quelqu’un intéressé ?', '', '', '', 'Vendo %category, ¿alguien interesado?', '', ''),
('suggest_trade', 'Anyone needs %category?', 0, 0, '', 'Quelqu’un a besoin de %category ?', '', '', '', '¿Alguien necesita %category?', '', ''),
('suggest_trade', 'Selling %category, whisper me.', 0, 0, '', 'Je vends %category, chuchotez-moi.', '', '', '', 'Vendo %category, susúrrame.', '', ''),
('suggest_trade', 'Looking to trade %category for %category.', 0, 0, '', 'Cherche à échanger %category contre %category.', '', '', '', 'Busco intercambiar %category por %category.', '', ''),
('suggest_trade', 'Need %category, anyone selling?', 0, 0, '', 'Besoin de %category, quelqu’un vend ?', '', '', '', 'Necesito %category, ¿alguien vende?', '', ''),
('suggest_trade', 'Got %category for sale, hit me up!', 0, 0, '', 'j’ai %category à vendre, contactez-moi!', '', '', '', '¡Tengo %category en venta, contáctame!', '', ''),
('suggest_trade', 'Anyone willing to farm %category?', 0, 0, '', 'Quelqu’un prêt à farmer %category ?', '', '', '', '¿Alguien dispuesto a farmear %category?', '', ''),
('suggest_trade', 'Want to trade %category for %category.', 0, 0, '', 'Je veux échanger %category contre %category.', '', '', '', 'Quiero intercambiar %category por %category.', '', ''),
('suggest_trade', 'Selling %category, best price in the market.', 0, 0, '', 'Je vends %category, meilleur prix du marché.', '', '', '', 'Vendo %category, el mejor precio del mercado.', '', ''),
('suggest_trade', 'Looking for %category in exchange for %category.', 0, 0, '', 'Je cherche %category en échange de %category.', '', '', '', 'Busco %category a cambio de %category.', '', ''),
('suggest_trade', 'Anyone got %category to trade?', 0, 0, '', 'Quelqu’un a %category à échanger ?', '', '', '', '¿Alguien tiene %category para intercambiar?', '', ''),
('suggest_trade', 'Who needs %category? I got some for sale!', 0, 0, '', 'Qui a besoin de %category ? j’en ai à vendre !', '', '', '', '¿Quién necesita %category? ¡Tengo algo para vender!', '', ''),
('suggest_trade', 'I have %category for sale, whisper for price.', 0, 0, '', 'j’ai %category à vendre, chuchotez-moi pour le prix.', '', '', '', 'Tengo %category en venta, susúrrame para el precio.', '', ''),
('suggest_trade', 'Looking to sell %category, whisper for details.', 0, 0, '', 'Je cherche à vendre %category, chuchotez-moi pour les détails.', '', '', '', 'Busco vender %category, susúrrame para detalles.', '', ''),
('suggest_trade', 'WTB %category. Anyone selling?', 0, 0, '', 'Cherche à acheter %category. Quelqu’un vend ?', '', '', '', 'Busco comprar %category. ¿Alguien vende?', '', ''),
('suggest_trade', 'I am looking for %category, please contact me.', 0, 0, '', 'Je cherche %category, contactez-moi.', '', '', '', 'Estoy buscando %category, por favor contáctame.', '', ''),
('suggest_trade', 'Any takers for %category?', 0, 0, '', 'Des preneurs pour %category ?', '', '', '', '¿Alguien interesado en %category?', '', ''),
('suggest_trade', 'Can anyone help farm %category?', 0, 0, '', 'Quelqu’un peut m’aider à farmer %category ?', '', '', '', '¿Alguien puede ayudar a farmear %category?', '', ''),
('suggest_trade', 'Anyone have %category to sell?', 0, 0, '', 'Quelqu’un a %category à vendre ?', '', '', '', '¿Alguien tiene %category para vender?', '', ''),
('suggest_trade', 'Looking to buy %category, please message me.', 0, 0, '', 'Je cherche à acheter %category, envoyez-moi un message.', '', '', '', 'Busco comprar %category, por favor mándame un mensaje.', '', ''),
('suggest_trade', 'I am looking for %category. Who has it?', 0, 0, '', 'Je cherche %category. Qui l’a ?', '', '', '', 'Estoy buscando %category. ¿Quién lo tiene?', '', ''),
('suggest_trade', 'WTB %category at a fair price.', 0, 0, '', 'Cherche à acheter %category à un prix raisonnable.', '', '', '', 'Busco comprar %category a un precio justo.', '', ''),
('suggest_trade', 'Need %category urgently, anyone got it?', 0, 0, '', 'Besoin de %category d’urgence, quelqu’un l’a ?', '', '', '', 'Necesito %category urgente, ¿alguien lo tiene?', '', ''),
('suggest_trade', 'Got %category, selling at a low price.', 0, 0, '', 'j’ai %category, je vends à bas prix.', '', '', '', 'Tengo %category, vendo a un precio bajo.', '', ''),
('suggest_trade', 'Selling %category for a good deal.', 0, 0, '', 'Je vends %category à un bon prix.', '', '', '', 'Vendo %category a buen precio.', '', ''),
('suggest_trade', 'I’m looking to buy %category, anyone selling cheap?', 0, 0, '', 'Je cherche à acheter %category, quelqu’un le vend pas cher ?', '', '', '', 'Busco comprar %category, ¿alguien lo vende barato?', '', ''),
('suggest_trade', 'Anyone selling %category? I’m interested.', 0, 0, '', 'Quelqu’un vend %category ? Je suis intéressé.', '', '', '', '¿Alguien vende %category? Estoy interesado.', '', ''),
('suggest_trade', 'Need %category, please whisper if selling.', 0, 0, '', 'Besoin de %category, chuchotez-moi si vous vendez.', '', '', '', 'Necesito %category, susúrrame si lo vendes.', '', ''),
('suggest_trade', 'Anyone got extra %category for trade?', 0, 0, '', 'Quelqu’un a des %category supplémentaires à échanger ?', '', '', '', '¿Alguien tiene %category extra para intercambiar?', '', ''),
('suggest_trade', 'Looking for %category. Anyone selling?', 0, 0, '', 'Cherche %category. Quelqu’un vend ?', '', '', '', 'Busco %category. ¿Alguien lo vende?', '', ''),
('suggest_trade', 'Anyone interested in trading %category?', 0, 0, '', 'Quelqu’un intéressé par un échange de %category ?', '', '', '', '¿Alguien interesado en intercambiar %category?', '', ''),
('suggest_trade', 'Selling %category, hit me up for a trade.', 0, 0, '', 'Je vends %category, contactez-moi pour un échange.', '', '', '', 'Vendo %category, contáctame para un intercambio.', '', ''),
('suggest_trade', 'WTB %category, looking for good deals.', 0, 0, '', 'Cherche à acheter %category, recherche de bonnes affaires.', '', '', '', 'Busco comprar %category, buscando buenas ofertas.', '', ''),
('suggest_trade', 'Who needs %category? I have extras.', 0, 0, '', 'Qui a besoin de %category ? j’en ai en trop.', '', '', '', '¿Quién necesita %category? Tengo extras.', '', ''),
('suggest_trade', 'Anyone wants %category? Selling for cheap.', 0, 0, '', 'Quelqu’un veut %category ? Je le vends pas cher.', '', '', '', '¿Alguien quiere %category? Lo vendo barato.', '', ''),
('suggest_trade', 'Looking for %category, anyone selling?', 0, 0, '', 'Je cherche %category, quelqu’un le vend ?', '', '', '', 'Busco %category, ¿alguien lo vende?', '', ''),
('suggest_trade', 'Anyone need %category? I have some for trade.', 0, 0, '', 'Quelqu’un a besoin de %category ? j’en ai à échanger.', '', '', '', '¿Alguien necesita %category? Tengo algunos para intercambiar.', '', ''),
('suggest_trade', 'WTB %category. Anyone selling cheaper than AH?', 0, 0, '', 'Cherche à acheter %category. Quelqu’un le vend moins cher que l’AH ?', '', '', '', 'Busco comprar %category. ¿Alguien lo vende más barato que en la subasta?', '', ''),
('suggest_trade', 'Selling %category cheap, whisper for details.', 0, 0, '', 'Je vends %category pas cher, chuchotez-moi pour les détails.', '', '', '', 'Vendo %category barato, susúrrame para detalles.', '', ''),
('suggest_trade', 'Anyone needs %category? I have some for sale.', 0, 0, '', 'Quelqu’un a besoin de %category ? j’en ai à vendre.', '', '', '', '¿Alguien necesita %category? Tengo algunos para vender.', '', ''),
('suggest_trade', 'Looking to trade %category for something else.', 0, 0, '', 'Cherche à échanger %category contre autre chose.', '', '', '', 'Busco intercambiar %category por algo más.', '', ''),
('suggest_trade', 'Got %category, willing to trade it for %category.', 0, 0, '', 'j’ai %category, prêt à l’échanger contre %category.', '', '', '', 'Tengo %category, dispuesto a intercambiarlo por %category.', '', ''),
('suggest_trade', 'WTB %category, anyone has extras?', 0, 0, '', 'Cherche à acheter %category, quelqu’un en a de surplus ?', '', '', '', 'Busco comprar %category, ¿alguien tiene extras?', '', ''),



-- random faction rep grind
-- usable placeholders:
-- %my_role
-- %rep_level
-- %rndK
-- %faction
-- %my_class
-- %my_race
-- %my_level
('suggest_faction', 'Anyone farming %faction rep?', 0, 0, '', 'Quelqu’un farm la réput de %faction ?', '', '', '', '¿Alguien farmea reputación con %faction?', '', ''),
('suggest_faction', 'Anyone help with %faction?', 0, 0, '', 'Quelqu’un aide-t-il %faction ?', '', '', '', '¿Alguien me ayuda con %faction?', '', ''),
('suggest_faction', 'Wanna quest for %faction.', 0, 0, '', 'Je veux quêter %faction', '', '', '', 'Quiero buscar a %faction.', '', ''),
('suggest_faction', '%faction is the best.', 0, 0, '', '%faction est la meilleure.', '', '', '', '%faction es el mejor.', '', ''),
('suggest_faction', 'Need just a bit to be %rep_level with %faction.', 0, 0, '', 'j’ai besoin d’un peu pour être au %rep_level avec %faction', '', '', '', 'Solo necesito un poco para estar %rep_level con %faction.', '', ''),
('suggest_faction', 'Anyone got %rep_level with %faction?', 0, 0, '', 'Quelqu’un a-t-il %rep_level avec %faction ?', '', '', '', '¿Alguien tiene %rep_level con %faction?', '', ''),
('suggest_faction', 'Who wants to be %rep_level with %faction?', 0, 0, '', 'Qui veut être au %rep_level avec %faction ?', '', '', '', '¿Quién quiere estar en %rep_level con %faction?', '', ''),
('suggest_faction', 'I’ll never be %rep_level with %faction.', 0, 0, '', 'Je ne serai jamais au %rep_level avec %faction', '', '', '', 'Nunca estaré %rep_level con %faction.', '', ''),
('suggest_faction', 'Someone missing %faction rep?', 0, 0, '', 'Quelqu’un manque de reput de %faction ?', '', '', '', '¿Alguien falta reputación con %faction?', '', ''),
('suggest_faction', 'Could help farming %faction rep.', 0, 0, '', 'Pourrait aider à farmer réput de %faction', '', '', '', 'Podría ayudar a farmear %faction rep.', '', ''),
('suggest_faction', 'The more rep the better. Especially with %faction.', 0, 0, '', 'Plus il y a de réput, mieux c''est. Surtout avec %faction', '', '', '', 'Cuantas más repeticiones, mejor. Especialmente con %faction.', '', ''),
('suggest_faction', '%faction: need %rndK for %rep_level.', 0, 0, '', '%faction: besoin %rndK pour %rep_level.', '', '', '', '%faction: necesita %rndK para %rep_level.', '', ''),
('suggest_faction', 'Who can share %faction quests?', 0, 0, '', 'Qui peut partager les quêtes de %faction ?', '', '', '', '¿Quién puede compartir misiones de %faction?', '', ''),
('suggest_faction', 'Any dungeons for %faction?', 0, 0, '', 'Des donjons pour %faction ?', '', '', '', '¿Alguna mazmorra para %faction?', '', ''),
('suggest_faction', 'Wanna do %faction rep grind.', 0, 0, '', 'Je veux faire monter ma réput %faction', '', '', '', 'Quiero hacer %faction rep grind.', '', ''),
('suggest_faction', 'Let’s farm %faction rep!', 0, 0, '', 'Farmons la réput %faction !', '', '', '', '¡Vamos a farmear reputación con %faction!', '', ''),
('suggest_faction', 'Farming for %faction rep.', 0, 0, '', 'Farmons pour la réput %faction', '', '', '', 'Farming for %faction rep.', '', ''),
('suggest_faction', 'Wanna farm for %faction.', 0, 0, '', 'Je veux farmer pour %faction', '', '', '', 'Quiero farmear para %faction.', '', ''),
('suggest_faction', 'Need help with %faction.', 0, 0, '', 'Besoin d’aide avec %faction', '', '', '', 'Necesito ayuda con %faction.', '', ''),
('suggest_faction', 'Is %faction sells something useful?', 0, 0, '', 'Est-ce que %faction vend quelque chose d’utile ?', '', '', '', '¿%faction vende algo útil?', '', ''),
('suggest_faction', 'Are there %faction vendors?', 0, 0, '', 'Y a-t-il des vendeurs %faction ?', '', '', '', '¿Hay %proveedores de facciones?', '', ''),
('suggest_faction', 'Who farms %faction?', 0, 0, '', 'Qui farm %faction ?', '', '', '', '¿Quién farmea %faction?', '', ''),
('suggest_faction', 'Which is the best way to farm %faction?', 0, 0, '', 'Quelle est la meilleure façon de farmer %faction ?', '', '', '', '¿Cuál es la mejor manera de farmear %faction?', '', ''),
('suggest_faction', 'I hate %faction rep grind.', 0, 0, '', 'Je déteste le travail de %faction rep.', '', '', '', 'Odio %repetir de facción', '', ''),
('suggest_faction', 'I am so tired of %faction.', 0, 0, '', 'Je suis tellement fatigué de %faction', '', '', '', 'Estoy tan cansado de %faction.', '', ''),
('suggest_faction', 'Go for %faction?', 0, 0, '', 'Vous optez pour %faction ?', '', '', '', '¿Quereis ir a por %faction?', '', ''),
('suggest_faction', 'Seems everyone is %rep_level with %faction. Only me is late as usually.', 0, 0, '', 'On dirait que tout le monde est %rep_level avec %faction Il n’y a que moi qui suis en retard comme d’habitude.', '', '', '', 'Parece que todo el mundo está %rep_level con %faction. Solo yo llego tarde como siempre.', '', ''),
('suggest_faction', 'LFG for %faction rep grind?', 0, 0, '', 'Cherche un groupe pour monter la réput des représentants de %faction ?', '', '', '', 'Busco grupo para subir reputación con %faction', '', ''),
('suggest_faction', 'Can anobody suggest a good spot for %faction rep grind?', 0, 0, '', 'Quelqu’un peut-il suggérer un bon endroit pour monter les réput de %faction ?', '', '', '', '¿Alguien puede sugerir un buen lugar farmear reputacion con %faction?', '', ''),
('suggest_faction', 'Would %faction rep benefit me?', 0, 0, '', 'Est-ce que la réput %faction me serait bénéfique ?', '', '', '', '¿Me beneficiaría subir reputación con %faction?', '', ''),
('suggest_faction', 'Who would’ve thought that %faction rep will be useful after all...', 0, 0, '', 'Qui aurait pensé que le représentant de %faction serait utile après tout...', '', '', '', 'Quién hubiera pensado que subir reputacion con %faction sería útil después de todo...', '', ''),
('suggest_faction', 'I wanna be exalted with all factions, starting with %faction.', 0, 0, '', 'Je veux être exalté avec toutes les factions, à commencer par %faction', '', '', '', 'Quiero ser exaltado con todas las facciones, comenzando con %faction.', '', ''),
('suggest_faction', 'Is there any point to improve my rep with %faction?', 0, 0, '', 'Y a-t-il un intérêt à améliorer ma réputation avec %faction ?', '', '', '', '¿Hay algún punto para mejorar mi reputación con %faction?', '', ''),
('suggest_faction', 'What is better for %faction? Quests or mob grinding?', 0, 0, '', 'Quoi de mieux pour %faction ? Quêtes ou mob pour XP?', '', '', '', '¿Qué es mejor para %faction? ¿Misiones o mafiosos?', '', ''),
('suggest_faction', 'Will grind %faction rep for you. Just give me some gold.', 0, 0, '', 'Monte la reput de %faction pour vous. Donnez-moi juste de l’or.', '', '', '', 'Ganare reputacion con %faction por ti. Sólo dame un poco de oro.', '', ''),
('suggest_faction', 'I think grinding rep with %faction would take forever.', 0, 0, '', 'Je pense que monter la réput avec %faction prendrait une éternité.', '', '', '', 'Creo que subir la reputación con %faction tomaría una eternidad.', '', ''),
('suggest_faction', 'I am killing for %faction every day now but still far from %rep_level.', 0, 0, '', 'Je tue pour %faction tous les jours maintenant, mais encore loin du %rep_level', '', '', '', 'Estoy matando por %faction todos los días ahora, pero aún estoy lejos del %rep_level.', '', ''),
('suggest_faction', 'At %my_level AH deposits will decrease, right?', 0, 0, '', 'Au %rep_level, les dépôts hôtel des ventes diminueront, n’est-ce pas?', '', '', '', 'Al %rep_level, los depósitos subastas disminuirán, ¿verdad?', '', ''),
('suggest_faction', 'How many exalted reps do you have?', 0, 0, '', 'Combien de représentants exaltés avez-vous ?', '', '', '', '¿Cuántas reputaciones exaltadas tienes?', '', ''),
('suggest_faction', 'Who wants to be %my_level with %faction?', 0, 0, '', 'Qui veut être au %rep_level avec %faction ?', '', '', '', '¿Quién quiere estar en %rep_level con %faction?', '', ''),
('suggest_faction', 'Damn. My guild did a good %faction grind yesterday without me.', 0, 0, '', 'Mince. Ma guilde a fait un bon travail de %faction hier sans moi.', '', '', '', 'Maldición. Mi hermandad hizo un buen farmeo de %faction ayer sin mí.', '', ''),
('suggest_faction', 'Nobody wants to help me because I am %rep_level with %faction.', 0, 0, '', 'Personne ne veut m’aider parce que je suis %rep_level avec %faction', '', '', '', 'Nadie quiere ayudarme porque estoy %rep_level con %faction.', '', ''),
('suggest_faction', 'Please stay away from %faction.', 0, 0, '', 'Veuillez rester à l’écart de %faction', '', '', '', 'Por favor manténgase alejado de %faction.', '', ''),
('suggest_faction', 'Anyone farming %faction rep today?', 0, 0, '', 'Quelqu’un farm la réput de %faction aujourd’hui ?', '', '', '', '¿Alguien farmea reputación de %faction hoy?', '', ''),
('suggest_faction', 'Help me grind %faction rep!', 0, 0, '', 'Aidez-moi à faire monter ma réput %faction !', '', '', '', '¡Ayúdame a subir mi reputación con %faction!', '', ''),
('suggest_faction', 'I need %rep_level with %faction so badly!', 0, 0, '', 'j’ai tellement besoin de %rep_level avec %faction !', '', '', '', '¡Necesito %rep_level con %faction con urgencia!', '', ''),
('suggest_faction', 'Is %faction grind worth it?', 0, 0, '', 'Est-ce que le grind %faction en vaut la peine ?', '', '', '', '¿Vale la pena el farmeo de %faction?', '', ''),
('suggest_faction', 'Anyone know %faction rep farm spots?', 0, 0, '', 'Quelqu’un connaît des spots pour farm la réput %faction ?', '', '', '', '¿Alguien conoce lugares para farmear reputación con %faction?', '', ''),
('suggest_faction', 'I love grinding %faction rep!', 0, 0, '', 'j’adore farmer la réput de %faction !', '', '', '', '¡Me encanta hacer grind de reputación con %faction!', '', ''),
('suggest_faction', 'Need help to finish %faction rep grind!', 0, 0, '', 'Besoin d’aide pour finir le grind de réput %faction !', '', '', '', '¡Necesito ayuda para terminar el grind de reputación de %faction!', '', ''),
('suggest_faction', 'Only a few more %rep_level with %faction to go!', 0, 0, '', 'Plus que quelques %rep_level pour être %faction', '', '', '', '¡Solo me faltan unos %rep_level para llegar a %faction!', '', ''),
('suggest_faction', 'Anyone up for %faction grind today?', 0, 0, '', 'Quelqu’un veut faire du grind %faction aujourd’hui ?', '', '', '', '¿Alguien se anima al farmeo de %faction hoy?', '', ''),
('suggest_faction', 'Looking for %faction rep grind buddies!', 0, 0, '', 'Je cherche des compagnons pour grinder %faction', '', '', '', 'Busco compañeros para farmear %faction.', '', ''),
('suggest_faction', 'I’m tired of grinding %faction rep!', 0, 0, '', 'Je suis fatigué de farmer la réput %faction !', '', '', '', '¡Estoy cansado de hacer grind de %faction!', '', ''),
('suggest_faction', 'Anyone on %faction rep grind too?', 0, 0, '', 'Quelqu’un est aussi en train de faire le grind %faction ?', '', '', '', '¿Alguien más está haciendo el grind de %faction?', '', ''),
('suggest_faction', 'Time to hit %faction rep grind hard!', 0, 0, '', 'C''est le moment de faire le grind %faction sérieusement !', '', '', '', '¡Es hora de darle duro al grind de %faction!', '', ''),
('suggest_faction', 'Help me finish %faction rep!', 0, 0, '', 'Aidez-moi à finir la réput %faction !', '', '', '', '¡Ayúdame a completar la reputación de %faction!', '', ''),
('suggest_faction', 'Need just %rndK more for %faction rep!', 0, 0, '', 'Il me manque juste %rndK pour la réput %faction !', '', '', '', '¡Solo me faltan %rndK para la reputación de %faction!', '', ''),
('suggest_faction', 'Grinding %faction rep all day!', 0, 0, '', 'Je farm la réput %faction toute la journée !', '', '', '', '¡Farmearé la reputación de %faction todo el día!', '', ''),
('suggest_faction', 'Almost at %rep_level with %faction!', 0, 0, '', 'Presque au %rep_level avec %faction !', '', '', '', '¡Casi al %rep_level con %faction!', '', ''),
('suggest_faction', '%faction rep grind never ends!', 0, 0, '', 'Le grind de réputation %faction ne finit jamais !', '', '', '', '¡El grind de reputación de %faction nunca termina!', '', ''),
('suggest_faction', 'Help with %faction daily quests?', 0, 0, '', 'Besoin d’aide pour les quêtes journalières %faction ?', '', '', '', '¿Ayuda con las misiones diarias de %faction?', '', ''),
('suggest_faction', 'Who’s on %faction grind right now?', 0, 0, '', 'Qui est en train de faire le grind %faction en ce moment ?', '', '', '', '¿Quién está farmeando %faction ahora?', '', ''),
('suggest_faction', 'Let’s do %faction quests together!', 0, 0, '', 'Faisons les quêtes %faction ensemble !', '', '', '', '¡Hagamos las misiones de %faction juntos!', '', ''),
('suggest_faction', 'Anyone farming %faction rep today?', 0, 0, '', 'Qui farm la réput %faction aujourd’hui ?', '', '', '', '¿Alguien está farmeando reputación con %faction hoy?', '', ''),
('suggest_faction', 'Need %faction rep help for %rep_level!', 0, 0, '', 'Besoin d’aide pour obtenir %rep_level avec %faction !', '', '', '', '¡Necesito ayuda con la reputación de %faction para %rep_level!', '', ''),
('suggest_faction', 'Who wants to grind %faction rep with me?', 0, 0, '', 'Qui veut grinder la réput %faction avec moi ?', '', '', '', '¿Quién quiere farmeo de %faction conmigo?', '', ''),
('suggest_faction', 'Come join me for %faction rep grind!', 0, 0, '', 'Venez me rejoindre pour le grind %faction !', '', '', '', '¡Ven a unirte a mí para el grind de reputación de %faction!', '', ''),
('suggest_faction', 'On a roll with %faction rep grind!', 0, 0, '', 'Je suis en train de faire du bon grind pour %faction !', '', '', '', '¡Estoy avanzando mucho con el grind de %faction!', '', ''),
('suggest_faction', 'Anyone want %faction rep grind boosts?', 0, 0, '', 'Quelqu’un veut des boosts pour le grind %faction ?', '', '', '', '¿Alguien quiere ayudas para el grind de %faction?', '', ''),
('suggest_faction', 'Just hit %rep_level with %faction!', 0, 0, '', 'Je viens d’atteindre %rep_level avec %faction !', '', '', '', '¡Acabo de alcanzar %rep_level con %faction!', '', ''),
('suggest_faction', '%faction is so close to %rep_level, help me out!', 0, 0, '', 'Je suis si près de %rep_level avec %faction, aidez-moi !', '', '', '', '¡%faction está tan cerca de %rep_level, ayúdame!', '', ''),
('suggest_faction', 'Wanna help me grind %faction rep?', 0, 0, '', 'Tu veux m’aider à farmer la réput %faction ?', '', '', '', '¿Quieres ayudarme a farmear reputación de %faction?', '', ''),
('suggest_faction', 'Farming %faction rep again!', 0, 0, '', 'Je farm encore la réput %faction !', '', '', '', '¡Farmear reputación de %faction otra vez!', '', ''),
('suggest_faction', 'Anyone know a good spot for %faction rep?', 0, 0, '', 'Quelqu’un connaît un bon spot pour le grind %faction ?', '', '', '', '¿Alguien conoce un buen lugar para farmear %faction?', '', ''),
('suggest_faction', 'How much more for %faction rep grind?', 0, 0, '', 'Combien de plus pour finir le grind %faction ?', '', '', '', '¿Cuánto más para terminar el grind de %faction?', '', ''),
('suggest_faction', 'Anyone doing %faction quests with me?', 0, 0, '', 'Quelqu’un fait des quêtes %faction avec moi ?', '', '', '', '¿Alguien haciendo misiones de %faction conmigo?', '', ''),
('suggest_faction', 'Looking for %rep_level %faction rep grind!', 0, 0, '', 'Je cherche %rep_level pour le grind de réput %faction !', '', '', '', 'Busco %rep_level para el grind de %faction!', '', ''),
('suggest_faction', 'Only need %rndK more %faction rep!', 0, 0, '', 'Il ne me manque plus que %rndK pour la réput %faction !', '', '', '', '¡Solo necesito %rndK más de reputación con %faction!', '', ''),
('suggest_faction', '%faction grind is real!', 0, 0, '', 'Le grind %faction est bien réel !', '', '', '', '¡El grind de %faction es real!', '', ''),
('suggest_faction', 'Anyone farming %faction rep for %rep_level?', 0, 0, '', 'Quelqu’un farm la réput %faction pour %rep_level ?', '', '', '', '¿Alguien farmeando %faction para %rep_level?', '', ''),
('suggest_faction', 'Let’s do %faction rep grind together!', 0, 0, '', 'Faisons ensemble le grind de %faction !', '', '', '', '¡Hagamos juntos el grind de reputación de %faction!', '', ''),
('suggest_faction', 'More %faction rep, more rewards!', 0, 0, '', 'Plus de réput %faction, plus de récompenses !', '', '', '', '¡Más reputación de %faction, más recompensas!', '', ''),
('suggest_faction', 'Farming %faction rep with %my_class', 0, 0, '', 'Farm de réputation %faction avec %my_class', '', '', '', 'Farmeando reputación con %faction con %my_class', '', ''),
('suggest_faction', 'Anyone else stuck at %rep_level with %faction?', 0, 0, '', 'Quelqu’un d’autre coincé à %rep_level avec %faction ?', '', '', '', '¿Alguien más atascado en %rep_level con %faction?', '', ''),
('suggest_faction', 'Doing %faction rep grind all night long!', 0, 0, '', 'Je fais le grind %faction toute la nuit !', '', '', '', '¡Voy a farmear reputación de %faction toda la noche!', '', ''),
('suggest_faction', 'Who wants to finish %faction rep grind with me?', 0, 0, '', 'Qui veut finir le grind de réputation %faction avec moi ?', '', '', '', '¿Quién quiere terminar el grind de %faction conmigo?', '', ''),
('suggest_faction', 'I can help with %faction rep grind!', 0, 0, '', 'Je peux aider avec le grind de réputation %faction !', '', '', '', '¡Puedo ayudar con el grind de reputación de %faction!', '', ''),
('suggest_faction', 'I’m ready for more %faction rep grind!', 0, 0, '', 'Je suis prêt pour encore plus de grind de réputation %faction !', '', '', '', '¡Estoy listo para más grind de reputación de %faction!', '', ''),
('suggest_faction', 'Wanna help with %faction rep? I need it bad!', 0, 0, '', 'Tu veux m’aider avec la réputation %faction ? j’en ai vraiment besoin !', '', '', '', '¿Quieres ayudarme con %faction rep? ¡Lo necesito mucho!', '', ''),
('suggest_faction', 'I need %rndK more for %faction rep!', 0, 0, '', 'Il me manque %rndK pour la réputation %faction !', '', '', '', '¡Me faltan %rndK para la reputación de %faction!', '', ''),
('suggest_faction', 'Farm with me for %faction rep!', 0, 0, '', 'Farme avec moi pour la réputation %faction !', '', '', '', '¡Farmea conmigo para reputación de %faction!', '', ''),
('suggest_faction', 'I just need %rndK more %faction rep!', 0, 0, '', 'Il me manque juste %rndK pour la réputation %faction !', '', '', '', '¡Solo necesito %rndK más de %faction!', '', ''),
('suggest_faction', 'Doing %faction rep grind is never-ending...', 0, 0, '', 'Faire le grind %faction n’a pas de fin...', '', '', '', 'El grind de %faction nunca termina...', '', ''),
('suggest_faction', 'Anyone got %faction rep boost items?', 0, 0, '', 'Quelqu’un a des objets pour booster la réputation %faction ?', '', '', '', '¿Alguien tiene objetos para boostear reputación de %faction?', '', ''),
('suggest_faction', 'I need %rep_level %faction to unlock cool rewards!', 0, 0, '', 'Il me faut %rep_level %faction pour débloquer des récompenses sympas !', '', '', '', '¡Necesito %rep_level de %faction para desbloquear buenas recompensas!', '', ''),
('suggest_faction', 'Anyone doing %faction rep for mounts?', 0, 0, '', 'Quelqu’un fait du grind %faction pour des montures ?', '', '', '', '¿Alguien farmeando reputación de %faction para monturas?', '', ''),
('suggest_faction', 'Any chance to farm %faction rep with me?', 0, 0, '', 'Il y a moyen de farmer la réput %faction avec moi ?', '', '', '', '¿Algún chance de farmear reputación de %faction conmigo?', '', ''),
('suggest_faction', 'I am so close to %rep_level with %faction!', 0, 0, '', 'Je suis si proche de %rep_level avec %faction !', '', '', '', '¡Estoy tan cerca de %rep_level con %faction!', '', ''),
('suggest_faction', 'Help me get %rep_level with %faction!', 0, 0, '', 'Aidez-moi à obtenir %rep_level avec %faction !', '', '', '', '¡Ayúdame a conseguir %rep_level con %faction!', '', ''),
('suggest_faction', 'Getting %faction rep grind done!', 0, 0, '', 'Je finis le grind de réputation %faction !', '', '', '', '¡Estoy terminando el grind de reputación de %faction!', '', ''),
('suggest_faction', 'I’m in need of %faction rep grinding help!', 0, 0, '', 'j’ai besoin d’aide pour le grind de réputation %faction !', '', '', '', '¡Necesito ayuda con el grind de %faction!', '', ''),
('suggest_faction', 'Need %rep_level %faction for the next rewards!', 0, 0, '', 'Besoin de %rep_level %faction pour les prochaines récompenses !', '', '', '', '¡Necesito %rep_level de %faction para las siguientes recompensas!', '', ''),
('suggest_faction', 'Who can farm %faction rep with me?', 0, 0, '', 'Qui peut farmer la réputation %faction avec moi ?', '', '', '', '¿Quién puede farmear la reputación de %faction conmigo?', '', ''),
('suggest_faction', 'Grinding %faction rep non-stop!', 0, 0, '', 'Grind %faction non-stop !', '', '', '', '¡Grind de %faction sin parar!', '', ''),
('suggest_faction', 'Who else is grinding %faction rep right now?', 0, 0, '', 'Qui d’autre est en train de farmer la réputation %faction ?', '', '', '', '¿Quién más está farmeando %faction ahora?', '', ''),
('suggest_faction', 'Anyone want to farm %faction rep for fun?', 0, 0, '', 'Quelqu’un veut farm la réputation %faction pour le fun ?', '', '', '', '¿Alguien quiere farmear %faction por diversión?', '', ''),
('suggest_faction', 'Need %faction rep grind to unlock new quests!', 0, 0, '', 'Besoin de grinder la réputation %faction pour débloquer de nouvelles quêtes !', '', '', '', '¡Necesito hacer grind de %faction para desbloquear nuevas misiones!', '', ''),
('suggest_faction', 'Wanna grind %faction rep for rewards!', 0, 0, '', 'Tu veux grinder la réputation %faction pour les récompenses !', '', '', '', '¡Vamos a grindear reputación de %faction por recompensas!', '', ''),
('suggest_faction', 'Grinding %faction rep, one day at a time!', 0, 0, '', 'Grind de réputation %faction, un jour à la fois !', '', '', '', '¡Farmeando reputación de %faction, un día a la vez!', '', ''),
('suggest_faction', 'Anyone want to help with %faction rep grind?', 0, 0, '', 'Quelqu’un veut aider avec le grind de réputation %faction ?', '', '', '', '¿Alguien quiere ayudar con el grind de %faction?', '', ''),
('suggest_faction', 'Help me unlock rewards with %faction rep!', 0, 0, '', 'Aidez-moi à débloquer des récompenses avec la réput %faction !', '', '', '', '¡Ayúdame a desbloquear recompensas con la reputación de %faction!', '', ''),
('suggest_faction', 'I’m grinding %faction rep for the mounts!', 0, 0, '', 'Je farm la réputation %faction pour les montures !', '', '', '', '¡Estoy farmeando %faction para las monturas!', '', ''),
('suggest_faction', 'Come farm %faction rep with me!', 0, 0, '', 'Viens farmer la réputation %faction avec moi !', '', '', '', '¡Ven a farmear %faction conmigo!', '', ''),
('suggest_faction', 'I’m almost there with %faction rep!', 0, 0, '', 'Je suis presque là avec la réputation %faction !', '', '', '', '¡Casi llego con %faction!', '', ''),
('suggest_faction', 'Grinding %faction rep with my friends!', 0, 0, '', 'Je farm la réputation %faction avec mes amis !', '', '', '', '¡Estoy farmeando %faction con mis amigos!', '', ''),



-- random generic
-- usable placeholders:
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('suggest_something', 'Wanna party in %zone_name.', 0, 0, '', 'Je veux faire la fête dans %zone_name.', '', '', '', '¡Vamos a perrear a %zone_name!', '', 'Ищу группу в %zone_name.'),
('suggest_something', 'Anyone is looking for %my_role?', 0, 0, '', 'Quelqu’un cherche un %my_role ?', '', '', '', '¿Alguien está buscando %my_role?', '', 'Кто-нибудь ищет %my_role?'),
('suggest_something', '%my_role is looking for guild.', 0, 0, '', '%my_role recherche une guilde.', '', '', '', '%my_role está buscando hermandad.', '', '%my_role ищу гильдию.'),
('suggest_something', 'Looking for gold.', 0, 0, '', 'A la recherche de l’or.', '', '', '', 'Buscando oro.', '', 'Дайте голды'),
('suggest_something', '%my_role wants to join a good guild.', 0, 0, '', '%my_role veut rejoindre une bonne guilde.', '', '', '', '%my_role quiere unirse a una buen hermandad.', '', '%my_role хочу в хорошую гильдию.'),
('suggest_something', 'Need a friend.', 0, 0, '', 'Besoin d’un ami.', '', '', '', 'Necesito un amigo...', '', 'Ищу друга.'),
('suggest_something', 'Anyone feels alone?', 0, 0, '', 'Quelqu’un se sent seul?', '', '', '', '¿Alguien se siente solo?', '', 'Кому-нибудь одиноко?'),
('suggest_something', 'Boring...', 0, 0, '', 'Ennuyeux...', '', '', '', 'Aburrido...', '', 'Скучно...'),
('suggest_something', 'Who wants some?', 0, 0, '', 'Qui en veut?', '', '', '', '¿Quién quiere hacer grupo para levear?', '', 'Кому навалять?'),
('suggest_something', 'Go get me!', 0, 0, '', 'Allez me chercher !', '', '', '', '¡Ven a buscarme!', '', 'Поймайте меня!'),
('suggest_something', 'Maybe a duel in %zone_name?', 0, 0, '', 'Peut-être un duel dans %zone_name ?', '', '', '', '¿Quizás un duelo en %zone_name?', '', 'Кто на дуэль в %zone_name?'),
('suggest_something', 'Anybody doing something?', 0, 0, '', 'Quelqu’un fait quelque chose?', '', '', '', '¿Alguien está haciendo algo?', '', 'Кто чем занят?'),
('suggest_something', '%zone_name: is anybody here?', 0, 0, '', '%zone_name : est-ce que quelqu’un est là?', '', '', '', '%zone_name: ¿hay alguien aquí?', '', '%zone_name: есть кто нибудь?'),
('suggest_something', '%zone_name: where is everyone?', 0, 0, '', '%zone_name : où est tout le monde?', '', '', '', '¿Hay alguien en %zone_name?', '', '%zone_name: где все?'),
('suggest_something', 'Looks like I am alone in %zone_name.', 0, 0, '', 'On dirait que je suis seul dans %zone_name', '', '', '', 'Parece que estoy solo en %zone_name.', '', 'Похоже я один в %zone_name.'),
('suggest_something', 'Meet me in %zone_name.', 0, 0, '', 'Retrouvez-moi dans %zone_name', '', '', '', 'Encuéntrame en %zone_name.', '', 'Встретимся в %zone_name.'),
('suggest_something', 'Let’s quest in %zone_name!', 0, 0, '', 'Partons en quête dans %zone_name !', '', '', '', '¡Vamos a hacer un grupo para levear en %zone_name!', '', 'Давайте квестить в %zone_name!'),
('suggest_something', '%zone_name is the best place to be!', 0, 0, '', '%zone_name est le meilleur endroit où être!', '', '', '', '%zone_name es el mejor lugar para estar!', '', '%zone_name лучшее место!'),
('suggest_something', 'Wanna go to %zone_name. Anybody with me?', 0, 0, '', 'Je veux aller à %zone_name Quelqu’un avec moi?', '', '', '', 'Quiero ir a %zone_name. ¿Alguien se apunta?', '', 'Собираюсь в %zone_name. Кто со мной7'),
('suggest_something', 'Who wants going to %zone_name?', 0, 0, '', 'Qui veut aller à %zone_name ?', '', '', '', '¿Quién quiere ir a %zone_name?', '', 'Кто собирается в %zone_name?'),
('suggest_something', 'I don’t like %zone_name. Where to go?', 0, 0, '', 'Je n’aime pas %zone_name Où aller?', '', '', '', 'No me gusta %zone_name. ¿Dónde podria ir?', '', 'Не нравится %zone_name. Куда идти?'),
('suggest_something', 'Are there a good quests in %zone_name?', 0, 0, '', 'Y a-t-il de bonnes quêtes dans %zone_name ?', '', '', '', '¿Hay buenas misiones en %zone_name?', '', 'А в %zone_name есть хорошие квесты?'),
('suggest_something', 'Where to go after %zone_name?', 0, 0, '', 'Où aller après %zone_name ?', '', '', '', '¿Adónde puedo ir después de %zone_name?', '', 'Куда идти после %zone_name?'),
('suggest_something', 'Who is in %zone_name?', 0, 0, '', 'Qui est dans %zone_name ?', '', '', '', '¿Quién está en %zone_name?', '', 'Кто в %zone_name?'),
('suggest_something', 'LFG in %zone_name.', 0, 0, '', 'Cherche un groupe dans %zone_name', '', '', '', 'Busco grupo en %zone_name.', '', 'ЛФГ %zone_name.'),
('suggest_something', '%zone_name is the worst place to be.', 0, 0, '', '%zone_name est le pire endroit où être.', '', '', '', '%zone_name es el peor lugar para estar', '', '%zone_name это худшее место.'),
('suggest_something', 'Catch me in %zone_name!', 0, 0, '', 'Attrape-moi dans %zone_name !', '', '', '', '¡Atrápame en %zone_name!', '', 'Поймай меня в %zone_name!'),
('suggest_something', 'Go for %zone_name!', 0, 0, '', 'Allez pour %zone_name !', '', '', '', 'Me voy a %zone_name!', '', 'Гоу в %zone_name!'),
('suggest_something', 'Wanna quest in %zone_name', 0, 0, '', 'Je veux une quête dans %zone_name', '', '', '', 'Quiero hacer una búsqueda en %zone_name', '', 'Хочешь поделать квесты в %zone_name'),
('suggest_something', 'Anyone has quests in %zone_name?', 0, 0, '', 'Quelqu’un a des quêtes dans %zone_name ?', '', '', '', '¿Alguien tiene misiones en %zone_name?', '', 'Есть у кого квесты в %zone_name?'),
('suggest_something', 'Come here to %zone_name!', 0, 0, '', 'Venez ici à %zone_name !', '', '', '', '¡Ven aquí a %zone_name!', '', 'Приходите сюда в %zone_name!'),
('suggest_something', 'Seems there is no Horde in %zone_name', 0, 0, '', 'Il semble qu’il n’y ait pas de Horde dans %zone_name', '', '', '', 'Parece que no hay Horda en %zone_name', '', '%zone_name Орды вроде нет.'),
('suggest_something', 'Seems there is no Alliance in %zone_name', 0, 0, '', 'Il semble qu’il n’y ait pas d’alliance dans %zone_name', '', '', '', 'Parece que no hay Alianza en %zone_name', '', '%zone_name Альянса вроде нет'),
('suggest_something', 'I am really tired of %zone_name. Maybe go somewhere else?', 0, 0, '', 'Je suis vraiment fatigué de %zone_name Peut-être aller ailleurs?', '', '', '', 'Estoy realmente cansado de %zone_name. Quizás ire a otro lugar', '', 'В %zone устал уже, может в другое место пойдем?'),
('suggest_something', 'GL', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'I want to go home, and then edge', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'does anyone know what you need to dual wield?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'hi everyone!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', '%zone_name is comfy', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'i feel great', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'i don’t ignore people i troll them until they ignore me', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'What do you guys think about my build? %my_role', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'good to see chat still remembers', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'like all weapons its hunter BIS', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'whole point to the game for me is soloing and finding new ways to solo stuff', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'i’ve NEVER ripped off anyone', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'ah yes the world of warcraft, where i come for life advice', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'HELLO?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Time to fight my way into %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', '%zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'gotta poop', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'if you don’t loot your skinnable kills your pp loses 1mm permanently', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'NOOOOOOOOOOOOO', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'I LIKE POTATO', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'w chat', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'hi how are you guys', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'i just logged out and logged back in', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'can you guys keep it down a bit, im lost in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'anyone want to have a drink with me at %zone_name ...hic!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'hahahahaheeeeeeee dirin diring ingggggg hahahahaheeeeeeeeeeeeee', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'bait used to be believeable', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'maybe you just lost your innocence', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'any guilds willing to carry %my_role?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'once you start getting higher, gold is so easy to get', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'morning', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'why does my ass hurt?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'I feel like spirit is bis for leveling', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Even more for troll', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'CAN SOMEONE INVITE ME', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'need a lot of drinks', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'damn gnomes', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'no one likes gnomes', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'gnomes are only good for one thing', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Well', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'mushrooms', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'automatic thoughts are a scary thing', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'the mind is more pliable than we''d like to believe', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'any leveling guilds out there?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'brb', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Why is snow white but Ice is clear ? made of the same thing', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'why is whipped cream fluffy and regular not', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'why do feet smell if they have no nose', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'seems a can of newbies arrived', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'stop trolling new players with BS answers', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Is there PvP on this server?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'duh', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'phew... :)', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'did you guys know that', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'i don’t try to imagine what other creatures feel like', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'oh whoops, wrong chat', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'bruh you guys are wildin out today', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'just let it be known that my text was here', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'grrr angyy', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'the grind is fun', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Wow keeps me sharp', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'hey have a question where can i take the roll for more xp? im in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'do you guys like sausages?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Invite me. I’ll help', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'which class do you think is better for pvp?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'where the fuck is the cooking trainer in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'you know what happens in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'I need to craft something', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something', 'Anyone up for some nonsense in %zone_name?', 0, 0, '', 'Quelqu’un pour des bêtises dans %zone_name ?', '', '', '', '¿Alguien para tonterías en %zone_name?', '', 'Кто-нибудь для глупости в %zone_name?'),
('suggest_something', 'Let’s make %zone_name our playground.', 0, 0, '', 'Faisons de %zone_name notre terrain de jeu.', '', '', '', 'Hagamos de %zone_name nuestro patio de recreo.', '', 'Давайте сделаем %zone_name нашей игровой площадкой.'),
('suggest_something', 'I’m gonna get lost in %zone_name again.', 0, 0, '', 'Je vais encore me perdre dans %zone_name.', '', '', '', 'Voy a perderme otra vez en %zone_name.', '', 'Снова потеряюсь в %zone_name.'),
('suggest_something', 'Can’t believe I’m still here in %zone_name.', 0, 0, '', 'Je ne peux pas croire que je sois encore ici dans %zone_name.', '', '', '', 'No puedo creer que todavía esté aquí en %zone_name.', '', 'Не верю, что я до сих пор здесь в %zone_name.'),
('suggest_something', 'Wanna team up for %zone_name madness?', 0, 0, '', 'Envie de faire équipe pour un peu de folie dans %zone_name ?', '', '', '', '¿Quieres unirte para la locura en %zone_name?', '', 'Хочешь составить команду для сумасшествия в %zone_name?'),
('suggest_something', 'Let’s raid the mailbox in %zone_name!', 0, 0, '', 'Faisons une razzia sur la boîte aux lettres de %zone_name !', '', '', '', '¡Vamos a asaltar el buzón en %zone_name!', '', 'Давайте захватим почтовый ящик в %zone_name!'),
('suggest_something', 'I swear, %zone_name is full of trolls.', 0, 0, '', 'Je jure que %zone_name est rempli de trolls.', '', '', '', 'Juro que %zone_name está lleno de trolls.', '', 'Клянусь, %zone_name полно троллей.'),
('suggest_something', 'Let’s create chaos in %zone_name together.', 0, 0, '', 'Créons le chaos dans %zone_name ensemble.', '', '', '', 'Creamos caos juntos en %zone_name.', '', 'Давайте устроим хаос в %zone_name вместе.'),
('suggest_something', 'I feel like doing absolutely nothing in %zone_name.', 0, 0, '', 'j’ai envie de ne rien faire dans %zone_name.', '', '', '', 'Tengo ganas de no hacer nada en %zone_name.', '', 'Хочу просто ничего не делать в %zone_name.'),
('suggest_something', 'No one ever listens in %zone_name.', 0, 0, '', 'Personne n’écoute jamais dans %zone_name.', '', '', '', 'Nadie escucha nunca en %zone_name.', '', 'В %zone_name никто никогда не слушает.'),
('suggest_something', 'Is %zone_name even real?', 0, 0, '', 'Est-ce que %zone_name est réel ?', '', '', '', '¿Es %zone_name siquiera real?', '', 'Существует ли вообще %zone_name?'),
('suggest_something', 'Someone send help to %zone_name! It’s wild!', 0, 0, '', 'Envoyez de l’aide à %zone_name ! C''est la folie !', '', '', '', '¡Alguien envíe ayuda a %zone_name! ¡Es una locura!', '', 'Кто-нибудь, отправьте помощь в %zone_name! Тут творится безумие!'),
('suggest_something', 'I am sure %zone_name is haunted.', 0, 0, '', 'Je suis sûr que %zone_name est hantée.', '', '', '', 'Estoy seguro de que %zone_name está embrujado.', '', 'Уверен, что в %zone_name есть призраки.'),
('suggest_something', 'If %zone_name could talk, it would probably scream.', 0, 0, '', 'Si %zone_name pouvait parler, elle crierait probablement.', '', '', '', 'Si %zone_name pudiera hablar, probablemente gritaría.', '', 'Если бы %zone_name мог говорить, он, наверное, кричал бы.'),
('suggest_something', 'There’s something fishy about %zone_name.', 0, 0, '', 'Il y a quelque chose de louche à propos de %zone_name.', '', '', '', 'Hay algo sospechoso acerca de %zone_name.', '', 'В %zone_name что-то подозрительное.'),
('suggest_something', 'I’m about to break %zone_name with my awesomeness.', 0, 0, '', 'Je suis sur le point de casser %zone_name avec ma grandeur.', '', '', '', 'Estoy a punto de romper %zone_name con mi grandeza.', '', 'Я собираюсь разрушить %zone_name своей крутостью.'),
('suggest_something', 'Someone tell %zone_name that I am here.', 0, 0, '', 'Quelqu’un dit à %zone_name que je suis là.', '', '', '', 'Alguien dígale a %zone_name que estoy aquí.', '', 'Кто-нибудь скажите %zone_name, что я здесь.'),
('suggest_something', 'I’m lost in %zone_name, send help.', 0, 0, '', 'Je suis perdu dans %zone_name, envoyez de l’aide.', '', '', '', 'Estoy perdido en %zone_name, envíen ayuda.', '', 'Я заблудился в %zone_name, отправьте помощь.'),
('suggest_something', 'I’m starting a revolution in %zone_name.', 0, 0, '', 'Je lance une révolution dans %zone_name.', '', '', '', 'Estoy comenzando una revolución en %zone_name.', '', 'Я начинаю революцию в %zone_name.'),
('suggest_something', 'Anyone else find %zone_name suspiciously quiet?', 0, 0, '', 'Quelqu’un d’autre trouve %zone_name étrangement silencieuse ?', '', '', '', '¿Alguien más encuentra %zone_name sospechosamente tranquila?', '', 'Кто-нибудь еще находит %zone_name подозрительно тихой?'),
('suggest_something', 'I bet %zone_name has secrets we don’t know about.', 0, 0, '', 'Je parie que %zone_name a des secrets que nous ne connaissons pas.', '', '', '', 'Apuesto a que %zone_name tiene secretos que no conocemos.', '', 'Держу пари, что в %zone_name есть секреты, о которых мы не знаем.'),
('suggest_something', 'You can never have enough %my_race in %zone_name.', 0, 0, '', 'On n’a jamais assez de %my_race dans %zone_name.', '', '', '', 'Nunca se tiene suficiente %my_race en %zone_name.', '', 'Никогда не бывает слишком много %my_race в %zone_name.'),
('suggest_something', 'I’m beginning to think %zone_name is just a myth.', 0, 0, '', 'Je commence à penser que %zone_name n’est qu’un mythe.', '', '', '', 'Empiezo a pensar que %zone_name es solo un mito.', '', 'Я начинаю думать, что %zone_name — это миф.'),
('suggest_something', 'Who knew %zone_name would be this crazy?', 0, 0, '', 'Qui savait que %zone_name serait aussi fou ?', '', '', '', '¿Quién sabía que %zone_name sería tan loco?', '', 'Кто знал, что %zone_name будет таким безумным?'),
('suggest_something', 'It’s like the game broke in %zone_name.', 0, 0, '', 'C''est comme si le jeu avait buggé dans %zone_name.', '', '', '', 'Es como si el juego se rompiera en %zone_name.', '', 'Похоже, игра сломалась в %zone_name.'),
('suggest_something', 'Is %zone_name actually a desert or am I just lost?', 0, 0, '', 'Est-ce que %zone_name est vraiment un désert ou est-ce que je suis juste perdu ?', '', '', '', '¿%zone_name es realmente un desierto o simplemente estoy perdido?', '', '%zone_name реально пустыня или я просто заблудился?'),
('suggest_something', 'This place is starting to feel like %zone_name in a blender.', 0, 0, '', 'Cet endroit commence à ressembler à %zone_name dans un mixeur.', '', '', '', 'Este lugar empieza a sentirse como %zone_name en una licuadora.', '', 'Это место начинает напоминать %zone_name в блендере.'),
('suggest_something', 'Anyone got a map for %zone_name?', 0, 0, '', 'Quelqu’un a une carte pour %zone_name ?', '', '', '', '¿Alguien tiene un mapa para %zone_name?', '', 'Кто-нибудь имеет карту для %zone_name?'),
('suggest_something', 'Who else thinks %zone_name is trying to kill us?', 0, 0, '', 'Qui d’autre pense que %zone_name essaie de nous tuer ?', '', '', '', '¿Quién más cree que %zone_name está tratando de matarnos?', '', 'Кто еще думает, что %zone_name пытается нас убить?'),
('suggest_something', 'I’m on a wild ride through %zone_name!', 0, 0, '', 'Je fais un tour sauvage à travers %zone_name !', '', '', '', '¡Estoy en una aventura salvaje a través de %zone_name!', '', 'Я на дикой поездке через %zone_name!'),
('suggest_something', 'Is it just me or is %zone_name getting weirder?', 0, 0, '', 'Est-ce que c''est moi ou %zone_name devient de plus en plus étrange ?', '', '', '', '¿Soy solo yo o %zone_name se está volviendo más raro?', '', 'Только мне кажется, что %zone_name становится все страннее?'),
('suggest_something', 'I’m convinced %zone_name is secretly an amusement park.', 0, 0, '', 'Je suis convaincu que %zone_name est secrètement un parc d’attractions.', '', '', '', 'Estoy convencido de que %zone_name es secretamente un parque de atracciones.', '', 'Я убежден, что %zone_name — это тайный парк аттракционов.'),


-- random generic toxic phrases
-- usable placeholders:
-- %random_inventory_item_link
-- %my_role
-- %zone_name
-- %area_name
-- %my_class
-- %my_race
-- %my_level
('suggest_something_toxic', 'why is my keyboard not working, is it because I don’t have enough friends', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'do you think the trees in this game have feelings', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I keep pressing the same button, but i’m still not winning', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I bet i could win a raid if i just yelled loot', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’ve been trying to mount my mount for an hour, nothing’s happening', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Is there a way to become invincible without actually playing the game', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'can i just press random keys and still win, asking for a friend', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'If i stand still, does that mean i’m a better player than you', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'why doesn’t my pet heal me when i stand in fire', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'so wait, do i need armor to be a true hero or is it optional', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I need to collect 10000 Lucky Charm’s to level up, right?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'my weapon is broken, but can I still just slap enemies with my huge Di.. err fist', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Can we just rename this game to “World of DieCraft a Player keeps killing me”', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'why doesn’t my horse fly? can’t it just go up', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Is there an option to turn off the graphics so it looks like my old calculator', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I was supposed to kill something but i fell asleep, is that normal', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'does anyone else just kill random NPCs for fun or is it just me', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’m %my_level and i already know all the secrets of the game', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'can i just talk to the NPCs until they give me all the loot?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'WWWWWWwwwwwwwww ssssSSsssss dDDddddd aaaAA', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'why am i losing, i’ve been playing for 15 minutes', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I don’t need a strategy, i just need to spam this ability and hope for the best', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'If i equip a bunch of Lucky Charm’s, does that make me get more loot?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Is the secret to the game just screaming at the screen', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'can i just put on every item i find and hope for the best', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'do i really need to do quests or can i just explore and yell at people', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’m at work playing this game, they say I should be working >.> but it’s WOW!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'If i eat all the food in the game, will it make me invincible', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'so what if i just throw my pet into every boss fight and need all time, it’s hunters rules', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'can you die if you’re already dead, asking for a ghost', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'does anyone know where the legendary sword is, i’ve only looked for 5 minutes', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I was told if I come here I get 100000 Gold where do I go?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I play this game like I had your mum lastnight, in a screaming glory!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'so when does the game start getting fun, or is it all just grinding forever', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I keep asking for directions, but they keep sending me to random places, is this normal', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I found out if your press ALF+f4 you get 100000 Gold, trick is you need to attact a red NPC 1st', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'why does my character always look so confused, is that my fault', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Hey your mum said goto bed!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Warcraft is a grind just like your mum was other day, she even has monthly subscription!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Is it true that Chuck Norris once leveled up without even pressing a button', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Chuck Norris doesn’t need a mount, the ground just moves under him', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'when Chuck Norris enters a raid, the boss just surrenders', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Chuck Norris doesn’t use abilities, he just looks at the enemies and they die', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Chuck Norris doesn’t need healing, his enemies just heal him out of respect', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Chuck Norris can solo a raid... with no gear... and while AFK', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Chuck Norris doesn’t need to dodge, the game dodges him', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Chuck Norris once used a low-level weapon, and the game gave him the best loot', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'When Chuck Norris enters the instance, the loot automatically drops into his inventory', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'what is ligma', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'what is sugma', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'ligma balls', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'sugma balls', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I EAT ASS', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I want to shove %random_inventory_item_link up my ass', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I want to shove %random_inventory_item_link up your ass', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Darnasses', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'seems like your suffering from sugma', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'deez nutz in ur mouth', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'cool boner, bro', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'ERP?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'i tried everything, but at the end ERP did the trick', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I want to bonk in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'looking for female gnome with gorilla pet for erp in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'i may understand an asshole, but a pervert?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'there is no gyat in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’m killing all the animals in %zone_name. Fuck the animals!!!', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'good think i got 3 legs', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'dont be mad im goonin like a sigma', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'try finger, but hole', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'You somehow managed to mess that up, impressive', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I could do this blindfolded, but I don’t need to', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'You sure you know how to play this game?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'It’s like you’re trying to lose on purpose', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’ve seen better plays from level 1 characters', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'You’re like a speed bump in this game', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Maybe you should try a different hobby', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Wow, your gameplay is like watching paint dry', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I think my alt could carry you through this', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Are you sure you’re not trolling us?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I think the enemy team is actually doing better with you on our side', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Maybe play the game instead of watching the scenery', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I think you missed every single ability there', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Are you lost or just pretending to be bad?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’ve seen level 1 characters take less damage than that', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'That was embarrassing for both of us', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’ve seen better teamwork in a solo game', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I’m just going to stand back and watch you fail', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Did you really think that was a good idea?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I can’t believe you managed to fail that simple task', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Are you here to help, or just to waste time?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'It’s a good thing I’m here to carry this team', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I swear you make this harder than it needs to be', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'How do you keep failing the simplest things?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Maybe try watching the game, instead of just playing', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'You could at least try to pretend you’re good', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'If you keep this up, we’ll be here all day', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Honestly, that was the worst decision ever', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'I don’t think you’re cut out for this game', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'What was the point of that move?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'You’re like a burden on this team', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'You really think you’re a pro, huh?', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_something_toxic', 'Maybe it’s time to just quit and save us all the trouble', 0, 0, '', '', '', '', '', '', '', ''),


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
('suggest_toxic_links', '%prefix %random_taken_quest_or_item_link', 0, 0, '', '', '', '', '', '', '', ''),
('suggest_toxic_links', '%prefix %random_inventory_item_link', 0, 0, '', '', '', '', '', '', '', ''),


-- thunderfury spam
-- usable placeholders:
-- %thunderfury_link
('thunderfury_spam', '%thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link%thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link%thunderfury_link%thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I think I just heard %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I think I heard %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I definitely heard %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I dunno but I’m pretty sure I heard %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Did you just say %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'did someone say %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Did someone say %thunderfury_link ?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Someone said %thunderfury_link', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link is coming out of the closet', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'could swear it was a %thunderfury_link, might have been %thunderfury_link tho', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Why use %thunderfury_link when %thunderfury_link is clearly way more OP', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Did I just hear %thunderfury_link?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'No way... is that a %thunderfury_link I hear?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link? Sounds like something only legends wield!', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I can feel the power of %thunderfury_link in the air!', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link is the *real* MVP', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Can someone get me a %thunderfury_link? Asking for a friend... maybe even two!', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Hey, did %thunderfury_link just *smite* something? I swear I felt the ground shake.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link on the battlefield... It’s like an instant flex.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I *think* I heard %thunderfury_link... Or was that just a myth?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Wait, did I just see someone wielding %thunderfury_link?!', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'The power of %thunderfury_link is *undeniable*', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I’m 99% sure that was %thunderfury_link. 1% might have been a random NPC, tho.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', '%thunderfury_link in action! Who needs an army when you’ve got that kind of power?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I heard %thunderfury_link is what really brings all the boys to the yard.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Why even use %thunderfury_link when %thunderfury_link is clearly the superior option?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Is it me or does %thunderfury_link have a “smack talk” passive effect?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'I thought I heard %thunderfury_link, but it was just the sound of sheer awesome.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Oh, %thunderfury_link... Where *have* you been all my life?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Did someone say %thunderfury_link? I think I need to go pick mine up.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'That was definitely the sound of %thunderfury_link! I can feel the epicness.', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'Is it just me, or does %thunderfury_link have a tendency to make everyone else *look* weak?', 0, 0, '', '', '', '', '', '', '', ''),
('thunderfury_spam', 'You know what they say... the louder the %thunderfury_link, the bigger the bragging rights!', 0, 0, '', '', '', '', '', '', '', ''),


-- random WTS
-- usable placeholders:
-- %%item_formatted_link
-- %item_formatted_link
-- %item_count
-- %cost_gold
-- %my_class
-- %my_race
-- %my_level
('suggest_sell', 'WTS %item_formatted_link for %cost_gold.', 0, 0, '', 'Cherche à vendre %item_formatted_link pour %cost_gold', '', '', '', 'Quiero vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Who wants %item_formatted_link for %cost_gold?', 0, 0, '', 'Qui veut %item_formatted_link pour %cost_gold ?', '', '', '', '¿Quién quiere %item_formatted_link por %cost_gold?', '', ''),
('suggest_sell', 'Anyone wants %item_formatted_link? Only %cost_gold.', 0, 0, '', 'Quelqu’un veut %item_formatted_link ? Seulement %cost_gold', '', '', '', '¿Alguien quiere %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'Just %cost_gold for %item_formatted_link!', 0, 0, '', 'Seulement %cost_gold pour %item_formatted_link!', '', '', '', '¡Solo %cost_gold para %item_formatted_link!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold.', 0, 0, '', 'Vendre %item_formatted_link pour %cost_gold', '', '', '', 'Vendo %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', '%item_formatted_link is yours just for %cost_gold!', 0, 0, '', '%item_formatted_link est à vous juste pour %cost_gold !', '', '', '', '%item_formatted_link es tuyo solo por %cost_gold!', '', ''),
('suggest_sell', 'Ridiculus price of %cost_gold for %item_formatted_link!', 0, 0, '', 'Prix ridicule de %cost_gold pour %item_formatted_link !', '', '', '', '¡Precio ridículo de %cost_gold para %item_formatted_link_link!', '', ''),
('suggest_sell', 'Wanna sell %item_formatted_link for %cost_gold.', 0, 0, '', 'Je veux vendre %item_formatted_link pour %cost_gold', '', '', '', 'Quiero vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Who needs %item_formatted_link? Only %cost_gold.', 0, 0, '', 'Qui a besoin de %item_formatted_link ? Seulement %cost_gold', '', '', '', '¿Quién necesita %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'Anyone needs %item_formatted_link for %cost_gold?', 0, 0, '', 'Quelqu’un a besoin de %item pour %cost_gold ?', '', '', '', '¿Alguien necesita %item_formatted_link_link por %cost_gold?', '', ''),
('suggest_sell', '%cost_gold for %item_formatted_link. Less than AH!', 0, 0, '', '%cost_gold pour %item_formatted_link. Moins que l’hôtel des ventes!', '', '', '', '%cost_gold para %item_formatted_link. ¡Menos que en las subastas!', '', ''),
('suggest_sell', '%item_formatted_link is expensive, but I’d sell it for %cost_gold.', 0, 0, '', '%item_formatted_link est cher, mais je le vendrais pour %cost_gold', '', '', '', '%item_formatted_link es caro, pero lo vendería por %cost_gold.', '', ''),
('suggest_sell', 'You’ll never find %item_formatted_link cheaper than %cost_gold!', 0, 0, '', 'Vous ne trouverez jamais %item_formatted_link moins cher que %cost_gold !', '', '', '', '¡Nunca encontrarás %item_formatted_link más barato que %cost_gold!', '', ''),
('suggest_sell', 'Need more than %item_formatted_link!', 0, 0, '', 'Besoin de plus de %item_formatted_link !', '', '', '', '¡Necesito más %item_formatted_link!', '', ''),
('suggest_sell', 'I have %item_formatted_link and need more.', 0, 0, '', 'j’ai %item_formatted_link et j’en ai besoin de plus.', '', '', '', 'Tengo %item_formatted_link y necesito más. ¿Alguien me vende alguno?', '', ''),
('suggest_sell', 'Have %item_formatted_link. Who wants to buy for %cost_gold?', 0, 0, '', 'Avoir %item_formatted_link. Qui veut acheter pour %cost_gold ?', '', '', '', 'Tengo %item_formatted_link. ¿Quién quiere comprarlo por %cost_gold?', '', ''),
('suggest_sell', 'Anyone WTB %item_formatted_link for %cost_gold?', 0, 0, '', 'Quelqu’un cherche à acheter %item_formatted_link pour %cost_gold ?', '', '', '', '¿Alguien compra %item_formatted_link por %cost_gold?', '', ''),
('suggest_sell', 'What about %item_formatted_link? For %cost_gold.', 0, 0, '', 'qu’en est-il de %item_formatted_link ? Pour %cost_gold', '', '', '', '¿Qué pasa con %item_formatted_link? Por el oro.', '', ''),
('suggest_sell', 'Who said I am a bastard? %item_formatted_link for %cost_gold is a good price.', 0, 0, '', 'Qui a dit que j’étais un bâtard? %item_formatted_link pour %cost_gold est un bon prix.', '', '', '', '¿Quién ha dicho que soy un rata? %item_formatted_link por %cost_gold es una ganga.', '', ''),
('suggest_sell', 'I am selling %item_formatted_link? Just %cost_gold.', 0, 0, '', 'Je vends %item_formatted_link ? Juste %cost_gold', '', '', '', '¿Vendo %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'LFG for farming. You can still buy %item_formatted_link I have for %cost_gold.', 0, 0, '', 'Cherche un groupe pour farming. Vous pouvez toujours acheter %item_formatted_link que j’ai pour %cost_gold', '', '', '', 'LFG para farmear. Todavía puedes comprar %item_formatted_link que tengo por %cost_gold.', '', ''),
('suggest_sell', 'Sold almost everything today. Still have %item_formatted_link for %cost_gold.', 0, 0, '', 'Vendu presque tout aujourd’hui. Vous avez encore %item_formatted_link pour %cost_gold', '', '', '', 'Se vendió casi todo hoy. Todavía tengo %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'What use for trade chat? Of course to sell %item_formatted_link for %cost_gold.', 0, 0, '', 'A quoi sert le tchat commercial? Bien sûr pour vendre %item_formatted_link pour %cost_gold', '', '', '', '¿De qué sirve el chat comercial? Por supuesto, para vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Can anyone beat the price of %cost_gold for %item_formatted_link?', 0, 0, '', 'Quelqu’un peut-il battre le prix de %cost_gold pour %item_formatted_link ?', '', '', '', '¿Alguien puede superar el precio de %cost_gold por %item_formatted_link?', '', ''),
('suggest_sell', 'Wanna stop trade chat? Just buy %item_formatted_link? For %cost_gold!', 0, 0, '', 'Vous voulez arrêter le chat commercial? Vous venez d’acheter %item_formatted_link ? Pour %cost_gold !', '', '', '', '¿Quieres detener el chat comercial? ¿Solo compra %item_formatted_link? ¡Por el oro!', '', ''),
('suggest_sell', 'Everybody spams in trade chat. Me too - %cost_gold for %item_formatted_link!', 0, 0, '', 'Tout le monde spamme dans le chat commercial. Moi aussi - %cost_gold pour %item_formatted_link !', '', '', '', 'Todo el mundo envía spam en el chat y yo también! ¡%item_formatted_link por %cost_gold!', '', ''),
('suggest_sell', 'Is %item_formatted_link any use? Just selling it for %cost_gold.', 0, 0, '', '%item_formatted_link est-il utile? Je le vends juste pour %cost_gold', '', '', '', '¿Es %item_formatted_link útil? Solo lo vendo por %cost_gold.', '', ''),
('suggest_sell', 'I have %item_formatted_link ready to sell you for %cost_gold.', 0, 0, '', 'j’ai %item_formatted_link prêt à vous vendre pour %cost_gold', '', '', '', 'Tengo %item_formatted_link listo a la venta por %cost_gold.', '', ''),
('suggest_sell', 'Did nothing yesterday but have got %item_formatted_link. Selling it for %cost_gold.', 0, 0, '', 'Je n’ai rien fait hier mais j’ai %item_formatted_link Je le vends pour %cost_gold', '', '', '', 'No hice nada ayer pero consegu''i %item_formatted_link. Lo vendo por %cost_gold.', '', ''),
('suggest_sell', 'Farmed yesterday and got %item_formatted_link. Anyone wtb for %cost_gold?', 0, 0, '', 'Farmé hier et obtenu %item_formatted_link Quelqu’un cherche à acheter pour %cost_gold ?', '', '', '', 'Ayer farmee y conseguí mucho %item_formatted_link. ¿Alguien lo quiere comprar por %cost_gold?', '', ''),
('suggest_sell', 'Bought %item_formatted_link yesterday. Anyone needs it for %cost_gold?', 0, 0, '', 'Acheté %item_formatted_link hier. Quelqu’un en a besoin pour %cost_gold ?', '', '', '', 'Ayer compré %item_formatted_link. ¿Alguien lo quiere por %cost_gold?', '', ''),
('suggest_sell', 'Who asked for %item_formatted_link? The price is the same - %cost_gold.', 0, 0, '', 'Qui a demandé %item_formatted_link ? Le prix est le même - %cost_gold', '', '', '', '¿Quién pidió %item_formatted_link? El precio es el mismo: %cost_gold.', '', ''),
('suggest_sell', 'I sill have %item_formatted_link. WTB for %cost_gold?', 0, 0, '', 'j’ai toujours %item_formatted_link. Cherche à acheter pour %cost_gold ?', '', '', '', 'Todavía tengo %item_formatted_link. ¿Alguien lo compra por %cost_gold?', '', ''),
('suggest_sell', 'I used to have more than %item_formatted_link. Now needs to sell it for %cost_gold.', 0, 0, '', 'j’avais plus de %item_formatted_link Il faut maintenant le vendre pour %cost_gold', '', '', '', 'Solía tener más de %item_formatted_link. Ahora necesito venderlo por %cost_gold.', '', ''),
('suggest_sell', 'I wish I have more than %item_formatted_link. You could buy it for %cost_gold anyways.', 0, 0, '', 'j’aimerais avoir plus de %item_formatted_link . Vous pouvez l’acheter pour %cost_gold de toute façon.', '', '', '', 'Desearía tener más de %item_formatted_link. Podrías comprarlo por %cost_gold de todos modos.', '', ''),
('suggest_sell', 'What use for your gold? To buy my %item_formatted_link for %cost_gold.', 0, 0, '', 'A quoi sert votre or ? Pour acheter mon %item_formatted_link pour %cost_gold', '', '', '', '¿De qué sirve tu oro? Para comprar mi %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Please spare some gold for me. You can buy %item_formatted_link for %cost_gold.', 0, 0, '', 'S''il vous plaît, épargnez-moi de l’or. Vous pouvez acheter %item_formatted_link pour %cost_gold', '', '', '', 'Por favor, dame algo de oro. Puedes comprar %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Is %cost_gold is a good price for %item_formatted_link?', 0, 0, '', '%cost_gold est-il un bon prix pour %item_formatted_link ?', '', '', '', '¿%cost_gold es un buen precio para %item_formatted_link?', '', ''),
('suggest_sell', 'Just bought yesterday %item_formatted_links, but do not need it anymore. Anyone wants for %cost_gold?', 0, 0, '', 'Je viens d’acheter hier %item_formatted_links , mais je n’en ai plus besoin. Quelqu’un veut %cost_gold ?', '', '', '', 'Ayer compré %item_formatted_link, pero ya no los necesito. ¿Alguien lo quiere por %cost_gold?', '', ''),
('suggest_sell', 'I am going to post %item_formatted_link on the AH but you can buy it now cheaper just for %cost_gold.', 0, 0, '', 'Je vais poster %item_formatted_link à l’hôtel des ventes mais vous pouvez l’acheter maintenant moins cher juste pour %cost_gold', '', '', '', 'Voy a poner %item_formatted_link en la casa de subastas pero puedes comprarlo ahora más barato solo por %cost_gold.', '', ''),
('suggest_sell', 'Why the #!@ have I bought %item_formatted_link? Anyone needs it for %cost_gold?', 0, 0, '', 'Pourquoi ai-je acheté le #!@ %item_formatted_link ? Quelqu’un en a besoin pour %cost_gold ?', '', '', '', '¿Por qué #!@ he comprado %item_formatted_link? ¿Alguien lo necesita por %cost_gold?', '', ''),
('suggest_sell', 'WTS %item_formatted_link for %cost_gold.', 0, 0, '', 'Cherche à vendre %item_formatted_link pour %cost_gold', '', '', '', 'Quiero vender %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', 'Who wants %item_formatted_link for %cost_gold?', 0, 0, '', 'Qui veut %item_formatted_link pour %cost_gold ?', '', '', '', '¿Quién quiere %item_formatted_link por %cost_gold?', '', ''),
('suggest_sell', 'Anyone wants %item_formatted_link? Only %cost_gold.', 0, 0, '', 'Quelqu’un veut %item_formatted_link ? Seulement %cost_gold', '', '', '', '¿Alguien quiere %item_formatted_link? Solo %cost_gold.', '', ''),
('suggest_sell', 'Just %cost_gold for %item_formatted_link!', 0, 0, '', 'Seulement %cost_gold pour %item_formatted_link!', '', '', '', '¡Solo %cost_gold para %item_formatted_link!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold.', 0, 0, '', 'Vendre %item_formatted_link pour %cost_gold', '', '', '', 'Vendo %item_formatted_link por %cost_gold.', '', ''),
('suggest_sell', '%item_formatted_link is yours just for %cost_gold!', 0, 0, '', '%item_formatted_link est à vous juste pour %cost_gold !', '', '', '', '%item_formatted_link es tuyo solo por %cost_gold!', '', ''),
('suggest_sell', 'WTS %item_formatted_link for %cost_gold. DM if interested.', 0, 0, '', 'Cherche à vendre %item_formatted_link pour %cost_gold. MP si intéressé.', '', '', '', 'Quiero vender %item_formatted_link por %cost_gold. Envíame un mensaje si te interesa.', '', ''),
('suggest_sell', 'Need to sell %item_formatted_link, %cost_gold or best offer.', 0, 0, '', 'Je dois vendre %item_formatted_link, %cost_gold ou meilleure offre.', '', '', '', 'Necesito vender %item_formatted_link, %cost_gold o la mejor oferta.', '', ''),
('suggest_sell', '%item_formatted_link available for %cost_gold. Get it now!', 0, 0, '', '%item_formatted_link disponible pour %cost_gold. Obtenez-le maintenant !', '', '', '', '%item_formatted_link disponible por %cost_gold. ¡Consíguelo ahora!', '', ''),
('suggest_sell', 'Got %item_formatted_link, asking %cost_gold.', 0, 0, '', 'j’ai %item_formatted_link, demande %cost_gold.', '', '', '', 'Tengo %item_formatted_link, pido %cost_gold.', '', ''),
('suggest_sell', 'Selling %item_formatted_link, only %cost_gold left.', 0, 0, '', 'Vente de %item_formatted_link, il ne reste plus que %cost_gold.', '', '', '', 'Vendo %item_formatted_link, solo queda %cost_gold.', '', ''),
('suggest_sell', 'Anyone want %item_formatted_link for %cost_gold? DM me!', 0, 0, '', 'Quelqu’un veut %item_formatted_link pour %cost_gold ? MP moi !', '', '', '', '¿Alguien quiere %item_formatted_link por %cost_gold? ¡Envíame un mensaje!', '', ''),
('suggest_sell', 'Selling %item_formatted_link at %cost_gold, grab it while you can!', 0, 0, '', 'Je vends %item_formatted_link à %cost_gold, attrape-le tant que tu peux !', '', '', '', 'Vendo %item_formatted_link a %cost_gold, ¡agárralo mientras puedas!', '', ''),
('suggest_sell', '%item_formatted_link for %cost_gold, hurry up before it’s gone!', 0, 0, '', '%item_formatted_link pour %cost_gold, dépêche-toi avant qu’il ne parte !', '', '', '', '%item_formatted_link por %cost_gold, ¡apúrate antes de que se vaya!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, who’s in?', 0, 0, '', 'Je vends %item_formatted_link pour %cost_gold, qui est intéressé ?', '', '', '', 'Vendo %item_formatted_link por %cost_gold, ¿quién está dentro?', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, first come, first serve!', 0, 0, '', 'Je vends %item_formatted_link pour %cost_gold, premier arrivé, premier servi !', '', '', '', 'Vendo %item_formatted_link por %cost_gold, ¡el primero que llega se lo lleva!', '', ''),
('suggest_sell', 'Only %cost_gold for %item_formatted_link, limited time offer!', 0, 0, '', 'Seulement %cost_gold pour %item_formatted_link, offre limitée !', '', '', '', 'Solo %cost_gold para %item_formatted_link, ¡oferta por tiempo limitado!', '', ''),
('suggest_sell', '%item_formatted_link available for %cost_gold, hit me up if you need it!', 0, 0, '', '%item_formatted_link disponible pour %cost_gold, contactez-moi si vous en avez besoin !', '', '', '', '%item_formatted_link disponible por %cost_gold, ¡contáctame si lo necesitas!', '', ''),
('suggest_sell', 'Anyone interested in %item_formatted_link for %cost_gold?', 0, 0, '', 'Quelqu’un est intéressé par %item_formatted_link pour %cost_gold ?', '', '', '', '¿Alguien interesado en %item_formatted_link por %cost_gold?', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, need to clear inventory.', 0, 0, '', 'Vente de %item_formatted_link pour %cost_gold, besoin de vider l’inventaire.', '', '', '', 'Vendo %item_formatted_link por %cost_gold, necesito vaciar mi inventario.', '', ''),
('suggest_sell', 'Get %item_formatted_link for %cost_gold, it’s a great deal!', 0, 0, '', 'Procurez-vous %item_formatted_link pour %cost_gold, c''est une bonne affaire !', '', '', '', 'Consigue %item_formatted_link por %cost_gold, ¡es una gran oferta!', '', ''),
('suggest_sell', '%item_formatted_link for %cost_gold, get it while it’s hot!', 0, 0, '', '%item_formatted_link pour %cost_gold, obtenez-le tant qu’il est chaud !', '', '', '', '%item_formatted_link por %cost_gold, ¡cómpralo mientras esté caliente!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, don’t miss out!', 0, 0, '', 'Vente de %item_formatted_link pour %cost_gold, ne manquez pas ça !', '', '', '', 'Vendo %item_formatted_link por %cost_gold, ¡no te lo pierdas!', '', ''),
('suggest_sell', 'Only %cost_gold for %item_formatted_link, hit me up for details!', 0, 0, '', 'Seulement %cost_gold pour %item_formatted_link, contactez-moi pour plus de détails !', '', '', '', 'Solo %cost_gold para %item_formatted_link, ¡contáctame para más detalles!', '', ''),
('suggest_sell', '%item_formatted_link is available for %cost_gold, let’s trade!', 0, 0, '', '%item_formatted_link est disponible pour %cost_gold, faisons un échange !', '', '', '', '%item_formatted_link está disponible por %cost_gold, ¡hagamos un intercambio!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, message me for more info.', 0, 0, '', 'Vente de %item_formatted_link pour %cost_gold, envoyez-moi un message pour plus d’informations.', '', '', '', 'Vendo %item_formatted_link por %cost_gold, envíame un mensaje para más información.', '', ''),
('suggest_sell', 'Looking to sell %item_formatted_link, %cost_gold takes it!', 0, 0, '', 'Je cherche à vendre %item_formatted_link, %cost_gold suffit !', '', '', '', 'Busco vender %item_formatted_link, ¡%cost_gold lo consigue!', '', ''),
('suggest_sell', 'I’m selling %item_formatted_link, %cost_gold only.', 0, 0, '', 'Je vends %item_formatted_link, seulement %cost_gold.', '', '', '', 'Estoy vendiendo %item_formatted_link, solo %cost_gold.', '', ''),
('suggest_sell', 'Got %item_formatted_link for %cost_gold, let’s make a deal!', 0, 0, '', 'j’ai %item_formatted_link pour %cost_gold, faisons une affaire !', '', '', '', 'Tengo %item_formatted_link por %cost_gold, ¡hagamos un trato!', '', ''),
('suggest_sell', 'Selling %item_formatted_link, %cost_gold takes it home!', 0, 0, '', 'Vente de %item_formatted_link, %cost_gold suffit pour l’emporter !', '', '', '', 'Vendo %item_formatted_link, ¡%cost_gold lo lleva a casa!', '', ''),
('suggest_sell', 'WTS %item_formatted_link for %cost_gold, anyone interested?', 0, 0, '', 'Je vends %item_formatted_link pour %cost_gold, quelqu’un est intéressé ?', '', '', '', 'Vendo %item_formatted_link por %cost_gold, ¿alguien interesado?', '', ''),
('suggest_sell', 'Need to sell %item_formatted_link fast, %cost_gold.', 0, 0, '', 'Je dois vendre %item_formatted_link rapidement, %cost_gold.', '', '', '', 'Necesito vender %item_formatted_link rápido, %cost_gold.', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, best price around!', 0, 0, '', 'Vente de %item_formatted_link pour %cost_gold, meilleur prix ici !', '', '', '', 'Vendo %item_formatted_link por %cost_gold, ¡el mejor precio!', '', ''),
('suggest_sell', '%item_formatted_link for %cost_gold, who’s buying?', 0, 0, '', '%item_formatted_link pour %cost_gold, qui achète ?', '', '', '', '%item_formatted_link por %cost_gold, ¿quién compra?', '', ''),
('suggest_sell', '%item_formatted_link for %cost_gold, let’s make it happen!', 0, 0, '', '%item_formatted_link pour %cost_gold, faisons-le !', '', '', '', '%item_formatted_link por %cost_gold, ¡hagámoslo realidad!', '', ''),
('suggest_sell', 'Selling %item_formatted_link for %cost_gold, grab it before it’s gone!', 0, 0, '', 'Vente de %item_formatted_link pour %cost_gold, attrapez-le avant qu’il ne parte !', '', '', '', 'Vendo %item_formatted_link por %cost_gold, ¡agárralo antes de que se vaya!', '', ''),
('suggest_sell', 'Want %item_formatted_link? %cost_gold gets you one!', 0, 0, '', 'Tu veux %item_formatted_link ? %cost_gold et il est à toi !', '', '', '', '¿Quieres %item_formatted_link? ¡%cost_gold te consigue uno!', '', ''),
('suggest_sell', '%item_formatted_link for %cost_gold, quick sale!', 0, 0, '', '%item_formatted_link pour %cost_gold, vente rapide !', '', '', '', '%item_formatted_link por %cost_gold, ¡venta rápida!', '', ''),


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
('response_lfg_quests_channel', 'I have %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I also have %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I also have %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I also have %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I also have %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I am up for %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I am up for %quest_links, I am %my_role', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I am up for %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', '%other_name, I am up for %quest_links, I am %my_role', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone need %quest_links? I’m in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for more for %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Need some help with %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone want to do %quest_links with me in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I have %quest_links, looking for group in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Who needs %quest_links in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Up for %quest_links in %zone_name, who’s in?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', '%my_role looking for group to complete %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I need %quest_links, anyone available in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for more people for %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I’m doing %quest_links in %zone_name, need more help.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Any %my_role interested in %quest_links in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Help needed for %quest_links in %zone_name, looking for more.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I have %quest_links, %my_role in %zone_name, who’s in?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Who wants to do %quest_links in %zone_name? I am %my_role.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone doing %quest_links right now? I’m in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', '%quest_links needed in %zone_name, any takers?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Up for %quest_links in %zone_name, looking for group.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I need %quest_links and some company, %zone_name anyone?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Who wants to join me in %zone_name for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Any %my_role available for %quest_links in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I have %quest_links and I am %my_role, need more people in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for people to do %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I am %my_role and have %quest_links, looking for group in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Need help with %quest_links in %zone_name, any %my_role available?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Any %my_role interested in completing %quest_links with me in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Let’s complete %quest_links together in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I am in %zone_name and have %quest_links, who wants to join?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for more %my_role to complete %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I have %quest_links and I need %my_role in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone up for %quest_links? I am %my_role in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for a group to do %quest_links in %zone_name, %my_role needed.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone need help with %quest_links in %zone_name? I’m %my_role.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I have %quest_links to complete, need %my_role in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I’m in %zone_name looking for %quest_links help, who’s in?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for a group to finish %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone looking for %quest_links in %zone_name? I’m %my_role.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Let’s do %quest_links together! %zone_name is waiting.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Need a group for %quest_links, %zone_name, anyone interested?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Who wants to join for %quest_links? I’m in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I’m in %zone_name, looking for people to complete %quest_links with.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Need a hand with %quest_links in %zone_name, any %my_role?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for group to finish %quest_links, %zone_name is the place to be.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone want to help with %quest_links in %zone_name? I’m %my_role.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for %my_role to join me for %quest_links in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I’ve got %quest_links, %my_role needed for %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking to finish %quest_links in %zone_name, need more people.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I need help with %quest_links, who’s around in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Need more people to complete %quest_links, %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for group in %zone_name to finish %quest_links.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Who wants to do %quest_links in %zone_name? I’m ready to go.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Anyone want to join me for %quest_links? I’m %my_role in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for %my_role to do %quest_links with in %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I’m in %zone_name, anyone need help with %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Need more for %quest_links in %zone_name, any takers?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'I need help with %quest_links in %zone_name, who’s up for it?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_channel', 'Looking for more to finish %quest_links, %zone_name.', 0, 0, '', '', '', '', '', '', '', ''),


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
('response_lfg_quests_whisper', 'Hey, I’m up for %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I could do %quest_links with you', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I also have %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, I’m up for %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, I could do %quest_links with you', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, I also have %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'wanna group up for %quest_links ?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I am up for %quest_links, I am in %zone_name right now', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I am up for %quest_links, I am %my_role', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I’m down for %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I can help with %quest_links if you need', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Let’s do %quest_links together!', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links to do too', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’m available for %quest_links if you want', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’m down for %quest_links, just let me know', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Looking for %quest_links? I’m in %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I need %quest_links too, let’s go', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links, want to join me?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I could use %quest_links, let’s do this', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, want to join for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I can help with %quest_links if you need', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, up for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Want to group up for %quest_links %other_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Let’s do %quest_links together, %other_name', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’m %my_role and ready for %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I can join for %quest_links, %my_role here', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’m up for %quest_links, %my_role!', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Looking to do %quest_links with a %my_role', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Anyone need help with %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Who else is up for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Anyone down for %quest_links in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links, %my_role on the way', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I need %quest_links too, who’s in?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve been grinding %quest_links, need a hand?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’m ready for %quest_links, anyone else?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Can anyone join me for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Looking for help with %quest_links, %my_role here', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I can help out with %quest_links, anyone need me?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Anyone else doing %quest_links? Let’s go', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Anyone need %quest_links help? %my_role here', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Who’s up for some %quest_links in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I can join for %quest_links if you want', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Looking for a %my_role to do %quest_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Any %my_role up for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links, anyone want to join me?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey %other_name, up for some %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Let’s team up for %quest_links, %other_name', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I need %quest_links, anyone else want to do it?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I need %quest_links too, anyone else around?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Looking for %quest_links in %zone_name, who’s with me?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Anyone available for %quest_links?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Hey, I could join you for %quest_links!', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links too, let’s team up', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links to do, want to join?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Any %my_role up for %quest_links in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Let’s finish %quest_links together, %other_name', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’m up for %quest_links, anyone else want to join?', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'Who else is doing %quest_links? I’m in', 0, 0, '', '', '', '', '', '', '', ''),
('response_lfg_quests_whisper', 'I’ve got %quest_links in %zone_name, need help?', 0, 0, '', '', '', '', '', '', '', ''),


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
('response_wtb_items_channel', '%other_name, I could sell you %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_channel', 'I could potentially sell %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_channel', 'Think I could sell %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_channel', '%other_name, I could potentially sell %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_channel', '%other_name, Think I could sell %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),




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
('response_wtb_items_whisper', 'I could sell you %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I got %formatted_item_links for sale', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I could potentially sell %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I could sell you %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I got %formatted_item_links for sale', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I could potentially sell %formatted_item_links', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, are you interested?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Looking to sell %formatted_item_links, what’s your offer?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can sell you %formatted_item_links for a fair price', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links in stock, how much are you offering?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’m selling %formatted_item_links for %cost_gold, let me know if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Got %formatted_item_links for sale, what are you willing to pay?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links, how does %cost_gold sound for you?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links for %cost_gold, message me if you want it', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, hit me up if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can part with %formatted_item_links if you make the right offer', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Looking to get rid of %formatted_item_links, hit me up', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Got %formatted_item_links for %cost_gold, are you buying?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, let’s negotiate', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I have %formatted_item_links for %cost_gold', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can hook you up with %formatted_item_links, just name your price', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links available, let me know if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, let’s do business', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, feel free to make an offer', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links, send me a message if you want to buy', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can offer you %formatted_item_links for %cost_gold', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links for sale, message me for the price', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’m selling %formatted_item_links, get it while it’s hot', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, you won’t find a better deal', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links for sale, how much are you willing to pay?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, feel free to message me for the price', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Interested in %formatted_item_links? I’ve got them for sale', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, PM me if you want them', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links at %cost_gold, hit me up', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’m selling %formatted_item_links, make me an offer', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Got %formatted_item_links, message me for the details', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I’ve got %formatted_item_links, hit me up if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, let’s make a deal', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, let me know if you want them', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, I’ll make you a good deal', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links, make me an offer and it’s yours', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, I’m open to offers', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, message me if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can sell you %formatted_item_links for a good price', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links for sale, I’ll consider any offers', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links for sale, message me for details', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links for sale, make me an offer', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, how much are you willing to pay?', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’m selling %formatted_item_links, message me for more info', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links, let me know if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, let’s talk numbers', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can sell you %formatted_item_links, just let me know what you’re offering', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Hey, I’ve got %formatted_item_links for sale, hit me up if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, PM me for the price', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, I’m open to offers', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, feel free to message me', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I can offer %formatted_item_links, just let me know if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, I can work with any reasonable offer', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, hit me up for more info', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links, make me an offer', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'Selling %formatted_item_links, shoot me a message if you’re interested', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I’ve got %formatted_item_links for sale, let’s talk business', 0, 0, '', '', '', '', '', '', '', ''),
('response_wtb_items_whisper', 'I have %formatted_item_links, let me know if you want them', 0, 0, '', '', '', '', '', '', '', ''),



-- /////////////////////////////
-- generic events

-- quest
('quest_accept', 'Quest accepted', 0, 0, '', '', '', '', '', 'Misión aceptada', '', 'Квест взят'),
('quest_remove', 'Quest removed', 0, 0, '', '', '', '', '', 'Misión eliminada', '', 'Квест отменен'),
('quest_cant_take', 'I can’t take this quest', 0, 0, '', '', '', '', '', 'No puedo tomar esta búsqueda', '', 'Я не могу взять этот квест'),
('quest_error_talk', 'I can’t talk with the quest giver', 0, 0, '', '', '', '', '', 'No puedo hablar con el propietario de la mision', '', ''),
('quest_error_completed', 'I have already completed %quest', 0, 0, '', '', '', '', '', 'Ya he completado la mision %quest', '', ''),
('quest_error_have_quest', 'I already have %quest', 0, 0, '', '', '', '', '', 'Ya tengo la mision %quest', '', ''),
('quest_error_cant_take', 'I can’t take %quest', 0, 0, '', '', '', '', '', 'No puedo aceptar la mision %quest', '', ''),
('quest_error_log_full', 'I can’t take %quest because my quest log is full', 0, 0, '', '', '', '', '', 'No puedo aceptar la mision %quest porque mi registro de misiones esta completo', '', ''),
('quest_error_bag_full', 'I can’t take %quest because my bag is full', 0, 0, '', '', '', '', '', 'No puedo aceptar la mision %quest porque mi inventario esta lleno', '', ''),
('quest_accepted', 'I have accepted %quest', 0, 0, '', '', '', '', '', 'He aceptado la mision %quest', '', ''),
('quest_status_incomplete', 'I have not completed the quest %quest', 0, 0, '', '', '', '', '', 'No he completado la mision %quest', '', ''),
('quest_status_available', 'Quest %quest available', 0, 0, '', '', '', '', '', 'Mision %quest disponible', '', ''),
('quest_status_failed', 'I have failed the quest %quest', 0, 0, '', '', '', '', '', 'He fallado la mision %quest', '', ''),
('quest_status_unable_to_complete', 'I am unable to turn in the quest %quest', 0, 0, '', '', '', '', '', 'No puedo entregar la mision %quest', '', ''),
('quest_status_completed', 'I have completed the quest %quest', 0, 0, '', '', '', '', '', 'He completado la mision %quest', '', ''),
('quest_status_complete_single_reward', 'I have completed the quest %quest and received %item', 0, 0, '', '', '', '', '', 'He completado la mision %quest y he recibido %item', '', ''),
('quest_status_complete_pick_reward', 'Which reward should I pick for completing the quest %quest?%rewards', 0, 0, '', '', '', '', '', '¿Que recompensa deberia escoger por completar la mision %quest?%rewards', '', ''),
('quest_choose_reward', 'Okay, I will pick %item as a reward', 0, 0, '', '', '', '', '', 'De acuerdo, voy a escoger %item de recompensa', '', ''),


-- stuff
('hello', 'Hello', 0, 0, '', '', '', '', '', 'Hola', '', 'Привет'),
('hello', 'Hello!', 0, 0, '', '', '', '', '', '¡Hola!', '', 'Привет!'),
('hello', 'Hi', 0, 0, '', '', '', '', '', 'Hola', '', 'Прив'),
('hello', 'Hi!', 0, 0, '', '', '', '', '', '¡Hola!', '', 'Прив!'),
('hello', 'Hello there!', 0, 0, '', '', '', '', '', '¡Hola!', '', 'Здарова!'),



('hello_follow', 'Hello, I follow you!', 0, 0, '', '', '', '', '', '¡Hola, te sigo!', '', 'Привет, иду за тобой!'),
('hello_follow', 'Hello, lead the way!', 0, 0, '', '', '', '', '', '¡Hola, guía el camino!', '', 'Привет, веди!'),
('hello_follow', 'Hi, lead the way!', 0, 0, '', '', '', '', '', '¡Hola, guía el camino!', '', 'Прив, веди!'),



('join_group', 'Hey %player, do you want join my group?', 0, 0, '', '', '', '', '', 'Oye %player, ¿Quieres unirte a mi grupo?', '', ''),



('join_raid', 'Hey %player, do you want join my group?', 0, 0, '', '', '', '', '', 'Oye %player, ¿Quieres unirte a mi banda?', '', ''),



('logout_cancel', 'Logout cancelled!', 0, 0, '', '', '', '', '', 'Cerrar sesión cancelado!', '', 'Логаут отменен!'),



('logout_start', 'I’m logging out!', 0, 0, '', '', '', '', '', '¡Me estoy desconectando!', '', 'Я выхожу из игры!'),



('goodbye', 'Goodbye!', 0, 0, '', '', '', '', '', '¡Adiós!', '', 'Пока!'),
('goodbye', 'Bye bye!', 0, 0, '', '', '', '', '', '¡Adiós!', '', 'Пока пока!'),
('goodbye', 'See you later!', 0, 0, '', '', '', '', '', '¡Hasta luego!', '', 'Увидимся позже!'),



-- /////////////////////////////
-- replies


-- replies 0
('reply', 'what was that %s?', 0, 0, '', 'c''était quoi ce %s?', '', '', '', '¿qué has dicho %s?', '', ''),
('reply', 'not sure I understand %s?', 0, 0, '', 'pas sûr de comprendre %s?', '', '', '', '¿no estoy seguro de entenderte %s?', '', ''),
('reply', 'uh... no clue what yer talkin bout', 0, 0, '', 'euh... aucune idée de quoi tu parles', '', '', '', 'uh... ni idea de lo que estás hablando', '', ''),
('reply', 'you talkin to me %s?', 0, 0, '', 'tu me parles %s?', '', '', '', '¿Me hablas a mí %s?', '', ''),
('reply', 'whaaaa?', 0, 0, '', 'quoiaaa?', '', '', '', '¿quéaaa?', '', ''),
('reply', 'huh?', 0, 0, '', 'hein?', '', '', '', '¿eh?', '', ''),
('reply', 'what?', 0, 0, '', 'Quoi?', '', '', '', '¿qué?', '', ''),
('reply', 'are you talking?', 0, 0, '', 'parles-tu?', '', '', '', '¿estás hablando?', '', ''),
('reply', 'whatever dude', 0, 0, '', 'peu importe mec', '', '', '', 'Menudo tio...', '', ''),
('reply', 'you lost me', 0, 0, '', 'tu m''as perdu', '', '', '', 'No entiendo ni papa', '', ''),
('reply', 'Bla bla bla...', 0, 0, '', 'Bla bla bla...', '', '', '', 'Bla bla bla...', '', ''),
('reply', 'What did you say, %s?', 0, 0, '', 'qu’as-tu dis, %s?', '', '', '', '¿Qué me estas contando %s?', '', ''),
('reply', 'Concentrate on the game, %s!', 0, 0, '', 'Concentres-toi sur le jeu, %s!', '', '', '', '¡Concéntrate en el juego, %s!', '', ''),
('reply', 'Chatting with you %s is so great! I always wanted to meet you', 0, 0, '', 'Discuter avec vous %s est tellement génial! j’ai toujours voulu te rencontrer', '', '', '', '¡%s, chatear contigo es genial! Siempre quise conocerte', '', ''),
('reply', 'These chat-messages are freaking me out! I feel like I know you all!', 0, 0, '', 'Ces messages du chat me font flipper! j’ai l’impression de vous connaître tous!', '', '', '', '¡Estos mensajes de chat me están asustando! ¿De que cojones estais hablando?', '', ''),
('reply', 'YEAH RIGHT! HAHA SURE!!!', 0, 0, '', 'OUI EN EFFET! AHAH BIEN SÛR!!!', '', '', '', '¡SÍ, CORRECTO! ¡¡¡JAJA SEGURO!!!', '', ''),
('reply', 'I believe you!!!', 0, 0, '', 'Je te crois!!!', '', '', '', '¡Si, claro!', '', ''),
('reply', 'OK, uhuh LOL', 0, 0, '', 'OK, euh LOL', '', '', '', 'OK, uhuh LOL', '', ''),
('reply', 'Why is everybody always saying the same things???', 0, 0, '', 'Pourquoi tout le monde dit toujours la même chose???', '', '', '', '¿Por qué todo el mundo siempre dice lo mismo?', '', ''),
('reply', 'Hey %s....oh nevermind!', 0, 0, '', 'Hé %s.... oh tant pis!', '', '', '', 'Oye %s... ¡oh, no importa!', '', ''),
('reply', 'What are you talking about %s', 0, 0, '', 'De quoi parlez-vous %s', '', '', '', 'De qué estás hablando %s', '', ''),
('reply', 'Who said that? I resemble that remark', 0, 0, '', 'Qui a dit ça? je ressemble à cette remarque', '', '', '', '¿Quién ha dicho eso? ¿Estais hablando de mi?', '', ''),
('reply', 'wtf are you all talking about', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'fr fr no cap on a stack', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'your not getting jack shit', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'swag', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'ty!', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'no', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'Yep', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'f', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '%s no shit xD', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'why is that', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'lmao', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'thought i should remain silent, was getting confused from chat again', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'i can get realy jealous', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '%s you can’t hear the dripping sarcasm in my text', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'he said no homo, it’s fine', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'dwarf moment', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'Yes %s', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'interesting...', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'lol', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '%s fuck you man :D', 0, 0, '', '', '', '', '', '', '', ''),
('reply', ':^)', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'ty', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '%s well said', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'yay', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'yeah', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'ooooooh', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'hmm', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'yeah right', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'you made me gag wtf', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'hot', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'hoes mad', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'what have you been eating %s', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'wtf', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'i will attempt to understand that comment', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '*confused*', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'fuck yea', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '0/10 would not read again', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '10/10 would read again', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '6/10 would read', 0, 0, '', '', '', '', '', '', '', ''),
('reply', '7/10 would', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'based', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'oh yeah maybe', 0, 0, '', '', '', '', '', '', '', ''),
('reply', 'yeah so what', 0, 0, '', '', '', '', '', '', '', ''),







-- replies 1
('reply', 'hey %s i havent forgotten you', 0, 1, '', 'salut %s je ne t''ai pas oublié', '', '', '', 'hey %s no te he olvidado', '', ''),
('reply', 'you piss me off %s', 0, 1, '', 'tu me fais chier %s', '', '', '', 'me cabreas %s', '', ''),
('reply', 'im gunna get you this time %s', 0, 1, '', 'je vais t''avoir cette fois %s', '', '', '', 'voy a atraparte esta vez %s', '', ''),
('reply', 'better watch your back %s', 0, 1, '', 'mieux vaut surveiller vos arrières %s', '', '', '', 'mejor cuida tu espalda %s', '', ''),
('reply', 'i did not like last round so much', 0, 1, '', 'je n’ai pas tellement aimé le dernier tour', '', '', '', 'no me gustó tanto la última ronda', '', ''),
('reply', 'i sucked last round thanks to %s', 0, 1, '', 'j’ai merdé le dernier tour grâce à %s', '', '', '', 'chupé la última ronda gracias a %s', '', ''),
('reply', 'prepare to die %s', 0, 1, '', 'préparez-vous à mourir %s', '', '', '', 'prepárate para morir %s', '', ''),
('reply', 'dont appreciate you killin me %s', 0, 1, '', 'j’apprécie pas que tu me tues %s', '', '', '', 'no aprecio que me mates %s', '', ''),
('reply', '%s, i hate you', 0, 1, '', '%s, je te déteste', '', '', '', '%s, te odio', '', ''),
('reply', 'grrrrrr, ill get you this time %s', 0, 1, '', 'grrrrrr, je vais t''avoir cette fois %s', '', '', '', 'grrrrrr, te libraste esta vez %s', '', ''),
('reply', 'well fuck you', 0, 1, '', 'eh bien va te faire foutre', '', '', '', 'vamos a la mierda', '', ''),
('reply', '%s i’ll vomit into your fkin mouth', 0, 1, '', '', '', '', '', '', '', ''),
('reply', 'dont fucking judge me', 0, 1, '', '', '', '', '', '', '', ''),
('reply', 'Yo momma so fat, she can’t even fit through the Dark Portal', 0, 1, '', '', '', '', '', '', '', ''),



-- replies 2
('reply', 'wtf', 0, 2, '', 'wtf', '', '', '', 'wtf', '', ''),
('reply', 'wtf??', 0, 2, '', 'wtf??', '', '', '', 'wtf??', '', ''),
('reply', 'low life', 0, 2, '', 'faible durée de vie', '', '', '', 'low life', '', ''),
('reply', 'wth', 0, 2, '', 'wth', '', '', '', 'wth', '', ''),
('reply', 'sucked', 0, 2, '', 'Ça craint', '', '', '', 'chupate esa', '', ''),
('reply', 'REMATCH!!! im taking him down', 0, 2, '', 'REMATCH!!! je le descends', '', '', '', '¡¡REVANCHA!!! lo estoy derribando', '', ''),
('reply', 'pathetic, i got killed by %s', 0, 2, '', 'pathétique, j’ai été tué par %s', '', '', '', 'patético, me mató %s', '', ''),
('reply', 'ok I’m done', 0, 2, '', '', '', '', '', '', '', ''),



-- replies 3
('reply', 'hehe, i nailed %s?', 0, 3, '', 'hehe, j’ai cloué %s?', '', '', '', 'jeje, ¿acerté a %s?', '', ''),
('reply', 'that was too easy, killin %s', 0, 3, '', 'c''était trop facile, de tuer %s', '', '', '', 'fue demasiado fácil matar a %s', '', ''),
('reply', 'gotcha mofo', 0, 3, '', 'je t''ai eu enfoiré', '', '', '', 'te pille hijo de perra', '', ''),
('reply', 'ha ha', 0, 3, '', 'ha ha', '', '', '', 'ja, ja', '', ''),
('reply', 'loser', 0, 3, '', 'loser', '', '', '', 'loser', '', ''),
('reply', 'i killed %s and yer all next dudes', 0, 3, '', 'j’ai tué %s et vous tous les prochains mecs', '', '', '', 'yo maté a %s y a todos los siguientes tipos', '', ''),
('reply', 'oh yeah i owned him', 0, 3, '', 'oh ouais je l’ai possédé', '', '', '', 'oh, sí, lo tenía', '', ''),
('reply', 'im a killin machine', 0, 3, '', 'je suis une machine à tuer', '', '', '', 'soy una máquina de matar', '', ''),
('reply', '%s,  this reminds me of a Slayer song...all this bloodshed', 0, 3, '', '%s, ça me rappelle une chanson de Slayer... tout ce bain de sang', '', '', '', '%s, esto me recuerda a una canción de Slayer... todo este derramamiento de sangre', '', ''),
('reply', 'sorry, %s. can we do the scene again?', 0, 3, '', 'désolé, %s. peut-on refaire la scène?', '', '', '', 'lo siento, %s. ¿Podemos hacer la escena de nuevo?', '', ''),
('reply', 'so....how do you like being worm food %s???', 0, 3, '', 'alors... comment aimez-vous être de la nourriture pour vers %s???', '', '', '', 'así que... ¿cómo te gusta ser comida de gusanos %s???', '', ''),
('reply', 'yer supposed to be dead, %s its part of the game!!!!!', 0, 3, '', 'tu es censé être mort, %s ça fait partie du jeu!!!!!', '', '', '', '¡¡Se supone que estás muerto, %s es parte del juego!!!!!', '', ''),
('reply', 'sorry, %s. that looked as good as an Andy Worhol painting!', 0, 3, '', 'désolé, %s. ça avait l’air aussi bien qu’une peinture d’Andy Warhol!', '', '', '', 'lo siento, %s. ¡Eso se veía tan bien como una pintura de Andy Worhol!', '', ''),
('reply', '%s, ill use the rubber bullets next time !', 0, 3, '', '%s, j’utiliserai pas les balles en caoutchouc la prochaine fois!', '', '', '', '%s, ¡usaré las balas de goma la próxima vez!', '', ''),
('reply', 'whatsamatter, %s?? lose your head? hahaha gotta keep cool!!', 0, 3, '', 'qu’importe, %s ?? tu perds la tête? hahaha faut rester cool!!', '', '', '', '¿qué pasa, %s?? ¿has perdido la cabeza? jajaja tengo que mantener la calma!!', '', ''),
('reply', 'i had to do it, %s. You understand. The Director said so!!', 0, 3, '', 'je devais le faire, %s. Tu comprends. Le directeur l’a dit!!', '', '', '', 'Tenía que hacerlo, %s. Entiendelo... ¡¡Dios me lo ordenó!!', '', ''),
('reply', 'hey %s.......MUAHAHAHAHAHAHAHAHAHAHA', 0, 3, '', 'Salut %s.......MUAHAHAHAHAHAHAHAHAHAHA', '', '', '', 'hey %s.......MUAHAHAHAHAHAHAHAHAHAHA', '', ''),
('reply', '%s, i enjoyed that one!! Lets play it again Sam', 0, 3, '', '%s, j’ai adoré celui-là !! Jouons à nouveau Sam', '', '', '', '%s, ¡disfruté ese! Juguemos de nuevo Sam', '', ''),
('reply', 'hey, %s! ju can start callin me scarface.. ju piece of CHIT!!!!', 0, 3, '', 'Hé, %s ! ju peut commencer à m''appeler Scarface .. ju merde!!!!', '', '', '', '¡oye, %s! ¡Puedes empezar a llamarme caracortada... ¡¡¡un trozo de CHIT!!!', '', ''),
('reply', 'are you talking to me %s??', 0, 3, '', 'tu me parles %s ??', '', '', '', '¿me estás hablando a mí %s?', '', ''),
('reply', '%s get it right this time, dont stand in front of my bullets.', 0, 3, '', '%s réussis cette fois, ne te tiens pas devant mes balles.', '', '', '', '%s hazlo bien esta vez, no te pares frente a mis balas.', '', ''),
('reply', '%s, what are you laying around for??? hehe', 0, 3, '', '%s, pourquoi traînes-tu??? hé hé', '', '', '', '%s, ¿para qué estás tirado? jeje', '', ''),
('reply', 'laughed real hard', 0, 3, '', '', '', '', '', '', '', ''),



-- replies 4
('reply', 'hi %s', 0, 4, '', 'salut %s', '', '', '', 'hola %s', '', ''),
('reply', 'oh, hi %s', 0, 4, '', 'oh, salut %s', '', '', '', 'oh, hola %s', '', ''),
('reply', 'wazzup %s!!!', 0, 4, '', 'Quoi de neuf %s!!!', '', '', '', 'wazzup %s!!!', '', ''),
('reply', 'hi', 0, 4, '', 'salut', '', '', '', 'hi', '', ''),
('reply', 'wazzup', 0, 4, '', 'wazzup', '', '', '', 'wazzup', '', ''),
('reply', 'hello %s', 0, 4, '', 'bonjour %s', '', '', '', 'hola %s', '', ''),
('reply', 'hi %s, do i know you?', 0, 4, '', 'Salut %s, est-ce que je te connais ?', '', '', '', 'hola %s, ¿te conozco?', '', ''),
('reply', 'hey %s', 0, 4, '', 'salut %s', '', '', '', 'hey %s', '', ''),
('reply', 'hai %s', 0, 4, '', 'ha %s', '', '', '', 'hai %s', '', ''),
('reply', 'wth', 0, 6, '', 'wth', '', '', '', 'wth', '', ''),
('reply', 'wtf', 0, 6, '', 'wtf', '', '', '', 'que cojones!', '', ''),
('reply', 'this is bs', 0, 6, '', 'c''est bs', '', '', '', 'esto es una mierda', '', ''),
('reply', 'admin', 0, 6, '', 'admin', '', '', '', 'admin', '', ''),
('reply', 'hey %s quit abusing your admin', 0, 6, '', 'hé %s arrête d’abuser de ton admin', '', '', '', 'hey %s deja de abusar de tu poder de administrador', '', ''),
('reply', 'leave me alone admin!', 0, 6, '', 'laissez-moi tranquille admin!', '', '', '', '¡déjame en paz administrador!', '', ''),
('reply', 'you suck admin', 0, 6, '', 'tu es nul admin', '', '', '', 'apestas administrador', '', ''),
('reply', 'thats my name, what you want %s', 0, 5, '', 'c''est mon nom, qu’est-ce que tu veux %s', '', '', '', 'ese es mi nombre, ¿que quieres %s?', '', ''),
('reply', 'yes???', 0, 5, '', 'Oui???', '', '', '', 'sí???', '', ''),
('reply', 'uh... what', 0, 5, '', 'euh ...quoi', '', '', '', 'uh... qué', '', ''),
('reply', 'you talkin to me %s?', 0, 5, '', 'tu me parles %s ?', '', '', '', '¿Me hablas a mí %s?', '', ''),





-- taunts
('taunt', 'I have puppies under my armor!', 0, 0, '', 'j’ai des chiots sous mon armure !', '', '', '', '¡Tengo cachorros bajo mi armadura!', '', ''),
('taunt', 'Bite me, <target>!', 0, 0, '', 'Mords moi, <target>!', '', '', '', '¡Muérdeme, <target>!', '', ''),
('taunt', 'Hey <target>! Guess what your mom said last night!', 0, 0, '', 'Hey <target>! Devine ce que ta mère a dit hier soir!', '', '', '', '¡Oye, <target>! ¡Adivina lo que me dijo tu madre anoche!', '', ''),
('taunt', '<target>, you’re so ugly you couldn’t score in a monkey whorehouse with a bag of bananas!', 0, 0, '', '<target>, tu es si moche que tu ne pourrais pas marquer dans un bordel de singes avec un sac de bananes!', '', '', '', '<target>, ¡eres tan feo que no podrías anotar en un burdel de monos con una bolsa de plátanos!', '', ''),
('taunt', 'Shut up <target>, you’ll never be the man your mother is!!', 0, 0, '', 'Tais-toi <target>, tu ne seras jamais l’homme qu’est ta mère!!', '', '', '', 'Cállate <target>, ¡nunca serás el hombre que es tu madre!', '', ''),
('taunt', 'Your mother was a hampster and your father smelt of elderberries!!!!', 0, 0, '', 'Ta mère était un hamster et ton père sentait le sureau!!!!', '', '', '', 'Tu madre era un hámster y tu padre olía a bayas de saúco!!!!', '', ''),
('taunt', 'I don’t want to talk to you no more, you empty headed animal food trough wiper!!!', 0, 0, '', 'Je ne veux plus te parler, espèce d’essuyeur d’abreuvoirs pour animaux tête vide!!!', '', '', '', '¡¡No quiero hablar más contigo, limpiaparabrisas de cabeza vacía!', '', ''),
('taunt', 'I fart in your general direction!!!', 0, 0, '', 'Je pète dans votre direction générale!!!', '', '', '', '¡¡Me tiro un pedo en tu dirección general!!!', '', ''),
('taunt', 'Go and boil your bottom, you son of a silly person!!!', 0, 0, '', 'Va te faire cuire le cul, fils d’idiot!!!', '', '', '', '¡Ve y hierve tu trasero, hijo de una persona tonta!', '', ''),
('taunt', 'What are you going to do <target>, bleed on me? HAVE AT YOU!', 0, 0, '', 'qu’est ce que tu vas faire <target>, saigner sur moi? A VOUS!', '', '', '', '¿Qué vas a hacer <target>, llorar?', '', ''),
('taunt', 'M-O-O-N! That spells aggro!', 0, 0, '', 'PUNAISE! Ce sort aggro!', '', '', '', '¡LUNA! ¡Eso deletrea agresividad!', '', ''),
('taunt', 'You’re about as useful as a one-legged man in an ass kicking contest.', 0, 0, '', 'Tu es à peu près aussi utile qu’un homme unijambiste dans un concours de coups de pied au cul.', '', '', '', 'Eres tan útil como un hombre con una sola pierna en un concurso de patadas en el culo.', '', ''),
('taunt', 'Hey <target>! Stop hitting on them, they’re not your type. They aren’t inflatable.', 0, 0, '', 'Hey <target>! Arrête de les draguer, ce n’est pas ton genre. Ils ne sont pas gonflables.', '', '', '', '¡Oye, <target>! Deja de coquetear con ellos, no son tu tipo. No son inflables.', '', ''),
('taunt', '<target> you’re so far outta your league, you’re playing a different sport.', 0, 0, '', '<target> vous êtes tellement hors de votre ligue que vous pratiquez un sport différent.', '', '', '', '<target> estás tan lejos de tu liga, estás jugando un deporte diferente.', '', ''),
('taunt', 'You made a big mistake today <target>, you got out of bed.', 0, 0, '', 'Tu as fait une grosse erreur aujourd’hui <target>, tu es sorti du lit.', '', '', '', 'Cometiste un gran error hoy <target>, te levantaste de la cama.', '', ''),
('taunt', 'I wanna try turning into a horse, but I need help. I’ll be the front, you be yourself.', 0, 0, '', 'Je veux essayer de me transformer en cheval, mais j’ai besoin d’aide. Je serai devant, tu seras toi-même.', '', '', '', 'Quiero intentar convertirme en un caballo, pero necesito ayuda. Yo seré el frente, tú mismo.', '', ''),
('taunt', 'Can I borrow your face for a few days? My ass is going on holiday....', 0, 0, '', 'Puis-je emprunter votre visage pendant quelques jours? Mon cul part en vacances....', '', '', '', '¿Me prestas tu cara por unos días? Mi culo se va de vacaciones...', '', ''),
('taunt', 'I’d like to give you a going away present... First you do your part.', 0, 0, '', 'j’aimerais vous offrir un cadeau de départ... d’abord, donnes ta part.', '', '', '', 'Me gustaría darte un regalo de despedida... Primero haz tu parte.', '', ''),
('taunt', 'Before you came along we were hungry, Now we’re just fed up.', 0, 0, '', 'Avant que tu n’arrives nous avions faim, maintenant nous en avons juste marre.', '', '', '', 'Antes de que llegaras teníamos hambre, ahora estamos hartos', '', ''),
('taunt', 'I like you. People say I have no taste, but I like you.', 0, 0, '', 'Je t''aime bien. Les gens disent que je n’ai pas de goût, mais je t''aime bien.', '', '', '', 'Me gustas. La gente dice que no tengo gusto, pero me gustas.', '', ''),
('taunt', 'I think you have an inferiority complex, but that’s okay, it’s justified.', 0, 0, '', 'Je pense que tu as un complexe d’infériorité, mais il n’y a pas de problème, c''est justifié.', '', '', '', 'Creo que tienes complejo de inferioridad, pero está bien, está justificado', '', ''),
('taunt', 'Hence rotten thing! Or I shall shake thy bones out of thy garments.', 0, 0, '', 'Par conséquent chose pourrie! Je vais secouer tes os de tes vêtements.', '', '', '', '¡Por lo tanto, cosa podrida! o te sacudiré los huesos de las vestiduras.', '', ''),
('taunt', 'I can’t believe I’m wasting my time with you!', 0, 0, '', 'Je n’arrive pas à croire que je perds mon temps avec toi!', '', '', '', '¡No puedo creer que estoy perdiendo el tiempo contigo!', '', ''),
('taunt', 'I love it when someone insults me, it means I don’t have to be nice anymore.', 0, 0, '', 'j’adore quand quelqu’un m''insulte, ça veut dire que je n’ai plus besoin d’être gentil.', '', '', '', 'Me encanta cuando alguien me insulta, significa que ya no tengo que ser amable', '', ''),
('taunt', 'Thou leathern-jerkin, crystal-button, knot-pated, agatering, puke-stocking, caddis-garter, smooth-tongue, Spanish pouch!', 0, 0, '', 'Toi l’agent fédéral, pourpoint en cuir, bouton de cristal, anneau d’agate, bas vomi, jarretière caddis, pochette espagnole!', '', '', '', '¡Tú, jubón de cuero, botón de cristal, anudado, ágata, medias de vómito, liga de caddis, lengua suave, bolsa española!', '', ''),
('taunt', 'Thou qualling bat-fowling malt-worm!', 0, 0, '', 'Toi qualificatif de chauve-souris qui chasse le ver de malt!', '', '', '', '¡Tú, gusano de malta cazador de murciélagos!', '', ''),
('taunt', 'Thou art truely an idol of idiot-worshippers!', 0, 0, '', 'Tu es vraiment une idole d’adorateurs d’idiots!', '', '', '', '¡Eres verdaderamente un ídolo de los adoradores de idiotas!', '', ''),
('taunt', 'Thou misbegotten knotty-pated wagtail!', 0, 0, '', 'Tu es une bergeronnette printanière mal engendrée!', '', '', '', '¡Maldito lavandero nudoso!', '', ''),
('taunt', 'Thou whoreson mandrake, thou art fitter to be worn in my cap than to wait at my heels!', 0, 0, '', 'Toi putain de mandragore, tu es plus apte à être porté dans ma casquette qu’à attendre sur mes talons!', '', '', '', '¡Hijo de puta mandrágora, eres mejor para que te lleve en mi gorra que para que me pise los talones!', '', ''),
('taunt', 'You! You scullion! You rampallian! You fustilarian! I’ll tickle your catastrophe!', 0, 0, '', 'Toi ! Espèce de marmiton ! Espèce de rampallian ! Espèce de fusilier ! Je vais chatouiller ta catastrophe!', '', '', '', '¡Tú! ¡Scullion! ¡Ramaliano! ¡Fustilarista! ¡Te haré cosquillas en tu catástrofe!', '', ''),
('taunt', 'Oh <target>! Thou infectious ill-nurtured flax-wench!', 0, 0, '', 'Oh <target>! Tu es contagieuse et mal nourrie!', '', '', '', '¡Oh, <target>! ¡Tú, contagiosa moza del lino mal educada!', '', ''),
('taunt', 'We leak in your chimney, <target>!', 0, 0, '', 'On a une fuite dans votre cheminée, <target>!', '', '', '', '¡Tenemos una fuga en tu chimenea, <target>!', '', ''),
('taunt', 'Oh thou bootless fen-sucked canker-blossom!', 0, 0, '', 'Oh toi, fleur de chancre sucée par les marais sans botte!', '', '', '', '¡Oh tú, flor de cancro chupada por el pantano sin botas!', '', ''),
('taunt', 'Were I like thee I’d throw away myself!', 0, 0, '', 'Si j’étais comme toi, je me jetterais!', '', '', '', '¡Si yo fuera como tú, me tiraría a la basura!', '', ''),
('taunt', 'O teach me <target>, how I should forget to think!', 0, 0, '', 'O apprends-moi <target>, comme je devrais oublier de penser!', '', '', '', '¡Oh, enséñame <target>, cómo debo olvidarme de pensar!', '', ''),
('taunt', 'Truly thou art damned, like an ill-roasted egg, all on one side!', 0, 0, '', 'Vraiment tu es damné, comme un œuf mal rôti, tout d’un côté!', '', '', '', '¡Verdaderamente estás condenado, como un huevo mal asado, todo de un lado!', '', ''),
('taunt', 'You starvelling, you eel-skin, you dried neat’s-tongue, you bull’s-pizzle, you stock-fish- O for breath to utter what is like thee!! -you tailor’s-yard, you sheath, you bow-case, you vile standing tuck!', 0, 0, '', 'Toi affamé, espèce de peau d’anguille, ta langue bien séchée, espèce de taureau, poisson séché, O pour le souffle de dire ce qui te ressemble !! -espèce de vergue de tailleur, espèce de fourreau, espèce d’étui à archet, vil tuck debout', '', '', '', 'Estás muerto de hambre, piel de anguila, te secaste la lengua limpiamente, toro-pizzle, pescado-O para respirar pronunciar lo que es como tú !! ¡Tú, sastrería, vaina, estuche de arco, vil flaco de pie!', '', ''),
('taunt', 'Fie! Drop thee into the rotten mouth of Death!', 0, 0, '', 'Fi ! Jette-toi dans la bouche pourrie de la Mort!', '', '', '', '¡Fie! ¡Déjate caer en la podrida boca de la Muerte!', '', ''),
('taunt', '<target>, you are a fishmonger!', 0, 0, '', '<target>, vous êtes poissonnier!', '', '', '', '<target>, ¡eres un pescadero!', '', ''),
('taunt', 'I shall live to knock thy brains out!', 0, 0, '', 'Je vivrai pour te faire sauter la cervelle!', '', '', '', '¡Viviré para romperte los sesos!', '', ''),
('taunt', 'Most shallow are you, <target>!! Thou art worms-meat in respect of a good piece of flesh, indeed!!', 0, 0, '', 'Tu es très superficiel, <target>!! Tu es de la viande de vers par rapport à un bon morceau de chair, en effet!!', '', '', '', '¡¡Eres el más superficial, <target>!! ¡Eres carne de gusano con respecto a un buen trozo de carne, de hecho!', '', ''),
('taunt', 'Vile wretch! O <target>, thou odiferous hell-hated pignut!', 0, 0, '', 'Misérable ! O <target>, toi, ignoble cochon de l’enfer!', '', '', '', '¡Miserable vil! ¡Oh, <target>, fétido cerdo odiado por el infierno!', '', ''),
('taunt', '<target>! Thy kiss is as comfortless as frozen water to a starved snake!', 0, 0, '', '<target>! Ton baiser est aussi désagréable que de l’eau gelée pour un serpent affamé!', '', '', '', '<target>! ¡Tu beso es tan desconsolador como el agua helada para una serpiente hambrienta!', '', ''),
('taunt', 'I scorn you, scurvy companion. What, you poor, base, rascally, cheating, lack-linen mate! Away, you moldy rogue, away!', 0, 0, '', 'Je te méprise, compagnon du scorbut. Quoi, pauvre, bas, coquin, tricheur, camarade en manque de linge ! Partez, espèce de voyou moisi, partez!', '', '', '', 'Te desprecio, compañero escorbuto. ¡Qué, pobre, vil, pícaro, tramposo, compañero falto de lino! ¡Fuera, mohoso granuja, fuera!', '', ''),
('taunt', 'Out of my sight! Thou dost infect my eyes <target>!', 0, 0, '', 'Hors de ma vue! Tu infectes mes yeux <target>!', '', '', '', '¡Fuera de mi vista! ¡Me infectas los ojos <target>!', '', ''),
('taunt', 'PLAY TIME!!!!', 0, 0, '', 'RÉCRÉATION!!!!', '', '', '', '¡¡¡HORA DE JUGAR!!!!', '', ''),
('taunt', 'None shall pass!', 0, 0, '', 'Personne ne passera!', '', '', '', '¡Ninguno pasará!', '', ''),
('taunt', 'We’re under attack! A vast, ye swabs! Repel the invaders!', 0, 0, '', 'Nous sommes attaqués ! Un vaste écouvillons! Repoussez les envahisseurs!', '', '', '', '¡Estamos bajo ataque! ¡Un vasto, hisopos! ¡Repeler a los invasores!', '', ''),
('taunt', 'None may challenge the Brotherhood!', 0, 0, '', 'Personne ne peut défier la Confrérie!', '', '', '', '¡Nadie puede desafiar a la Hermandad!', '', ''),
('taunt', 'Foolsss...Kill the one in the dress!', 0, 0, '', 'Foolsss... Tuez celui qui porte la robe!', '', '', '', 'Tontosss... ¡Matad al del vestido!', '', ''),
('taunt', 'I’ll feed your soul to Hakkar himself! ', 0, 0, '', 'Je donnerai ton âme à Hakkar lui-même! ', '', '', '', '¡Le daré de comer con tu alma al mismísimo Hakkar! ', '', ''),
('taunt', 'Pride heralds the end of your world! Come, mortals! Face the wrath of the <randomfaction>!', 0, 0, '', 'La fierté annonce la fin de votre monde ! Venez, mortels ! Affrontez la colère des <randomfaction>!', '', '', '', '¡El orgullo anuncia el fin de tu mundo! ¡Venid, mortales! ¡Enfréntate a la ira de la <facción aleatoria>!', '', ''),
('taunt', 'All my plans have led to this!', 0, 0, '', 'Tous mes plans ont mené à cela!', '', '', '', '¡Todos mis planes me han llevado a esto!', '', ''),
('taunt', 'Ahh! More lambs to the slaughter!', 0, 0, '', 'Ahhh ! Plus d’agneaux à l’abattoir!', '', '', '', '¡Ahh! ¡Más corderos al matadero!', '', ''),
('taunt', 'Another day, another glorious battle!', 0, 0, '', 'Un autre jour, une autre bataille glorieuse!', '', '', '', '¡Otro día, otra batalla gloriosa!', '', ''),
('taunt', 'So, business... or pleasure?', 0, 0, '', 'Alors, affaires... ou plaisir?', '', '', '', 'Entonces, ¿negocios... o placer?', '', ''),
('taunt', 'You are not prepared!', 0, 0, '', 'Vous n’êtes pas prêt!', '', '', '', '¡No estás preparado!', '', ''),
('taunt', 'The <randomfaction>’s final conquest has begun! Once again the subjugation of this world is within our grasp. Let none survive! ', 0, 0, '', 'La conquête finale de la <randomfaction> a commencé ! Une fois de plus, l’assujettissement de ce monde est à notre portée. Que personne ne survive!', '', '', '', '¡La conquista final de <randomfaction> ha comenzado! Una vez más, la subyugación de este mundo está a nuestro alcance. ¡Que nadie sobreviva! ', '', ''),
('taunt', 'Your death will be a painful one. ', 0, 0, '', 'Ta mort sera douloureuse.', '', '', '', 'Tu muerte será dolorosa. ', '', ''),
('taunt', 'Cry for mercy! Your meaningless lives will soon be forfeit. ', 0, 0, '', 'Crie miséricorde ! Vos vies insignifiantes seront bientôt perdues.', '', '', '', '¡Llora por piedad! Sus vidas sin sentido pronto se perderán. ', '', ''),
('taunt', 'Abandon all hope! The <randomfaction> has returned to finish what was begun so many years ago. This time there will be no escape! ', 0, 0, '', 'Abandonner tout espoir! La <randomfaction> est revenue pour terminer ce qui avait été commencé il y a tant d’années. Cette fois, il n’y aura pas d’échappatoire!', '', '', '', '¡Abandona toda esperanza! La <randomfaction> ha regresado para terminar lo que comenzó hace tantos años. ¡Esta vez no habrá escapatoria! ', '', ''),
('taunt', 'Alert! You are marked for Extermination! ', 0, 0, '', 'Alerte! Vous êtes marqué pour l’extermination!', '', '', '', '¡Alerta! ¡Estás marcado para el exterminio! ', '', ''),
('taunt', 'The <subzone> is for guests only...', 0, 0, '', 'La <subzone> est réservée aux invités...', '', '', '', 'La <subzone> es solo para invitados...', '', ''),
('taunt', 'Ha ha ha! You are hopelessly outmatched!', 0, 0, '', 'Hahaha! Vous êtes désespérément surpassé!', '', '', '', '¡Ja, ja, ja! ¡Estás irremediablemente superado!', '', ''),
('taunt', 'I will crush your delusions of grandeur! ', 0, 0, '', 'Je vais écraser vos illusions de grandeur!', '', '', '', '¡Aplastaré tus delirios de grandeza! ', '', ''),
('taunt', 'Forgive me, for you are about to lose the game.', 0, 0, '', 'Pardonnez-moi, car vous êtes sur le point de perdre la partie.', '', '', '', 'Perdóname, porque estás a punto de perder el juego.', '', ''),
('taunt', 'Struggling only makes it worse.', 0, 0, '', 'Lutter ne fait qu’empirer les choses.', '', '', '', 'La lucha solo lo empeora.', '', ''),
('taunt', 'Vermin! Leeches! Take my blood and choke on it!', 0, 0, '', 'Vermine! sangsues ! Prends mon sang et étouffe-toi avec!', '', '', '', '¡Alimañas! ¡Sanguijuelas! ¡Toma mi sangre y atragantate con ella!', '', ''),
('taunt', 'Not again... NOT AGAIN!', 0, 0, '', 'Pas encore... PAS ENCORE!', '', '', '', 'No otra vez... ¡NO OTRA VEZ!', '', ''),
('taunt', 'My blood will be the end of you!', 0, 0, '', 'Mon sang sera ta fin!', '', '', '', '¡Mi sangre será tu fin!', '', ''),
('taunt', 'Good, now you fight me!', 0, 0, '', 'Bien, maintenant tu me combats!', '', '', '', 'Bien, ¡ahora pelea conmigo!', '', ''),
('taunt', 'Get da move on, guards! It be killin’ time!', 0, 0, '', 'Allez-y, gardes ! Ça va tuer le temps!', '', '', '', '¡Muévanse, guardias! ¡Será para matar el tiempo!', '', ''),
('taunt', 'Don’t be delayin’ your fate. Come to me now. I make your sacrifice quick.', 0, 0, '', 'Ne retardez pas votre destin. Viens à moi maintenant. Je fais votre sacrifice rapide.', '', '', '', 'No demores tu destino. Ven a mí ahora. Hago que tu sacrificio sea rápido.', '', ''),
('taunt', 'You be dead soon enough!', 0, 0, '', 'Tu es mort bien assez tôt!', '', '', '', '¡Estarás muerto lo suficientemente pronto!', '', ''),
('taunt', 'Mua-ha-ha!', 0, 0, '', 'Mua-ha-ha!', '', '', '', '¡Mua-ja-ja!', '', ''),
('taunt', 'I be da predator! You da prey...', 0, 0, '', 'Je suis un prédateur ! Tu es une proie...', '', '', '', '¡Soy un depredador! Eres presa...', '', ''),
('taunt', 'You gonna leave in pieces!', 0, 0, '', 'Tu vas partir en morceaux!', '', '', '', '¡Te vas a ir en pedazos!', '', ''),
('taunt', 'Death comes. Will your conscience be clear? ', 0, 0, '', 'La mort vient. Votre conscience sera-t-elle claire?', '', '', '', 'La muerte llega. ¿Tu conciencia estará limpia? ', '', ''),
('taunt', 'Your behavior will not be tolerated.', 0, 0, '', 'Votre comportement ne sera pas toléré.', '', '', '', 'Tu comportamiento no será tolerado', '', ''),
('taunt', 'The Menagerie is for guests only.', 0, 0, '', 'La Ménagerie est réservée aux invités.', '', '', '', 'The Menagerie es solo para invitados', '', ''),
('taunt', 'Hmm, unannounced visitors, Preparations must be made... ', 0, 0, '', 'Hmm, visiteurs inopinés, il faut faire des préparatifs...', '', '', '', 'Hmm, visitantes inesperados, se deben hacer preparativos... ', '', ''),
('taunt', 'Hostile entities detected. Threat assessment protocol active. Primary target engaged. Time minus thirty seconds to re-evaluation.', 0, 0, '', 'Entités hostiles détectées. Protocole d’évaluation de la menace actif. Cible principale engagée. Délai de réévaluation moins trente secondes.', '', '', '', 'Entidades hostiles detectadas. Protocolo de evaluación de amenazas activo. Objetivo principal comprometido. Tiempo menos treinta segundos para la reevaluación.', '', ''),
('taunt', 'New toys? For me? I promise I won’t break them this time!', 0, 0, '', 'Nouveaux jouets? Pour moi? Promis je ne les casserai pas cette fois !', '', '', '', '¿Juguetes nuevos? ¿Para mi? ¡Te prometo que no los romperé esta vez!', '', ''),
('taunt', 'I’m ready to play!', 0, 0, '', 'Je suis prêt à jouer!', '', '', '', '¡Estoy listo para jugar!', '', ''),
('taunt', 'Shhh... it will all be over soon.', 0, 0, '', 'Chut... tout sera bientôt fini.', '', '', '', 'Shhh... todo terminará pronto.', '', ''),
('taunt', 'Aaaaaughibbrgubugbugrguburgle!', 0, 0, '', 'Aaaaaughibbrgubugbugrguburgle!', '', '', '', '¡Aaaaaughibbrgubugbugrguburgle!', '', ''),
('taunt', 'RwlRwlRwlRwl!', 0, 0, '', 'RwlRwlRwlRwl!', '', '', '', 'RwlRwlRwlRwl!', '', ''),
('taunt', 'You too, shall serve!', 0, 0, '', 'Toi aussi, tu serviras!', '', '', '', '¡Tú también, debes servir!', '', ''),
('taunt', 'Tell me... tell me everything!  Naughty secrets! I’ll rip the secrets from your flesh!', 0, 0, '', 'Dis-moi... dis-moi tout ! Secrets coquins ! j’arracherai les secrets de ta chair!', '', '', '', 'Cuéntame... ¡cuéntame todo! ¡Secretos traviesos! ¡Arrancaré los secretos de tu carne!', '', ''),
('taunt', 'Prepare yourselves, the bells have tolled! Shelter your weak, your young and your old! Each of you shall pay the final sum! Cry for mercy, the reckoning has come!', 0, 0, '', 'Préparez-vous, les cloches ont sonné ! Mettez à l’abri vos faibles, vos jeunes et vos vieux ! Chacun de vous paiera la somme finale ! Implorez la miséricorde , le jugement est venu!', '', '', '', '¡Prepárense, las campanas han doblado! ¡Abriga a tus débiles, a tus jóvenes ya tus viejos! ¡Cada uno de ustedes pagará la suma final! Clama por misericordia, el ajuste de cuentas ha llegado!', '', ''),
('taunt', 'Where in Bonzo’s brass buttons am I?', 0, 0, '', 'Où suis-je dans les boutons en laiton de Bonzo?', '', '', '', '¿Dónde estoy en los botones de latón de Bonzo?', '', ''),
('taunt', 'I can bear it no longer! Goblin King! Goblin King! Wherever you may be! Take this <target> far away from me!', 0, 0, '', 'Je ne peux plus le supporter ! Roi Gobelin ! Roi Gobelin ! Où que vous soyez ! Emmenez cette <target> loin de moi!', '', '', '', '¡No puedo soportarlo más! ¡Rey de los duendes! ¡Rey de los duendes! ¡WHERE quiera que estés! ¡Lleva a este <target> lejos de mí!', '', ''),
('taunt', 'You have thirteen hours in which to solve the labyrinth, before your baby brother becomes one of us... forever.', 0, 0, '', 'Vous avez treize heures pour résoudre le labyrinthe, avant que ton petit frère ne devienne l’un des nôtres... pour toujours.', '', '', '', 'Tienes trece horas para resolver el laberinto, antes de que tu hermanito se convierta en uno de nosotros... para siempre', '', ''),
('taunt', 'So, the <subzone> is a piece of cake, is it? Well, let’s see how you deal with this little slice... ', 0, 0, '', 'Donc, la <subzone> est un jeu d’enfant, n’est-ce pas ? Eh bien, voyons comment tu gères cette petite tranche...', '', '', '', 'Entonces, la <subzone> es pan comido, ¿verdad? Bueno, veamos cómo lidias con esta pequeña porción... ', '', ''),
('taunt', 'Back off, I’ll take you on, headstrong to take on anyone, I know that you are wrong, and this is not where you belong', 0, 0, '', 'Reculez, je vais vous attaquer, têtu à affronter n’importe qui, je sais que vous vous trompez, et ce n’est pas votre place', '', '', '', 'Atrás, te enfrentaré, testarudo para enfrentar a cualquiera, sé que estás equivocado, y este no es el lugar al que perteneces', '', ''),
('taunt', 'Show me whatcha got!', 0, 0, '', 'Montre-moi ce que tu as!', '', '', '', '¡Muéstrame lo que tienes!', '', ''),
('taunt', 'To the death!', 0, 0, '', 'A la mort!', '', '', '', '¡Hasta la muerte!', '', ''),
('taunt', 'Twin blade action, for a clean close shave every time.', 0, 0, '', 'Action à double lame, pour un rasage de près propre à chaque fois.', '', '', '', 'Acción de cuchilla doble, para un afeitado limpio y apurado en todo momento.', '', ''),
('taunt', 'Bring it on!', 0, 0, '', 'Allez-y!', '', '', '', '¡Adelante!', '', ''),
('taunt', 'You’re goin’ down!', 0, 0, '', 'Tu descends!', '', '', '', '¡Vas a caer!', '', ''),
('taunt', 'Stabby stab stab!', 0, 0, '', 'Coup de poignard poignardé!', '', '', '', '¡Puñalada puñalada puñalada!', '', ''),
('taunt', 'Let’s get this over quick, time is mana.', 0, 0, '', 'Finissons-en vite , le temps c''est du mana.', '', '', '', 'Terminemos con esto rápido, el tiempo es maná.', '', ''),
('taunt', 'I do not think you realise the gravity of your situation.', 0, 0, '', 'Je ne pense pas que tu te rendes compte de la gravité de ta situation.', '', '', '', 'No creo que te des cuenta de la gravedad de tu situación.', '', ''),
('taunt', 'I will bring honor to my family and my kingdom!', 0, 0, '', 'Je ferai honneur à ma famille et à mon royaume!', '', '', '', '¡Llevaré honor a mi familia y mi reino!', '', ''),
('taunt', 'Light, give me strength!', 0, 0, '', 'Lumière, donne-moi la force!', '', '', '', '¡Luz, dame fuerza!', '', ''),
('taunt', 'My church is the field of battle - time to worship...', 0, 0, '', 'Mon église est le champ de bataille - il est temps d’adorer...', '', '', '', 'Mi iglesia es el campo de batalla - hora de adorar...', '', ''),
('taunt', 'I hold you in contempt...', 0, 0, '', 'Je te méprise...', '', '', '', 'Te tengo en desacato...', '', ''),
('taunt', 'Face the hammer of justice!', 0, 0, '', 'Affrontez le marteau de la justice!', '', '', '', '¡Enfréntate al martillo de la justicia!', '', ''),
('taunt', 'Prove your worth in the test of arms under the Light!', 0, 0, '', 'Prouvez votre valeur dans l’épreuve des armes sous la Lumière!', '', '', '', '¡Demuestra tu valía en la prueba de las armas bajo la Luz!', '', ''),
('taunt', 'All must fall before the might and right of my cause, you shall be next!', 0, 0, '', 'Tout doit tomber devant la force et le droit de ma cause, vous serez le prochain!', '', '', '', 'Todo debe caer ante el poder y el derecho de mi causa, ¡tú serás el próximo!', '', ''),
('taunt', 'Prepare to die!', 0, 0, '', 'Préparez-vous à mourir!', '', '', '', '¡Prepárate para morir!', '', ''),
('taunt', 'The beast with me is nothing compared to the beast within...', 0, 0, '', 'La bête avec moi n’est rien comparée à la bête à l’intérieur...', '', '', '', 'La bestia conmigo no es nada comparada con la bestia interior...', '', ''),
('taunt', 'Witness the firepower of this fully armed huntsman!', 0, 0, '', 'Soyez témoin de la puissance de feu de ce chasseur entièrement armé!', '', '', '', '¡Sea testigo de la potencia de fuego de este cazador completamente armado!', '', ''),



-- combat events
('critical health', 'Heal me! Quick!', 0, 0, '', 'Soigne moi! Vite!', '', '', '', '¡Cúrame! ¡Rápido!', '', ''),
('critical health', 'Almost dead! Heal me!', 0, 0, '', 'Presque mort! Soigne moi!', '', '', '', '¡Estoy casi muerto! ¡Cúrame!', '', ''),
('critical health', 'Help! Heal me!', 0, 0, '', 'Aider! Soigne moi!', '', '', '', '¡Ayuda! ¡Cúrame!', '', ''),
('critical health', 'Somebody! Heal me!', 0, 0, '', 'Quelqu’un! Soigne moi!', '', '', '', '¡Alguien! ¡Cúrame!', '', ''),
('critical health', 'Heal! Heal! Heal!', 0, 0, '', 'Soigner! Soigner! Soigner!', '', '', '', '¡Curame! ¡Saname! ¡Socorro!', '', ''),
('critical health', 'I am dying! Heal! Aaaaarhg!', 0, 0, '', 'Je meurs! Soigner! Aaaaarhg!', '', '', '', '¡Me estoy muriendo! ¡Saname! ¡Aaaaarhg!', '', ''),
('critical health', 'Heal me!', 0, 0, '', 'Soigne moi!', '', '', '', '¡Cúrame!', '', ''),
('critical health', 'I will die. I will die. I will die. Heal!', 0, 0, '', 'Je vais mourir. Je vais mourir. Je vais mourir. Soigner!', '', '', '', 'Voy a morir. Voy a morir. Voy a morir. ¡Saname!', '', ''),
('critical health', 'Healers, where are you? I am dying!', 0, 0, '', 'Guérisseurs, où êtes-vous ? Je meurs!', '', '', '', 'Sanadores, ¿dónde estais? ¡Me estoy muriendo!', '', ''),
('critical health', 'Oh the pain. Heal me quick!', 0, 0, '', 'Ah la douleur. Guéris-moi vite!', '', '', '', 'Ouch, qué dolor. ¡Cúrame rápido!', '', ''),



('low health', 'Need heal', 0, 0, '', 'Besoin de guérir', '', '', '', 'Necesito una cura', '', ''),
('low health', 'Low health', 0, 0, '', 'Santé faible', '', '', '', 'Salud baja', '', ''),
('low health', 'Drop a heal. Please.', 0, 0, '', 'Dépose un soin. S''il te plaît.', '', '', '', 'Una sanación por favor.', '', ''),
('low health', 'Could somebody drop a heal on me?', 0, 0, '', 'Quelqu’un pourrait-il me soigner?', '', '', '', '¿Alguien podría curarme?', '', ''),
('low health', 'Hey! Better heal me now than rez later', 0, 0, '', 'Hey! Mieux vaut me soigner maintenant que rez plus tard', '', '', '', '¡Oye! Mejor curarme ahora que resucitarme después', '', ''),
('low health', 'I am sorry. Need another heal', 0, 0, '', 'Je suis désolé. j’ai besoin d’un autre soin', '', '', '', 'Lo siento. Necesito otra curación', '', ''),
('low health', 'Damn mobs. Heal me please', 0, 0, '', 'Maudits mobs. Guéris moi s''il te plait', '', '', '', '¡Malditos bichos! ¡Una curita por favor!', '', ''),
('low health', 'One more hit and I am done for. Heal please', 0, 0, '', 'Un coup de plus et c''est fini. Guéris moi s''il te plait', '', '', '', 'Un golpe más y estoy acabado. Cura por favor', '', ''),
('low health', 'Are there any healers?', 0, 0, '', 'Y a-t-il des guérisseurs?', '', '', '', '¿Hay sanadores?', '', ''),
('low health', 'Why do they always punch me in the face? Need heal', 0, 0, '', 'Pourquoi me frappent-ils toujours au visage? Besoin de soins', '', '', '', '¿Por qué siempre me golpean en la cara? Necesito sanacion', '', ''),
('low health', 'Can anybody heal me a bit?', 0, 0, '', 'Quelqu’un peut-il me guérir un peu?', '', '', '', '¿Alguien puede curarme un poco?', '', ''),



('low mana', 'OOM', 0, 0, '', 'MOO', '', '', '', 'No tengo mana!', '', ''),
('low mana', 'I am out of mana', 0, 0, '', 'je n’ai plus de mana', '', '', '', 'No tengo maná', '', ''),
('low mana', 'Damn I wasted all my mana on this', 0, 0, '', 'Putain j’ai gaspillé tout mon mana sur ça', '', '', '', 'Me cago en todo! Malgasté todo mi maná enseguida', '', ''),
('low mana', 'You should wait until I drink or regenerate my mana', 0, 0, '', 'Vous devriez attendre que je boive ou que je régénère mon mana', '', '', '', 'Deberías esperar hasta que beba o regenere mi maná', '', ''),
('low mana', 'Low mana', 0, 0, '', 'Mana faible', '', '', '', 'Maná bajo', '', ''),
('low mana', 'No mana. Again?', 0, 0, '', 'Pas de mana. De nouveau?', '', '', '', 'Sin maná. ¿Otra vez?', '', ''),
('low mana', 'Low mana. Wanna drink', 0, 0, '', 'Mana faible. Je veux boire', '', '', '', 'Maná bajo. Quiero beber', '', ''),
('low mana', 'Do we have a vending machine? Out of mana again', 0, 0, '', 'Avons-nous un distributeur automatique? Plus de mana à nouveau', '', '', '', '¿Tenemos una máquina expendedora? Sin maná otra vez', '', ''),
('low mana', 'My mana is history', 0, 0, '', 'Mon mana appartient à l’histoire', '', '', '', 'Mi maná es historia', '', ''),
('low mana', 'I’d get some drinks next time. Out of mana', 0, 0, '', 'Je prendrais quelques verres la prochaine fois. Manque de mana', '', '', '', '¡No tengo maná! Tengo que comprar bebidas la próxima vez...', '', ''),
('low mana', 'Where is my mana?', 0, 0, '', 'Où est mon mana?', '', '', '', '¿Dónde está mi maná?', '', ''),



('low ammo', 'I have few <ammo> left!', 0, 0, '', 'Il me reste peu de <ammo>!', '', '', '', '¡Me quedan pocas <ammo>!', '', ''),
('low ammo', 'I need more <ammo>!', 0, 0, '', 'j’ai besoin de plus de <ammo>!', '', '', '', '¡Necesito más <ammo>!', '', ''),
('low ammo', '100 <ammo> left!', 0, 0, '', 'Il reste 100 <ammo>!', '', '', '', '¡Quedan 100 <ammo>!', '', ''),



('no ammo', 'That’s it! No <ammo>!', 0, 0, '', 'C''est ça! Pas de <ammo>!', '', '', '', '¡Eso es todo! ¡Sin <ammo>!', '', ''),
('no ammo', 'And you have my bow... Oops, no <ammo>!', 0, 0, '', 'Et vous avez mon arc... Oups, pas de <ammo>!', '', '', '', 'Y tú tienes mi arco... ¡Uy, sin <ammo>!', '', ''),
('no ammo', 'Need ammo!', 0, 0, '', 'Besoin de munitions!', '', '', '', '¡Necesito munición!', '', ''),



('aoe', 'Oh god!', 0, 0, '', 'Oh mon Dieu!', '', '', '', '¡Oh, Dios!', '', ''),
('aoe', 'I am scared', 0, 0, '', 'j’ai peur', '', '', '', 'Tengo miedo', '', ''),
('aoe', 'We are done for', 0, 0, '', 'Nous avons fini pour', '', '', '', 'Hemos terminado', '', ''),
('aoe', 'This is over', 0, 0, '', 'C''est terminé', '', '', '', 'Esto se acabó', '', ''),
('aoe', 'This ends now', 0, 0, '', 'Cela se termine maintenant', '', '', '', 'Esto termina ahora', '', ''),
('aoe', 'Could somebody cast blizzard or something?', 0, 0, '', 'Quelqu’un pourrait-il lancer le blizzard ou quelque chose comme ça?', '', '', '', '¿Alguien podría lanzar ventisca o algo así?', '', ''),
('aoe', 'Damn. The tank aggroed all the mobs around', 0, 0, '', 'Mince. Le tank a agro toutes les mobs autour', '', '', '', '¡Me cago en todo! El tanque atrajo a todos los mobs...', '', ''),
('aoe', 'We gonna die. We gonna die. We gonna die.', 0, 0, '', 'Nous allons mourir. Nous allons mourir. Nous allons mourir.', '', '', '', 'Vamos a morir. Vamos a morir. Vamos a morir.', '', ''),
('aoe', 'Whoa! So many toys to play with', 0, 0, '', 'Ouah ! Tant de jouets avec lesquels jouer', '', '', '', '¡Vaya! Tantos juguetes con los que jugar', '', ''),
('aoe', 'I gonna kill them all!', 0, 0, '', 'Je vais tous les tuer!', '', '', '', '¡Voy a matarlos a todos!', '', ''),
('aoe', 'If the tank dies we are history', 0, 0, '', 'Si le tank meurt, nous appartenons à l’histoire', '', '', '', 'Si el tanque muere somos historia', '', ''),
('aoe', 'Aaaaaargh!', 0, 0, '', 'Aaaaaargh!', '', '', '', '¡Aaaaaargh!', '', ''),
('aoe', 'LEEEEERROOOYYYYYYYYYYYY JENNKINNNSSSSSS!!!!!!!', 0, 0, '', 'LEEEEERROOOYYYYYYYYYYYY JENNKINNNSSSSSS!!!!!!!', '', '', '', 'LEEEEERROOOYYYYYYYYYYYY JENNKINNNSSSSSS!!!!!!!', '', ''),
('aoe', 'Right. What do we have in AOE?', 0, 0, '', 'd’accord. qu’avons-nous en dégats de zone?', '', '', '', 'Claro. ¿Qué tenemos en AOE?', '', ''),
('aoe', 'This gets interesting', 0, 0, '', 'Cela devient intéressant', '', '', '', 'Esto se pone interesante', '', ''),
('aoe', 'Cool. Get them in one place for a good flamestrike', 0, 0, '', 'Cool. Rassemblez-les au même endroit pour un bon coup de flamme', '', '', '', 'Genial! Ponlos todos juntos para un buen ataque en area', '', ''),
('aoe', 'Kill! Kill! Kill!', 0, 0, '', 'Tuer! Tuer! Tuer!', '', '', '', '¡Matar! ¡Matar! ¡Matar!', '', ''),
('aoe', 'I think my pants are wet', 0, 0, '', 'Je pense que mon pantalon est mouillé', '', '', '', 'Creo que mis pantalones están mojados', '', ''),
('aoe', 'We are history', 0, 0, '', 'Nous sommes l’histoire', '', '', '', 'Somos historia', '', ''),
('aoe', 'I hope healers are ready. Leeeeroy!', 0, 0, '', 'j’espère que les guérisseurs sont prêts. Leeeeeroy!', '', '', '', 'Espero que los sanadores estén listos. ¡Leeeroy!', '', ''),
('aoe', 'I hope they won’t come for me', 0, 0, '', 'j’espère qu’ils ont gagné, qu’ils ne viendront pas pour moi', '', '', '', 'Espero que no vengan por mí', '', ''),
('aoe', 'Oh no. I can’t see at this slaugther', 0, 0, '', 'Oh non. Je ne peux pas voir à ce massacre', '', '', '', 'Oh, no. No puedo ver esta matanza', '', ''),



-- on looting
('loot', 'I hope there will be some money', 0, 0, '', 'j’espère qu’il y aura de l’argent', '', '', '', 'Espero que haya algo de dinero...', '', ''),
('loot', 'Loot! Loot!', 0, 0, '', 'Butin! Butin!', '', '', '', '¡Botín! ¡Botín!', '', ''),
('loot', 'My precious', 0, 0, '', 'Mon précieux', '', '', '', 'Mi tesoro...', '', ''),
('loot', 'I hope there is a shiny epic item waiting for me there', 0, 0, '', 'j’espère qu’il y a un objet épique brillant qui m''attend ici', '', '', '', 'Espero que haya un objeto épico brillante esperándome allí', '', ''),
('loot', 'I have deep pockets and bags', 0, 0, '', 'j’ai des poches et des sacs profonds', '', '', '', 'Tengo bolsillos profundos y infinitos', '', ''),
('loot', 'All is mine!', 0, 0, '', 'Tout est à moi!', '', '', '', '¡Todo es mío!', '', ''),
('loot', 'Hope no gray shit today', 0, 0, '', 'j’espère pas de merde grise aujourd’hui', '', '', '', 'Espero que hoy no haya mierda gris', '', ''),
('loot', 'This loot is MINE!', 0, 0, '', 'Ce butin est à MOI!', '', '', '', '¡Este botín es MÍO!', '', ''),
('loot', 'Looting is disgusting but I need money', 0, 0, '', 'Le pillage est dégoûtant mais j’ai besoin d’argent', '', '', '', 'El saqueo es repugnante pero necesito dinero...', '', ''),
('loot', 'Gold!', 0, 0, '', 'Or!', '', '', '', '¡Oro!', '', ''),
('loot', 'OK. Let’s see what they’ve got', 0, 0, '', 'd’ACCORD. Voyons ce qu’ils ont', '', '', '', 'OK. Veamos qué tienen', '', ''),
('loot', 'Do not worry. I will loot eveything', 0, 0, '', 'Ne t''inquiète pas. je vais tout piller', '', '', '', 'No te preocupes. Saquearé todo', '', ''),
('loot', 'I am loot ninja', 0, 0, '', 'je suis un ninja de butin', '', '', '', 'Soy un ladron de tesoros', '', ''),
('loot', 'Do I neeed to roll?', 0, 0, '', 'Dois-je recommencer?', '', '', '', '¿Necesito lanzar dados?', '', ''),
('loot', 'Somebody explain me, where they did put all this stuff?', 0, 0, '', 'Quelqu’un m''explique, où ils ont mis tout ça?', '', '', '', 'Alguien me explica, ¿dónde pusieron todas estas cosas?', '', ''),
('loot', 'No, I won’t loot gray shit', 0, 0, '', 'Non, je ne pillerai pas la merde grise', '', '', '', 'No, no saquearé mierda gris', '', ''),
('loot', 'I’m first. I’m first. I’m first.', 0, 0, '', 'Je suis le premier. Je suis le premier. Je suis le premier.', '', '', '', '¡Es mio! ¡Es mio! ¡Es mio!', '', ''),
('loot', 'Give me your money!', 0, 0, '', 'Donne-moi ton argent!', '', '', '', '¡Dame todo tu dinero!', '', ''),
('loot', 'My pockets are empty, I need to fill them', 0, 0, '', 'Mes poches sont vides, j’ai besoin de les remplir', '', '', '', 'Mis bolsillos están vacíos y necesitan llenarse!', '', ''),
('loot', 'I’ve got a new bag for this', 0, 0, '', 'j’ai un nouveau sac pour ça', '', '', '', 'Tengo una nueva bolsa para esto', '', ''),
('loot', 'I hope I won’t aggro anybody while looting', 0, 0, '', 'j’espère que je n’agresserai personne pendant le pillage', '', '', '', 'Espero no ofender a nadie mientras saqueo', '', ''),
('loot', 'Please don’t watch. I am looting', 0, 0, '', 'S''il vous plaît, ne regardez pas. je pille', '', '', '', 'Por favor, no mires. Estoy saqueando', '', ''),
('loot', 'Ha! You won’t get any piece of it!', 0, 0, '', 'Ha! Vous n’en aurez aucun morceau!', '', '', '', '¡Ja! ¡No obtendrás nada de eso!', '', ''),
('loot', 'Looting is cool', 0, 0, '', 'Le pillage c''est cool', '', '', '', 'Saquear es genial', '', ''),
('loot', 'I like new gear', 0, 0, '', 'j’aime le nouveau matos', '', '', '', 'Me gusta el equipo nuevo', '', ''),
('loot', 'I’l quit if there is nothing valuable again', 0, 0, '', 'j’arrêterai s''il n’y a plus rien de précieux', '', '', '', 'Me doy por vencido si no hay nada util', '', ''),
('loot', 'I hope it is be a pretty ring', 0, 0, '', 'j’espère que c''est une jolie bague', '', '', '', 'Espero que sea un anillo bonito', '', ''),
('loot', 'I’l rip the loot from you', 0, 0, '', 'Je t''arracherai le butin', '', '', '', 'Te arrancaré el botín', '', ''),
('loot', 'Everybody stay off. I’m going to loot', 0, 0, '', 'Tout le monde reste à l’écart. je vais piller', '', '', '', '¡Alejaos todos! Voy a lootear', '', ''),
('loot', 'Sweet loot', 0, 0, '', 'Doux butin', '', '', '', 'Dulce botín', '', ''),
('loot', 'The Roll God! Give me an epic today', 0, 0, '', 'Le Dieu du rouleau ! Donnez-moi de l’épique aujourd’hui', '', '', '', '¡El Dios de los dados! Dame un epico hoy', '', ''),
('loot', 'Please give me new toys', 0, 0, '', 'S''il te plaît, donne-moi de nouveaux jouets', '', '', '', 'Por favor, dame nuevos juguetes', '', ''),
('loot', 'I hope they carry tasties', 0, 0, '', 'j’espère qu’ils portent des saveurs', '', '', '', 'Espero que caiga algo sabrosos', '', ''),
('loot', 'The gold is mine. I’ll leave everyting, I promise', 0, 0, '', 'l’or est à moi. Je laisse tout, je promets', '', '', '', 'El oro es mío. Dejaré todo lo demas, lo prometo!', '', ''),
('loot', 'No, I can’t resist', 0, 0, '', 'Non, je ne peux pas résister', '', '', '', 'No, no puedo resistir', '', ''),
('loot', 'I want more!', 0, 0, '', 'Je veux plus!', '', '', '', '¡Quiero más!', '', ''),



-- wait signals
('wait_travel_close', 'I am close, wait for me!', 0, 0, '', '', '', '', '', '¡Estoy cerca, espérame!', '', 'Я тут рядом, подожди!'),
('wait_travel_close', 'I’m not far, please wait!', 0, 0, '', '', '', '', '', 'No estoy lejos, ¡esperame por favor!', '', 'Я недалеко, погодите!'),



('wait_travel_medium', 'I’m heading to your location', 0, 0, '', '', '', '', '', 'Me dirijo a tu ubicación', '', 'Двигаюсь к тебе'),
('wait_travel_medium', 'I’m coming to you', 0, 0, '', '', '', '', '', 'Voy hacia ti', '', 'Иду к тебе'),



('wait_travel_far', 'I’m traveling to your location', 0, 0, '', '', '', '', '', 'Voy a viajar a tu ubicación', '', 'Направляюсь к тебе'),
('wait_travel_far', 'I’m trying to get to you', 0, 0, '', '', '', '', '', 'Estoy tratando de llegar a ti', '', 'Пытаюсь до тебя добраться'),




-- commands
('equip_command', 'Equipping %item', 0, 0, '', '', '', '', '', 'Me he equipado %item', '', ''),



('unequip_command', '%item unequipped', 0, 0, '', '', '', '', '', 'Me he quitado %item', '', ''),



('auto_learn_spell', 'I have learned the spells: %spells', 0, 0, '', '', '', '', '', 'He aprendido los hechizos: %spells', '', ''),



('use_command_item_cooldown', '%item is in cooldown', 0, 0, '', '', '', '', '', '%item esta recargandose', '', ''),



('use_command_item_not_owned', 'I don’t have %item in my inventory', 0, 0, '', '', '', '', '', 'No tengo %item en mi inventario', '', ''),



('use_command_invalid_item', 'The item with the id %item does not exist', 0, 0, '', '', '', '', '', 'El objeto con el id %item no existe', '', ''),



('use_command_socket', 'Socketing %gem into %item', 0, 0, '', '', '', '', '', 'Insertando %gem en %item', '', ''),



('use_command_item_error', 'I can’t use %item', 0, 0, '', '', '', '', '', 'No puedo usar %item', '', ''),



('following', 'Following', 0, 0, '', '', '', '', '', 'Te sigo', '', 'Следую'),



('staying', 'Staying', 0, 0, '', '', '', '', '', 'Me quedo aqui', '', 'Стою'),



('fleeing', 'Fleeing', 0, 0, '', '', '', '', '', 'Huyendo', '', 'Убегаю'),



('fleeing_far', 'I won’t flee with you, you are too far away', 0, 0, '', '', '', '', '', 'No huiré contigo, estás demasiado lejos', '', 'Я с тобой не побегу, ты слишком далеко'),



('grinding', 'Grinding', 0, 0, '', '', '', '', '', 'Farmeando', '', 'Гриндю'),



('attacking', 'Attacking', 0, 0, '', '', '', '', '', 'Atacando', '', 'Атакую'),



('error_far', 'It is too far away', 0, 0, '', '', '', '', '', 'Estas demasiado lejos', '', 'Слишком далеко'),



('error_water', 'It is under water', 0, 0, '', '', '', '', '', 'Estas debajo del agua', '', 'Это под водой'),



('error_cant_go', 'I can’t go there', 0, 0, '', '', '', '', '', 'No puedo ir allí', '', 'Я не могу туда пройти'),



('error_guild', 'I’m not in your guild!', 0, 0, '', '', '', '', '', '¡No estoy en tu hermandad!', '', 'Я не в твоей гильдии'),



('error_gbank_found', 'Can not find a guild bank nearby', 0, 0, '', '', '', '', '', 'No se puede encontrar un banco de hermandad cercano', '', 'Не могу найти гильд банк рядом'),



('error_cant_put', 'I can’t put ', 0, 0, '', '', '', '', '', 'No puedo depositar ', '', 'Я не могу положить'),



('error_gbank_rights', 'I have no rights to put items in the first guild bank tab', 0, 0, '', '', '', '', '', 'No tengo derechos para depositar objetos en la primera pestaña del banco de hermandad', '', 'Нет прав чтобы класть вещи в первую вкладку гильд банка'),



('gbank_put', ' put to guild bank', 0, 0, '', '', '', '', '', ' depositado en el banco de hermandad', '', ' теперь в гильд банке'),



('free_moving', 'Free moving', 0, 0, '', '', '', '', '', 'Moviendome libremente', '', ''),



('guarding', 'Guarding', 0, 0, '', '', '', '', '', 'Protegiendo la posición', '', ''),



('use_command', 'Using %target', 0, 0, '', '', '', '', '', 'Usando %target', '', ''),



('command_target_unit', 'on %unit', 0, 0, '', '', '', '', '', 'en %unit', '', ''),



('use_command_remaining', '(%amount available)', 0, 0, '', '', '', '', '', '(%amount restante)', '', ''),



('use_command_last', '(the last one)', 0, 0, '', '', '', '', '', '(el último)', '', ''),



('use_command_socket_error', 'The socket does not fit', 0, 0, '', '', '', '', '', 'La ranura no sirve', '', ''),



('command_target_trade', 'on trade item', 0, 0, '', '', '', '', '', 'en objeto comerciado', '', ''),



('command_target_self', 'on myself', 0, 0, '', '', '', '', '', 'en mi', '', ''),



('command_target_item', 'on %item', 0, 0, '', '', '', '', '', 'en %item', '', ''),



('command_target_go', 'on %gameobject', 0, 0, '', '', '', '', '', 'en %gameobject', '', ''),



('loot_command', 'Looting %item', 0, 0, '', '', '', '', '', 'Despojando %item', '', ''),



('cast_spell_command_summon', 'Summoning %target', 0, 0, '', '', '', '', '', 'Invocando a %target', '', ''),
('cast_spell_command_summon_error_members', 'I don’t have enough party members around to cast a summon', 0, 0, '', '', '', '', '', 'No tengo suficientes miembros de grupo alrededor para invocar', '', ''),
('cast_spell_command_summon_error_target', 'Failed to find the summon target', 0, 0, '', '', '', '', '', 'No he podido encontrar al objetivo de la invocación', '', ''),
('cast_spell_command_summon_error_combat', 'I can’t summon while I’m in combat', 0, 0, '', '', '', '', '', 'No puedo invocar durante un combate', '', ''),



('cast_spell_command_error_unknown_spell', 'I don’t know the spell %spell', 0, 0, '', '', '', '', '', 'No conozco el hechizo %spell', '', ''),



('cast_spell_command_spell', 'Casting %spell', 0, 0, '', '', '', '', '', 'Lanzando %spell', '', ''),



('cast_spell_command_craft', 'Crafting %spell', 0, 0, '', '', '', '', '', 'Creando %spell', '', ''),



('cast_spell_command_error', 'Cannot cast %spell', 0, 0, '', '', '', '', '', 'No puedo lanzar %spell', '', ''),
('cast_spell_command_error_failed', 'Failed to cast %spell', 0, 0, '', '', '', '', '', 'Fallo al lanzar %spell', '', ''),



('cast_spell_command_amount', ' |cffffff00(x%amount left)|r', 0, 0, '', '', '', '', '', ' |cffffff00(x%amount restante)|r', '', ''),




('dummy_end', 'dummy', 0, 0, '', '', '', '', '', '', '', '');
