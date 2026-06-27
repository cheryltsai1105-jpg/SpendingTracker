# Spending Tracker — C++ / Qt

A three-tab desktop app for tracking spending, managing goals, and reviewing history.

---

## Tabs

### Tab 1 — Tracker
- Interactive pie chart (left): one slice per transaction, randomised golden-ratio colours
- Hover a slice → tooltip shows **title · $amount · % of total**
- Hovered slice pops outward
- **Total Spending** label updates live
- Entry box (bottom-right): Title + Amount → Enter or "Add Entry"
- Pie chart **resets** when a goal cycle ends (success or failure) so the next goal starts fresh

### Tab 2 — Goals & Analysis
- **Bar chart** (left half): Mon–Sun for the current calendar week, bars = daily spending
  - Hover a bar → tooltip shows `$xx.xx` for that day
  - Dashed red line = adaptive daily budget
  - Date range shown below chart; chart auto-advances every Monday
- **Right panel**:
  - Goal summary label: `Goal: $500 / 1 week · Spent so far: $xx.xx`
  - Daily budget label: `$71.43 / day  (remaining: $xxx.xx)` — recalculated after every entry
  - Entry box: Spending goal ($) + Time range (count + day/week/month/year)
  - Entry box **locks** while a goal is active; **unlocks** on success or failure
- On success (under/at budget when deadline reached): 3-second congratulations pop-up
- On failure (over budget at any point): warning dialog; entry box unlocks for a new goal

### Tab 3 — History
- Permanent, ever-growing table: **Title | Amount | Date & Time**
- Appends a new row every time an entry is added in Tab 1
- Data is stored in its own `QSettings` key — **never cleared** by goal resets
- Rows are sorted oldest-first; newest entry scrolls into view automatically

---

## Data persistence

| What | Cleared when | Storage key |
|------|-------------|-------------|
| Pie chart entries | Goal resets (success/fail) | `entries[]` |
| Active goal | Goal resets | `goal/*` |
| History rows | Never | `history[]` |

Storage location:
- **Windows** – Registry `HKCU\Software\SpendingTrackerApp\SpendingTracker`
- **macOS** – `~/Library/Preferences/com.SpendingTrackerApp.SpendingTracker.plist`
- **Linux** – `~/.config/SpendingTrackerApp/SpendingTracker.conf`

---

## Build requirements

| Tool | Version |
|------|---------|
| C++ compiler | GCC 10+, Clang 12+, or MSVC 2019+ |
| CMake | 3.16+ |
| Qt | 6.x (or 5.15 — uncomment Qt5 lines in CMakeLists.txt) |

### Install Qt

**Windows** — https://www.qt.io/download-qt-installer → Qt 6.x Desktop  
**macOS** — `brew install qt@6`  
**Linux** — `sudo apt install qt6-base-dev cmake g++`

### Build & run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./SpendingTracker        # Linux / macOS
SpendingTracker.exe      # Windows
```

---

## File map

| File | Purpose |
|------|---------|
| `main.cpp` | Entry point |
| `MainWindow.*` | Tab container + cross-tab signal wiring |
| `DataStore.h` | Shared data model (entries, goal) + QSettings persistence |
| `PieChartWidget.*` | QPainter pie chart — hover tooltip with title, $, % |
| `BarChartWidget.*` | QPainter bar chart — Mon–Sun, hover tooltip, budget line |
| `TrackerTab.*` | Tab 1 — pie chart + entry box + total label |
| `GoalsTab.*` | Tab 2 — bar chart + goal entry + adaptive daily budget |
| `HistoryTab.*` | Tab 3 — permanent QTableWidget transaction log |
| `CMakeLists.txt` | CMake build file |
