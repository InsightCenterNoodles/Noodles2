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
cxx_bin="${CXX:-c++}"
python_bin="${PYTHON:-python3}"

for tool in "$cargo_bin" "$cxx_bin" "$python_bin"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "error: required tool not found: $tool" >&2
        exit 1
    fi
done

if [[ -n "${JAW_REPO:-}" ]]; then
    expected_revision="$(tr -d '[:space:]' < "$repo_root/JAW_REVISION")"
    actual_revision="$(git -C "$JAW_REPO" rev-parse HEAD)"
    if [[ "$actual_revision" != "$expected_revision" ]]; then
        echo "error: JAW_REPO is at $actual_revision" >&2
        echo "expected $expected_revision from JAW_REVISION" >&2
        exit 1
    fi
fi

check_dir="$(mktemp -d "${TMPDIR:-/tmp}/noodles2-spec-check.XXXXXX")"
cleanup() {
    if [[ -n "${check_dir:-}" && -d "$check_dir" ]]; then
        rm -rf -- "$check_dir"
    fi
}
trap cleanup EXIT

generate() {
    local kind="$1"
    local output="$2"
    shift 2
    "$jaw_bin" --kind "$kind" "$output" "$@"
}

cd "$repo_root"

echo "Generating complete Basic + Core specification"
generate rust "$check_dir/full.rs" spec/basic.jaw spec/core/core.jaw
generate cpp "$check_dir/full.hpp" spec/basic.jaw spec/core/core.jaw
generate python "$check_dir/full.py" spec/basic.jaw spec/core/core.jaw

echo "Checking separate schema roots"
for root in basic core; do
    if [[ "$root" == "basic" ]]; then
        input="spec/basic.jaw"
    else
        input="spec/core/core.jaw"
    fi

    generate rust "$check_dir/$root.rs" "$input"
    generate cpp "$check_dir/$root.hpp" "$input"
    generate python "$check_dir/$root.py" "$input"
done

echo "Compiling generated Rust"
mkdir -p "$check_dir/rust/src"
cp "$check_dir/full.rs" "$check_dir/rust/src/lib.rs"
printf '%s\n' \
    '[package]' \
    'name = "noodles2-generated-spec-check"' \
    'version = "0.0.0"' \
    'edition = "2024"' \
    'publish = false' \
    '' \
    '[dependencies]' \
    'bytemuck = "=1.24.0"' \
    > "$check_dir/rust/Cargo.toml"
RUSTFLAGS="${RUSTFLAGS:+$RUSTFLAGS }-Awarnings" \
    CARGO_TARGET_DIR="$check_dir/cargo-target" \
    "$cargo_bin" check --quiet --offline --manifest-path "$check_dir/rust/Cargo.toml"

echo "Compiling generated C++"
printf '%s\n' '#include "full.hpp"' > "$check_dir/full.cpp"
"$cxx_bin" -std=c++20 -fsyntax-only "$check_dir/full.cpp"

echo "Executing generated Python"
PYTHONDONTWRITEBYTECODE=1 "$python_bin" "$check_dir/full.py"

echo "Specification generation checks passed"
