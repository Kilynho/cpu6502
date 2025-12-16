
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "scripting_api.hpp"


// Fixture para pruebas de ScriptingAPI
class ScriptingApiTest : public ::testing::Test {
protected:
    ScriptingAPI api;
};


TEST_F(ScriptingApiTest, StartAndStopHooksAreCalled) {
    int start_count = 0;
    int stop_count = 0;
    api.on_start([&]() { start_count++; });
    api.on_stop([&]() { stop_count++; });
    api.trigger_start();
    api.trigger_stop();
    EXPECT_EQ(start_count, 1);
    EXPECT_EQ(stop_count, 1);
}


TEST_F(ScriptingApiTest, BreakpointAndIOHooksAreCalled) {
    int bp_called = 0;
    int io_called = 0;
    uint16_t bp_addr = 0;
    uint16_t io_addr = 0;
    uint8_t io_val = 0;
    api.on_breakpoint([&](uint16_t addr) { bp_called++; bp_addr = addr; });
    api.on_io([&](uint16_t addr, uint8_t val) { io_called++; io_addr = addr; io_val = val; });
    api.trigger_breakpoint(0x1234);
    api.trigger_io(0xABCD, 0x42);
    EXPECT_EQ(bp_called, 1);
    EXPECT_EQ(bp_addr, 0x1234);
    EXPECT_EQ(io_called, 1);
    EXPECT_EQ(io_addr, 0xABCD);
    EXPECT_EQ(io_val, 0x42);
}


TEST_F(ScriptingApiTest, NoHooksSetDoesNotCrash) {
    // Llamar a los triggers sin hooks registrados no debe causar crash
    EXPECT_NO_THROW({
        api.trigger_start();
        api.trigger_stop();
        api.trigger_breakpoint(0x1111);
        api.trigger_io(0x2222, 0x33);
    });
}

// Puedes agregar más pruebas según la API expuesta

// Nota: Asegúrate de que 'tests/data/sample.bin' exista y sea válido para los tests.
// Los nombres de funciones y hooks deben coincidir con la API real.
