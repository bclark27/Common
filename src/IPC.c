#include "IPC.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

// Define the CRC-16 polynomial
#define POLYNOMIAL 0xA001
#define BUFFER_SIZE                 MB_SIZE
#define MAGIC_BYTES_LEN             6
static const char MAGIC_BYTES[] =   { 0xde, 0xad, 0xfa, 0xce, 0xbe, 0xef };

typedef unsigned short MessageChecksum;

typedef struct message_header {
    char magicBytes[MAGIC_BYTES_LEN];
    MessageType messageType;
    MessageSize messageLen;
    MessageChecksum messageChecksum;
} message_header_t;

// ---------------------------------------------------------------------
// Global variables for service (server) side state
// ---------------------------------------------------------------------
static int g_server_socket = -1;
static int g_server_port = 0;
static pthread_t g_server_thread;
static int g_service_running = 0;
static char g_service_name[256] = {0};

// Linked list of connected client sockets for sending messages
typedef struct client_node {
    int socket_fd;
    struct client_node *next;
} client_node_t;

static client_node_t *g_clients = NULL;
static pthread_mutex_t g_clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// ---------------------------------------------------------------------
// Helper: compute a port number from the service name (hash-based)
// ---------------------------------------------------------------------
static int compute_port_from_service(const char* serviceName) {
    unsigned long hash = 5381;
    const char *s = serviceName;
    while (*s) {
        hash = ((hash << 5) + hash) + (unsigned char)(*s);
        s++;
    }
    // Map hash into a port range (e.g. 50000-60000)
    int port = 50000 + (hash % 10000);
    return port;
}

MessageChecksum compute_checksum(void* data, size_t length)
{
    // Initialize the CRC register with all bits set
    MessageChecksum crc = 0xFFFF;

    for (size_t i = 0; i < length; ++i)
    {
        crc ^= ((uint8_t*)data)[i];  // XOR byte into least significant byte of CRC

        // Loop through each bit of the byte
        for (unsigned char j = 0; j < 8; ++j)
        {
            if (crc & 0x0001)  // If the LSB is set, XOR with polynomial
                crc = (crc >> 1) ^ POLYNOMIAL;
            else              // Otherwise, just shift right
                crc >>= 1;
        }
    }

    return crc;
}

static size_t create_header(message_header_t* header, void* message, MessageType messageType, MessageSize messageLen)
{
    memcpy(&header->magicBytes, MAGIC_BYTES, MAGIC_BYTES_LEN);
    header->messageType = messageType;
    header->messageLen = messageLen;
    header->messageChecksum = compute_checksum(message, messageLen);
    return messageLen + sizeof(message_header_t);
}

// ---------------------------------------------------------------------
// Server accept thread: listens for new client connections.
// ---------------------------------------------------------------------
static void* server_accept_thread(void* arg) {
    (void)arg;
    while (g_service_running) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(g_server_socket, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR || !g_service_running)
                break;
            perror("accept");
            continue;
        }

        // Add client_fd to global list
        client_node_t *node = malloc(sizeof(client_node_t));
        if (!node) {
            perror("malloc");
            close(client_fd);
            continue;
        }
        node->socket_fd = client_fd;
        pthread_mutex_lock(&g_clients_mutex);
        node->next = g_clients;
        g_clients = node;
        pthread_mutex_unlock(&g_clients_mutex);
    }
    return NULL;
}

// ---------------------------------------------------------------------
// Service Side API Implementation
// ---------------------------------------------------------------------

int IPC_StartService(const char* myServiceName) {
    if (g_service_running) {
        return CC_ERR_ALREADY_HOSTING;
    }
    if (!myServiceName) {
        return CC_ERR_INTERNAL;
    }

    strncpy(g_service_name, myServiceName, sizeof(g_service_name)-1);
    g_service_name[sizeof(g_service_name)-1] = '\0';

    // Compute port using hash-based strategy
    g_server_port = compute_port_from_service(myServiceName);

    // Create a TCP socket
    g_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_socket < 0) {
        perror("socket");
        return CC_ERR_INTERNAL;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to localhost on the computed port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(g_server_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Change to INADDR_LOOPBACK to restrict to localhost

    if (bind(g_server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(g_server_socket);
        return CC_ERR_INTERNAL;
    }

    if (listen(g_server_socket, 10) < 0) {
        perror("listen");
        close(g_server_socket);
        return CC_ERR_INTERNAL;
    }

    g_service_running = 1;

    // Start the accept thread
    if (pthread_create(&g_server_thread, NULL, server_accept_thread, NULL) != 0) {
        perror("pthread_create");
        close(g_server_socket);
        g_service_running = 0;
        return CC_ERR_INTERNAL;
    }

    printf("Service '%s' started on port %d\n", myServiceName, g_server_port);
    return CC_SUCCESS;
}

// PostMessage() sends a message to all connected clients.
// It automatically prepends a one-byte message type header.
int IPC_PostMessage(MessageType msgType, void* data, unsigned int dataLen) {
    if (!data || dataLen == 0) return CC_ERR_INTERNAL;

    // Allocate a buffer for header (1 byte) + payload + magic bytes
    uint8_t packet[BUFFER_SIZE];
    size_t packetSize = create_header((message_header_t*)packet, data, msgType, dataLen);
    memcpy(packet + sizeof(message_header_t), data, dataLen);

    pthread_mutex_lock(&g_clients_mutex);
    client_node_t *curr = g_clients;
    while (curr) {
        ssize_t sent = send(curr->socket_fd, packet, packetSize, 0);
        if (sent < 0) {
            perror("send");
            // In a more robust implementation, mark and remove a disconnected client.
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&g_clients_mutex);

    return CC_SUCCESS;
}

int IPC_CloseService() {
    if (!g_service_running) return CC_ERR_INTERNAL;
    g_service_running = 0;
    // Close the listening socket to unblock accept
    close(g_server_socket);
    pthread_join(g_server_thread, NULL);

    // Optionally notify clients about shutdown, then close each connection.
    pthread_mutex_lock(&g_clients_mutex);
    client_node_t *curr = g_clients;
    while (curr) {
        close(curr->socket_fd);
        client_node_t *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    g_clients = NULL;
    pthread_mutex_unlock(&g_clients_mutex);

    printf("Service '%s' closed.\n", g_service_name);
    return CC_SUCCESS;
}

// ---------------------------------------------------------------------
// Client Side API Implementation
// ---------------------------------------------------------------------

// Structure to hold a client connection.
typedef struct client_connection {
    char serviceName[256];
    int socket_fd;
    pthread_t thread;
    // Updated callback: first parameter is the message type.
    void (*onDataReceivedCallback)(MessageType, void*, MessageSize);
    struct client_connection* next;
} client_connection_t;


typedef struct message_parser_state {
    char magicBytesState[MAGIC_BYTES_LEN];
    char workingBuffer[BUFFER_SIZE];
    unsigned int currentBufferIdx;
    char gotMagic;
    char parsingMessage;
    char gotHeader;
    message_header_t* header;
} message_parser_state_t;

static void* parse_byte(message_parser_state_t* state, uint8_t byte)
{
    for (int i = 0; i < MAGIC_BYTES_LEN - 1; i++)
    {
        state->magicBytesState[i] = state->magicBytesState[i + 1];
    }
    state->magicBytesState[MAGIC_BYTES_LEN - 1] = byte;
    
    // check if we got magic
    char gotNewMagic = 1;
    for (int i = 0; gotNewMagic && i < MAGIC_BYTES_LEN; i++)
        gotNewMagic &= MAGIC_BYTES[i] == state->magicBytesState[i];

    // if we got magic then restart the finished message and return 0
    if (gotNewMagic)
    {
        state->currentBufferIdx = MAGIC_BYTES_LEN;
        state->gotMagic = 1;
        state->parsingMessage = 1;
        return NULL;
    }

    if (!state->gotMagic || !state->parsingMessage)
        return NULL;

    state->workingBuffer[state->currentBufferIdx++] = byte;

    // if just now got the header, parse out the checksum and size
    if (state->currentBufferIdx == sizeof(message_header_t))
    {
        state->gotHeader = 1;
        state->header = (message_header_t*)state->workingBuffer;
        return NULL;
    }

    if (!state->gotHeader)
        return NULL;

    if (state->currentBufferIdx == state->header->messageLen + sizeof(message_header_t))
    {
        void* payload = state->workingBuffer + sizeof(message_header_t);
        char validated = compute_checksum(payload, state->header->messageLen) == state->header->messageChecksum;
        
        state->gotMagic = 0;

        if (validated)
            return state->workingBuffer + sizeof(message_header_t);
        else
            return NULL;
    }

    return NULL;
}


static client_connection_t *g_client_connections = NULL;
static pthread_mutex_t g_client_connections_mutex = PTHREAD_MUTEX_INITIALIZER;

// Client receive thread: reads messages and calls the callback.
// It expects each message to begin with a one-byte message type.
static void* client_receive_thread(void* arg) {
    client_connection_t *conn = (client_connection_t*) arg;
    char buffer[BUFFER_SIZE];
    message_parser_state_t parseState;
    parseState.gotMagic = 0;

    while (1) {
        ssize_t recvd = recv(conn->socket_fd, buffer, BUFFER_SIZE, 0);
        if (recvd <= 0) {
            // Connection closed or error.
            break;
        }
        if (recvd < 1) {
            // Should never happen because we expect at least one byte (the type)
            continue;
        }
        
        for (ssize_t i = 0; i < recvd; i++)
        {
            void* message = parse_byte(&parseState, buffer[i]);
            if (message)
                if (conn->onDataReceivedCallback)
                    conn->onDataReceivedCallback(parseState.header->messageType, message, parseState.header->messageLen);
        }
    }
    // Clean up: close socket and remove connection from list.
    close(conn->socket_fd);
    pthread_mutex_lock(&g_client_connections_mutex);
    if (g_client_connections == conn) {
        g_client_connections = conn->next;
    } else {
        client_connection_t *prev = g_client_connections;
        while (prev && prev->next != conn) prev = prev->next;
        if (prev) prev->next = conn->next;
    }
    pthread_mutex_unlock(&g_client_connections_mutex);
    free(conn);
    return NULL;
}

int IPC_ConnectToService(const char* serviceName, void (*onDataReceivedCallback)(MessageType, void*, MessageSize)) {
    if (!serviceName) return CC_ERR_INTERNAL;

    // Check if we already have a connection to this service.
    pthread_mutex_lock(&g_client_connections_mutex);
    client_connection_t *iter = g_client_connections;
    while (iter) {
        if (strcmp(iter->serviceName, serviceName) == 0) {
            pthread_mutex_unlock(&g_client_connections_mutex);
            return CC_ERR_ALREADY_CONNECTED;
        }
        iter = iter->next;
    }
    pthread_mutex_unlock(&g_client_connections_mutex);

    // Assume the service is on localhost; compute the port using the same hash.
    int port = compute_port_from_service(serviceName);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return CC_ERR_INTERNAL;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return CC_ERR_CONNECTION_FAIL;
    }

    // Create a new client connection structure.
    client_connection_t *conn = malloc(sizeof(client_connection_t));
    if (!conn) {
        close(sockfd);
        return CC_ERR_INTERNAL;
    }
    strncpy(conn->serviceName, serviceName, sizeof(conn->serviceName)-1);
    conn->serviceName[sizeof(conn->serviceName)-1] = '\0';
    conn->socket_fd = sockfd;
    conn->onDataReceivedCallback = onDataReceivedCallback;
    conn->next = NULL;

    // Start the receive thread.
    if (pthread_create(&conn->thread, NULL, client_receive_thread, conn) != 0) {
        perror("pthread_create");
        close(sockfd);
        free(conn);
        return CC_ERR_INTERNAL;
    }

    // Add connection to global list.
    pthread_mutex_lock(&g_client_connections_mutex);
    conn->next = g_client_connections;
    g_client_connections = conn;
    pthread_mutex_unlock(&g_client_connections_mutex);

    printf("Connected to service '%s' on port %d\n", serviceName, port);
    return CC_SUCCESS;
}

int IPC_CloseConnection(const char* serviceName) {
    if (!serviceName) return CC_ERR_INTERNAL;

    pthread_mutex_lock(&g_client_connections_mutex);
    client_connection_t *iter = g_client_connections;
    client_connection_t *prev = NULL;
    while (iter) {
        if (strcmp(iter->serviceName, serviceName) == 0) {
            // Close the connection by closing the socket. The receive thread will exit.
            close(iter->socket_fd);
            // Join the thread to ensure cleanup.
            pthread_join(iter->thread, NULL);
            // Remove the connection from the list.
            if (prev) {
                prev->next = iter->next;
            } else {
                g_client_connections = iter->next;
            }
            free(iter);
            pthread_mutex_unlock(&g_client_connections_mutex);
            printf("Closed connection to service '%s'\n", serviceName);
            return CC_SUCCESS;
        }
        prev = iter;
        iter = iter->next;
    }
    pthread_mutex_unlock(&g_client_connections_mutex);
    return CC_ERR_NOT_CONNECTED;
}
