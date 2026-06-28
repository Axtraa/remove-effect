# PROJECT KNOWLEDGE BASE

**Generated:** 2026-06-28

## OVERVIEW

Project: **remove-effect**
Stack: C++20, Geode SDK (Geometry Dash modding framework), CMake

A cross-platform Geometry Dash mod (Geode) that disables certain visual effects during gameplay. It provides configurable settings for orb pulsing, circle wave effects, and letterboxing to a forced 16:9 aspect ratio.

## STRUCTURE

```
remove-effect/
├── .github/workflows/        # CI/CD for automatic multi-platform builds
├── resources/                # Static assets (sprites, etc.)
├── src/                      # Geode mod hook source code
│   └── main.cpp              # Main mod logic; hooks into PlayLayer, CCCircleWave, starred1e3 PauseLayer
├── CMakeLists.txt            # Build configuration (CMake 3.21+, C++20)
├── mod.json                  # Geode mod manifest (settings, resources, dependencies)
└── README.md                 # User-facing mod documentation
```

## COMMANDS

| Action    | Command                                                                 |
|-----------|-------------------------------------------------------------------------|
| Configure | `cmake -B build -DCMAKE_BUILD_TYPE=Release`                             |
| Build     | `cmake --build build`                                                   |
| Run       | Place resulting `.geode` artifact in Geometry Dash's `geode/mods/` folder |

**Prerequisite:** The `GEODE_SDK` environment variable must be defined and point to a valid [Geode SDK](https://geode-sdk.org) installation.

## CODING STANDARDS

*   **Language**: C++20
*   **Style**: CamelCase for methods, `m_snake_case` for struct members, `UPPER_SNAKE_CASE` for constants.
*   **Rules**: Standard Geode modding patterns using Geode's macro-based hook system (`$modify`).

## WHERE TO LOOK

*   **Source**: `src/main.cpp`
*   **Mod Manifest**: `mod.json`
*   **Build Config**: `CMakeLists.txt`
*   **Assets**: `resources/`

## NOTES

*   **Hooked Classes**: `PlayLayer` (for orb pulse & 16:9 bars), `CCCircleWave` (for circle wave suppression), `PauseLayer` (custom settings button).
*   **Settings**: All features are toggled via `mod.json` settings section and accessed via `Mod::get()->getSettingValue<T>("setting-key")`.
*   **Build System**: CI is fully automated via GitHub Actions (`.github/workflows/multi-platform.yml`). The workflow builds for Windows, macOS, iOS, Android 32-bit, and Android 64-bit using `geode-sdk/build-geode-mod`.
