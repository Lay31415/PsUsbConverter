/*
PsUsbConverter enables the use of gamepads for the PlayStation series via USB connection.

Copyright (C) 2019-2020 by SukkoPera <software@sukkology.net>
Copyright (C) 2023 by Lay31415 <lay31415@bm5keys-forever.com>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "HID-Project.h"
#include <EEPROM.h>

// Config
////////////////////
#define JOY_DEADZONE 8192   // Disabled at 0
#define ALWAYS_LED false
#define CHECK_LED true
#define DUPE_ASSIGN true
#define LED_R A1
#define LED_G A3
#define LED_B A2

// PsxNewLib HwSpi Config
// DAT CMD CLK must be connected to HWSPI pin
////////////////////

#include <PsxControllerHwSpi.h>
#define PIN_PS2_ATT 10
PsxControllerHwSpi<PIN_PS2_ATT> psx;

// PsxNewLib BitBang Config
// Specify any GPIO
////////////////////

// #include <PsxControllerBitBang.h>
// #define PIN_PS2_CLK 13
// #define PIN_PS2_DAT 12
// #define PIN_PS2_CMD 11
// #define PIN_PS2_ATT 10
// PsxControllerBitBang<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;

// Function Analog stick value
////////////////////
#define PSX_ANALOG_MIN 0
#define PSX_ANALOG_MAX 255
#define JOY_ANALOG_MIN -32768
#define JOY_ANALOG_MAX 32767
int psx_analog_to_joy(byte val){
  return map(val, PSX_ANALOG_MIN, PSX_ANALOG_MAX, JOY_ANALOG_MIN, JOY_ANALOG_MAX);
}

// Function set button for boolean
////////////////////
void setButton(uint8_t key, bool state) {
  if (state) {
    Gamepad.press(key);
  } else {
    Gamepad.release(key);
  }
}

// Function Arrow key to Dpad
////////////////////
byte setDpad(bool upKey, bool downKey, bool leftKey, bool rightKey) {
  byte udlr = upKey << 3 | downKey << 2 | leftKey << 1 | rightKey;
  switch (udlr) {
    case 0b1000:
      return (GAMEPAD_DPAD_UP);
    case 0b0100:
      return (GAMEPAD_DPAD_DOWN);
    case 0b0010:
      return (GAMEPAD_DPAD_LEFT);
    case 0b0001:
      return (GAMEPAD_DPAD_RIGHT);
    case 0b1010:
      return (GAMEPAD_DPAD_UP_LEFT);
    case 0b1001:
      return (GAMEPAD_DPAD_UP_RIGHT);
    case 0b0110:
      return (GAMEPAD_DPAD_DOWN_LEFT);
    case 0b0101:
      return (GAMEPAD_DPAD_DOWN_RIGHT);
    case 0b1111:
      return (GAMEPAD_DPAD_UP); // pop'n support
    default:
      return (GAMEPAD_DPAD_CENTERED);
  }
}

// Mode pattern
////////////////////
enum Mode {POV, BUTTON, KONAMI, MODE_LEN};
byte mode;
enum Dpad {DPAD, LS, RS, DPAD_LEN};
byte dpad;

// Function blank LED
void blank_LED() {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
}

// Function show LED
void show_LED() {
  switch (mode) {
    case POV:
      digitalWrite(LED_B, HIGH);
      switch (dpad) {
        case LS:
          digitalWrite(LED_G, HIGH);
          break;
        case RS:
          digitalWrite(LED_R, HIGH);
          break;
      }
      break;
    case BUTTON:
      digitalWrite(LED_G, HIGH);
      break;
    case KONAMI:
      digitalWrite(LED_R, HIGH);
      break;
  }
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
  if (! ALWAYS_LED) blank_LED();
  return;
}

// Main
////////////////////
void setup() {
  // Read savedata
  EEPROM.get(0, mode);
  if (mode >= MODE_LEN) mode = 0;
  EEPROM.get(1, dpad);
  if (dpad >= DPAD_LEN) dpad = 0;

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  flash_LED();

  psx.begin();
  NKROKeyboard.begin();
  Gamepad.begin();
}

void loop() {
  // Read state
  ////////////////////
  if (!psx.read()){
    // Not connected
    Gamepad.releaseAll();
    Gamepad.write();
    NKROKeyboard.releaseAll();
    NKROKeyboard.send();
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

  if (state_Sl && state_St && CHECK_LED) {
    show_LED();
  } else if (! ALWAYS_LED) {
    blank_LED();
  }
  if (state_Sl && state_St && state_U) {
    // mode select
    ////////////////////
    if (++mode >= MODE_LEN) mode = 0;

    // Reset state
    Gamepad.releaseAll();
    Gamepad.write();
    NKROKeyboard.releaseAll();
    NKROKeyboard.send();

    // mode save
    EEPROM.put(0, mode);

    // LED flashes according to the selected mode
    flash_LED();
  }

  switch (mode) {
    case POV:
      // Dpad select
      ////////////////////
      if (state_Sl && state_St) {
        if (!state_U && state_L && !state_D && !state_R){
          dpad = DPAD;
        }
        if (!state_U && !state_L && state_D && !state_R){
          dpad = LS;
        }
        if (!state_U && !state_L && !state_D && state_R){
          dpad = RS;
        }
        if (dpad >= DPAD_LEN) dpad = 0;

        if (state_L || state_D || state_R){
          // Reset state
          Gamepad.releaseAll();
          Gamepad.write();
          NKROKeyboard.releaseAll();
          NKROKeyboard.send();

          // mode save
          EEPROM.put(1, dpad);

          // LED flashes according to the selected mode
          flash_LED();
        }
      }
    case BUTTON:
      // Gamepad Mode
      ////////////////////

      // Set Joystick
      Gamepad.xAxis(0);
      Gamepad.yAxis(0);
      Gamepad.rxAxis(0);
      Gamepad.ryAxis(0);
      byte x, y;
      if (psx.getLeftAnalog(x, y)) {
        int X = psx_analog_to_joy(x);
        int Y = psx_analog_to_joy(y);
        if (abs(X) < JOY_DEADZONE && abs(Y) < JOY_DEADZONE) {
          X = 0;
          Y = 0;
        }
        Gamepad.xAxis(X);
        Gamepad.yAxis(Y);
      }
      if (psx.getRightAnalog(x, y)) {
        int X = psx_analog_to_joy(x);
        int Y = psx_analog_to_joy(y);
        if (abs(X) < JOY_DEADZONE && abs(Y) < JOY_DEADZONE) {
          X = 0;
          Y = 0;
        }
        Gamepad.rxAxis(X);
        Gamepad.ryAxis(Y);
      }

      // Set Dpad
      if (mode == POV) {
        switch (dpad) {
          case DPAD:
            Gamepad.dPad1(setDpad(state_U, state_D, state_L, state_R));
            break;
          case LS:
            if (!DUPE_ASSIGN) {
              Gamepad.xAxis(0);
              Gamepad.yAxis(0);
            }
            if(state_U) Gamepad.yAxis(JOY_ANALOG_MIN);
            if(state_D) Gamepad.yAxis(JOY_ANALOG_MAX);
            if(state_L) Gamepad.xAxis(JOY_ANALOG_MIN);
            if(state_R) Gamepad.xAxis(JOY_ANALOG_MAX);
            break;
          case RS:
            if (!DUPE_ASSIGN) {
              Gamepad.rxAxis(0);
              Gamepad.ryAxis(0);
            }
            if(state_U) Gamepad.ryAxis(JOY_ANALOG_MIN);
            if(state_D) Gamepad.ryAxis(JOY_ANALOG_MAX);
            if(state_L) Gamepad.rxAxis(JOY_ANALOG_MIN);
            if(state_R) Gamepad.rxAxis(JOY_ANALOG_MAX);
            break;
        }
      } else if (mode == BUTTON) {
        setButton(13, state_U);
        setButton(14, state_D);
        setButton(15, state_L);
        setButton(16, state_R);
      }

      // Set button state
      setButton(1, state_X);
      setButton(2, state_O);
      setButton(3, state_Squ);
      setButton(4, state_Tri);
      setButton(5, state_L1);
      setButton(6, state_R1);
      setButton(7, state_L2);
      setButton(8, state_R2);
      setButton(9, state_Sl);
      setButton(10, state_St);
      setButton(11, state_L3);
      setButton(12, state_R3);
      break;
    case KONAMI:
      // KONAMI STATION Mode
      ////////////////////

      if (state_L && state_D && state_R) {
        // pop'n music Mode
        ////////////////////

        setButton(1, state_Tri);
        setButton(2, state_O);
        setButton(3, state_R1);
        setButton(4, state_X);
        setButton(5, state_L1);
        setButton(6, state_Squ);
        setButton(7, state_R2);
        setButton(8, state_U);
        setButton(9, state_L2);
        if (state_Sl) NKROKeyboard.add(KEY_BACKSPACE); else NKROKeyboard.remove(KEY_BACKSPACE);
        // St = Esc
        if (!state_Sl && state_St) NKROKeyboard.add(KEY_ESC); else NKROKeyboard.remove(KEY_ESC);
        // Sl + St = F1
        if (state_Sl && state_St) NKROKeyboard.add(KEY_F1); else NKROKeyboard.remove(KEY_F1);
      } else {
        // IIDX Mode
        ////////////////////

        setButton(1, !state_Sl && state_Squ);
        setButton(2, !state_Sl && state_L1);
        setButton(3, !state_Sl && state_X);
        setButton(4, !state_Sl && state_R1);
        setButton(5, !state_Sl && state_O);
        setButton(6, !state_Sl && state_L2);
        setButton(7, !state_Sl && state_L);

        // SCR Assign to button
        ////////////////////
        setButton(8, state_U);
        setButton(9, state_D);

        // SCR Assign to analog
        ////////////////////
        // if (state_U) {
        //   // Lstick Up
        //   Gamepad.yAxis(0x8000);
        // } else if (state_D) {
        //   // Lstick Down
        //   Gamepad.yAxis(0x7FFF);
        // } else {
        //   // Lstick Center
        //   Gamepad.yAxis(0);
        // }
        // // X-axis centered
        // Gamepad.xAxis(0);

        // START
        setButton(10, state_St);

        // Select + 2 = 11
        setButton(11, state_Sl && state_L1);
        // Select + 4 = 12
        setButton(12, state_Sl && state_R1);
        // Select + 6 = 13
        setButton(13, state_Sl && state_L2);

        // Select + 1 = UP
        if (state_Sl && state_Squ) NKROKeyboard.add(KEY_UP); else NKROKeyboard.remove(KEY_UP);
        // Select + 3 = DOWN
        if (state_Sl && state_X) NKROKeyboard.add(KEY_DOWN); else NKROKeyboard.remove(KEY_DOWN);
        // Select + 5 = LEFT
        if (state_Sl && state_O) NKROKeyboard.add(KEY_LEFT); else NKROKeyboard.remove(KEY_LEFT);
        // Select + 7 = RIGHT
        if (state_Sl && state_L) NKROKeyboard.add(KEY_RIGHT); else NKROKeyboard.remove(KEY_RIGHT);
      }
      break;
    default:
      // Irregular
      ////////////////////
      Gamepad.releaseAll();
      NKROKeyboard.releaseAll();
  }

  // Send
  ////////////////////
  Gamepad.write();
  NKROKeyboard.send();
}
