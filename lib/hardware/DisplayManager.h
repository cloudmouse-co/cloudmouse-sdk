/**
 * CloudMouse SDK - Display Management System
 *
 * Comprehensive display controller for ILI9488-based TFT screens with hardware-accelerated
 * sprite rendering, event-driven UI updates, and integrated QR code generation capabilities.
 * Provides high-level interface for multi-screen applications with smooth animations and responsive feedback.
 *
 * Features:
 * - Hardware-accelerated sprite rendering using PSRAM buffering
 * - Event-driven architecture with automatic screen state management
 * - Multi-screen support with smooth transitions and animations
 * - Integrated QR code generation for WiFi setup and configuration
 * - Responsive UI elements with encoder interaction feedback
 * - Color-coded status indication and visual feedback systems
 * - Memory-efficient double buffering for flicker-free updates
 *
 * Hardware Integration:
 * - ILI9488 TFT controller (480x320 resolution, 16-bit color)
 * - SPI communication with configurable frequency (40MHz write, 16MHz read)
 * - PWM backlight control with adjustable brightness (0-255)
 * - PSRAM utilization for large sprite buffers (307KB full-screen buffer)
 * - Hardware power management for display enable/disable
 *
 * Display Architecture:
 * - Resolution: 480x320 pixels (landscape orientation)
 * - Color depth: 16-bit RGB565 (65,536 colors)
 * - Memory usage: 307KB PSRAM for full-screen sprite buffer
 * - Refresh strategy: Event-driven updates with selective redraw
 * - Animation support: Smooth 60fps capability with frame limiting
 *
 * Screen Management:
 * - HELLO_WORLD: Default interactive screen with encoder status
 * - WIFI_CONNECTING: Animated connection progress with spinner
 * - WIFI_AP_MODE: QR code for WiFi network connection
 * - WIFI_AP_CONNECTED: QR code for web-based configuration
 *
 * Event Processing:
 * - Receives events from EventBus for screen state changes
 * - Processes encoder input for interactive feedback
 * - Handles WiFi status updates for connection visualization
 * - Manages animation timing and frame rate control
 *
 * Performance Characteristics:
 * - Rendering speed: ~16ms full-screen update (60fps capable)
 * - Memory footprint: 307KB PSRAM + ~5KB RAM for state
 * - SPI bandwidth: 40MHz sustained transfer rate
 * - Animation smoothness: 10fps spinner, 2Hz text blinking
 *
 * Integration Pattern:
 * 1. Initialize during system startup after PSRAM availability
 * 2. Call update() regularly in UI task loop (20-50Hz recommended)
 * 3. Events automatically trigger appropriate screen updates
 * 4. Manual rendering methods available for custom UI elements
 */

#pragma once

#include <Arduino.h>
#include "LGFX_ILI9488.h"
#include "../core/Events.h"
#include "../utils/QRCodeManager.h"
#include "../config/DeviceConfig.h"

/**
 * Operational and idle brightness targets with dimmer fade out
 */
#define BRIGHTNESS_UP_TARGET 180
#define BRIGHTNESS_IDLE_TARGET 10
#define FADE_OUT_STEP_DELAY_MS 20
#define FADE_OUT_STEP_VALUE 2

/**
 * Display Management Controller
 *
 * High-level display controller providing event-driven UI management with multiple
 * screen states, smooth animations, and integrated QR code generation capabilities.
 *
 * Architecture:
 * - Event-driven screen state machine with automatic transitions
 * - Hardware-accelerated sprite rendering with PSRAM buffering
 * - Modular screen rendering system for extensible UI development
 * - Integrated timing management for smooth animations and responsive updates
 *
 * Memory Management:
 * - Uses PSRAM for large sprite buffer to preserve main RAM
 * - Automatic sprite allocation and cleanup in constructor/destructor
 * - Efficient color palette with predefined UI theme colors
 * - Memory-conscious rendering with selective update regions
 *
 * Thread Safety:
 * - Designed for single UI task usage (typically Core1)
 * - Event processing is non-blocking with queue-based communication
 * - Sprite operations are inherently thread-safe when called from single task
 * - No shared mutable state across multiple tasks
 */

namespace CloudMouse::Hardware
{
    using namespace CloudMouse::Utils;

    /**
     * custom DisplayManager callback definition
     */
    typedef void (*AppDisplayCallback)(const CloudMouse::Event& event);

    class DisplayManager
    {
    public:
        /**
         * Constructor - initializes display manager instance
         * Prepares internal state but does not configure hardware
         * Call init() separately for actual display initialization
         */
        DisplayManager();

        /**
         * Destructor - ensures proper cleanup of sprite resources
         * Automatically frees PSRAM sprite buffer and resets display state
         */
        ~DisplayManager();

        // ========================================================================
        // SYSTEM LIFECYCLE
        // ========================================================================

        /**
         * Initialize display hardware and create sprite buffer
         * Configures ILI9488 controller, allocates PSRAM buffer, and sets up QR code manager
         *
         * Initialization Steps:
         * 1. Configure ILI9488 hardware (SPI, pins, power management)
         * 2. Set display brightness and orientation (landscape mode)
         * 3. Allocate full-screen sprite buffer in PSRAM (307KB)
         * 4. Initialize QR code generation system
         * 5. Set default text properties and color scheme
         *
         * Hardware Requirements:
         * - Sufficient PSRAM available (>400KB recommended)
         * - Properly configured SPI pins and power management
         * - ILI9488 controller properly connected and powered
         *
         * @note Must be called after PSRAM initialization
         * @note Not thread-safe - call only from single initialization thread
         */
        void init();

        /**
         * Process pending display events and update animations
         * Handles incoming events from EventBus and manages animation timing
         * Should be called regularly from UI task loop for responsive display updates
         *
         * Processing Operations:
         * 1. Receive and process all pending events from EventBus
         * 2. Update animation frames for active animated screens
         * 3. Trigger redraws based on timing and state changes
         * 4. Manage frame rate limiting for smooth animations
         *
         * Recommended Call Frequency:
         * - 20-50Hz for responsive UI updates
         * - Higher frequencies improve animation smoothness
         * - Lower frequencies reduce CPU usage but may impact responsiveness
         *
         * @note Non-blocking operation with configurable timeout
         */
        void update();

        // ========================================================================
        // EVENT PROCESSING INTERFACE
        // ========================================================================

        /**
         * Process individual display event and update screen state
         * Handles event-driven screen transitions and UI state updates
         * Called internally by update() but available for manual event injection
         *
         * @param event Event structure containing type and payload data
         *
         * Supported Event Types:
         * - DISPLAY_WAKE_UP: Activate display and show default screen
         * - DISPLAY_WIFI_CONNECTING: Show animated connection progress
         * - DISPLAY_WIFI_AP_MODE: Display QR code for WiFi network access
         * - DISPLAY_WIFI_SETUP_URL: Show QR code for web configuration
         * - ENCODER_ROTATION: Update encoder interaction feedback
         * - ENCODER_CLICK: Process button click feedback
         * - ENCODER_LONG_PRESS: Handle long press interaction
         * - DISPLAY_CLEAR: Clear screen to background color
         *
         * State Management:
         * - Automatic screen state transitions based on event type
         * - Animation control (start/stop) based on screen requirements
         * - Persistent state tracking for interactive elements
         */
        void processEvent(const CloudMouse::Event &event);

        /**
         * Register callback function for custom DislpayManager
         * 
         * @param callback Custom DisplayManager event process function
         */
        void registerAppCallback(AppDisplayCallback callback) {
            appCallback = callback;
        }

        // ========================================================================
        // STATUS QUERY INTERFACE
        // ========================================================================

        /**
         * Check if display manager is properly initialized
         * Validates hardware initialization and sprite buffer allocation
         *
         * @return true if display is ready for rendering operations
         */
        bool isReady() const { return initialized; }

        /**
         * Get display width in pixels
         * Returns configured display width for layout calculations
         *
         * @return Display width (480 pixels in landscape mode)
         */
        int getWidth() const { return 480; }

        /**
         * Get display height in pixels
         * Returns configured display height for layout calculations
         *
         * @return Display height (320 pixels in landscape mode)
         */
        int getHeight() const { return 320; }

        /**
         * Check if animations are currently active
         * Useful for performance monitoring and power management
         *
         * @return true if screen requires continuous animation updates
         */
        bool isAnimating() const { return needsRedraw; }

    private:
        // ========================================================================
        // SCREEN STATE ENUMERATION
        // ========================================================================

        /**
         * Screen State Definitions
         * Defines all possible display states with specific rendering and behavior characteristics
         */
        enum class Screen
        {
            /**
             * Default interactive screen showing encoder status and system information
             * Features: Real-time encoder feedback, button press indication, usage instructions
             * Animation: None (static content with event-driven updates)
             */
            HELLO_WORLD,

            /**
             * WiFi connection progress screen with animated spinner
             * Features: Connection status text, animated loading spinner, progress indication
             * Animation: 10fps spinner rotation, 2Hz text blinking for dots
             */
            WIFI_CONNECTING,

            /**
             * Access Point mode screen with WiFi connection QR code
             * Features: AP credentials display, WiFi connection QR code, setup instructions
             * Animation: None (static QR code and credentials)
             */
            WIFI_AP_MODE,

            /**
             * Configuration screen with web setup QR code
             * Features: Success confirmation, web configuration QR code, URL display
             * Animation: None (static configuration interface)
             */
            WIFI_AP_CONNECTED
        };

        // ========================================================================
        // HARDWARE INTERFACE COMPONENTS
        // ========================================================================

        LGFX_ILI9488 display;          // Hardware display controller interface
        LGFX_Sprite *sprite = nullptr; // PSRAM-based sprite buffer for flicker-free rendering
        QRCodeManager qrCodeManager;   // Integrated QR code generation and rendering

        AppDisplayCallback appCallback = nullptr;   // Custom DisplayManager callback for SDK event forwarding

        // ========================================================================
        // STATE MANAGEMENT VARIABLES
        // ========================================================================

        bool initialized = false; // Hardware initialization status flag

        // Current display state
        Screen currentScreen = Screen::HELLO_WORLD; // Active screen state
        bool needsRedraw = true;                    // Animation/continuous update flag

        // Interactive state tracking
        int lastEncoderValue = 0;            // Most recent encoder rotation value
        int lastEncoderPushAndRotateValue = 0;
        unsigned long lastClickTime = 0;     // Timestamp of last button click
        unsigned long lastLongPressTime = 0; // Timestamp of last long press
        unsigned long lastDoubleClickTime = 0;
        unsigned long lastPushAndRotateTime = 0;
        
        // Animation timing control
        unsigned long lastWiFiConnectingRender = 0;                       // Last WiFi animation frame time
        static const unsigned long WIFI_CONNECTING_RENDER_INTERVAL = 100; // 100ms = 10fps animation

        // Legacy state variables (maintained for compatibility)
        bool inAPMode = false; // Access Point mode flag

        // Brightness management variables
        int currentBrightness = BRIGHTNESS_UP_TARGET;
        unsigned long lastInteractionTime = 0;
        unsigned long lastFadeTime = 0;

        // ========================================================================
        // UI COLOR SCHEME DEFINITIONS
        // ========================================================================

        static const uint16_t COLOR_BG = 0x0000;      // Background: Black
        static const uint16_t COLOR_TEXT = 0xFFFF;    // Primary text: White
        static const uint16_t COLOR_ACCENT = 0x07FF;  // Accent elements: Cyan
        static const uint16_t COLOR_SUCCESS = 0x07E0; // Success indicators: Green
        static const uint16_t COLOR_WARNING = 0xFD20; // Warning indicators: Orange

        // ========================================================================
        // SCREEN RENDERING METHODS
        // ========================================================================

        /**
         * Render default interactive screen with encoder status
         * Displays system information, encoder interaction feedback, and usage instructions
         * Updates based on recent user interactions and encoder state
         */
        void renderHelloWorld();

        /**
         * Render Access Point setup screen with QR code
         * Shows WiFi network credentials and generates QR code for easy connection
         * Static content with embedded AP credentials and connection instructions
         */
        void renderAPMode();

        /**
         * Render configuration success screen with setup QR code
         * Displays web configuration URL and generates QR code for browser access
         * Confirms successful AP connection and provides next steps
         */
        void renderAPConnected();

        /**
         * Render animated WiFi connection progress screen
         * Shows connection status with animated spinner and dynamic text updates
         * Provides visual feedback during network connection attempts
         */
        void renderWiFiConnecting();

        /**
         * Push sprite buffer contents to physical display
         * Transfers complete sprite buffer to display controller via SPI
         * Handles double-buffering for flicker-free updates
         */
        void pushSprite();

        // ========================================================================
        // UI ELEMENT RENDERING UTILITIES
        // ========================================================================

        /**
         * Draw header bar with title text
         * Renders consistent header design across all screens
         *
         * @param title Header text to display
         */
        void drawHeader(const String &title);

        /**
         * Draw centered text at specified vertical position
         * Utility for consistent text alignment and positioning
         *
         * @param text Text string to display
         * @param y Vertical position (pixels from top)
         * @param color Text color (default: COLOR_TEXT)
         */
        void drawCenteredText(const String &text, int y, uint16_t color = COLOR_TEXT);

        /**
         * Draw usage instructions footer
         * Displays encoder operation instructions at bottom of screen
         * Provides consistent user guidance across interactive screens
         */
        void drawInstructions();

        // ========================================================================
        // SCREEN BRIGHTNESS MANAGEMENT
        // ========================================================================
        
        void wakeUp();
        void handleDimmer();
    };
};