# Fidelity and validation

Version 0.3.0 uses PEXY's dedicated Sonic 2 player in a separate sandboxed Qt WebEngine process. Readiness requires actual tsonic2 machine arguments and title, a running machine, no ROM problems and a populated canvas. No Sonic 1 fallback exists. The LCD canvas and original inlay fill the model screen.

The hosted n86.program, n86.melody and tsonic2.svg manifest sizes and CRCs match MAME's driver. No game content is redistributed. Internet is required. Service updates can affect loading and save compatibility.

Frame tests verified machine identity, completed frames, and browser-observed press/release for arrows, Action, Pause, Sound and Start. Silent exit saved successfully; a fresh browser process restored the saved game. Downloads, popups and permissions are denied. Requests are limited to relevant PEXY HTTPS player/core/game paths; optional external beacon requests were blocked. HTTP cache is memory only; a private profile retains saves.

Model tests cover 12 measured targets from front/oblique rays, backside rejection and inert grille/neutral regions. Model and controller holds remain independent. Both ATTACK halves and ACTION map to the single Action input. X remains ON/START. Grip pickup, placement, settings and default 28 percent volume are preserved.

The prior version's 3D ON/START was physically confirmed by the user. Remaining individual buttons, perceived audio level and long-session performance require headset review. Automated key delivery does not establish physical usability of every printed button.

The 2048 MiB process-tree budget covers the browser, MAME and VR renderer. Browser work remains separate from tracking. Retained native SM511 code is a development reference, not the shipped backend.
