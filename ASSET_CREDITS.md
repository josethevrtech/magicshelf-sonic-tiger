# Asset credits

This work is based on "Tiger Handheld Games Console" (https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c) by Mattyew (https://sketchfab.com/mrgrandgrind), licensed under CC-BY-4.0 (http://creativecommons.org/licenses/by/4.0/).

Modifications: converted to MagicShelf mesh, scaled to 140 mm width, removed two baked screen triangles, converted base texture, replaced screen with live LCD output. Casing artwork is Sonic 2. Attribution does not establish separate trademark or game-artwork rights.

CPU: LCD-Game-Emulator by bzhxx, pinned in emulator.lock.json; SM5xx implementation by hap, derived from MAME, BSD-3-Clause headers retained. System/save glue GPL-3.0-or-later; combined application GPL-3.0-or-later. Sonic host wiring, rendering and runtime adapters added here. SDK and reused creation shell MIT, see SDK-LICENSE.txt. OpenVR: OPENVR-LICENSE.txt.

User-supplied n71.program, n71.melody, tsonic.svg and generated segment images are private local content, excluded from public commits and release packages. Sega/Tiger game material is not relicensed by this project.

Online backend (0.2.0): Internet Archive hosted Emularity/MAME, https://archive.org/details/hh_tsonic and https://archive.org/embed/hh_tsonic. Emularity source: https://github.com/db48x/emularity (GPL-3.0). The official embed fetches player/emulator/game content; none is packaged here. The retained bzhxx native backend is a development reference, not the online executable. Qt WebEngine is dynamically linked; Chromium remains sandboxed.
