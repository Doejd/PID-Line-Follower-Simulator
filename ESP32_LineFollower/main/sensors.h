#include <cstdint>
#include <cstddef>
#include <SPI.h>
#include <Arduino.h>

const int SPI_MOSI_PIN = 11; //  DOUT (Pin 19)
const int SPI_SCK_PIN  = 12; //  CLK (Pin 20)
const int SPI_MISO_PIN = 13; //  DIN (Pin 21)

const int CS_CHIP1 = 4;      // CS_1 (Pin 4) -> Handles Sensors 1-8
const int CS_CHIP2 = 5;      // CS_2 (Pin 5) -> Handles Sensors 9-16

const int SAMPLES  = 16;      // Number of readings averaged per sensor

//sensor 1 -> right    sensor 16 -> left
// positive correction -> go right
const float SENSOR_POS[16] = {
  82,  71,  59,  46,  35,  25,  15,  5,
  -5, -15, -25, -35, -46, -59, -71, -82
};

// Sensor Data Arrays
uint16_t rawReadings[16];
int16_t  normalizedVal[16]; // Normalized 0..1000 scale
uint16_t minVal[16], maxVal[16];


// microchip documentation  https://ww1.microchip.com/downloads/en/DeviceDoc/21298E.pdf
uint16_t readADC(int csPin, int channel) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    //start bit and single-ended mode bit
    uint8_t packetStructure = 0b00000110;
    // select highest channel bit (D2) and put it in the lowest bit
    uint8_t highestChannelBit = ((channel & 0b100) >> 2); 
    // add together
    uint8_t commandHigh = packetStructure | highestChannelBit;
    // put the remaining bits (D1 and D0) into the next command
    uint8_t commandLow  = (channel & 0b011) << 6;

    SPI.transfer(commandHigh);
    // retrieve sensor data
    uint8_t highByte = SPI.transfer(commandLow); // 4 bit number
    uint8_t lowByte  = SPI.transfer(0);          // 8 bit number

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
    // put the sensor data together
    return ((static_cast<int16_t>(highByte) & 0b1111) << 8) | static_cast<int16_t>(lowByte);
}

// Read all 16 sensors with oversampling and calibration mapping
void readAllSensors() {
  for (int i = 0; i < 16; i++) {
    uint32_t sum = 0;
    for (int s = 0; s < SAMPLES; s++) {
      sum += (i < 8) ? readADC(CS_CHIP1, i) : readADC(CS_CHIP2, i - 8);
    }
    rawReadings[i] = sum / SAMPLES;
  }

  for (int i = 0; i < 16; i++) {
    int low = minVal[i], high = maxVal[i];

    if (high - low < 50) high = low + 50; // Prevent division by zero
    
    int value = (int)(rawReadings[i] - low) * 1000L / (high - low);
    normalizedVal[i] = constrain(value, 0, 1000);
  }
}

void calibrationStep() {
  for (int i = 0; i < 16; i++) {
      if (rawReadings[i] < minVal[i]) minVal[i] = rawReadings[i];
      if (rawReadings[i] > maxVal[i]) maxVal[i] = rawReadings[i];
  }
}

// Calculate line position in mm from center. Returns false if line is lost.
bool getLinePosition(float &positionOut) {
  float numerator = 0;
  float denominator = 0;
  
  for (int i = 0; i < 16; i++) {
    int value = normalizedVal[i] - 200; // Filter out weak background noise
    if (value > 0) {
      numerator += value * SENSOR_POS[i];
      denominator += value;
    }
  }
  
  if (denominator <= 0) return false; // Line not found
  
  positionOut = numerator / denominator;
  return true;
}

void sensorsSetup(){
  pinMode(CS_CHIP1, OUTPUT);
  pinMode(CS_CHIP2, OUTPUT);
  
  digitalWrite(CS_CHIP1, HIGH);
  digitalWrite(CS_CHIP2, HIGH);
  
  SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
  
  for (int i = 0; i < 16; i++) {
    minVal[i] = 4095; // min val is set to the maximum so the calibration overwrites it
    maxVal[i] = 0; // same here
  }
}