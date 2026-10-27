# Fidelity and validation

Uses the original 4096-byte SM511 program and 256-byte melody with 132 supplied SVG segments. Sonic input matrix, fixed ground Start row, active-low BA sound/B power, and R1+2*S1 DAC follow MAME hh_sm510.cpp tsonic. CPU core is pinned, not a scripted game simulation.

LCD deflicker uses upstream core state. Audio is 32768 Hz mono from emulated outputs. LCD backing is neutral: Sonic 1 inlay was not supplied. Model is Sonic 2. No claim of cycle-level parity with current MAME or physical sound calibration.

Emulation, audio and durable save writes run on a worker. Tracking consumes an immutable snapshot and atomic input levels. Saves retain CPU state rather than inventing an RTC. Settings are creation-owned; subscriptions, On/Off, updates and removal remain host-owned.

Pending: differential comparison with current MAME and physical Frame tracking/grip/depth/controls/dock recovery/audio checks.

Verified on 2026-10-26: ARM64 builds in WSL and on Steam Frame. Input matrix, all four DAC levels, original ROM progression (15 distinct LCD states in nine seconds with controls), settings persistence, exact deterministic CPU save replay, and missing-ROM handling passed. Frame preview used Zink/Turnip Adreno 750. A bounded VR smoke run initialized controller actions, loaded content, wrote telemetry and exited on SIGTERM. No physical controller presses were observed; docked visibility and interactive rendering were not validated. One recorded loop exceeded 33 ms; this smoke run does not establish active tracking performance.

Version 0.1.1 private headset installation: the initial 256 MiB limit killed SteamVR texture interop during visible rendering. Raised the declared budget to 768 MiB; measured active use about 376 MiB, with no restarts after correction. Installed through the host package transaction with config backups; local ROM content lives in the module private data directory. Verified headset tracking, overlay visibility, held state, controller input, connected unmuted audio, and nonzero generated sound. User confirmed seeing and hearing it. Detailed shake/snap recovery, long-session comfort, and current-MAME differential validation remain pending.
