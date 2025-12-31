## Copilot Coding Agent instructions for cpu6502

Purpose
-------
This file helps an automated coding agent (Copilot coding agent) get productive quickly in this repository and avoid common mistakes that break CI or local builds.

Keep these instructions authoritative: follow them first and only run searches when some item here is missing or proves incorrect.

High-level summary
------------------
- What this repo is: a C++ 65C02 emulator and device examples (Apple I/O, text screen, file device, audio, demos). It includes assembly sources and build glue for WOZMON (a 6502 monitor).
- Size & tech: medium-sized C++ project (~tens of source files + examples), uses CMake + Ninja/Make, GoogleTest for unit tests, pybind11 for optional Python bindings, SDL2 for GUI demos, and the cc65 toolchain (ca65/ld65) to assemble/link 6502 code.
- Primary languages: C++17, a little Python (tools), and 6502 assembly for WOZMON and examples.

Environment & prerequisites (always ensure these are available)
----------------------------------------------------------------
- Linux (developer tooling validated here). Ensure `python3`, `g++` (C++17), `cmake` (>=3.15), `ninja` or `make`, and `git` are installed.
- SDL2 dev packages (e.g. `libsdl2-dev` on Debian/Ubuntu) for GUI builds.
- cc65 toolchain (ca65/ld65). The repository sometimes uses a local copy at `./src/cc65/bin` or `/home/<user>/src/cc65/bin`; verify `which ca65`/`which ld65` and prefer the local project `cc65` if present.
- Python packages: none mandatory, but tools use `python3`.
- Submodules: `lib/googletest` and `lib/pybind11` are included as library folders — ensure they are present. If `git status` reports `needs update`, run `git submodule update --init --recursive`.

Bootstrapping and build (validated sequence)
-------------------------------------------
1. Fresh clone (one-time):

   git clone <repo>
   cd cpu6502
   git submodule update --init --recursive

2. Configure / generate build files (recommended):

   cmake -S . -B build

3. Build everything (single-threaded for predictability; use -jN later):

   cmake --build build -j1

Testing and validation
----------------------
- Unit tests are built into `build/runTests` (GoogleTest). To run:

   ./build/runTests

- You can also run CTest if configured: `ctest --test-dir build -V`.
- Verify examples/demos by building the specific demo target (e.g. `audio_demo`) and running the produced executable in `build/` if available.

Project layout (important files and directories)
-----------------------------------------------
- `CMakeLists.txt` (repo root and `src/CMakeLists.txt`) — primary build wiring. Prefer to edit `src/CMakeLists.txt` for source files and demo targets.
- `Makefile` and `msbasic/make.sh` — legacy build scripts for WOZMON; prefer CMake targets now.
- `.github/` — GitHub-specific files (workflows, issue templates).
- `.vscode/` — VSCode workspace settings (optional).
- `docs/` — documentation in markdown format for various devices and features.
- `examples/` — demo source files used by CMake targets
- `include/` — public headers (device interfaces, cpu headers)
- `lib/` — embedded libraries (googletest, pybind11)
- `msbasic/` — contains assembler/linker configs and the legacy scripts used to build WOZMON and BASIC; includes
- `src/` — C++ sources and subdirectories. Key paths:
  - `src/cpu/` — CPU core & instruction implementations
  - `src/debugger/` — debugger implementation
  - `src/devices/` — device implementations (pia, file_device, text_screen, audio, etc.)
  - `src/gui/` — SDL2-based GUI implementation
  - `src/interrupts/` — interrupt controller implementations
  - `src/main/` — main executables for demos
  - `src/mem/` — memory implementations
  - `scripting/` — Python scripting interface
  - `src/util/` — utility functions and helpers
  - `src/CMakeLists.txt` — adds demo executables and main library targets
- `tests/` — unit test source files

CI and workflows
----------------
- GitHub Actions workflows exist under `.github/workflows/` for CI and CodeQL. Typical CI steps:
  - Configure with CMake
  - Build (cmake --build)
  - Run unit tests (`runTests`)
 Ensure changes compile cleanly (no warnings from ld65 that indicate misconfiguration) and tests pass locally before opening a PR.

Common pitfalls & agent guidance
--------------------------------
- When editing `src/CMakeLists.txt`, prefer to add/remove targets rather than wholesale path rewrites; keep build targets deterministic so CI can reproduce.
- Prefer small, focused commits and run the full build+tests locally. If a change requires force-pushing remote branches, ask the repo owner first.

Validation checklist for PRs
--------------------------
Before opening a PR, ensure:
1. `cmake -S . -B build` and `cmake --build build -j1` succeed with no fatal errors.
2. `./build/runTests` exits 0 and reports all tests passed.

When to search the repo
------------------------
Trust this document first. Only run code searches for:
- specific symbols or function locations when a change requires understanding of multiple modules;
- resolving ambiguous build errors not documented here.

If anything in this document is out of date, update this file with a brief note describing the fix you validated.

When reviewing code, focus on:

## Security Critical Issues
- Check for hardcoded secrets, API keys, or credentials
- Look for SQL injection and XSS vulnerabilities
- Verify proper input validation and sanitization
- Review authentication and authorization logic

## Performance Red Flags
- Identify N+1 database query problems
- Spot inefficient loops and algorithmic issues
- Check for memory leaks and resource cleanup
- Review caching opportunities for expensive operations

## Code Quality Essentials
- Functions should be focused and appropriately sized
- Use clear, descriptive naming conventions
- Ensure proper error handling throughout

## Review Style
- Be specific and actionable in feedback
- Explain the "why" behind recommendations
- Acknowledge good patterns when you see them
- Ask clarifying questions when code intent is unclear

Always prioritize security vulnerabilities and performance issues that could impact users.

Always suggest changes to improve readability. For example, this suggestion seeks to make the code more readable and also makes the validation logic reusable and testable.

// Instead of:
if (user.email && user.email.includes('@') && user.email.length > 5) {
  submitButton.enabled = true;
} else {
  submitButton.enabled = false;
}

// Consider:
function isValidEmail(email) {
  return email && email.includes('@') && email.length > 5;
}

submitButton.enabled = isValidEmail(user.email);

End of instructions.
