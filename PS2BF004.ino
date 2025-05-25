/*
PS2BF004 enables the use of gamepads for the PlayStation series via USB connection.

Copyright (C) 2023 by Lay31415 <lay31415@bm5keys-forever.com>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

This program incorporates work covered by the following copyright and permission notice:
Copyright (C) 2019-2020 by SukkoPera <software@sukkology.net> 
*/

#include "HID-Project.h"

// Config
////////////////////
#define LED_R A1
#define LED_G A3
#define LED_B A2
#define ROTATION_SCALE 96 // Recommended 60 or higher

// PsxNewLib Config
////////////////////

// Define this if you need to change the ATT pin
#define PIN_PS2_ATT 10

// Define these pins if not using HW SPI
// #define PIN_PS2_CLK 13
// #define PIN_PS2_DAT 12
// #define PIN_PS2_CMD 11

#if defined(PIN_PS2_CLK) && defined(PIN_PS2_DAT) && defined(PIN_PS2_CMD)
    // Use BitBang mode if all pins are defined
    #include <PsxControllerBitBang.h>
    PsxControllerBitBang<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;
#else
    // Use SPI mode if some pins are undefined
    #include <PsxControllerHwSpi.h>
    PsxControllerHwSpi<PIN_PS2_ATT> psx;
#endif

// Function set button for boolean
////////////////////
void setButton(uint8_t key, bool state) {
  if (state) {
    Gamepad.press(key);
  } else {
    Gamepad.release(key);
  }
}

// Function blank LED
void blank_LED() {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
}

// Function show LED
void show_LED() {
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_R, HIGH);
}

// Function Flash LED
void flash_LED() {
// LED flashes according to the selected mode
  for (int flash = 0; flash < 3; flash++){
    blank_LED();
    delay(100);
    show_LED();
    delay(100);
  }
  delay(500);
  blank_LED();
  return;
}

// x-coordinate value
short x = 0;

// Main
////////////////////
void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  flash_LED();

  psx.begin();
  Gamepad.begin();
}

void loop() {
  // Read state
  ////////////////////
  if (!psx.read()){
    // Not connected
    Gamepad.releaseAll();
    Gamepad.write();
    return;
  }

  bool state_X = psx.buttonPressed(PSB_CROSS);
  bool state_O = psx.buttonPressed(PSB_CIRCLE);
  bool state_Squ = psx.buttonPressed(PSB_SQUARE);
  bool state_Tri = psx.buttonPressed(PSB_TRIANGLE);
  bool state_L1 = psx.buttonPressed(PSB_L1);
  bool state_R1 = psx.buttonPressed(PSB_R1);
  bool state_L2 = psx.buttonPressed(PSB_L2);
  bool state_R2 = psx.buttonPressed(PSB_R2);
  bool state_Sl = psx.buttonPressed(PSB_SELECT);
  bool state_St = psx.buttonPressed(PSB_START);
  bool state_L3 = psx.buttonPressed(PSB_L3);
  bool state_R3 = psx.buttonPressed(PSB_R3);
  bool state_U = psx.buttonPressed(PSB_PAD_UP);
  bool state_D = psx.buttonPressed(PSB_PAD_DOWN);
  bool state_L = psx.buttonPressed(PSB_PAD_LEFT);
  bool state_R = psx.buttonPressed(PSB_PAD_RIGHT);

  if (state_Sl && state_St) {
    show_LED();
  } else {
    blank_LED();
  }

  setButton(1, state_Squ);
  setButton(2, state_L1);
  setButton(3, state_X);
  setButton(4, state_R1);
  setButton(5, state_O);
  setButton(6, state_L2);
  setButton(7, state_L);
  setButton(9, state_St);
  setButton(10, state_Sl);

  // SCR Assign to analog
  ////////////////////
  if (state_U) {
    x += ROTATION_SCALE;
  } else if (state_D) {
    x -= ROTATION_SCALE;
  }
  Gamepad.xAxis(x);
  Gamepad.yAxis(0);

  // Send
  Gamepad.write();
}
