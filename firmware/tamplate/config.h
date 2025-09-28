#define ID 3
uint8_t peerMacAddress[6] = { 0x48, 0x27, 0xE2, 0x47, 0x3D, 0x74 };
uint8_t reaperMacAddress[6] = { 0x84, 0xF7, 0x03, 0xF2, 0x54, 0x62 };

#define PIN_SDA 35
#define PIN_SCL 37


#define MIDI_CHANNEL 11
#define POSITON_CC 119

const unsigned long sendInterval = 4;  // 10ms interval for sending MIDI data
float speedSensitivity = 360.0;         // Maximum rotation speed in degrees/second

#define NUMBER_OF_BUTTONS 5
int buttonPins[] = { 10, 11, 12, 13, 16 };
#define NUMBER_OF_PIXELS 4
#define PIN_NEOPIXEL 6