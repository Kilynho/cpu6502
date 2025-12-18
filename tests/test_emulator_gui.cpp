#include <gtest/gtest.h>
#include "gui/emulator_gui.hpp"
#include "devices/text_screen.hpp"
#include <memory>

/**
 * @brief Unit tests for EmulatorGUI
 * 
 * Note: These tests are limited because SDL2 GUI testing requires
 * a display environment. We test the basic functionality that doesn't
 * require actual window creation.
 */

class EmulatorGUITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create GUI and TextScreen instances
        textScreen = std::make_shared<TextScreen>();
    }
    
    void TearDown() override {
        textScreen.reset();
    }
    
    std::shared_ptr<TextScreen> textScreen;
};

// Test basic construction
TEST_F(EmulatorGUITest, Construction) {
    EmulatorGUI gui("Test Window", 8, 8);
    EXPECT_FALSE(gui.isInitialized());
}

// Test TextScreen attachment
TEST_F(EmulatorGUITest, AttachTextScreen) {
    EmulatorGUI gui("Test Window", 8, 8);
    
    // Should not crash when attaching TextScreen
    EXPECT_NO_THROW(gui.attachTextScreen(textScreen));
}

// Test key handling before initialization
TEST_F(EmulatorGUITest, KeyHandlingBeforeInit) {
    EmulatorGUI gui("Test Window", 8, 8);
    
    // Should have no keys available before initialization
    EXPECT_FALSE(gui.hasKey());
    EXPECT_EQ(gui.getLastKey(), 0);
}

// Test that GUI construction with different parameters works
TEST_F(EmulatorGUITest, DifferentParameters) {
    EmulatorGUI gui1("Window 1", 16, 16);
    EmulatorGUI gui2("Window 2", 8, 12);
    EmulatorGUI gui3("Window 3", 12, 8);
    
    // All should be uninitialized until initialize() is called
    EXPECT_FALSE(gui1.isInitialized());
    EXPECT_FALSE(gui2.isInitialized());
    EXPECT_FALSE(gui3.isInitialized());
}

// Test integration with TextScreen
TEST_F(EmulatorGUITest, TextScreenIntegration) {
    EmulatorGUI gui("Test Window", 8, 8);
    gui.attachTextScreen(textScreen);
    
    // Write some text to TextScreen
    const char* testMsg = "Hello, GUI!";
    for (const char* p = testMsg; *p; p++) {
        textScreen->writeCharAtCursor(*p);
    }
    
    // Verify TextScreen has the content
    std::string buffer = textScreen->getBuffer();
    EXPECT_TRUE(buffer.find("Hello, GUI!") != std::string::npos);
}

// Test that TextScreen can be used without GUI
TEST_F(EmulatorGUITest, TextScreenStandalone) {
    textScreen->clear();
    
    // Write some characters
    textScreen->writeCharAtCursor('A');
    textScreen->writeCharAtCursor('B');
    textScreen->writeCharAtCursor('C');
    
    std::string buffer = textScreen->getBuffer();
    EXPECT_TRUE(buffer.find("ABC") != std::string::npos);
}

// Test cursor positioning with GUI
TEST_F(EmulatorGUITest, CursorPositioning) {
    EmulatorGUI gui("Test Window", 8, 8);
    gui.attachTextScreen(textScreen);
    
    // Set cursor position
    textScreen->setCursorPosition(10, 5);
    
    uint8_t col, row;
    textScreen->getCursorPosition(col, row);
    
    EXPECT_EQ(col, 10);
    EXPECT_EQ(row, 5);
    
    // Write a character
    textScreen->writeCharAtCursor('X');
    
    // Cursor should have advanced
    textScreen->getCursorPosition(col, row);
    EXPECT_EQ(col, 11);
    EXPECT_EQ(row, 5);
}
