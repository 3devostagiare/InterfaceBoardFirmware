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

#include "Arduino.h"

static const int H_Led = PIN_A7;
static const int H_Sens = PIN_A1;
static const int H_Sens_ADC_Channel = 1;
static_assert(analogInputToDigitalPin(H_Sens_ADC_Channel) == H_Sens, "Hopper sensor ADC channel mismatch");
static const int H_Out = PIN_A0;

static const int EN_Boost = PIN_A3;
static const int EN_3V3 = PIN_A2;
static const int RES_Display = PIN_B0;

static const int ENC_B = PIN_B1;
static const int ENC_A = PIN_B2;
static const int ENC_SW = PIN_A5;

static const int SCL = PIN_A4;
static const int SDA = PIN_A6;

static const int LED_ON = HIGH;
static const int LED_OFF = LOW;

static const int HOPPER_FULL = HIGH;
static const int HOPPER_EMPTY = LOW;

static const uint8_t I2C_ADDRESS = 8;
