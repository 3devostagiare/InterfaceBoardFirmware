#pragma once

#include <stdint.h>

struct Status {
	static const uint8_t COMMAND_OK            = 0x00;
	static const uint8_t COMMAND_FAILED        = 0x01;
	static const uint8_t COMMAND_NOT_SUPPORTED = 0x02;
	static const uint8_t INVALID_TRANSFER      = 0x03;
	static const uint8_t INVALID_CRC           = 0x04;
	static const uint8_t INVALID_ARGUMENTS     = 0x05;
	// Never sent, used internally to indicate no status should be
	// returned.
	static const uint8_t NO_REPLY              = 0xff;
};

struct GeneralCallCommands {
	static const uint8_t RESET = 0x06;
	static const uint8_t RESET_ADDRESS = 0x04;
};

struct cmd_result {
	cmd_result(uint8_t status, uint8_t len = 0) : status(status), len(len) {}
	uint8_t status;
	uint8_t len;
};

inline cmd_result cmd_ok(uint8_t len = 0) {
	return cmd_result(Status::COMMAND_OK, len);
}

cmd_result processCommand(uint8_t cmd, uint8_t *data, uint8_t len, uint8_t maxLen);
