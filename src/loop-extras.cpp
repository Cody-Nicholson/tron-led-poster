

// void introSuitLoop() {
//   uint32_t timeDelta = millis() - startTime;

//   if (timeDelta > startSuitDelay) {
//     fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
//   }

//   if (timeDelta > startSuitDelay + 20448) {
//     fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
//   }

//   /* LEFT SUIT  */
//   if (timeDelta > startSuitDelay + 383) {
//     fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorHalf);
//   }

//   if (timeDelta > startSuitDelay + 448) {
//     fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
//   }

//   if (timeDelta > startSuitDelay + 532) {
//     fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorHalf);
//   }

//   if (timeDelta > startSuitDelay + 781) {
//     fill_solid(suitLeds, NUM_LEFT_SUIT_LEDS, suitColorFull);
//   }

//   if (timeDelta > startSuitDelay + 448) {
//     fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
//                suitColorHalf);
//   }

//   if (timeDelta > startSuitDelay + 532) {
//     fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
//                suitColorFull);
//   }

//   if (timeDelta > startSuitDelay + 781) {
//     fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
//                suitColorHalf);
//   }

//   if (timeDelta > startSuitDelay + 835) {
//     fill_solid(suitLeds, NUM_RIGHT_SUIT_LEDS, NUM_LEFT_SUIT_LEDS,
//                suitColorFull);
//   }
// }


// void handleRoot() {  // When URI / is requested, send a web page with a
// button
//                      // to toggle the LED
//   server.send(200, "text/html",
//               "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" "
//               "value=\"Toggle LED\"></form>");
// }

// void handleLED() {  // If a POST request is made to URI /LED
//   turnOff();
//   server.send(200);  // Send it back to the browser with an HTTP status 303
//   (See
//                      // Other) to redirect
// }

// void handleNotFound() { server.send(404, "text/plain", "404: Not found"); }

// void handlePowerStatus() {
//   api.sendHeader("Access-Control-Allow-Origin", "*");
//   api.send(200, "text/html", isOff ? "off" : "on");
// }

// void handlePowerUpdate() {
//   if (api.arg("plain") == "on") {
//     turnOn();
//   } else {
//     turnOff();
//   }
//   api.sendHeader("Access-Control-Allow-Origin", "*");
//   api.send(200);
// }

// void handleAPIRoot() {
//   api.sendHeader("Access-Control-Allow-Origin",
//                  "*");  // When URI / is requested, send a web page with a
//                         // button to toggle the LED
//   api.send(200, "text/html", "API ACTION");
// }

//void initServer() {
  // if (MDNS.begin("esp")) {              // Start the mDNS responder for
  // esp8266.local
  //   Serial.println("mDNS responder started");
  // } else {
  //   Serial.println("Error setting up MDNS responder!");
  // }

  // server.on("/", HTTP_GET, handleRoot);  // Call the 'handleRoot' function
  // when
  //                                        // a client requests URI "/"
  // server.on("/LED", HTTP_POST,
  //           handleLED);  // Call the 'handleLED' function when a POST request
  //           is
  //                        // made to URI "/LED"
  // server.onNotFound(
  //     handleNotFound);  // When a client requests an unknown URI (i.e.
  //     something
  //                       // other than "/"), call function "handleNotFound"

  // server.begin();  // Actually start the server

  // api.on("/", HTTP_GET, handleAPIRoot);
  // api.on("/power", HTTP_GET, handlePowerStatus);
  // api.on("/power", HTTP_POST, handlePowerUpdate);
  // api.begin();

  // Serial.println("HTTP server started");
//}