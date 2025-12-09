# Makefile - CMake wrapper for CPU 6502 Emulator
# This Makefile provides convenient shortcuts for CMake-based builds

# Build directory
BUILDDIR := build

# Default target: build everything
all: configure
	@echo "Building project with CMake..."
	@$(MAKE) -C $(BUILDDIR) --no-print-directory
	@echo ""
	@echo "✓ Build complete!"
	@echo "  - Library: $(BUILDDIR)/src/libcpu6502_lib.a"
	@echo "  - Demo:    $(BUILDDIR)/cpu_demo"
	@echo "  - Tests:   $(BUILDDIR)/runTests"

# Configure CMake (only runs if build directory doesn't exist or CMakeLists.txt changed)
configure:
	@if [ ! -d "$(BUILDDIR)" ] || [ ! -f "$(BUILDDIR)/Makefile" ]; then \
		echo "Initializing Git submodules (GoogleTest)..."; \
		git submodule update --init --recursive; \
		echo "Configuring project with CMake..."; \
		mkdir -p $(BUILDDIR); \
		cd $(BUILDDIR) && cmake ..; \
	fi

# Run tests
test: all
	@echo "Running tests with CTest..."
	@cd $(BUILDDIR) && ctest --output-on-failure

# Run tests directly (without CTest wrapper)
runTests: all
	@echo "Running tests directly..."
	@$(BUILDDIR)/runTests

# Run demo
demo: all
	@echo "Running CPU demo..."
	@$(BUILDDIR)/cpu_demo

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILDDIR)
	@echo "✓ Clean complete!"

# Rebuild everything from scratch
rebuild: clean all

# Force reconfigure (useful if CMakeLists.txt changed)
reconfigure:
	@echo "Forcing CMake reconfiguration..."
	@rm -rf $(BUILDDIR)
	@$(MAKE) configure

# Install (if CMAKE_INSTALL_PREFIX is set)
install: all
	@echo "Installing..."
	@cd $(BUILDDIR) && $(MAKE) install

# Show help
help:
	@echo "CPU 6502 Emulator - Makefile targets:"
	@echo ""
	@echo "  make              - Build all targets (library, demo, tests)"
	@echo "  make test         - Build and run tests with CTest"
	@echo "  make runTests     - Build and run tests directly"
	@echo "  make demo         - Build and run demo program"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make rebuild      - Clean and build from scratch"
	@echo "  make reconfigure  - Force CMake reconfiguration"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Build artifacts will be in: $(BUILDDIR)/"

# Declare phony targets
.PHONY: all configure test runTests demo clean rebuild reconfigure install help
