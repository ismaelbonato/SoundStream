# SoundStream tests

This folder contains automated tests split by layer:

- **core (Catch2)**: mapping/state/controller logic
- **qt (Qt Test)**: Qt-thread/event-loop specific behavior

## Running tests

Configure/build as usual, then run CTest from the build directory.

## Optional coverage mode

Coverage instrumentation can be enabled at configure time with:

- `-DSOUNDSTREAM_ENABLE_COVERAGE=ON`

When enabled, compile and link flags include `--coverage` (`-O0 -g`), and a convenience `coverage` target runs the full test suite.

Notes:

- The `coverage` target executes tests; report generation (for example via `gcovr`/`lcov`) is intentionally left optional to keep dependencies minimal.
- Works best with GCC/Clang toolchains.
