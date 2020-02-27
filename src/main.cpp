#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FastLED.h>
#include <creds.h>
#include <string>

ESP8266WiFiMulti wifiMulti;

#define NUM_BEAM_LEDS 64
#define BEAM_COL_LEN 22

#define NUM_LETTER_LEDS 107
#define NUM_TRON_LETTER_LEDS 88
#define LETTER_ROW_LEN 22

#define NUM_SUIT_LEDS 26 // Includes Recognizer
#define NUM_LEFT_SUIT_LEDS 14
#define NUM_RIGHT_SUIT_LEDS 11

#define NUM_LINE_LEDS 8

#define BEAM_DATA_PIN 1
#define SUIT_DATA_PIN 2
#define LETTERS_DATA_PIN 3
#define LINE_DATA_PIN 4

CRGB beamLeds[NUM_BEAM_LEDS];
CRGB suitLeds[NUM_SUIT_LEDS];
CRGB letterLeds[NUM_LETTER_LEDS];
CRGB lineLeds[NUM_LINE_LEDS];

const uint8_t INACTIVE_SLOT = 255;
uint8_t beamToStrip[BEAM_COL_LEN + 1][3];
uint8_t lettersToStrip[4][LETTER_ROW_LEN];

uint16_t durationA = 40;    // How often to run Event A [milliseconds]
uint16_t durationB = 2000;  // How long after A to run Event B [milliseconds]
uint16_t startSuitDelay = 2000;
uint8_t ringWaveSpeed = 50;
unsigned long startTime;

boolean beamIntroTriggered = 0;
boolean suitIntro = 0;

const CHSV ringColor = CHSV(HUE_BLUE, 255, 80);
const CHSV ringInnerColor = CHSV(HUE_BLUE, 255, 70);
const CHSV ringOuterColor = CHSV(HUE_BLUE, 255, 50);
const CHSV beamColor = CHSV(HUE_BLUE, 255, 50);
const CHSV leadLetterColor = CHSV(HUE_BLUE, 20, 50);

const CHSV suitColorFull = CHSV(HUE_BLUE, 100, 75);
const CHSV suitColorHalf = CHSV(HUE_BLUE, 100, 40);

const CHSV lineColor = CHSV(HUE_BLUE, 100, 40);

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

Point *pointList;

void setLetterLed(Point point, CRGB color) {
  letterLeds[lettersToStrip[point.y][point.x]] = color;
}

void fill_solid(struct CRGB *targetArray, int startFill, int numToFill,
                const struct CHSV &hsvColor) {
  for (int i = startFill; i < startFill + numToFill; i++) {
    targetArray[i] = hsvColor;
  }
}
/* Map matrix to strip position  */
void initBeam() {
  for (uint8_t i = 0; i < 23; i++) {
    beamToStrip[i][0] = i;
    beamToStrip[22 - i][1] = i + 23;
    beamToStrip[i][2] = i + 46 - 5;

    beamToStrip[0][1] = INACTIVE_SLOT;
    beamToStrip[1][1] = INACTIVE_SLOT;

    beamToStrip[0][2] = INACTIVE_SLOT;
    beamToStrip[1][2] = INACTIVE_SLOT;
    beamToStrip[2][2] = INACTIVE_SLOT;
  }
}

boolean isInBeamRange(uint8_t x) { return (x - BEAM_COL_LEN) * (x) <= 0; }

void setBeamLed(uint8_t ledNum, CRGB color) {
  if (ledNum != INACTIVE_SLOT) {
    beamLeds[ledNum] = color;
  }
}

/* Safe array */
void setBeamLed(uint8_t x, uint8_t y, CHSV color) {
  if (isInBeamRange(x)) {
    setBeamLed(beamToStrip[x][y], color);
  }
}

void setBeamRow(uint8_t rowNum, CRGB color) {
  setBeamLed(beamToStrip[rowNum][0], color);
  setBeamLed(beamToStrip[rowNum][1], color);
  setBeamLed(beamToStrip[rowNum][2], color);
}

void ringWaveBeamLoopOuter() {
  static int8_t wavePos = 22;
  EVERY_N_MILLISECONDS_I(thisTimer, ringWaveSpeed) {
    /* Replace Normal beam color  */
    setBeamLed(wavePos - 3, 0, beamColor);
    setBeamLed(wavePos - 3, 2, beamColor);
    setBeamLed(wavePos - 2, 0, ringOuterColor);
    setBeamLed(wavePos - 2, 2, ringOuterColor);
    setBeamLed(wavePos - 1, 0, ringInnerColor);
    setBeamLed(wavePos - 1, 2, ringInnerColor);
    setBeamLed(wavePos, 0, ringColor);
    setBeamLed(wavePos, 2, ringColor);
    setBeamLed(wavePos + 1, 0, ringInnerColor);
    setBeamLed(wavePos + 1, 2, ringInnerColor);
    setBeamLed(wavePos + 2, 0, ringOuterColor);
    setBeamLed(wavePos + 2, 2, ringOuterColor);
    setBeamLed(wavePos + 3, 0, beamColor);
    setBeamLed(wavePos + 3, 2, beamColor);

    FastLED.show();
    wavePos++;
    if (wavePos >= BEAM_COL_LEN + 5) {
      wavePos = 0;
    }
  }
}

void ringWaveBeamLoop() {
  static int8_t wavePos = 22;
  EVERY_N_MILLISECONDS_I(thisTimer, ringWaveSpeed) {
    /* Replace Normal beam color  */
    setBeamRow(wavePos - 3, beamColor);
    setBeamRow(wavePos - 2, ringOuterColor);
    setBeamRow(wavePos - 1, ringInnerColor);
    setBeamRow(wavePos, beamColor);
    setBeamRow(wavePos + 1, ringInnerColor);
    setBeamRow(wavePos + 2, ringOuterColor);
    setBeamRow(wavePos + 3, beamColor);

    FastLED.show();
    wavePos++;
    if (wavePos >= BEAM_COL_LEN + 5) {
      wavePos = 0;
    }
  }
}

void introBeamLoop() {
  static uint8_t rowNumber = BEAM_COL_LEN;

  EVERY_N_MILLISECONDS_I(thisTimer, durationA) {
    setBeamRow(rowNumber, beamColor);
    if (rowNumber <= 0) {
      beamIntroTriggered = true;
    }
    FastLED.show();
    rowNumber--;
  }
}

void introSuitLoop() {
  uint16_t timeDelta = millis() - startTime;

  /* LEFT SUIT  */
  if (timeDelta > startSuitDelay + 383) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorHalf);
  }

  if (timeDelta > startSuitDelay + 448) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
  }

  if (timeDelta > startSuitDelay + 532) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorHalf);
  }

  if (timeDelta > startSuitDelay + 781) {
    fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
  }

  if (timeDelta > startSuitDelay + 448) {
    fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
               suitColorHalf);
  }

  if (timeDelta > startSuitDelay + 532) {
    fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
               suitColorFull);
  }

  if (timeDelta > startSuitDelay + 781) {
    fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
               suitColorHalf);
  }

  if (timeDelta > startSuitDelay + 835) {
    fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
               suitColorFull);
  }
}

void addMoveTo(Point start, Point end, uint8_t &pos,
               Point points[NUM_TRON_LETTER_LEDS]) {
  if (start.x != end.x) {
    if (start.x < end.x) {
      for (uint8_t i = 0; i < end.x - start.x; i++) {
        points[pos] = Point(start.x + i, start.y);
        pos++;
      }
    } else {
      for (uint8_t i = 0; i < start.x - end.x; i++) {
        points[pos] = Point(start.x - i, start.y);
        pos++;
      }
    }
  } else {
    if (start.y < end.y) {
      for (uint8_t i = 0; i < end.y - start.y; i++) {
        points[pos] = Point(start.x, start.y + i);
        pos++;
      }
    } else {
      for (uint8_t i = 0; i < start.y - end.y; i++) {
        points[pos] = Point(start.x, start.y - i);
        pos++;
      }
    }
  }
}

void initLetterCoords() {
  for (uint8_t i = 0; i < LETTER_ROW_LEN; i++) {
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
  addMoveTo(Point(20, 2), Point(0, 2), pos, points);  // Last point doesn't fill
  return points;
}

void loopLetters() {
  static uint8_t pos = 0;

  EVERY_N_MILLISECONDS(60) {
    if (pos > 0) {
      setLetterLed(pointList[pos - 1], beamColor);
    }
    setLetterLed(pointList[pos], leadLetterColor);
    pos++;
    if (pos > NUM_TRON_LETTER_LEDS - 1) {
      pos = 0;
      setLetterLed(pointList[NUM_TRON_LETTER_LEDS - 1], beamColor);
    }
    FastLED.show();
  }
}

void setupOTA() {
  Serial.begin(115200);
  delay(10);
  wifiMulti.addAP(ROUTER_SSID, ROUTER_PASS);

  Serial.println("\nConnecting ...");
  while (wifiMulti.run() != WL_CONNECTED) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.print("\nConnected to ");
  Serial.println(WiFi.SSID());  // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(
      WiFi.localIP());  // Send the IP address of the ESP8266 to the computer

  ArduinoOTA.setHostname("ESP8266");
  ArduinoOTA.setPassword("esp8266");
  ArduinoOTA.setPort(8266);

  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void setupLetters() {
  initLetterCoords();
  pointList = getTronLetterPattern();
}

void setup() {
  setupOTA();
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 800);
  // FastLED.addLeds<NEOPIXEL, LETTERS_DATA_PIN>(letterLeds, NUM_LETTER_LEDS);
  FastLED.addLeds<NEOPIXEL, BEAM_DATA_PIN>(beamLeds, NUM_BEAM_LEDS);
  FastLED.addLeds<NEOPIXEL, SUIT_DATA_PIN>(suitLeds, NUM_SUIT_LEDS);
  initBeam();
  // setupLetters();
  startTime = millis();
}

void mainBeamLoop() {
  if (!beamIntroTriggered) {
    introBeamLoop();
  } else {
    ringWaveBeamLoop();
  }
}

void loop() {
  ArduinoOTA.handle();

  mainBeamLoop();
  // loopLetters();
  // static CEveryNMilliseconds suitTimer(20000);
  introSuitLoop();
  // fill_solid(letterLeds, NUM_LETTER_LEDS, CRGB(10, 0, 0));
}