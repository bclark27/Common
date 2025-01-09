#include <stdio.h>
#include "Common.h"

int main()
{
  mem_init();

  void * z = malloc(12);

  mem_print();

  mem_freeTracker();
}
