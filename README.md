# Blitzkrieg

**Blitzkrieg** turns the Blitzkrieg training method into an automatic practice tracker for Geometry Dash.

No spreadsheets. No manual run tracking.

Create a profile, play the level, and let the mod track your **runs, stages, attempts, playtime, and progress** automatically.

- **Mod ID:** `zhulis.blitzkrieg`
- **Version:** `v2.2.5`
- **Geode:** `5.10.1`
- **Geometry Dash:** `2.2081`
- **Platforms:** Windows, Android, macOS, iOS

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

Each gameplay attempt is assigned to exactly one range.

First, all ranges actually overlapped by the run are collected. If any overlapping range is unchecked, the first unchecked range is selected. If all overlapping ranges have the same state, the first range by `from` is selected.

Only the selected range receives `attempts`, `timePlayed`, `bestRun`, and a possible `PASS`.

A `PASS` is counted only when the run fully covers the selected range: the run starts no later than `from` and ends no earlier than `to`.

### Formal Rules

```text
Touched(range) =
    min(runEnd, to) - max(runStart, from) > eps

target =
    first unchecked from touched
    else first touched

PASS =
    runStart <= target.from + eps
    &&
    runEnd + eps >= target.to

1 attempt -> 1 target range
```

### Examples

```text
0->15 | 0->10 unchecked, 10->20 unchecked
-> 0->10 stats + PASS

0->15 | 0->10 checked, 10->20 unchecked
-> 10->20 stats
```

```text
40->60 | 30->50 unchecked, 50->70 unchecked
-> 30->50 stats

40->60 | 30->50 checked, 50->70 unchecked
-> 50->70 stats
```

```text
0->28 | 0->14.59 checked, 14.59->30.81 checked, 30.81->47.03 unchecked
-> 0->14.59 stats + PASS

0->35 | 0->14.59 checked, 14.59->30.81 checked, 30.81->47.03 unchecked
-> 30.81->47.03 stats
```

```text
0->100 | 0->20 checked, 20->40 checked, 40->60 unchecked,
          60->80 unchecked, 80->100 unchecked
-> 40->60 stats + PASS
```

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
- **Homepage:** https://dgkr-community.vercel.app/blitzkrieg
- **Source:** https://github.com/ZhulinskiiDanil/blitzkrieg

Bug reports and feature requests can also be submitted through GitHub Issues.
