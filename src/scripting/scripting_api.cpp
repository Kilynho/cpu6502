#include "scripting_api.hpp"
#include <pybind11/pybind11.h>
#include <vector>
#include <mutex>

struct ScriptingAPI::Impl {
    std::vector<Callback> start_cbs;
    std::vector<Callback> stop_cbs;
    std::vector<BreakpointCallback> breakpoint_cbs;
    std::vector<IOCallback> io_cbs;
    std::mutex mtx;
};

ScriptingAPI::ScriptingAPI() : impl_(new Impl) {}
ScriptingAPI::~ScriptingAPI() = default;

void ScriptingAPI::on_start(const Callback& cb) {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    impl_->start_cbs.push_back(cb);
}
void ScriptingAPI::on_stop(const Callback& cb) {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    impl_->stop_cbs.push_back(cb);
}
void ScriptingAPI::on_breakpoint(const BreakpointCallback& cb) {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    impl_->breakpoint_cbs.push_back(cb);
}
void ScriptingAPI::on_io(const IOCallback& cb) {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    impl_->io_cbs.push_back(cb);
}

void ScriptingAPI::trigger_start() {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    for (auto& cb : impl_->start_cbs) cb();
}
void ScriptingAPI::trigger_stop() {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    for (auto& cb : impl_->stop_cbs) cb();
}
void ScriptingAPI::trigger_breakpoint(uint16_t address) {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    for (auto& cb : impl_->breakpoint_cbs) cb(address);
}
void ScriptingAPI::trigger_io(uint16_t address, uint8_t value) {
    std::lock_guard<std::mutex> lock(impl_->mtx);
    for (auto& cb : impl_->io_cbs) cb(address, value);
}

void ScriptingAPI::bind(pybind11::module_& m) {
    namespace py = pybind11;
    py::class_<ScriptingAPI>(m, "ScriptingAPI")
        .def(py::init<>())
        .def("on_start", [](ScriptingAPI& self, py::function f) {
            self.on_start([f]() {
                py::gil_scoped_acquire acquire;
                f();
            });
        })
        .def("on_stop", [](ScriptingAPI& self, py::function f) {
            self.on_stop([f]() {
                py::gil_scoped_acquire acquire;
                f();
            });
        })
        .def("on_breakpoint", [](ScriptingAPI& self, py::function f) {
            self.on_breakpoint([f](uint16_t addr) {
                py::gil_scoped_acquire acquire;
                f(addr);
            });
        })
        .def("on_io", [](ScriptingAPI& self, py::function f) {
            self.on_io([f](uint16_t addr, uint8_t val) {
                py::gil_scoped_acquire acquire;
                f(addr, val);
            });
        })
        .def("trigger_start", &ScriptingAPI::trigger_start)
        .def("trigger_stop", &ScriptingAPI::trigger_stop)
        .def("trigger_breakpoint", &ScriptingAPI::trigger_breakpoint)
        .def("trigger_io", &ScriptingAPI::trigger_io);
}