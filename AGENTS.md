# Repository Guidelines

## Project Structure & Module Organization
This repository is a firmware workspace with multiple device trees. Use `FF_1070/` as the primary application project, `ff-582-70/` as a legacy variant, `AK37E_SDK_V1.05/` for the external SDK/toolchain, and `PCB/` for hardware files. Inside `FF_1070/`, keep feature code in `layout/` and `common/`, public headers in `include/`, bundled third-party code in `share/`, UI assets and fonts in `res/`, generated outputs in `build/`, and packaging scripts/artifacts in `upgrade/`.

## Build, Test, and Development Commands
- `cd FF_1070 && make`: configures CMake, builds `build/FF.BIN`, copies assets into `upgrade/app/`, and runs `upgrade/make_image.sh`.
- `cd FF_1070 && make clean`: clears generated files in `build/`.
- `cd FF_1070/build && cmake . && make -j16`: lower-level rebuild when you need to inspect CMake output directly.
- `cd ff-582-70 && make`: builds the older firmware branch when working there.

The `FF_1070` build expects the SDK/toolchain under `AK37E_SDK_V1.05/`; check hard-coded paths in packaging scripts before changing directory layout.

## Coding Style & Naming Conventions
Most editable code is C. Follow the local style already in the touched file: 4-space indentation is common, function braces are typically on the next line, and filenames use lowercase snake_case such as `intercom_interface.c`. Prefer `static` helpers for file-local logic, keep headers paired with source names, and avoid broad cleanup in vendored code under `share/`.

## Testing Guidelines
There is no dedicated app-level unit test suite in this workspace. For every change, rebuild the affected variant and confirm the expected artifacts appear under `FF_1070/build/` and `FF_1070/upgrade/app/`. For UI, audio, or upgrade-flow changes, add a short manual verification note with the target board, feature exercised, and result.

## Commit & Pull Request Guidelines
Recent history uses short, imperative subjects and often a prefix such as `fix:`. Prefer messages like `fix: stabilize intercom UART receive handling`. Keep each commit focused on one device tree or subsystem.

Merge requests should state the affected variant (`FF_1070`, `ff-582-70`, SDK, or PCB), summarize behavioral impact, list build/validation steps, and attach screenshots or photos for UI or hardware-facing changes.
