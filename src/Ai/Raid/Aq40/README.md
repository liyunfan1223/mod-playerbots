# AQ40 raid strategy (Temple of Ahn'Qiraj)

Bot behaviour for the Temple of Ahn'Qiraj, focused on the two encounters that stop most bot raids:
**Twin Emperors** and **C'Thun**. Other bosses get lighter handling (see the end of this file).

Everything keys on roles, classes and raid marks, never on character names, so it works with any mix of random bots.

## Turning it on

Nothing to configure. Inside the Temple of Ahn'Qiraj (map 531), bots load the `aq40` strategy automatically, in combat
and out of combat. To check a bot, whisper it `co ?` and look for `aq40`.

## Raid marks at a glance

| Mark | Put it on | Effect |
|---|---|---|
| **Moon** (Twin Emperors) | A **warlock** bot | Tanks Vek'lor on the north side. Walks to its pre-pull spot by itself. |
| **Square** (Twin Emperors) | A second **warlock** bot | Tanks Vek'lor on the south side. Walks to its waiting spot by itself. |
| **Moon** (C'Thun) | A **tank** bot | Walks in alone and pulls the Eye of C'Thun. |
| **Skull** (Bug Trio) | Kri, Yauj or Vem | That bug is killed next, after the Brood adds. |

Moon means different things in the two fights: on a warlock it matters only in the Twin Emperors room, on a tank only
in C'Thun's room (and only once the Twin Emperors are dead). Moving the mark from the warlock to the tank after the
Twins kill is all it takes.

The marks must be set by the raid leader or an assistant.

## Twin Emperors

### Before the pull
1. Kill all five **Anubisath Defenders** in the room first. They patrol, and a missed one walks into the fight.
2. Bring **two warlocks with Voidwalkers**. Mark one **Moon** and the other **Square**.
3. Have priests keep **Shadow Protection** up. Tell them in raid chat: `@priest nc +rshadow`.
4. Keep the bots on normal **follow**. Don't use `stay`; it breaks their positioning in the fight.
5. Out of combat, the Moon warlock walks to about 31 yd from Vek'lor's pedestal (just outside his aggro reach, inside
   Searing Pain range), and the Square warlock walks to the south waiting spot. Wait until both are in place.

### The pull
- A tank pulls **Vek'nilash** (melee emperor). The Moon warlock opens on **Vek'lor** (caster emperor) with Searing
  Pain at the same moment.

### What the bots do
- **Floor spots:** both emperors are brought down off their pedestals to two floor spots about 118 yd apart (south
  −9016, 1262 and north −8900, 1284). Keeping them over 60 yd apart is what stops them healing each other.
- **Vek'lor:** tanked by the warlock on his side (Moon north, Square south). Warriors only hold him for the few
  seconds after a teleport. The waiting warlock stands at his side's spot, using Life Tap, and Shadow Ward before the
  teleport window.
- **Vek'nilash:** warriors always bring him to their side's floor spot. Spare tanks stay second on threat and take over
  if his tank dies.
- **Teleports** (every 30–40 s): melee leave the landing spots for the middle of the path just before each teleport.
  After a teleport, the emperors attack the nearest player, so tanks are positioned to be nearest.
- **Arcane Burst:** everyone except Vek'lor's tank keeps clear of Vek'lor.
- **Blizzard:** bots step out of it in one move and fan out, so they don't stack up along one escape line.
- **Healers** stand between their tank and their side's warlock.

### For a human player
- Stay out of melee range of Vek'lor (Arcane Burst hits within about 5 yd and knocks you far back).
- If an emperor targets you, drop threat (Fade, Feign Death, Vanish, Ice Block).
- Don't pull an emperor toward the middle: if they get within about 60 yd of each other, they heal each other.

## C'Thun

### Before the pull
1. The **Twin Emperors must be dead** this lockout. C'Thun can't be engaged before that.
2. Clear the trash in the tunnel on the way in (Qiraji Brainwashers, Vekniss Warriors and Guardians).
3. Bring at least one **tank bot**. Have class buffs up. Optionally, whisper one warlock `co +curse of elements`;
   affliction and demonology warlocks default to Curse of Agony.
4. Stop the raid at the **top of the south-southeast entrance ramp** (the only slope down to the floor), about
   80–105 yd from the Eye. Keep the bots on normal **follow**; don't use `stay`.
5. Put the **Moon** mark on the tank bot. Moon only works on a tank, within about 140 yd of C'Thun and on the room
   level.

### The pull
- The Moon tank walks alone to a spot at the room's west edge. The Eye sees him and engages about 6 s after the mark.
  The first three Eye Beams always hit him, and he is far from everyone else.
- While he walks in, the other bots follow him down the slope about 22 yd behind, never nearer the Eye than he is, so
  they reach their spots sooner. If he stops, they stop. Human players should wait until the Eye is fighting him.
- Once the fight starts, bots stop following and run to their own spots. Follow is ignored for the whole fight.

### Phase 1 — Eye of C'Thun
- **Spread:** every raid member gets a spot on three rings around the Eye, at least 13 yd apart, because Eye Beam
  jumps to anyone within about 13 yd and grows 50% per jump.
  - Four melee stand on the Eye (north, east, south, west, about 23 yd apart).
  - Ranged and the remaining melee take the middle ring (30–42 yd). Spare melee only attack tentacles that come up
    near their spot.
  - Healers take the outer ring.
- **Dark Glare:** bots dodge the sweeping beam, then re-sort onto the nearest free spot of their kind. Where the floor
  ends (48–64 yd from the Eye in places, over a 17 yd drop), a dodge steps closer to the Eye instead.
- **Targets:** Eye Tentacles first, then Claw Tentacles, then the Eye.
- **Pets** are kept; they don't carry Eye Beam.

### Phase 2 — C'Thun
- **The stack:** everyone gathers about 14 yd due south of C'Thun (−8592.8, 1986.2), a tight group about 5 yd
  across, outside his 7 yd center circle.
- **Giant Eye Tentacle** (highest priority): interrupted or stunned the moment it starts casting, by whoever has a
  kick, pummel, shield bash, wind shear, counterspell, mind freeze, silencing shot, hammer of justice, concussion blow,
  bash, war stomp, kidney shot or gouge ready. If one spawns more than 20 yd from the stack, the whole stack moves
  next to it and kills it there.
- **Eye Tentacles:** two ranged bots (hunters first, then mages, then other ranged) take the Eye Tentacles more than
  30 yd from the stack, out of reach for casters standing in it, even during Weakened windows. They step out just far
  enough to reach each one and return when it dies; everyone else handles the near ones from the stack.
- **Giant Claw Tentacle:** tanks take claws near the raid. **Nobody chases a claw more than 20 yd from the stack**;
  with nobody near it, it burrows and comes back up under someone anyway.
- **The stomach:** C'Thun swallows a random player about every 14 s.
  - Swallowed bots kill the Flesh Tentacles. Swallowed healers attack them too (druids leave Tree of Life to do it).
  - Bots leave at 6 Digestive Acid stacks (9 if a healer is inside with them), below 55% health, or once both Flesh
    Tentacles are dead. Swallowed tanks leave at once, because Giant Claws need them.
  - The exit pad is at the north end of the stomach, about 33 yd north of the point under C'Thun. Standing on it for
    3 s throws you up onto C'Thun, and his center circle knocks you back out.
- **Weakened window:** when both Flesh Tentacles die, C'Thun is Weakened for 45 s (raid emote
  "C'Thun is weakened!" and a buff on him). Only now does he take damage.
  - Everyone burns him: ranged from the stack; melee move to about 8 yd from his center and strike only from within
    10 yd.
  - A Giant Eye Tentacle still dies first. A Giant Claw Tentacle that a tank is holding is left to that tank; one that
    nobody holds is killed first.
  - With the raid mostly alive, each window takes 35–40% of his health, so about three windows kill him.

### For a human player
- **Phase 1:** find a gap about 40 yd from the Eye, at least 13 yd from anyone, and step out of the red Dark Glare beam.
- **Phase 2:** join the stack south of C'Thun. Save interrupts for Giant Eye Tentacles. Burn C'Thun during Weakened
  windows and save cooldowns for them.
- **If swallowed:** kill the Flesh Tentacles, leave at about 6 stacks, then run straight back to the stack.

## Other bosses

These bosses are handled automatically too. They have had less full-raid testing than the two above. No raid marks
are needed except Skull on the Bug Trio.

- **Trash (Anubisath Defenders):** each Defender reflects two spell schools for its whole fight (Shadow + Frost or
  Fire + Arcane). Casters don't cast those schools at it.
- **The Prophet Skeram:**
  - Each tank takes one of the three platforms. When Skeram splits, each image is tanked on its own platform.
  - The raid is split across the platforms by role, so every platform gets healers, mages and rogues for interrupts.
  - Arcane Explosion is interrupted.
  - Mind-controlled raid members are crowd-controlled (Polymorph, Hammer of Justice or Cyclone) instead of attacked.
  - Non-tanks don't use area damage.
- **Bug Trio:**
  - Kill order is Yauj's brood adds, then Kri, Yauj, Vem. Put **Skull** on a bug to kill it next instead.
  - Tanks keep the three bugs at least 18 yd apart without dragging them away from their healers.
- **Battleguard Sartura:** her guards die first. While she or a guard is whirlwinding, bots stop attacking it and keep
  18 yd away.
- **Fankriss the Unyielding:** his spawned worms die first. Tanks swap at 3 Mortal Wound stacks.
- **Viscidus:**
  - Casters freeze him with frost spells (rank 1 Frostbolt for the most hits, Frost Shock, Icy Touch, Ice Lance).
  - Once he's frozen, melee move in to shatter him. Globs die first.
  - Everyone avoids the Toxin clouds. Ranged and healers keep a ring around him, off the entrance ramp.
- **Princess Huhuran:**
  - Tanks swap at 5 Acid Spit stacks.
  - Hunters use Tranquilizing Shot on Frenzy.
  - Wyvern Sting is cleansed from tanks while they're above 4,500 health (the dispel deals 3,000).
  - Enough raid members stand close to soak each Poison Bolt Volley.
  - Damage cooldowns are saved for the last 30%.
- **Ouro:**
  - Everyone avoids his frontal cone and the dirt mounds. Scarabs die first.
  - After Sand Blast wipes a tank's threat, the next tank takes him over, and backup tanks keep building threat.

## Known limitations

- **Twin Emperors:** after a teleport, the warrior Vek'lor lands next to can keep him, and Arcane Burst knockbacks
  can walk Vek'lor toward the middle. This costs separation but hasn't caused a wipe on its own.
- **C'Thun:**
  - A Giant Eye Tentacle that spawns inside the stack and gets its first beam off can kill most of the raid at once.
  - During a Weakened window C'Thun outranks Giant Claws, so a claw in the stack goes unanswered if no tank picks it up.
  - The stomach clears one Flesh Tentacle at a time, so Weakened windows come 2–4 minutes apart.
- **Server settings:** an emperor that can't reach his target (for example, a tank who fell off the room) regenerates
  quickly unless `NpcRegenHPIfTargetIsUnreachable = 0` in `worldserver.conf`. Tanking on the floor spots avoids this.

## Test results

Measured on a level-60 server with a 40-bot raid (one human player), production difficulty:
- **Twin Emperors:** 10 kills in 11 full-raid test pulls with the floor-spot strategy; killed live.
- **C'Thun:** killed live on the fourth live attempt, with 33 of 40 alive at the kill. With the Eye Tentacle killers:
  5 kills in 6 live-style test pulls, against 4 in 8 without them. With the floor-safe glare dodge, the Weakened
  target order and the raid trailing the puller as well: 5 kills in 6 with no pull lost in phase 1, then a confirming
  kill; killed live again on the second attempt, with 29 of 40 alive.
