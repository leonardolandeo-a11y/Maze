#!/usr/bin/env bash
set -euo pipefail
root=$(dirname -- "$(readlink -f -- "$0")")
name=$(basename -- "$0")
case "$name" in
    escape_circuit) args=(--maps "$root/assets/maps") ;;
    map_generator) args=() ;;
    *) echo "Unknown launcher: $name" >&2; exit 1 ;;
esac
exec "$root/lib/ld-linux-x86-64.so.2" --library-path "$root/lib" \
    "$root/bin/$name" "${args[@]}" "$@"
