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

#include "board.h"
#include "chip.h"

#include "peripherals/pio.h"
#include "peripherals/pmc.h"
#include "peripherals/aic.h"

#include "peripherals/spid.h"
#include "peripherals/spi.h"

#include "memories/at25.h"

#include "trace.h"
#include "compiler.h"
#include "math.h"

#include <stdio.h>
#include <assert.h>

#define MODE_3B_MAX_SIZE    16*1024*1024

/** Array of recognized serial firmware dataflash chips. */
static const struct _at25_desc at25_devices[] = {
	/* name,        Jedec ID,       size,  page size, block size, block erase command */
	{"AT25DF041A" , 0x0001441F,      512 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT25DF161"  , 0x0002461F, 2 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT26DF081A" , 0x0001451F, 1 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT26DF0161" , 0x0000461F, 2 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT26DF161A" , 0x0001461F, 2 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT25DF321"  , 0x0000471F, 4 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_64K},
	{"AT25DF321A" , 0x0001471F, 4 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_64K},
	{"AT25DF512B" , 0x0001651F,       64 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT25DF512B" , 0x0000651F,       64 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT25DF021"  , 0x0000431F,      256 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"AT26DF641"  , 0x0000481F, 8 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	/* Manufacturer: ST */
	{"M25P05"     , 0x00102020,       64 * 1024, 256, 32 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P10"     , 0x00112020,      128 * 1024, 256, 32 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P20"     , 0x00122020,      256 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P40"     , 0x00132020,      512 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P80"     , 0x00142020, 1 * 1024 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P16"     , 0x00152020, 2 * 1024 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P32"     , 0x00162020, 4 * 1024 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	{"M25P64"     , 0x00172020, 8 * 1024 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	/* Manufacturer: Windbond */
	{"W25X10"     , 0x001130EF,      128 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"W25X20"     , 0x001230EF,      256 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"W25X40"     , 0x001330EF,      512 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"W25X80"     , 0x001430EF, 1 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"W25Q256"    , 0x001940EF, 32* 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	/* Manufacturer: Macronix */
	{"MX25L512"   , 0x001020C2,       64 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"MX25L3205"  , 0x001620C2, 4 * 1024 * 1024, 256, 64 * 1024, AT25_BLOCK_ERASE_64K},
	{"MX25L6405"  , 0x001720C2, 8 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"MX25L8005"  , 0x001420C2,     1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	/* Other */
	{"SST25VF040" , 0x008D25BF,      512 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"SST25VF080" , 0x008E25BF, 1 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"SST25VF032" , 0x004A25BF, 4 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	{"SST25VF064" , 0x004B25BF, 8 * 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K},
	/* Manufacturer: Micron */
	{"N25Q256"    , 0x0019BA20, 32* 1024 * 1024, 256,  4 * 1024, AT25_BLOCK_ERASE_4K}
};

static uint32_t _at25_compute_addr(struct _at25* at25, uint8_t* cmd,
				   uint32_t addr)
{
	switch (at25->addressing) {
	case AT25_ADDRESS_4_BYTES:
		cmd[0] = (addr & 0xFF000000) >> 24;
		cmd[1] = (addr & 0x00FF0000) >> 16;
		cmd[2] = (addr & 0x0000FF00) >> 8;
		cmd[3] = (addr & 0x000000FF);
		return 4;
	case AT25_ADDRESS_3_BYTES:
	default:
		cmd[0] = (addr & 0x00FF0000) >> 16;
		cmd[1] = (addr & 0x0000FF00) >> 8;
		cmd[2] = (addr & 0x000000FF);
		return 3;
	}
}

static void _at25_send_write_cmd(struct _at25* at25, uint32_t addr)
{
	uint8_t cmd[5];
	uint8_t dummy_byte = 0;

	struct _buffer out = {
		.data = cmd,
		.size = 1
	};

	if (at25_man_id(at25) == SST_SPI_FLASH)
	{
		cmd[0] = AT25_SEQUENTIAL_PROGRAM_1;
		dummy_byte = 1;
	} else {
		cmd[0] = AT25_BYTE_PAGE_PROGRAM;
	}
	out.size += _at25_compute_addr(at25, &cmd[1], addr);
	out.size += dummy_byte ? 1 : 0;
	spid_transfert(at25->spid, 0, &out, SPID_NO_CALLBACK, 0);
}

static uint32_t _at25_check_writable(struct _at25* at25)
{
	uint32_t status;
	status = at25_check_status(at25,
				   AT25_STATUS_RDYBSY_BUSY | AT25_STATUS_SWP);
	if (status & AT25_STATUS_RDYBSY_BUSY) {
		trace_debug("Device %s is busy\r\n", at25->desc->name);
		return AT25_ERROR_BUSY;
	}
	if (status & AT25_STATUS_SWP) {
		trace_error("Device %s is write protected\r\n",
			    at25->desc->name);
		return AT25_ERROR_PROTECTED;
	}
	return AT25_SUCCESS;
}

static void _at25_enable_write(struct _at25* at25)
{
	spid_begin_transfert(at25->spid);
	uint8_t opcode = AT25_WRITE_ENABLE;
	struct _buffer out = {
		.data = &opcode,
		.size = 1
	};
	spid_transfert(at25->spid, 0, &out, spid_finish_transfert_callback, 0);
}

static void _at25_disable_write(struct _at25* at25)
{
	spid_begin_transfert(at25->spid);
	uint8_t opcode = AT25_WRITE_DISABLE;
	struct _buffer out = {
		.data = &opcode,
		.size = 1
	};
	spid_transfert(at25->spid, 0, &out, spid_finish_transfert_callback, 0);
}

static uint32_t _at25_write_status(struct _at25* at25, uint8_t value)
{
	uint32_t status = 0;
	uint8_t cmd[2] ={AT25_WRITE_STATUS, value};
	struct _buffer out = {
		.data = cmd,
		.size = 2
	};

	_at25_enable_write(at25);

	spid_begin_transfert(at25->spid);
	status = spid_transfert(at25->spid, 0, &out,
				spid_finish_transfert_callback, 0);
	_at25_disable_write(at25);
	if (status) {
		return AT25_ERROR_SPI;
	}
	return AT25_SUCCESS;
}

static uint32_t _at25_compute_read_cmd(struct _at25* at25, uint8_t* cmd,
				   uint32_t addr)
{
	struct _buffer out = {
		.data = cmd,
		.size = 1
	};
	cmd[0] = AT25_READ_ARRAY;
	out.size += _at25_compute_addr(at25, &cmd[1], addr);
	out.size += 1;
	return out.size;
}

static void _at25_set_addressing(struct _at25* at25)
{
	assert(at25->desc);

	if (at25->desc->size > MODE_3B_MAX_SIZE) {
		at25->addressing = AT25_ADDRESS_4_BYTES;
	} else {
		at25->addressing = AT25_ADDRESS_3_BYTES;
	}
}

uint32_t at25_check_status(struct _at25* at25, uint32_t mask)
{
	uint32_t status = at25_read_status(at25);
	if (status & mask) {
		return status & mask;
	}
	return AT25_SUCCESS;
}

void at25_wait(struct _at25* at25)
{
	trace_debug("Device in busy status, Waiting...\r\n");
	while (at25_check_status(at25, AT25_STATUS_RDYBSY_BUSY));
}

uint32_t at25_configure(struct _at25* at25, struct _spi_desc* spid)
{
	at25->spid = spid;
	spid_configure(spid);
	uint32_t jedec_id = at25_read_jedec_id(at25);
	at25_find_device(at25, jedec_id);
	if (!at25->desc) {
		return AT25_DEVICE_NOT_SUPPORTED;
	}
	_at25_set_addressing(at25);
	return AT25_SUCCESS;
}

const struct _at25_desc* at25_find_device(struct _at25* at25, uint32_t jedec_id)
{
	uint32_t i = 0;
	assert(at25);

	/* Search if device is recognized */
	at25->desc = 0;
	for (i = 0; i < ARRAY_SIZE(at25_devices) && !(at25->desc); ++i) {
		if ((jedec_id) == (at25_devices[i].jedec_id)) {
			at25->desc = (struct _at25_desc*)&(at25_devices[i]);
		}
	}
	return at25->desc;
}

uint32_t at25_read_jedec_id(struct _at25* at25)
{
	assert(at25);
	assert(at25->spid);
	uint32_t jedec;
	uint8_t opcode = AT25_READ_JEDEC_ID;
	struct _buffer in = {
		.data = (uint8_t*)&jedec,
		.size = sizeof(jedec)
	};
	struct _buffer out = {
		.data = &opcode,
		.size = sizeof(opcode)
	};

	spid_begin_transfert(at25->spid);
	spid_transfert(at25->spid, &in, &out,
		       spid_finish_transfert_callback, 0);
	spid_wait_transfert(at25->spid);

	return jedec;
}

uint32_t at25_read_status(struct _at25* at25)
{
	assert(at25);
	assert(at25->spid);
	uint8_t status;
	uint8_t opcode = AT25_READ_STATUS;
		struct _buffer in = {
		.data = &status,
		.size = sizeof(status)
	};
	struct _buffer out = {
		.data = &opcode,
		.size = sizeof(opcode)
	};

	spid_begin_transfert(at25->spid);
	spid_transfert(at25->spid, &in, &out,
		       spid_finish_transfert_callback, 0);
	spid_wait_transfert(at25->spid);

	return status;
}

uint32_t at25_protect(struct _at25* at25)
{
	assert(at25);
	assert(at25->spid);

	/* Perform a global protect command */
	_at25_write_status(at25, 0x7F);
	return AT25_SUCCESS;
}

uint32_t at25_unprotect(struct _at25* at25)
{
	assert(at25);
	assert(at25->spid);

	/* Get the status register value to check the current protection */
	uint32_t status = at25_read_status(at25);
	if ((status & AT25_STATUS_SWP) == AT25_STATUS_SWP_PROTNONE) {
		return 0;
	}

	/* Perform a global unprotect command */
	_at25_write_status(at25, 0x0);

	_at25_disable_write(at25);
	/* Check the new status */
	if (at25_check_status(at25, AT25_STATUS_SPRL | AT25_STATUS_SWP)) {
		return AT25_ERROR_PROTECTED;
	}
	else {
		return AT25_SUCCESS;
	}
}

void at25_print_device_info(struct _at25* at25)
{
	assert(at25);
	assert(at25->spid);

	uint32_t device_info = at25_read_jedec_id(at25);

	device_info = BIG_ENDIAN_TO_HOST(device_info);

	printf("Device info:\r\n");
	printf("\t- Manufacturer ID:\t\t0x%X "
	       "(Should be equal to 0x1F)\r\n",
	       (unsigned int)(device_info & 0xFF000000) >> 24);
	printf("\t- Device Family Code:\t\t0x%X\r\n",
	       (unsigned int)(device_info & 0x00E00000) >> 21);
	printf("\t- Device Density Code:\t\t0x%X\r\n",
	       (unsigned int)(device_info & 0x001F0000) >> 16);
	printf("\t- Device Sub Code:\t\t0x%X\r\n",
	       (unsigned int)(device_info & 0xE000) >> 13);
	printf("\t- Device Product Version:\t0x%X\r\n",
	       (unsigned int)(device_info & 0x1F00) >> 8);
}

uint32_t at25_is_busy(struct _at25* at25)
{
	return at25_check_status(at25, AT25_STATUS_RDYBSY_BUSY);
}

uint32_t at25_read(struct _at25* at25, uint32_t addr,
		   uint8_t* data, uint32_t length)
{
	if (addr > at25->desc->size) {
		return AT25_ADDR_OOB;
	}

	trace_debug("Start flash read at address: 0x%08X\r\n",
		    (unsigned int)(addr & (at25->desc->size - 1)));

	assert(at25);
	assert(at25->spid);

	uint32_t status = 0;

	if (at25_is_busy(at25)) {
		return AT25_ERROR_BUSY;
	}

	uint8_t cmd[5];
	uint32_t cmd_size = _at25_compute_read_cmd(at25, cmd, addr);
	struct _buffer out = {
		.data = (uint8_t*)&cmd,
		.size = cmd_size
	};
	struct _buffer in = {
		.data = data,
		.size = length
	};

	spid_begin_transfert(at25->spid);
	status = spid_transfert(at25->spid, &in, &out,
				spid_finish_transfert_callback, 0);
	spid_wait_transfert(at25->spid);
	if(status) {
		return AT25_ERROR_SPI;
	}
	return AT25_SUCCESS;
}

uint32_t at25_erase_chip(struct _at25* at25)
{
	trace_debug("Start flash reset all block will be erased\r\n");

	assert(at25);
	assert(at25->spid);

	uint32_t status = _at25_check_writable(at25);
	if (status) {
		return status;
	}

	uint8_t cmd = AT25_CHIP_ERASE_1;
	struct _buffer out = {
		.data = &cmd,
		.size = 1
	};

	_at25_enable_write(at25);
	spid_begin_transfert(at25->spid);
	status = spid_transfert(at25->spid, 0, &out,
		       spid_finish_transfert_callback, 0);
	spid_wait_transfert(at25->spid);
	if (status) {
		return AT25_ERROR_SPI;
	}
	_at25_disable_write(at25);
	return AT25_SUCCESS;
}

uint32_t at25_erase_block(struct _at25* at25, uint32_t addr,
			  uint32_t erase_type)
{
	trace_debug("Start flash erase at address: 0x%08X\r\n",
		    (unsigned int)(addr & (at25->desc->size - 1)));

	assert(at25);
	assert(at25->spid);
	if (addr > at25->desc->size) {
		return AT25_ADDR_OOB;
	}

	uint32_t status = _at25_check_writable(at25);
	if (status) {
		return status;
	}

	uint8_t cmd[5];

	struct _buffer out = {
		.data = cmd,
		.size = 1
	};

	uint32_t applied_erase = erase_type;
	uint32_t dev_max_erase = at25->desc->block_erase_cmd;

	uint32_t num_pass = 1;

	switch(erase_type) {
	case AT25_BLOCK_ERASE_64K:
		if (dev_max_erase != AT25_BLOCK_ERASE_64K) {
			trace_debug("Device %s does not support 64k erase\r\n",
				    at25->desc->name);
			trace_debug("Falling back to 32k erase\r\n");
			applied_erase = AT25_BLOCK_ERASE_32K;
			num_pass <<= 1;
		}
	case AT25_BLOCK_ERASE_32K:
		if (dev_max_erase != AT25_BLOCK_ERASE_32K ||
			dev_max_erase != AT25_BLOCK_ERASE_64K) {
			trace_debug("Device %s does not support 32k and 64k "
				    "erase\r\n", at25->desc->name);
			trace_debug("Falling back to 4k erase\r\n");
			applied_erase = AT25_BLOCK_ERASE_4K;
			num_pass <<= 3;
		}
	case AT25_BLOCK_ERASE_4K:
		break;
	default:
		return AT25_ERROR_PROGRAM;
	}
	cmd[0] = applied_erase;

	uint32_t i = 0;
	for (i = 0; i < num_pass; ++i) {
		trace_debug("\r\nClearing block at addr 0x%x\r\n",
			    (unsigned int)((i*4*1024)+addr));

		_at25_enable_write(at25);
		out.size = 1 + _at25_compute_addr(at25, &cmd[1],
						  (i*4*1024)+addr);
		spid_begin_transfert(at25->spid);
		spid_transfert(at25->spid, 0, &out,
			       spid_finish_transfert_callback, 0);
		spid_wait_transfert(at25->spid);
		if (at25_check_status(at25, AT25_STATUS_EPE)) {
			return AT25_ERROR_PROGRAM;
		}
		at25_wait(at25);
	}
	_at25_disable_write(at25);
	return AT25_SUCCESS;
}

uint32_t at25_write(struct _at25* at25, uint32_t addr,
		    const uint8_t* data, uint32_t length)
{
	if (addr > at25->desc->size) {
		return AT25_ADDR_OOB;
	}

	trace_debug("Start flash write at address: 0x%08X\r\n",
		    (unsigned int)(addr & (at25->desc->size - 1)));
	assert(at25);
	assert(at25->spid);
	assert(data);

	uint32_t status = _at25_check_writable(at25);
	if (status) {
		return status;
	}

	/* Retrieve device page size */
	uint32_t page_size = _at25_page_size(at25);

	struct _buffer out = {
		.data = (uint8_t*)data,
		.size = length
	};

	while(length > 0) {
		/* Compute number of bytes to program in page */
		uint32_t write_size;
		write_size = min(length, page_size - (addr % page_size));

		at25_wait(at25);

		_at25_enable_write(at25);

		spid_begin_transfert(at25->spid);
		_at25_send_write_cmd(at25, addr);
		out.size = write_size;
		status = spid_transfert(at25->spid, 0, &out,
					spid_finish_transfert_callback, 0);
		if (status) {
			return AT25_ERROR_SPI;
		}
		spid_wait_transfert(at25->spid);
		if (at25_check_status(at25, AT25_STATUS_EPE)) {
			return AT25_ERROR_PROGRAM;
		}

		length -= write_size;
		out.data += write_size;
		addr += write_size;
	}

	_at25_disable_write(at25);

	return AT25_SUCCESS;
}
