# Genera la ROM de msbasic antes de configurar el emulador
.PHONY: msbasic_rom
msbasic_rom:
	cd msbasic && ./make.sh
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
configure: msbasic_rom
	@if [ ! -d "$(BUILDDIR)" ] || [ ! -f "$(BUILDDIR)/Makefile" ]; then \
		echo "Initializing Git submodules (GoogleTest)..."; \
		git submodule update --init --recursive; \
		echo "Configurando proyecto con CMake..."; \
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

# Run Apple IO demo
apple_io_demo: all
	@echo "Running Apple IO demo..."
	@$(BUILDDIR)/apple_io_demo

# Run File Device demo
file_device_demo: all
	@echo "Running File Device demo..."
	@$(BUILDDIR)/file_device_demo

# Run Text Screen demo
text_screen_demo: all
	@echo "Running Text Screen demo..."
	@$(BUILDDIR)/text_screen_demo

# Run Audio demo
audio_demo: all
	@echo "Running Audio demo..."
	@$(BUILDDIR)/audio_demo

# Run TCP Serial demo
tcp_serial_demo: all
	@echo "Running TCP Serial demo..."
	@$(BUILDDIR)/tcp_serial_demo

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
	@echo "  make demo         - Build and run CPU demo program"
	@echo "  make apple_io_demo    - Build and run Apple IO demo"
	@echo "  make file_device_demo - Build and run File Device demo"
	@echo "  make text_screen_demo - Build and run Text Screen demo"
	@echo "  make audio_demo       - Build and run Audio demo"
	@echo "  make tcp_serial_demo  - Build and run TCP Serial demo"
	@echo "  make interrupt_demo   - Build and run Interrupt demo"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make rebuild      - Clean and build from scratch"
	@echo "  make reconfigure  - Force CMake reconfiguration"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Build artifacts will be in: $(BUILDDIR)/"


# Run Interrupt demo
interrupt_demo: all
	@echo "Running Interrupt demo..."
	@$(BUILDDIR)/interrupt_demo

# Declare phony targets
.PHONY: all configure test runTests demo apple_io_demo file_device_demo text_screen_demo audio_demo tcp_serial_demo interrupt_demo clean rebuild reconfigure install help
