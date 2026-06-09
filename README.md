<div align="center">
  <img src="logo.png" alt="Remove Effect" width="200" />
  <h1>Remove Effect</h1>
  <p>Disable visual effects and force 16:9 in Geometry Dash.</p>

  [![Build](https://github.com/Axtraa/remove-effect/actions/workflows/multi-platform.yml/badge.svg)](https://github.com/Axtraa/remove-effect/actions/workflows/multi-platform.yml)
  [![Geode](https://img.shields.io/badge/Geode-5.7.1-blue)](https://geode-sdk.org)
  [![GD](https://img.shields.io/badge/Geometry%20Dash-2.2081-orange)](https://geometrydash.com)
</div>

---

## Features

| Feature | Description |
|---------|-------------|
| **No Orb Pulse** | Disables the pulsing effect on orbs — cleaner gameplay visuals |
| **Orb Pulse Size** | Fine-tune the orb pulse size when not fully disabled |
| **No Circles** | Removes all circle wave effects during gameplay |
| **Force 16:9** | Adds black letterbox bars for a consistent 16:9 aspect ratio on any screen |

## Installation

### Via Geode (recommended)

1. Open **Geometry Dash** with the [Geode loader](https://geode-sdk.org) installed
2. Go to **Mods** → **Download Mods**
3. Search for **Remove Effect**
4. Click **Install**

### Manual

Download the latest `.geode` file from the [Releases](https://github.com/Axtraa/remove-effect/releases) page and drop it into your Geode mods folder (`geode/mods/`).

## Usage

1. Open Geometry Dash
2. Play any level
3. Press **Pause** during gameplay
4. Click the **gear icon** in the pause menu (bottom-right) to open Remove Effect settings
5. Toggle features on/off

All settings can also be accessed from the Geode mod settings menu.

## Platforms

| Platform | Status |
|----------|--------|
| Windows | ✅ |
| macOS | ✅ |
| iOS | ✅ |
| Android (ARM64) | ✅ |
| Android (ARM32) | ✅ |

## Building from Source

> **Note:** Local builds are discouraged. CI builds via GitHub Actions handle all setup automatically.

```bash
# Requires GEODE_SDK env var pointing to your Geode SDK installation
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The `.geode` artifact will be in the `build/` directory.

### GitHub Actions

Simply push to any branch — CI builds all platforms automatically.

```bash
git add .
git commit -m "feat: your change"
git push
```

## Dependencies

- [Geode SDK](https://geode-sdk.org) ≥ 5.7.1
- [geode.node-ids](https://geode-sdk.org/mods/geode.node-ids) ≥ 1.20.0

## License

This project does not currently have a license. All rights reserved.
