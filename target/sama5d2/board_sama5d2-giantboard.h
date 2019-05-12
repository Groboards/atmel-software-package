/**
 * \page sama5d2_giant_board_desc sama5d2-giantboard - Board Description
 *
 * \section Purpose
 *
 * This file is dedicated to describe the sama5d2-giantboard.
 *
 * \section Contents
 *
 *  - sama5d2-XULT
 *  - For sama5d2-XULT information, see \subpage sama5d2_xult_board_info.
 *  - For operating frequency information, see \subpage sama5d2_xult_opfreq.
 *  - For using portable PIO definitions, see \subpage sama5d2_xult_piodef.
 *  - For on-board memories, see \subpage sama5d2_xult_mem.
 *  - Several USB definitions are included here, see \subpage sama5d2_xult_usb.
 *  - For External components, see \subpage sama5d2_xult_extcomp.
 *  - For Individual chip definition, see \subpage sama5d2_xult_chipdef.
 *
 * To get more software details and the full list of parameters related to the
 * sama5d2-XULT board configuration, please have a look at the source file:
 * \ref board.h\n
 *
 * \section Usage
 *
 *  - The code for booting the board is provided by board_cstartup_xxx.c and
 *    board_lowlevel.c.
 *  - For using board PIOs, board characteristics (clock, etc.) and external
 *    components, see board.h.
 *  - For manipulating memories, see board_memories.h.
 *
 * This file can be used as a template and modified to fit a custom board, with
 * specific PIOs usage or memory connections.
 */

/**
 *  \file board.h
 *
 *  Definition of sama5d2-giantboard
 *  characteristics, sama5d4-dependant PIOs and external components interfacing.
 */

#ifndef _BOARD_D2_H
#define _BOARD_D2_H

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "chip.h"
#include "peripherals/bus.h"

#include "board_support.h"


/*----------------------------------------------------------------------------
 *        HW BOARD Definitions
 *----------------------------------------------------------------------------*/

/* Uncomment next line to use early prototype versions of SAMA5D2-XULT. */
//#define CONFIG_BOARD_SAMA5D2_XPLAINED_PROTO

/**
 * \page sama5d2_xult_board_info "sama5d2-XULT - Board informations"
 * This page lists several definition related to the board description.
 *
 * \section Definitions
 * - \ref BOARD_NAME
 */

/** Name of the board */
#define BOARD_NAME "sama5d2-giantboard"

/*----------------------------------------------------------------------------*/

/** Frequency of the board main clock oscillator */
#define BOARD_MAIN_CLOCK_EXT_OSC 24000000

/** PMC PLLA configuration */
#define BOARD_PMC_PLLA_MUL 40
#define BOARD_PMC_PLLA_DIV 1

/** /def Definition of DDRAM's type */
#define BOARD_DDRAM_TYPE W971GG6SB

/** \def Board DDR memory size in bytes */
#define BOARD_DDR_MEMORY_SIZE 128*1024*1024

/** \def Board System timer */
#define BOARD_TIMER_TC      TC1
#define BOARD_TIMER_CHANNEL 0

/* =================== PIN CONSOLE definition ================== */

/** CONSOLE pin definition: use UART1 IOSET1 */
/* Note that these definitions could be omitted if the console is configured in
 * the active boot config word. */
#define BOARD_CONSOLE_ADDR     UART1
#define BOARD_CONSOLE_BAUDRATE 115200
#define BOARD_CONSOLE_TX_PIN   PIN_UART1_TXD_IOS1
#define BOARD_CONSOLE_RX_PIN   PIN_UART1_RXD_IOS1

/* =================== PIN LED definition ====================== */

/* LED index */
#define LED_GREEN 0

/** LED #0 pin definition (Green). */
#define PIN_LED_0 { PIO_GROUP_A, PIO_PA6, PIO_OUTPUT_1, PIO_DEFAULT }

/** List of all LEDs definitions. */
#define PINS_LEDS { PIN_LED_0 }

#define NUM_LEDS  1

/* ================== TWI bus definition ====================== */

#define BOARD_TWI_BUS0      TWI0
#define BOARD_TWI_BUS0_FREQ 400000
#define BOARD_TWI_BUS0_PINS PINS_TWI0_IOS1
#define BOARD_TWI_BUS0_MODE BUS_TRANSFER_MODE_DMA

#define BOARD_TWI_BUS1      TWI1
#define BOARD_TWI_BUS1_FREQ 400000
#define BOARD_TWI_BUS1_PINS PINS_TWI1_IOS2
#define BOARD_TWI_BUS1_MODE BUS_TRANSFER_MODE_DMA

/* ================== SPI bus definition ====================== */

#define BOARD_SPI_BUS0       SPI0
#define BOARD_SPI_BUS0_PINS  PINS_SPI0_IOS1
#define BOARD_SPI_BUS0_MODE  BUS_TRANSFER_MODE_DMA

/* ================== ACT8945A PMIC definition ====================== */

#define BOARD_ACT8945A_TWI_BUS    BUS(BUS_TYPE_I2C, 1)
#define BOARD_ACT8945A_TWI_ADDR   0x5b
#define BOARD_ACT8945A_PIN_CHGLEV { PIO_GROUP_A, PIO_PA12, PIO_OUTPUT_0, PIO_PULLUP }
#define BOARD_ACT8945A_PIN_IRQ    { PIO_GROUP_B, PIO_PB13, PIO_INPUT, PIO_PULLUP | PIO_IT_FALL_EDGE }
#ifndef CONFIG_BOARD_SAMA5D2_XPLAINED_PROTO
#define BOARD_ACT8945A_PIN_LBO    { PIO_GROUP_C, PIO_PC8, PIO_INPUT, PIO_PULLUP | PIO_IT_FALL_EDGE }
#else
/* on prototype board, ACT8945A_LBO shares a pin with ACT8945A_IRQ */
#define BOARD_ACT8945A_PIN_LBO    { PIO_GROUP_B, PIO_PB13, PIO_INPUT, PIO_PULLUP | PIO_IT_FALL_EDGE }
#endif

/* ================== PIN USB definition ======================= */

/** USB VBus pin */
#define PIN_USB_VBUS \
	{ PIO_GROUP_A, PIO_PA31, PIO_INPUT, PIO_DEBOUNCE | PIO_IT_BOTH_EDGE }

/** USB OverCurrent detection*/
#define PIN_USB_OVCUR \
	{ PIO_GROUP_A, PIO_PA29, PIO_INPUT, PIO_DEFAULT }

/** USB Power Enable A, Active high */
#define PIN_USB_POWER_ENA \
	{ PIO_GROUP_B, PIO_PB9, PIO_OUTPUT_0, PIO_DEFAULT }

/** USB Power Enable B, Active high  */
#define PIN_USB_POWER_ENB \
	{ PIO_GROUP_B, PIO_PB10, PIO_OUTPUT_0, PIO_DEFAULT }

/**
 * USB attributes configuration descriptor (bus or self powered,
 * remote wakeup)
 */
#define BOARD_USB_BMATTRIBUTES \
	USBConfigurationDescriptor_SELFPOWERED_NORWAKEUP

/* =================== SDMMC device definition ==================== */

#define BOARD_SDMMC1_PINS { PIN_SDMMC1_CD_IOS1, PIN_SDMMC1_CK_IOS1,\
                            PIN_SDMMC1_CMD_IOS1, PINS_SDMMC1_DATA4B_IOS1 }

#define BOARD_SDMMC1_CAPS0 (SDMMC_CA0R_V33VSUP | \
                            SDMMC_CA0R_SLTYPE_REMOVABLECARD)

#endif /* #ifndef _BOARD_D2_H */
