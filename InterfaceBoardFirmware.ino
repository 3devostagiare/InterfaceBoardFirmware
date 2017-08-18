/*
 * Copyright (C) 2017 3devo (http://3devo.eu)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Hardware.h"
#include "Arduino.h"
#include "TwoWire.h"
#include "BaseProtocol.h"
#include <util/atomic.h>

const uint16_t hopper_threshold = 20;
uint16_t measurement[2];

//#define ENABLE_SERIAL

struct Commands {
  enum {
    GET_LAST_MEASUREMENT = 0x80,
  };
};

cmd_result processCommand(uint8_t cmd, uint8_t * /*datain*/, uint8_t len, uint8_t *dataout, uint8_t maxLen) {
  switch (cmd) {
    case Commands::GET_LAST_MEASUREMENT:
      if (len != 0 || maxLen < 4)
        return cmd_result(Status::INVALID_ARGUMENTS);
      dataout[0] = measurement[0] >> 8;
      dataout[1] = measurement[0];
      dataout[2] = measurement[1] >> 8;
      dataout[3] = measurement[1];
      return cmd_result(Status::COMMAND_OK, 4);
    default:
      return cmd_result(Status::COMMAND_NOT_SUPPORTED);
  }
}

void start_display()
{
  // This pin has a pullup to 3v3, so the display comes out of
  // reset as soon as the 3v3 is powered up. To prevent that, pull
  // it low now.
  digitalWrite(RES_Display, LOW);
  pinMode(RES_Display, OUTPUT);

  // Reset sequence for the display according to datasheet: Enable
  // 3v3 logic supply, then release the reset, then powerup the
  // boost converter for LED power. This is a lot slower than
  // possible according to the datasheet.
  pinMode(EN_3V3, OUTPUT);
  digitalWrite(EN_3V3, HIGH);

  delay(1);
  // Switch to input to let external 3v3 pullup work instead of
  // making it high (which would be 5v);
  pinMode(RES_Display, INPUT);

  delay(1);
  pinMode(EN_Boost, OUTPUT);
  digitalWrite(EN_Boost, HIGH);

  delay(5);

  #ifdef ENABLE_SERIAL
  Serial.println("Display turned on");
  #endif
}

void measure_hopper()
{
#ifndef ENABLE_SERIAL // Serial reuses the H_sens pin
  digitalWrite(H_Led, LED_ON);
  delay(10);
  uint16_t on = analogRead(H_Sens_ADC_Channel);

  digitalWrite(H_Led, LED_OFF);
  delay(10);
  uint16_t off = analogRead(H_Sens_ADC_Channel);

  // Store the raw measurements to be read through I²C
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    measurement[0] = on;
    measurement[1] = off;
  }

  // Lower reading means more light
  if (on < off && (off - on) > hopper_threshold)
    digitalWrite(H_Out, HOPPER_EMPTY);
  else
    digitalWrite(H_Out, HOPPER_FULL);
#endif
}


void setup()
{
  #ifdef ENABLE_SERIAL
  Serial.begin(1000000);
  Serial.println("Starting");
  #endif

  pinMode(H_Led, OUTPUT);
  pinMode(H_Out, OUTPUT);
  #ifndef ENABLE_SERIAL // Serial reuses the H_sens pin
  pinMode(H_Sens, INPUT);
  #endif

  TwoWireInit(/* useInterrupts */ true, I2C_ADDRESS);

  start_display();
}

void loop()
{
  measure_hopper();
}
