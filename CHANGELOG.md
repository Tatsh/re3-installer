<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

### Fixed

- Added possibly missing header in tests.

## [0.2.2] - 2025-08-01

### Added

- Functional tests for `utils` module.
- Comprehensive test coverage for all modules.
- Added attributes to function declarations.
- Added `--output-on-failure` flag to test workflow.

### Changed

- Renamed `helpers` module to `utils`.
- Simplified test `CMakeLists` configuration.
- Switched to `coverallsapps/github-action` for coverage reporting.
- Removed `geninfo` step from test workflow.
- Enabled debug mode for coverage builds.
- Pass mode parameter to `open()` function.
- Updated `README` with corrected information.

### Fixed

- Fixed truncation issue in `installer_tests`.
- Fixed `get_installation_dir()` to return `~/re3` if path retrieval fails on macOS.
- Removed extra headers from `env`, `installer`, and `mkdir_p` modules.
- Removed unnecessary mocks from extractor tests.

## [0.2.1] - 2025-07-24

### Added

- Windows support with 32-bit and 64-bit builds.
- CPack packaging configuration.
- Version resource file for Windows.
- Runtime dependencies bundling for non-Windows platforms.
- Upload of 32-bit Windows package in workflow.

### Changed

- Updated `README.md` with Windows information.
- Improved Windows path handling to avoid `MAX_PATH` issues.
- Use dynamic buffers in `copy_tree` for Windows.
- Bumped project dependencies.

### Fixed

- Fixed MinGW dependency resolution.
- Fixed Windows-specific issues in `helpers` module.

## [0.2.0] - 2025-07-19

### Added

- Man page for `re3-installer`.
- Comprehensive test suite with CMocka.
- Tests for `installer`, `extractor`, `env`, `mkdir_p`, and `helpers` modules.
- Logging system with configurable verbosity.
- Support for GTA Vice City (reVC).
- GitHub Copilot instructions for C development.

### Changed

- Switched license from ISC to GPL-3.0.
- Complete rewrite using native functions and `libcdio` instead of external tools.
- Replaced `NULL` with `nullptr` throughout codebase.
- Renamed `is_iso` to `ends_with_iso` for clarity.
- Replaced some `strncmp` calls with more appropriate functions.
- Made extractor more lenient with missing file groups.
- Header cleanup across all modules.

### Fixed

- Fixed installation of Audio files from second disc.
- Fixed potential memory leak in extractor.
- Added missing header for macOS in `env` module.
- Removed unnecessary checks in extractor.

## [0.1.0] - 2021-04-04

### Added

- Success message on completion.
- CMake build system.
- Simplified build instructions.

### Changed

- Switched from Makefile to CMake.
- Updated CI workflow for CMake.

### Removed

- Xcode project (no longer directly supported).
- Removed `.vscode` directory.

## [0.0.1] - 2020-10-22

### Added

- VSCode settings configuration.
- Initial project structure.
- Basic installer functionality for GTA III files.
- Support for ISO files and directories.
- XDG Base Directory support on Linux.
- macOS support.
- GitHub Actions CI workflow.
- Jekyll theme for GitHub Pages.
- `README` with usage instructions.
- Makefile with clean target and XDG option.
- Code organization into separate modules:
  - `env.c`/`env.h` - Environment and path handling.
  - `helpers.c`/`helpers.h` - Helper functions.
  - `mkdir_p.c`/`mkdir_p.h` - Recursive directory creation.
  - `support.h` - Common definitions.
- Xcode project support.
- Close inactive issues workflow.

### Changed

- Removed unnecessary header strings in header files.

### Fixed

- Fixed some print statements.

[unreleased]: https://github.com/Tatsh/re3-installer/compare/v0.2.2...HEAD
[0.2.2]: https://github.com/Tatsh/re3-installer/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/Tatsh/re3-installer/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/Tatsh/re3-installer/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/Tatsh/re3-installer/compare/v0.0.1...v0.1.0
[0.0.1]: https://github.com/Tatsh/re3-installer/releases/tags/v0.0.1
