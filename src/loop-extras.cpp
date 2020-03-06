

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