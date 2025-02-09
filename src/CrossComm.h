#ifndef CROSS_COMM_H
#define CROSS_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"

/*
 * ================================
 * Custom Types Section
 * ================================
 */
// Define message types. Extend this list with your own types as needed.
typedef enum {
    MSG_TYPE_RAW = 0,       // Raw binary data
    MSG_TYPE_EXAMPLE = 1,   // Example struct message
    // Add additional message types here...
} MessageType;

// Optionally, you might want to define the structures that go with your messages.
// For example:
typedef struct {
    int someField;
    float anotherField;
} ExampleMessage;

typedef unsigned int MessageSize;
/*
 * ================================
 * API Functions
 * ================================
 */

/* 
 * Service (server) side functions.
 * A process can host one service (and can also act as a client).
 */

// StartService() begins hosting a TCP service identified by a name.
// Returns TS_SUCCESS or an error code.
int StartService(const char* myServiceName);

// PostMessage() sends a message to all connected clients.
// The protocol automatically prepends a one-byte message type header.
// Returns TS_SUCCESS or an error code.
// Note: dataLen is the length of the payload (excluding the type byte).
int PostMessage(MessageType msgType, void* data, unsigned int dataLen);

// CloseService() stops the service and closes all client connections.
// Returns TS_SUCCESS or an error code.
int CloseService();

/*
 * Client side functions.
 * A client can connect to many services, but only once per service.
 * The callback will be called with the message type, a pointer to the payload,
 * and the payload length.
 */
int ConnectToService(const char* serviceName, void (*onDataReceivedCallback)(MessageType, void*, MessageSize));
int CloseConnection(const char* serviceName);

/*
 * Return Codes
 */
#define TS_SUCCESS 0
#define TS_ERR_ALREADY_HOSTING -1
#define TS_ERR_NO_SERVICE -2
#define TS_ERR_CONNECTION_FAIL -3
#define TS_ERR_ALREADY_CONNECTED -4
#define TS_ERR_NOT_CONNECTED -5
#define TS_ERR_INTERNAL -6

#ifdef __cplusplus
}
#endif

#endif // TCP_SERVICE_H