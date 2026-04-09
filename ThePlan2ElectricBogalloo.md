# ASCII Clash — Project Plan & Architecture Summary

## Project Overview

A C++ web-based monster battle game hosted on a local university laptop server. Players manage a team of monsters, fight AI opponents in roguelike singleplayer runs, and can challenge other players to async turn-based multiplayer matches. The server uses **cpp-httplib** for HTTP and **SQLite3** for persistence.

---

## Tech Stack

| Layer | Technology |
|---|---|
| HTTP Server | cpp-httplib |
| Database | SQLite3 (raw C API, no wrapper) |
| Templating | inja (header-only, for dynamic HTML) |
| Frontend | Static HTML + shared CSS, served via `set_mount_point` |
| Language | C++20 |

**File structure:**
```
ASCII-Clash/
├── Server/
│   ├── HTML/
│   │   ├── style.css        ← shared styles for all pages
│   │   ├── index.html       ← login page
│   │   └── ...              ← future pages
│   └── main.cpp
├── GameLogic/
│   ├── Monsters/
│   │   ├── Monsters.h / .cpp
│   │   ├── StatDict.h
│   │   └── MathHelpers.h
│   └── Fighting/
│       └── Logger.h
└── Database/
    └── (generated .db file)
```

---

## Database Schema

### Tables

```sql
-- Players
players (
    id          INTEGER PRIMARY KEY,
    username    TEXT,
    password    TEXT,         -- plaintext for now (no hashing)
    score       INTEGER,      -- lifetime leaderboard score
    active_run_id INTEGER     -- NULL if no run active (points to in-RAM run)
)

-- Monsters
monsters (
    id          INTEGER PRIMARY KEY,
    owner_id    INTEGER,      -- FK → players.id
    name        TEXT,
    type        INTEGER,      -- MonsterType enum value
    level       INTEGER,
    exp         INTEGER,
    skill_points INTEGER,
    hp          INTEGER,
    damage      INTEGER,
    offense     INTEGER,
    defense     INTEGER,
    special     INTEGER
)

-- Match log (written at end of each fight within a run)
match_log (
    id          INTEGER PRIMARY KEY,
    run_id      TEXT,         -- links log entries to a run
    player_id   INTEGER,      -- FK → players.id
    player2_id  INTEGER,      -- 2nd player (empty/null if AI??)
    started_at  TEXT,
    ended_at    TEXT,
    events      TEXT          -- serialized Logger output (JSON or newline-separated)
)
```

### DB Initialization Strategy

Two separate executables (or `main` CLI args):
- `./ASCII-Clash` — normal server startup
- `./ASCII-Clash --init-db` (or a separate `./setup` binary) — creates a fresh `.db` file with all `CREATE TABLE IF NOT EXISTS` statements

---

## Architecture

### Data Flow (DTO Pattern)

```
Monster ──implements──> ISavable ──produces──> MonsterData (DTO)
                                                     ↕
                                              Database class
                                                     ↕
                                           sqlite3 raw C API
```

**ISavable interface** — game objects know how to serialize themselves into plain data structs, but know nothing about SQL:
```cpp
struct MonsterData {
    int id, owner_id, level, exp, hp, damage, offense, defense, special, skill_points;
    std::string name;
    MonsterType type;
    bool is_dead;
};

class ISavable {
public:
    virtual MonsterData ToData() const = 0;
    virtual void FromData(const MonsterData &data) = 0;
};
```

**Database class** — owns the sqlite3 connection, all SQL lives here, speaks only in DTOs:
```cpp
class Database {
    sqlite3 *db;
public:
    void SaveMonster(const MonsterData &data);
    MonsterData LoadMonster(int id);
    void UpdateMonster(const MonsterData &data);
    // ... players, match_log etc.
};
```

---

## Existing Game Logic (already built)

### Stats (`StatDict.h`)
Each monster has these stats stored in a `StatDict` (map of Stat enum → int):

| Stat | Default | LevelUp Amount |
|---|---|---|
| Damage | 1 | +1 |
| Offense | 1 | +1 |
| Defense | 1 | +1 |
| Special | 1 | +1 |
| Health | 10 | +2 |
| EXP | 0 | — |
| Level | 1 | — |
| SkillPoints | 0 | — |

### Monster Types (planned)
Type is a `MonsterType` enum. Type affects **behavior** (method overrides) and **starting stats** (set in constructor), not the schema. Examples so far:
- `Human` — overrides `ReceiveAttack` (dodge logic)
- `Orc` — overrides `Attack`
- Planned: type-specific stat bonuses in constructor (e.g. Orc gets +1 offense, +1 damage)

### Logger
```cpp
enum class LogType { event, info, nerd };
// Stores (message, type) pairs, filterable by LogType
```
Each monster holds a `Logger *LogPtr`. During a match, all monsters share the same Logger instance. Log output gets serialized into `match_log.events` at match end.

### Leveling
- `ReceiveEXP(amount)` — adds EXP, auto-levels when `EXP >= Level * 2`
- `TryLevelStat(stat)` — spends a SkillPoint to increase a stat

---

## Singleplayer Roguelike Run

### Loop
```
Start Run
└── Fight vs AI
    └── Win → short heal phase → can Retreat or continue
        └── Continue → next Fight (AI scales in difficulty by fight_number)
    └── Lose → all monsters die permanently, run ends
```

### Rules
- All of a player's monsters enter the run (no squad selection)
- Monsters that are **downed** (HP = 0) during a fight receive a small HP restoration at the start of the next fight
- If a monster is downed and the player **retreats**, that monster **dies permanently**
- If the player retreats with all monsters alive, all survive and get EXP + score
- Only one active run per player at a time (`active_run_id` on player row)

### Score Formula
```
score_gained = (alive_monsters / starting_monsters) * run_score
```

### In-RAM Run State (never persisted mid-run)
```cpp
struct RunState {
    std::string id;
    int player_id;
    int fight_number;         // determines AI difficulty
    int score_so_far;
    std::vector<Monster*> monsters;
    std::map<int, bool> is_downed;   // monster_id → downed this fight
    Match *current_fight;     // null between fights
};
```

---

## Multiplayer (async, chess-style)

- Players can have **multiple concurrent matches**
- Matches live in RAM only; lost on server restart (acceptable — server runs all day)
- Client polls `GET /match/{id}/state` every few seconds to check whose turn it is
- AI opponent is handled purely in code — no DB row for AI
- Match results (winner, exp) written to DB on match end

---

## HTTP Endpoints (planned)

### Static
All files in `Server/HTML/` served automatically:
```cpp
svr.set_mount_point("/", "./Server/HTML");
```

### Dynamic (to be implemented)
```
POST /register           ← create new player
POST /login              ← authenticate, return session
POST /run/start          ← start a singleplayer run
POST /run/fight          ← submit turn action during a fight
POST /run/retreat        ← end run voluntarily, collect reward
GET  /run/state          ← poll current run/fight state
POST /match/challenge    ← challenge another player
POST /match/{id}/action  ← submit a turn in a multiplayer match
GET  /match/{id}/state   ← poll match state (is it my turn?)
GET  /leaderboard        ← fetch top scores
```

---

## TODO List

### Phase 1 — Infrastructure
- [ ] Add `--init-db` CLI argument to `main` (or separate binary) that creates the `.db` and all tables
- [ ] Write `CREATE TABLE IF NOT EXISTS` SQL for all 3 tables
- [ ] Implement `Database` class with sqlite3 connection management
- [ ] Implement `ISavable` interface + `MonsterData` DTO struct
- [ ] Implement `Monster::ToData()` and `Monster::FromData()`
- [ ] Implement `Database::SaveMonster()`, `LoadMonster()`, `UpdateMonster()`
- [ ] Implement `PlayerData` DTO + player save/load in Database class

### Phase 2 — Auth & Players
- [ ] `POST /register` endpoint
- [ ] `POST /login` endpoint (plaintext password check for now)
- [ ] Session handling (simple token or cookie — decide approach)
- [ ] Register / Login HTML pages

### Phase 3 — Singleplayer Run
- [ ] `RunState` struct in RAM
- [ ] `RunManager` class — creates, stores, retrieves runs by player ID
- [ ] Wire existing `Monster` + fight logic into a `Match` object
- [ ] AI turn logic (target lowest HP enemy)
- [ ] `POST /run/start` — create run, load player's monsters
- [ ] `POST /run/fight` — submit player turn, resolve AI turn, return log
- [ ] `POST /run/retreat` — calculate score, apply EXP, mark dead monsters, write match_log
- [ ] Heal phase between fights
- [ ] Difficulty scaling per `fight_number`
- [ ] Run HTML pages (fight screen, retreat confirm, results)

### Phase 4 — Multiplayer
- [ ] `MatchState` struct in RAM
- [ ] `MatchManager` class — keyed by match ID
- [ ] `POST /match/challenge`
- [ ] `POST /match/{id}/action`
- [ ] `GET /match/{id}/state` (polling endpoint)
- [ ] Match result written to DB on finish
- [ ] Match/lobby HTML pages

### Phase 5 — Polish
- [ ] Leaderboard page (`GET /leaderboard`)
- [ ] Monster stat leveling UI (spend SkillPoints after match)
- [ ] More monster types with unique behaviors
- [ ] Timeout/cleanup for abandoned matches