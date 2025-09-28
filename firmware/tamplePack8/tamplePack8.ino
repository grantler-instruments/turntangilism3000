#include "./config.h"
#include <Wire.h>
#include <esp_now_midi.h>
#include <WiFi.h>
#include <MIDI.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_TCS34725.h>
#include <AceButton.h>
#include <TCA9548.h>
#include <tampleDetector.h>
#include <Parameter.h>
#include <movingAvg.h>  // https://github.com/JChristensen/movingAvg


using namespace ace_button;


void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState);

Adafruit_TCS34725 _tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
// Array to hold all 8 sensors
Adafruit_TCS34725 _tcsArray[NUMBER_OF_SLOTS] = {
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X),
  Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X)
};

float red, green, blue;
TampleDetector _tampleDetectors[NUMBER_OF_SLOTS];


AceButton _buttons[NUMBER_OF_SLOTS];


unsigned long _timestamp = 0;


esp_now_midi ESP_NOW_MIDI;

TCA9548 _mux(0x70);

// Parameter<int> _sampleBank;
movingAvg _faderValue(16);


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12 bits = 0–4095
  delay(1000);
  Wire.setClock(400000);  // Set I2C to 400 kHz
  Wire.begin();
  delay(1000);

  // _sampleBank.setup("sampleBank", 1, 1, 16);
  // _sampleBank.addListener([](int value) {
  //   Serial.println(" changed, new value: " + String(value));
  // });
  _faderValue.begin();
  _faderValue.reading(0);


  Serial.print("setting up buttons ... ");
  for (auto i = 0; i < NUMBER_OF_SLOTS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    _buttons[i].init(buttonPins[i], LOW, i);
  }
  ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  // buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  Serial.println("done");


  Serial.print("setting up i2c mux ... ");
  if (_mux.begin()) {
    Serial.println("done");
  } else {
    Serial.println("error");
  }

  Serial.println("setting up color sensors ... ");
  for (int i = 0; i < 8; i++) {
    Serial.print(i);
    Serial.print(": ");
    _mux.selectChannel(i);
    if (!_tcsArray[i].begin()) {
      Serial.println("no TCS34725 found on channel ");
    } else {
      _tcsArray[i].setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);  // Fastest
      _tcsArray[i].setInterrupt(false);
      Serial.println("ok");
    }
    _mux.enableChannel(i);
  }
  Serial.println("done");

  Serial.print("setting up wifi and esp now to midi ... ");
  WiFi.mode(WIFI_STA);
  ESP_NOW_MIDI.setup(peerMacAddress);
  ESP_NOW_MIDI.addPeer(reaperMacAddress);
  Serial.println("done");
}

void readAllSensors() {
  for (auto i = 0; i < NUMBER_OF_SLOTS; i++) {
    _buttons[i].check();
  }

  for (int i = 0; i < NUMBER_OF_SLOTS; i++) {
    auto buttonState = _buttons[i].getLastButtonState();
    // if (buttonState) {
      _mux.selectChannel(i);  // Activate only one sensor channel
      delay(1);               // Give time for I2C switching stability if needed
      _tcsArray[i].getRGB(&red, &green, &blue);
      _tampleDetectors[i].addEntryToHistory(red, green, blue);
    // }
  }


  auto oldSampleBank = 0;
  if (_faderValue.getAvg() < 64) {
    oldSampleBank = 1;
  } else if (_faderValue.getAvg() < 2000) {
    oldSampleBank = 2;
  } else if (_faderValue.getAvg() < 4000) {
    oldSampleBank = 3;
  } else {
    oldSampleBank = 4;
  }

  int rawValue = analogRead(PIN_FADER);
  _faderValue.reading(rawValue);
  auto sampleBank = 0;
  if (_faderValue.getAvg() < 64) {
    sampleBank = 1;
  } else if (_faderValue.getAvg() < 2000) {
    sampleBank = 2;
  } else if (_faderValue.getAvg() < 4000) {
    sampleBank = 3;
  } else {
    sampleBank = 4;
  }

  if (oldSampleBank != sampleBank) {
    ESP_NOW_MIDI.sendProgramChange(sampleBank, ID);
  }
}

void loop() {
  auto timestamp = millis();
  readAllSensors();
}

void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState) {
  uint8_t id = button->getId();
  Serial.println(id);
  if (id > NUMBER_OF_SLOTS) {
    return;
  }
  auto tample = _tampleDetectors[id].getTample();
  if (tample._name == "invalid") {
    Serial.println("invalid tample");
    _tampleDetectors[id].printCurrentHistoryEntry();
    return;
  }
  _tampleDetectors[id].printToken();


  switch (eventType) {
    case AceButton::kEventPressed:
      ESP_NOW_MIDI.sendControlChange(NOTE + id, tample._note, ID);
      break;
    case AceButton::kEventReleased:
      break;
  }
}