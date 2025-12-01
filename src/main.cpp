// /**
//  * CloudMouse SDK - Boilerplate Firmware
//  * 
//  * This is the main entry point for CloudMouse applications.
//  * Fork this project and modify it to build your custom applications.
//  * 
//  * Features:
//  * - Dual-core architecture (UI on Core 1, Logic on Core 0)
//  * - Event-driven system with hardware abstraction
//  * - Multi-platform support (Arduino IDE + PlatformIO)
//  * - Hardware components: Display, Encoder, LEDs, WiFi, Buzzer
//  */

// // Platform-specific includes for maximum compatibility
// #include "../lib/core/Core.h"
// #include "../lib/hardware/EncoderManager.h"
// #include "../lib/hardware/DisplayManager.h"
// #include "../lib/hardware/SimpleBuzzer.h"
// #include "../lib/network/WiFiManager.h"
// #include "../lib/network/WebServerManager.h"
// #include "../lib/hardware/LEDManager.h"
// #include "../lib/utils/Logger.h"

// using namespace CloudMouse;

// // Hardware component instances
// EncoderManager encoder;
// DisplayManager display;
// WiFiManager wifi;
// WebServerManager webServer(wifi);
// LEDManager ledManager;

// void setup() {
//     Serial.begin(115200);
//     delay(1000);

//     // Welcome message
//     SDK_LOGGER("");
//     SDK_LOGGER("🚀 CloudMouse SDK Boilerplate v1.0");
//     SDK_LOGGER("   Ready to build something amazing! 🎯");
    
//     // Initialize hardware components
//     SimpleBuzzer::init();
//     encoder.init();
//     display.init();
//     ledManager.init();

//     // Register components with the Core event system
//     Core::instance().setEncoder(&encoder);
//     Core::instance().setDisplay(&display);
//     Core::instance().setWiFi(&wifi);
//     Core::instance().setWebServer(&webServer);
//     Core::instance().setLEDManager(&ledManager);

//     // Start dual-core operation
//     Core::instance().startUITask();     // UI rendering on Core 1
//     Core::instance().initialize();      // Event system on Core 0
    
//     SDK_LOGGER("✅ System ready!");
// }

// void loop() {
//     // Main coordination loop (20Hz on Core 0)
//     // Core 1 handles UI independently for smooth performance
//     Core::instance().coordinationLoop();
//     delay(50);
// }