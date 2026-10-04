#include <cstdint>
#include <cstddef>
#include <SPI.h>

// --- SPI PIN CONFIGURATION (FROM YOUR SCHEMATIC) ---
const int SPI_MISO_PIN = 10; // Labeled DOUT (Pin 19)
const int SPI_SCK_PIN  = 11; // Labeled CLK (Pin 20)
const int SPI_MOSI_PIN = 12; // Labeled DIN (Pin 21)

const int CS_CHIP1 = 4;      // Labeled CS_1 (Pin 4) -> Handles Sensors 1-8
const int CS_CHIP2 = 5;      // Labeled CS_2 (Pin 5) -> Handles Sensors 9-16

// Global storage array for all 16 line sensors
uint16_t sensorValues[16];

void setup() {
    Serial.begin(115200);

    // Configure Chip Select lines as digital outputs
    pinMode(CS_CHIP1, OUTPUT);
    pinMode(CS_CHIP2, OUTPUT);

    // Set CS pins HIGH to keep both chips idle initially
    digitalWrite(CS_CHIP1, HIGH);
    digitalWrite(CS_CHIP2, HIGH);

    // Initialize custom SPI bus mapping to your explicit ESP32-S3 pins
    // Format: SPI.begin(SCK, MISO, MOSI, SS);
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
}


// --- HELPER FUNCTION: LOW-LEVEL MCP3208 SPI READ ---
uint16_t readADC(int csPin, int channel) {
    // Set SPI settings: 1MHz clock speed, MSB First, SPI Mode 0
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    // Select the chip to open communication
    digitalWrite(csPin, LOW);

    // Build configuration command bit packet for MCP3208
    // Format requires: Start Bit (1) + Single-Ended Bit (1) + D2 Channel Bit
    std::byte commandHigh = 0b00000110 | ((channel & 0x04) >> 2);
    // Contains D1 and D0 Channel Bits shifted left into position
    std::byte commandLow  = (channel & 0x03) << 6;

    // Execute continuous byte transfer sequence
    SPI.transfer(commandHigh);                    // Transmit configuration byte
    std::byte highByte = SPI.transfer(commandLow);      // Send channel select data, catch top bits
    std::byte lowByte  = SPI.transfer(0x00);            // Send empty data byte, catch remaining bottom bits

    // Deselect the chip to free up the shared SPI bus lines
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    // Strip away irrelevant formatting flags and stitch bytes into a 12-bit number
    // Returns clean tracking data value ranging from 0 (White reflection) up to 4095 (Black line)
    return ((static_cast<int16_t>(highByte) & 0x0F) << 8) | static_cast<int16_t>(lowByte);
}

void loop() {
    // 1. Read Channels 0-7 from the first MCP3208 Chip (Sensors 1 to 8)
    for (int channel = 0; channel < 8; channel++) {
      sensorValues[channel] = readADC(CS_CHIP1, channel);
    }

    // 2. Read Channels 0-7 from the second MCP3208 Chip (Sensors 9 to 16)
    for (int channel = 0; channel < 8; channel++) {
      sensorValues[channel + 8] = readADC(CS_CHIP2, channel);
    }

    // 3. Print the data array to the Serial Monitor for calibration tracking
    for (int i = 0; i < 16; i++) {
      Serial.print(sensorValues[i]);
      Serial.print("\t"); // Tab-separated layout for easy reading
    }
    Serial.println();

    delay(20); // Quick iterative polling loop cycle time
}
