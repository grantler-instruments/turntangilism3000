#include "./config.h"
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_TCS34725.h>
#include <AceButton.h>
#include <Adafruit_NeoPixel.h>
#include "esp_now_midi.h"
#include <tampleDetector.h>

using namespace ace_button;

enum Mode {
  NONE,
  SCRATCH,
  POSITION
};



TampleDetector _tampleDetector;
Adafruit_NeoPixel _leds(NUMBER_OF_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Mode _mode = Mode::NONE;

// Timing variables for MIDI message sending
unsigned long lastSendTime = 0;


esp_now_midi ESP_NOW_MIDI;
// void customOnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
//   // Serial.print("Custom Callback - Status: ");
//   // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failure");
// }

Adafruit_MPU6050 _mpu;
Adafruit_TCS34725 _tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Tracking variables
float yaw = 0;            // Current angle (0-359°)
float rotationSpeed = 0;  // Current speed (°/sec)
float gyroZoffset = 0;    // Calibration offset
unsigned long lastMicros = 0;
bool isCalibrating = true;

AceButton _buttons[NUMBER_OF_BUTTONS];
void handleEvent(AceButton*, uint8_t, uint8_t);

void scanI2C() {
  Serial.println("Scanning I2C bus...");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(address, HEX);
    }
  }
  Serial.println("I2C scan complete");
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  // Wire.begin(PIN_SDA, PIN_SCL);
  Serial.print("setting up i2c ... ");
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);  // Set I2C to 400 kHz
  Serial.println("done");
  scanI2C();

  Serial.print("setting up color sensor ... ");
  if (!_tcs.begin()) {
    Serial.println("No TCS34725 found ... check your connections");
    // while (1)
    // {
    // }
  }
  _tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);  // Fastest
  _tcs.setInterrupt(false);

  for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    _buttons[i].init(buttonPins[i], LOW, i);
  }
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  buttonConfig->setLongPressDelay(1000);  // 1 second for long press
  buttonConfig->setDebounceDelay(20);
  buttonConfig->setClickDelay(200);
  buttonConfig->setDoubleClickDelay(400);



  _leds.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  _leds.show();   // Turn OFF all pixels ASAP
  _leds.setBrightness(50);

  WiFi.mode(WIFI_STA);
  ESP_NOW_MIDI.setup(peerMacAddress);
  ESP_NOW_MIDI.addPeer(reaperMacAddress);


  Serial.print("setting up mpu ... ");
  if (!_mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  } else {
    Serial.println("done");
  }

  _mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  Serial.println("Calibrating... keep sensor still for 3 seconds");
  calibrateZaxis();
  Serial.println("Ready - Rotate to measure");
  Serial.println("Angle(°)\tSpeed(°/s)");


  Serial.print("setting up color sensor ... ");
  if (!_tcs.begin()) {
    Serial.println("No TCS34725 found ... check your connections");
    // while (1)
    // {
    // }
  }
  _tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);  // Fastest
  _tcs.setInterrupt(false);
  Serial.println("done");


  lastMicros = micros();
  lastSendTime = millis();
}

void loop() {
  for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
    _buttons[i].check();
  }
  sensors_event_t a, g, temp;
  _mpu.getEvent(&a, &g, &temp);

  float red, green, blue;
  _tcs.getRGB(&red, &green, &blue);
  _tampleDetector.addEntryToHistory(red, green, blue);

  if (!isCalibrating && _mode == Mode::NONE) {
    return;
  }

  // Calculate time difference in seconds
  unsigned long now = micros();
  float deltaTime = (now - lastMicros) / 1000000.0;
  lastMicros = now;

  // Get raw rotation speed (in rad/s) and apply offset
  float rawGyroZ = g.gyro.z - gyroZoffset;

  // Convert from rad/s to deg/s
  rotationSpeed = rawGyroZ * RAD_TO_DEG;

  // Integrate to get angle
  yaw += rotationSpeed * deltaTime;

  // Convert angle to 0-359° range
  yaw = fmod(yaw, 360);
  if (yaw < 0) yaw += 360;

  // Handle micros() overflow
  if (deltaTime > 0.1 || deltaTime < 0) {
    // Unusually large time difference detected, likely an overflow
    deltaTime = 0.05;  // Use a reasonable default
  }

  // Only perform continuous calibration during first 3 seconds if still calibrating
  if (isCalibrating && millis() < 3000) {
    gyroZoffset = gyroZoffset * 0.99 + g.gyro.z * 0.01;
  } else if (isCalibrating) {
    isCalibrating = false;
    Serial.println("Calibration complete");
  }

  // Apply complementary filter for drift compensation if device is relatively still
  // if (abs(rotationSpeed) < 1.0) {
  //   // Apply small correction to reduce drift when not moving
  //   rotationSpeed = 0;
  // }

  // Check if it's time to send MIDI data
  unsigned long currentMillis = millis();
  if (currentMillis - lastSendTime >= sendInterval) {
    lastSendTime = currentMillis;
    auto pitchBendValue = (int)(map(yaw, 359, 0, 1, 16383));
    auto controlChangeValue = (int)(map(yaw, 359, 0, 0, 127));

    if (_mode == Mode::SCRATCH) {
      ESP_NOW_MIDI.sendPitchBend(pitchBendValue, ID);
    } else if (_mode == Mode::POSITION) {
      ESP_NOW_MIDI.sendControlChange(POSITON_CC, controlChangeValue, ID);
    }
  }
}

void calibrateZaxis() {
  // Average 200 readings for initial calibration
  float sum = 0;
  for (int i = 0; i < 200; i++) {
    sensors_event_t a, g, temp;
    _mpu.getEvent(&a, &g, &temp);
    sum += g.gyro.z;
    delay(10);
  }
  gyroZoffset = sum / 200;
  yaw = 0;
  rotationSpeed = 0;
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(AceButton::eventName(eventType));
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
  Serial.println(button->getId());

  auto id = button->getId();
  auto tample = _tampleDetector.getTample();

  auto color = _leds.Color(0, 0, 255);
  switch (eventType) {
    case AceButton::kEventPressed:
      switch (id) {
        case 0:
        case 1:
        case 2:
        case 3:
          {
            auto newMode = Mode::SCRATCH;
            if (_mode == Mode::NONE) {
              color = _leds.Color(255, 0, 255);
              newMode = Mode::SCRATCH;
            } else if (_mode == Mode::SCRATCH) {
              color = _leds.Color(255, 255, 255);
              newMode = Mode::POSITION;
            } else if (_mode == Mode::POSITION) {
              newMode = Mode::NONE;
              color = _leds.Color(0, 0, 0);
            }
            _leds.clear();
            _leds.setPixelColor(0, color);
            _leds.setPixelColor(1, color);
            _leds.setPixelColor(2, color);
            _leds.setPixelColor(3, color);
            _leds.show();
            _mode = newMode;
            break;
          }
        case 4:
          {
            Serial.println("tample has been pressed");
            Serial.println(tample._name);
            ESP_NOW_MIDI.sendProgramChange(ID, ID);
            ESP_NOW_MIDI.sendControlChange(127, tample._note, ID);
          }
      }

      break;
    case AceButton::kEventReleased:
      break;
    case AceButton::kEventLongPressed:
      if (id == 4) {
        Serial.println("long press");
        _mode = Mode::NONE;
        color = _leds.Color(0, 0, 0);

        _leds.clear();
        _leds.setPixelColor(0, color);
        _leds.setPixelColor(1, color);
        _leds.setPixelColor(2, color);
        _leds.setPixelColor(3, color);
        _leds.show();
        ESP_NOW_MIDI.sendControlChange(127, 127, ID);
      }
      break;
  }
}