#pragma once
#define ID 1
#define NUMBER_OF_VOICES 16
#define RECORD_BUTTON_PIN 4
#define DISTANCE_SENSOR_PIN 21

#define SDA_0 33
#define SCL_0 35
#define SDA_1 25
#define SCL_1 26

// cc:8d:a2:8b:85:1c
uint8_t broadcastAddress[6] = {0xCC, 0x8D, 0xA2, 0x8B, 0x85, 0x1C};

struct Token
{
    int playCC;   // MIDI CC number for playback (0-127)
    int recordCC; // MIDI CC number for recording (0-127)
    float hueMin; // Minimum hue (0.0 - 360.0)
    float hueMax; // Maximum hue (0.0 - 360.0)

    Token() {}

    // Constructor for initializing values
    Token(int pCC, int rCC, float minHue, float maxHue)
        : playCC(pCC), recordCC(rCC), hueMin(minHue), hueMax(maxHue) {}
};

inline Token *getTokens()
{
    static Token midiHueArray[16]; // Static to retain data after the function call
    midiHueArray[0] = Token(1, 65, 0.0f, 30.0f);
    midiHueArray[1] = Token(2, 66, 30.0f, 60.0f);
    midiHueArray[2] = Token(3, 67, 60.0f, 90.0f);
    midiHueArray[3] = Token(4, 68, 90.0f, 120.0f);
    midiHueArray[4] = Token(5, 69, 120.0f, 150.0f);
    midiHueArray[5] = Token(6, 70, 150.0f, 180.0f);
    midiHueArray[6] = Token(7, 71, 180.0f, 210.0f);
    midiHueArray[7] = Token(8, 72, 210.0f, 240.0f);
    midiHueArray[8] = Token(9, 73, 240.0f, 270.0f);
    midiHueArray[9] = Token(10, 74, 270.0f, 300.0f);
    midiHueArray[10] = Token(11, 75, 300.0f, 330.0f);
    midiHueArray[11] = Token(12, 76, 330.0f, 360.0f);
    midiHueArray[12] = Token(13, 77, 0.0f, 45.0f);
    midiHueArray[13] = Token(14, 78, 45.0f, 90.0f);
    midiHueArray[14] = Token(15, 79, 90.0f, 135.0f);
    midiHueArray[15] = Token(16, 80, 135.0f, 180.0f);

    return midiHueArray; // Return the array
}

inline Token getToken(float hue) {
    Token* midiHueArray = getTokens();

    // Loop through the array and check if the hue falls within the range
    for (int i = 0; i < 16; ++i) {
        if (hue >= midiHueArray[i].hueMin && hue < midiHueArray[i].hueMax) {
            return midiHueArray[i];  // Return the matching MidiHueRange
        }
    }

    // Return a default invalid range if no match is found (optional)
    return Token(-1, -1, -1.0f, -1.0f);  // Indicates no matching hue found
}