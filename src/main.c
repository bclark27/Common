#include <ncurses.h>
#include "IPC.h"
#include "Common.h"

void OnPushEvent(MessageType t, void* d, MessageSize s)
{
  if (t == MSG_TYPE_ABL_PAD)
  {
    AbletonPkt_pad* pad = d;
    AbletonPkt_Cmd_Pad cmd_p = { 
      .x=pad->padX, 
      .y=pad->padY,
      .setColor=true,
      .color=ColorStates_LIGHT_RED,

    };
    IPC_PostMessage(MSG_TYPE_ABL_CMD_PAD, &cmd_p, sizeof(AbletonPkt_Cmd_Pad));

    AbletonPkt_Cmd_Text cmd_t =
    {
      .x=0,
      .y=0,
    };

    char* str = malloc(68);
    int size = snprintf(str, 68, "%d, %d, %d\n", pad->padX, pad->padY, pad->padVelocity);
    memcpy(cmd_t.text, str, size);
    cmd_t.length = size;

    IPC_PostMessage(MSG_TYPE_ABL_CMD_TEXT, &cmd_t, sizeof(AbletonPkt_Cmd_Text));
  }
}

int main()
{
  IPC_StartService("Controller"); 
  IPC_ConnectToService("PushEvents", OnPushEvent);
  while (1){}
  return 0;
}



