/*

RC Controller Reader for FPU Roboboat Competition Team
First Iteration w/ Arduino Mega: 1/17/2026
First Iteration w/ Pico: 3/4/26
Second Iteration w/ Manual Thruster Control: 4/4/26
Controller: FS-i6X, Receiver: FS-iA10B
Contributors: Rainer Dolmanet, David Zambrano

This file converts the data received from the FS-iA10B receiver into the necessary throttle values to drive the ESC for the for manual thruster control

Features include: 
- Adjustable thruster sensitivity
- Automatic spin direction change

ESC used: https://www.amazon.com/SEQURE-Brushless-Controller-30-5x30-5mm-Freestyle/dp/B0FB8J15M2

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

DIRECTIONS FOR USE:

With the FS-i6X controller: t

The left stick serves as a thruster senstivity multiplier. When all the way down, this multiplier is set to 0 (No movement).
As you push it up, it will increase the multiplier's value. 

The right stick provides complete control of each thruster. Vertical movement (UP - DOWN) corresponds
to the back two thrusters on the boat. Push forward to go forward, push back to go backward. Horizontal movement (LEFT - RIGHT) corresponds to the front two thrusters. 
Pushing right will turn the boat right in place. Pushing left will turn the boat left in place. Any combination of these movements will be handled as specified.


*/

#include "FlySkyIBus.h"
#include <PIO_DShot.h>
#include <cstdlib> 

// DEFINES ===========================================================

#define UP -1 
#define MIDDLE 0
#define DOWN 1
#define FORWARD 7
#define BACKWARD 8

#define MOTOR1_PIN 16   // Motor Pins.
#define MOTOR2_PIN 17
#define MOTOR3_PIN 18
#define MOTOR4_PIN 19

#define MOTOR_POLES 14 //constant for thrusters (always 14)

// Values from FlySky Receiver ===========================================================

int Swi_A, Swi_B, Swi_C, Swi_D;   
int LStick_LR, LStick_UD, RStick_LR, RStick_UD; 
int Pot_L, Pot_R;


// Array Variables ===========================================================

BidirDShotX1 *esc[4] = {nullptr, nullptr, nullptr, nullptr}; // Array of pointers to BidirDShotX1 objects for each motor
uint8_t commandCount[4] = {0, 0, 0, 0};                      // # of times a command (change direction) is sent, starts at 0
bool backwardThrottle[4] = {false, false, false, false};     // Boolean to know when motor is spinning in reverse (or SPIN_DIRECTION_2)
int throttle[4] = {0, 0, 0, 0};                              // Instantaenous throttle value for each esc at any given point
uint32_t thruster_rpm[4] = {0, 0, 0, 0};                     // Instantaenous RPM read from a given ESC at any given point
uint32_t returnValue[4] = {0, 0, 0, 0};                      // Telemetry dump variabe
bool esc_enable[4] = {true, true, true, true};               // Either continue sending read values from receiver, or update spin direction.



void setup() {
  Serial.begin(9600);
  while(!Serial) {}       // Ensure Serial boots.
  Serial.print("HALLO");
  esc[0] = new BidirDShotX1(MOTOR1_PIN); 
  esc[1] = new BidirDShotX1(MOTOR2_PIN);
  esc[2] = new BidirDShotX1(MOTOR3_PIN);
  esc[3] = new BidirDShotX1(MOTOR4_PIN);
}

void setup1() {
  Serial1.begin(115200); 
  IBus.begin(Serial1);
  while(!Serial1) {}     // Ensure Serial1 boots.
}

void loop() {

  delayMicroseconds(200);     // Manual delay needed as sendThrottle command is non-blocking.

  for(int esc_num = 0; esc_num < 4; esc_num++) {    // Iterate through each ESC every loop.

    if(esc_enable[esc_num] == true) {                 

      switch(esc_num) {
        case 0: case 1:  
         throttle[esc_num] = RStick_LR;
         break;
        case 2: case 3:
         throttle[esc_num] = RStick_UD;
         break;
    }
  
    if(throttle[esc_num] >= -20 && throttle[esc_num] <= 20) {
      throttle[esc_num] = 0;
    }

    throttle[esc_num] = throttle[esc_num] * LStick_UD; // Throttle multiplier

    if(throttle[esc_num] <= -20 && backwardThrottle[esc_num] == false) {
      Serial.println("Change to backward.");
      backwardThrottle[esc_num] = true;
      esc_enable[esc_num] = false;
      continue;
    }
  
    if(throttle[esc_num] >= 20 && backwardThrottle[esc_num] == true) {
      Serial.println("Change to forward.");
      backwardThrottle[esc_num] = false;
      esc_enable[esc_num] = false;
      continue;
    }


  /*
  /*   
  if(esc_num == 0) {
  Serial.print("Throttle 0: ");
  Serial.print(abs(throttle[esc_num]));
    if(backwardThrottle[esc_num] == true) {
        Serial.print(" REVERSE"); 
    }
  }
  
  if(esc_num == 1) {
  Serial.print("Throttle 1: ");
  Serial.print(abs(throttle[esc_num]));
    if(backwardThrottle[esc_num] == true) {
        Serial.print(" REVERSE"); 
    }
  }
  if(esc_num == 2) {
  Serial.print("Throttle 2: ");
  Serial.print(abs(throttle[esc_num]));
    if(backwardThrottle[esc_num] == true) {
        Serial.print(" REVERSE"); 
    }
  }
  if(esc_num == 3) {
  Serial.print("Throttle 3: ");
  Serial.print(abs(throttle[esc_num]));
    if(backwardThrottle[esc_num] == true) {
        Serial.print(" REVERSE"); 
    }
  }
   
   //Serial.print(" | ");
  */

   esc[esc_num]->sendThrottle(max(0, min(200, abs(static_cast<int32_t>(throttle[esc_num])))));    // Main sendThrottle command, ensures throttle is within acceptable range.
   
  } else if(esc_enable[esc_num] == false) {

     BidirDshotTelemetryType type = esc[esc_num]->getTelemetryPacket(&returnValue[esc_num]);
     switch (type) {
	    case BidirDshotTelemetryType::ERPM:
	  	thruster_rpm[esc_num] = returnValue[esc_num] / (MOTOR_POLES / 2);
		  break;
     }
     Serial.print("ESC: ");
     Serial.print(esc_num);
     Serial.print("\t RPM: ");
     Serial.println(thruster_rpm[esc_num]);

     if(thruster_rpm[esc_num] == 0) { 
      if(backwardThrottle[esc_num] == true) {
		    esc[esc_num]->sendRaw11Bit(BACKWARD);
        Serial.print("Sending Direction Backward ESC: ");
        Serial.println(esc_num);
      } else {
          esc[esc_num]->sendRaw11Bit(FORWARD);
          Serial.println("Sending Direction Forward ESC:");
          Serial.println(esc_num);
      }
      commandCount[esc_num]++;
      Serial.print("Command Count ESC ");
      Serial.print(esc_num);
      Serial.print(": ");
      Serial.println(commandCount[esc_num]);
      if(commandCount[esc_num] > 9) {
      Serial.println("Done.");
      commandCount[esc_num] = 0;
      esc_enable[esc_num] = true;
      }
    } else {
       esc[esc_num]->sendThrottle(0);
    }
   }
  }                                     // End of for loop.
  //Serial.println("");
}

void loop1() {                  // Second RP2040 / RP2350 core handles reading from FlySky receiver.
 IBus.loop();
  for(int i = 0; i < 10; i++) {
    switch(i) {
      case 0: // Right stick LR
      RStick_LR = map(IBus.readChannel(i), 1000, 2000, -100, 100);
      break;
      case 1: 
      RStick_UD = map(IBus.readChannel(i), 1000, 2000, -100, 100);
      break;
      case 2:
      LStick_UD = map(IBus.readChannel(i), 1000, 2000, 0, 3);
      break;
      case 3: //Left Stick LR
      LStick_LR = map(IBus.readChannel(i), 1000, 2000, -100, 100);
      break;
      case 4:
      Pot_L = map(IBus.readChannel(i), 1000, 2000, 0, 100);
      break;
      case 5:
      Pot_R = map(IBus.readChannel(i), 1000, 2000, 0, 100);
      break;
      case 6: 
      Swi_A = map(IBus.readChannel(i), 1000, 2000, -1, 1);
      break;
      case 7: 
      Swi_B = map(IBus.readChannel(i), 1000, 2000, -1, 1);
      break;
      case 8: 
      Swi_C = map(IBus.readChannel(i), 1000, 2000, -1, 1);
      break;
      case 9: 
      Swi_D = map(IBus.readChannel(i), 1000, 2000, -1, 1);  // Toggle between manual and autonomous control
      break;
    }
  }
}
