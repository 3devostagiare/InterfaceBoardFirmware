#include <stdint.h>
#include <avr/wdt.h>
#include <util/crc16.h>
#include "TwoWire.h"
#include "BaseProtocol.h"

static uint8_t calcCrc(uint8_t *data, uint8_t len) {
	uint8_t crc = 0;
	for (uint8_t i = 0; i < len; ++i)
		crc = _crc8_ccitt_update(crc, data[i]);
	return crc;
}

static int handleGeneralCall(uint8_t *data, uint8_t len, uint8_t /* maxLen */) {
	if (len >= 1 && data[0] == GeneralCallCommands::RESET) {
		wdt_enable(WDTO_15MS);
		while(true) /* wait */;

	} else if (len >= 1 && data[0] == GeneralCallCommands::RESET_ADDRESS) {
		TwoWireResetDeviceAddress();
	}
	return 0;
}

int TwoWireCallback(uint8_t address, uint8_t *data, uint8_t len, uint8_t maxLen) {
	if (address == 0)
		return handleGeneralCall(data, len, maxLen);

	// Check that there is at least room for a status byte and a CRC
	if (maxLen < 2)
		return 0;

	if (len < 2) {
		data[0] = Status::INVALID_TRANSFER;
		len = 1;
	} else {
		uint8_t crc = calcCrc(data, len);
		if (crc != 0) {
			data[0] = Status::INVALID_CRC;
			len = 1;
		} else {
			// CRC checks out, process a command
			cmd_result res = processCommand(data[0], data + 1, len - 2, maxLen - 2);
			if (res.status == Status::NO_REPLY)
				return 0;

			data[0] = res.status;
			len = res.len + 1;
		}
	}

	data[len] = calcCrc(data, len);
	++len;

	return len;
}


