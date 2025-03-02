#include <stdio.h>
#include "Common.h"
#include "USB.h"


int main()
{

  USB_handle* h = USB_init(1, 21, 2536);
  int ok[] = {120, 120 ,120,120,123,123,123,123};
  int a = USB_send_data(h, ok, 8 * 4);
  printf("%d\n", a);
  USB_free(h);
  USB_destroy_context();
  return 0;
}



