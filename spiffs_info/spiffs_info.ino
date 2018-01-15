#include "FS.h"

void setup() {

  Serial.begin(9600);
  delay(1000);
  Serial.println("Done setup");
}

void loop() {
  Serial.println("In loop");
  delay(2000);
  Serial.print("Begin result:");
  int i = SPIFFS.begin();
  Serial.println(i);

  FSInfo fs_info;
  Serial.print("Get info result:");
  i = SPIFFS.info(fs_info);
  Serial.println(i);

  Serial.println("Info:");
  Serial.print("total bytes:"); Serial.println(fs_info.totalBytes);
  Serial.print("used bytes:"); Serial.println(fs_info.usedBytes);
  Serial.print("blockSize:"); Serial.println(fs_info.blockSize);
  Serial.print("pageSize:"); Serial.println(fs_info.pageSize);
  Serial.print("maxOpenFiles:"); Serial.println(fs_info.maxOpenFiles);
  Serial.print("maxPathLength:"); Serial.println(fs_info.maxPathLength);

}
