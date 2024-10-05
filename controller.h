#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

// Device Address Enum
typedef enum {
    ADDR_CONTROLLER  = 0x01,
    ADDR_MEMORY_CARD = 0x81
} DeviceAddress;

// Device Command Enum
typedef enum {
    CMD_INIT_PRESSURE   = '@', // Initialize DualShock pressure sensors (config)
    CMD_POLL            = 'B', // Read controller state
    CMD_CONFIG_MODE     = 'C', // Enter or exit configuration mode
    CMD_SET_ANALOG      = 'D', // Set analog mode/LED state (config)
    CMD_GET_ANALOG      = 'E', // Get analog mode/LED state (config)
    CMD_GET_MOTOR_INFO  = 'F', // Get information about a motor (config)
    CMD_GET_MOTOR_LIST  = 'G', // Get list of all motors (config)
    CMD_GET_MOTOR_STATE = 'H', // Get current state of vibration motors (config)
    CMD_GET_MODE        = 'L', // Get list of all supported modes (config)
    CMD_REQUEST_CONFIG  = 'M', // Configure poll request format (config)
    CMD_RESPONSE_CONFIG = 'O', // Configure poll response format (config)
    CMD_CARD_READ       = 'R', // Read 128-byte memory card sector
    CMD_CARD_IDENTIFY   = 'S', // Retrieve memory card size information
    CMD_CARD_WRITE      = 'W'  // Write 128-byte memory card sector
} DeviceCommand;

// Constants
#define DTR_DELAY   60
#define DSR_TIMEOUT 120

// Function declarations
void delayMicroseconds(int time);
void initControllerBus(void);
bool waitForAcknowledge(int timeout);
void selectPort(int port);
uint8_t exchangeByte(uint8_t value);
int exchangePacket(DeviceAddress address, const uint8_t *request, uint8_t *response, int reqLength, int maxRespLength);
void printControllerInfo(int port, char *output);

#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    240
#define FONT_WIDTH       96
#define FONT_HEIGHT      56
#define FONT_COLOR_DEPTH GP0_COLOR_4BPP

#endif // CONTROLLER_H
