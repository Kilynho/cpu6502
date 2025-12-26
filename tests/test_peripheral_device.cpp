#include "peripheral_device.hpp"
#include "include/src/devices/via_device.hpp"
#include "include/devices/joystick_device.hpp"
#include <gtest/gtest.h>

TEST(PeripheralDeviceTest, VIADeviceAccess) {
    VIADevice via;
    via.writeRegister(0x00, 0x42);
    EXPECT_EQ(via.readRegister(0x00), 0x00); // TODO: Update expected value
}

TEST(PeripheralDeviceTest, JoystickDeviceAccess) {
    JoystickDevice js;
    js.writeRegister(0x00, 0x99);
    EXPECT_EQ(js.readRegister(0x00), 0x00); // TODO: Update expected value
}
