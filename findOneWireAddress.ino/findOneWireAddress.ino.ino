/*-----( Import needed libraries )-----*/
#include <OneWire.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define SENSOR_PIN 4  // Any pin 2 to 12 (not 13) and A0 to A5

/*-----( Declare objects )-----*/
OneWire  ourBus(SENSOR_PIN);  // Create a 1-wire object

void setup()  /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(115200);

  discoverOneWireDevices();  // Everything happens here!
}//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  discoverOneWireDevices();
  delay(1000);
}
 

/*-----( Declare User-written Functions )-----*/
void discoverOneWireDevices(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  Serial.print("Looking for 1-Wire devices...\n\r");// "\n\r" is NewLine
  while (ourBus.search(addr)) {
    Serial.print("\n\r\n\rFound \'1-Wire\' device with address:\n\r");
    for ( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n\r");
      return;
    }
  }
  Serial.println();
  Serial.print("Done");
  ourBus.reset_search();
  return;
}

