

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "images.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "PCF8574.h"

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16


//#ifdef SSD_DISPLAY
SSD1306 display(0x3c,4,15);
//#endif
PCF8574 expander(4,15,0x38);

#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 1;

volatile boolean serveIRQ = false;

void expanderIRQ() {
  serveIRQ = true;
}

void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  Serial.print("ARDUINO:");
  Serial.println(ARDUINO);
  
  // Initialising the UI will init the display too.
//#ifdef SSD_DISPLAY
  display.init();
//#endif

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  pumpDemoS();

  // Set I2C clock down to 100khz since the expander can't handle more!
  Wire.setClock(100000);

  pinMode(23,INPUT);
  attachInterrupt(digitalPinToInterrupt(23),expanderIRQ, FALLING);
  
  expander.begin();

}

//#ifdef SSD_DISPLAY
void pumpDemoS() {
  display.setFont(ArialMT_Plain_10);
//  display.setFont(ArialMT_Plain_16);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "RSSI");
  display.drawString(32, 0, ": 12");

  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, "14:01:23");

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 12, "Tank");
  display.drawString(32, 12, ": 88 % Full");
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 24, "Pump");
  display.drawString(32, 24, ": On");
  
  display.drawString(66, 24, "Flow");
  display.drawString(98, 24, ": 100");

  display.drawString(0, 36, "ZoneA");
  display.drawString(32, 36, ": 12 s");

  display.drawString(66, 36, "ZoneC");
  display.drawString(98, 36, ": 12 s");

  display.drawString(0, 48, "ZoneB");
  display.drawString(32, 48, ": Off");

  display.drawString(66, 48, "ZoneD");
  display.drawString(98, 48, ": Off");

  display.display();
}
//#endif

//Demo demos[] = {drawFontFaceDemo, drawTextFlowDemo, drawTextAlignmentDemo, drawRectDemo, drawCircleDemo, drawProgressBarDemo, drawImageDemo, pumpDemo};
//Demo demos[] = {pumpDemoS};
//int demoLength = (sizeof(demos) / sizeof(Demo));
long timeSinceLastModeSwitch = 0;

void loop() {
  // clear the display
//  display.clear();
  // draw the current demo method
 // demos[demoMode]();

//#ifdef SSD_DISPLAY
  display.setColor(BLACK);
  display.fillRect(70,0,128-70,10);
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, String(millis()));
  
  // write the buffer to the display
  display.display();
//#endif

  if (serveIRQ) {
    serveIRQ = false;
    int x = expander.read8();
    Serial.print("Read ");
    Serial.println(x, HEX);
  }

  delay(10);
}
