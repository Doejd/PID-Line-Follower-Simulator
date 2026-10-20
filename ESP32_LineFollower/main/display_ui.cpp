#include "display_ui.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "telemetry_api.h"

namespace {

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;
constexpr uint32_t SENSOR_REFRESH_MS = 500;
constexpr uint32_t CALIBRATION_REFRESH_MS = 250;
constexpr uint32_t MESSAGE_DURATION_MS = 1200;

constexpr char MENU_CALIBRATE[] = "Calibrate 360";
constexpr char MENU_SENSORS[] = "Sensors";
constexpr char MENU_START[] = "Start robot";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

enum class Screen : uint8_t {
  Menu,
  Sensors,
  ConfirmCalibration,
  CalibrationRunning,
  CalibrationResult,
  ConfirmStart,
  Message,
};

struct ButtonState {
  uint8_t pin = 0;
  bool lastReading = HIGH;
  bool stableState = HIGH;
  uint32_t changedAtMs = 0;
};

enum class MessageKind : uint8_t { CalibrationQueued, StartQueued };

ButtonState buttons[3];
DisplayUiConfig uiConfig{};
bool uiReady = false;
uint8_t menuIndex = 0;
Screen currentScreen = Screen::Menu;
DisplayUiAction pendingAction = DisplayUiAction::None;
MessageKind currentMessage = MessageKind::CalibrationQueued;
uint32_t messageUntilMs = 0;
uint32_t lastSensorDrawMs = 0;
uint32_t calibrationStartedAtMs = 0;
uint32_t calibrationElapsedMs = 0;
uint32_t lastCalibrationDrawMs = 0;

const char* menuItem(uint8_t index) {
  switch (index) {
    case 0:
      return MENU_CALIBRATE;
    case 1:
      return MENU_SENSORS;
    default:
      return MENU_START;
  }
}

bool validPins(const DisplayUiConfig& config) {
  const uint8_t pins[] = {config.sdaPin, config.sclPin, config.button1Pin,
                          config.button2Pin, config.button3Pin};
  for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
    for (size_t j = i + 1; j < sizeof(pins) / sizeof(pins[0]); j++) {
      if (pins[i] == pins[j]) {
        return false;
      }
    }
  }
  return config.i2cAddress >= 0x08 && config.i2cAddress <= 0x77;
}

bool pressed(ButtonState& button, uint32_t now) {
  const bool reading = digitalRead(button.pin);
  if (reading != button.lastReading) {
    button.lastReading = reading;
    button.changedAtMs = now;
  }

  if ((now - button.changedAtMs) >= BUTTON_DEBOUNCE_MS &&
      button.stableState != button.lastReading) {
    button.stableState = button.lastReading;
    return button.stableState == LOW;
  }
  return false;
}

void drawHeader(const char* title) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("HERMES / ");
  display.println(title);
  display.drawLine(0, 10, SCREEN_WIDTH - 1, 10, SSD1306_WHITE);
}

void drawMenu() {
  drawHeader("MENU");
  for (uint8_t i = 0; i < 3; i++) {
    const int y = 16 + i * 14;
    if (i == menuIndex) {
      display.fillRect(0, y, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(4, y + 2);
    display.print(i == menuIndex ? "> " : "  ");
    display.println(menuItem(i));
  }
  display.setTextColor(SSD1306_WHITE);
}

void drawSensorGraph() {
  drawHeader("SENSORS 1-16");
  if (!telemetryData.hasSensors) {
    display.setCursor(0, 28);
    display.println("Waiting for sensor data");
    display.setCursor(0, 46);
    display.println("B2: back to menu");
    return;
  }

  uint16_t peak = 0;
  for (int i = 0; i < 16; i++) {
    if (telemetryData.rawSensors[i] > peak) {
      peak = telemetryData.rawSensors[i];
    }
  }

  constexpr int graphTop = 18;
  constexpr int graphBottom = 54;
  constexpr int barWidth = 5;
  constexpr int barStep = 8;
  for (int i = 0; i < 16; i++) {
    const int barHeight = peak == 0
                              ? 0
                              : (static_cast<uint32_t>(telemetryData.rawSensors[i]) *
                                 (graphBottom - graphTop)) /
                                    peak;
    const int x = i * barStep;
    display.drawRect(x, graphTop, barWidth, graphBottom - graphTop, SSD1306_WHITE);
    if (barHeight > 0) {
      display.fillRect(x + 1, graphBottom - barHeight, barWidth - 2, barHeight,
                       SSD1306_WHITE);
    }
  }
  display.setCursor(0, 56);
  display.print("B2: back");
  display.setCursor(70, 56);
  display.print("#");
  display.print(telemetryData.sensorSequence);
}

void drawConfirmation(const char* title, const char* firstLine,
                      const char* secondLine) {
  drawHeader(title);
  display.setCursor(0, 22);
  display.println(firstLine);
  display.println(secondLine);
  display.setCursor(0, 43);
  display.println("B2: confirm");
  display.setCursor(0, 55);
  display.println("B1/B3: back");
}

void drawMessage() {
  drawHeader("REQUEST");
  display.setCursor(0, 28);
  display.println(currentMessage == MessageKind::CalibrationQueued
                      ? "Calibration queued"
                      : "Start request queued");
  display.setCursor(0, 46);
  display.println("Firmware must handle it");
}

void drawCalibrationRunning(uint32_t now) {
  drawHeader("CALIBRATING");
  const uint32_t elapsedMs = now - calibrationStartedAtMs;
  display.setCursor(0, 28);
  display.print("Elapsed: ");
  display.print(elapsedMs / 1000);
  display.print('.');
  display.print((elapsedMs % 1000) / 100);
  display.println(" s");
  display.setCursor(0, 48);
  display.println("Waiting for completion");
}

void drawCalibrationResult() {
  drawHeader("CALIBRATION DONE");
  display.setCursor(0, 26);
  display.print("Time: ");
  display.print(calibrationElapsedMs / 1000);
  display.print('.');
  display.print((calibrationElapsedMs % 1000) / 100);
  display.println(" s");
  display.setCursor(0, 46);
  display.println("B1: repeat  B2: menu");
}

void drawCurrentScreen(uint32_t now) {
  if (currentScreen == Screen::Message) {
    if (static_cast<int32_t>(messageUntilMs - now) > 0) {
      drawMessage();
    } else {
      currentScreen = Screen::Menu;
      drawMenu();
    }
  } else if (currentScreen == Screen::Menu) {
    drawMenu();
  } else if (currentScreen == Screen::Sensors) {
    drawSensorGraph();
    lastSensorDrawMs = now;
  } else if (currentScreen == Screen::ConfirmCalibration) {
    drawConfirmation("CALIBRATE", "Rotate 360 degrees?",
                     "Keep the area clear.");
  } else if (currentScreen == Screen::CalibrationRunning) {
    drawCalibrationRunning(now);
    lastCalibrationDrawMs = now;
  } else if (currentScreen == Screen::CalibrationResult) {
    drawCalibrationResult();
  } else {
    drawConfirmation("START ROBOT", "Start line following?",
                     "Confirm to begin.");
  }
  display.display();
}

void selectMenuItem(uint32_t now) {
  if (menuIndex == 0) {
    currentScreen = Screen::ConfirmCalibration;
  } else if (menuIndex == 1) {
    currentScreen = Screen::Sensors;
  } else {
    currentScreen = Screen::ConfirmStart;
  }
  drawCurrentScreen(now);
}

void startCalibrationRequest(uint32_t now) {
  pendingAction = DisplayUiAction::CalibrationRequested;
  calibrationStartedAtMs = now;
  calibrationElapsedMs = 0;
  currentScreen = Screen::CalibrationRunning;
  drawCurrentScreen(now);
}

void queueAction(DisplayUiAction action, MessageKind message, uint32_t now) {
  pendingAction = action;
  currentMessage = message;
  currentScreen = Screen::Message;
  messageUntilMs = now + MESSAGE_DURATION_MS;
  drawCurrentScreen(now);
}

}  // namespace

bool beginDisplayUi(const DisplayUiConfig& config) {
  if (!validPins(config)) {
    return false;
  }

  uiConfig = config;
  buttons[0].pin = uiConfig.button1Pin;
  buttons[1].pin = uiConfig.button2Pin;
  buttons[2].pin = uiConfig.button3Pin;

  Wire.begin(uiConfig.sdaPin, uiConfig.sclPin);
  if (!display.begin(SSD1306_SWITCHCAPVCC, uiConfig.i2cAddress)) {
    return false;
  }

  const uint32_t now = millis();
  for (ButtonState& button : buttons) {
    pinMode(button.pin, INPUT_PULLUP);
    button.lastReading = digitalRead(button.pin);
    button.stableState = button.lastReading;
    button.changedAtMs = now;
  }

  uiReady = true;
  drawCurrentScreen(now);
  return true;
}

void serviceDisplayUi() {
  if (!uiReady) {
    return;
  }

  const uint32_t now = millis();
  if (currentScreen == Screen::Message &&
      static_cast<int32_t>(now - messageUntilMs) >= 0) {
    currentScreen = Screen::Menu;
    drawCurrentScreen(now);
  }
  const bool button1 = pressed(buttons[0], now);
  const bool button2 = pressed(buttons[1], now);
  const bool button3 = pressed(buttons[2], now);

  if (currentScreen == Screen::Menu) {
    if (button1) {
      menuIndex = (menuIndex + 1) % 3;
      drawCurrentScreen(now);
    } else if (button3) {
      menuIndex = (menuIndex + 2) % 3;
      drawCurrentScreen(now);
    } else if (button2) {
      selectMenuItem(now);
    }
  } else if (currentScreen == Screen::Sensors) {
    if (button2 || button1 || button3) {
      currentScreen = Screen::Menu;
      drawCurrentScreen(now);
    } else if ((now - lastSensorDrawMs) >= SENSOR_REFRESH_MS) {
      drawCurrentScreen(now);
    }
  } else if (currentScreen == Screen::ConfirmCalibration) {
    if (button2) {
      startCalibrationRequest(now);
    } else if (button1 || button3) {
      currentScreen = Screen::Menu;
      drawCurrentScreen(now);
    }
  } else if (currentScreen == Screen::ConfirmStart) {
    if (button2) {
      queueAction(DisplayUiAction::StartRequested, MessageKind::StartQueued,
                  now);
    } else if (button1 || button3) {
      currentScreen = Screen::Menu;
      drawCurrentScreen(now);
    }
  } else if (currentScreen == Screen::CalibrationRunning) {
    if ((now - lastCalibrationDrawMs) >= CALIBRATION_REFRESH_MS) {
      drawCurrentScreen(now);
    }
  } else if (currentScreen == Screen::CalibrationResult) {
    if (button1) {
      currentScreen = Screen::ConfirmCalibration;
      drawCurrentScreen(now);
    } else if (button2 || button3) {
      currentScreen = Screen::Menu;
      drawCurrentScreen(now);
    }
  } else if (currentScreen == Screen::Message &&
             (button1 || button2 || button3)) {
    currentScreen = Screen::Menu;
    drawCurrentScreen(now);
  }
}

DisplayUiAction consumeDisplayUiAction() {
  const DisplayUiAction action = pendingAction;
  pendingAction = DisplayUiAction::None;
  return action;
}

void finishCalibrationDisplay() {
  if (!uiReady || currentScreen != Screen::CalibrationRunning) {
    return;
  }

  const uint32_t now = millis();
  calibrationElapsedMs = now - calibrationStartedAtMs;
  currentScreen = Screen::CalibrationResult;
  drawCurrentScreen(now);
}

