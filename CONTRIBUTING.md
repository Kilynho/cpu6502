# Contribution Guide

Thank you for your interest in contributing to the CPU 6502 Emulator project! This document provides guidelines and best practices for contributing to the project.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How can I contribute?](#how-can-i-contribute)
- [Pull Request Process](#pull-request-process)
- [Style Guide](#style-guide)
- [Project Structure](#project-structure)
- [Testing](#testing)

## 📜 Code of Conduct

This project adheres to a code of conduct that we expect all participants to follow. By participating, you are committing to maintaining a respectful and welcoming environment for all.

### Our Standards

**Examples of behavior that contribute to a positive environment:**
- Using welcoming and inclusive language
- Being respectful of different viewpoints and experiences
- Accepting constructive criticism gracefully
- Focusing on what is best for the community
- Showing empathy towards other community members

**Examples of unacceptable behavior:**
- Use of sexualized language or images
- Insulting or derogatory comments (trolling)
- Public or private harassment
- Posting other people's private information without explicit permission
- Any other conduct that could reasonably be considered inappropriate

## 🤝 How can I contribute?

### Reporting Bugs

Bugs are tracked as [GitHub Issues](https://github.com/Kilynho/cpu6502/issues). Before creating a new issue:

1. **Check** if the bug has already been reported
2. **Use the template** for bug reports
3. **Provide specific** and reproducible details

#### Information needed to report a bug:
- Clear description of the problem
- Steps to reproduce the behavior
- Expected behavior vs. actual behavior
- Screenshots if applicable
- Compiler version and operating system
- Relevant logs

### Suggesting Improvements

Improvements are also tracked as GitHub Issues. To suggest an improvement:

1. **Use the template** for feature requests
2. **Clearly explain** the problem it solves
3. **Describe the solution** you would like to see
4. **Consider alternatives** you have evaluated

### Your First Code Contribution

Not sure where to start? You can look for issues labeled as:

- `good first issue` - Issues that should require only a few lines of code
- `help wanted` - Issues that may be more complex but need help

### Implementing New Instructions

If you want to implement new 6502 instructions:

1. Refer to the [official 6502 documentation](http://www.6502.org/tutorials/6502opcodes.html)
2. Follow the pattern established in `cpu.cpp` for existing instructions
3. Define the instruction in `cpu.hpp`
4. Implement the logic in `cpu.cpp`
5. Add tests in `test.cpp`
6. Update the documentation in `README.md`

## 🔄 Pull Request Process

1. **Fork** the repository and create your branch from `main`
   ```bash
   git checkout -b feature/your-descriptive-name
   ```

2. **Make your changes** following the style guide

3. **Add tests** for your new code

4. **Ensure** all tests pass
   ```bash
   make test
   ```

5. **Compile** the project with no errors or warnings
   ```bash
   make clean
   make
   ```

6. **Document** your changes in the code and update README if necessary

7. **Commit** your changes with descriptive messages
   ```bash
   git commit -m "feat: brief description of the change"
   ```

8. **Push** to your fork
   ```bash
   git push origin feature/your-descriptive-name
   ```

9. **Open a Pull Request** using the provided template

### Commit Message Format

We use the [Conventional Commits](https://www.conventionalcommits.org/) format:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Formatting changes (no code affect)
- `refactor`: Code refactoring
- `test`: Add or modify tests
- `chore`: Changes to build process or tools

**Examples:**
```
feat(cpu): add support for ADC instruction
fix(mem): fix overflow on memory read
docs(readme): update installation instructions
test(cpu): add tests for branch instructions
```

## 🎨 Style Guide

### C++ Code Style

- **Indentation**: 4 spaces (no tabs)
- **Braces**: K&R style (opening brace on the same line)
- **Variable names**: camelCase for local variables, PascalCase for types
- **Function names**: PascalCase
- **Constants**: UPPER_SNAKE_CASE
- **Line length**: Maximum 100 characters

### Example:

```cpp
// Correct
void CPU::ExecuteInstruction(Mem& mem) {
    const Byte opcode = FetchByte(mem);
    
    if (opcode == INS_LDA_IM.opcode) {
        const Byte value = FetchByte(mem);
        A = value;
        SetZeroAndNegativeFlags(A);
    }
}

// Incorrect
void CPU::ExecuteInstruction(Mem& mem){
  byte opcode=FetchByte(mem);
  if(opcode==INS_LDA_IM.opcode){
    byte value=FetchByte(mem);
    A=value;
    SetZeroAndNegativeFlags(A);
  }
}
```

### Comments

- Write comments in Spanish or English consistently
- Comment on the "why," not the "what"
- Use documentation comments for public functions

```cpp
// Correct
// Adjust the program counter when crossing a page boundary
// to emulate the extra cycle of the original hardware
if (PageCrossed(oldPC, PC)) {
    cycles++;
}

// Incorrect
// Incrementa cycles si hay page cross
if (PageCrossed(oldPC, PC)) {
    cycles++;  // suma 1 a cycles
}
```

## 🏗️ Project Structure

The project follows a modern modular architecture:

```
cpu6502/
├── .github/              # GitHub templates and workflows
│   ├── ISSUE_TEMPLATE/   # Issue templates
│   └── pull_request_template.md
├── include/              # Public API headers
│   ├── cpu.hpp          # Public interface of the CPU
│   ├── mem.hpp          # Public interface of memory
│   └── util/
│       └── logger.hpp   # Logging system
├── src/                  # Implementations
│   ├── cpu/             # CPU implementation
│   ├── mem/             # Memory implementation
│   ├── util/            # Utilities (logger)
│   ├── main/            # Demo executable
│   └── CMakeLists.txt
├── tests/                # Test suite
│   ├── test_main.cpp
│   └── CMakeLists.txt
├── examples/             # Examples and reference binaries
├── lib/                  # External libraries (googletest)
├── build/                # Build files (ignored in git)
├── CMakeLists.txt        # Main CMake configuration
├── Makefile              # Make wrapper
├── README.md             # Main documentation
├── CONTRIBUTING.md       # This guide
├── CHANGELOG.md          # Change history
├── LICENSE               # MIT License
└── SECURITY.md           # Security policy
```

### Working Directory

- **include/**: Public headers - modify here for changes to the public API
- **src/**: Implementations - source code of the components
- **tests/**: Unit tests - always add tests for new features

## 🧪 Testing

### Running Tests

The project provides multiple ways to run tests:

#### Option 1: Make with CTest (Recommended)
```bash
make test
```

#### Option 2: Run tests directly with Make
```bash
make runTests
```

#### Option 3: With CMake manually
```bash
mkdir -p build
cd build
cmake ..
make
ctest --output-on-failure  # Or ./runTests
```

### Useful Development Commands

```bash
# Build everything
make

# Run tests
make test

# Run demo
make demo

# Clean build
make clean

# Rebuild from scratch
make rebuild

# See Makefile help
make help
```

### Writing Tests

All tests are in the `tests/` directory. We use Google Test as the testing framework.

**Test file locations:**
- Main tests: `tests/test_main.cpp`
- New tests can be added to the same file or create new files in `tests/`

Tests should:
- Be independent of each other
- Have descriptive names indicating what is being tested
- Test a single behavior or scenario
- Include edge cases
- Validate flag behavior when relevant
- Follow the Arrange-Act-Assert pattern

Example using Google Test:

```cpp
TEST_F(M6502Test1, TestLDA_IM_Zero) {
    // Arrange
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0x00;
    
    // Act
    cpu.Execute(2, mem);
    
    // Assert
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1);  // Zero flag should be set
    EXPECT_EQ(cpu.N, 0);  // Negative flag should be clear
}
```

**Update tests CMakeLists.txt:**

If you add new test files, update `tests/CMakeLists.txt`:

```cmake
set(TEST_SOURCES
    test_main.cpp
    test_nuevo.cpp  # Add here
)
```

### Required Test Categories

When adding new instructions, ensure to include tests for:

1. **Normal cases**: Typical behavior of the instruction
2. **Edge cases**: 
   - Zero values (0x00)
   - Maximum values (0xFF)
   - Memory boundaries
   - Wraparound in zero page
3. **Flag behavior**: Ensure Z, N, C, V flags are set correctly
4. **Error cases**: Behavior under unusual conditions

### Continuous Integration (CI)

The project uses GitHub Actions for CI/CD. Every push and pull request automatically runs:

1. **Build**: With CMake and Make
2. **Tests**: Full suite of unit tests
3. **Validation**: Ensures code compiles with no warnings

Tests must pass in CI before a PR can be merged. You can check the CI status in:
- The badge in the README.md
- The "Actions" tab on GitHub
- The checks in your pull request

### Debugging CI Failures

If tests fail in CI:

1. Check the logs in the "Actions" tab on GitHub
2. Reproduce the failure locally:
   ```bash
   make clean
   make
   ./runTests
   ```
3. Fix the issue and push again
4. CI will automatically run the tests again

## 📝 Documentation

- Update README.md if you add new features
- Document public functions with comments
- Add usage examples when appropriate
- Keep CHANGELOG.md updated

## ❓ Questions

If you have questions, you can:

1. Review the [existing documentation](README.md)
2. Search in [existing issues](https://github.com/Kilynho/cpu6502/issues)
3. Open a new issue with the `question` label

## 🎉 Acknowledgments

All contributions are valued! Contributors will be recognized in:
- The README.md file
- Release notes
- CHANGELOG.md

---

Thank you for contributing to the CPU 6502 Emulator! 🚀
