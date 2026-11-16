#!/usr/bin/env bash
set -euo pipefail

# Per-user installation; no compiler, package manager or sudo needed.
prefix="$HOME/.local"
target="$prefix/opt/circuito-escape"
marker='.circuito-escape-linux-package'
commands=(escape_circuit map_generator)
action=${1:-install}
if [[ $# -gt 1 || ( "$action" != install && "$action" != uninstall ) ]]; then
    echo "Usage: $0 [install|uninstall]" >&2
    exit 2
fi

if [[ "$action" == uninstall ]]; then
    if [[ ! -f "$target/$marker" || -L "$target" ]]; then
        echo "No managed installation at $target" >&2
        exit 1
    fi
    for command in "${commands[@]}"; do
        link="$prefix/bin/$command"
        if [[ -L "$link" && $(readlink -- "$link") == "$target/$command" ]]; then
            rm -- "$link"
        fi
    done
    rm -rf -- "$target"
    echo "Uninstalled. Maps generated outside the installation are preserved."
    exit
fi

source_dir=$(dirname -- "$(readlink -f -- "$0")")
. /etc/os-release
if [[ "$ID" != ubuntu || $(uname -m) != x86_64 ]]; then
    echo 'This package requires Ubuntu x86_64.' >&2
    exit 1
fi
if [[ -e "$target" || -L "$target" ]]; then
    echo "Destination exists: $target. Uninstall the previous package first." >&2
    exit 1
fi
for command in "${commands[@]}"; do
    if [[ -e "$prefix/bin/$command" || -L "$prefix/bin/$command" ]]; then
        echo "Refusing to overwrite $prefix/bin/$command" >&2
        exit 1
    fi
    [[ -x "$source_dir/$command" && -x "$source_dir/bin/$command" ]]
done
[[ -f "$source_dir/$marker" && -x "$source_dir/lib/ld-linux-x86-64.so.2" ]]
mkdir -p -- "$prefix/opt" "$prefix/bin"
mkdir -- "$target"
cleanup() {
    for command in "${commands[@]}"; do
        link="$prefix/bin/$command"
        if [[ -L "$link" && $(readlink -- "$link") == "$target/$command" ]]; then
            rm -- "$link"
        fi
    done
    rm -rf -- "$target"
}
trap cleanup ERR
cp -a -- "$source_dir/." "$target/"
for command in "${commands[@]}"; do
    ln -s -- "$target/$command" "$prefix/bin/$command"
done
trap - ERR
printf 'Installed: %s\nRun: %s/bin/escape_circuit\nUninstall: %s/install.sh uninstall\n' \
    "$target" "$prefix" "$target"
echo 'Add "$HOME/.local/bin" to PATH to use the command names directly.'
