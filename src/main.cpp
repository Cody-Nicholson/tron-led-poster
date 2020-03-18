#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FastLED.h>
#include <string>

/* Project H Files */
#include <Point.h>
#include <creds.h>
#include <ota.h>
#include "aWOT.h"
#include "StaticFiles.h"

ESP8266WiFiMulti wifiMulti;
WiFiServer server(80);
Application app;

#define NUM_BEAM_LEDS 63
#define BEAM_COL_LEN 23

#define NUM_LETTER_LEDS 107
#define NUM_TRON_LETTER_LEDS 88
#define LETTER_ROW_LEN 22

#define NUM_SUIT_LEDS 25  // excludes recognizer
#define NUM_LEFT_SUIT_LEDS 14
#define NUM_RIGHT_SUIT_LEDS 11
#define RECOGNIZER_INDEX 0
#define NUM_RECOGNIZER_LEDS 7

#define NUM_LINE_LEDS 8
#define NUM_QUORRA_LEDS 7

#define BEAM_DATA_PIN 1
#define SUIT_DATA_PIN 2
#define LETTERS_DATA_PIN 3
#define LINE_DATA_PIN 4

CRGB beamLeds[NUM_BEAM_LEDS];
CRGB suitLeds[NUM_SUIT_LEDS + NUM_RECOGNIZER_LEDS];
CRGB letterLeds[NUM_LETTER_LEDS];
CRGB quorraAndLineLeds[NUM_LINE_LEDS + NUM_QUORRA_LEDS];
CRGB *quorraSuit = quorraAndLineLeds;
CRGB *lineLeds = &quorraAndLineLeds[NUM_QUORRA_LEDS];
CRGB *recognizerLeds = &suitLeds[NUM_SUIT_LEDS];

const uint8_t INACTIVE_SLOT = 255;
uint8_t beamToStrip[BEAM_COL_LEN][3];
uint8_t lettersToStrip[4][LETTER_ROW_LEN];

uint8_t beamLoopSpeed = 40;
uint8_t ringWaveSpeed = 50;
unsigned long startTime;
unsigned long lastTime;

boolean beamIntroTriggered = 0;
boolean suitIntro = 0;

CHSV ringColor = CHSV(HUE_BLUE, 150, 165);
CHSV ringInnerColor = CHSV(HUE_BLUE, 180, 145);
CHSV ringOuterColor = CHSV(HUE_BLUE, 185, 135);
CHSV beamColor = CHSV(HUE_BLUE, 190, 125);

CHSV letterBgColor = CHSV(HUE_BLUE, 225, 60);
CHSV leadLetterColor = CHSV(HUE_BLUE, 20, 100);
CHSV legacyLetterColor = CHSV(HUE_BLUE, 20, 100);

CHSV suitColorFull = CHSV(HUE_BLUE, 100, 90);
//  CHSV suitColorHalf = CHSV(HUE_BLUE, 255, 60);

CHSV lineColor = CHSV(HUE_BLUE, 100, 70);

const CHSV badOrange = CHSV(21, 241, 150);
const CHSV otherbadOrange = CHSV(25, 216, 150);

void turnOn();

Point *pointList;

void setMasterHue(uint8_t ringHue){
  letterBgColor = CHSV(ringHue, 225, 60);
  leadLetterColor = CHSV(ringHue, 20, 100);
  legacyLetterColor = CHSV(ringHue, 20, 100);

  suitColorFull = CHSV(ringHue, 100, 90);

  lineColor = CHSV(ringHue, 100, 70);
}

void setRingColors(uint8_t ringHue) {
  beamColor = CHSV(ringHue, 190, 125);
  ringOuterColor = CHSV(beamColor.hue, beamColor.sat - 5, beamColor.val + 10);
  ringInnerColor = CHSV(beamColor.hue, beamColor.sat - 10, beamColor.val + 20);
  ringColor = CHSV(beamColor.hue, beamColor.sat - 40, beamColor.val + 40);
}

void setLetterLed(Point point, CRGB color) {
  // Serial.println(lettersToStrip[point.y][point.x]);
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
  for (uint8_t i = 0; i < BEAM_COL_LEN; i++) {
    beamToStrip[i][0] = i;
    beamToStrip[BEAM_COL_LEN - 1 - i][1] = i + BEAM_COL_LEN - 1;
    beamToStrip[i][2] = i + 46 - 6;

    beamToStrip[0][1] = INACTIVE_SLOT;
    beamToStrip[1][1] = INACTIVE_SLOT;

    beamToStrip[0][2] = INACTIVE_SLOT;
    beamToStrip[1][2] = INACTIVE_SLOT;
    beamToStrip[2][2] = INACTIVE_SLOT;

    beamToStrip[22][1] = INACTIVE_SLOT;
  }
}

boolean isInBeamRange(uint8_t rowNum) {
  return rowNum >= 0 && rowNum < BEAM_COL_LEN;
}

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

void setBeamRow(int8_t rowNum, CRGB color) {
  if (rowNum >= 0 && rowNum < BEAM_COL_LEN) {
    setBeamLed(beamToStrip[rowNum][0], color);
    setBeamLed(beamToStrip[rowNum][1], color);
    setBeamLed(beamToStrip[rowNum][2], color);
  }
}

void introBeamLoop() {
  static uint8_t rowNumber = BEAM_COL_LEN - 1;

  EVERY_N_MILLISECONDS_I(thisTimer, beamLoopSpeed) {
    setBeamRow(rowNumber, beamColor);
    if (rowNumber <= 0) {
      beamIntroTriggered = true;
      FastLED.show();
      return;
    }
    FastLED.show();
    rowNumber--;
  }
}

/* Cubic eased Wave */
void beamWaveEase() {
  static uint8_t easeOutVal = 0;
  static uint8_t lerpVal = 0;
  static int8_t wavePos = 0;

  EVERY_N_MILLISECONDS_I(thisTimer, 20) {
    easeOutVal = ease8InOutCubic(wavePos);
    lerpVal = lerp8by8(0, BEAM_COL_LEN + 6, easeOutVal) - 3;

    /* Replace Normal beam color  */
    setBeamRow(lerpVal - 3, beamColor);
    setBeamRow(lerpVal - 2, ringOuterColor);
    setBeamRow(lerpVal - 1, ringInnerColor);
    setBeamRow(lerpVal, ringColor);
    setBeamRow(lerpVal + 1, ringInnerColor);
    setBeamRow(lerpVal + 2, ringOuterColor);
    setBeamRow(lerpVal + 3, beamColor);

    FastLED.show();
    wavePos += 3;
  }
}

void ringWaveBeamLoop() {
  static int8_t wavePos = 22;
  EVERY_N_MILLISECONDS_I(thisTimer, ringWaveSpeed) {
    /* Replace Normal beam color  */
    setBeamRow(wavePos - 3, beamColor);
    setBeamRow(wavePos - 2, ringOuterColor);
    setBeamRow(wavePos - 1, ringInnerColor);
    setBeamRow(wavePos, ringColor);
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

void lightSuit() {
  fill_solid(suitLeds, NUM_SUIT_LEDS, badOrange);
  fill_solid(quorraSuit, NUM_QUORRA_LEDS, badOrange);
  //fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS, suitColorFull);
}

void lightRecognizer(){
  fill_solid(recognizerLeds, NUM_RECOGNIZER_LEDS, badOrange);
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

void debugLetterPattern() {
  Serial.println();
  for (uint8_t i = 0; i < NUM_TRON_LETTER_LEDS; i++) {
    Serial.print(pointList[i].x);
    Serial.print(',');
    Serial.print(pointList[i].y);
    Serial.println();
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
    /* Trail with bg color  */
    if (pos > 0) {
      setLetterLed(pointList[pos - 1], letterBgColor);
    }

    /* Set Lead color  */
    setLetterLed(pointList[pos], leadLetterColor);
    pos++;
    if (pos > NUM_TRON_LETTER_LEDS - 1) {
      pos = 0;
      setLetterLed(pointList[NUM_TRON_LETTER_LEDS - 1], letterBgColor);
    }
    FastLED.show();
  }
}

void lightLegacyLetters() {
  for (uint8_t i = NUM_TRON_LETTER_LEDS; i < NUM_LETTER_LEDS; i++) {
    letterLeds[i] = legacyLetterColor;
  }
}

void initWifi() {
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
}

void breathLoop() {
  float breath = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;
  breath = map(breath, 0, 255, 0, 100);
  recognizerLeds[RECOGNIZER_INDEX] = CHSV(HUE_BLUE, 255, breath);
}

void initLetters() {
  initLetterCoords();
  pointList = getTronLetterPattern();
  // debugLetterPattern();
}

void lightSolarSailer() {
  for (int i = 0; i < NUM_LINE_LEDS; i++) {
    lineLeds[i] = lineColor;
  }
}

void debugBeamIntro() {
  Serial.println();
  for (uint8_t i = 0; i < BEAM_COL_LEN; i++) {
    for (uint8_t j = 0; j < 3; j++) {
      Serial.print(beamToStrip[22 - i][2 - j]);
      Serial.print(',');
    }
    Serial.println();
  }
}

boolean isOff = false;
boolean wasOff = false;
boolean wasOn = false;

void turnOff() {
  Serial.println("Shut that thing down");
  isOff = true;
  FastLED.showColor(CRGB(0, 0, 0), 0);
}

void readPower(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  res.print(isOff ? "off" : "on");
}

void updatePower(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  String body = req.readString();
  Serial.println(body);
  if (body == "on") {
    turnOn();
    res.println("on");

  } else {
    turnOff();
    res.println("off");
  }
}

void updateHue(Request &req, Response &res) {
  res.set("Access-Control-Allow-Origin", "*");
  String body = req.readString();
  uint8_t hue = body.toInt();
  setRingColors(hue);
  setMasterHue(hue);
  res.println("set");
}

void initApi() {
  app.get("/power", &readPower);
  app.post("/power", &updatePower);
  app.post("/color/hue", &updateHue);

  app.route(staticFiles());

  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(10);
  initWifi();
  initOTA();
  initApi();

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 8000);
  FastLED.addLeds<NEOPIXEL, BEAM_DATA_PIN>(beamLeds, NUM_BEAM_LEDS);
  FastLED.addLeds<NEOPIXEL, SUIT_DATA_PIN>(suitLeds, NUM_SUIT_LEDS + NUM_RECOGNIZER_LEDS);
  FastLED.addLeds<NEOPIXEL, LETTERS_DATA_PIN>(letterLeds, NUM_LETTER_LEDS);
  FastLED.addLeds<NEOPIXEL, LINE_DATA_PIN>(quorraAndLineLeds, NUM_QUORRA_LEDS + NUM_LINE_LEDS);
  

  startTime = millis();

  initBeam();
  initLetters();

  lightSolarSailer();
  lightLegacyLetters();
  lightSuit();
  lightRecognizer();
  // debugBeamIntro();
}

void mainBeamLoop() {
  if (!beamIntroTriggered) {
    FastLED.show();
    introBeamLoop();
  } else {
    // ringWaveBeamLoop();
    beamWaveEase();
  }
}

void turnOn() {
  Serial.println("Boot UP");
  isOff = false;
  lightSolarSailer();
  lightLegacyLetters();
  lightSuit();
  lightRecognizer();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
  // server.handleClient();
  // api.handleClient();
  ArduinoOTA.handle();

  // if (millis() - startTime > 10000 && !wasOff) {
  //   wasOff = true;
  //   turnOff();
  // }

  // if (millis() - startTime > 20000 && !wasOn) {
  //   wasOn = true;
  //   turnOn();
  // }

  if (!isOff) {
    mainBeamLoop();
    loopLetters();
    // introSuitLoop();
    breathLoop();
  }
}