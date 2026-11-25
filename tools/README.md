# NOODLES2 conformance tools

Build the Rust tools from the repository root:

```console
./scripts/build-tools.sh
```

The script finds `jaw` on `PATH`, falls back to `../jaw/target/debug/jaw`, or
uses the executable named by `JAW_BIN`. Generated Rust bindings and Cargo build
artifacts live in a temporary directory and are removed after the finished
executables are copied to `tools/bin/`.

## Handshake validator

`noodles2-handshake-validator` accepts one WebSocket client, validates its
NOODLES2 `Introduction`, sends an `IntroductionReply`, reports the result, and
exits. A successful run verifies that the accepting parent can advertise the
fixed session extension set and its associated OOB link.

```console
tools/bin/noodles2-handshake-validator --listen 127.0.0.1:50000
```

The first status line is suitable for test harnesses:

```text
LISTENING ws://127.0.0.1:50000
```

Success exits with status 0. Invalid introductions, transport failures, and
timeouts exit nonzero and print a `FAIL:` diagnostic to standard error. Run
with `--help` to see resource-limit and timeout options.
