#ifndef CONF_H
#define CONF_H

// Set to the number of LEDs in your LED strip
#define NUM_LEDS 31

// Toggles FPS output (1 = print FPS over serial, 0 = disable output)
#define PRINT_FPS 0

// Maximum number of packets to hold in the buffer. Don't change this.
#define BUFFER_LEN 1024

// NeoPixelBus settings
// make sure to set this to the correct pin
#define PIXEL_PIN 32

// Wifi and socket settings
const char* ssid     = "";
const char* password = "";
unsigned int localPort = 7777;

// Network information
IPAddress ip(192, 168, 100, 250);
// Set gateway to your router's gateway
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> ledstrip(NUM_LEDS, PIXEL_PIN);
#endif