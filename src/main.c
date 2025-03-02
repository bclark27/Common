#include <ncurses.h>
#include "IPC.h"

void OnPushEvent(MessageType t, void* d, MessageSize s)
{
  if (t == MSG_TYPE_ABL_PAD)
  {
    AbletonPkt_pad* pad = d;
    printf("%d, %d, %d\n", pad->padX, pad->padY, pad->padVelocity);
    IPC_PostMessage(MSG_TYPE_RAW, "hello", 5);
  }
}

int main()
{
  IPC_StartService("Controller"); 
  IPC_ConnectToService("PushEvents", OnPushEvent);
  while (1){}
  return 0;
}



