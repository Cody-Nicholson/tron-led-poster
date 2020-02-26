#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
//#define FASTLED_ALLOW_INTERRUPTS 1
//#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include <string>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>

ESP8266WiFiMulti wifiMulti;  

#define NUM_BEAM_LEDS 64
#define NUM_LETTER_LEDS 107
#define NUM_TRON_LETTER_LEDS 88
#define NUM_LEFT_SUIT_LEDS 14
#define NUM_RIGHT_SUIT_LEDS 11
#define NUM_SUIT_LEDS 25
#define BEAM_LEN 22
#define LETTERS_DATA_PIN 1
#define SUIT_DATA_PIN 2
#define LETTER_ROW_LEN 22

CRGB beamLeds[NUM_BEAM_LEDS];
CRGB letterLeds[NUM_LETTER_LEDS];
CRGB suitLeds[NUM_SUIT_LEDS];

const uint8_t INACTIVE_SLOT = 255;
uint8_t beamToStrip[BEAM_LEN + 1][3];
uint8_t lettersToStrip[4][LETTER_ROW_LEN];

uint16_t durationA = 40;    // How often to run Event A [milliseconds]
uint16_t durationB = 2000;  // How long after A to run Event B [milliseconds]
uint16_t startSuitDelay = 2000;
uint8_t ringWaveSpeed = 50;
unsigned long startTime;

boolean setBlack = false;

boolean beamIntroTriggered = 0;
boolean suitIntro = 0;

const CHSV ringColor = CHSV(HUE_BLUE, 255, 80);
const CHSV ringInnerColor = CHSV(HUE_BLUE, 255, 70);
const CHSV ringOuterColor = CHSV(HUE_BLUE, 255, 50);
const CHSV beamColor = CHSV(HUE_BLUE, 255, 50);
const CHSV leadLetterColor = CHSV(HUE_BLUE, 0, 50);

const CHSV suitColorFull = CHSV(HUE_BLUE, 100, 75);
const CHSV suitColorHalf = CHSV(HUE_BLUE, 100, 40);

class Point {
 public:
  uint8_t x, y;
  Point() {
    x = 0;
    y = 0;
  }

  Point(uint8_t a, uint8_t b) {
    x = a;
    y = b;
  }
};

Point startPos(0, 0);
Point *pointList;

void setLetterLed(Point point, CRGB color) {
  letterLeds[lettersToStrip[point.y][point.x]] = color;
}

void introSuitLoop() {
  uint16_t timeDelta = millis() - startTime;
  if (timeDelta > startSuitDelay + 383) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorHalf);
  }

  if (timeDelta > startSuitDelay + 416 + 16 * 2) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
  }

  if (timeDelta > startSuitDelay + 499 + 16 * 2) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorHalf);
  }

  if (timeDelta > startSuitDelay + 749 + 16 * 2) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
  }
}

void addMoveTo(Point start, Point end, uint8_t &pos,
               Point points[NUM_TRON_LETTER_LEDS]) {
  
  if (start.x != end.x) {
    if (start.x < end.x) {
      for (uint8_t i = 0; i < end.x - start.x; i++) {
        if(pos > NUM_TRON_LETTER_LEDS - 1){
          Serial.println("OOB SON");
          return;
        }
        if(start.x + i > 24){
          Serial.println("WTF SON");
           Serial.println(String(start.x));
           Serial.println(i);
        }
        points[pos] = Point(start.x + i, start.y);
        pos++;
      }
    } else {
      for (uint8_t i = 0; i < start.x - end.x; i++) {
        if(pos > NUM_TRON_LETTER_LEDS - 1){
          Serial.println("OOB SON");
          return;
        }
        if(start.x - i > 23){
          Serial.println("WTF SON");
           Serial.println(String(start.x));
           Serial.println(i);
        }
        points[pos] = Point(start.x - i, start.y);
        pos++;
      }
    }
  } else {
    if (start.y < end.y) {
      for (uint8_t i = 0; i < end.y - start.y; i++) {
        if(pos > NUM_TRON_LETTER_LEDS - 1){
          Serial.println("OOB SON");
          return;
        }
        points[pos] = Point(start.x, start.y + i);
        pos++;
      }
    } else {
      for (uint8_t i = 0; i < start.y - end.y; i++) {
        if(pos > NUM_TRON_LETTER_LEDS - 1){
          Serial.println("OOB SON");
          return;
        }
        points[pos] = Point(start.x, start.y - i);
        pos++;
      }
    }
  }
}

void initLetterCoords() {
  for (uint8_t i = 0; i < LETTER_ROW_LEN; i++) {
    Serial.println();
    lettersToStrip[0][LETTER_ROW_LEN - i - 1] = i;
    lettersToStrip[1][i] = LETTER_ROW_LEN + i;
    lettersToStrip[2][LETTER_ROW_LEN - i - 1] = LETTER_ROW_LEN * 2 + i;
    lettersToStrip[3][i] = LETTER_ROW_LEN * 3 + i;
  }
}

Point *getTronLetterPattern() {
  uint8_t pos = 0;
  Point *points = new Point[NUM_TRON_LETTER_LEDS];
  addMoveTo(Point(0, 0), Point(21, 0), pos, points);
  addMoveTo(Point(21, 0), Point(21, 3), pos, points);
  addMoveTo(Point(21, 3), Point(0, 3), pos, points);
  addMoveTo(Point(0, 3), Point(0, 1), pos, points);
  addMoveTo(Point(0, 1), Point(20, 1), pos, points);
  addMoveTo(Point(20, 1), Point(20, 2), pos, points);
  addMoveTo(Point(20, 2), Point(2, 2), pos, points);
  addMoveTo(Point(2, 2), Point(20, 2), pos, points);
  return points;
}

void loopLetters() {
  static uint8_t pos = 0;

  EVERY_N_MILLISECONDS(60) {
    if (pos > NUM_TRON_LETTER_LEDS - 1) {
      return;
    }
    Point point = pointList[pos];

    Serial.println("POS: " + String(pos) + "Lighting: " + String(point.x) + "," + String(point.y));
    if (pos > 1) {
      setLetterLed(pointList[pos - 1], beamColor);
    }
    setLetterLed(pointList[pos], leadLetterColor);

    if (pos == NUM_TRON_LETTER_LEDS) {
      setLetterLed(pointList[pos - 1], beamColor);
    }

    pos++;
    if (pos > NUM_TRON_LETTER_LEDS - 1) {
      pos = 0;
    }
    FastLED.show();
  }
}

void setupOTA(){
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');
  wifiMulti.addAP("HackNet", "thecodemanhackedthisrouter");  

  Serial.println("Connecting ...");
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  
  ArduinoOTA.setHostname("ESP8266");
  ArduinoOTA.setPassword("esp8266");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
   Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void setup() {
  setupOTA();
  delay(200);
  FastLED.addLeds<NEOPIXEL, LETTERS_DATA_PIN>(letterLeds, NUM_LETTER_LEDS);

  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(beamLeds, NUM_BEAM_LEDS);
  
  // //FastLED.addLeds<NEOPIXEL, SUIT_DATA_PIN>(suitLeds, NUM_SUIT_LEDS);
  // initBeam();
  startTime = millis();
  initLetterCoords();
  pointList = getTronLetterPattern();
}

void loop() {
  ArduinoOTA.handle();
  // if (!beamIntroTriggered)
  // {
  //   introBeamLoop();
  // }
  // else
  // {
  //   waveBeamLoop();
  // }
  loopLetters();
  // static CEveryNMilliseconds suitTimer(20000);
  //introSuitLoop();
  // fill_solid(letterLeds, NUM_LETTER_LEDS, CRGB(10, 0, 0));
}