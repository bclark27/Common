#include <stdio.h>
#include "Common.h"
#include "CrossComm.h"
#include "MemoryPool.h"
#include "Entity.h"

// Use a volatile global counter so that the compiler doesn't optimize it away.
// (Note: In production code you might want to use atomic operations.)
unsigned long messages_received = 0;

// Client callback: increments the counter for each message received.
void speed_callback(uint8_t msgType, void* payload, unsigned int payloadLen) {
    // We don't print every message to avoid slowing down the callback.
    messages_received++;

    printf("Client got: ");

    for(int i = 0; i < payloadLen; i++)
    {
        printf("%c", ((char*)payload)[i]);
    }
    printf("\n");

}

static long countingUp = 0;
bool callback(void* data)
{
    countingUp += ((int*)data)[0];
    return true;
}

int main()
{
    /*
  pid_t pid = fork();
  if (pid < 0) {
      perror("fork");
      exit(EXIT_FAILURE);
  }

  // Number of messages to send from the server.
  const unsigned long NUM_MESSAGES = 10; // Adjust this as needed

  if (pid == 0) {
      // -------------------------------------------------
      // Child Process: Acts as the Client
      // -------------------------------------------------
      // Give the server time to start.
      sleep(1);

      printf("Client: Connecting to service 'SpeedTestService'...\n");
      int ret = ConnectToService("SpeedTestService", speed_callback);
      if (ret != TS_SUCCESS) {
          fprintf(stderr, "Client: Failed to connect to service (error %d)\n", ret);
          exit(EXIT_FAILURE);
      }
      printf("Client: Connected. Waiting for messages...\n");

      // Wait long enough for all messages to be received.
      sleep(5);

      printf("Client: Total messages received: %lu (expected ~%lu)\n", messages_received, NUM_MESSAGES);

      ret = CloseConnection("SpeedTestService");
      if (ret != TS_SUCCESS) {
          fprintf(stderr, "Client: Failed to close connection (error %d)\n", ret);
      }
      exit(EXIT_SUCCESS);
  } else {
      // -------------------------------------------------
      // Parent Process: Acts as the Server
      // -------------------------------------------------
      printf("Server: Starting service 'SpeedTestService'...\n");
      int ret = StartService("SpeedTestService");
      if (ret != TS_SUCCESS) {
          fprintf(stderr, "Server: Failed to start service (error %d)\n", ret);
          exit(EXIT_FAILURE);
      }

      // Allow time for the client to connect.
      sleep(2);

      // Prepare a test message.
      const char *msg = "Speed test message";

      // Send NUM_MESSAGES messages as fast as possible.
      for (unsigned long i = 0; i < NUM_MESSAGES; i++) {
          ret = PostMessage(MSG_TYPE_RAW, msg, (unsigned int) (sizeof("Speed test message") - 1));
          if (ret != TS_SUCCESS) {
              fprintf(stderr, "Server: Failed to post message %lu (error %d)\n", i, ret);
          }
      }
      printf("Server: Finished sending %lu messages.\n", NUM_MESSAGES);

      // Allow some time for the client to finish receiving messages.
      sleep(5);

      ret = CloseService();
      if (ret != TS_SUCCESS) {
          fprintf(stderr, "Server: Failed to close service (error %d)\n", ret);
      } else {
          printf("Server: Service closed.\n");
      }

      // Wait for the child process to finish.
      wait(NULL);
  }
    */
  
    clock_t start, end;
    int dataCount = 400000;
    int dataSize = 4;
    int count = 0;


    // void* allData[dataCount];
    // for (int i = 0; i < dataCount; i++)
    // {
    //     const int okok = 100;
    //     void* tmps[okok];
    //     for (int k = 0; k < okok; k++)
    //         tmps[k] = malloc(dataSize);


    //     allData[i] = malloc(dataSize);

    // }

    // start = clock();

    // for (int i = 0; i < dataCount; i++)
    // {
    //     callback(allData[i]);
    // }

    // end = clock();


  MemoryPool* mp = MemoryPool_init(dataSize );
  for (int i = 0; i < dataCount; i++)
  {
    MemoryPool_AddItemInitialData(mp, &i);
  }



    start = clock();

  MemoryPool_Iter(mp, callback);

  end = clock();



  printf("TIME: %lf\n%d\n", (double)(end - start) / CLOCKS_PER_SEC, count);
  
  return 0;
}



