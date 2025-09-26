UPDATE guild
SET
  EmblemStyle     = FLOOR(RAND() * 181),
  EmblemColor     = FLOOR(RAND() * 18),
  BorderStyle     = FLOOR(RAND() * 8),
  BorderColor     = FLOOR(RAND() * 18),
  BackgroundColor = FLOOR(RAND() * 52)
WHERE EmblemStyle=0 AND EmblemColor=0 AND BorderStyle=0 AND BorderColor=0 AND BackgroundColor=0;