#include <cstdint>
#include <cstddef>
#include <SPI.h>
#include <Arduino.h>

//Not sure on the pin numbers
const int SPI_MOSI_PIN = 11; //  DOUT (Pin 19)
const int SPI_SCK_PIN  = 12; //  CLK (Pin 20)
const int SPI_MISO_PIN = 13; //  DIN (Pin 21)

const int CS_CHIP1 = 4;      // CS_1 (Pin 4) -> Handles Sensors 1-8
const int CS_CHIP2 = 5;      // CS_2 (Pin 5) -> Handles Sensors 9-16

uint16_t sensorValues[16];


void sensorsSetup(){
    pinMode(CS_CHIP1, OUTPUT);
    pinMode(CS_CHIP2, OUTPUT);

    //keep the chips idle initially
    digitalWrite(CS_CHIP1, HIGH);
    digitalWrite(CS_CHIP2, HIGH);
    
    // Initialize custom SPI bus mapping to ESP32-S3 pins
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
}

uint16_t readADC(int csPin, int channel) {
    // Set SPI settings: 1MHz clock speed, MSB First, SPI Mode 0
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    // Open communication
    digitalWrite(csPin, LOW);

    // Build configuration command bit packet for MCP3208
    // Format requires: Start Bit (1) + Single-Ended Bit (1) + D2 Channel Bit
    uint8_t commandHigh = 0b00000110 | ((channel & 0x04) >> 2);
    // Contains D1 and D0 Channel Bits shifted left into position
    uint8_t commandLow  = (channel & 0x03) << 6;

    // Execute continuous byte transfer sequence
    SPI.transfer(commandHigh);                    // Transmit configuration byte
    uint8_t highByte = SPI.transfer(commandLow);      // Send channel select data, catch top bits
    uint8_t lowByte  = SPI.transfer(0);            // Send empty data byte, catch remaining bottom bits

    // Deselect the chip
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    // Returns 12-bit number: 0 (White reflection) up to 4095 (Black line)
    return ((static_cast<int16_t>(highByte) & 0x0F) << 8) | static_cast<int16_t>(lowByte);
}

void readSensorValues(){
    for (int channel = 0; channel < 8; channel++) {
      sensorValues[channel] = readADC(CS_CHIP1, channel);
    }
    for (int channel = 0; channel < 8; channel++) {
      sensorValues[channel + 8] = readADC(CS_CHIP2, channel);
    }
}