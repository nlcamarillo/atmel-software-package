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
 *  \page getting-started Getting Started with sama5d4x Microcontrollers
 *
 *  \section Purpose
 *
 *  The Getting Started example will help new users get familiar with Atmel's
 *  sama5d4x microcontroller. This basic application shows the startup
 *  sequence of a chip and how to use its core peripherals.
 *
 *  \section Requirements
 *
 *  This package can be used with SAMA5D4-EK and SAMA5D4-XULT.
 *
 *  \section Description
 *
 *  The demonstration program makes two LEDs on the board blink at a fixed rate.
 *  This rate is generated by using Time tick timer. The blinking can be stopped
 *  using two buttons (one for each LED). If there is no enough buttons on board, please
 *  type "1" or "2" in the terminal application on PC to control the LEDs
 *  instead.
 *
 *  \section Usage
 *
 *  -# Build the program and download it inside the evaluation board. Please
 *     refer to the
 *     <a href="http://www.atmel.com/dyn/resources/prod_documents/6421B.pdf">
 *     SAM-BA User Guide</a>, the
 *     <a href="http://www.atmel.com/dyn/resources/prod_documents/doc6310.pdf">
 *     GNU-Based Software Development</a>
 *     application note or to the
 *     <a href="ftp://ftp.iar.se/WWWfiles/arm/Guides/EWARM_UserGuide.ENU.pdf">
 *     IAR EWARM User Guide</a>,
 *     depending on your chosen solution.
 *  -# On the computer, open and configure a terminal application
 *     (e.g. HyperTerminal on Microsoft Windows) with these settings:
 *    - 115200 bauds
 *    - 8 bits of data
 *    - No parity
 *    - 1 stop bit
 *    - No flow control
 *  -# Start the application.
 *  -# Two LEDs should start blinking on the board. In the terminal window, the
 *     following text should appear (values depend on the board and chip used):
 *     \code
 *      -- Getting Started Example xxx --
 *      -- SAMxxxxx-xx
 *      -- Compiled: xxx xx xxxx xx:xx:xx --
 *     \endcode
 *  -# Pressing and release button 1 or type "1" in the terminal application on
 *     PC should make the first LED stop & restart blinking.
 *     Pressing and release button 2 or type "2" in the terminal application on
 *     PC should make the other LED stop & restart blinking.
 *
 *  \section References
 *  - getting-started/main.c
 *  - pio.h
 *  - pio_it.h
 *  - led.h
 *  - trace.h
 */

/** \file
 *
 *  This file contains all the specific code for the getting-started example.
 *
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"
#include "chip.h"
#include "trace.h"
#include "compiler.h"
#include "timer.h"

#include "irq/irq.h"
#include "gpio/pio.h"
#include "peripherals/pmc.h"
#include "peripherals/tcd.h"

#include "led/led.h"


#include <stdbool.h>
#include <stdio.h>

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/

/** Delay for pushbutton debouncing (in milliseconds). */
#define DEBOUNCE_TIME       500

struct _tcd_desc tc = {
	.addr = TC0,
	.channel = 0,
};

// #define ISOKO

#ifdef ISOKO
	//todo: for oko, sw_1 is on PIOBU01, sw_2 is on PIOBU00
	#define PIN_SW_1 { PIO_GROUP_B, PIO_PB9, PIO_INPUT, PIO_CFG_PB }
	#define PIN_ATMEL_RED { PIO_GROUP_B, PIO_PB1, PIO_OUTPUT_1, PIO_OPENDRAIN }
#else
	#define PIN_SW_1 { PIO_GROUP_B, PIO_PB9, PIO_INPUT, PIO_CFG_PB }
	#define PIN_ATMEL_RED { PIO_GROUP_B, PIO_PB6, PIO_OUTPUT_1, PIO_OPENDRAIN }
#endif

//copied from board def
#define PINS_PUSHBUTTONS { PIN_SW_1 }
#define NUM_LEDS 1

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/


/** Pushbutton \#1 pin instance. */
static const struct _pin button_pins[] = PINS_PUSHBUTTONS;

volatile bool led_status[NUM_LEDS];

/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/

/**
 *  \brief Process Buttons Events
 *
 *  Change active states of LEDs when corresponding button events happened.
 */
static void process_button_evt(void)
{
	led_status[0] = !led_status[0];
	if (!led_status[0]) {
		led_clear(0);
	}
}

/**
 *  \brief Handler for Buttons rising edge interrupt.
 *
 *  Handle process led1 status change.
 */
static void pio_handler(uint32_t group, uint32_t status, void* user_arg)
{
	/* unused */
	(void)user_arg;
	process_button_evt();
}

/**
 *  \brief Configure the Pushbuttons
 *
 *  Configure the PIO as inputs and generate corresponding interrupt when
 *  pressed or released.
 */
static void configure_buttons(void)
{
	/* Adjust debounce filter parameters, use 10 Hz filter */
	pio_set_debounce_filter(10);

	/* Configure PIO */
	pio_configure(&button_pins[0], 1);

	/* Initialize interrupt with its handlers */
	pio_add_handler_to_group(button_pins[0].group,
					button_pins[0].mask, pio_handler, NULL);

	/* Enable interrupts */
	pio_enable_it(button_pins);
}

/*----------------------------------------------------------------------------
 *        Global functions
 *----------------------------------------------------------------------------*/

/**
 *  \brief getting-started Application entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	led_status[0] = true;

	printf("Configure buttons with debouncing.\n\r");
	configure_buttons();
	printf("Use push buttons\n\r");

	printf("LED 0 uses softpack timer functions\r\n");

	while (1) {

		/* Wait for LED to be active */
		while (!led_status[0]);

		/* Toggle LED state if active */
		if (led_status[0]) {
			led_toggle(0);
			printf("0 ");
		}

		/* Wait for 250ms (4Hz) */
		msleep(250);
	}
}
