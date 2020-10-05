/*
*This works for ESP32 and uses the NeoPixelBus library instead of the one bundle
*
*/
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NeoPixelBrightnessBus.h>

#include <config.h>
#include "ACS712.h"
#include <PubSubClient.h>

char packetBuffer[BUFFER_LEN];

uint8_t N = 0;

WiFiUDP port;

// ACS712 5A  uses 185 mV per A
// ACS712 20A uses 100 mV per A
// ACS712 30A uses  66 mV per A
ACS712 ACS(ACS_PIN, 5.0, 4095, 100);

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
const char* mqtt_server = "192.168.100.200";
const char* mqtt_topic = "LED_POWER";
int lastmA = 0;

// Set MQTT reconnect to core 0 task handler
TaskHandle_t Task1; // core 0 task handler

void reconnect() {
  // Loop until we're reconnected
  // while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      // delay(5000);
    }
  // }
}

void MQTT_TASK(void * pvParameters){
    for(;;){
    if (WiFi.status() != WL_CONNECTED && !client.connected()) {
        reconnect();
    }
    vTaskDelay(1000);
    }
}

void showBatteryPercentage() {
    // Current there's a 4000mAh LiPo connected
    int mA = ACS.mA_DC();

    int r, g, b = 0;
    int len = 0;
    // Shows Percentage of the Battery
    if (mA >= 3600) { // 90%
        // Color green on all rings
        r = 0;
        g = 255;
        b = 0;
        len = NUM_LEDS;                     // 60
    } else if (mA >= 3000) { // 75%
        // Color green on 4 rings
        r = 0;
        g = 255;
        b = 0;
        len = NUM_LEDS * 0.80;              // 48
    } else if (mA > 2000) { // 50%
        // Color green on 3 rings
        r = 0;
        g = 255;
        b = 0;
        len = NUM_LEDS / 2;                 // 30
    } else if (mA > 1000) { // 25%
        // Color yellow on 2 rings
        r = 255;
        g = 255;
        b = 0;
        len = NUM_LEDS / 2.5;               // 24
    } else {
        // Color red on 1 ring (blink 5 secs)
        r = 255;
        g = 0;
        b = 0;
        len = NUM_LEDS / 6;                 // 12
    }
    
    ledstrip.SetBrightness(10);
    for(int i = 0; i < len; i++) {
        RgbColor pixel(r, g, b); //color
        ledstrip.SetPixelColor(i, pixel); //N is the pixel number
    }

    ledstrip.Show();
}

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

    showBatteryPercentage();

    ACS.autoMidPoint();

    // client.setServer(mqtt_server, 1883);
    // xTaskCreatePinnedToCore(MQTT_TASK,"Task1",10000,NULL,1,&Task1,0);
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
              Serial.print("LED: ");Serial.print(N);
              Serial.print(", R: ");Serial.print((uint8_t)packetBuffer[i+1]);
              Serial.print(", G: ");Serial.print((uint8_t)packetBuffer[i+2]);
              Serial.print(", B: ");Serial.println((uint8_t)packetBuffer[i+3]);
            #endif
            ledstrip.SetPixelColor(N, pixel);//N is the pixel number
        } 
        ledstrip.Show();
        #if PRINT_FPS
            fpsCounter++;
            Serial.print("/");//Monitors connection(shows jumps/jitters in packets)
        #endif
    }

    int mA = ACS.mA_DC();

    #if PRINT_FPS
        if (millis() - secondTimer >= 1000U) {
            secondTimer = millis();
            Serial.printf("FPS: %d\n", fpsCounter);
            fpsCounter = 0;

            /*
            Serial.println(mA);
            if (Serial.available() > 0) {
                char c = Serial.read();
                if (c == '+') ACS.incMidPoint();
                if (c == '-') ACS.decMidPoint();
                if (c == '0') ACS.setMidPoint(512);
                Serial.println(ACS.getMidPoint());

                if (c == '*') ACS.setmVperAmp(ACS.getmVperAmp() * 1.05);
                if (c == '/') ACS.setmVperAmp(ACS.getmVperAmp() / 1.05);
                Serial.println(ACS.getmVperAmp());
            }
            */
        }   
    #endif
    /*
    if (lastmA != mA) {
        lastmA = mA;
        snprintf (msg, MSG_BUFFER_SIZE, "#%ld", mA);

        client.publish(mqtt_topic, msg);
        Serial.print("send ");
        Serial.print(mA);
        Serial.print(" to mqtt server via ");
        Serial.print(mqtt_topic);
        Serial.println(" Topic");
    }
    */
}