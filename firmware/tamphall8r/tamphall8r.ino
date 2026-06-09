#include "config.h"
#include "enomik_client.h"
#include <AceButton.h>
using namespace ace_button;
AceButton _buttons[NUMBER_OF_TRACKS];


enomik::Client _client;

void onNoteOn(byte channel, byte note, byte velocity) {
  Serial.printf("Note On - Channel: %d, Note: %d, Velocity: %d\n", channel, note, velocity);
}

void onNoteOff(byte channel, byte note, byte velocity) {
  Serial.printf("Note Off - Channel: %d, Note: %d, Velocity: %d\n", channel, note, velocity);
}

void onControlChange(byte channel, byte control, byte value) {
  Serial.printf("Control Change - Channel: %d, Control: %d, Value: %d\n", channel, control, value);
}

void onProgramChange(byte channel, byte program) {
  Serial.printf("Program Change - Channel: %d, Program: %d\n", channel, program);
}

void onPitchBend(byte channel, int value) {
  Serial.printf("Pitch Bend - Channel: %d, Value: %d\n", channel, value);
}
void onAfterTouch(byte channel, byte value) {
  Serial.printf("After Touch - Channel: %d, Value: %d\n", channel, value);
}
void onPolyAfterTouch(byte channel, byte note, byte value) {
  Serial.printf("Poly After Touch - Channel: %d, note: %d, Value: %d\n", channel, note, value);
}
void onStart() {
  Serial.printf("Start");
}
void onStop() {
  Serial.printf("Stop");
}
void onContinue() {
  Serial.printf("Continue");
}
void onClock() {
  Serial.printf("Clock");
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < NUMBER_OF_TRACKS; i++) {
    pinMode(trackPins[i], INPUT_PULLUP);
    _buttons[i].init(trackPins[i], HIGH, i);
  }
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  // buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  // buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  Serial.print("setting up enomik client ... ");
  _client.begin();
  _client.addPeer(broadcastAddress);
  _client.addPeer(reaperMacAddress);
  Serial.println("done");

  // all of these midi handlers are optional, depends on the usecase, very often you just wanna send data and not receive
  // e.g. this can be used for calibration, or maybe you wanna connect an amp via i2s and render some sound
  _client.setHandleNoteOn(onNoteOn);
  _client.setHandleNoteOff(onNoteOff);
  _client.setHandleControlChange(onControlChange);
  _client.setHandleProgramChange(onProgramChange);
  _client.setHandlePitchBend(onPitchBend);
  _client.setHandleAfterTouchChannel(onAfterTouch);
  _client.setHandleAfterTouchPoly(onPolyAfterTouch);
  _client.setHandleStart(onStart);
  _client.setHandleStop(onStop);
  _client.setHandleContinue(onContinue);
  _client.setHandleClock(onClock);
}

void loop() {
  _client.loop();
  for (uint8_t i = 0; i < NUMBER_OF_TRACKS; i++) {
    _buttons[i].check();
  }


  // esp_err_t result = ESP_NOW_MIDI.sendNoteOn(60, 127, 1);

  // if (result != ESP_OK) {
  //   Serial.println("Error sending the data");
  // }
  // delay(100);
  // result = ESP_NOW_MIDI.sendNoteOff(60, 0, 1);
  // delay(100);
  // result = ESP_NOW_MIDI.sendControlChange(1, 127, 1);
  // delay(100);
  // result = ESP_NOW_MIDI.sendControlChange(1, 0, 1);
  // delay(100);
  // result = ESP_NOW_MIDI.sendPitchBend(16383, 1);
  // delay(100);
  // result = ESP_NOW_MIDI.sendPitchBend(0, 1);

  // delay(2000);
}

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  auto index = button->getId();


  switch (eventType) {
    case AceButton::kEventPressed:
      {
        Serial.print("pressed: ");
        Serial.println(index);
        _client.sendNoteOn(trackNotes[index], 127, MIDI_CHANNEL);
        break;
      }
    case AceButton::kEventReleased:
      {
        Serial.print("released: ");
        Serial.println(index);
        _client.sendNoteOff(trackNotes[index], 0, MIDI_CHANNEL);
        break;
      }
  }
}