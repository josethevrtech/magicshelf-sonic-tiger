# Sonic Tiger LCD

Play Sonic the Hedgehog 2 from PEXY inside a 3D Tiger handheld on Steam Frame. Pick it up, play, and keep it on your MagicShelf. Free. Internet required. No ROM import.

![Sonic gameplay in the handheld](media/sonic-tiger-gameplay.png)

## Playing

Wait for the game screen, then press **X** or point at the model's **ON/START** button and pull the trigger.

- Directions move and jump; **A** activates Action, **B** pauses, **X** starts, **Y** opens settings.
- Use your free hand to point at the model buttons. Hold the trigger to hold a key; release to let go.
- Both **ATTACK** halves and **ACTION** activate the same Action input. **ACL** resets the game.
- Hold grip to pick up; release to place. The holding stick adjusts distance. Home or a deliberate shake returns it to the shelf.
- **Y** opens sound, volume and Reload Online Player. Loading failures offer Retry.

The casing and hosted game are Sonic 2. The original handheld's sound cues are intermittent. Settings and browser saves persist locally. Off and reload explicitly save first; saves depend on the hosted player version. PEXY availability and loading times can vary.

## Credits and source links

This work is based on **[Tiger Handheld Games Console](https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c)** by **[Mattyew](https://sketchfab.com/mattyu)**, licensed under **[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)**. The original supplied author link, https://sketchfab.com/mrgrandgrind, is retained in [ASSET_CREDITS.md](ASSET_CREDITS.md). Changes: converted/scaled mesh and textures, removed the baked screen, added live screen rendering and interactive button targets.

- [PEXY Sonic 2 player](https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/): hosted emulator, game content and original screen background.
- [MAME](https://www.mamedev.org/): MAMEDev and contributors' emulation, provided by the hosted player.
- [Qt WebEngine](https://doc.qt.io/qt-6.10/qtwebengine-licensing.html): Qt Project and Chromium contributors; dynamically linked SteamOS browser runtime.
- [Valve OpenVR](https://github.com/ValveSoftware/openvr): VR integration, BSD-3-Clause.
- [MagicShelf](https://github.com/josethevrtech/magicshelf): SDK and framework, MIT. Integration by josethevrtech.
- [bzhxx / hap](https://github.com/bzhxx/LCD-Game-Emulator): retained native-core development reference; not the online backend.

Sega/Tiger game material, characters and trademarks retain their respective rights. No ROM, melody ROM or SVG game artwork is packaged. The model license does not relicense the game. This is a fan-made integration, not an official Sega, Tiger Electronics or PEXY product. See [ASSET_CREDITS.md](ASSET_CREDITS.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), [LICENSE](LICENSE), [SDK-LICENSE.txt](SDK-LICENSE.txt) and [OPENVR-LICENSE.txt](OPENVR-LICENSE.txt).

## Development and release

Use `python3 tools/fetch_sdk.py`, then `./build.sh`. Requires C++17/GCC, Qt6 Widgets/WebEngine Widgets, Cairo, EGL/GLES3, json-c, PulseAudio simple and OpenVR. The release target is Steam Frame ARM64 on SteamOS, tested with Qt 6.10.2. Override `OPENVR_LIB_DIR` for local WSL builds.

`./build.sh --tests-only` checks measured model hit targets and the retained native reference. Private reference-ROM tests run only when `local/content` exists. `SONIC_NO_AUDIO=1 ./build.sh --online-tests` checks the actual online player, short taps, independent holds, settings and offline handling without making headset sound.

The sandboxed browser runs separately from VR tracking, with memory-only HTTP cache and no persistent cookies. Game content is loaded by PEXY's dedicated tsonic2 player. HTTPS requests are restricted to its game and player resource paths. Downloads, popups and browser permissions are denied. A private shared buffer exchanges completed frames and input; the tracking thread never waits on browser work.

Screenshots are real native renders using a completed frame from the actual online player and the actual settings UI. [Capture provenance and image credits](media/README.md). [Validation details](FIDELITY.md).

[Download the release](https://github.com/josethevrtech/magicshelf-sonic-tiger/releases/latest) or follow its catalog in MagicShelf: https://github.com/josethevrtech/magicshelf-sonic-tiger/releases/latest/download/catalog.json . See the [release review](release/RELEASE_REVIEW.md) for validation and remaining limits.
