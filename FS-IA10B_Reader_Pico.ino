/*

RC Controller Reader for FPU Roboboat Competition Team
First Iteration: 1/17/2026
First Iteration w/ Pico: 3/4/26
Controller: FS-i6X, Receiver: FS-iA10B
Contributors: Rainer Dolmanet, David Zambrano

Ch1:  R-Sti LR
Ch2:  R-Sti UD
Ch3:  L-Sti UD
Ch4:  L-Sti LR
Ch5:  L-Pot
Ch6:  R-Pot
Ch7:  Swi-A
Ch8:  Swi-B
Ch9:  Swi-C
Ch10: Swi-D

NOTE: When uploading, ensure RX pin not receiving any data.

*/

#include "FlySkyIBus.h"

void setup() {
  // put your setup code here, to run once:
Serial1.begin(115200); 
IBus.begin(Serial1);
while(!Serial1) {}

}

void loop() {
  IBus.loop();
  for(int i = 0; i < 10; i++) {
    switch(i) {
      case 2: case 4: case 5: //Value map for pots and non-centered sticks
        Serial.print(map(IBus.readChannel(i), 1000, 2000, 0, 100)); 
        break;
      case 6: case 7: case 8: case 9: //Value map for switches
        Serial.print(map(IBus.readChannel(i), 1000, 2000, -1, 1)); 
        break;
      default: //Value map for centered sticks
        Serial.print(map(IBus.readChannel(i), 1000, 2000, -100, 100));
    }
    Serial.print("|");
  }
  Serial.println();
  delay(1); //Adjust delay if program moves too fast for Pico
}
