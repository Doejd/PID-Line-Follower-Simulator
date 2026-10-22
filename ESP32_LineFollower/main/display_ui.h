#pragma once

#include <Arduino.h>

constexpr uint8_t OLED_SDA_GPIO = 16;
constexpr uint8_t OLED_SCL_GPIO = 15; // SDL
constexpr uint8_t BUTTON_1_GPIO = 17;
constexpr uint8_t BUTTON_2_GPIO = 18;
constexpr uint8_t BUTTON_3_GPIO = 8;

// Supply the pins from the verified board schematic when starting the UI.
// The three buttons are expected to connect their GPIO to GND when pressed.
struct DisplayUiConfig {
  uint8_t sdaPin;
  uint8_t sclPin;
  uint8_t button1Pin;
  uint8_t button2Pin;
  uint8_t button3Pin;
  uint8_t i2cAddress;
};

// Starts the OLED and configures the buttons. Returns false if the OLED
// cannot be initialized or the provided pins overlap.
bool beginDisplayUi(const DisplayUiConfig& config);

// Call often from the existing loop(). This function does not wait for input.
void serviceDisplayUi();

enum class DisplayUiAction : uint8_t {
  None,
  CalibrationRequested,
  StartRequested,
};

// Returns each confirmed menu action once. The main firmware passes it to the
// algorithm team's existing calibration/start routines.
DisplayUiAction consumeDisplayUiAction();

// Call when the algorithm team's calibration routine actually finishes.
// The display then freezes and shows the measured elapsed time.
void finishCalibrationDisplay();
