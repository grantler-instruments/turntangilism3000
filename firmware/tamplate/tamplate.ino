#include "./config.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_TCS34725.h>
#include <AceButton.h>
#include <Adafruit_NeoPixel.h>
#include "enomik_client.h"
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

unsigned long lastSendTime = 0;
unsigned long lastForcedSendTime = 0;

int lastSentPitchBend = INT_MIN;
int lastSentCC = -1;

static const int PITCH_BEND_THRESHOLD = 64;
static const int CC_THRESHOLD = 1;
static const unsigned long FORCE_SEND_MS = 500;

enomik::Client _client;

Adafruit_MPU6050 _mpu;
Adafruit_TCS34725 _tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

float yaw = 0;
float rotationSpeed = 0;
float gyroZoffset = 0;
unsigned long lastMicros = 0;
bool isCalibrating = true;

AceButton _buttons[NUMBER_OF_BUTTONS];
void handleEvent(AceButton*, uint8_t, uint8_t);

void resetMidiOutputs() {
  _client.sendPitchBend(-8192, ID);
  _client.sendControlChange(POSITON_CC, 0, ID);
  lastSentPitchBend = 0;
  lastSentCC = 0;
}

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
  Serial.print("setting up i2c ... ");
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);
  Serial.println("done");
  scanI2C();

  Serial.print("setting up color sensor ... ");
  if (!_tcs.begin()) {
    Serial.println("No TCS34725 found ... check your connections");
  }
  _tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);
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
  buttonConfig->setLongPressDelay(1000);
  buttonConfig->setDebounceDelay(20);
  buttonConfig->setClickDelay(200);
  buttonConfig->setDoubleClickDelay(400);

  _leds.begin();
  _leds.show();
  _leds.setBrightness(50);

  Serial.print("setting up enomik client ... ");
  _client.begin();
  _client.addPeer(peerMacAddress);
  _client.addPeer(reaperMacAddress);
  Serial.println("done");

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
  }
  _tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);
  _tcs.setInterrupt(false);
  Serial.println("done");

  lastMicros = micros();
  lastSendTime = millis();
  lastForcedSendTime = millis();
}

void loop() {
  _client.loop();
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

  unsigned long now = micros();
  float deltaTime = (now - lastMicros) / 1000000.0;
  lastMicros = now;

  float rawGyroZ = g.gyro.z - gyroZoffset;
  rotationSpeed = rawGyroZ * RAD_TO_DEG;
  yaw += rotationSpeed * deltaTime;
  yaw = fmod(yaw, 360);
  if (yaw < 0) yaw += 360;

  if (deltaTime > 0.1 || deltaTime < 0) {
    deltaTime = 0.05;
  }

  if (isCalibrating && millis() < 3000) {
    gyroZoffset = gyroZoffset * 0.99 + g.gyro.z * 0.01;
  } else if (isCalibrating) {
    isCalibrating = false;
    Serial.println("Calibration complete");
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastSendTime >= sendInterval) {
    lastSendTime = currentMillis;

    bool forceSend = (currentMillis - lastForcedSendTime >= FORCE_SEND_MS);

    if (_mode == Mode::SCRATCH) {
      int pitchBendValue = map((int)yaw, 0, 359, -8192, 8191);

      if (forceSend ||
          lastSentPitchBend == INT_MIN ||
          abs(pitchBendValue - lastSentPitchBend) >= PITCH_BEND_THRESHOLD) {
        _client.sendPitchBend(pitchBendValue, ID);
        lastSentPitchBend = pitchBendValue;
        lastForcedSendTime = currentMillis;
      }

    } else if (_mode == Mode::POSITION) {
      int controlChangeValue = map((int)yaw, 0, 359, 0, 127);

      if (forceSend ||
          lastSentCC < 0 ||
          abs(controlChangeValue - lastSentCC) >= CC_THRESHOLD) {
        _client.sendControlChange(POSITON_CC, controlChangeValue, ID);
        lastSentCC = controlChangeValue;
        lastForcedSendTime = currentMillis;
      }
    }
  }
}

void calibrateZaxis() {
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
            resetMidiOutputs();
            break;
          }
        case 4:
          {
            Serial.println("tample has been pressed");
            Serial.println(tample._name);
            _client.sendProgramChange(ID, ID);
            _client.sendControlChange(127, tample._note, ID);
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
        _client.sendControlChange(127, 127, ID);
        resetMidiOutputs();
      }
      break;
  }
}