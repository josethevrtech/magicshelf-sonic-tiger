# Sonic Tiger LCD

Independent MagicShelf creation. Work in progress; physical headset validation pending.

Build with `python3 tools/fetch_sdk.py` and `./build.sh`. Dependencies: C++17, GCC, Qt6 Widgets, Cairo, EGL/GLES3, json-c, PulseAudio simple, OpenVR. Python asset import uses Pillow and CairoSVG.

Import your local ROM with `python3 tools/import_assets.py --rom /path/to/tsonic.zip --destination /path/to/private/data/content`. Set `MAGICSHELF_DATA_DIR` to that data directory or `SONIC_CONTENT_DIR` for development. ROMs and segment artwork stay out of the distributable package.

Desktop: arrows move/jump, Enter spins, Backspace pauses, F powers on/starts, M opens settings. Frame: directions move/jump, A spins, B pauses, X starts, Y settings. Grip picks up; release places; holding stick adjusts depth; shake or Home returns to dock. Physical interactions require headset validation.

The supplied Mattyew model depicts Sonic 2; the supplied tsonic ROM runs Sonic 1. Sonic 1 background inlay was not supplied, so the screen uses neutral LCD backing. No public catalog entry or release has been published.

For local ARM64 WSL builds, set `OPENVR_LIB_DIR` to your OpenVR `bin/linuxarm64` directory. Frame builds default to `/opt/steamvr/bin/linuxarm64`. Run `./build.sh --tests-only` for checks; private gameplay tests run when `local/content` exists, otherwise shared checks cover wiring and missing-content handling. The review package is intentionally not registered in the public catalog.
