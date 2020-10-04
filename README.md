# NeoPixel_UDP_Server

A UDP server which provides an interface to the NeoPixelBus based on [audio-reactive-led-strip](https://github.com/scottlawsonbc/audio-reactive-led-strip).

# Setup and Configuration

- Set `const char* ssid` to your router&#39;s SSID
- Set `const char* password` to your router&#39;s password
- Set `IPAddress gateway` to match your router&#39;s gateway
- Set `IPAddress ip` to the IP address that you would like your ESP8266 to use (your choice)
- Set `#define NUM_LEDS` to the number of LEDs in your LED strip
- Set `#define PIXEL_PIN` to the Data Pin on your ESP32


