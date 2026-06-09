#include "./config.h"
#include <Wire.h>
#include "enomik_client.h"
#include <Adafruit_TCS34725.h> //https://github.com/adafruit/Adafruit_TCS34725
#include <AceButton.h> //https://github.com/bxparks/AceButton
#include <tampleDetector.h>
#include <movingAvg.h>  // https://github.com/JChristensen/movingAvg

using namespace ace_button;
float red, green, blue;

Adafruit_TCS34725 _tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

AceButton _button(PIN_BUTTON);

enomik::Client _client;

movingAvg _faderValue(16);

TampleDetector _tampleDetector;

void handleButtonEvent(AceButton *button, uint8_t eventType, uint8_t buttonState);

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // 12 bits = 0–4095

  delay(3000);
  Serial.print("setting up enomik client ... ");
  _client.begin();
  _client.addPeer(peerMacAddress);
  _client.addPeer(reaperMacAddress);
  Serial.println("done");


  Wire.setClock(400000);  // Set I2C to 400 kHz

  _faderValue.begin();
  _faderValue.reading(0);

  Serial.print("setting up button ... ");
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  _button.setEventHandler(handleButtonEvent);
  ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  // buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  Serial.println("done");

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
}

void loop() {
  _client.loop();
  _button.check();
  _tcs.getRGB(&red, &green, &blue);
  _tampleDetector.addEntryToHistory(red, green, blue);

  int rawValue = analogRead(PIN_FADER);
  _faderValue.reading(rawValue);
}

void handleButtonEvent(AceButton * /*button*/, uint8_t eventType,
                       uint8_t /*buttonState*/) {
  auto tample = _tampleDetector.getTample();
  auto hsv = _tampleDetector.getCurrentHistoryEntry();
  Serial.println("-");
  Serial.println(hsv.h);
  Serial.println(hsv.s);
  Serial.println(hsv.v);
  Serial.println("-");

  if (tample._name == "invalid") {
    Serial.println("invalid tample");
    return;
  }

  Serial.println(tample._name);
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

  //backup tamplifier
  // if (_faderValue.getAvg() < 64) {
  //   sampleBank = 4;
  // } else if (_faderValue.getAvg() < 2000) {
  //   sampleBank = 3;
  // } else if (_faderValue.getAvg() < 4000) {
  //   sampleBank = 2;
  // } else {
  //   sampleBank = 1;
  // }



  switch (eventType) {
    case AceButton::kEventDoubleClicked:
      {
        break;
      }
    case AceButton::kEventReleased:
      {
        Serial.println("AceButton::kEventPressed");
        _client.sendControlChange(tample._recordCC, 127, sampleBank);
        break;
      }
    case AceButton::kEventPressed:
      {
        Serial.println("AceButton::kEventReleased");
        _client.sendControlChange(tample._recordCC, 0, sampleBank);
        break;
      }
  }
}