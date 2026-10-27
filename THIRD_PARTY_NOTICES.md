# Third-party notices

Sharp SM5xx CPU portions: Copyright hap. Modified by bzhxx. BSD 3-Clause License:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

LCD-Game-Emulator system/save portions: bzhxx, GPL-3.0-or-later. Full GPL in LICENSE. Exact source repository/revision in emulator.lock.json. Source retrieval: python3 tools/prepare_emulator.py. Sonic-specific wiring lives in src/chip_host.c; upstream system I/O symbols are renamed at compile time. Graphics/ROM-loader stubs bypass embedded-only formats; runtime renders supplied SVG segments. Upstream CPU source files are unmodified.

MagicShelf SDK and reused controller/placement shell: MIT, full notice in SDK-LICENSE.txt. SDK revision in sdk.lock.json. OpenVR notice in OPENVR-LICENSE.txt. Mattyew model notice in ASSET_CREDITS.md. Game ROM, melody and SVG artwork are not included in review packages and are not relicensed.
