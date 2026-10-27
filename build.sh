#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
sdk="${MAGICSHELF_SDK_DIR:-$PWD/vendor/magicshelf}"
case "$(uname -m)" in aarch64) platform=linuxarm64;; x86_64) platform=linux64;; *) exit 1;; esac
vr="${OPENVR_LIB_DIR:-/opt/steamvr/bin/$platform}"
mkdir -p build/package/{bin,lib,assets} build/obj build/tests
python3 tools/prepare_emulator.py
core="$PWD/vendor/lcd-emulator/src"
cflags=(-O2 -std=gnu99 -fgnu89-inline -I"$core/gw_sys" -I"$core/cpus")
for source in "$core"/cpus/*.c; do
 gcc "${cflags[@]}" -c "$source" -o "build/obj/$(basename "${source%.c}").o"
done
gcc "${cflags[@]}" -Dgw_readK=legacy_readK -Dgw_readBA=legacy_readBA -Dgw_readB=legacy_readB -Dgw_writeR=legacy_writeR -c "$core/gw_sys/gw_system.c" -o build/obj/gw_system.o
gcc "${cflags[@]}" -c src/chip_host.c -o build/obj/chip_host.o
flags=(-O2 -std=c++17 -fPIC -pthread -DQT_NO_KEYWORDS -Isrc -I"$sdk/native/compat" -I"$sdk/native" -I"$core/gw_sys" -I"$core/cpus" '-DMAGICSHELF_CREATION_PROFILE="creation_profile.h"')
c++ "${flags[@]}" tests/model_buttons_test.cpp -o build/tests/model_buttons_test $(pkg-config --cflags --libs egl glesv2 cairo)
build/tests/model_buttons_test
c++ "${flags[@]}" tests/shelf_fit_test.cpp -o build/tests/shelf_fit_test $(pkg-config --cflags --libs egl glesv2 cairo json-c)
build/tests/shelf_fit_test
c++ -O2 -std=c++17 -fPIC -Isrc tests/player_policy_test.cpp -o build/tests/player_policy_test $(pkg-config --cflags --libs Qt6Core)
build/tests/player_policy_test
if [[ -f tests/emulator_test.cpp ]]; then
 c++ "${flags[@]}" tests/emulator_test.cpp build/obj/*.o -o build/tests/emulator_test $(pkg-config --cflags --libs Qt6Widgets egl glesv2 cairo libpulse-simple)
 SONIC_NO_AUDIO=1 SONIC_CONTENT_DIR="$PWD/local/content" build/tests/emulator_test
fi
if [[ "${1:-}" == "--tests-only" ]]; then exit 0; fi
c++ -O2 -std=c++17 -fPIC -pthread -Isrc src/pexy_player.cpp -o build/package/bin/archive-player $(pkg-config --cflags --libs Qt6WebEngineWidgets)
c++ "${flags[@]}" -DSONIC_ARCHIVE_PLAYER=1 src/main.cpp -o build/package/bin/creation -L"$vr" -lopenvr_api '-Wl,-rpath,$ORIGIN/../lib' $(pkg-config --cflags --libs Qt6Widgets egl glesv2 cairo json-c libpulse-simple)
cp "$vr/libopenvr_api.so" build/package/lib/
cp assets/device.mesh assets/body-0.rgba assets/model-info.json build/package/assets/
cp -r assets/input build/package/assets/
mkdir -p build/package/media
cp media/sonic-tiger-*.png build/package/media/
cp assets/model-license.txt build/package/MODEL-LICENSE.txt
cp LICENSE SDK-LICENSE.txt OPENVR-LICENSE.txt ASSET_CREDITS.md THIRD_PARTY_NOTICES.md README.md FIDELITY.md build/package/
printf '#!/bin/sh\nexec "$(dirname "$0")/creation" "$@"\n' > build/package/bin/control
chmod +x build/package/bin/control
python3 - <<'PY'
import json,platform
from pathlib import Path
m=json.loads(Path('module.json').read_text());m['architecture']=platform.machine()
Path('build/package/module.json').write_text(json.dumps(m,indent=2)+'\n')
PY
if [[ "${1:-}" == "--online-tests" ]]; then
 c++ "${flags[@]}" tests/online_test.cpp -o build/tests/online_test $(pkg-config --cflags --libs Qt6Widgets egl glesv2 cairo)
 build/tests/online_test "$PWD/build/package" "$PWD/local"
fi
echo 'Built online creation. ROM content is never staged.'
