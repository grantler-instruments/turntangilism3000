#include <Arduino.h>
#include "./config.h"
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include "esp_now_midi.h"
#include "Adafruit_TCS34725.h"

struct HSV
{
  float h; // Hue: 0-360 degrees
  float s; // Saturation: 0-1
  float v; // Value (brightness): 0-1
};
HSV rgbToHsv(int r, int g, int b)
{
  float rf = r / 255.0f;
  float gf = g / 255.0f;
  float bf = b / 255.0f;

  float maxVal = std::max({rf, gf, bf});
  float minVal = std::min({rf, gf, bf});
  float delta = maxVal - minVal;

  HSV hsv;

  // Calculate hue
  if (delta == 0)
  {
    hsv.h = 0; // Undefined hue
  }
  else if (maxVal == rf)
  {
    hsv.h = 60.0f * (fmod(((gf - bf) / delta), 6));
  }
  else if (maxVal == gf)
  {
    hsv.h = 60.0f * (((bf - rf) / delta) + 2);
  }
  else
  {
    hsv.h = 60.0f * (((rf - gf) / delta) + 4);
  }

  if (hsv.h < 0)
  {
    hsv.h += 360.0f;
  }

  // Calculate saturation
  hsv.s = (maxVal == 0) ? 0 : (delta / maxVal);

  // Calculate value
  hsv.v = maxVal;

  return hsv;
}

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
esp_now_midi ESP_NOW_MIDI;
Token *midiHueArray = getTokens();
Token _token = Token(-1, -1, -1, -1);
unsigned long _timestamp = 0;

void customOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  // Serial.print("Custom Callback - Status: ");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failure");
}

void setup()
{
  Serial.begin(115200);
  Wire.setClock(400000);

  pinMode(DISTANCE_SENSOR_PIN, INPUT);

  if (!tcs.begin())
  {
    Serial.println("No TCS34725 found ... check your connections");
    while (1)
      ; // halt!
  }
  tcs.setInterrupt(false); // turn on LED

  WiFi.mode(WIFI_STA);
  ESP_NOW_MIDI.setup(broadcastAddress, customOnDataSent);
}

void loop()
{
  auto isToken = digitalRead(DISTANCE_SENSOR_PIN);
  if (isToken)
  {
    Serial.println(isToken);
    float red, green, blue;
    tcs.getRGB(&red, &green, &blue);

    auto hue = rgbToHsv(red, green, blue).h;
    auto token = getToken(hue);
    Serial.println(hue);

    // if(token.playCC != _token.playCC){
    //   _token = token;
    //   esp_err_t result = ESP_NOW_MIDI.sendControlChange(token.playCC, 127, ID);
    //   Serial.println(token.playCC);
    // }
  }
}
