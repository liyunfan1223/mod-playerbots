DELETE FROM ai_playerbot_texts
WHERE name IN (
  'rp_missing_reagent_greater_blessing',
  'rp_missing_reagent_gift_of_the_wild',
  'rp_missing_reagent_arcane_brilliance',
  'rp_missing_reagent_generic'
);

DELETE FROM ai_playerbot_texts_chance
WHERE name IN (
  'rp_missing_reagent_greater_blessing',
  'rp_missing_reagent_gift_of_the_wild',
  'rp_missing_reagent_arcane_brilliance',
  'rp_missing_reagent_generic'
);

INSERT INTO ai_playerbot_texts (name, text, say_type, reply_type, text_loc1, text_loc2, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES
  ('rp_missing_reagent_greater_blessing',
    'By the Light... I forgot my Symbols of Kings. We’ll make do with %base_spell!', 0, 0,
    '', 'Par la Lumière... J''ai oublié mes Symboles du roi. On se contentera de %base_spell !', '', '', '', '', '', ''),
  ('rp_missing_reagent_gift_of_the_wild',
    'Nature is generous, my bags are not... out of herbs for %group_spell. Take %base_spell for now!', 0, 0,
    '', 'La nature est généreuse, pas mes sacs... plus d''herbes pour %group_spell. Prenez %base_spell pour l''instant !', '', '', '', '', '', ''),
  ('rp_missing_reagent_arcane_brilliance',
    'Out of Arcane Powder... %group_spell will have to wait. Casting %base_spell!', 0, 0,
    '', 'Plus de poudre des arcanes... %group_spell attendra. Je lance %base_spell !', '', '', '', '', '', ''),
  ('rp_missing_reagent_generic',
    'Oops, I’m out of components for %group_spell. We’ll go with %base_spell!', 0, 0,
    '', 'Oups, je n''ai plus de composants pour %group_spell. On fera avec %base_spell !', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
  ('rp_missing_reagent_greater_blessing', 100),
  ('rp_missing_reagent_gift_of_the_wild', 100),
  ('rp_missing_reagent_arcane_brilliance', 100),
  ('rp_missing_reagent_generic', 100);
