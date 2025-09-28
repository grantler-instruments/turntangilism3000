#define NUMBER_OF_TRACKS 8
int trackPins[] = { 9, 10, 11, 12, 13, 14, 40, 39 };
int trackNotes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
#define ID 1

uint8_t broadcastAddress[6] = { 0x48, 0x27, 0xE2, 0x47, 0x3D, 0x74 };
uint8_t reaperMacAddress[6] = { 0x84, 0xF7, 0x03, 0xF2, 0x54, 0x62 };
