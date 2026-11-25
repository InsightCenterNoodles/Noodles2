#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/.." && pwd)"

if [[ -n "${JAW_BIN:-}" ]]; then
    jaw_bin="$JAW_BIN"
elif command -v jaw >/dev/null 2>&1; then
    jaw_bin="$(command -v jaw)"
elif [[ -x "$repo_root/../jaw/target/debug/jaw" ]]; then
    jaw_bin="$repo_root/../jaw/target/debug/jaw"
elif [[ -x "$repo_root/../../jaw/target/release/jaw" ]]; then
    jaw_bin="$repo_root/../../jaw/target/release/jaw"
else
    echo "error: JAW compiler not found" >&2
    echo "set JAW_BIN to the compiler built from the commit in JAW_REVISION" >&2
    exit 1
fi

if [[ ! -x "$jaw_bin" ]]; then
    echo "error: JAW_BIN is not executable: $jaw_bin" >&2
    exit 1
fi

cargo_bin="${CARGO:-cargo}"
if ! command -v "$cargo_bin" >/dev/null 2>&1; then
    echo "error: Cargo not found: $cargo_bin" >&2
    exit 1
fi

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/noodles2-tools-build.XXXXXX")"
cleanup() {
    if [[ -n "${build_dir:-}" && -d "$build_dir" ]]; then
        rm -rf -- "$build_dir"
    fi
}
trap cleanup EXIT

mkdir -p "$repo_root/tools/bin"

echo "Building NOODLES2 conformance tools"
JAW_BIN="$jaw_bin" CARGO_TARGET_DIR="$build_dir/target" \
    "$cargo_bin" build --release --locked \
    --manifest-path "$repo_root/tools/rust/Cargo.toml"

install -m 755 \
    "$build_dir/target/release/noodles2-handshake-validator" \
    "$repo_root/tools/bin/noodles2-handshake-validator"

echo "Built tools/bin/noodles2-handshake-validator"
