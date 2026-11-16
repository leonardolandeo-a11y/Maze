#!/usr/bin/env bash
set -euo pipefail
repo=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
. /etc/os-release
[[ "$ID" == ubuntu && $(uname -m) == x86_64 ]] || {
    echo 'Build on Ubuntu x86_64.' >&2; exit 1;
}
build="$repo/build-linux-release"
cmake -S "$repo" -B "$build" -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF -DFTXUI_BUILD_TESTS=OFF -DFTXUI_BUILD_EXAMPLES=OFF "$@"
cmake --build "$build" --config Release --target escape_circuit map_generator --parallel "${JOBS:-2}"

name="CircuitoEscape-Ubuntu-${VERSION_ID}-x86_64-Release"
mkdir -p "$repo/dist"
stage=$(mktemp -d "$repo/dist/.linux-package.XXXXXX")
trap 'rm -rf -- "$stage"' EXIT
package="$stage/$name"
mkdir -p "$package/"{bin,lib,assets/maps,licenses}
for executable in escape_circuit map_generator; do
    install -m 755 "$build/$executable" "$package/bin/$executable"
    install -m 755 "$repo/installer/linux/launch.sh" "$package/$executable"
    dependencies=$(ldd "$build/$executable")
    if [[ "$dependencies" == *'not found'* ]]; then
        echo "$dependencies" >&2; exit 1
    fi
    # Include the complete ELF dependency closure, including glibc and its
    # matching loader. Launchers explicitly use this loader and library path.
    while IFS= read -r library; do
        cp -L -- "$library" "$package/lib/"
    done < <(awk '/=> \// {print $3} /^[[:space:]]*\// {print $1}' <<< "$dependencies")
done
for level in facil medio dificil; do
    install -m 644 "$repo/assets/maps/mapa_$level.txt" "$package/assets/maps/"
done
install -m 755 "$repo/installer/linux/install.sh" "$package/install.sh"
touch "$package/.circuito-escape-linux-package"
ftxui_source=${FTXUI_SOURCE_DIR:-$build/_deps/ftxui-src}
install -m 644 "$ftxui_source/LICENSE" "$package/licenses/FTXUI.txt"
for dependency in libc6 libgcc-s1 libstdc++6; do
    install -m 644 "/usr/share/doc/$dependency/copyright" "$package/licenses/$dependency.txt"
done
{
    printf 'OS=%s\nArchitecture=x86_64\nBuildType=Release\n' "$PRETTY_NAME"
    printf 'SourceCommit=%s\n' "$(git -C "$repo" rev-parse HEAD)"
    c++ --version | head -n 1
    dpkg-query -W libc6 libgcc-s1 libstdc++6
} > "$package/BUILD-INFO.txt"
(cd "$package" && find bin lib assets licenses -type f -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS)
tar -czf "$repo/dist/$name.tar.gz" -C "$stage" "$name"
echo "$repo/dist/$name.tar.gz"
