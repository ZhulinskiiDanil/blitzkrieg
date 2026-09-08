# Blitzkrieg

**Blitzkrieg** turns the Blitzkrieg training method into an automatic practice tracker for Geometry Dash.

No spreadsheets. No manual run tracking.

Create a profile, play the level, and let the mod track your **runs, stages, attempts, playtime, and progress** automatically.

## Features

- **Automatic run tracking** during gameplay
- **Attempts and playtime** tracked per range and stage
- **Best Run** tracking for each range
- **First Clear** statistics
- **Repeated passes** and pass-rate tracking
- **Stage Browser** for completed and upcoming stages
- **Stage Graph** for attempts and playtime
- **Multi-Level Profiles** for sharing progress between level copies
- **Noclip and speedhack detection** so invalid runs can be ignored
- **Custom notifications and SFX**
- **Streamer Mode**
- **Curated startpos copies**

## Quick Start

1. Create or find a level copy with **start positions**. Around **12-18 start positions** is recommended.
2. Open Blitzkrieg and create a profile.
3. Blitzkrieg generates the training stages automatically.
4. Start playing.

The same profile can be connected to multiple levels, for example a startpos copy and the original level.

## How Runs Are Assigned

Each gameplay attempt is assigned to exactly one range. Only that range receives `attempts`, `timePlayed`, `bestRun`, and a possible `PASS`.

Blitzkrieg first collects all ranges actually touched by the run. If at least one touched range is unchecked, checked ranges are ignored for target selection.

Among touched unchecked ranges, the mod first prefers a range the run can fully complete. If several are passable, the range with the smallest `from` is selected; if `from` is equal, the smaller range is preferred.

If no unchecked range can be completed, the attempt is assigned to the unchecked range that was covered the most proportionally by the run.

If all touched ranges are already checked, the same idea is applied to checked ranges. A fully covered checked range still counts as another pass.

```text
Touched(range) =
    min(runEnd, to) - max(runStart, from) > eps

Passable(range) =
    runStart <= range.from + eps
    &&
    runEnd + eps >= range.to

Coverage(range) =
    overlap / (range.to - range.from)

1 attempt -> 1 target range
```

### Examples

```text
20->40 | 10->30 unchecked, 20->30 unchecked, 30->40 unchecked
-> 20->30 stats + PASS

25->28 | 10->30 unchecked, 20->35 unchecked, 25->40 unchecked
-> 25->40 stats

40->60 | 30->50 unchecked, 50->70 unchecked
-> 30->50 stats

65->100 | 50->70 checked, 70->85 unchecked, 85->100 unchecked
-> 70->85 stats + PASS

0->100 | 0->20 checked, 20->40 checked, 40->60 unchecked,
          60->80 unchecked, 80->100 unchecked
-> 40->60 stats + PASS
```

The important rule is simple: one attempt always belongs to one target range. Stats and `PASS` are never split between different ranges.

## Recommended Setup

For the most reliable tracking, using **2.1% percentages** while practicing Blitzkrieg is recommended.

The mod can be used together with `zsa.percentage-toggle`.

## Tutorials

- **English:** https://youtu.be/v2CqPpV1kds
- **Spanish:** https://youtu.be/3zeuT4ggzZk
- **Russian:** https://youtu.be/d6jAYVD1144

## Credits

The **Blitzkrieg training method** was originally created by [Rudder](https://www.youtube.com/@TheRealRudder).

Blitzkrieg automates the tracking workflow around that training method.

## Support

Found a bug or need help?

- **Discord:** https://discord.gg/duuG4RqJKh
- **Homepage:** https://www.blitzkrieg-mod.com
- **Source:** https://github.com/ZhulinskiiDanil/blitzkrieg

Bug reports and feature requests can also be submitted through GitHub Issues.
