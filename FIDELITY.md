# Fidelity and validation

The default 0.2.0 backend hosts https://archive.org/embed/hh_tsonic in a separate Qt WebEngine process. Archive Emularity/MAME fetches and runs the game. Keyboard input and completed screen frames, including the original inlay, connect to the handheld. No game ROM or native emulator is linked into the online executable. Internet is required at launch; service changes/outages affect availability. Online game state restarts across Off/reload. Creation settings persist.

Model click targets were measured from the supplied Mattyew GLB. Tests cover all 12 targets from frontal/oblique rays, backside rejection and inert speaker/neutral regions. Held model input has a separate channel, so controller updates cannot erase it. Trigger release and tracking loss clear it. Sonic 2 extra Attack/Action controls map to Sonic 1 Spin/Jump without invented game actions.

Frame online tests passed without local ROMs: actual MAME machine readiness, original gameplay, independent model/controller hold-release, sound settings, child shutdown and a deliberately blocked-network retry screen. Captures in local/ document the live game. The user confirmed 3D ON/START after the 0.2.2 delivery fix; the remaining model targets have automated coverage.

Retained native reference backend (0.1.x): original SM511 program/melody, 132 SVG segments, MAME tsonic wiring and two-bit DAC. Input/DAC, ROM progression and deterministic save replay tests passed. Native SteamVR interop needed a 768 MiB budget (about 376 MiB active use). User confirmed seeing/hearing it; a 30 percent app-stream volume reduction was applied.

Online mode has a 2048 MiB process-tree budget for browser/MAME and native VR rendering. Browser work and screen capture are separate from tracking. Long-session performance, shake/snap recovery, perceived online audio level and all physical model controls require headset review. No cycle-level parity claim with current MAME is made.

0.2.1 fixes Archive default mute through its normal unmute preference. A live audio assertion passed; subsequent test runs are explicitly silent to avoid confusing headset audio. 0.2.2 removes the conflicting render-model trigger release check, preserves very short pointer taps through both input workers, and reports browser key-event acknowledgements. A zero-duration model tap delivered both press and release in the real player. The user confirmed the 3D ON/START button works after this fix. Other individual model controls have not all been physically confirmed.
