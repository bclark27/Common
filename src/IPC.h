#ifndef IPC_H_
#define IPC_H_

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
    MSG_TYPE_RAW = 0,       // Raw binary data with header to define length
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
int IPC_StartService(const char* myServiceName);

// PostMessage() sends a message to all connected clients.
// The protocol automatically prepends a one-byte message type header.
// Returns TS_SUCCESS or an error code.
// Note: dataLen is the length of the payload (excluding the type byte).
int IPC_PostMessage(MessageType msgType, void* data, unsigned int dataLen);

// CloseService() stops the service and closes all client connections.
// Returns TS_SUCCESS or an error code.
int IPC_CloseService();

/*
 * Client side functions.
 * A client can connect to many services, but only once per service.
 * The callback will be called with the message type, a pointer to the payload,
 * and the payload length.
 */
int IPC_ConnectToService(const char* serviceName, void (*onDataReceivedCallback)(MessageType, void*, MessageSize));
int IPC_CloseConnection(const char* serviceName);

/*
 * Return Codes
 */
#define CC_SUCCESS 0
#define CC_ERR_ALREADY_HOSTING -1
#define CC_ERR_NO_SERVICE -2
#define CC_ERR_CONNECTION_FAIL -3
#define CC_ERR_ALREADY_CONNECTED -4
#define CC_ERR_NOT_CONNECTED -5
#define CC_ERR_INTERNAL -6

#ifdef __cplusplus
}
#endif

#endif // TCP_SERVICE_H
