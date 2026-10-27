# Sonic Tiger LCD v0.2.3

Play Sonic the Hedgehog through Internet Archive's online player inside a 3D Tiger handheld on Steam Frame. Free, with no ROM import. Internet is required to load the player.

- Pick up the handheld, place it nearby, or return it to MagicShelf.
- Use Frame controls or point at the model's buttons and hold the trigger.
- Adjust sound and volume, or reload the online player, in the creation's settings.
- Includes gameplay/settings/back screenshots, linked credits, model attribution and license notices.

The supplied model is Mattyew's Sonic 2 casing; the hosted game is Sonic 1. Both ATTACK halves activate Spin; ACTION jumps. Online game state restarts when turned Off or reloaded; sound/volume settings persist. Archive.org loading times and availability can vary.

Model: [Tiger Handheld Games Console](https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c) by [Mattyew](https://sketchfab.com/mattyu), [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Converted/scaled mesh and textures; baked screen replaced with a live display and interactive controls.

Player: [Internet Archive](https://archive.org/details/hh_tsonic), using [Emularity](https://github.com/db48x/emularity) and [MAME](https://www.mamedev.org/). Integration by josethevrtech; see ASSET_CREDITS.md and THIRD_PARTY_NOTICES.md for all contributors and licenses. Sega/Tiger game material retains its original rights. No ROM files are distributed.

Target: Steam Frame ARM64, MagicShelf 0.3.21 or newer. Uses the SteamOS Qt WebEngine runtime. Browser work runs separately from the headset tracking loop. This is a fan-made integration, not an official Sega, Tiger Electronics or Internet Archive product.
