# PROJECT KNOWLEDGE BASE

**Generated:** 2026-06-28

## OVERVIEW

Project: **remove-effect**
Stack: C++23, Geode SDK 5.7.1 (Geometry Dash modding framework), CMake 3.21+

A cross-platform Geometry Dash mod (Geode) that disables visual effects, provides auto-click key simulation, and forces 16:9 aspect ratio during gameplay. Targets GD version 2.2081 on Windows, macOS, iOS, and Android.

## STRUCTURE

```
remove-effect/
├── .github/workflows/        # CI/CD — multi-platform builds (Win, Mac, iOS, Android)
├── resources/                # Static assets
│   └── button.png            # Pause menu button sprite
├── src/
│   └── main.cpp              # All mod logic (single file, ~430 lines)
├── CMakeLists.txt            # Build config (CMake 3.21+, C++23, Geode SDK bindings pinning)
├── mod.json                  # Geode mod manifest — settings, metadata, dependencies
├── README.md                 # User-facing documentation
└── logo.png                  # Mod logo
```

## COMMANDS

| Action    | Command                                                          |
|-----------|------------------------------------------------------------------|
| Build     | `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` |
| Run       | Place `.geode` artifact in Geometry Dash `geode/mods/` folder    |

**Prerequisite:** `GEODE_SDK` environment variable pointing to Geode SDK installation. Local builds discouraged — CI handles all setup.

## CODING STANDARDS

- **Language**: C++23 (`CMAKE_CXX_STANDARD 23`)
- **Style**: Standard Geode patterns — `$modify` macro hooks, `m_snake_case` fields, `CamelCase` classes
- **Platform guards**: `#ifdef GEODE_IS_WINDOWS` for Win32 API (`SendInput`, `GetAsyncKeyState`)
- **VK codes**: Use cross-platform `RK_*` constants (raw hex) instead of `VK_*` from `windows.h` — required for macOS/iOS compilation
- **Cocos2d**: Uses cocos2d-x 2.x API (NOT 3.x). No `EventListenerTouchOneByOne`, `cocos2d::Touch`, etc.
- **Menu items**: Only `CCMenuItemSpriteExtra` works cross-platform. `CCMenuItemLabel`, `CCMenuItemFont` fail to link on iOS
- **Key storage**: `Mod::get()->getSavedValue<T>()` / `setSavedValue()` for persistent data (e.g., recorded key binding)

## KEY ARCHITECTURE

### `$modify(PlayLayer)` — Main gameplay hooks
- `setupHasCompleted()` — Level init (16:9 overlay setup)
- `updateVisibility(float dt)` — Per-frame hook for:
  - Orb pulse suppression (`no-orb-pulse` setting)
  - Auto-click key simulation at configurable percentage
  - 16:9 letterbox overlay toggle
- **Auto-click percentage**: Calculated as `(playerX / (levelLength * 30)) * 100`. Do NOT use `m_level->m_normalPercent` — it returns best %, not real-time progress.

### `$modify(CCCircleWave)` — Circle effect suppression
- `draw()` override — skips rendering when `no-circles` enabled

### `$modify(PauseLayer)` — UI additions
- Settings button (gear icon) → opens Geode settings popup
- Record Key button → opens `KeyRecordLayer` for interactive key binding

### `KeyRecordLayer` — Interactive key recording
- Polls `GetAsyncKeyState()` in `update()` to detect new key presses
- Snapshots key state on init to avoid triggering on already-held keys
- Saves key name via `Mod::get()->setSavedValue("auto-click-key", name)`
- ESC to cancel, any valid key to record

## SETTINGS (mod.json)

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `auto-click` | bool | false | Enable auto key press at percentage |
| `auto-click-percent` | float | 50.0 | Trigger percentage (0-100) |
| `no-orb-pulse` | bool | false | Disable orb pulsing |
| `orb-pulse-size` | float | 0.6 | Orb pulse size (0-1) |
| `no-circles` | bool | false | Disable circle wave effects |
| `force-16-9` | bool | false | Force 16:9 letterbox bars |

**Key binding**: Stored via `getSavedValue("auto-click-key", "F2")`, not in mod.json settings.

## CI/CD

- **Workflow**: `.github/workflows/multi-platform.yml`
- **Trigger**: Push to any branch
- **Platforms**: Windows, macOS, iOS, Android32, Android64
- **Build tool**: `geode-sdk/build-geode-mod@main`
- **Post-change**: Must push to trigger build. Use `git commit --allow-empty -m "ci: trigger build" && git push` if needed.

## GOTCHAS

- `PlayLayer::update` hook does NOT work reliably — use `updateVisibility` for per-frame logic
- `m_level->m_normalPercent` is the "best percentage", NOT real-time progress. Calculate from `m_player->getPositionX()` instead
- cocos2d-x 2.x (GD 2.2) lacks `EventListenerTouchOneByOne`, `cocos2d::Touch`, `cocos2d::Event` — use manual touch delegates or avoid touch listeners
- `CCMenuItemLabel` / `CCMenuItemFont` cause linker errors on iOS — use `CCMenuItemSpriteExtra` for all buttons
- VK_* constants from `windows.h` only exist on Windows — define cross-platform `RK_*` equivalents for macOS/iOS builds
