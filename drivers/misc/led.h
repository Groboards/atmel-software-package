/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2015, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 * \file
 *
 *  \section Purpose
 *
 *  Small set of functions for simple and portable LED usage.
 *
 *  \section Usage
 *
 *  -# Configure one or more LEDs using led_configure and
 *     LED_ConfigureAll.
 *  -# Set, clear and toggle LEDs using led_set, led_clear and
 *     led_toggle.
 *
 *  LEDs are numbered starting from 0; the number of LEDs depend on the
 *  board being used. All the functions defined here will compile properly
 *  regardless of whether the LED is defined or not; they will simply
 *  return 0 when a LED which does not exist is given as an argument.
 *  Also, these functions take into account how each LED is connected on to
 *  board; thus, \ref led_set might change the level on the corresponding pin
 *  to 0 or 1, but it will always light the LED on; same thing for the other
 *  methods.
 */

#ifndef _LED_
#define _LED_

#include <stdint.h>
#include "peripherals/pio.h"

//------------------------------------------------------------------------------
//         Global Functions
//------------------------------------------------------------------------------

/**
 *  Configures the pin associated with the given LED number. If the LED does
 *  not exist on the board, the function does nothing.
 *  \param leds   Array of led pins (led driver keep a pointer to leds array)
 *  \param count  Number of the LED to configure.
 *  \return a bitfield of initialiased LEDs
 */
extern uint32_t led_configure(struct _pin *leds, uint32_t count);

/**
 *  Turns the given LED on if it exists; otherwise does nothing.
 *  \param dwLed  Number of the LED to turn on.
 *  \return 1 if the LED has been turned on; 0 otherwise.
 */
extern uint32_t led_set(uint32_t led);

/**
 *  Turns a LED off.
 *
 *  \param dwLed  Number of the LED to turn off.
 *  \return 1 if the LED has been turned off; 0 otherwise.
 */
extern uint32_t led_clear(uint32_t led);

/**
 *  Toggles the current state of a LED.
 *
 *  \param dwLed  Number of the LED to toggle.
 *  \return 1 if the LED has been toggled; otherwise 0.
 */
extern uint32_t led_toggle(uint32_t led);

#endif				/* #ifndef LED_H */
