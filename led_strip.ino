#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <JsonListener.h>
#include <time.h>
#include "OpenWeatherMapCurrent.h"
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>

RTC_Millis rtc;

boolean IS_METRIC = false;

const char* ssid = "TP-Link_F510";
const char* password = "23263376";

String OPEN_WEATHER_MAP_LANGUAGE = "en";
String OPEN_WEATHER_MAP_APP_ID = "56917cc227986aeab113d819b96220ab";
String OPEN_WEATHER_MAP_LOCATION_ID = "4781708";

WiFiClient wifiClient;

OpenWeatherMapCurrent client;

OpenWeatherMapCurrentData data;

const int motionPin = 4;
const int ledPin = 14;

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 24

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, ledPin, NEO_GRB + NEO_KHZ800);

const int UPDATE_INTERVAL = 20 * 60;

void connectWifi() {
	WiFi.begin(ssid, password);
	Serial.print("Connecting to wifi");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected!");
	Serial.println(WiFi.localIP());
	Serial.println();
}


void setup()
{
	Serial.begin(9600);
  pinMode(motionPin, INPUT);
  rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
	connectWifi();
	client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
	client.setMetric(IS_METRIC);
	strip.begin();
	strip.show();
	strip.setBrightness(50);
}

void loop() {
  byte is_motion_detected = digitalRead(motionPin);
  DateTime now = rtc.now();
  if ((now.hour()>=20 || now.hour()<=7) && is_motion_detected == 1){
    sunnyDay(strip.Color(30,0,50), 200);
  }else if (is_motion_detected == 1){   
    updateWeather();
    makeLightDecision();
  }else{
    strip.clear();
    strip.show();
  }
  delay(100); 
}

void updateWeather() {
	client.updateCurrentById(&data, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID);
}

void lightning() {
  sunnyDay(strip.Color(10, 100, 130), 80);
  stormyLights();
}

void stormyLights()
{
  int flashCount = random(3, 15);        // Min. and max. number of flashes each loop
  int flashBrightnessMin = 10;           // LED flash min. brightness (0-255)
  int flashBrightnessMax = 255;          // LED flash max. brightness (0-255)

  int flashDurationMin = 1;               // Min. duration of each seperate flash
  int flashDurationMax = 30;              // Max. duration of each seperate flash

  int nextFlashDelayMin = 1;              // Min, delay between each flash and the next
  int nextFlashDelayMax = 80;            // Max, delay between each flash and the next

  int loopDelay = random(5000, 30000);   // Min. and max. delay between each loop

  for (int flash = 0; flash <= flashCount; flash += 1) { // Flashing LED strip in a loop, random count

    strip.setBrightness(random(flashBrightnessMin, flashBrightnessMax));
    strip.setPixelColor(24, (10, 100, 130));
    strip.show();
    delay(random(flashDurationMin, flashDurationMax)); // Keep it tured on, random duration
    delay(random(nextFlashDelayMin, nextFlashDelayMax)); // Random delay before next flash
  }
   
}

void rainEffect(uint32_t color, int wait) {
  for(int a=0; a<100; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      for(int c=b; c<strip.numPixels(); c += 4) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void sunnyDay(uint32_t color, int wait) {
  for(int a=0; a<100; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void otherWeather(uint8_t red, uint8_t green, uint8_t blue, uint8_t wait) {
  for(uint8_t b = 0; b <200; b++) {
     for(uint8_t i=0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, red * b/255, green * b/255, blue * b/255);
     }

     strip.show();
     delay(wait);
  };

  for(uint8_t b=200; b > 0; b--) {
     for(uint8_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, red * b/255, green * b/255, blue * b/255);
     }
     strip.show();
     delay(wait);
  }
}

void makeLightDecision() {
  DateTime now = rtc.now();
  if (data.main == "Clear"){
    sunnyDay(strip.Color(220, 140, 0), 50);
  }else if (data.main == "Clouds") {
    sunnyDay(strip.Color(80, 50, 20), 150);
  }else if (data.main == "Thunderstorm") {
    lightning();
  }else if (data.main == "Tornado" || data.main ==  "Squall") {
    strip.Color(255, 0, 0);
    strip.show();
  }else if (data.main == "Drizzle" || data.main == "Rain") {
	  rainEffect(strip.Color(0, 0, 200), 300);
  }else{
    otherWeather(200,200,200,10);
  }
}
