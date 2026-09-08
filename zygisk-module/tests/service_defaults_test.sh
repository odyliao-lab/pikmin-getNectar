#!/usr/bin/env bash
set -eu
# Host shell fixture only: do not run Android service or change any device.
source_file="${1:?service.sh path required}"
fixture_dir="$(mktemp -d -t pikmin-defaults-XXXXXXXX)"
CONTROL_MODE="$fixture_dir/nectar.txt"
RETURN_CONTROL_MODE="$fixture_dir/return.txt"
defaults="$(sed -n '/^\[ -f "\$CONTROL_MODE" \]/p; /^\[ -f "\$RETURN_CONTROL_MODE" \]/p' "$source_file")"
test "$(printf '%s\n' "$defaults" | wc -l)" -eq 2
eval "$defaults"
test "$(<"$CONTROL_MODE")" = off
test "$(<"$RETURN_CONTROL_MODE")" = off
printf 'auto\n' > "$CONTROL_MODE"
printf 'all\n' > "$RETURN_CONTROL_MODE"
eval "$defaults"
test "$(<"$CONTROL_MODE")" = auto
test "$(<"$RETURN_CONTROL_MODE")" = all
printf 'PASS: 4 service default/preservation checks in isolated host fixture\n'
