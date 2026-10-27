# Sonic Tiger LCD v0.3.1

The docked handheld now fits its shelf slot, preventing its wide casing from overlapping the Android phone at the default spacing. The fit uses the converted mesh bounds and leaves a scaled 10 mm slot margin. Pickup restores the user-selected size.

Drawing, foot placement and button raycasts use the same fitted pose and scale. The larger transparent interaction canvas is retained so the model and grab handle stay visible. No shelf preferences or game saves are changed.

Validation: 81 actual-mesh geometry cases cover 12/18/30 cm spacing, dashboard scales 0.6/1/1.8, normal/enlarged sizes and shelf lean. Tests verify foot alignment, a visible gap beside the measured Android phone, button-coordinate round trips and unchanged free-held size. Frame-native build and existing model/player-policy tests pass.

The Sonic 2 online player, controls, saves and 28 percent default volume are unchanged. Internet required; no ROM import.

Model: [Tiger Handheld Games Console](https://sketchfab.com/3d-models/tiger-handheld-games-console-20d9825be3b64e89b275ee062455691c) by [Mattyew](https://sketchfab.com/mattyu), [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Mesh, scale, textures, screen and button targets modified. [PEXY Sonic 2 player](https://pexy.io/game/sonic-the-hedgehog-2-tiger-handheld/) uses [MAME](https://github.com/mamedev/mame). Original game/artwork retains its respective rights; none is bundled.
