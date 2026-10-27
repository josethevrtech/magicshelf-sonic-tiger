# Sonic Tiger LCD v0.3.0

The handheld now runs Sonic the Hedgehog 2 through [PEXY](https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/), matching its Sonic 2 casing. Internet required; no ROM import.

- Verifies the actual MAME tsonic2 machine before showing gameplay.
- Remaps Start, Pause, Sound, Off, ACL and Action to the new player's inputs. Both ATTACK halves and ACTION share the game's Action input.
- Saves locally before Off/reload and restores compatible browser saves.
- Keeps X to start, grip pickup/placement, independent model/controller holds and 28 percent default volume.
- Restricts browser requests, blocks downloads/popups and includes new real gameplay/settings/back screenshots.

Frame tests passed for machine identity, browser key press/release, save/restore, model target rays, short taps, combined holds, settings, offline errors and clean child exit. Remaining individual model controls and perceived audio level need headset review.

Model: [Tiger Handheld Games Console](https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c) by [Mattyew](https://sketchfab.com/mattyu), [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Modified mesh, scale, textures, screen and button targets. Player: PEXY and [MAME contributors](https://github.com/mamedev/mame). Sega/Tiger game material retains its original rights. No game files are distributed. See ASSET_CREDITS.md and THIRD_PARTY_NOTICES.md.
