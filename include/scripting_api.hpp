#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>

// Forward declaration for pybind11
namespace pybind11 { class module_; }

/**
 * @brief Scripting API for event hooks and Python bindings.
 *
 * Allows registration of C++ and Python callbacks for CPU events.
 * Events: on_start, on_stop, on_breakpoint, on_io
 */
class ScriptingAPI {
public:
    using Callback = std::function<void()>;
    using BreakpointCallback = std::function<void(uint16_t)>;
    using IOCallback = std::function<void(uint16_t, uint8_t)>;

    ScriptingAPI();
    ~ScriptingAPI();

    // Register C++ callbacks
    void on_start(const Callback& cb);
    void on_stop(const Callback& cb);
    void on_breakpoint(const BreakpointCallback& cb);
    void on_io(const IOCallback& cb);

    // Trigger events (for testability)
    void trigger_start();
    void trigger_stop();
    void trigger_breakpoint(uint16_t address);
    void trigger_io(uint16_t address, uint8_t value);

    // Python binding
    static void bind(pybind11::module_& m);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

