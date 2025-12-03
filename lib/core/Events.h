/**
 * CloudMouse SDK - Event System Core
 * 
 * Comprehensive event type definitions and data structures for thread-safe inter-task communication.
 * Forms the foundation of the SDK's event-driven architecture using FreeRTOS queues.
 * 
 * Architecture:
 * - Strongly-typed event enumeration for compile-time safety
 * - Fixed-size event structure optimized for FreeRTOS queue transmission
 * - Support for numeric data, string payloads, and specialized data formats
 * - Memory-efficient design with stack allocation and minimal heap usage
 * - Built-in helper methods for common data patterns (WiFi, encoder, display)
 * 
 * Event Flow:
 * Hardware/System → Event Creation → EventBus Queue → Target Task → Event Processing
 * 
 * Usage Patterns:
 * 1. Hardware events: ENCODER_ROTATION, ENCODER_CLICK, ENCODER_LONG_PRESS
 * 2. System events: BOOTING_COMPLETE, WIFI_CONNECTED, WIFI_DISCONNECTED
 * 3. Display events: DISPLAY_UPDATE, DISPLAY_CLEAR, DISPLAY_WAKE_UP
 * 4. UI events: DISPLAY_WIFI_CONNECTING, DISPLAY_WIFI_SETUP_URL
 * 
 * Memory Layout:
 * - Event: ~260 bytes total (4 + 4 + 252 bytes padding)
 * - Optimized for FreeRTOS queue efficiency
 * - String data uses fixed buffer to avoid heap fragmentation
 * - Safe for cross-task transmission without pointer issues
 * 
 * Thread Safety:
 * - All Event operations are stack-based and thread-safe
 * - No shared mutable state or heap allocations
 * - Safe for concurrent access from multiple FreeRTOS tasks
 * - Immutable after creation for predictable behavior
 */

#pragma once
#include <Arduino.h>

namespace CloudMouse {

/**
 * Event Type Enumeration
 * 
 * Defines all possible events in the CloudMouse SDK ecosystem.
 * Organized by functional category for maintainability and extensibility.
 * 
 * Categories:
 * - System: Boot sequence and lifecycle events
 * - Encoder: Hardware input events from rotary encoder
 * - Display: Screen control and content update events  
 * - WiFi Display: UI feedback for WiFi connection states
 * - WiFi System: Network stack state changes and events
 */
enum class EventType {
    // ========================================================================
    // SYSTEM LIFECYCLE EVENTS
    // ========================================================================
    
    /**
     * System boot sequence completed successfully
     * Fired when all hardware managers are initialized and system is operational
     * Usage: Trigger initial UI state, start main application logic
     */
    BOOTING_COMPLETE,
    
    // ========================================================================
    // HARDWARE INPUT EVENTS (Rotary Encoder)
    // ========================================================================
    
    /**
     * Rotary encoder rotation detected
     * value: Rotation delta (-N to +N clicks, positive = clockwise)
     * Usage: Menu navigation, value adjustment, scrolling
     */
    ENCODER_ROTATION,
    
    /**
     * Rotary encoder button pressed and released (short press)
     * value: Press duration in milliseconds
     * Usage: Menu selection, action confirmation, mode switching
     */
    ENCODER_CLICK,
    
    /**
     * Rotary encoder button held down (long press)
     * value: Total press duration in milliseconds
     * Usage: Context menus, settings access, power functions
     */
    ENCODER_LONG_PRESS,

    /**
     * Rotary encoder button is holding down
     * value: Press duration in milliseconds
     * Usage: Context menus, settings access, power functions
     */
    ENCODER_PRESS_TIME,

    /**
     * Rotary encoder button is released
     * value: Last press duration in milliseconds
     * Usage: Managing user interaction timing
     */
    ENCODER_BUTTON_RELEASED,
    
    // ========================================================================
    // DISPLAY CONTROL EVENTS
    // ========================================================================
    
    /**
     * Display should wake up from sleep/screensaver
     * Usage: User interaction detected, restore screen brightness
     */
    DISPLAY_WAKE_UP,
    
    /**
     * Display content should be refreshed
     * stringData: Optional update reason or content identifier
     * Usage: Periodic refresh, data changes, UI state transitions
     */
    DISPLAY_UPDATE,
    
    /**
     * Display should be cleared/reset
     * Usage: Mode transitions, error recovery, screen cleaning
     */
    DISPLAY_CLEAR,
    
    // ========================================================================
    // WIFI UI FEEDBACK EVENTS
    // ========================================================================
    
    /**
     * Display WiFi connection attempt in progress
     * stringData: SSID being connected to
     * value: Connection attempt number or timeout
     * Usage: Show connecting animation, SSID name, progress indication
     */
    DISPLAY_WIFI_CONNECTING,
    
    /**
     * Display successful WiFi connection status
     * stringData: "SSID|IP_ADDRESS" format (use getSSID/getIP helpers)
     * value: Connection time in milliseconds
     * Usage: Show success message, network info, IP address
     */
    DISPLAY_WIFI_CONNECTED,
    
    /**
     * Display WiFi connection error or failure
     * stringData: Error message or failed SSID
     * value: Error code or retry count
     * Usage: Show error message, suggest solutions, retry options
     */
    DISPLAY_WIFI_ERROR,
    
    /**
     * Display Access Point mode activation
     * stringData: "AP_SSID|AP_PASSWORD" format
     * Usage: Show AP credentials, setup instructions, QR code
     */
    DISPLAY_WIFI_AP_MODE,
    
    /**
     * Display WiFi setup URL for configuration
     * stringData: Setup URL (typically "http://192.168.4.1")
     * Usage: Show configuration URL, QR code generation, setup instructions
     */
    DISPLAY_WIFI_SETUP_URL,
    
    // ========================================================================
    // WIFI SYSTEM STATE EVENTS
    // ========================================================================
    
    /**
     * WiFi connection attempt started
     * stringData: Target SSID
     * value: Timeout in milliseconds
     * Usage: Internal state tracking, LED indicators, system coordination
     */
    WIFI_CONNECTING,
    
    /**
     * WiFi successfully connected with IP assignment
     * stringData: "SSID|IP_ADDRESS|GATEWAY|DNS" format
     * value: Signal strength (RSSI) in dBm
     * Usage: Enable network features, sync time, update status
     */
    WIFI_CONNECTED,
    
    /**
     * WiFi connection lost or terminated
     * stringData: Disconnection reason or last known SSID
     * value: Uptime before disconnection in seconds
     * Usage: Disable network features, attempt reconnection, update status
     */
    WIFI_DISCONNECTED,
    
    /**
     * WiFi connection error or failure
     * stringData: Error description or failed SSID
     * value: Error code (timeout, authentication, etc.)
     * Usage: Error handling, fallback to AP mode, user notification
     */
    WIFI_ERROR,
    
    /**
     * WiFi Access Point mode activated
     * stringData: "AP_SSID|AP_PASSWORD|AP_IP" format
     * Usage: Start web server, enable configuration, LED indicators
     */
    WIFI_AP_MODE,
};

/**
 * Event Data Structure
 * 
 * Unified data container for all event types with optimized memory layout.
 * Designed for efficient FreeRTOS queue transmission and minimal memory usage.
 * 
 * Memory Layout:
 * - type: 4 bytes (EventType enumeration)
 * - value: 4 bytes (signed 32-bit integer for counters, timing, codes)
 * - stringData: 256 bytes (null-terminated string buffer)
 * - Total: 264 bytes (aligned for efficient queue operations)
 * 
 * Design Principles:
 * - Fixed size for predictable memory usage
 * - No pointers to avoid cross-task memory issues
 * - Embedded string buffer to prevent heap fragmentation
 * - Helper methods for common data patterns
 * - Safe defaults and automatic null termination
 */
struct Event {
    EventType type;        // Event classification and routing information
    int32_t value;         // Numeric payload: counters, timing, error codes, measurements
    char stringData[256];  // String payload: messages, identifiers, formatted data
    
    // ========================================================================
    // CONSTRUCTORS - Safe initialization with proper defaults
    // ========================================================================
    
    /**
     * Default constructor - creates safe empty event
     * Initializes with ENCODER_ROTATION type and zero values
     */
    Event() : type(EventType::ENCODER_ROTATION), value(0) {
        memset(stringData, 0, sizeof(stringData));
    }
    
    /**
     * Type-only constructor
     * Creates event with specified type and zero values
     * 
     * @param t Event type from EventType enumeration
     */
    Event(EventType t) : type(t), value(0) {
        memset(stringData, 0, sizeof(stringData));
    }
    
    /**
     * Type and value constructor
     * Creates event with specified type and numeric value
     * 
     * @param t Event type from EventType enumeration
     * @param v Numeric value (counter, timing, error code, etc.)
     */
    Event(EventType t, int32_t v) : type(t), value(v) {
        memset(stringData, 0, sizeof(stringData));
    }
    
    // ========================================================================
    // STRING DATA MANAGEMENT - Safe string operations with bounds checking
    // ========================================================================
    
    /**
     * Set string payload with automatic truncation and null termination
     * Safely copies string data with bounds checking to prevent buffer overflow
     * 
     * @param str String data to store (will be truncated if > 255 characters)
     */
    void setStringData(const String& str) {
        strncpy(stringData, str.c_str(), sizeof(stringData) - 1);
        stringData[sizeof(stringData) - 1] = '\0';  // Ensure null termination
    }
    
    /**
     * Get string payload as Arduino String object
     * Safe accessor that always returns valid string (empty if unset)
     * 
     * @return String object containing current string data
     */
    String getStringData() const { 
        return String(stringData); 
    }
    
    /**
     * Check if string data is present and non-empty
     * 
     * @return true if string data contains at least one character
     */
    bool hasStringData() const {
        return stringData[0] != '\0';
    }
    
    /**
     * Clear string data buffer
     * Resets string buffer to empty state
     */
    void clearStringData() {
        memset(stringData, 0, sizeof(stringData));
    }
    
    // ========================================================================
    // WIFI DATA HELPERS - Specialized accessors for WiFi event data
    // ========================================================================
    
    /**
     * Set WiFi-specific data using structured format
     * Stores SSID, IP address, and connection timing in standardized format
     * Format: "SSID|IP_ADDRESS" with connection time in value field
     * 
     * @param ssid Network SSID (max ~120 characters)
     * @param ip IP address string (typically "192.168.1.100" format)
     * @param connectionTime Connection duration in milliseconds (stored in value)
     */
    void setWiFiData(const char* ssid, const char* ip = "", int32_t connectionTime = 0) {
        value = connectionTime;
        snprintf(stringData, sizeof(stringData), "%s|%s", ssid, ip);
    }
    
    /**
     * Extract SSID from WiFi event data
     * Parses SSID from "SSID|IP" format string
     * 
     * @return SSID string, or complete string if no separator found
     */
    String getSSID() const {
        String data(stringData);
        int separatorIndex = data.indexOf('|');
        return separatorIndex >= 0 ? data.substring(0, separatorIndex) : data;
    }
    
    /**
     * Extract IP address from WiFi event data
     * Parses IP address from "SSID|IP" format string
     * 
     * @return IP address string, or empty string if no separator found
     */
    String getIP() const {
        String data(stringData);
        int separatorIndex = data.indexOf('|');
        return separatorIndex >= 0 ? data.substring(separatorIndex + 1) : "";
    }
    
    /**
     * Get WiFi connection time from value field
     * Convenience accessor for connection timing data
     * 
     * @return Connection time in milliseconds
     */
    int32_t getConnectionTime() const {
        return value;
    }
    
    // ========================================================================
    // ENCODER DATA HELPERS - Specialized accessors for encoder events
    // ========================================================================
    
    /**
     * Get encoder rotation delta
     * Returns rotation amount with sign indicating direction
     * 
     * @return Rotation clicks (positive = clockwise, negative = counter-clockwise)
     */
    int32_t getRotationDelta() const {
        return value;
    }
    
    /**
     * Get encoder press duration
     * Returns button press time for click/long-press differentiation
     * 
     * @return Press duration in milliseconds
     */
    int32_t getPressDuration() const {
        return value;
    }
    
    /**
     * Check if encoder rotation is clockwise
     * 
     * @return true if rotation value is positive (clockwise)
     */
    bool isClockwise() const {
        return value > 0;
    }
    
    /**
     * Check if encoder press qualifies as long press
     * Uses common threshold of 1000ms for long press detection
     * 
     * @param threshold Long press threshold in milliseconds (default: 1000)
     * @return true if press duration exceeds threshold
     */
    bool isLongPress(int32_t threshold = 1000) const {
        return value >= threshold;
    }
};

}  // namespace CloudMouse