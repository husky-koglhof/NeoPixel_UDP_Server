/*
*This works for ESP32 and uses the NeoPixelBus library instead of the one bundle
*
*/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NeoPixelBrightnessBus.h>
#include <config.h>

char packetBuffer[BUFFER_LEN];

uint8_t N = 0;

WiFiUDP port;

void setup() {
    Serial.begin(115200);
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, password);
    Serial.println("");
    // Connect to wifi and print the IP address over serial
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    port.begin(localPort);

    ledstrip.Begin();//Begin output
    ledstrip.Show();//Clear the strip for use
}

#if PRINT_FPS
    uint16_t fpsCounter = 0;
    uint32_t secondTimer = 0;
#endif

void loop() {
    // Read data over socket
    int packetSize = port.parsePacket();
    // If packets have been received, interpret the command
    if (packetSize) {
        #if PRINT_FPS
          Serial.print("packetSize = ");
          Serial.println(packetSize);
        #endif
        int len = port.read(packetBuffer, BUFFER_LEN);
        #if PRINT_FPS
          Serial.print("brightness: ");
          Serial.println((uint8_t)packetBuffer[0]);
        #endif

        ledstrip.SetBrightness((uint8_t)packetBuffer[0]);

        // First Element is the brightness of the actual packet
        for(int i = 1; i < len; i+=4) {
            packetBuffer[len] = 0;
            N = packetBuffer[i];
            RgbColor pixel((uint8_t)packetBuffer[i+1], (uint8_t)packetBuffer[i+2], (uint8_t)packetBuffer[i+3]);//color
            #if PRINT_FPS
              Serial.print("LED: ");Serial.println(N);
              Serial.print("R: ");Serial.println((uint8_t)packetBuffer[i+1]);
              Serial.print("G: ");Serial.println((uint8_t)packetBuffer[i+2]);
              Serial.print("B: ");Serial.println((uint8_t)packetBuffer[i+3]);
            #endif
            ledstrip.SetPixelColor(N, pixel);//N is the pixel number
        } 
        ledstrip.Show();
        #if PRINT_FPS
            fpsCounter++;
            Serial.print("/");//Monitors connection(shows jumps/jitters in packets)
        #endif
    }
    #if PRINT_FPS
        if (millis() - secondTimer >= 1000U) {
            secondTimer = millis();
            Serial.printf("FPS: %d\n", fpsCounter);
            fpsCounter = 0;
        }   
    #endif
}