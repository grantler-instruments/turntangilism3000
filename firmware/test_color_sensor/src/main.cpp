#include <Wire.h>
#include "Adafruit_TCS34725.h"


struct HSV {
    float h; // Hue: 0-360 degrees
    float s; // Saturation: 0-1
    float v; // Value (brightness): 0-1
};
HSV rgbToHsv(int r, int g, int b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float maxVal = std::max({rf, gf, bf});
    float minVal = std::min({rf, gf, bf});
    float delta = maxVal - minVal;

    HSV hsv;
    
    // Calculate hue
    if (delta == 0) {
        hsv.h = 0; // Undefined hue
    } else if (maxVal == rf) {
        hsv.h = 60.0f * (fmod(((gf - bf) / delta), 6));
    } else if (maxVal == gf) {
        hsv.h = 60.0f * (((bf - rf) / delta) + 2);
    } else {
        hsv.h = 60.0f * (((rf - gf) / delta) + 4);
    }

    if (hsv.h < 0) {
        hsv.h += 360.0f;
    }

    // Calculate saturation
    hsv.s = (maxVal == 0) ? 0 : (delta / maxVal);

    // Calculate value
    hsv.v = maxVal;

    return hsv;
}


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(9600);
  //Serial.println("Color View Test!");

  if (tcs.begin()) {
    //Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }
  tcs.setInterrupt(false);  // turn on LED
}

void loop() {
  float red, green, blue;
  tcs.getRGB(&red, &green, &blue);
  auto hsv = rgbToHsv(red, green, blue);
  Serial.print("hue: ");
  Serial.println(hsv.h);
  delay(60); 
  // Serial.print("R:\t"); Serial.print(int(red)); 
  // Serial.print("\tG:\t"); Serial.print(int(green)); 
  // Serial.print("\tB:\t"); Serial.print(int(blue));
  // Serial.print("\n");
}
