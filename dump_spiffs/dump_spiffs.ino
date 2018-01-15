
#include "FS.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Setup done");
}

void loop() {
  Serial.println("Awake");
  if (!SPIFFS.begin()) {
    Serial.println("Can't start spiffs");    
  }
  String str = "";    
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
  
    str = dir.fileName();
    str += "  ";
    str += dir.fileSize();
    Serial.println(str); 
  }
  Serial.println("Waiting...");
  delay(5000);
}
