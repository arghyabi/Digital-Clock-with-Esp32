#include <RTClib.h>
#include <stdint.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "esp32PinMap.h"
#include "digitalClock.h"

#include <IRremote.h>
#include "remote_map.h"
#include "AutoGen/config.h"


int brightnessLevel = DISPLAY_BRIGHTNESS;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Function to get key name from IR code
String getKeyName(uint32_t irCode) {
    // First try matching full 32-bit codes from remote_map.h
    switch(irCode) {
        case NUMBER_1: return "NUMBER_1";
        case NUMBER_2: return "NUMBER_2";
        case NUMBER_3: return "NUMBER_3";
        case NUMBER_4: return "NUMBER_4";
        case NUMBER_5: return "NUMBER_5";
        case NUMBER_6: return "NUMBER_6";
        case NUMBER_7: return "NUMBER_7";
        case NUMBER_8: return "NUMBER_8";
        case NUMBER_9: return "NUMBER_9";
        case NUMBER_0: return "NUMBER_0";
        case STAR_KEY: return "STAR_KEY";
        case HASH_KEY: return "HASH_KEY";
        case UP_KEY:
            brightnessLevel += 5;
            if (brightnessLevel > 255) brightnessLevel = 255;
                return "UP_KEY";
        case DOWN_KEY:
            brightnessLevel -= 5;
            if (brightnessLevel < 0) brightnessLevel = 0;
                return "DOWN_KEY";
        case LEFT_KEY: return "LEFT_KEY";
        case RGHT_KEY: return "RIGHT_KEY";
        case OK_KEY:   return "OK_KEY";
        default:       break; // Continue to command byte matching
    }

    return "UNKNOWN_KEY";
}// Function to handle key press events

void handleKeyPress(uint32_t irCode) {
    String keyName = getKeyName(irCode);

    Serial.print("Key Pressed: ");
    Serial.println(keyName);
}


// Number of shift registers
#define NUM_REGISTERS 3
#define TOTAL_BITS (NUM_REGISTERS * 8)

// Function to shift out data to all registers
void shiftOutData(byte reg1, byte reg2, byte reg3) {
    digitalWrite(SIPO_LATCH_PIN, LOW);  // Prepare for data input

    // Shift out data for register 3 (furthest from ESP32)
    shiftOut(SIPO_DATA_PIN, SIPO_CLOCK_PIN, MSBFIRST, reg3);
    // Shift out data for register 2 (middle)
    shiftOut(SIPO_DATA_PIN, SIPO_CLOCK_PIN, MSBFIRST, reg2);
    // Shift out data for register 1 (closest to ESP32)
    shiftOut(SIPO_DATA_PIN, SIPO_CLOCK_PIN, MSBFIRST, reg1);

    digitalWrite(SIPO_LATCH_PIN, HIGH); // Latch the data to outputs
}


RTC_DS3231 rtc;
ClockTime  clockTime;
MillisTracker millisTracker;
bool blinkState = false;

#define PWM_CHANNEL 0
#define PWM_FREQ 25000      // 25 kHz (silent)
#define PWM_RESOLUTION 8

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Digital Clock"+String(" - Version ")+String(APP_VERSION));
    Serial.println("===================");

    // Initialize pins
    pinMode(SIPO_DATA_PIN,  OUTPUT);
    pinMode(SIPO_LATCH_PIN, OUTPUT);
    pinMode(SIPO_CLOCK_PIN, OUTPUT);
    // pinMode(BRIGHTNESS_PIN, OUTPUT);
    pinMode(BLINK_PIN,      OUTPUT);

    // analogWrite(BRIGHTNESS_PIN, brightnessLevel); // Set brightness to mid-level (example use)

    ledcAttach(BRIGHTNESS_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcWrite(BRIGHTNESS_PIN, brightnessLevel);

    bool wireInitialized = Wire.begin(); // Initialize I2C
    bool rtcInitialized = rtc.begin();
    IrReceiver.begin(IR_RECV_PIN);

    Serial.print("I2C Initialized: ");
    Serial.println(wireInitialized ? "Yes" : "No");
    Serial.print("RTC Initialized: ");
    Serial.println(rtcInitialized ? "Yes" : "No");

    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // Start with all outputs low
    // clearAllRegisters();
    shiftOutData(0x00, 0x00, 0x00);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay();

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("ESP32 Digital Clock - "+String(APP_VERSION));
    display.display();
    delay(2000);

}

void IrReceiver_loop() {
    if (IrReceiver.decode()) {
        // Filter out unknown protocols
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            IrReceiver.resume();
            return;
        }

        // Try to get the full raw data first, then fallback to command byte
        uint32_t receivedCode = IrReceiver.decodedIRData.decodedRawData;
        if (receivedCode == 0) {
            // If raw data is 0, use command byte
            receivedCode = IrReceiver.decodedIRData.command;
        }

        // Handle the key press
        handleKeyPress(receivedCode);
        // analogWrite(BRIGHTNESS_PIN, brightnessLevel);
        ledcWrite(BRIGHTNESS_PIN, brightnessLevel);

        IrReceiver.resume();
    }
}


void loop() {
    IrReceiver_loop();
    DateTime now = rtc.now();
    uint16_t hour24 = now.hour();
    uint16_t hour = hour24;
    if (hour24 == 0) {
        hour = 12; // Midnight becomes 12 AM
    } else if (hour24 > 12) {
        hour = hour24 - 12; // Convert to 12-hour format
    }
    uint16_t minute = now.minute();
    uint16_t second = now.second();

    Serial.print("Current Time: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);

    display.clearDisplay();
    display.setCursor(1,0);
    display.printf("Time: %02d:%02d:%02d\n", hour, minute, second);
    display.printf("Brightness: %d", brightnessLevel);
    display.display();

    // Convert to BCD
    byte hourTens = hour / 10;
    byte hourOnes = hour % 10;
    byte minuteTens = minute / 10;
    byte minuteOnes = minute % 10;
    byte secondTens = second / 10;
    byte secondOnes = second % 10;
    byte reg1 = (hourOnes << 4) | hourTens;       // HH
    byte reg2 = (minuteOnes << 4) | minuteTens;   // MM
    byte reg3 = (secondOnes << 4) | secondTens;   // SS

    shiftOutData(reg1, reg2, reg3);
    // delay(500); // Update every second
    digitalWrite(BLINK_PIN, blinkState ? HIGH : LOW);

    if (millis() - millisTracker.lastBlinkMillis >= BLINK_INTERVAL) {
        millisTracker.lastBlinkMillis = millis();
        blinkState = !blinkState;
    }
}