#!/bin/sh
set -eu
package=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ref="$package/reference/stable-v0.5.html"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
node "$package/tests/pattern-reference.cjs" "$ref" firmware/src
cc -O2 -std=c11 -fsanitize=undefined -fno-sanitize-recover -Ifirmware/src "$package/tests/test-integration.c" -o "$tmp/integration"
"$tmp/integration"
cc -O2 -Ifirmware/hal -Ifirmware/src -Ifirmware/gen "$package/tests/test-midi.c" -o "$tmp/midi"
"$tmp/midi"
cc -O2 -Ifirmware/src "$package/tests/stage-reference.c" -o "$tmp/stage"
node "$package/tests/stage-reference.cjs" "$ref" "$tmp/stage"
cc -O2 -Ifirmware/src "$package/tests/lead-pitch-trajectory.c" -lm -o "$tmp/pitch"
"$tmp/pitch"
"$tmp/pitch" grid
cc -O2 -Ifirmware/src "$package/tests/lead-reference.c" -o "$tmp/lead"
for city in 0 1 2; do node "$package/tests/lead-reference.cjs" "$ref" "$tmp/lead" "$city"; done
for mode in alien offset sync band linear; do node "$package/tests/lead-reference.cjs" "$ref" "$tmp/lead" 0 "$mode"; done
if [ "$#" -gt 0 ]; then node "$package/tests/test-lcd.mjs" "$1" "$package/build/MDMA-0.5.5-alpha-stage.fwsc"; fi
