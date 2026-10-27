# Sonic Tiger LCD

A MagicShelf handheld powered by the Internet Archive player at https://archive.org/details/hh_tsonic. Open the creation, wait for the online player, then press X or the model ON/START button. Internet is required; there is no ROM import step and no game ROM is packaged.

Frame controls remain: directions move/jump, A spins, B pauses, X powers on/starts, Y opens settings. Grip picks up; release places; holding stick adjusts depth. Shake or Home returns to the shelf.

Point and hold the trigger on the model D-pad to move. OFF, SOUND, PAUSE, ON/START and ACL now use their actual mesh locations. Both ATTACK halves activate Sonic 1 Spin; ACTION duplicates Jump Up. The casing depicts Sonic 2 while the game is Sonic 1. Release the trigger to release the key. Tracking loss clears held model input. Model and controller input can be combined.

Y opens sound, volume and Reload Online Player. Connection failures show a retry screen. Settings persist. Online game state restarts when the player is reloaded or the creation is turned Off; previous native-backend saves remain untouched.

Development: run python3 tools/fetch_sdk.py, then ./build.sh. Dependencies: C++17/GCC, Qt6 Widgets and WebEngine Widgets, Cairo, EGL/GLES3, json-c, PulseAudio simple and OpenVR. Frame uses /opt/steamvr/bin/linuxarm64; local WSL builds can set OPENVR_LIB_DIR.

./build.sh --tests-only checks model hit targets and the retained native reference core. Private reference-ROM tests run only if local/content exists. ./build.sh --online-tests additionally exercises the live player, held model input and offline handling, using temporary data without local ROM content.

The sandboxed browser runs in a separate process with memory-only HTTP cache and no persistent cookies. Tracking reads completed frames and sends input through a private shared buffer. Archive MAME runs in the browser; this is not remote video streaming. Archive.org supplies its player, emulator and game content through its own embed.

Private review installation uses the installed host package transaction and preserves data/backups. Turn this creation Off before updating. No public catalog entry or release has been published.
