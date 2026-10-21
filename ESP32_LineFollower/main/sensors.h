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


void sensorsSetup(){
    pinMode(CS_CHIP1, OUTPUT);
    pinMode(CS_CHIP2, OUTPUT);

    digitalWrite(CS_CHIP1, HIGH);
    digitalWrite(CS_CHIP2, HIGH);
    
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
}

uint16_t readADC(int csPin, int channel) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    digitalWrite(csPin, LOW);

    uint8_t commandHigh = 0b00000110 | (channel >> 2);
    uint8_t commandLow  = (channel & 0x03) << 6;

    SPI.transfer(commandHigh);
    uint8_t highByte = SPI.transfer(commandLow);
    uint8_t lowByte  = SPI.transfer(0);

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    return ((static_cast<int16_t>(highByte) & 0x0F) << 8) | static_cast<int16_t>(lowByte);
}

void readSensorValues(std::array<uint16_t, 16> &values){
    for (int channel = 0; channel < 8; channel++) {
		values[channel] = readADC(CS_CHIP1, channel);
    }

    for (int channel = 0; channel < 8; channel++) {
      values[channel + 8] = readADC(CS_CHIP2, channel);
    }
}